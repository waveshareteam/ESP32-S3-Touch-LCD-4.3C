/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   Main function
 * | Info       :   
 *                  Mount an SD card, output related parameters via serial, 
 *                  unmount the SD card, and display memory information on the screen.
 *----------------
 * | Version    :   V1.0
 * | Date       :   2024-11-19
 * | Info       :   Basic version
 *
 ******************************************************************************/
#include "rgb_lcd_port.h"    // Header for Waveshare RGB LCD driver
#include "gui_paint.h"       // Header for graphical drawing functions
#include "image.h"           // Header for image resources
#include "sd.h"              // Header for SD card operations

void app_main()
{
    // Initialize I2C communication and configure the IO EXTENSION GPIO expander
    DEV_I2C_Init();                // Initialize the I2C bus
    IO_EXTENSION_Init();                     // Initialize IO EXTENSION chip

    IO_EXTENSION_Output(IO_EXTENSION_IO_4, 1);  // Set CS (chip select) pin high
  
    // Initialize the Waveshare ESP32-S3 RGB LCD
    waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();         // Turn on the LCD backlight
    // wavesahre_rgb_lcd_bl_off();     // Uncomment to turn off the backlight if needed

    // Allocate memory for the LCD's frame buffer
    UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2; // Each pixel uses 2 bytes in RGB565 format
    UBYTE *BlackImage;
    if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) // Allocate memory for the image
    {
        printf("Failed to allocate memory for frame buffer...\r\n");
        exit(0); // Exit if memory allocation fails
    }

    // Create a new image canvas and set its background color to white
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);
    Paint_SetScale(65);                // Set the canvas scale
    Paint_Clear(BLACK);                // Clear the canvas with a black background

    // Draw static text on the canvas
    Paint_DrawString_EN(150, 130, "SD TEST", &Font48, CYAN, BLACK);
    Paint_DrawString_EN(150, 180, "Waveshare ESP32 S3", &Font24, CYAN, BLACK);
    Paint_DrawString_EN(150, 210, "https://www.waveshare.com", &Font24, CYAN, BLACK);

    char Total[100], Available[100]; // Buffers for formatted text
    if (sd_mmc_init() == ESP_OK) // Initialize SD card
    {
        Paint_DrawString_EN(150, 240, "SD Card OK!", &Font24, CYAN, BLACK); // Indicate SD card success

        sd_card_print_info(); // Print SD card information to serial
        size_t total, available; // Variables to store total and available space
        read_sd_capacity(&total, &available); // Read SD card capacity
        printf("Total:%d MB,Available:%d MB\r\n", (int)total / 1024, (int)available / 1024);

        // Format total space into a human-readable string
        if (((int)total / 1024) > 1024)
            sprintf(Total, "Total:%d GB", (int)total / 1024 / 1024);
        else
            sprintf(Total, "Total:%d MB", (int)total / 1024);

        // Format available space into a human-readable string
        if (((int)available / 1024) > 1024)
            sprintf(Available, "Available:%d GB", (int)available / 1024 / 1024);
        else
            sprintf(Available, "Available:%d MB", (int)available / 1024);

        // Draw the total and available space information on the canvas
        Paint_DrawString_EN(150, 270, Total, &Font24, CYAN, BLACK);
        Paint_DrawString_EN(150, 300, Available, &Font48, CYAN, BLACK);

        printf("Filesystem unmount\r\n");
        ESP_ERROR_CHECK(sd_mmc_unmount()); // Unmount the SD card
    }
    else
        Paint_DrawString_EN(150, 240, "SD Card Fail!", &Font24, CYAN, BLACK);

    // Display the prepared canvas on the LCD
    wavesahre_rgb_lcd_display(BlackImage);
}
