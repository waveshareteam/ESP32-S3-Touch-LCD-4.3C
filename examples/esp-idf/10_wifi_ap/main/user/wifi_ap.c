#include "wifi.h"

const char *TAG_AP = "WiFi SoftAP";  // Tag for logging in SoftAP mode

wifi_sta_list_t sta_list;  // List to hold connected stations' information

// Function to initialize the Wi-Fi Access Point (AP) mode
void wifi_ap_init(uint8_t *ssid, uint8_t *pwd, uint8_t channel)
{
    wifi_config_t wifi_ap_config = {
        .ap = {
            .channel = channel, // Wi-Fi channel for the Access Point (AP)
            .max_connection = 5, // Maximum number of allowed devices that can connect to the AP
            .authmode = WIFI_AUTH_WPA2_PSK, // WPA2 encryption for secure connections
            .pmf_cfg = {
                .required = false, // Disable Protected Management Frames (PMF), not mandatory for the current setup
            },
        },
    };

    // Copy the SSID (network name) and password into the Wi-Fi configuration
    strcpy((char *)wifi_ap_config.ap.ssid, (const char *)ssid);  // Copy SSID to the configuration
    wifi_ap_config.ap.ssid_len = strlen((const char *)ssid);  // Set SSID length
    strcpy((char *)wifi_ap_config.ap.password, (const char *)pwd);  // Copy password to the configuration

    // If the password is empty, set the AP to open authentication (no password)
    if (strlen((const char *)pwd) == 0) {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;  // Open authentication mode
    }

    // Create the default Wi-Fi AP network interface
    esp_netif_create_default_wifi_ap();

    // Set the Wi-Fi mode to Access Point (AP)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    // Start the Wi-Fi AP (Access Point)
    ESP_ERROR_CHECK(esp_wifi_start());

    // Set the Wi-Fi AP configuration (SSID, password, and other settings)
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    // Log the status and the AP settings (SSID, password, and channel)
    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ssid, pwd, channel);
}
