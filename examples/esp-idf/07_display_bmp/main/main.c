/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   Main function
 * | Info       :   Read BMP files from the SD card and display on the screen.
 * |                Use the touchscreen to switch between images.
 *----------------
 * | Version    :   V1.0
 * | Date       :   2024-12-06
 * | Info       :   Basic version
 *
 ******************************************************************************/


#include "rgb_lcd_port.h"    // Header for Waveshare RGB LCD driver
#include "gui_paint.h"       // Header for graphical drawing functions
#include "gui_bmp.h"         // Header for BMP image handling
#include "gt911.h"           // Header for touch screen operations (GT911)
#include "sd.h"              // Header for SD card operations

#include <dirent.h>          // Header for directory operations

static char *BmpPath[256];        // Array to store BMP file paths
static uint8_t bmp_num;           // Number of BMP files found

// Function to list all BMP files in a directory
void list_files(const char *base_path) {
    int i = 0;
    DIR *dir = opendir(base_path);  // Open directory
    if (dir) {
        struct dirent *entry;
        // Loop through all files in the directory
        while ((entry = readdir(dir)) != NULL) {
            const char *file_name = entry->d_name;
            // Check if the file ends with ".bmp"
            size_t len = strlen(file_name);
            if (len > 4 && strcasecmp(&file_name[len - 4], ".bmp") == 0) {
                size_t length = strlen(base_path) + strlen(file_name) + 2; // 1 for '/' and 1 for '\0'
                BmpPath[i] = malloc(length);  // Allocate memory for the BMP path
                snprintf(BmpPath[i], length, "%s/%s", base_path, file_name); // Store the full path
                i++;
            }
        }
        bmp_num = i; // Set the number of BMP files found
        closedir(dir); // Close the directory
    }
}

// Main application function
void app_main()
{
    touch_gt911_point_t point_data;  // Structure to store touch point data

    // Initialize the GT911 touch screen controller
    touch_gt911_init();  
    
    // Initialize the Waveshare ESP32-S3 RGB LCD hardware
    waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();         

    // EXAMPLE_PIN_NUM_TOUCH_INT
    // Allocate memory for the LCD's frame buffer
    UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2; // Each pixel uses 2 bytes in RGB565 format
    UBYTE *BlackImage;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) // Check if memory allocation is successful
    {
        printf("Failed to allocate memory for frame buffer...\r\n");
        exit(0); // Exit if memory allocation fails
    }

    // Initialize the graphics canvas with the allocated buffer
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

    // Set the scale for the graphical canvas
    Paint_SetScale(65);

    // Clear the canvas and fill it with a white background
    Paint_Clear(WHITE);

    // Initialize SD card
    if (sd_mmc_init() == ESP_OK) 
    {
        Paint_DrawString_EN(200, 200, "SD Card OK!", &Font24, BLACK, WHITE); // Display SD card success message
        Paint_DrawString_EN(200, 240, "Click the arrow to start.", &Font24, BLACK, WHITE); // Display prompt
        
        
        
        // List BMP files from the SD card
        list_files(MOUNT_POINT);
        if (bmp_num == 0)
        {
            Paint_DrawString_EN(200, 280, "There is no BMP file in the memory card.", &Font24, RED, WHITE); // Display prompt
            wavesahre_rgb_lcd_display(BlackImage);
            return;
        }
        else
        {
            // Draw navigation arrows
            Paint_DrawLine(540, 450, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
            Paint_DrawLine(575, 435, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); // Right arrow
            Paint_DrawLine(575, 465, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); 

            // Display the initial image on the screen
            wavesahre_rgb_lcd_display(BlackImage);
        }
        
    }
    else
    {
        // If SD card initialization fails
        Paint_DrawString_EN(200, 200, "SD Card Fail!", &Font24, BLACK, WHITE);
        wavesahre_rgb_lcd_display(BlackImage);
        return;
        
    }

    // Initial touch point variables
    int8_t i = 0;
    static uint16_t prev_x;
    static uint16_t prev_y;

    while (1)
    {
        point_data = touch_gt911_read_point(1);  // Read touch data
        if (point_data.cnt == 1)  // Check if touch is detected
        {
            // If touch position hasn't changed, continue the loop
            if ((prev_x == point_data.x[0]) && (prev_y == point_data.y[0]))
            {
                continue;
            }
            
            // If touch is within the left navigation area, switch to the previous image
            if (point_data.x[0] > 200 && point_data.x[0] < 260 && point_data.y[0] > 420 && point_data.y[0] < 480)
            {
                i--;
                if (i < 0)  // If index goes below 0, wrap around to the last image
                {
                    i = bmp_num - 1;
                }
                Paint_Clear(WHITE);  // Clear the screen
                GUI_ReadBmp(200, 123, BmpPath[i]);  // Read and display the previous BMP image

                // Draw navigation arrows
                Paint_DrawLine(200, 450, 260, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); // Left arrow
                Paint_DrawLine(200, 450, 225, 435, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(200, 450, 225, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

                // Right arrow
                Paint_DrawLine(540, 450, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(575, 435, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(575, 465, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

                wavesahre_rgb_lcd_display(BlackImage);  // Update display
                
                prev_x = point_data.x[0];  // Update previous touch position
                prev_y = point_data.y[0];
            }
            // If touch is within the right navigation area, switch to the next image
            else if (point_data.x[0] > 540 && point_data.x[0] < 600 && point_data.y[0] > 420 && point_data.y[0] < 480)
            {
                i++;
                if (i > bmp_num - 1)  // If index exceeds the number of images, wrap around to the first image
                {
                    i = 0;
                }
                Paint_Clear(WHITE);  // Clear the screen
                GUI_ReadBmp(200, 123, BmpPath[i]);  // Read and display the next BMP image

                // Draw navigation arrows
                Paint_DrawLine(200, 450, 260, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); // Left arrow
                Paint_DrawLine(200, 450, 225, 435, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(200, 450, 225, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

                // Right arrow
                Paint_DrawLine(540, 450, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(575, 435, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
                Paint_DrawLine(575, 465, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);

                wavesahre_rgb_lcd_display(BlackImage);  // Update display

                prev_x = point_data.x[0];  // Update previous touch position
                prev_y = point_data.y[0];
            }          
        }
        vTaskDelay(30);  // Delay for 30ms to avoid high CPU usage
    }
}
