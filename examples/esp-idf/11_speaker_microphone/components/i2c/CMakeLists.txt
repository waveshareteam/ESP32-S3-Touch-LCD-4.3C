idf_component_register(SRCS "i2c.c" 
                        INCLUDE_DIRS "."
                        REQUIRES esp_driver_i2c gpio
                    )
