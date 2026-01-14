/*
* LVGL UI ported from 15_UDP_TCP_NTP
* Integration of WiFi, NTP, TCP/UDP with LVGL
* Auto-Connect WiFi after UI Init
*/

#include "src/lvgl_port/lvgl_port.h"       // LVGL porting functions for integration
#include <demos/lv_demos.h>        // LVGL demo headers
#include <stdarg.h> // for va_list, vsnprintf
#include <WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

static const char *TAG = "main";

// ===== WiFi Configuration =====
const char* ssid     = "CQ793";
const char* password = "12345678";

// ===== TCP/UDP Configuration =====
WiFiClient tcpClient;
WiFiUDP udp;
const char* serverIP_Host = "192.168.1.100"; // Target Server IP
const uint16_t tcpPort = 12345;
const uint16_t udpPort = 12346;

// ===== NTP Configuration =====
const char* ntpServer = "ntp.ntsc.ac.cn";
const long gmtOffset_sec = 8 * 3600;   // GMT+8
const int daylightOffset_sec = 0;
TaskHandle_t ntpTaskHandle = NULL;

// ===== UI Variables =====
lv_obj_t *label_time;
lv_obj_t *label_hostip;
lv_obj_t *label_espip;
lv_obj_t *log_box;
lv_obj_t *btn_tcp_send;
lv_obj_t *btn_udp_send;
lv_obj_t *input_box;
lv_obj_t *keyboard;
// lv_obj_t *btn_connect_wifi; // Auto connect, no button needed

// ===== Log Function =====
void ui_log(const char *fmt, ...) {
  static char buf[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  // Ensure log_box exists before using it
  if (log_box && lvgl_port_lock(200)) {
    lv_textarea_add_text(log_box, buf);
    lv_textarea_add_text(log_box, "\n");
    lvgl_port_unlock();
  }
}

// ===== WiFi Connect Task =====
void wifi_connect_task(void *pvParameters) {
  // Initial delay to let UI show up properly
  vTaskDelay(pdMS_TO_TICKS(1000));
  
  if (lvgl_port_lock(200)) {
      ui_log("Auto-Connecting to WiFi...");
      lvgl_port_unlock();
  }

  // LVGL should run during WiFi connection to prevent UI freeze
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait a bit for WiFi stack to stabilize
  vTaskDelay(pdMS_TO_TICKS(200));

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
      retry++;
      Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Connected.");
      if (lvgl_port_lock(500)) {
          lv_label_set_text_fmt(label_espip, "ESP-IP: %s", WiFi.localIP().toString().c_str());
          ui_log("WiFi-Connected: %s", WiFi.localIP().toString().c_str());
          lvgl_port_unlock();
      }
      
      ui_log("[NTP] Starting time sync...");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      if (lvgl_port_lock(500)) {
          lv_label_set_text(label_time, "Time: Syncing...");
          lvgl_port_unlock();
      }
      
      // Wait for time to be set
      retry = 0;
      struct tm timeinfo;
      while (!getLocalTime(&timeinfo) && retry < 10) {
           vTaskDelay(pdMS_TO_TICKS(500));
           retry++;
      }
      
      if (getLocalTime(&timeinfo)) {
          char buf[64];
          strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
          ui_log("[NTP] Time synced: %s", buf);
      } else {
          ui_log("[NTP] Failed to obtain time");
      }
  } else {
      Serial.println("\nWiFi Failed.");
      if (lvgl_port_lock(500)) {
          lv_label_set_text(label_espip, "ESP-IP: Failed");
          ui_log("WiFi connection failed.");
          lvgl_port_unlock();
      }
  }
  
  vTaskDelete(NULL);
}

// ===== Event Callbacks =====

void event_tcp_send(lv_event_t * e) {
  const char *msg = lv_textarea_get_text(input_box);
  ui_log("[TCP] Clicked: %s", msg);
  
  if (WiFi.status() == WL_CONNECTED) {
      if (tcpClient.connect(serverIP_Host, tcpPort)) {
          tcpClient.print(msg);
          tcpClient.stop();
          ui_log("[TCP] Sent to %s:%d", serverIP_Host, tcpPort);
      } else {
          ui_log("[TCP] Connection Failed");
      }
  } else {
      ui_log("[TCP] WiFi Not Connected");
  }
}

void event_udp_send(lv_event_t * e) {
  const char *msg = lv_textarea_get_text(input_box);
  ui_log("[UDP] Clicked: %s", msg);

  if (WiFi.status() == WL_CONNECTED) {
      udp.beginPacket(serverIP_Host, udpPort);
      udp.print(msg);
      udp.endPacket();
      ui_log("[UDP] Sent to %s:%d", serverIP_Host, udpPort);
  } else {
      ui_log("[UDP] WiFi Not Connected");
  }
}

void input_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *textarea = lv_event_get_target(e);
  if (code == LV_EVENT_FOCUSED || code == LV_EVENT_VALUE_CHANGED) {
      if (keyboard) {
          lv_keyboard_set_textarea(keyboard, textarea);
      }
  }
}

// ===== UI Initialization =====
void ui_init() {
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
  lv_label_set_text_fmt(label_hostip, "HOST: %s", serverIP_Host);

  label_espip = lv_label_create(top_row);
  lv_label_set_text(label_espip, "ESP: --------");

  label_time = lv_label_create(top_row);
  lv_label_set_text(label_time, "Time: --:--:--");

  // Input area (left side)
  lv_obj_t *host_title = lv_label_create(lv_scr_act());
  lv_label_set_text(host_title, "USER-INPUT");
  lv_obj_set_style_text_font(host_title, LV_FONT_DEFAULT, 0);
  lv_obj_align(host_title, LV_ALIGN_TOP_LEFT, 10, 60);

  input_box = lv_textarea_create(lv_scr_act());
  lv_obj_set_size(input_box, lv_pct(50), 130);
  lv_obj_align(input_box, LV_ALIGN_TOP_LEFT, 10, 80);
  lv_textarea_set_placeholder_text(input_box, "input_your_message...");
  lv_obj_add_event_cb(input_box, input_event_cb, LV_EVENT_ALL, NULL);

  // Log area (right side)
  lv_obj_t *log_title = lv_label_create(lv_scr_act());
  lv_label_set_text(log_title, "INFO-LOG");
  lv_obj_set_style_text_font(log_title, LV_FONT_DEFAULT, 0);
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
  static lv_style_t style_btn_pressed;
  lv_style_init(&style_btn_pressed);
  lv_style_set_bg_color(&style_btn_pressed, lv_color_make(0, 0, 150));
  lv_obj_add_style(btn_tcp_send, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_t *label_tcp = lv_label_create(btn_tcp_send);
  lv_label_set_text(label_tcp, "TCP_SEND");

  // UDP Button
  btn_udp_send = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn_udp_send, 120, 36);
  lv_obj_align(btn_udp_send, LV_ALIGN_LEFT_MID, 140, 0);
  lv_obj_add_event_cb(btn_udp_send, event_udp_send, LV_EVENT_CLICKED, NULL);
  static lv_style_t style_btn_udp_pressed;
  lv_style_init(&style_btn_udp_pressed);
  lv_style_set_bg_color(&style_btn_udp_pressed, lv_color_make(150, 0, 0));
  lv_obj_add_style(btn_udp_send, &style_btn_udp_pressed, LV_STATE_PRESSED);
  lv_obj_t *label_udp = lv_label_create(btn_udp_send);
  lv_label_set_text(label_udp, "UDP_SEND");

  // Clear Log Button
  lv_obj_t *btn_clear_log = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn_clear_log, 120, 36);
  lv_obj_align(btn_clear_log, LV_ALIGN_LEFT_MID, 270, 0);
  lv_obj_add_event_cb(btn_clear_log, [](lv_event_t *e){
      lv_textarea_set_text(log_box, "");
  }, LV_EVENT_CLICKED, NULL);
  static lv_style_t style_btn_clear;
  lv_style_init(&style_btn_clear);
  lv_style_set_bg_color(&style_btn_clear, lv_color_make(150, 0, 0));
  lv_obj_add_style(btn_clear_log, &style_btn_clear, LV_STATE_PRESSED);
  lv_obj_t *label_clear = lv_label_create(btn_clear_log);
  lv_label_set_text(label_clear, "CLEAR LOG");

  // Keyboard
  keyboard = lv_keyboard_create(lv_scr_act());
  lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
  lv_obj_set_size(keyboard, lv_pct(100), 210);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_keyboard_set_textarea(keyboard, input_box);
}

void setup() {
    static esp_lcd_panel_handle_t panel_handle = NULL;
    static esp_lcd_touch_handle_t tp_handle = NULL;
    
    Serial.begin(115200);
    DEV_I2C_Init();
    IO_EXTENSION_Init();

    // Initialize the GT911 touch screen controller
    tp_handle = touch_gt911_init(DEV_I2C_Get_Bus_Device());  
    
    // Initialize the Waveshare ESP32-S3 RGB LCD hardware
    panel_handle = waveshare_esp32_s3_rgb_lcd_init(); 

    // Turn on the LCD backlight
    wavesahre_rgb_lcd_bl_on();   

    // Initialize LVGL with the panel and touch handles
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    Serial.println("Display UI");

    // Lock the mutex because LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
         ui_init();
         ui_log("System Initialized. Waiting for WiFi...");
         lvgl_port_unlock();
    }

    // Auto-connect task
    xTaskCreate(wifi_connect_task, "WiFiConnect", 8192, NULL, 3, &ntpTaskHandle);
}

void loop() {
  // Update time
  struct tm timeinfo;
  if (WiFi.status() == WL_CONNECTED && getLocalTime(&timeinfo)) {
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    if (lvgl_port_lock(100)) {
      lv_label_set_text_fmt(label_time, "Time: %s", buf);
      lvgl_port_unlock();
    }
  }
  delay(1000);
}
