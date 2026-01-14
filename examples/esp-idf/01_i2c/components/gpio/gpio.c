/*****************************************************************************
 * | File      	 :   gpio.c
 * | Author      :   Waveshare team
 * | Function    :   Hardware underlying interface
 * | Info        :
 *                   GPIO driver code
 *----------------
 * |This version :   V1.0
 * | Date        :   2024-11-19
 * | Info        :   Basic version
 *
 ******************************************************************************/
#include "gpio.h"

/**
 * @brief Configure a GPIO pin as input or output
 *
 * This function initializes a GPIO pin with the specified mode (input or output).
 * If set as input, it also enables the pull-up resistor by default.
 *
 * @param Pin GPIO pin number
 * @param Mode GPIO mode: 0 or GPIO_MODE_INPUT for input, others for output
 */
void DEV_GPIO_Mode(uint16_t Pin, uint16_t Mode)
{
    // Zero-initialize the GPIO configuration structure
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // Disable interrupts for this pin
    io_conf.pin_bit_mask = 1ULL << Pin;    // Select the GPIO pin using a bitmask

    if (Mode == 0 || Mode == GPIO_MODE_INPUT)
    {
        io_conf.mode = GPIO_MODE_INPUT;          // Set pin as input
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Enable internal pull-up resistor
    }
    else if(Mode == GPIO_MODE_INPUT_OUTPUT)
    {
        io_conf.mode = GPIO_MODE_INPUT_OUTPUT;          // Set pin as input
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Enable internal pull-up resistor
    }
    else
    {
        io_conf.mode = GPIO_MODE_OUTPUT;          // Set pin as output
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE; // Disable pull-up
    }

    gpio_config(&io_conf); // Apply the configuration
}

/**
 * @brief Configure a GPIO pin for interrupt handling
 *
 * This function sets up a GPIO pin to generate an interrupt on a negative edge
 * (falling edge) and registers the specified interrupt handler.
 *
 * @param Pin GPIO pin number
 * @param isr_handler Pointer to the interrupt handler function
 */
void DEV_GPIO_INT(int32_t Pin, gpio_isr_t isr_handler)
{
    // Zero-initialize the GPIO configuration structure
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;        // Trigger on negative edge (falling edge)
    io_conf.mode = GPIO_MODE_INPUT;               // Set pin as input mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
    io_conf.pin_bit_mask = 1ULL << Pin;           // Select the GPIO pin using a bitmask

    gpio_config(&io_conf); // Apply the configuration

    // Install the GPIO interrupt service if not already installed
    gpio_install_isr_service(0); // Pass 0 for default ISR flags

    // Register the interrupt handler for the specified pin
    gpio_isr_handler_add(Pin, isr_handler, (void *)Pin);
}

/**
 * @brief Set the logic level of a GPIO pin
 *
 * This function sets the logic level (high or low) of a GPIO pin.
 *
 * @param Pin GPIO pin number
 * @param Value Logic level: 0 for low, 1 for high
 */
void DEV_Digital_Write(uint16_t Pin, uint8_t Value)
{
    gpio_set_level(Pin, Value); // Set the GPIO pin level
}

/**
 * @brief Read the logic level of a GPIO pin
 *
 * This function reads and returns the current logic level of a GPIO pin.
 *
 * @param Pin GPIO pin number
 * @return uint8_t Logic level: 0 for low, 1 for high
 */
uint8_t DEV_Digital_Read(uint16_t Pin)
{
    return gpio_get_level(Pin); // Get the GPIO pin level
}
