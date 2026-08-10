idf_component_register(SRCS "sd.c"  
                        INCLUDE_DIRS "."
                        REQUIRES esp_driver_sdmmc sdmmc fatfs i2c gpio io_extension
                    )
