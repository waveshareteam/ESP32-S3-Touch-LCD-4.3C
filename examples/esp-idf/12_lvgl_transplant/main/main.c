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
#include "esp_check.h"    // Error handling macros
#include "lv_demos.h"     // LVGL demo headers  
#include "lvgl_port.h"    // LVGL porting functions for integration

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

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        // lv_demo_stress();
        // lv_demo_benchmark();
        // lv_demo_music();
        lv_demo_widgets();
        // Release the mutex
        lvgl_port_unlock();
    }
    wavesahre_rgb_lcd_bl_on(); // Turn on the LCD backlight
}
