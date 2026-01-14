idf_component_register(
    SRCS "main.c" 
    INCLUDE_DIRS "." ${IDF_PATH}/examples/peripherals/i2s/common
    WHOLE_ARCHIVE
    )
