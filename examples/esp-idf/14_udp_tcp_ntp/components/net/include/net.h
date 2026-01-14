#pragma once

#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// WiFi configuration
#define EXAMPLE_ESP_WIFI_SSID      "CQ793"
#define EXAMPLE_ESP_WIFI_PASS      "123456789"
#define EXAMPLE_SERVER_IP          "192.168.137.1"
#define EXAMPLE_TCP_PORT           12345
#define EXAMPLE_UDP_PORT           12346

// WiFi event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_IP_UPDATED_BIT BIT1

/**
 * @brief Initialize WiFi STA mode
 */
void wifi_init_sta(void);

/**
 * @brief Initialize SNTP service and sync time
 */
void initialize_sntp(void);

/**
 * @brief Send TCP message
 * @param msg String to send
 */
void app_send_tcp(const char *msg);

/**
 * @brief Send UDP message
 * @param msg String to send
 */
void app_send_udp(const char *msg);

/**
 * @brief Get server IP address
 * @return Server IP string
 */
const char* app_get_server_ip(void);

/**
 * @brief Get WiFi event group handle
 * @return EventGroupHandle_t 
 */
EventGroupHandle_t get_wifi_event_group(void);

const char* net_get_last_ip(void);

void net_start_udp_server(void);

void net_start_tcp_server(void);
