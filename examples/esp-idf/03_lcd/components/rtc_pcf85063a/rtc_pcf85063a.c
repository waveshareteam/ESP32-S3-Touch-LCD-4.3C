/*****************************************************************************
 * | File      	:   rtc_pcf85063a.c
 * | Author      :   Waveshare team
 * | Function    :   PCF85063A driver
 * | Info        :   PCF85063A 
 *----------------
 * |	This version:   V1.0
 * | Date        :   2024-02-02
 * | Info        :   Basic version
 *
 ******************************************************************************/

#include "rtc_pcf85063a.h"

static uint8_t decToBcd(int val);
static int bcdToDec(uint8_t val);
i2c_master_dev_handle_t RTC_DEV;

const unsigned char MonthStr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**
 * Initialize PCF85063A 
 **/
void PCF85063A_Init()
{
	// Set the I2C slave address for the IO_EXTENSION device
    DEV_I2C_Set_Slave_Addr(&RTC_DEV, PCF85063A_ADDRESS);

	uint8_t Value = RTC_CTRL_1_DEFAULT | RTC_CTRL_1_CAP_SEL;
	DEV_I2C_Write_Byte(RTC_DEV, RTC_CTRL_1_ADDR, Value);
}

/**
 * Software reset PCF85063A 
 **/
void PCF85063A_Reset()
{
	uint8_t Value = RTC_CTRL_1_DEFAULT | RTC_CTRL_1_CAP_SEL | RTC_CTRL_1_SR;
	DEV_I2C_Write_Byte(RTC_DEV, RTC_CTRL_1_ADDR, Value);
}

/**
 * Set RTC time 
 **/
void PCF85063A_Set_Time(datetime_t time)
{
	uint8_t buf[4] = {RTC_SECOND_ADDR,
					  decToBcd(time.sec),
					  decToBcd(time.min),
					  decToBcd(time.hour)};
	DEV_I2C_Write_Nbyte(RTC_DEV, buf, 4);
}

/**
 * Set RTC date 
 **/
void PCF85063A_Set_Date(datetime_t date)
{
	uint8_t buf[5] = {RTC_DAY_ADDR,
					  decToBcd(date.day),
					  decToBcd(date.dotw),
					  decToBcd(date.month),
					  decToBcd(date.year - YEAR_OFFSET)};
	DEV_I2C_Write_Nbyte(RTC_DEV, buf, 5);
}

/**
 * Set both RTC time and date 
 **/
void PCF85063A_Set_All(datetime_t time)
{
	uint8_t buf[8] = {RTC_SECOND_ADDR,
					  decToBcd(time.sec),
					  decToBcd(time.min),
					  decToBcd(time.hour),
					  decToBcd(time.day),
					  decToBcd(time.dotw),
					  decToBcd(time.month),
					  decToBcd(time.year - YEAR_OFFSET)};
	DEV_I2C_Write_Nbyte(RTC_DEV, buf, 8);
}

/**
 * Read current RTC time and date 
 **/
void PCF85063A_Read_now(datetime_t *time)
{
	uint8_t bufss[7] = {0};
	DEV_I2C_Read_Nbyte(RTC_DEV, RTC_SECOND_ADDR, bufss, 7);
	time->sec = bcdToDec(bufss[0] & 0x7F);
	time->min = bcdToDec(bufss[1] & 0x7F);
	time->hour = bcdToDec(bufss[2] & 0x3F);
	time->day = bcdToDec(bufss[3] & 0x3F);
	time->dotw = bcdToDec(bufss[4] & 0x07);
	time->month = bcdToDec(bufss[5] & 0x1F);
	time->year = bcdToDec(bufss[6]) + YEAR_OFFSET;
}

/**
 * Enable Alarm and Clear Alarm flag 
 **/
void PCF85063A_Enable_Alarm()
{
	uint8_t Value = RTC_CTRL_2_DEFAULT | RTC_CTRL_2_AIE;
	Value &= ~RTC_CTRL_2_AF;
	DEV_I2C_Write_Byte(RTC_DEV, RTC_CTRL_2_ADDR, Value);
}

/**
 * Get Alarm flag 
 **/
uint8_t PCF85063A_Get_Alarm_Flag()
{
	uint8_t Value = 0;
	DEV_I2C_Read_Nbyte(RTC_DEV, RTC_CTRL_2_ADDR, &Value, 1);
	Value &= RTC_CTRL_2_AF | RTC_CTRL_2_AIE;
	return Value;
}

/**
 * Set Alarm time 
 **/
void PCF85063A_Set_Alarm(datetime_t time)
{
	uint8_t buf[6] = {
		RTC_SECOND_ALARM,
		decToBcd(time.sec) & (~RTC_ALARM),
		decToBcd(time.min) & (~RTC_ALARM),
		decToBcd(time.hour) & (~RTC_ALARM),
		RTC_ALARM, // Disable day 
		RTC_ALARM  // Disable weekday 
	};
	DEV_I2C_Write_Nbyte(RTC_DEV, buf, 6);
}

/**
 * Read the alarm time set 
 **/
void PCF85063A_Read_Alarm(datetime_t *time)
{
	// Define a buffer to store the alarm time
	uint8_t bufss[7] = {0};

	// Read 7 bytes of data from the RTC alarm register
	DEV_I2C_Read_Nbyte(RTC_DEV, RTC_SECOND_ALARM, bufss, 7);
	
	// Convert the BCD format seconds, minutes, hours, day, and weekday into decimal and store them in the time structure
	time->sec = bcdToDec(bufss[0] & 0x7F);	// Seconds, up to 7 valid bits, mask processing 										
	time->min = bcdToDec(bufss[1] & 0x7F);	// Minutes, up to 7 valid bits, mask processing 										 
	time->hour = bcdToDec(bufss[2] & 0x3F); // Hours, 24-hour format, up to 6 valid bits, mask processing 										 
	time->day = bcdToDec(bufss[3] & 0x3F);	// Date, up to 6 valid bits, mask processing 										
	time->dotw = bcdToDec(bufss[4] & 0x07); // Day of the week, up to 3 valid bits, mask processing 									
}

/**
 * Convert normal decimal numbers to binary coded decimal 
 **/
static uint8_t decToBcd(int val)
{
	return (uint8_t)((val / 10 * 16) + (val % 10));
}

/**
 * Convert binary coded decimal to normal decimal numbers  
 **/
static int bcdToDec(uint8_t val)
{
	return (int)((val / 16 * 10) + (val % 16));
}

/**
 * Convert time to string 
 **/
void datetime_to_str(char *datetime_str, datetime_t time)
{
	sprintf(datetime_str, " %d.%d.%d  %d %d:%d:%d ", time.year, time.month,
			time.day, time.dotw, time.hour, time.min, time.sec);
}
