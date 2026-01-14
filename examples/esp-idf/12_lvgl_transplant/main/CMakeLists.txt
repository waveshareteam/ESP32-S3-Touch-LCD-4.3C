idf_component_register(
    SRCS "main.c" 
    INCLUDE_DIRS "." 
    WHOLE_ARCHIVE
    )

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format) 