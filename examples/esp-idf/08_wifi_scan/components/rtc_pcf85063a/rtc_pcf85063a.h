/*
 *  rtc_pcf85063a.h
 *
 *  Created on: 13 Aug. 2021
 *      Author: Zildjhan
 */

#ifndef INC_PCF85063A_H_
#define INC_PCF85063A_H_

#include <stdint.h>
#include "i2c.h"  // Include I2C header for I2C communication functions

#define PCF85063A_ADDRESS   (0x51) /*!< I2C address of PCF85063A */

/* 
 * Year offset 
 */
#define YEAR_OFFSET			(1970)

/* 
 * Register overview - control & status registers 
 */
#define RTC_CTRL_1_ADDR     (0x00) /*!< Control Register 1 Address  */
#define RTC_CTRL_2_ADDR     (0x01) /*!< Control Register 2 Address  */
#define RTC_OFFSET_ADDR     (0x02) /*!< Offset Register Address  */
#define RTC_RAM_by_ADDR     (0x03) /*!< RAM Register Address  */

/* 
 * Register overview - time & data registers 
 */
#define RTC_SECOND_ADDR		(0x04) /*!< Seconds Register Address  */
#define RTC_MINUTE_ADDR		(0x05) /*!< Minutes Register Address  */
#define RTC_HOUR_ADDR		(0x06) /*!< Hours Register Address  */
#define RTC_DAY_ADDR		(0x07) /*!< Day Register Address  */
#define RTC_WDAY_ADDR		(0x08) /*!< Weekday Register Address  */
#define RTC_MONTH_ADDR		(0x09) /*!< Month Register Address  */
#define RTC_YEAR_ADDR		(0x0A)	/*!< Year Register Address (0-99); real year = 1970 + RCC register year */

/* 
 * Register overview - alarm registers 
 */
#define RTC_SECOND_ALARM	(0x0B) /*!< Alarm Seconds Register Address  */
#define RTC_MINUTE_ALARM	(0x0C) /*!< Alarm Minutes Register Address  */
#define RTC_HOUR_ALARM		(0x0D) /*!< Alarm Hours Register Address  */
#define RTC_DAY_ALARM		(0x0E) /*!< Alarm Day Register Address  */
#define RTC_WDAY_ALARM		(0x0F) /*!< Alarm Weekday Register Address  */

/* 
 * Register overview - timer registers 
 */
#define RTC_TIMER_VAL 	    (0x10) /*!< Timer Value Register Address  */
#define RTC_TIMER_MODE	    (0x11) /*!< Timer Mode Register Address  */

/* 
 * RTC_CTRL_1 Register 
 */
#define RTC_CTRL_1_EXT_TEST (0x80) /*!< External Test  */
#define RTC_CTRL_1_STOP     (0x20) // 0-RTC clock runs, 1-RTC clock is stopped 
#define RTC_CTRL_1_SR       (0X10) // 0-No software reset, 1-Initiate software reset 
#define RTC_CTRL_1_CIE      (0X04) // 0-No correction interrupt generated, 1-Interrupt pulses are generated at every correction cycle 
#define RTC_CTRL_1_CAP_SEL  (0X01) // 0-7PF, 1-12.5PF / 0-7PF，1-12.5PF

/* 
 * RTC_CTRL_2 Register 
 */
#define RTC_CTRL_2_AIE      (0X80) // Alarm Interrupt 0-Disable, 1-Enable 
#define RTC_CTRL_2_AF       (0X40) // Alarm Flag 0-Inactive/Cleared, 1-Active/Unchanged 
#define RTC_CTRL_2_MI       (0X20) // Minute Interrupt 0-Disable, 1-Enable 
#define RTC_CTRL_2_HMI      (0X10) // Half Minute Interrupt 
#define RTC_CTRL_2_TF       (0X08) // Timer Flag 

#define RTC_OFFSET_MODE     (0X80) // Offset Mode 

#define RTC_TIMER_MODE_TE   (0X04) // Timer Enable 0-Disable, 1-Enable 
#define RTC_TIMER_MODE_TIE  (0X02) // Timer Interrupt Enable 0-Disable, 1-Enable 
#define RTC_TIMER_MODE_TI_TP (0X01) // Timer Interrupt Mode 0-Interrupt follows Timer Flag, 1-Interrupt generates a pulse 


// Format 
#define RTC_ALARM 			(0x80)	// Set AEN_x registers 
#define RTC_CTRL_1_DEFAULT	(0x00) // RTC_CTRL_1 Default Value 
#define RTC_CTRL_2_DEFAULT	(0x00) // RTC_CTRL_2 Default Value 

#define RTC_TIMER_FLAG		(0x08) // Timer Flag 


/* 
 * DateTime structure 
 */
typedef struct {
    uint16_t year;     // Year 
    uint8_t month;    // Month 
    uint8_t day;      // Day 
    uint8_t dotw;     // Day of the Week 
    uint8_t hour;     // Hour 
    uint8_t min;      // Minute 
    uint8_t sec;      // Second 
} datetime_t;

void PCF85063A_Init(void); // Initialize PCF85063A 
void PCF85063A_Reset(void); // Reset PCF85063A 

void PCF85063A_Set_Time(datetime_t time); // Set time on PCF85063A 
void PCF85063A_Set_Date(datetime_t date); // Set date on PCF85063A 
void PCF85063A_Set_All(datetime_t time); // Set both time and date on PCF85063A 

void PCF85063A_Read_now(datetime_t *time); // Read current time from PCF85063A 

void PCF85063A_Enable_Alarm(void); // Enable alarm on PCF85063A 
uint8_t PCF85063A_Get_Alarm_Flag(); // Get alarm flag from PCF85063A 
void PCF85063A_Set_Alarm(datetime_t time); // Set alarm time on PCF85063A 
void PCF85063A_Read_Alarm(datetime_t *time); // Read alarm time from PCF85063A 

void datetime_to_str(char *datetime_str, datetime_t time); // Convert datetime to string 

#endif /* INC_PCF85063A_H_ */

// weekday format
// 0 - sunday
// 1 - monday
// 2 - tuesday
// 3 - wednesday
// 4 - thursday
// 5 - friday
// 6 - saturday