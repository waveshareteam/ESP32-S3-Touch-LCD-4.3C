idf_component_register(SRCS "rgb_lcd_port.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver esp_lcd gpio i2c io_extension lvgl_port
                        )
