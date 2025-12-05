/*
 * PIXEL - Main Screen Implementation
 * 
 * Central pet display with decorative background, status bars,
 * and top/bottom button bands.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_api.h"
#include "archi/archi_stats.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>

static lv_obj_t* g_label_uptime = NULL;
static lv_obj_t* g_bg_img = NULL;
static uint8_t g_bg_index = 1;
static lv_obj_t* g_bottom_band = NULL;
static lv_obj_t* g_bottom_button = NULL;
static lv_obj_t* g_bottom_button_label = NULL;
static lv_timer_t* g_uptime_timer = NULL;
static lv_timer_t* g_wallpaper_timer = NULL;

static ui_screen_id_t g_screen_state = UI_SCREEN_MAIN;
static lv_event_cb_t g_bottom_button_handler = NULL;

// Forward declarations
static void on_wifi_btn_click(lv_event_t* e);
static void on_ble_btn_click(lv_event_t* e);
static void on_monitor_btn_click(lv_event_t* e);
static void on_menu_btn_click(lv_event_t* e);
static void on_back_btn_click(lv_event_t* e);
static void update_uptime_cb(lv_timer_t* timer);
static void wallpaper_timer_cb(lv_timer_t* timer);
static void update_bottom_button(const char* label, lv_event_cb_t handler);
static void dispatch_bottom_button(lv_event_t* e);
static void apply_bottom_button_state(void);
static void on_main_screen_delete(lv_event_t* e);

lv_obj_t* ui_create_main_screen(void)
{
  archi_log_heap(" [UI] main_screen build");

  if (g_bottom_band) {
    lv_obj_del(g_bottom_band);
    g_bottom_band = NULL;
    g_bottom_button = NULL;
    g_bottom_button_label = NULL;
    g_bottom_button_handler = NULL;
  }

  if (g_uptime_timer) {
    lv_timer_del(g_uptime_timer);
    g_uptime_timer = NULL;
  }

  if (g_wallpaper_timer) {
    lv_timer_del(g_wallpaper_timer);
    g_wallpaper_timer = NULL;
  }

  // Create main screen container
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // Background image (binary format stored on S: driver)
  g_bg_img = lv_img_create(scr);
  lv_img_set_src(g_bg_img, "S:/img/bg_1.bin");
  lv_obj_set_pos(g_bg_img, 0, 0);

  // === TOP BUTTON BAND ===
  lv_obj_t* band_top = lv_obj_create(scr);
  lv_obj_set_size(band_top, LV_HOR_RES, BAND_HEIGHT);
  lv_obj_set_pos(band_top, 0, 0);
  lv_obj_set_style_bg_color(band_top, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(band_top, LV_OPA_30, 0);
  lv_obj_set_style_border_width(band_top, 0, 0);
  lv_obj_set_style_pad_all(band_top, PAD_SMALL, 0);
  lv_obj_set_flex_flow(band_top, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(band_top, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  // WiFi button
  lv_obj_t* btn_wifi = lv_btn_create(band_top);
  lv_obj_set_size(btn_wifi, 40, 30);
  lv_obj_add_style(btn_wifi, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_wifi, on_wifi_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_wifi = lv_label_create(btn_wifi);
  lv_label_set_text(label_wifi, "WiFi");
  lv_obj_center(label_wifi);
  lv_obj_add_style(label_wifi, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_wifi, lv_color_hex(COLOR_TEXT), 0);

  // BLE button
  lv_obj_t* btn_ble = lv_btn_create(band_top);
  lv_obj_set_size(btn_ble, 40, 30);
  lv_obj_add_style(btn_ble, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_ble, on_ble_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_ble = lv_label_create(btn_ble);
  lv_label_set_text(label_ble, "BLE");
  lv_obj_center(label_ble);
  lv_obj_add_style(label_ble, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_ble, lv_color_hex(COLOR_TEXT), 0);

  // Monitor button
  lv_obj_t* btn_monitor = lv_btn_create(band_top);
  lv_obj_set_size(btn_monitor, 50, 30);
  lv_obj_add_style(btn_monitor, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_monitor, on_monitor_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_monitor = lv_label_create(btn_monitor);
  lv_label_set_text(label_monitor, "SYS");
  lv_obj_center(label_monitor);
  lv_obj_add_style(label_monitor, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_monitor, lv_color_hex(COLOR_TEXT), 0);
  
  // === CENTRAL PET AREA ===
  int pet_start_y = BAND_HEIGHT + PAD_LARGE;

  // Pet label floating on black background
  lv_obj_t* label_pet = lv_label_create(scr);
  lv_label_set_text(label_pet, "( 0_0 )");
  lv_obj_set_width(label_pet, LV_HOR_RES);
  lv_obj_set_style_text_align(label_pet, LV_TEXT_ALIGN_CENTER, 0);
  // Swap to a monospace font here if enabled in lv_conf.h for perfect ASCII alignment.
  lv_obj_set_style_text_font(label_pet, &lv_font_unscii_16, 0);
  lv_obj_add_style(label_pet, ui_get_style_label_title(), 0);
  lv_obj_align(label_pet, LV_ALIGN_TOP_MID, 0, pet_start_y);

  // Status info bar (pet name, health, etc.)
  int status_y = pet_start_y + MAIN_SCREEN_PET_SIZE + PAD_NORMAL;

  static lv_style_t label_status_bg_style;
  static bool label_status_bg_style_inited = false;
  if (!label_status_bg_style_inited) {
    lv_style_init(&label_status_bg_style);
    lv_style_set_bg_color(&label_status_bg_style, lv_color_hex(COLOR_BACKGROUND));
    lv_style_set_bg_opa(&label_status_bg_style, LV_OPA_30);
    lv_style_set_pad_left(&label_status_bg_style, PAD_SMALL);
    lv_style_set_pad_right(&label_status_bg_style, PAD_SMALL);
    lv_style_set_pad_top(&label_status_bg_style, PAD_TINY);
    lv_style_set_pad_bottom(&label_status_bg_style, PAD_TINY);
    label_status_bg_style_inited = true;
  }

  lv_obj_t* label_status = lv_label_create(scr);
  lv_label_set_text(label_status, "Acyd | Health: 100% | Mood: :) ");
  lv_obj_set_pos(label_status, PAD_NORMAL, status_y);
  lv_obj_set_width(label_status, LV_HOR_RES - 2 * PAD_NORMAL);
  lv_label_set_long_mode(label_status, LV_LABEL_LONG_WRAP);
  lv_obj_add_style(label_status, ui_get_style_label_normal(), 0);
  lv_obj_add_style(label_status, &label_status_bg_style, 0);
  lv_obj_set_style_text_color(label_status, lv_color_hex(COLOR_TEXT), 0);
  
  // === BOTTOM BUTTON BAND ===
  int band_bottom_y = LV_VER_RES - BAND_HEIGHT;
  
  g_bottom_band = lv_obj_create(lv_layer_top());
  lv_obj_set_size(g_bottom_band, LV_HOR_RES, BAND_HEIGHT);
  lv_obj_set_pos(g_bottom_band, 0, band_bottom_y);
  lv_obj_set_style_bg_color(g_bottom_band, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(g_bottom_band, LV_OPA_30, 0);
  lv_obj_set_style_border_width(g_bottom_band, 0, 0);
  lv_obj_set_style_pad_all(g_bottom_band, PAD_SMALL, 0);
  lv_obj_set_flex_flow(g_bottom_band, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_bottom_band, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(g_bottom_band, LV_OBJ_FLAG_SCROLLABLE);

  // Uptime label
  g_label_uptime = lv_label_create(g_bottom_band);
  lv_label_set_text(g_label_uptime, "UP: 00:00:00");
  lv_label_set_long_mode(g_label_uptime, LV_LABEL_LONG_CLIP);
  lv_obj_add_style(g_label_uptime, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_uptime, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_set_style_text_align(g_label_uptime, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_style_pad_left(g_label_uptime, PAD_SMALL, 0);
  lv_obj_set_width(g_label_uptime, LV_SIZE_CONTENT);
  lv_obj_set_flex_grow(g_label_uptime, 1);

  // Menu button
  g_bottom_button = lv_btn_create(g_bottom_band);
  lv_obj_set_size(g_bottom_button, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(g_bottom_button, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(g_bottom_button, dispatch_bottom_button, LV_EVENT_CLICKED, NULL);

  g_bottom_button_label = lv_label_create(g_bottom_button);
  lv_obj_center(g_bottom_button_label);
  lv_obj_add_style(g_bottom_button_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_bottom_button_label, lv_color_hex(COLOR_TEXT), 0);
  update_bottom_button("Menu", on_menu_btn_click);

  g_uptime_timer = lv_timer_create(update_uptime_cb, 1000, NULL);
  g_wallpaper_timer = lv_timer_create(wallpaper_timer_cb, 30000, NULL);

  lv_obj_add_event_cb(scr, on_main_screen_delete, LV_EVENT_DELETE, NULL);

  Serial.println("PIXEL: Main screen created");
  return scr;
}

// Button callbacks
static void on_wifi_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: WiFi button clicked");
  ui_post_event(UI_EVENT_BUTTON_WIFI);
}

static void on_ble_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE button clicked");
  ui_post_event(UI_EVENT_BUTTON_BLE);
}

static void on_monitor_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: Monitor button clicked");
  ui_post_event(UI_EVENT_BUTTON_MONITOR);
}

static void on_menu_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: Menu button clicked");
  ui_post_event(UI_EVENT_BUTTON_MENU);
}

static void on_back_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: Back button clicked");
  ui_post_event(UI_EVENT_BACK);
}

static void update_uptime_cb(lv_timer_t* timer)
{
  (void)timer;

  if (!g_label_uptime) return;

  uint32_t elapsed_ms = millis();
  uint32_t total_seconds = elapsed_ms / 1000;
  uint32_t hours = total_seconds / 3600;
  uint32_t minutes = (total_seconds % 3600) / 60;
  uint32_t seconds = total_seconds % 60;

  lv_label_set_text_fmt(g_label_uptime, "UP: %02lu:%02lu:%02lu", hours, minutes, seconds);
}

static void wallpaper_timer_cb(lv_timer_t* timer)
{
  (void)timer;

  if (!g_bg_img) return;

  g_bg_index++;
  if (g_bg_index > 6) {
    g_bg_index = 1;
  }

  char path[32];
  sprintf(path, "S:/img/bg_%d.bin", g_bg_index);
  Serial.printf("PIXEL: Changing wallpaper to %s\n", path);
  lv_img_set_src(g_bg_img, path);
}

// Screen management
static void update_bottom_button(const char* label, lv_event_cb_t handler)
{
  if (!g_bottom_button_label) return;

  lv_label_set_text(g_bottom_button_label, label);
  g_bottom_button_handler = handler;
}

static void dispatch_bottom_button(lv_event_t* e)
{
  if (g_bottom_button_handler) {
    g_bottom_button_handler(e);
  }
}

static void apply_bottom_button_state(void)
{
  switch (g_screen_state) {
    case UI_SCREEN_MAIN:
      update_bottom_button("Menu", on_menu_btn_click);
      break;
    case UI_SCREEN_WIFI:
    case UI_SCREEN_BLE:
    case UI_SCREEN_SETTINGS:
    case UI_SCREEN_MONITOR:
      update_bottom_button("Back", on_back_btn_click);
      break;
  }
}

void ui_bottom_button_set(const char* label, lv_event_cb_t handler)
{
  update_bottom_button(label ? label : "", handler);
}

void ui_bottom_button_restore(void)
{
  apply_bottom_button_state();
}

void ui_set_bottom_bar_for_screen(ui_screen_id_t screen_id)
{
  g_screen_state = screen_id;
  apply_bottom_button_state();
}

static void on_main_screen_delete(lv_event_t* e)
{
  (void)e;

  g_bg_img = NULL;
  g_label_uptime = NULL;

  if (g_uptime_timer) {
    lv_timer_del(g_uptime_timer);
    g_uptime_timer = NULL;
  }

  if (g_wallpaper_timer) {
    lv_timer_del(g_wallpaper_timer);
    g_wallpaper_timer = NULL;
  }

  archi_log_heap(" [UI] main_screen delete");
}
