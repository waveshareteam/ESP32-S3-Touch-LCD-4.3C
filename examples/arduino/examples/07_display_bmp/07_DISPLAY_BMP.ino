/*
* Read BMP files from the SD card and display on the screen.
* Use the touchscreen to switch between images.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"              // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"       // Header for graphical drawing functions
#include "src/gt911/gt911.h"           // Header for touch screen operations (GT911)
#include "src/rgb_lcd_port/gui_paint/gui_bmp.h"         // Header for BMP image handling
#include "src/user_sd/user_sd.h"           // 

#define MAX_BMP_FILES 50  // Maximum number of BMP files supported
static char *BmpPath[MAX_BMP_FILES]; // Array to store full paths of BMP files
static uint8_t bmp_num; // Counter for the number of BMP files found

/**
 * Function to list all BMP files in a given directory.
 * 
 * @param base_path The base directory to search for BMP files.
 */
void list_files(const char *base_path) {
    // Open the specified directory
    File dir = SD_MMC.open(base_path); 
    if (!dir) {
        Serial.println("Failed to open directory!"); // Print an error if the directory can't be opened
        return;
    }

    // Check if the provided path is a valid directory
    if (!dir.isDirectory()) {
        Serial.println("Provided path is not a directory!"); // Print an error if the path isn't a directory
        dir.close();
        return;
    }

    int i = 0; // Initialize file counter
    static char mount_point[] = MOUNT_POINT; // Mount point for the SD card
    File file = dir.openNextFile(); // Open the first file in the directory

    // Iterate through the directory and process files
    while (file && i < MAX_BMP_FILES) {
        if (!file.isDirectory()) { // Skip directories
            const char *file_name = file.name(); // Get the name of the current file
            size_t len = strlen(file_name);

            // Check if the file extension is ".bmp" (case insensitive)
            if (len > 4 && strcasecmp(&file_name[len - 4], ".bmp") == 0) {
                size_t length = strlen(mount_point) + strlen(file_name) + 2; // Calculate memory needed for full path
                BmpPath[i] = (char *)malloc(length); // Allocate memory for the full path
                if (BmpPath[i]) {
                    // Combine mount point and file name into the full path
                    snprintf(BmpPath[i], length, "%s/%s", mount_point, file_name); 
                    i++; // Increment the file counter
                } else {
                    Serial.println("Memory allocation failed for BMP path!"); // Print an error if memory allocation fails
                }
            }
        }
        file.close(); // Close the current file
        file = dir.openNextFile(); // Open the next file in the directory
    }

    bmp_num = i; // Set the number of BMP files found
    Serial.print("Found BMP files: "); 
    Serial.println(bmp_num); // Print the number of BMP files found

    dir.close(); // Close the directory
}


void setup() {
  touch_gt911_point_t point_data;  // Structure to store touch point data
  
  Serial.begin(115200);
  DEV_I2C_Init();
  IO_EXTENSION_Init();
  // Initialize the GT911 touch screen controller
  touch_gt911_init(DEV_I2C_Get_Bus_Device());

  // Initialize the Waveshare ESP32-S3 RGB LCD
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
        
        // Draw navigation arrows
        Paint_DrawLine(540, 450, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(575, 435, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); // Right arrow
        Paint_DrawLine(575, 465, 600, 450, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID); 
        
        // List BMP files from the SD card
        list_files("/");
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
        delay(30);  // Delay for 30ms to avoid high CPU usage
    }
}

void loop() {
  // put your main code here, to run repeatedly:
    
}
