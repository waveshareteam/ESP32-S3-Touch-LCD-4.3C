/*****************************************************************************
 * | File        :   main.c
 * | Author      :   Waveshare team
 * | Function    :   Main function
 * | Info        :   
 *                    Create an Access Point (AP), and when devices connect, 
 *                    display their MAC addresses on the screen.
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

#define USER_SSID "ESP32-S3-Touch-LCD-4.3C"  // Wi-Fi SSID (network name) for the Access Point
#define USER_PASS "66668888"  // Wi-Fi password for the Access Point

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
    Paint_DrawString_EN(10, 200, "WiFi AP Test", &Font24, RED, WHITE);          // Display Wi-Fi test message
    Paint_DrawString_EN(10, 240, "800s480", &Font24, RED, WHITE);                // Display screen resolution
    Paint_DrawString_EN(430, 160, "Connected: 0", &Font24, BLACK, WHITE);        // Display initial connection status
    Paint_DrawLine(400, 0, 400, 480, BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);    // Draw a vertical line on the display
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display with the updated image (BlackImage is the framebuffer)
    
    // Initialize SoftAP (Wi-Fi Access Point) with SSID, password, and channel
    wifi_ap_init((uint8_t *)USER_SSID, (uint8_t *)USER_PASS, 1);

    static uint8_t connection_num = 0;  // Variable to track the number of connected stations
    while (1)
    {
        esp_err_t ret = esp_wifi_ap_get_sta_list(&sta_list);  // Get the list of connected stations (devices)
        if (ret == ESP_OK)
        {
            // If the number of connected stations has changed, update the UI
            if (connection_num != sta_list.num)
            {
                char station_num[32];
                char station_mac[32];  // Buffer to hold formatted MAC address string
                

                Paint_ClearWindows(430, 160, 800, 480, WHITE);  // Clear the section of the screen displaying the connection info
                
                snprintf(station_num, sizeof(station_num), "Connected: %d", sta_list.num); // Format the number of connected stations
                Paint_DrawString_EN(430, 160, station_num, &Font24, BLACK, WHITE);  // Display the number of connected devices
                if (sta_list.num == 0)
                {
                    ESP_LOGE(TAG_AP, "No device connected.");  // Log error if no devices are connected
                }
                else
                {
                    for (int i = 0; i < sta_list.num; i++)
                    {
                        wifi_sta_info_t sta_info = sta_list.sta[i];  // Get station info (MAC address, RSSI, etc.)

                        // Format the MAC address and display it in the list
                        snprintf(station_mac, sizeof(station_mac), MACSTR, MAC2STR(sta_info.mac));
                        Paint_DrawString_EN(430, 200 , "MAC:", &Font24, BLACK, WHITE);  // Label for MAC address
                        Paint_DrawString_EN(430, 240 + (i * 40), station_mac, &Font24, BLACK, WHITE);  // Display MAC address

                        // Log information about the connected stations
                        ESP_LOGI(TAG_AP, "STA %d: MAC Address: " MACSTR, i, MAC2STR(sta_info.mac));  // Log MAC address
                        ESP_LOGI(TAG_AP, "STA %d: RSSI: %d", i, sta_info.rssi);  // Log signal strength (RSSI)
                    }
                }
                // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
                wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display again to show the updated image
                connection_num = sta_list.num;  // Update the connection number variable
            }
        }
        else
        {
            ESP_LOGE(TAG_AP, "Failed to get STA list");  // Log error if failed to get list of connected stations
        }
        // Delay for 10ms before the next loop iteration
        vTaskDelay(100);  // Short delay to avoid overloading the CPU
    }
}
