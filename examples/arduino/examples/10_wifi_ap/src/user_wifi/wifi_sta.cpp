#include "user_wifi.h"
#include "../rgb_lcd_port/gui_paint/gui_paint.h"

// Function to initialize WiFi in station mode
void wifi_sta_init(const char *ssid, const char *pwd) {

    // Start by attempting to connect to a WiFi network
    Serial.println();
    Serial.println("******************************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.println(pwd);

    WiFi.begin(ssid, pwd);  // Begin the WiFi connection
    uint8_t s_retry_num = 0;  // Retry counter

    // Wait until WiFi is connected or retry limit is reached
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);  // Wait for 500ms before retrying
      Serial.print(".");  // Display a dot for each retry attempt
      s_retry_num++;
      
      // If retry count exceeds 30, stop attempting and indicate failure
      if(s_retry_num > 30)
      {
        s_retry_num = 0;  // Reset retry counter
        Serial.println("");
        Serial.println("WiFi connection Failed.");

        // Clear the display and show the failure message
        Paint_ClearWindows(440, 160, 800, 185, WHITE);
        Paint_DrawString_EN(440, 160, "Failed to connect to the AP.", &Font24, BLACK, WHITE);        
        return;  // Exit the function
      }
    }

    // If WiFi is connected, print the connection status and IP address
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  // Print the IP address to serial monitor

    char ip[20];  // Buffer to store the formatted IP address
    String ipStr = WiFi.localIP().toString();  // Convert IP address to a string
    snprintf(ip, sizeof(ip), "IP %s", ipStr.c_str());  // Format the IP address string

    // Clear the display window and show the formatted IP address on the screen
    Paint_ClearWindows(440, 160, 800, 185, WHITE);
    Paint_DrawString_EN(440, 160, ip, &Font24, BLACK, WHITE);  // Display IP on the screen
    Paint_DrawString_EN(440, 200, ssid, &Font24, BLACK, WHITE); // Display the SSID of the connected WiFi network
    Paint_DrawString_EN(440, 240, pwd, &Font24, BLACK, WHITE); // Display the WiFi password (may be hidden in production)
}
