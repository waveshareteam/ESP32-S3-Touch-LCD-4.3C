#ifndef __USER_WIFI_H
#define __USER_WIFI_H

// Include the WiFi library for handling wireless functionality
#include "WiFi.h"

// Function declarations

/**
 * @brief Initializes WiFi in station mode and disconnects any previous connections.
 * This function sets up the ESP32 for WiFi scanning and other WiFi-related operations.
 */
void wifi_scan_init();

/**
 * @brief Scans for available WiFi networks and displays the results.
 * The scan results include SSID, signal strength, channel, and encryption type.
 */
void wifi_scan();

/**
 * @brief Initializes WiFi in station mode and connects to a specific network.
 * This function connects the ESP32 to a WiFi network using the provided SSID and password.
 * @param ssid The SSID (name) of the WiFi network to connect to.
 * @param pwd The password for the WiFi network.
 */
void wifi_sta_init(const char *ssid, const char *pwd);

#endif // __USER_WIFI_H
