/*
 * PIXEL - Main Screen Implementation
 * 
 * Central pet display with decorative background, status bars,
 * and top/bottom button bands.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_api.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>

// Screen references
static lv_obj_t* g_main_screen = NULL;
static lv_obj_t* g_active_screen = NULL;
static lv_obj_t* g_label_uptime = NULL;
static lv_obj_t* g_bg_img = NULL;
static uint8_t g_bg_index = 1;

// Forward declarations
static void on_wifi_btn_click(lv_event_t* e);
static void on_ble_btn_click(lv_event_t* e);
static void on_settings_btn_click(lv_event_t* e);
static void on_ok_btn_click(lv_event_t* e);
static void on_menu_btn_click(lv_event_t* e);
static void update_uptime_cb(lv_timer_t* timer);
static void wallpaper_timer_cb(lv_timer_t* timer);

lv_obj_t* ui_create_main_screen(void)
{
  // Create main screen container
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BG_MAIN), 0);
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
  lv_obj_set_style_bg_color(band_top, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_bg_opa(band_top, LV_OPA_30, 0);
  lv_obj_set_style_border_width(band_top, 0, 0);
  lv_obj_set_style_pad_all(band_top, PAD_SMALL, 0);
  lv_obj_set_flex_flow(band_top, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(band_top, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  // WiFi button
  lv_obj_t* btn_wifi = lv_btn_create(band_top);
  lv_obj_set_size(btn_wifi, 40, 30);
  lv_obj_add_style(btn_wifi, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_wifi, on_wifi_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_wifi = lv_label_create(btn_wifi);
  lv_label_set_text(label_wifi, "WiFi");
  lv_obj_center(label_wifi);
  lv_obj_add_style(label_wifi, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_wifi, lv_color_hex(COLOR_TEXT_PRIMARY), 0);

  // BLE button
  lv_obj_t* btn_ble = lv_btn_create(band_top);
  lv_obj_set_size(btn_ble, 40, 30);
  lv_obj_add_style(btn_ble, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_ble, on_ble_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_ble = lv_label_create(btn_ble);
  lv_label_set_text(label_ble, "BLE");
  lv_obj_center(label_ble);
  lv_obj_add_style(label_ble, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_ble, lv_color_hex(COLOR_TEXT_PRIMARY), 0);

  // Settings button
  lv_obj_t* btn_settings = lv_btn_create(band_top);
  lv_obj_set_size(btn_settings, 40, 30);
  lv_obj_add_style(btn_settings, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_settings, on_settings_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_settings = lv_label_create(btn_settings);
  lv_label_set_text(label_settings, LV_SYMBOL_SETTINGS);
  lv_obj_center(label_settings);
  lv_obj_add_style(label_settings, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_settings, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  
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
    lv_style_set_bg_color(&label_status_bg_style, lv_color_hex(0x000000));
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
  lv_obj_set_style_text_color(label_status, lv_color_hex(0x00FF00), 0);
  
  // === BOTTOM BUTTON BAND ===
  int band_bottom_y = LV_VER_RES - BAND_HEIGHT;
  
  lv_obj_t* band_bottom = lv_obj_create(scr);
  lv_obj_set_size(band_bottom, LV_HOR_RES, BAND_HEIGHT);
  lv_obj_set_pos(band_bottom, 0, band_bottom_y);
  lv_obj_set_style_bg_color(band_bottom, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_style_bg_opa(band_bottom, LV_OPA_30, 0);
  lv_obj_set_style_border_width(band_bottom, 0, 0);
  lv_obj_set_style_pad_all(band_bottom, PAD_SMALL, 0);
  lv_obj_set_flex_flow(band_bottom, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(band_bottom, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  // OK button
  lv_obj_t* btn_ok = lv_btn_create(band_bottom);
  lv_obj_set_size(btn_ok, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(btn_ok, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_ok, on_ok_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_ok = lv_label_create(btn_ok);
  lv_label_set_text(label_ok, "OK");
  lv_obj_center(label_ok);
  lv_obj_add_style(label_ok, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_ok, lv_color_hex(COLOR_TEXT_PRIMARY), 0);

  // Uptime label
  g_label_uptime = lv_label_create(band_bottom);
  lv_label_set_text(g_label_uptime, "UP: 00:00:00");
  lv_obj_add_style(g_label_uptime, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_uptime, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  lv_obj_set_style_text_align(g_label_uptime, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(g_label_uptime);

  // Menu button
  lv_obj_t* btn_menu = lv_btn_create(band_bottom);
  lv_obj_set_size(btn_menu, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(btn_menu, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(btn_menu, on_menu_btn_click, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t* label_menu = lv_label_create(btn_menu);
  lv_label_set_text(label_menu, "Menu");
  lv_obj_center(label_menu);
  lv_obj_add_style(label_menu, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_menu, lv_color_hex(COLOR_TEXT_PRIMARY), 0);
  
  g_main_screen = scr;
  g_active_screen = scr;

  lv_timer_create(update_uptime_cb, 1000, NULL);
  lv_timer_create(wallpaper_timer_cb, 30000, NULL);

  printf("PIXEL: Main screen created\n");
  return scr;
}

// Button callbacks
static void on_wifi_btn_click(lv_event_t* e)
{
  (void)e;
  printf("PIXEL: WiFi button clicked\n");
  
  // Post UI event to queue (if available)
  extern QueueHandle_t ui_event_queue;
  if (ui_event_queue) {
    ui_event_t event = UI_EVENT_BUTTON_WIFI;
    xQueueSend(ui_event_queue, &event, 0);
  }
}

static void on_ble_btn_click(lv_event_t* e)
{
  (void)e;
  printf("PIXEL: BLE button clicked\n");
  
  extern QueueHandle_t ui_event_queue;
  if (ui_event_queue) {
    ui_event_t event = UI_EVENT_BUTTON_BLE;
    xQueueSend(ui_event_queue, &event, 0);
  }
}

static void on_settings_btn_click(lv_event_t* e)
{
  (void)e;
  printf("PIXEL: Settings button clicked\n");
}

static void on_ok_btn_click(lv_event_t* e)
{
  (void)e;
  printf("PIXEL: OK button clicked\n");
}

static void on_menu_btn_click(lv_event_t* e)
{
  (void)e;
  printf("PIXEL: Menu button clicked\n");
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
  printf("PIXEL: Changing wallpaper to %s\n", path);
  lv_img_set_src(g_bg_img, path);
}

// Screen management
void ui_load_screen(lv_obj_t* screen)
{
  if (screen == NULL) return;
  
  lv_disp_t* disp = lv_disp_get_default();
  if (disp) {
    lv_disp_load_scr(screen);
    g_active_screen = screen;
    printf("PIXEL: Screen loaded\n");
  }
}

lv_obj_t* ui_get_active_screen(void)
{
  return g_active_screen;
}

