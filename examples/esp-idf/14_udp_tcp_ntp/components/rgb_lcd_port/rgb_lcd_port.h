/*****************************************************************************
 * | File        :   rgb_lcd_port.h
 * | Author      :   Waveshare team
 * | Function    :   Hardware underlying interface
 * | Info        :
 *                   This header file contains configuration and function 
 *                   declarations for the RGB LCD driver interface.
 *----------------
 * | Version     :   V1.0
 * | Date        :   2024-11-19
 * | Info        :   Basic version
 *
 ******************************************************************************/

#ifndef _RGB_LCD_H_
#define _RGB_LCD_H_

#pragma once

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "io_extension.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief LCD Resolution and Timing
 */
#define EXAMPLE_LCD_H_RES               (800)  ///< Horizontal resolution in pixels
#define EXAMPLE_LCD_V_RES               (480)  ///< Vertical resolution in pixels
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ      (16 * 1000 * 1000) ///< Pixel clock frequency in Hz

/**
 * @brief Color and Pixel Configuration
 */
#define EXAMPLE_LCD_BIT_PER_PIXEL       (16)   ///< Bits per pixel (color depth)
#define EXAMPLE_RGB_BIT_PER_PIXEL       (16)   ///< RGB interface color depth
#define EXAMPLE_RGB_DATA_WIDTH          (16)   ///< Data width for RGB interface
#define EXAMPLE_LCD_RGB_BUFFER_NUMS     (2)    ///< Number of frame buffers for double buffering
#define EXAMPLE_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_H_RES * 10) ///< Size of bounce buffer for RGB data

/**
 * @brief GPIO Pins for RGB LCD Signals
 */
#define EXAMPLE_LCD_IO_RGB_DISP         (-1)   ///< DISP signal, -1 if not used
#define EXAMPLE_LCD_IO_RGB_VSYNC        (GPIO_NUM_3)  ///< Vertical sync signal
#define EXAMPLE_LCD_IO_RGB_HSYNC        (GPIO_NUM_46) ///< Horizontal sync signal
#define EXAMPLE_LCD_IO_RGB_DE           (GPIO_NUM_5)  ///< Data enable signal
#define EXAMPLE_LCD_IO_RGB_PCLK         (GPIO_NUM_7)  ///< Pixel clock signal

/**
 * @brief GPIO Pins for RGB Data Signals
 */
// Blue data signals
#define EXAMPLE_LCD_IO_RGB_DATA0        (GPIO_NUM_14) ///< B3
#define EXAMPLE_LCD_IO_RGB_DATA1        (GPIO_NUM_38) ///< B4
#define EXAMPLE_LCD_IO_RGB_DATA2        (GPIO_NUM_18) ///< B5
#define EXAMPLE_LCD_IO_RGB_DATA3        (GPIO_NUM_17) ///< B6
#define EXAMPLE_LCD_IO_RGB_DATA4        (GPIO_NUM_10) ///< B7

// Green data signals
#define EXAMPLE_LCD_IO_RGB_DATA5        (GPIO_NUM_39) ///< G2
#define EXAMPLE_LCD_IO_RGB_DATA6        (GPIO_NUM_0)  ///< G3
#define EXAMPLE_LCD_IO_RGB_DATA7        (GPIO_NUM_45) ///< G4
#define EXAMPLE_LCD_IO_RGB_DATA8        (GPIO_NUM_48) ///< G5
#define EXAMPLE_LCD_IO_RGB_DATA9        (GPIO_NUM_47) ///< G6
#define EXAMPLE_LCD_IO_RGB_DATA10       (GPIO_NUM_21) ///< G7

// Red data signals
#define EXAMPLE_LCD_IO_RGB_DATA11       (GPIO_NUM_1)  ///< R3
#define EXAMPLE_LCD_IO_RGB_DATA12       (GPIO_NUM_2)  ///< R4
#define EXAMPLE_LCD_IO_RGB_DATA13       (GPIO_NUM_42) ///< R5
#define EXAMPLE_LCD_IO_RGB_DATA14       (GPIO_NUM_41) ///< R6
#define EXAMPLE_LCD_IO_RGB_DATA15       (GPIO_NUM_40) ///< R7

/**
 * @brief Reset and Backlight Configuration
 */
#define EXAMPLE_LCD_IO_RST              (-1)   ///< Reset pin, -1 if not used
#define EXAMPLE_PIN_NUM_BK_LIGHT        (-1)   ///< Backlight pin, -1 if not used
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL   (1)    ///< Logic level to turn on backlight
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL  (!EXAMPLE_LCD_BK_LIGHT_ON_LEVEL) ///< Logic level to turn off backlight

/**
 * @brief Function Declarations
 */
esp_lcd_panel_handle_t waveshare_esp32_s3_rgb_lcd_init();
/**
 * @brief Turn on the LCD backlight.
 */
void wavesahre_rgb_lcd_bl_on();
/**
 * @brief Turn off the LCD backlight.
 */
void wavesahre_rgb_lcd_bl_off();

/**
 * @brief Display a rectangular region of an image on the RGB LCD.
 *
 * @param Xstart Starting X coordinate of the region.
 * @param Ystart Starting Y coordinate of the region.
 * @param Xend Ending X coordinate of the region.
 * @param Yend Ending Y coordinate of the region.
 * @param Image Pointer to the image data buffer.
 */
void wavesahre_rgb_lcd_display_window(int16_t Xstart, int16_t Ystart, int16_t Xend, int16_t Yend, uint8_t *Image);

/**
 * @brief Display a full-frame image on the RGB LCD.
 *
 * @param Image Pointer to the image data buffer.
 */
void wavesahre_rgb_lcd_display(uint8_t *Image);

/**
 * @brief Retrieve pointers to the frame buffers for double buffering.
 *
 * @param buf1 Pointer to hold the address of the first frame buffer.
 * @param buf2 Pointer to hold the address of the second frame buffer.
 */
void waveshare_get_frame_buffer(void **buf1, void **buf2);

#endif // _RGB_LCD_H_
