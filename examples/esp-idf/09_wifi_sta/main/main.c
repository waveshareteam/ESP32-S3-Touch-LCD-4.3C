/*****************************************************************************
 * | File        :   main.c
 * | Author      :   Waveshare team
 * | Function    :   Main function
 * | Info        :   
 *                    This example connects to an AP. Once connected, it displays 
 *                    the IP address, Wi-Fi name, and password. If the connection 
 *                    fails, it shows a connection failure message.
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

#define USER_SSID "SSID"  // Wi-Fi SSID (network name)
#define USER_PASS "PASSWORD"  // Wi-Fi password

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
    DEV_I2C_Init();  // Initialize I2C
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
    
    // Initialize Wi-Fi settings (connect to the specified Wi-Fi network)
    wifi_init();  

    // Display some information on the LCD screen
    Paint_DrawString_EN(10, 160, "ESP32-S3-Touch-LCD-4.3C", &Font24, RED, WHITE);  // Display title
    Paint_DrawString_EN(10, 200, "WiFi STA Test", &Font24, RED, WHITE);          // Display Wi-Fi test message
    Paint_DrawString_EN(10, 240, "800x480", &Font24, RED, WHITE);                // Display screen resolution
    Paint_DrawString_EN(440, 160, "wifi connecting......", &Font24, BLACK, WHITE); // Display Wi-Fi connection status
    Paint_DrawLine(400, 0, 400, 480, BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);  // Draw a vertical line on the display

    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display with the updated image (BlackImage is the framebuffer)
    
    // Initialize Wi-Fi in STA mode and attempt to connect to the specified SSID and password
    wifi_sta_init((uint8_t *)USER_SSID, (uint8_t *)USER_PASS, WIFI_AUTH_WPA2_PSK);

    // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display again to show the updated image
}
