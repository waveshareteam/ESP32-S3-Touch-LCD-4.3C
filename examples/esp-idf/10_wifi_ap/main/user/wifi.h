#ifndef _WIFI_
#define _WIFI_

#include <string.h>
#include "esp_wifi.h"  // Include ESP32 Wi-Fi driver functions
#include "esp_log.h"   // Include ESP32 logging functions
#include "esp_mac.h"

extern const char *TAG_AP;

extern wifi_ap_record_t ap_info[];  // Declare an array to store the AP records

extern esp_netif_ip_info_t ip_info; // Stores IP information

extern wifi_sta_list_t sta_list;  // List to hold connected stations information

#define DEFAULT_SCAN_LIST_SIZE 15 // Max number of APs to store (0 to 20)

void wifi_init(void);  // Function to initialize Wi-Fi
void wifi_scan(void);  // Function to scan available Wi-Fi networks

// Initialize Wi-Fi in STA mode with SSID, password, and auth mode
void wifi_sta_init(uint8_t *ssid, uint8_t *pwd, wifi_auth_mode_t authmode);

// Initialize SoftAP with SSID, password, and channel
void wifi_ap_init(uint8_t *ssid, uint8_t *pwd, uint8_t channel);

#endif
