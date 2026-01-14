/*
* Performs a five-point touch test and demonstrates basic usage of double buffering.
*/
#include "src/rgb_lcd_port/rgb_lcd_port.h"    // Header for Waveshare RGB LCD driver
#include "src/rgb_lcd_port/gui_paint/gui_paint.h"       // Header for graphical drawing functions
#include "src/gt911/gt911.h"           // Header for touch screen operations (GT911)
#include "src/user_sd/user_sd.h"           // SD card (not used directly here)
#include "src/speaker_microphone/codec_dev.h"    // Codec driver
#include "esp_check.h"    // Error handling macros

// Configuration macros
#define RECORD_TIME_SEC 5
#define BUFFER_SIZE (CODEC_DEFAULT_SAMPLE_RATE * RECORD_TIME_SEC * CODEC_DEFAULT_CHANNEL * (CODEC_DEFAULT_BIT_WIDTH / 8))

static int16_t *record_buffer = NULL;
UBYTE *BlackImage;

// Function to handle recording and playback
void play_or_pause(bool play)
{
    if (play)
    {
        // Recording logic
        Paint_DrawLine(390, 435, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(410, 435, 410, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawString_EN(200, 150, "Start recording...", &Font48, BLACK, WHITE);
        wavesahre_rgb_lcd_display(BlackImage);
        
        size_t total_bytes = 0;
        while (total_bytes < BUFFER_SIZE)
        {
            size_t bytes_read = 0;
            mic_i2s_read(record_buffer + total_bytes / 2,
                         BUFFER_SIZE - total_bytes,
                         &bytes_read, portMAX_DELAY);
            total_bytes += bytes_read;
        }

        Paint_Clear(WHITE);
        Paint_DrawLine(420, 450, 390, 435, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(420, 450, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(390, 435, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawString_EN(250, 150, "Recording done.", &Font48, BLACK, WHITE);
        wavesahre_rgb_lcd_display(BlackImage);
    }
    else
    {
        // Playback logic
        Paint_DrawLine(390, 435, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(410, 435, 410, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawString_EN(200, 150, "Start playing...", &Font48, BLACK, WHITE);
        wavesahre_rgb_lcd_display(BlackImage);

        size_t total_bytes = 0;
        while (total_bytes < BUFFER_SIZE)
        {
            size_t bytes_written = 0;
            speaker_i2s_write(record_buffer + total_bytes / 2,
                              BUFFER_SIZE - total_bytes,
                              &bytes_written, portMAX_DELAY);
            total_bytes += bytes_written;
        }

        Paint_Clear(WHITE);
        Paint_DrawLine(420, 450, 390, 435, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(420, 450, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawLine(390, 435, 390, 465, RED, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        Paint_DrawString_EN(250, 150, "Playback done.", &Font48, BLACK, WHITE);
        wavesahre_rgb_lcd_display(BlackImage);
    }
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
    
    // Allocate LCD frame buffer
    UDOUBLE Imagesize = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES * 2;
    BlackImage = (UBYTE *)malloc(Imagesize);
    if (!BlackImage)
    {
        Serial.println("Failed to allocate memory for frame buffer...");
        exit(0);
    }

    // Initialize the graphics canvas with buf2
    Paint_NewImage(BlackImage, EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES, 0, WHITE);

    // Set the scale for the graphical canvas
    Paint_SetScale(65);

    // Clear the canvas and fill it with a white background
    Paint_Clear(WHITE);

    // Draw initial red record button
    Paint_DrawCircle(405, 450, 15, RED, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawString_EN(100, 150, "Click to start recording", &Font48, BLACK, WHITE);
    wavesahre_rgb_lcd_display(BlackImage);

    // Initialize speaker codec
    codec_init();
    speaker_codec_volume_set(100, NULL);
    microphone_codec_gain_set(30, NULL);

    // Allocate memory for recording buffer
    record_buffer = (int16_t *)heap_caps_malloc(BUFFER_SIZE * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!record_buffer)
    {
        Serial.println("Failed to allocate buffer");
        vTaskDelete(NULL);
        return;
    }

    // Touch handling loop
    static uint16_t prev_x;
    static uint16_t prev_y;
    bool is_playing = false;
  
    // Main application loop
    while (1)
    {
        point_data = touch_gt911_read_point(1);
        if (point_data.cnt == 1)
        {
            if (prev_x == point_data.x[0] && prev_y == point_data.y[0])
            {
                continue;
            }
            else if (point_data.x[0] > 390 && point_data.x[0] < 420 &&
                     point_data.y[0] > 420 && point_data.y[0] < 480)
            {
                Paint_Clear(WHITE);
                is_playing = !is_playing;
                play_or_pause(is_playing);

                prev_x = point_data.x[0];
                prev_y = point_data.y[0];
            }
        }
        delay(30);
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
