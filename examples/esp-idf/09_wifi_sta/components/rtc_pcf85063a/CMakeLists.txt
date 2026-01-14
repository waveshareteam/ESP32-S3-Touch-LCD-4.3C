idf_component_register(SRCS "rtc_pcf85063a.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver i2c gpio io_extension
                    )
