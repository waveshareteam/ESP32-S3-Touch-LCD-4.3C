#ifndef __USER_WIFI_H
#define __USER_WIFI_H

// Include the WiFi library for handling wireless functionality
#include "WiFi.h"

// Define the length of a MAC address string in the format "XX:XX:XX:XX:XX:XX"
#define MAC_Addr_length 18

// Function declarations

/**
 * @brief Initializes WiFi in station mode and disconnects any previous connections.
 *
 * This function sets up the ESP32 for WiFi scanning and other WiFi-related operations.
 * It ensures the device is ready to scan for available networks.
 */
void wifi_scan_init();

/**
 * @brief Scans for available WiFi networks and displays the results.
 *
 * The scan results include SSID (network name), RSSI (signal strength),
 * channel number, and encryption type (e.g., WPA2, open).
 */
void wifi_scan();

/**
 * @brief Initializes WiFi in station mode and connects to a specific network.
 *
 * This function configures the ESP32 to operate as a WiFi client and attempts to
 * connect to a specified wireless access point using the given SSID and password.
 *
 * @param ssid The SSID (name) of the WiFi network to connect to.
 * @param pwd The password for the WiFi network.
 */
void wifi_sta_init(const char *ssid, const char *pwd);

/**
 * @brief Initializes the ESP32 in Access Point (AP) mode with a given SSID and password.
 *
 * This function configures the ESP32 to create its own WiFi network, allowing other
 * devices to connect to it.
 *
 * @param ssid The SSID (name) for the AP network.
 * @param pwd The password for the AP network.
 */
void wifi_ap_init(const char *ssid, const char *pwd);

/**
 * @brief Returns the number of stations (devices) currently connected to the ESP32 AP.
 *
 * @return The number of connected devices.
 */
uint8_t wifi_ap_StationNum();

/**
 * @brief Retrieves the MAC address of a connected station (client device) by index.
 *
 * This function writes the MAC address of the device at the specified index to the provided buffer.
 *
 * @param mac A character buffer where the MAC address will be stored (must be at least MAC_Addr_length).
 * @param num The index of the connected station (0-based).
 */
void wifi_ap_StationMac(char *mac, uint8_t num);

#endif // __USER_WIFI_H
