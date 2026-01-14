/*
* Create an Access Point (AP), and when devices connect, 
* display their MAC addresses on the screen.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"              // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"       // Header for graphical drawing functions
#include "src/user_wifi/user_wifi.h"       // Header for Wi-Fi initialization and scanning functions

#define USER_SSID "ESP32-S3-Touch-LCD-4.3C"  // Wi-Fi SSID (network name)
#define USER_PASS "66668888"  // Wi-Fi password

void setup() {
    Serial.begin(115200);  // Initialize the serial port for debugging

    // Check if PSRAM is available and functional
    if (psramFound()) {
        Serial.println("PSRAM is enabled and found!");
    } else {
        Serial.println("PSRAM not found or not enabled.");
    }
    delay(100);  // Short delay to allow initialization processes

    // Initialize I2C and the IO EXTENSION GPIO hardware interface
    DEV_I2C_Init();  // Start I2C communication
    IO_EXTENSION_Init();   // Configure the IO EXTENSION GPIO control chip
    
    // Initialize the RGB LCD display
    waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight for visibility
    wavesahre_rgb_lcd_bl_on();
    // Uncomment the next line to turn off the backlight if needed
    // wavesahre_rgb_lcd_bl_off();

    // Allocate memory for the framebuffer (image buffer) in PSRAM
    UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2;  // Each pixel takes 2 bytes in RGB565 format
    UBYTE *BlackImage;
    if ((BlackImage = (UBYTE *)heap_caps_malloc(Imagesize, MALLOC_CAP_SPIRAM)) == NULL) {
        Serial.println("Failed to allocate memory for the framebuffer.");
        exit(0);  // Exit the program if memory allocation fails
    }

    // Initialize a new image canvas and set its background color
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

    // Configure the canvas properties (scale and rotation)
    Paint_SetScale(65);       // Set the image scale
    Paint_Clear(WHITE);       // Clear the canvas and fill it with a white background

    // Display static information on the screen
    Paint_DrawString_EN(10, 160, "ESP32-S3-Touch-LCD-4.3C", &Font24, RED, WHITE);  // Display title
    Paint_DrawString_EN(10, 200, "WiFi AP Test", &Font24, RED, WHITE);       // Display Wi-Fi scan test message
    Paint_DrawString_EN(10, 240, "800x480", &Font24, RED, WHITE);              // Display resolution
    Paint_DrawLine(400, 0, 400, 480, BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);   // Draw a vertical separator line
    Paint_DrawString_EN(430, 160, "Connected: 0", &Font24, BLACK, WHITE);        // Display initial connection status
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display to show the initial static messages

    // Initialize Wi-Fi functionality and attempt to connect to the provided network
    wifi_ap_init(USER_SSID, USER_PASS); 
    
    // Update the display with the new Wi-Fi scan results
    wavesahre_rgb_lcd_display(BlackImage);
    static uint8_t connection_num = 0;
    while (1) 
    {
      char mac[32];
      uint8_t n = wifi_ap_StationNum();
      if (connection_num != n) 
      {
        char station_num[32];
        char station_mac[MAC_Addr_length];

        Paint_ClearWindows(430, 160, 800, 480, WHITE);  // Clear the section of the screen displaying the connection info
        snprintf(station_num, sizeof(station_num), "Connected: %d", n); // Format the number of connected stations
        Paint_DrawString_EN(430, 160, station_num, &Font24, BLACK, WHITE);  // Display the number of connected devices

        if (n == 0)
        {
            Serial.println("No device connected.");  // Log error if no devices are connected
        }
        else 
        {
            Paint_DrawString_EN(430, 200 , "MAC:", &Font24, BLACK, WHITE);  // Label for MAC address
            for (int i = 0; i < n; i++) {
              wifi_ap_StationMac(station_mac, i);
              Paint_DrawString_EN(430, 240 + (i * 40), station_mac, &Font24, BLACK, WHITE);  // Display MAC address
              Serial.print("MAC Address: ");
              Serial.println(station_mac);
            }
            Serial.println("");
        }
        // Update the display with the new Wi-Fi scan results
        wavesahre_rgb_lcd_display(BlackImage);
        connection_num = n;
      }
      delay(100);
    }
}

void loop() {
    // Main loop remains empty for now
    // You can add additional functionality or updates to run continuously here
    
}
