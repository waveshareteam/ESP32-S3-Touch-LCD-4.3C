#include "user_wifi.h"
#include "../rgb_lcd_port/gui_paint/gui_paint.h"

// Function to initialize WiFi in station mode
void wifi_scan_init() {

    // Set WiFi to station mode (WIFI_STA) and ensure it is disconnected from any previously connected AP.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    // Short delay to allow WiFi mode and disconnection settings to take effect
    delay(100);
}

// Function to check if a given SSID contains Chinese characters
bool contains_chinese(const char *ssid) {
    while (*ssid) {
        // Check if the highest bit of the character is set, indicating a non-ASCII character (possibly Chinese)
        if ((*ssid & 0x80) != 0) { 
            return true; // Return true if a non-ASCII character is found
        }
        ssid++; // Move to the next character
    }
    return false; // Return false if no non-ASCII characters are found
}

// Function to scan available WiFi networks and display results
void wifi_scan() {
    Serial.println("Scan start");
    uint8_t chinese_num = 0; // Counter for networks with Chinese SSIDs

    // Start WiFi scan and get the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        // If no networks are found, print a message
        Serial.println("no networks found");
    } else {
        // Print the number of networks found
        Serial.print(n);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");

        // Iterate through each network found
        for (int i = 0; i < n; ++i) {
            // Print network details in a formatted table
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4ld", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2ld", WiFi.channel(i));
            Serial.print(" | ");
            switch (WiFi.encryptionType(i)) {
                case WIFI_AUTH_OPEN:            Serial.print("open"); break;
                case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
                case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
                case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
                case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
                case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
                case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
                case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
                case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
                default:                        Serial.print("unknown");
            }
            Serial.println();

            // Limit display to the first 20 networks to fit the screen
            if (i < 20) {
                // Check if the SSID contains Chinese characters
                if (contains_chinese(WiFi.SSID(i).c_str())) {
                    chinese_num++; // Increment the counter for Chinese SSIDs
                    printf("Skipping SSID with Chinese characters: %s \r\n", WiFi.SSID(i).c_str());
                } else {
                    // Display the SSID on the screen
                    Paint_DrawString_EN(440, (i - chinese_num) * 24, WiFi.SSID(i).c_str(), &Font24, BLACK, WHITE);
                }
            }
            delay(10); // Add a short delay to avoid overwhelming the CPU
        }
    }
    Serial.println("");

    // Free memory allocated for the scan results
    WiFi.scanDelete();
}
