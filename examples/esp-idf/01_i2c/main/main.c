/*****************************************************************************
 * | File         :   main.c
 * | Author       :   Waveshare team
 * | Function     :   Main function
 * | Info         :
 * |                 Basic implementation to toggle a backlight using IO EXTENSION.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-27
 * | Info         :   Basic version
 *
 ******************************************************************************/

#include "freertos/FreeRTOS.h"  // FreeRTOS main include file
#include "freertos/task.h"      // FreeRTOS task management functions
#include "io_extension.h"             // Include the IO EXTENSION driver for GPIO control

/**
 * @brief Main application entry point.
 * 
 * This function initializes the I2C interface and the IO EXTENSION hardware,
 * then enters a loop to toggle a backlight on and off with a 500ms interval.
 */
void app_main()
{ 
    // Initialize the I2C interface and configure it for IO EXTENSION communication
    DEV_I2C_Init();

    /*
     * After initializing I2C, a device name and slave address need to be created.
     * This step corresponds to the specific slave device you want to communicate with.
     * Note: The function `DEV_I2C_Set_Slave_Addr` is not explicitly called here, 
     * because `IO_EXTENSION_Init` already sets the slave address internally.
     * Example: 
     * DEV_I2C_Set_Slave_Addr(i2c_master_dev_handle_t *dev_handle, uint8_t Addr)
     */
    IO_EXTENSION_Init();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // Enter an infinite loop to control the backlight
    while (1)
    {
        // Set the IO_EXTENSION_IO_2 pin to high (turn on the backlight)
        IO_EXTENSION_Output(IO_EXTENSION_IO_2, 1);  // Turn on backlight
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait for 500 milliseconds

        // Set the IO_EXTENSION_IO_2 pin to low (turn off the backlight)
        IO_EXTENSION_Output(IO_EXTENSION_IO_2, 0);  // Turn off backlight
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Wait for 500 milliseconds
    }
}
