idf_component_register(SRCS "io_extension.c" 
                        INCLUDE_DIRS "."
                        REQUIRES driver i2c gpio
                    )
