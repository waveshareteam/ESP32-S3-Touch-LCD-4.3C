/*
* Ported LVGL 8.4 and display the official demo interface.
*/
#include "src/lvgl_port/lvgl_port.h"       // LVGL porting functions for integration
#include <demos/lv_demos.h>        // LVGL demo headers

void setup() {
    static esp_lcd_panel_handle_t panel_handle = NULL; // Declare a handle for the LCD panel
    static esp_lcd_touch_handle_t tp_handle = NULL;    // Declare a handle for the touch panel

    DEV_I2C_Init();
    IO_EXTENSION_Init();
    // Initialize the GT911 touch screen controller
    tp_handle = touch_gt911_init(DEV_I2C_Get_Bus_Device());
    
    // Initialize the Waveshare ESP32-S3 RGB LCD hardware
    panel_handle = waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();   

    // Initialize LVGL with the panel and touch handles
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    Serial.println("Display LVGL demos");

    // Lock the mutex because LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        // Uncomment and run the desired demo functions here
        // lv_demo_stress();  // Stress test demo
        // lv_demo_benchmark(); // Benchmark demo
        // lv_demo_music();     // Music demo
        lv_demo_widgets();    // Widgets demo
        
        // Release the mutex after the demo execution
        lvgl_port_unlock();
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
