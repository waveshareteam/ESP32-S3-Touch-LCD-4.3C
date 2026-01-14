idf_component_register(SRCS "src/ui_app.c"
                       INCLUDE_DIRS "include"
                       REQUIRES lvgl lvgl_port
                       PRIV_REQUIRES net)
