#ifndef _WIFI_
#define _WIFI_

#include <string.h>
#include "esp_wifi.h"  // Include ESP32 Wi-Fi driver functions
#include "esp_log.h"   // Include ESP32 logging functions

extern wifi_ap_record_t ap_info[];  // Declare an array to store the AP records

#define DEFAULT_SCAN_LIST_SIZE 15 // Max number of APs to store (0 to 20)

void wifi_init(void);  // Function to initialize Wi-Fi
void wifi_scan(void);  // Function to scan available Wi-Fi networks

#endif
