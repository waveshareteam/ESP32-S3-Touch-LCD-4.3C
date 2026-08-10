/*****************************************************************************
 * | File         :   io_extension.h
 * | Author       :   Waveshare team
 * | Function     :   GPIO control using io extension via I2C interface
 * | Info         :
 * |                 Header file for controlling GPIO pins via the io extension
 * |                 chip using I2C communication. This file defines the
 * |                 necessary I2C addresses, commands, and GPIO pin control
 * |                 functions.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-19
 * | Info         :   Basic version
 *
 ******************************************************************************/

 #ifndef __IO_EXTENSION_H
 #define __IO_EXTENSION_H
 
 #include "../i2c/i2c.h"  // Include I2C header for I2C communication functions
 
 /* 
  * IO EXTENSION GPIO control via I2C - Register and Command Definitions
  *
  *
  * Example usage:
  * 1. Set the working mode by writing to the register at address 0x24
  * 2. Send function commands to control the GPIO pins and modes
  */
 
 /* IO EXTENSION Function Register Addresses */
 #define IO_EXTENSION_ADDR          0x24  // Slave address for mode configuration register
 
 /* Mode control flags (from the chip manual) */
 #define IO_EXTENSION_Mode             0x02 // 
 #define IO_EXTENSION_IO_OUTPUT_ADDR   0x03 // 
 #define IO_EXTENSION_IO_INPUT_ADDR    0x04 // 
 #define IO_EXTENSION_PWM_ADDR         0x05 // 
 #define IO_EXTENSION_ADC_ADDR         0x06 // 
 #define IO_EXTENSION_RTC_INT_ADDR     0x07 // 

 /* Specific IO pin assignments */
 #define IO_EXTENSION_IO_0          0x00  // IO0 
 #define IO_EXTENSION_IO_1          0x01  // IO1 (used for touch reset)
 #define IO_EXTENSION_IO_2          0x02  // IO2 (backlight control)
 #define IO_EXTENSION_IO_3          0x03  // IO3 (used for lcd reset)
 #define IO_EXTENSION_IO_4          0x04  // IO4 (SD card CS pin)
 #define IO_EXTENSION_IO_5          0x05  // IO5 (Select communication interface: 0 for USB, 1 for CAN)
 #define IO_EXTENSION_IO_6          0x06  // IO6
 #define IO_EXTENSION_IO_7          0x07  // IO7
 
 #define DI0 IO_EXTENSION_IO_0
 #define DI1 IO_EXTENSION_IO_5
 #define DO0 IO_EXTENSION_IO_6
 #define DO1 IO_EXTENSION_IO_7
 
 /* Structure to represent the IO EXTENSION device */
 typedef struct _io_extension_obj_t {
     i2c_master_dev_handle_t addr;      // Handle for mode configuration
     uint8_t Last_io_value;
     uint8_t Last_od_value;
 } io_extension_obj_t;
 
 
 /* Function declarations */
 void IO_EXTENSION_Init();                     // Initialize the IO_EXTENSION device
 void IO_EXTENSION_IO_Mode(uint8_t pin);
 void IO_EXTENSION_Output(uint8_t pin, uint8_t value);     // Set IO pin output (high/low)
 uint8_t IO_EXTENSION_Input(uint8_t pin);   // Read IO pin input state
 void IO_EXTENSION_Pwm_Output(uint8_t Value);
 uint16_t IO_EXTENSION_Adc_Input();
 uint8_t IO_EXTENSION_Rtc_Int_Read();
 
 #endif  // __IO_EXTENSION_H
 