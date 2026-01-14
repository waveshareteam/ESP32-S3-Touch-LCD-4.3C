#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "i2c.h"
#include "io_extension.h"
#include "rgb_lcd_port.h"
#include "gt911.h"
#include "lvgl_port.h"
#include "ui_app.h"
#include "net.h"

static const char *TAG = "main";

void peripheral_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Initialize Hardware
    DEV_I2C_Init();
    IO_EXTENSION_Init();
    // Initialize LCD and Touch
    esp_lcd_panel_handle_t panel_handle = waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();
    esp_lcd_touch_handle_t tp_handle = touch_gt911_init(DEV_I2C_Get_Bus_Device());
    // Initialize LVGL
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));
    // Initialize UI
    ui_init();
    ui_log("System Initialized. Waiting for WiFi...");
    // Initialize WiFi
    wifi_init_sta();
    // Wait for WiFi connection
    xEventGroupWaitBits(get_wifi_event_group(), WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    net_start_udp_server();
    net_start_tcp_server();
    // Initialize NTP
    initialize_sntp();
}
    
void app_main(void)
{
    peripheral_init();
    while (1) {
        EventGroupHandle_t wifi_event_group = get_wifi_event_group();
        if (wifi_event_group) {
            const EventBits_t bits = xEventGroupGetBits(wifi_event_group);
            if (bits & WIFI_IP_UPDATED_BIT) {
                ui_update_ip(net_get_last_ip()) ? (void)xEventGroupClearBits(wifi_event_group, WIFI_IP_UPDATED_BIT) : (void)0;
                ui_log((bits & WIFI_CONNECTED_BIT) ? "WiFi Connected" : "WiFi Disconnected");
            }
        }

        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[64];
        if (timeinfo.tm_year > (2016 - 1900)) {
            strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
            ui_update_time(strftime_buf);
        } else {
            ui_update_time("Syncing...");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
