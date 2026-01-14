/*****************************************************************************
 * | File        :   main.c
 * | Author      :   Waveshare team
 * | Function    :   Main function
 * | Info        :   
 *                    This example scans for nearby Wi-Fi signals and displays
 *                    the Wi-Fi names (SSIDs) on the right side of the LCD screen.
 *----------------
 * | This version :   V1.0
 * | Date         :   2024-12-24
 * | Info         :   Basic version
 *
 ******************************************************************************/ 

#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"

#include "rgb_lcd_port.h"    // Header for Waveshare RGB LCD driver
#include "gui_paint.h"       // Header for graphical drawing functions
#include "image.h"           // Header for image resources
#include "wifi.h"

#define ROTATE ROTATE_0  // Set screen rotation: options are 0, 90, 180, 270 degrees

// Function to check if SSID contains Chinese characters
bool contains_chinese(const char *ssid) {
    while (*ssid) {
        if ((*ssid & 0x80) != 0) {  // If the highest bit is 1, it indicates a non-ASCII character (possibly Chinese)
            return true;
        }
        ssid++;
    }
    return false;
}

void app_main()
{
    // Initialize the Non-Volatile Storage (NVS) for Wi-Fi settings
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // If NVS has no free pages or a new version is found, erase and reinitialize NVS
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    // Initialize I2C communication and IO EXTENSION hardware interface for GPIO control
    DEV_I2C_Init();  // Initialize I2C communication
    IO_EXTENSION_Init();   // Initialize GPIO control using the IO EXTENSION chip
    
    // Initialize the Waveshare ESP32-S3 RGB LCD display
    waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();         
    // Uncomment the next line to turn off the backlight if needed
    // wavesahre_rgb_lcd_bl_off();

    // Allocate memory for the screen's framebuffer (image buffer)
    UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2;  // Each pixel takes 2 bytes in RGB565 format
    UBYTE *BlackImage;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {  // Allocate memory for the framebuffer
        printf("Failed to apply for black memory...\r\n");
        exit(0);  // Exit the program if memory allocation fails
    }

    // Create a new image canvas and set its background color to white
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

    // Set the canvas scale and rotation for the display
    Paint_SetScale(65);  // Set the scale for the image
    Paint_SetRotate(ROTATE);  // Set the rotation (0 degrees)
    
    // Clear the canvas and fill it with a white background
    Paint_Clear(WHITE);
    
    // Initialize Wi-Fi settings
    wifi_init();  
    uint8_t chinese_num = 0; // Counter for Wi-Fi networks with Chinese SSIDs

    // Display static information on the LCD screen
    Paint_DrawString_EN(10, 160, "ESP32-S3-Touch-LCD-4.3C", &Font24, RED, WHITE);  // Display title
    Paint_DrawString_EN(10, 200, "WiFi SCAN Test", &Font24, RED, WHITE);          // Display Wi-Fi scan message
    Paint_DrawString_EN(10, 240, "800x480", &Font24, RED, WHITE);                // Display screen resolution
    Paint_DrawLine(400, 0, 400, 480, BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);  // Draw a vertical line to separate sections
    Paint_DrawString_EN(440, 0, "Scanning now...", &Font24, BLACK, WHITE); // Show scanning status message
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display with the updated image
    
    // Clear the top section of the screen to display scanning results
    Paint_ClearWindows(440, 0, 800, 25, WHITE);
    
    // Start Wi-Fi scanning to find available networks
    wifi_scan();

    // Loop through the Wi-Fi APs found and display them on the screen
    for (int i = 0; i < DEFAULT_SCAN_LIST_SIZE; i++) {
        // Skip SSID with Chinese characters
        if (contains_chinese((const char *)ap_info[i].ssid)){
            chinese_num++;  // Increment the count for networks with Chinese SSIDs
            printf("Skipping SSID with Chinese characters: %s \r\n", ap_info[i].ssid);
        }
        else {
            // Display the SSID (Wi-Fi network name) on the screen
            Paint_DrawString_EN(440, (i - chinese_num) * 24, (const char *)ap_info[i].ssid, &Font24, BLACK, WHITE);
        }
    }

    // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display to show the updated list of networks
}
