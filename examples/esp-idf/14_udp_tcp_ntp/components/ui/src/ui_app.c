#include "ui_app.h"
#include "lvgl_port.h"
#include "net.h"
#include <stdio.h>
#include <stdarg.h>

static lv_obj_t *label_time;
static lv_obj_t *label_hostip;
static lv_obj_t *label_espip;
static lv_obj_t *log_box;
static lv_obj_t *btn_tcp_send;
static lv_obj_t *btn_udp_send;
static lv_obj_t *input_box;
static lv_obj_t *keyboard;

static char buf[128];
void ui_log(const char *fmt, ...) {

    if (!log_box) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (lvgl_port_lock(50)) {
        lv_textarea_add_text(log_box, buf);
        lv_textarea_add_text(log_box, "\n");
        lvgl_port_unlock();
    }
}

static void event_tcp_send(lv_event_t * e) {
    const char *msg = lv_textarea_get_text(input_box);
    ui_log("[TCP] Clicked: %s", msg);
    app_send_tcp(msg);
}

static void event_udp_send(lv_event_t * e) {
    const char *msg = lv_textarea_get_text(input_box);
    ui_log("[UDP] Clicked: %s", msg);
    app_send_udp(msg);
}

static void event_clear_log(lv_event_t *e) {
    lv_textarea_set_text(log_box, "");
}

static void input_event_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *textarea = lv_event_get_target(e);
    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED) {
        if (keyboard) {
            if (lvgl_port_lock(0)) {
                lv_keyboard_set_textarea(keyboard, textarea);
                lvgl_port_unlock();
            }
        }
    }
}

void ui_init(void) {
    if (!lvgl_port_lock(2000)) {
        return;
    }

    // Top row: Time, HOST-IP, ESP-IP
    lv_obj_t *top_row = lv_obj_create(lv_scr_act());
    lv_obj_set_size(top_row, lv_pct(100), 40);
    lv_obj_align(top_row, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_clear_flag(top_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_column(top_row, 50, 0);
    lv_obj_set_style_pad_left(top_row, 50, 0);
    lv_obj_set_style_pad_right(top_row, 50, 0);

    label_hostip = lv_label_create(top_row);
    lv_label_set_text_fmt(label_hostip, "HOST: %s", app_get_server_ip());

    label_espip = lv_label_create(top_row);
    lv_label_set_text(label_espip, "ESP: --------");

    label_time = lv_label_create(top_row);
    lv_label_set_text(label_time, "Time: --:--:--");

    // Input area
    lv_obj_t *host_title = lv_label_create(lv_scr_act());
    lv_label_set_text(host_title, "USER-INPUT");
    lv_obj_align(host_title, LV_ALIGN_TOP_LEFT, 10, 60);

    input_box = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(input_box, lv_pct(50), 130);
    lv_obj_align(input_box, LV_ALIGN_TOP_LEFT, 10, 80);
    lv_textarea_set_placeholder_text(input_box, "input_your_message...");
    lv_obj_add_event_cb(input_box, input_event_cb, LV_EVENT_ALL, NULL);

    // Log area
    lv_obj_t *log_title = lv_label_create(lv_scr_act());
    lv_label_set_text(log_title, "INFO-LOG");
    lv_obj_align(log_title, LV_ALIGN_TOP_RIGHT, -310, 60);

    log_box = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(log_box, lv_pct(47), 170);
    lv_obj_align(log_box, LV_ALIGN_TOP_RIGHT, -10, 80);
    lv_textarea_set_max_length(log_box, 1024);
    lv_obj_add_flag(log_box, LV_OBJ_FLAG_SCROLLABLE);

    // TCP Button
    btn_tcp_send = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_tcp_send, 120, 36);
    lv_obj_align(btn_tcp_send, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_event_cb(btn_tcp_send, event_tcp_send, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_tcp = lv_label_create(btn_tcp_send);
    lv_label_set_text(label_tcp, "TCP_SEND");

    // UDP Button
    btn_udp_send = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_udp_send, 120, 36);
    lv_obj_align(btn_udp_send, LV_ALIGN_LEFT_MID, 140, 0);
    lv_obj_add_event_cb(btn_udp_send, event_udp_send, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_udp = lv_label_create(btn_udp_send);
    lv_label_set_text(label_udp, "UDP_SEND");

    // Clear Log Button
    lv_obj_t *btn_clear_log = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_clear_log, 120, 36);
    lv_obj_align(btn_clear_log, LV_ALIGN_LEFT_MID, 270, 0);
    lv_obj_add_event_cb(btn_clear_log, event_clear_log, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_clear = lv_label_create(btn_clear_log);
    lv_label_set_text(label_clear, "CLEAR LOG");

    // Keyboard
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_set_size(keyboard, lv_pct(100), 210);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(keyboard, input_box);

    lvgl_port_unlock();
}

bool ui_update_ip(const char *ip) {
    if (!label_espip) {
        return false;
    }
    if (!lvgl_port_lock(50)) {
        return false;
    }
    lv_label_set_text_fmt(label_espip, "ESP-IP: %s", ip);
    lvgl_port_unlock();
    return true;
}

void ui_update_time(const char *time_str) {
    if (!label_time) {
        return;
    }
    if (!lvgl_port_lock(50)) {
        return;
    }
    lv_label_set_text_fmt(label_time, "Time: %s", time_str);
    lvgl_port_unlock();
}
