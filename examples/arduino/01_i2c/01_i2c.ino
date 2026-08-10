/*
 * This example demonstrates I2C communication to control the IO EXTENSION chip
 * for switching the LCD backlight on and off in a blinking pattern.
 */

#include "src/io_extension/io_extension.h"

void setup() {
  // Initialize the IO EXTENSION device for I2C communication.
  DEV_I2C_Init();
  IO_EXTENSION_Init();
  delay(10);
}

void loop() {
  // Turn on the LCD backlight by setting IO_EXTENSION_IO_2 high.
  IO_EXTENSION_Output(IO_EXTENSION_IO_2, 1);
  delay(500);  // Wait for 500 milliseconds.

  // Turn off the LCD backlight by setting IO_EXTENSION_IO_2 low.
  IO_EXTENSION_Output(IO_EXTENSION_IO_2, 0);
  delay(500);  // Wait for 500 milliseconds.
}
