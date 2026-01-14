/*
* Performs a five-point touch test and demonstrates basic usage of double buffering.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"    // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"       // Header for graphical drawing functions
#include "src/gt911/gt911.h"           // Header for touch screen operations (GT911)

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
  // Frame buffer pointers for double buffering
  void *buf1 = NULL;
  void *buf2 = NULL;

  // Retrieve pointers to the frame buffers
  waveshare_get_frame_buffer(&buf1, &buf2);
  if (buf1 == NULL || buf2 == NULL) {
      printf("Error: buf1 and buf2 are NULL!\n");
      return;
  }

  // Initialize the graphics canvas with buf2
  Paint_NewImage((uint8_t *)buf2, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

  // Set the scale for the graphical canvas
  Paint_SetScale(65);

  // Clear the canvas and fill it with a white background
  Paint_Clear(WHITE);

  // Copy buf2 content to buf1 to sync buffers
  memcpy(buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2);

  // Display the initial blank screen on the LCD
  wavesahre_rgb_lcd_display((uint8_t *)buf1);
  
  // Arrays to store previous touch point positions and their active states
  static uint16_t prev_x[ESP_LCD_TOUCH_MAX_POINTS];
  static uint16_t prev_y[ESP_LCD_TOUCH_MAX_POINTS];
  static bool active[ESP_LCD_TOUCH_MAX_POINTS];  // Track if a touch point is active
  static uint16_t color[ESP_LCD_TOUCH_MAX_POINTS] = {
      0x7DDF, 0xFBE4, 0x7FE0, 0xEC1D, 0xFEE0
  }; // Predefined colors for touch points

  // Flag to toggle between the two buffers
  bool flag = true;
  
  // Main application loop
  while (1)
  {
      // Read the touch points from the touchscreen controller
      point_data = touch_gt911_read_point(ESP_LCD_TOUCH_MAX_POINTS);

      // Process each touch point
      for (int i = 0; i < ESP_LCD_TOUCH_MAX_POINTS; i++)
      {     
          if (i < point_data.cnt) // If a valid touch point exists
          {
              if (prev_x[i] != 0 && prev_y[i] != 0)
              {
                  // Clear the previously drawn circle for this touch point
                  Paint_DrawCircle(prev_x[i], prev_y[i], 30, WHITE, 
                                    DOT_PIXEL_1X1, DRAW_FILL_FULL);
              }

              // Update the previous touch point position
              prev_x[i] = point_data.x[i];
              prev_y[i] = point_data.y[i];
              active[i] = true;  // Mark the point as active
              
              // Log the touch point coordinates
              printf("Touch position %d: %d,%d %d", i, 
                        point_data.x[i], point_data.y[i], point_data.cnt);
              
              // Draw a circle at the new touch point with a unique color
              Paint_DrawCircle(point_data.x[i], point_data.y[i], 30, color[i], 
                                DOT_PIXEL_1X1, DRAW_FILL_FULL);
          }
          else
          {
              // If no longer active, clear the circle for this touch point
              if (active[i]) {
                  Paint_DrawCircle(prev_x[i], prev_y[i], 30, WHITE, 
                                    DOT_PIXEL_1X1, DRAW_FILL_FULL);
                  active[i] = false;  // Mark the point as inactive
              }
          }     
      }

      // Display the updated buffer
      if (flag)
      {
          // Small delay to reduce screen tearing
          vTaskDelay(20);

          // Display the content from buf2
          wavesahre_rgb_lcd_display((uint8_t *)buf2);

          // Sync buf2 content to buf1
          memcpy(buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2);

          // Select buf1 as the active drawing buffer
          Paint_SelectImage((uint8_t *)buf1);
          flag = false;
      }
      else
      {
          // Small delay to reduce screen tearing
          vTaskDelay(20);

          // Display the content from buf1
          wavesahre_rgb_lcd_display((uint8_t *)buf1);

          // Sync buf1 content to buf2
          memcpy(buf2, buf1, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2);

          // Select buf2 as the active drawing buffer
          Paint_SelectImage((uint8_t *)buf2);
          flag = true;
      }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
