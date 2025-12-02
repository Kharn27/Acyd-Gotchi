/*
 * PIXEL - BLE Scan Screen
 *
 * Displays list of detected BLE devices with a two-state top band
 * (idle scan button then duration selection) and a scrollable list
 * with an empty state message.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_api.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>

static lv_obj_t* g_ble_screen = NULL;
static lv_obj_t* g_ble_scan_button = NULL;
static lv_obj_t* g_duration_container = NULL;
static lv_obj_t* g_idle_container = NULL;
static lv_obj_t* g_device_list = NULL;
static lv_obj_t* g_empty_label = NULL;
static lv_obj_t* g_status_label = NULL;
static lv_obj_t* g_finish_button = NULL;

static void on_scan_btn_click(lv_event_t* e);
static void on_duration_btn_click(lv_event_t* e);
static void on_cancel_btn_click(lv_event_t* e);
static void on_finish_btn_click(lv_event_t* e);
static void show_duration_choices(bool show);
static void clear_device_list(void);
static void add_device_row(const char* name, const char* mac, int rssi);
static void refresh_empty_state(void);
static void align_empty_label(void);

lv_obj_t* ui_create_ble_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // === TOP BUTTON BAND ===
  lv_obj_t* band_top = lv_obj_create(scr);
  lv_obj_set_size(band_top, LV_HOR_RES, BAND_HEIGHT);
  lv_obj_set_pos(band_top, 0, 0);
  lv_obj_set_style_bg_color(band_top, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(band_top, LV_OPA_30, 0);
  lv_obj_set_style_border_width(band_top, 0, 0);
  lv_obj_set_style_pad_all(band_top, PAD_SMALL, 0);
  lv_obj_set_flex_flow(band_top, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(band_top, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Screen title within the top band
  lv_obj_t* title = lv_label_create(band_top);
  lv_label_set_text(title, "BLE");
  lv_obj_add_style(title, ui_get_style_label_title(), 0);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_set_style_pad_left(title, PAD_SMALL, 0);
  lv_obj_set_flex_grow(title, 1);

  // Idle state container (Scan button)
  g_idle_container = lv_obj_create(band_top);
  lv_obj_set_style_bg_opa(g_idle_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(g_idle_container, 0, 0);
  lv_obj_set_flex_flow(g_idle_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_idle_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(g_idle_container, 0, 0);
  lv_obj_set_size(g_idle_container, LV_SIZE_CONTENT, LV_PCT(100));

  g_ble_scan_button = lv_btn_create(g_idle_container);
  lv_obj_set_size(g_ble_scan_button, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(g_ble_scan_button, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(g_ble_scan_button, on_scan_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_scan = lv_label_create(g_ble_scan_button);
  lv_label_set_text(label_scan, "SCAN");
  lv_obj_center(label_scan);
  lv_obj_add_style(label_scan, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_scan, lv_color_hex(COLOR_TEXT), 0);

  // Duration selection container (10/20/30s)
  g_duration_container = lv_obj_create(band_top);
  lv_obj_set_style_bg_opa(g_duration_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(g_duration_container, 0, 0);
  lv_obj_set_flex_flow(g_duration_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_duration_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(g_duration_container, PAD_SMALL, 0);
  lv_obj_set_style_pad_all(g_duration_container, 0, 0);
  lv_obj_set_size(g_duration_container, LV_SIZE_CONTENT, LV_PCT(100));
  lv_obj_add_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);

  const int durations[] = {10, 20, 30};
  for (uint8_t i = 0; i < sizeof(durations) / sizeof(durations[0]); i++) {
    lv_obj_t* btn = lv_btn_create(g_duration_container);
    lv_obj_set_size(btn, 50, BUTTON_HEIGHT);
    lv_obj_add_style(btn, ui_get_style_btn_primary(), 0);
    lv_obj_add_event_cb(btn, on_duration_btn_click, LV_EVENT_CLICKED, (void*)(intptr_t)durations[i]);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text_fmt(label, "%ds", durations[i]);
    lv_obj_center(label);
    lv_obj_add_style(label, ui_get_style_label_normal(), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);
  }

  lv_obj_t* cancel_btn = lv_btn_create(g_duration_container);
  lv_obj_set_size(cancel_btn, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(cancel_btn, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(cancel_btn, on_cancel_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* cancel_label = lv_label_create(cancel_btn);
  lv_label_set_text(cancel_label, "CANCEL");
  lv_obj_center(cancel_label);
  lv_obj_add_style(cancel_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(cancel_label, lv_color_hex(COLOR_TEXT), 0);

  // Title for list
  lv_obj_t* list_title = lv_label_create(scr);
  lv_label_set_text(list_title, "BLE Devices");
  lv_obj_set_pos(list_title, PAD_NORMAL, BAND_HEIGHT + PAD_SMALL);
  lv_obj_add_style(list_title, ui_get_style_label_title(), 0);

  // Scrollable list container
  g_device_list = lv_obj_create(scr);
  lv_obj_set_size(g_device_list, LV_HOR_RES - 2 * PAD_NORMAL,
                  LV_VER_RES - BAND_HEIGHT - PAD_LARGE - PAD_NORMAL);
  lv_obj_set_pos(g_device_list, PAD_NORMAL, BAND_HEIGHT + PAD_LARGE);
  lv_obj_set_style_bg_color(g_device_list, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(g_device_list, LV_OPA_20, 0);
  lv_obj_set_style_border_width(g_device_list, 0, 0);
  lv_obj_set_style_pad_all(g_device_list, PAD_SMALL, 0);
  lv_obj_set_style_pad_gap(g_device_list, PAD_SMALL, 0);
  lv_obj_set_style_radius(g_device_list, RADIUS_NORMAL, 0);
  lv_obj_set_scroll_dir(g_device_list, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(g_device_list, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_set_flex_flow(g_device_list, LV_FLEX_FLOW_COLUMN);

  // Empty state label
  g_empty_label = lv_label_create(scr);
  lv_label_set_text(g_empty_label, "No BLE devices found.");
  lv_obj_add_style(g_empty_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_empty_label, lv_color_hex(COLOR_TEXT), 0);
  align_empty_label();

  // Status text
  g_status_label = lv_label_create(scr);
  lv_label_set_text(g_status_label, "Tap Scan to search for BLE devices.");
  lv_obj_add_style(g_status_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_status_label, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_set_pos(g_status_label, PAD_NORMAL, LV_VER_RES - PAD_LARGE);

  g_finish_button = lv_btn_create(scr);
  lv_obj_set_size(g_finish_button, BUTTON_WIDTH, BUTTON_HEIGHT);
  lv_obj_add_style(g_finish_button, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(g_finish_button, on_finish_btn_click, LV_EVENT_CLICKED, NULL);
  lv_obj_set_pos(g_finish_button, LV_HOR_RES - BUTTON_WIDTH - PAD_NORMAL, LV_VER_RES - BAND_HEIGHT - BUTTON_HEIGHT);

  lv_obj_t* finish_label = lv_label_create(g_finish_button);
  lv_label_set_text(finish_label, "FINISH");
  lv_obj_center(finish_label);
  lv_obj_add_style(finish_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(finish_label, lv_color_hex(COLOR_TEXT), 0);

  g_ble_screen = scr;
  refresh_empty_state();
  Serial.println("PIXEL: BLE screen created");
  return scr;
}

lv_obj_t* ui_ble_get_scan_button(void)
{
  return g_ble_scan_button;
}

static void on_scan_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE scan button clicked");
  show_duration_choices(true);
  ui_post_event(UI_EVENT_SCAN_START);
  if (g_status_label) {
    lv_label_set_text(g_status_label, "Choose scan duration.");
  }
}

static void on_duration_btn_click(lv_event_t* e)
{
  int duration_s = (int)(intptr_t)lv_event_get_user_data(e);
  Serial.printf("PIXEL: BLE scan %ds requested\n", duration_s);

  show_duration_choices(false);
  clear_device_list();

  // Placeholder data to visualize results; real scan callback should populate rows.
  add_device_row("Keyboard", "AA:BB:CC:DD:EE:01", -55);
  add_device_row("Headphones", "AA:BB:CC:DD:EE:02", -68);
  add_device_row("Beacon", "AA:BB:CC:DD:EE:03", -80);

  if (g_status_label) {
    lv_label_set_text_fmt(g_status_label, "Scanning for %d s...", duration_s);
  }

  switch (duration_s) {
    case 10:
      ui_post_event(UI_EVENT_SCAN_DURATION_10S);
      break;
    case 20:
      ui_post_event(UI_EVENT_SCAN_DURATION_20S);
      break;
    case 30:
      ui_post_event(UI_EVENT_SCAN_DURATION_30S);
      break;
    default:
      break;
  }
}

static void on_cancel_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE scan canceled by user");
  show_duration_choices(false);
  if (g_status_label) {
    lv_label_set_text(g_status_label, "Scan canceled.");
  }
  ui_post_event(UI_EVENT_SCAN_CANCEL);
}

static void on_finish_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE scan finished acknowledgment");
  ui_post_event(UI_EVENT_SCAN_FINISHED);
}

static void show_duration_choices(bool show)
{
  if (!g_duration_container || !g_idle_container) return;

  if (show) {
    lv_obj_add_flag(g_idle_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(g_idle_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);
  }
}

static void clear_device_list(void)
{
  if (!g_device_list) return;

  lv_obj_clean(g_device_list);
  refresh_empty_state();
}

static void add_device_row(const char* name, const char* mac, int rssi)
{
  if (!g_device_list) return;

  lv_obj_t* row = lv_obj_create(g_device_list);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(row, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(row, LV_OPA_40, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_radius(row, RADIUS_SMALL, 0);
  lv_obj_set_style_pad_all(row, PAD_SMALL, 0);

  lv_obj_t* label = lv_label_create(row);
  lv_obj_add_style(label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text_fmt(label, "%s\n%s\nRSSI: %d dBm", name, mac, rssi);
  lv_obj_set_width(label, LV_PCT(100));
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

  refresh_empty_state();
}

static void refresh_empty_state(void)
{
  if (!g_empty_label || !g_device_list) return;

  bool has_entries = lv_obj_get_child_cnt(g_device_list) > 0;
  if (has_entries) {
    lv_obj_add_flag(g_empty_label, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(g_empty_label, LV_OBJ_FLAG_HIDDEN);
  }
}

static void align_empty_label(void)
{
  if (!g_empty_label || !g_device_list) return;

  lv_obj_align_to(g_empty_label, g_device_list, LV_ALIGN_CENTER, 0, 0);
}

