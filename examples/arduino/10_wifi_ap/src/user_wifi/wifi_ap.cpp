#include <sys/_stdint.h>
#include <esp_mac.h>
#include <esp_wifi.h>

#include "user_wifi.h"
#include "../rgb_lcd_port/gui_paint/gui_paint.h"

/**
 * @brief Initializes the ESP32 in Access Point (AP) mode.
 *
 * This function sets up the ESP32 to operate as a WiFi hotspot with the given SSID and password.
 * After starting the AP, it prints the assigned IP address to the serial monitor.
 *
 * @param ssid The SSID (name) of the WiFi network to be created.
 * @param pwd The password required to connect to the AP.
 */
void wifi_ap_init(const char *ssid, const char *pwd)
{
  WiFi.softAP(ssid, pwd);
  Serial.println("Hotspot Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP()); // Print the IP address of the AP
}

/**
 * @brief Returns the number of devices currently connected to the ESP32's AP.
 *
 * @return The number of connected stations (client devices).
 */
uint8_t wifi_ap_StationNum()
{
  return WiFi.softAPgetStationNum();
}

/**
 * @brief Retrieves the MAC address of a connected station (client) by index.
 *
 * This function fetches the list of currently connected stations and extracts
 * the MAC address of the station at the specified index, formatting it as a string.
 *
 * @param mac A character buffer to store the formatted MAC address (must be at least MAC_Addr_length).
 * @param num The index of the connected station (0-based).
 */
void wifi_ap_StationMac(char *mac, uint8_t num)
{
  static wifi_sta_list_t sta_list; // Structure to store station info list

  // Retrieve the current list of connected stations
  esp_wifi_ap_get_sta_list(&sta_list);

  // Get the station info at the specified index
  wifi_sta_info_t sta_info = sta_list.sta[num];

  // Format the MAC address as a human-readable string (e.g., "AA:BB:CC:DD:EE:FF")
  snprintf(mac, MAC_Addr_length, MACSTR, MAC2STR(sta_info.mac));
}
