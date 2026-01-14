/*****************************************************************************
 * | File       :   main.c
 * | Author     :   Waveshare team
 * | Function   :   Main function
 * | Info       :   Ported LVGL 9.2.0 and display the official demo interface
 * | Version    :   V1.0
 * | Date       :   2025-07-30
 * | Language   :   C (ESP-IDF)
 ******************************************************************************/

#include "rgb_lcd_port.h" // LCD display driver
#include "gt911.h"        // GT911 touch controller
#include "sd.h"        // GT911 touch controller
#include "codec_dev.h"  // Include I2C driver header for I2C functions
#include "esp_check.h"    // Error handling macros
#include "lvgl_port.h"    // LVGL porting functions for integration


#include "user_lv_demo_music.h"

static const char *TAG = "main";

void app_main()
{
    static esp_lcd_panel_handle_t panel_handle = NULL; // Declare a handle for the LCD panel
    static esp_lcd_touch_handle_t tp_handle = NULL;

    DEV_I2C_Init(); // Initialize I2C port
    IO_EXTENSION_Init(); // Initialize the IO EXTENSION GPIO chip 

    wavesahre_rgb_lcd_bl_off(); // Turn off the LCD backlight

    tp_handle = touch_gt911_init(DEV_I2C_Get_Bus_Device()); // Initialize the GT911 touch screen controller
    panel_handle = waveshare_esp32_s3_rgb_lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD hardware
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle)); // Initialize LVGL with the panel and touch handles

    ESP_LOGI(TAG, "Display LVGL demos");
    
    // Initialize SD card
    if (sd_mmc_init() == ESP_OK) 
    {
        ESP_LOGI(TAG, "SD Card OK!");
        ESP_LOGI(TAG, "Click the arrow to start.");

        list_files(MOUNT_POINT"/music");
        if (mp3_num == 0)
        {
            ESP_LOGI(TAG, "No MP3 file found in SD card.");
            return;
        }
        else
        {
            ESP_LOGI(TAG, "music start");
        }
    }
    else
    {
        ESP_LOGI(TAG, "SD Card Fail!");
        return;
    }

    // Initialize speaker and audio player
    speaker_codec_init();
    speaker_codec_volume_set(50, NULL);
    speaker_player_register_callback(speaker_callback, NULL);
    speaker_player_init();

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        user_lv_demo_music();
        // Release the mutex
        lvgl_port_unlock();
    }
    wavesahre_rgb_lcd_bl_on(); // Turn on the LCD backlight
}
