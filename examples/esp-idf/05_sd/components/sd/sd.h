/*****************************************************************************
 * | File         :   sd.h
 * | Author       :   Waveshare team
 * | Function     :   SD card driver configuration header file
 * | Info         :
 * |                 This header file provides definitions and function 
 * |                 declarations for initializing, mounting, and managing 
 * |                 an SD card using ESP-IDF.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-28
 * | Info         :   Basic version
 *
 ******************************************************************************/

#ifndef __SD_H
#define __SD_H

// Include necessary ESP-IDF and driver headers
#include <sys/unistd.h>      // For file system operations
#include <sys/stat.h>        // For file system metadata
#include "esp_vfs_fat.h"     // FAT filesystem and VFS integration
#include "sdmmc_cmd.h"       // SD card commands
#include "driver/sdmmc_host.h" // SDMMC host driver

#include "io_extension.h"          // IO EXTENSION I2C CAN control header (optional inclusion)

#define SD_TAG "sd"          // Log tag for SD card functions

// Define constants for SD card configuration
#define MOUNT_POINT "/sdcard"                // Mount point for SD card
#define EXAMPLE_FORMAT_IF_MOUNT_FAILED false // Format SD card if mounting fails
#define EXAMPLE_PIN_CLK GPIO_NUM_12          // GPIO pin for SD card clock
#define EXAMPLE_PIN_CMD GPIO_NUM_11          // GPIO pin for SD card command line
#define EXAMPLE_PIN_D0  GPIO_NUM_13          // GPIO pin for SD card data line (D0)

// Function declarations

/**
 * @brief Initialize the SD card and mount the FAT filesystem.
 * 
 * @retval ESP_OK if initialization and mounting succeed.
 * @retval ESP_FAIL if an error occurs.
 */
esp_err_t sd_mmc_init();

/**
 * @brief Unmount the SD card and release resources.
 * 
 * @retval ESP_OK if unmounting succeeds.
 * @retval ESP_FAIL if an error occurs.
 */
esp_err_t sd_mmc_unmount();

/**
 * @brief Print detailed information about the SD card.
 * 
 * This function uses `sdmmc_card_print_info` to display SD card details such 
 * as manufacturer, type, size, and more.
 */
void sd_card_print_info();

/**
 * @brief Get total and available memory capacity of the SD card.
 * 
 * @param total_capacity Pointer to store the total capacity (in KB).
 * @param available_capacity Pointer to store the available capacity (in KB).
 * 
 * @retval ESP_OK if memory information is successfully retrieved.
 * @retval ESP_FAIL if an error occurs.
 */
esp_err_t read_sd_capacity(size_t *total_capacity, size_t *available_capacity);

#endif  // __SD_H
