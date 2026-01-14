#include "wifi.h"

// Initialize Wi-Fi for STA (Station) and AP (Access Point) modes
void wifi_init(void)
{
    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the Wi-Fi driver with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}
