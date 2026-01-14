/*
* Set RTC time and alarm, then print current time via USB serial output
*/
#include "src/io_extension/io_extension.h"       // IO extension control (e.g., external interrupt input)
#include "src/rtc_pcf85063a/rtc_pcf85063a.h"     // PCF85063A RTC driver

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

void setup() {
  // Start Serial 
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
  
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

}

datetime_t Now_time;
void loop() {
  // Read current time from RTC
  PCF85063A_Read_now(&Now_time);

  // Format current time as a string
  datetime_to_str(datetime_str, Now_time);
  printf("Now_time is %s\r\n", datetime_str);

  // Poll external IO pin for alarm (low level = alarm triggered)
  if (IO_EXTENSION_Rtc_Int_Read() == 0)
  {
      // Re-enable alarm if repeated alarms are required
      PCF85063A_Enable_Alarm();
      printf("The alarm clock goes off.\r\n");
  }
  delay(1000);
}
