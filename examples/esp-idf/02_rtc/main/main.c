/*****************************************************************************
 * | File         :   main.c
 * | Author       :   Waveshare team
 * | Function     :   Main function
 * | Info         :
 * |                 Set RTC time and alarm, then print current time via USB serial output
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-27
 * | Info         :   Basic version
 ******************************************************************************/

#include "freertos/FreeRTOS.h"  // FreeRTOS core definitions
#include "freertos/task.h"      // FreeRTOS task management functions
#include "io_extension.h"       // IO extension control (e.g., external interrupt input)
#include "rtc_pcf85063a.h"      // PCF85063A RTC driver

static const char *TAG = "main";  // Tag for logging

// Initial RTC time to be set
static datetime_t Set_Time = {
    .year = 2025,
    .month = 07,
    .day = 30,
    .dotw = 3,   // Day of the week: 0 = Sunday
    .hour = 9,
    .min = 0,
    .sec = 0
};

// Alarm time to be set
static datetime_t Set_Alarm_Time = {
    .year = 2025,
    .month = 07,
    .day = 30,
    .dotw = 3,
    .hour = 9,
    .min = 0,
    .sec = 2
};

char datetime_str[256];  // Buffer to store formatted date-time string

/**
 * @brief Main application entry point.
 * 
 * This function initializes the I2C interface, IO extension hardware,
 * and the PCF85063A RTC. It sets the current time and alarm,
 * then enters a loop that reads the current time every second,
 * prints it over the serial interface, and checks the alarm status.
 * 
 * Note: Alarm status must be polled due to lack of direct interrupt connection.
 */
void app_main()
{
    datetime_t Now_time;

    // Initialize the I2C interface
    DEV_I2C_Init();

    // Initialize external IO extension chip
    IO_EXTENSION_Init();

    // Initialize PCF85063A RTC
    PCF85063A_Init();

    // Set current time
    PCF85063A_Set_All(Set_Time);

    // Set alarm time
    PCF85063A_Set_Alarm(Set_Alarm_Time);

    // Enable alarm interrupt
    PCF85063A_Enable_Alarm();

    while (1)
    {
        // Read current time from RTC
        PCF85063A_Read_now(&Now_time);

        // Format current time as a string
        datetime_to_str(datetime_str, Now_time);
        ESP_LOGI(TAG, "Now_time is %s", datetime_str);

        // Poll external IO pin for alarm (low level = alarm triggered)
        if (IO_EXTENSION_RTC_INT_READ() == 0)
        {
            // Re-enable alarm if repeated alarms are required
            PCF85063A_Enable_Alarm();
            ESP_LOGI(TAG, "The alarm clock goes off.");
        }

        // Wait for 1 second
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
