#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "net.h"
#include "ui_app.h" // Used for ui_log and ui_update_ip

static const char *TAG = "net";

// WiFi Event Group
static EventGroupHandle_t s_wifi_event_group;
static struct sockaddr_in s_learned_server_addr; // Used to save the learned server address
static bool s_ip_learned = false;               // Whether the IP has been learned
static char s_last_ip[IP4ADDR_STRLEN_MAX] = "Failed";
static QueueHandle_t s_net_work_q = NULL;
static TaskHandle_t s_tcp_server_task_handle = NULL;
static QueueHandle_t s_tcp_tx_q = NULL;
static volatile bool s_tcp_client_connected = false;

void udp_server_task(void *pvParameters);
static void tcp_server_task(void *pvParameters);

typedef enum {
    NET_WORK_TCP = 0,
    NET_WORK_UDP = 1,
} net_work_kind_t;

typedef struct {
    net_work_kind_t kind;
    char payload[256];
} net_work_item_t;

typedef struct {
    char payload[256];
} net_tcp_tx_item_t;

static void net_tcp_txq_start(void)
{
    s_tcp_tx_q ? (void)0 : (void)(s_tcp_tx_q = xQueueCreate(8, sizeof(net_tcp_tx_item_t)));
}

static int net_set_nonblocking(int sock, bool enable)
{
    const int flags = fcntl(sock, F_GETFL, 0);
    const int new_flags = enable ? (flags | O_NONBLOCK) : (flags & (~O_NONBLOCK));
    return (flags < 0) ? -1 : fcntl(sock, F_SETFL, new_flags);
}

static int net_connect_with_timeout(int sock, const struct sockaddr *addr, socklen_t addrlen, int timeout_ms)
{
    if (net_set_nonblocking(sock, true) < 0) {
        return -1;
    }

    const int rc = connect(sock, addr, addrlen);
    if (rc == 0) {
        net_set_nonblocking(sock, false);
        return 0;
    }

    if (errno != EINPROGRESS) {
        return -1;
    }

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);

    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };

    const int sel = select(sock + 1, NULL, &wfds, NULL, &tv);
    if (sel <= 0) {
        errno = (sel == 0) ? ETIMEDOUT : errno;
        return -1;
    }

    int so_error = 0;
    socklen_t so_error_len = sizeof(so_error);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len) < 0) {
        return -1;
    }

    if (so_error != 0) {
        errno = so_error;
        return -1;
    }

    net_set_nonblocking(sock, false);
    return 0;
}

static struct sockaddr_in net_get_dest_addr(uint16_t port)
{
    struct sockaddr_in dest_addr = { 0 };
    dest_addr = s_ip_learned ? s_learned_server_addr : dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = s_ip_learned ? dest_addr.sin_addr.s_addr : inet_addr(EXAMPLE_SERVER_IP);
    dest_addr.sin_port = htons(port);
    return dest_addr;
}

static void net_do_send_tcp(const char *msg)
{
    if (!(xEventGroupGetBits(s_wifi_event_group) & WIFI_CONNECTED_BIT)) {
        ui_log("[TCP] WiFi Not Connected");
        return;
    }

    struct sockaddr_in dest_addr = net_get_dest_addr(EXAMPLE_TCP_PORT);

    const int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ui_log("[TCP] Unable to create socket: errno %d", errno);
        return;
    }

    const int err = net_connect_with_timeout(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr), 5000);
    if (err != 0) {
        ui_log("[TCP] Socket unable to connect: errno %d", errno);
        close(sock);
        return;
    }

    const int sent = send(sock, msg, strlen(msg), 0);
    (sent < 0) ? ui_log("[TCP] Error occurred during sending: errno %d", errno)
               : ui_log("[TCP] Sent");

    close(sock);
}

static void net_do_send_udp(const char *msg)
{
    if (!(xEventGroupGetBits(s_wifi_event_group) & WIFI_CONNECTED_BIT)) {
        ui_log("[UDP] WiFi Not Connected");
        return;
    }

    struct sockaddr_in dest_addr = net_get_dest_addr(EXAMPLE_UDP_PORT);

    const int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ui_log("[UDP] Unable to create socket: errno %d", errno);
        return;
    }

    const int err = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    (err < 0) ? ui_log("[UDP] Error occurred during sending: errno %d", errno)
              : ui_log("[UDP] Sent");

    close(sock);
}

static void net_worker_task(void *pvParameters)
{
    net_work_item_t item;
    while (1) {
        if (xQueueReceive(s_net_work_q, &item, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        (item.kind == NET_WORK_TCP) ? net_do_send_tcp(item.payload) : net_do_send_udp(item.payload);
    }
}

static void net_worker_start(void)
{
    static bool started = false;
    if (!s_net_work_q) {
        s_net_work_q = xQueueCreate(8, sizeof(net_work_item_t));
    }
    if (!started && s_net_work_q) {
        xTaskCreate(net_worker_task, "net_worker", 4096, NULL, 6, NULL);
        started = true;
    }
}

const char* net_get_last_ip(void)
{
    return s_last_ip;
}

void net_start_udp_server(void)
{
    static bool udp_task_started = false;
    udp_task_started ? (void)0 : (void)xTaskCreate(udp_server_task, "udp_server", 4096, NULL, 5, NULL);
    udp_task_started = true;
}

void net_start_tcp_server(void)
{
    net_tcp_txq_start();
    s_tcp_server_task_handle ? (void)0 : (void)xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, &s_tcp_server_task_handle);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        strcpy(s_last_ip, "Failed");
        xEventGroupSetBits(s_wifi_event_group, WIFI_IP_UPDATED_BIT);
        ESP_LOGW(TAG, "wifi disconnected");
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        esp_ip4addr_ntoa(&event->ip_info.ip, s_last_ip, IP4ADDR_STRLEN_MAX);
        ESP_LOGI(TAG, "got ip: %s", s_last_ip);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_IP_UPDATED_BIT);
    }
}

void udp_server_task(void *pvParameters) {
    char rx_buffer[128];
    struct sockaddr_in local_addr;

    while (1) {
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all network interfaces
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(EXAMPLE_UDP_PORT); // Port number 12346

        if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ui_log("[UDP] Server listening on port %d", EXAMPLE_UDP_PORT);

        while (1) {
            struct sockaddr_in source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            } else {
                rx_buffer[len] = 0; // Ensure the string is null-terminated
                ui_log("[UDP] Recv: %s", rx_buffer);
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, inet_ntoa(source_addr.sin_addr));

                // Smart learning: remember the sender's IP
                if (!s_ip_learned || s_learned_server_addr.sin_addr.s_addr != source_addr.sin_addr.s_addr) {
                    s_learned_server_addr = source_addr;
                    s_ip_learned = true;
                    ui_log("[NET] Learned Server IP: %s", inet_ntoa(source_addr.sin_addr));
                }

                // Add echo functionality: send back an ACK for whatever is received
                char ack_msg[160];
                snprintf(ack_msg, sizeof(ack_msg), "ESP32 ACK: %s", rx_buffer);
                sendto(sock, ack_msg, strlen(ack_msg), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
            }
        }

        if (sock != -1) {
            close(sock);
        }
    }
}

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[256];
    struct sockaddr_in local_addr = { 0 };

    while (1) {
        const int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (listen_sock < 0) {
            ESP_LOGE(TAG, "[TCP_SRV] Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        int reuse = 1;
        setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(EXAMPLE_TCP_PORT);

        if (bind(listen_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
            ESP_LOGE(TAG, "[TCP_SRV] bind failed: errno %d", errno);
            close(listen_sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        if (listen(listen_sock, 2) < 0) {
            ESP_LOGE(TAG, "[TCP_SRV] listen failed: errno %d", errno);
            close(listen_sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ui_log("[TCP] Server listening on port %d", EXAMPLE_TCP_PORT);

        while (1) {
            struct sockaddr_in source_addr = { 0 };
            socklen_t addr_len = sizeof(source_addr);
            const int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
            if (sock < 0) {
                ESP_LOGE(TAG, "[TCP_SRV] accept failed: errno %d", errno);
                break;
            }

            struct timeval rcv_to = { .tv_sec = 0, .tv_usec = 200 * 1000 };
            struct timeval snd_to = { .tv_sec = 2, .tv_usec = 0 };
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &rcv_to, sizeof(rcv_to));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &snd_to, sizeof(snd_to));

            s_learned_server_addr = source_addr;
            s_ip_learned = true;
            s_tcp_client_connected = true;
            ui_log("[TCP] Client: %s", inet_ntoa(source_addr.sin_addr));

            while (1) {
                net_tcp_tx_item_t tx;
                for (int i = 0; i < 4; i++) {
                    if (xQueueReceive(s_tcp_tx_q, &tx, 0) != pdTRUE) {
                        break;
                    }
                    if (send(sock, tx.payload, strlen(tx.payload), 0) < 0) {
                        i = 4;
                        vTaskDelay(pdMS_TO_TICKS(10));
                        goto tcp_close;
                    }
                }

                const int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if (len > 0) {
                    rx_buffer[len] = 0;
                    ui_log("[TCP] Recv: %s", rx_buffer);
                    if (send(sock, rx_buffer, len, 0) < 0) {
                        break;
                    }
                    continue;
                }

                if (len == 0) {
                    break;
                }

                const int e = errno;
                if ((e == EAGAIN) || (e == EWOULDBLOCK)) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                    continue;
                }

                break;
            }

tcp_close:
            s_tcp_client_connected = false;
            close(sock);
        }

        close(listen_sock);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    net_worker_start();
    net_tcp_txq_start();

    ESP_ERROR_CHECK(esp_netif_init());
    // Note: The event loop may have already been created in main, assuming it is managed by the net component or already handled in main
    // If an error occurs, the next line can be removed
    // ESP_ERROR_CHECK(esp_event_loop_create_default()); 
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ui_log("Connecting to WiFi...");
}

void initialize_sntp(void)
{
    ui_log("[NTP] Starting time sync...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.ntsc.ac.cn");
    esp_sntp_init();
    
    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
}

void app_send_tcp(const char *msg) {
    net_tcp_txq_start();
    if (!s_tcp_tx_q) {
        ui_log("[TCP] Queue not ready");
        return;
    }

    if (!s_tcp_client_connected) {
        ui_log("[TCP] No client");
        return;
    }

    net_tcp_tx_item_t item;
    snprintf(item.payload, sizeof(item.payload), "%s", msg ? msg : "");
    (xQueueSend(s_tcp_tx_q, &item, 0) == pdTRUE) ? (void)0 : ui_log("[TCP] Queue full");
}

void app_send_udp(const char *msg) {
    net_worker_start();
    if (!s_net_work_q) {
        ui_log("[UDP] Queue not ready");
        return;
    }

    net_work_item_t item = { .kind = NET_WORK_UDP };
    snprintf(item.payload, sizeof(item.payload), "%s", msg ? msg : "");
    (xQueueSend(s_net_work_q, &item, 0) == pdTRUE) ? (void)0 : ui_log("[UDP] Queue full");
}

const char* app_get_server_ip(void) {
    return EXAMPLE_SERVER_IP;
}

EventGroupHandle_t get_wifi_event_group(void) {
    return s_wifi_event_group;
}
