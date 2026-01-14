#pragma once

#include "lvgl.h"

void ui_init(void);
void ui_log(const char *fmt, ...);
bool ui_update_ip(const char *ip);
void ui_update_time(const char *time_str);

// External functions to be implemented by main application
void app_send_tcp(const char *msg);
void app_send_udp(const char *msg);
const char* app_get_server_ip(void);
