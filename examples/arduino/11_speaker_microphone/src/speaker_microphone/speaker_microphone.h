/*****************************************************************************
 * | File         :   speaker_microphone.h
 * | Author       :   Waveshare team
 * | Function     :   Hardware underlying interface for speaker and microphone
 * | Info         :
 * |                 Provides initialization functions and configuration
 * |                 macros for I2S audio input/output with an external codec.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2025-07-28
 * | Info         :   Basic version
 *
 ******************************************************************************/

#ifndef __SPEAKER_MICROPHONE_H
#define __SPEAKER_MICROPHONE_H

#include <stdio.h>                      // Standard I/O functions
#include <string.h>                     // String operations
#include "lib/esp_codec_dev/include/esp_codec_dev_defaults.h"     // ESP default codec config
#include "lib/esp_codec_dev/include/esp_codec_dev.h"              // ESP codec driver API
#include "driver/i2s_std.h"             // Standard I2S driver
#include "driver/i2s_tdm.h"
#include "err_check.h"                  // Custom error checking utilities
#include "esp_err.h"                    // ESP error code definitions
#include "esp_log.h"                    // Logging macros
#include "../i2c/i2c.h"                        // I2C pin configuration
#include "../io_extension/io_extension.h"               // Additional I/O operations

// I2S peripheral instance number
#define I2S_NUM           (I2S_NUM_1)              // Use I2S port 1

// I2S pin assignments for ESP32-S3
#define I2S_SCLK          (GPIO_NUM_44)    // Serial Clock (Bit Clock)
#define I2S_MCLK          (GPIO_NUM_6)     // Master Clock
#define I2S_LCLK          (GPIO_NUM_16)    // Word Select / Left-Right Clock
#define I2S_DOUT          (GPIO_NUM_15)    // Data Output to Speaker
#define I2S_DSIN          (GPIO_NUM_43)    // Data Input from Microphone

// GPIO pin to enable external power amplifier (set to -1 if not used)
#define POWER_AMP_IO      (-1)

// I2C address for the ES7210 codec chip
#define ES7210_CODEC_ADDR ES7210_CODEC_DEFAULT_ADDR

// I2S GPIO configuration structure
#define I2S_GPIO_CFG       \
    {                          \
        .mclk = I2S_MCLK,      /* Master Clock GPIO */ \
        .bclk = I2S_SCLK,      /* Bit Clock GPIO */ \
        .ws   = I2S_LCLK,      /* Word Select GPIO */ \
        .dout = I2S_DOUT,      /* Data Out GPIO */ \
        .din  = I2S_DSIN,      /* Data In GPIO */ \
        .invert_flags = {      \
            .mclk_inv = false, /* No MCLK inversion */ \
            .bclk_inv = false, /* No BCLK inversion */ \
            .ws_inv   = false, /* No WS inversion */ \
        },                     \
    }

// I2S full-duplex mono configuration macro
// _sample_rate: desired audio sampling rate (e.g., 16000, 44100)
#define I2S_DUPLEX_MONO_CFG(_sample_rate)                                                         \
    {                                                                                             \
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(_sample_rate),                                      /* Clock settings */ \
        .slot_cfg = I2S_STD_PHILIP_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO), /* Mono, 16-bit */ \
        .gpio_cfg = I2S_GPIO_CFG,                                                                 /* Apply GPIO config */ \
    }

// I2S full-duplex mono configuration macro
// _sample_rate: desired audio sampling rate (e.g., 16000, 44100)
#define I2S_DUPLEX_MONO_CFG_TDM(_sample_rate,slot_mask)                                                         \
    {                                                                                             \
        .clk_cfg = I2S_TDM_CLK_DEFAULT_CONFIG(_sample_rate),                                      /* Clock settings */ \
        .slot_cfg = I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, slot_mask), /* Mono, 16-bit */ \
        .gpio_cfg = I2S_GPIO_CFG,                                                                 /* Apply GPIO config */ \
    }

// Initialize speaker output via I2S and codec
// Returns a handle to the codec device used for playback
esp_codec_dev_handle_t speaker_init(void);

// Initialize microphone input via I2S and codec
// Returns a handle to the codec device used for recording
esp_codec_dev_handle_t microphone_init(void);

#endif // __SPEAKER_MICROPHONE_H
