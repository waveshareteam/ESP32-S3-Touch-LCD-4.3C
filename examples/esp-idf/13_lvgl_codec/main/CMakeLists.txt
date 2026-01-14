file(GLOB_RECURSE srcs *.c
                   ui/*.c
                   ui/assets/*.c
                   )
set(include_dirs 
   . 
   ui
   ui/assets
   )

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    WHOLE_ARCHIVE
    )

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format) 