/*
* This example scans for nearby Wi-Fi signals and displays the 
* Wi-Fi names (SSIDs) on the right side of the LCD screen.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"              // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"       // Header for graphical drawing functions
#include "src/user_wifi/user_wifi.h"       // Header for Wi-Fi initialization and scanning functions

#define ROTATE ROTATE_0  // Set screen rotation: options are 0, 90, 180, 270 degrees

void setup() {
    Serial.begin(115200);  // Initialize the serial port for debugging

    // Check if PSRAM is available and functional
    if (psramFound()) {
        Serial.println("PSRAM is enabled and found!");
    } else {
        Serial.println("PSRAM not found or not enabled.");
    }
    delay(100);  // Short delay to allow initialization processes

    // Initialize Wi-Fi functionality
    wifi_scan_init(); 

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
    Paint_SetRotate(ROTATE);  // Set the rotation angle (default 0 degrees)
    Paint_Clear(WHITE);       // Clear the canvas and fill it with a white background

    // Counter for Wi-Fi networks with Chinese SSIDs
    uint8_t chinese_num = 0;

    // Display static information on the screen
    Paint_DrawString_EN(10, 160, "ESP32-S3-Touch-LCD-4.3C", &Font24, RED, WHITE);  // Display title
    Paint_DrawString_EN(10, 200, "WiFi SCAN Test", &Font24, RED, WHITE);       // Display Wi-Fi scan test message
    Paint_DrawString_EN(10, 240, "800x480", &Font24, RED, WHITE);              // Display resolution
    Paint_DrawLine(400, 0, 400, 480, BLUE, DOT_PIXEL_2X2, LINE_STYLE_SOLID);   // Draw a vertical separator line
    Paint_DrawString_EN(440, 0, "Scanning now...", &Font24, BLACK, WHITE);     // Display scanning status
    wavesahre_rgb_lcd_display(BlackImage);  // Refresh the display to show the initial static messages

    // Clear the scanning message area
    Paint_ClearWindows(440, 0, 800, 25, WHITE);

    // Start Wi-Fi scanning and display the results
    wifi_scan();

    // Update the display with the new Wi-Fi scan results
    wavesahre_rgb_lcd_display(BlackImage);
}

void loop() {
    // Main loop remains empty for now
    // You can add additional functionality or updates to run continuously here
}
