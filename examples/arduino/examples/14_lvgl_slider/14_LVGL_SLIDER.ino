/*
 * Demonstrates an LVGL slider to control LED brightness and display battery voltage.
 */

#include "src/lvgl_port/lvgl_port.h"       // LVGL porting functions for integration
#include <demos/lv_demos.h>      // Optional: LVGL demo headers for extra examples

static lv_obj_t *label;          // Label to display slider value
static lv_obj_t *BAT_Label;      // Label to display battery voltage
char bat_v[20];                  // Buffer to store formatted battery voltage string

/**
 * @brief Callback function to update the battery voltage label.
 * 
 * Called by an LVGL timer to refresh the battery voltage shown on the UI.
 * 
 * @param timer Pointer to the LVGL timer object
 */
static void bat_cb(lv_timer_t * timer) 
{
    lv_label_set_text(BAT_Label, bat_v); // Update the battery voltage label with latest value
}

/**
 * @brief Event callback for slider interaction.
 * 
 * Triggered when the slider value changes.
 * Updates the label with the current value and adjusts LED brightness via PWM.
 * 
 * @param e Pointer to the LVGL event descriptor
 */
static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t *slider = lv_event_get_target(e); // Get the slider that triggered the event

    // Update the label with the slider's current value
    lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));

    // Set the PWM duty cycle based on slider value (inverted: 100 = off, 0 = full brightness)
    uint8_t duty = (100 - lv_slider_get_value(slider));
    IO_EXTENSION_Pwm_Output(duty); // Optional external PWM controller
}

/**
 * @brief Creates an LVGL slider to control LED brightness.
 * 
 * Also creates a label to display slider value and another label to show battery voltage.
 */
void lvgl_slider(void)
{
    IO_EXTENSION_Pwm_Output(0); // Optional external control for brightness

    // Create the slider widget on the active LVGL screen
    lv_obj_t *slider = lv_slider_create(lv_scr_act());

    // Set slider dimensions and center it on screen
    lv_obj_set_width(slider, 200);
    lv_obj_center(slider);

    // Set initial slider value to 100 (LED off if active-low)
    lv_slider_set_value(slider, 100, LV_ANIM_OFF);

    // Attach the event callback for slider changes
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create a label to display the current slider value
    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "100"); // Initial label value
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); // Position above slider

    // Create a label for displaying battery voltage
    BAT_Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(BAT_Label, LV_SIZE_CONTENT);
    lv_obj_set_height(BAT_Label, LV_SIZE_CONTENT);
    lv_obj_center(BAT_Label);
    lv_obj_set_y(BAT_Label, 30); 
    lv_label_set_text(BAT_Label, "BAT:3.7V"); // Initial placeholder text

    // Style the battery label
    lv_obj_set_style_text_color(BAT_Label, lv_color_hex(0xFFA500), LV_PART_MAIN | LV_STATE_DEFAULT); // Orange text
    lv_obj_set_style_text_opa(BAT_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);                     // Full opacity
    lv_obj_set_style_text_font(BAT_Label, &lv_font_montserrat_44, LV_PART_MAIN | LV_STATE_DEFAULT); // Large font
}

/**
 * @brief Main setup function.
 * 
 * Initializes serial output, LCD panel, touch input, and LVGL.
 * Then creates the user interface with slider and labels.
 */
void setup() {
    static esp_lcd_panel_handle_t panel_handle = NULL; // LCD panel handle
    static esp_lcd_touch_handle_t tp_handle = NULL;    // Touch panel handle

    Serial.begin(115200);

    DEV_I2C_Init();
    IO_EXTENSION_Init();

    // Initialize GT911 capacitive touch controller
    tp_handle = touch_gt911_init(DEV_I2C_Get_Bus_Device());  

    // Initialize Waveshare ESP32-S3 RGB LCD panel
    panel_handle = waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();   

    // Initialize LVGL port with LCD and touch handles
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    Serial.println("Display LVGL demos");

    // Lock LVGL access, create UI, then unlock
    if (lvgl_port_lock(-1)) {
        lvgl_slider();  // Create UI elements
        lvgl_port_unlock();
    }
}

/**
 * @brief Main loop function.
 * 
 * Continuously reads battery voltage via ADC and updates the label via a one-shot LVGL timer.
 */
void loop() {
    float value = 0; // Battery voltage accumulator

    // Take 10 ADC readings and average them to reduce noise
    for (int i = 0; i < 10; i++) {
        value += IO_EXTENSION_Adc_Input(); // Custom function to read ADC input
        delay(20); // Small delay between samples
    }
    value /= 10.0; // Compute average

    // Convert ADC value to voltage (assuming 10-bit ADC, 3.3V reference, and 3:1 voltage divider)
    value *= 3 * 3.3 / 1023.0;

    // Clamp voltage to max 4.2V for safe display
    if (value > 4.2) {
        value = 4.2;
    }

    // Format battery voltage string for display
    sprintf(bat_v, "BAT:%0.2fV", value);

    // Create a one-shot LVGL timer to update the label
    lv_timer_t *t = lv_timer_create(bat_cb, 100, NULL); // Trigger after 100 ms
    lv_timer_set_repeat_count(t, 1); // Run only once

    delay(100); // Wait before the next cycle
}
