#include "wifi.h"
#include "gui_paint.h"

const char *TAG_STA = "wifi_sta";    // Tag for Station mode (Wi-Fi client mode)
esp_netif_ip_info_t ip_info; // Stores the IP information once connected to Wi-Fi

// Function to wait for Wi-Fi connection and obtain IP address
void wifi_wait_connect()
{
    static int s_retry_num = 0;  // Counter to track the number of connection retries
    wifi_config_t sta_config;

    // Get the current Wi-Fi station configuration (SSID, password, etc.)
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &sta_config));

    while (1)
    {
        // Get the network interface handle for the default Wi-Fi STA interface
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif)
        {
            // Get the IP information associated with the Wi-Fi STA interface
            esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
            if (ret == ESP_OK && ip_info.ip.addr != 0) {
                // If successfully connected and an IP address is obtained
                ESP_LOGI("WiFi", "Connected with IP: " IPSTR, IP2STR(&ip_info.ip));
                ESP_LOGI(TAG_STA, "Connected to AP SSID:%s, password:%s", sta_config.sta.ssid, sta_config.sta.password);
                
                char ip[20];
                // Clear the top section of the screen to display connection info
                Paint_ClearWindows(440, 160, 800, 185, WHITE);
                snprintf(ip, sizeof(ip), "IP " IPSTR, IP2STR(&ip_info.ip));  // Format the IP address
                Paint_DrawString_EN(440, 160, ip, &Font24, BLACK, WHITE);  // Display IP on the screen
                Paint_DrawString_EN(440, 200, (const char *)sta_config.sta.ssid, &Font24, BLACK, WHITE); // Display SSID
                Paint_DrawString_EN(440, 240, (const char *)sta_config.sta.password, &Font24, BLACK, WHITE); // Display password

                s_retry_num = 0;  // Reset retry counter on successful connection
                break;  // Exit the loop since the connection is successful
            } else {
                // Log the failure to connect or obtain an IP address
                ESP_LOGI(TAG_STA, "Failed to connect to the AP");

                // Retry connection if the retry counter is less than 5
                if (s_retry_num < 5)
                {
                    s_retry_num++;
                    ESP_LOGI(TAG_STA, "Retrying to connect to the AP");
                }
                else {
                    // Reset retry counter after 5 attempts and log the failure
                    s_retry_num = 0;

                    ESP_LOGI(TAG_STA, "Failed to connect to SSID:%s, password:%s",
                            sta_config.sta.ssid, sta_config.sta.password);
                    
                    // Clear the top section of the screen to display failure message
                    Paint_ClearWindows(440, 160, 800, 185, WHITE);
                    Paint_DrawString_EN(440, 160, "Failed to connect to the AP.", &Font24, BLACK, WHITE);        
                    break;  // Exit the loop after failed retries
                }

                // Wait for 1 second before retrying
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
        else 
        {
            // Log an error if the network interface handle is not found
            ESP_LOGE("WiFi", "Netif handle not found");
        }

        // Short delay (10ms) before checking the connection status again
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Function to initialize Wi-Fi in Station mode (STA mode) and connect to an AP
void wifi_sta_init(uint8_t *ssid, uint8_t *pwd, wifi_auth_mode_t authmode)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  // Set Wi-Fi mode to Station (STA) initially
    
    wifi_config_t wifi_config = {              \
        .sta = {                                \
            .threshold.authmode = authmode,     \
        },                                      \
    };

    // Copy SSID and password to the Wi-Fi configuration structure
    strcpy((char *)wifi_config.sta.ssid, (const char *)ssid);
    strcpy((char *)wifi_config.sta.password, (const char *)pwd);

    // Set the Wi-Fi configuration for the station (STA) interface
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_netif_create_default_wifi_sta();  // Create the default network interface for Wi-Fi STA
    esp_wifi_start(); // Start the Wi-Fi with the new configuration
    ESP_ERROR_CHECK(esp_wifi_connect());  // Attempt to connect to the Wi-Fi AP
    wifi_wait_connect();  // Wait for the connection to establish and get IP address
}
