/*****************************************************************************
 * | File         :   sd.c
 * | Author       :   Waveshare team
 * | Function     :   SD card driver code for mounting, reading capacity, and unmounting
 * | Info         :
 * |                  This is the C file for SD card configuration and usage.
 * ----------------
 * | This version :   V1.0
 * | Date         :   2024-11-28
 * | Info         :   Basic version, includes functions to initialize,
 * |                  read memory capacity, and manage SD card mounting/unmounting.   
 *
 ******************************************************************************/

#include "user_sd.h"  // Include the header file for SD card operations

// Define the mount point for the SD card
const char mount_point[] = MOUNT_POINT;

/**
 * @brief Initialize the SD card and mount the filesystem.
 * 
 * This function configures the SDMMC peripheral, sets up the host and slot,
 * and mounts the FAT filesystem from the SD card. The pins for SDMMC communication
 * are also configured.
 * 
 * @retval ESP_OK if initialization and mounting succeed.
 * @retval ESP_FAIL if an error occurs during the process.
 */
esp_err_t sd_mmc_init() {
    // Configure SDMMC pins
    if (!SD_MMC.setPins(EXAMPLE_PIN_CLK, EXAMPLE_PIN_CMD, EXAMPLE_PIN_D0)) {
        printf("Failed to set SDMMC pins.\r\n");
        return ESP_FAIL;
    }

    // Begin the SDMMC interface and attempt to mount the filesystem
    if (!SD_MMC.begin(mount_point, true, EXAMPLE_FORMAT_IF_MOUNT_FAILED)) {
        printf("Failed to mount SD card.\r\n");
        return ESP_FAIL;
    }

    // Retrieve and verify the type of SD card connected
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        printf("No SD card detected.\r\n");
        return ESP_FAIL;
    }

    // Print the type of SD card detected
    printf("SD Card Type: ");
    if (cardType == CARD_MMC) {
        printf("MMC\r\n");
    } else if (cardType == CARD_SD) {
        printf("SDSC\r\n");
    } else if (cardType == CARD_SDHC) {
        printf("SDHC\r\n");
    } else {
        printf("UNKNOWN\r\n");
    }

    return ESP_OK;
}

/**
 * @brief Unmount the SD card and release associated resources.
 * 
 * This function unmounts the FAT filesystem and ensures all resources
 * associated with the SD card are properly released.
 * 
 * @retval ESP_OK if unmounting succeeds.
 * @retval ESP_FAIL if an error occurs.
 */
void sd_mmc_unmount() {
    SD_MMC.end();
}

/**
 * @brief Retrieve total and available memory capacity of the SD card.
 * 
 * This function fetches the total memory size and the used memory size
 * on the SD card, both reported in kilobytes (KB).
 * 
 * @param[out] total_capacity Pointer to store the total capacity (in KB).
 * @param[out] available_capacity Pointer to store the available capacity (in KB).
 * 
 * @retval ESP_OK if capacity information is successfully retrieved.
 * @retval ESP_FAIL if an error occurs while fetching capacity information.
 */
esp_err_t read_sd_capacity(uint64_t *total_capacity, uint64_t *available_capacity) {
    // Validate input pointers
    if (total_capacity == NULL || available_capacity == NULL) {
        printf("Invalid pointers for capacity retrieval.\r\n");
        return ESP_FAIL;
    }

    // Retrieve card size and used bytes, converting to kilobytes
    *total_capacity = SD_MMC.cardSize() / 1024;  // Convert bytes to kilobytes
    *available_capacity = *total_capacity - (SD_MMC.usedBytes() / 1024);  // Convert bytes to kilobytes

    return ESP_OK;
}
