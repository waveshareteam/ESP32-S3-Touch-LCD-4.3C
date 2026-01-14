/*
* The LCD parameters and GPIO number used by this example can be changed in 
* [rgb_lcd_port.h](components/rgb_lcd_port.h). Especially, please pay attention 
* to the **vendor specific initialization**, it can be different between 
* manufacturers and should consult the LCD supplier for initialization sequence 
* code.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"          // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"   // Header for graphical drawing functions
#include "src/rgb_lcd_port/image/image.h"           // Header for image resources

#define ROTATE ROTATE_0//rotate = 0, 90, 180, 270

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  DEV_I2C_Init();
  IO_EXTENSION_Init();
  // Initialize the Waveshare ESP32-S3 RGB LCD
  waveshare_esp32_s3_rgb_lcd_init(); 
  // Turn on the LCD backlight
  wavesahre_rgb_lcd_bl_on();  
  UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2; // Each pixel takes 2 bytes in RGB565
  UBYTE *BlackImage;
  if ((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) // Allocate memory
  {
      printf("Failed to apply for black memory...\r\n");
      exit(0); // Exit the program if memory allocation fails
  }
  // Create a new image canvas and set its background color to white
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

    // Set the canvas scale
    Paint_SetScale(65);
    Paint_SetRotate(ROTATE);
    // Clear the canvas and fill it with a white background
    Paint_Clear(WHITE);
    if (ROTATE == ROTATE_0 || ROTATE == ROTATE_180)
    {
        // Draw gradient color stripes using RGB565 format
        // From red to blue
        Paint_DrawRectangle(1, 1, 50, 480, 0xF800, DOT_PIXEL_1X1, DRAW_FILL_FULL);  
        Paint_DrawRectangle(51, 1, 100, 480, 0x7800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(101, 1, 150, 480, 0x3800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(151, 1, 200, 480, 0x1800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(201, 1, 250, 480, 0x0800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 


        Paint_DrawRectangle(251, 1, 300, 480, 0x07E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(301, 1, 350, 480, 0x03E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(351, 1, 400, 480, 0x01E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(401, 1, 450, 480, 0x00E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(451, 1, 500, 480, 0x0060, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(501, 1, 550, 480, 0x0020, DOT_PIXEL_1X1, DRAW_FILL_FULL); 

        Paint_DrawRectangle(551, 1, 600, 480, 0x001F, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(601, 1, 650, 480, 0x000F, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(651, 1, 700, 480, 0x0007, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(701, 1, 750, 480, 0x0003, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(751, 1, 800, 480, 0x0001, DOT_PIXEL_1X1, DRAW_FILL_FULL); 

        // Display the gradient on the screen
        wavesahre_rgb_lcd_display(BlackImage);
        vTaskDelay(1000);

        Paint_Clear(WHITE);
        // Draw points with increasing sizes
        Paint_DrawPoint(2, 18, BLACK, DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 20, BLACK, DOT_PIXEL_2X2, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 23, BLACK, DOT_PIXEL_3X3, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 28, BLACK, DOT_PIXEL_4X4, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 33, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);

        // Draw solid and dotted lines
        Paint_DrawLine(20, 5, 80, 65, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(20, 65, 80, 5, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(148, 35, 208, 35, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
        Paint_DrawLine(178, 5, 178, 65, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

        // Draw rectangles (empty and filled)
        Paint_DrawRectangle(20, 5, 80, 65, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
        Paint_DrawRectangle(85, 5, 145, 65, BLUE, DOT_PIXEL_2X2, DRAW_FILL_FULL);

        // Draw circles (empty and filled)
        Paint_DrawCircle(178, 35, 30, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawCircle(240, 35, 30, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw text and numbers
        Paint_DrawString_EN(1, 70, "AaBbCc123", &Font16, RED, WHITE);
        Paint_DrawNum(1, 100, 9.87654321, &Font20, 7, WHITE, BLACK);
        Paint_DrawString_EN(1, 130, "AaBbCc123", &Font20, 0x000f, 0xfff0);
        Paint_DrawString_EN(1, 160, "AaBbCc123", &Font24, RED, WHITE);
        Paint_DrawString_CN(1, 190, "你好Abc", &Font24CN, WHITE, BLUE);

        // Update the display with the newly drawn elements (currently displaying BlackImage)
        wavesahre_rgb_lcd_display(BlackImage);
        vTaskDelay(1000); // Delay for 1000ms to allow the screen to update

        // Draw a bitmap at coordinates (0,0) with size 800x480 using the provided gImage_Bitmap
        Paint_BmpWindows(0, 0, gImage_Bitmap, 800, 480);

        // Update the screen with the updated image (BlackImage is the framebuffer being drawn to)
        wavesahre_rgb_lcd_display(BlackImage); // Refresh the display with the new content
        vTaskDelay(1000); // Delay for 1000ms to allow the screen to update

        // Draw an image resource gImage_picture at coordinates (0,0) with size 800x480
        Paint_DrawImage(gImage_picture, 0, 0, 800, 480);

        // Optionally, you can also call this to draw the bitmap, though it��s commented out here:
        // Paint_DrawBitMap(gImage_picture);

        // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
        wavesahre_rgb_lcd_display(BlackImage); // Refresh the display to show the updated image

    }
    else
    {
        // Draw gradient color stripes using RGB565 format
        // From red to blue
        Paint_DrawRectangle(1, 1, 480, 50, 0xF800, DOT_PIXEL_1X1, DRAW_FILL_FULL);  
        Paint_DrawRectangle(1, 51,  480, 100, 0x7800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 101, 480, 150, 0x3800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 151, 480, 200, 0x1800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 201, 480, 250, 0x0800, DOT_PIXEL_1X1, DRAW_FILL_FULL); 


        Paint_DrawRectangle(1, 251, 480, 300, 0x07E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 301, 480, 350, 0x03E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 351, 480, 400, 0x01E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 401, 480, 450, 0x00E0, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 451, 480, 500, 0x0060, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 501, 480, 550, 0x0020, DOT_PIXEL_1X1, DRAW_FILL_FULL); 

        Paint_DrawRectangle(1, 550, 480, 600, 0x001F, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 600, 480, 650, 0x000F, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 650, 480, 700, 0x0007, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 700, 480, 750, 0x0003, DOT_PIXEL_1X1, DRAW_FILL_FULL); 
        Paint_DrawRectangle(1, 750, 480, 800, 0x0001, DOT_PIXEL_1X1, DRAW_FILL_FULL); 

        // Display the gradient on the screen
        wavesahre_rgb_lcd_display(BlackImage);
        vTaskDelay(1000);

        Paint_Clear(WHITE);
        // Draw points with increasing sizes
        Paint_DrawPoint(2, 18, BLACK, DOT_PIXEL_1X1, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 20, BLACK, DOT_PIXEL_2X2, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 23, BLACK, DOT_PIXEL_3X3, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 28, BLACK, DOT_PIXEL_4X4, DOT_FILL_RIGHTUP);
        Paint_DrawPoint(2, 33, BLACK, DOT_PIXEL_5X5, DOT_FILL_RIGHTUP);

        // Draw solid and dotted lines
        Paint_DrawLine(20, 5, 80, 65, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(20, 65, 80, 5, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(148, 35, 208, 35, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
        Paint_DrawLine(178, 5, 178, 65, CYAN, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);

        // Draw rectangles (empty and filled)
        Paint_DrawRectangle(20, 5, 80, 65, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
        Paint_DrawRectangle(85, 5, 145, 65, BLUE, DOT_PIXEL_2X2, DRAW_FILL_FULL);

        // Draw circles (empty and filled)
        Paint_DrawCircle(178, 35, 30, GREEN, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
        Paint_DrawCircle(240, 35, 30, GREEN, DOT_PIXEL_1X1, DRAW_FILL_FULL);

        // Draw text and numbers
        Paint_DrawString_EN(1, 70, "AaBbCc123", &Font16, RED, WHITE);
        Paint_DrawNum(1, 100, 9.87654321, &Font20, 7, WHITE, BLACK);
        Paint_DrawString_EN(1, 130, "AaBbCc123", &Font20, 0x000f, 0xfff0);
        Paint_DrawString_EN(1, 160, "AaBbCc123", &Font24, RED, WHITE);
        Paint_DrawString_CN(1, 190, "你好Abc", &Font24CN, WHITE, BLUE);

        // Update the display with the newly drawn elements (currently displaying BlackImage)
        wavesahre_rgb_lcd_display(BlackImage);
        vTaskDelay(1000); // Delay for 1000ms to allow the screen to update

        // Draw a bitmap at coordinates (0,0) with size 800x480 using the provided gImage_Bitmap
        Paint_BmpWindows(0, 0, gImage_Bitmap_90, 480, 800);

        // Update the screen with the updated image (BlackImage is the framebuffer being drawn to)
        wavesahre_rgb_lcd_display(BlackImage); // Refresh the display with the new content
        vTaskDelay(1000); // Delay for 1000ms to allow the screen to update

        // Draw an image resource gImage_picture at coordinates (0,0) with size 800x480
        Paint_DrawImage(gImage_picture_90, 0, 0, 480, 800);

        // Optionally, you can also call this to draw the bitmap, though it commented out here:
        // Paint_DrawBitMap(gImage_picture);

        // Update the screen with the new image (BlackImage is the framebuffer being drawn to)
        wavesahre_rgb_lcd_display(BlackImage); // Refresh the display to show the updated image

    
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
