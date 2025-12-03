/*
 * PIXEL - BLE Scan Screen
 *
 * Displays list of detected BLE devices with a three-state top band
 * (idle scan button, duration selection, live scanning banner) and a
 * scrollable list with an empty state message.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_api.h"
#include "netsec_api.h"
#include "netsec/netsec_ble.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static lv_obj_t* g_ble_screen = NULL;
static lv_obj_t* g_ble_scan_button = NULL;
static lv_obj_t* g_duration_container = NULL;
static lv_obj_t* g_idle_container = NULL;
static lv_obj_t* g_scanning_container = NULL;
static lv_obj_t* g_scanning_spinner = NULL;
static lv_obj_t* g_device_list = NULL;
static lv_obj_t* g_empty_label = NULL;
static lv_obj_t* g_status_label = NULL;
static lv_timer_t* g_scan_timer = NULL;
static lv_obj_t* g_duration_buttons[3] = {NULL};
static bool g_has_scanned = false;

typedef struct {
  bool in_use;
  uint8_t mac_bytes[6];
  lv_obj_t* row;
  lv_obj_t* label;
} ble_device_entry_t;

static ble_device_entry_t g_device_entries[NETSEC_BLE_DEVICE_BUFFER_SIZE];
static uint32_t g_scan_remaining_ms = 0;
static bool g_scan_active = false;
static uint32_t g_last_duration_ms = 0;

typedef enum {
  TOP_STATE_IDLE = 0,
  TOP_STATE_DURATION,
  TOP_STATE_SCANNING,
} top_band_state_t;
static top_band_state_t g_top_state = TOP_STATE_IDLE;

static void on_scan_btn_click(lv_event_t* e);
static void on_duration_btn_click(lv_event_t* e);
static void on_cancel_btn_click(lv_event_t* e);
static void set_top_band_state(top_band_state_t state);
static void clear_device_list(void);
static void upsert_device_row(const netsec_ble_device_t* device);
static void refresh_empty_state(void);
static void align_empty_label(void);
static void start_scan_timer(uint32_t duration_ms);
static void stop_scan_timer(void);
static void update_scan_status_label(void);
static ble_device_entry_t* find_entry_by_addr(const uint8_t* addr);
static ble_device_entry_t* allocate_entry(const uint8_t* addr);
static void scan_timer_cb(lv_timer_t* timer);

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
  lv_label_set_text(title, "BLE Scan");
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
  lv_label_set_text(label_scan, "Scan");
  lv_obj_center(label_scan);
  lv_obj_add_style(label_scan, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_scan, lv_color_hex(COLOR_TEXT), 0);

  // Duration selection container (10/20/30s)
  g_duration_container = lv_obj_create(band_top);
  lv_obj_set_style_bg_opa(g_duration_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(g_duration_container, 0, 0);
  lv_obj_set_flex_flow(g_duration_container, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(g_duration_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(g_duration_container, PAD_TINY, 0);
  lv_obj_set_style_pad_all(g_duration_container, 0, 0);
  lv_obj_set_size(g_duration_container, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_add_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);

  const int durations[] = {10, 20, 30};
  for (uint8_t i = 0; i < sizeof(durations) / sizeof(durations[0]); i++) {
    lv_obj_t* btn = lv_btn_create(g_duration_container);
    lv_obj_set_size(btn, BUTTON_WIDTH, BUTTON_HEIGHT);
    lv_obj_add_style(btn, ui_get_style_btn_primary(), 0);
    lv_obj_add_event_cb(btn, on_duration_btn_click, LV_EVENT_CLICKED, (void*)(intptr_t)durations[i]);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text_fmt(label, "%ds", durations[i]);
    lv_obj_center(label);
    lv_obj_add_style(label, ui_get_style_label_normal(), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), 0);

    g_duration_buttons[i] = btn;
  }

  // Scanning state container
  g_scanning_container = lv_obj_create(band_top);
  lv_obj_set_style_bg_opa(g_scanning_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(g_scanning_container, 0, 0);
  lv_obj_set_flex_flow(g_scanning_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(g_scanning_container, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_all(g_scanning_container, 0, 0);
  lv_obj_set_style_pad_gap(g_scanning_container, PAD_TINY, 0);
  lv_obj_set_size(g_scanning_container, LV_SIZE_CONTENT, LV_PCT(100));
  lv_obj_add_flag(g_scanning_container, LV_OBJ_FLAG_HIDDEN);

  g_scanning_spinner = lv_spinner_create(g_scanning_container, 1000, 90);
  lv_obj_set_size(g_scanning_spinner, BUTTON_HEIGHT - PAD_TINY, BUTTON_HEIGHT - PAD_TINY);

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
  lv_label_set_text(g_empty_label, "Press Scan to search for BLE devices.");
  lv_obj_add_style(g_empty_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_empty_label, lv_color_hex(COLOR_TEXT), 0);
  align_empty_label();

  // Status text
  g_status_label = lv_label_create(scr);
  lv_label_set_text(g_status_label, "Press Scan to search for BLE devices.");
  lv_obj_add_style(g_status_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_status_label, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_set_pos(g_status_label, PAD_NORMAL, LV_VER_RES - PAD_LARGE);

  g_ble_screen = scr;
  set_top_band_state(TOP_STATE_IDLE);
  refresh_empty_state();
  Serial.println("PIXEL: BLE screen created");
  return scr;
}

lv_obj_t* ui_ble_get_scan_button(void)
{
  return g_ble_scan_button;
}

uint32_t ui_ble_get_last_scan_duration_ms(void)
{
  return g_last_duration_ms;
}

void ui_ble_set_state_idle(void)
{
  g_scan_active = false;
  stop_scan_timer();
  g_scan_remaining_ms = 0;
  set_top_band_state(TOP_STATE_IDLE);
}

void ui_ble_set_state_choosing_duration(void)
{
  set_top_band_state(TOP_STATE_DURATION);
  if (g_status_label) {
    lv_label_set_text(g_status_label, "Choose scan duration.");
  }
}

void ui_ble_set_state_scanning(uint32_t duration_ms)
{
  ui_ble_prepare_for_scan(duration_ms);
  set_top_band_state(TOP_STATE_SCANNING);
}

void ui_ble_cancel_scan(void)
{
  g_scan_active = false;
  stop_scan_timer();
  g_scan_remaining_ms = 0;
  g_has_scanned = false;
  set_top_band_state(TOP_STATE_IDLE);
  if (g_ble_scan_button) {
    lv_obj_clear_state(g_ble_scan_button, LV_STATE_DISABLED);
  }
  if (g_status_label) {
    lv_label_set_text(g_status_label, "Scan canceled.");
  }
  refresh_empty_state();
}

void ui_ble_prepare_for_scan(uint32_t duration_ms)
{
  clear_device_list();

  g_scan_active = true;
  g_last_duration_ms = duration_ms ? duration_ms : g_last_duration_ms;
  g_scan_remaining_ms = g_last_duration_ms;
  start_scan_timer(g_scan_remaining_ms);
  update_scan_status_label();
  g_has_scanned = false;

  if (g_ble_scan_button) {
    lv_obj_add_state(g_ble_scan_button, LV_STATE_DISABLED);
  }
}

void ui_ble_handle_device_found(const netsec_ble_device_t* device)
{
  if (!device) return;
  upsert_device_row(device);
}

void ui_ble_handle_scan_started(const netsec_scan_summary_t* meta)
{
  const uint32_t duration_ms = meta ? meta->duration_ms : g_last_duration_ms;
  ui_ble_set_state_scanning(duration_ms);
  g_scan_active = true;
  update_scan_status_label();
  refresh_empty_state();
}

void ui_ble_handle_scan_completed(const netsec_scan_summary_t* meta)
{
  g_scan_active = false;
  stop_scan_timer();
  g_scan_remaining_ms = 0;
  g_has_scanned = true;

  if (g_status_label) {
    uint16_t count = meta ? meta->item_count : 0;
    lv_label_set_text_fmt(g_status_label, "Scan complete (%u device%s).", count, (count == 1) ? "" : "s");
  }

  set_top_band_state(TOP_STATE_IDLE);
  if (g_ble_scan_button) {
    lv_obj_clear_state(g_ble_scan_button, LV_STATE_DISABLED);
  }
  refresh_empty_state();
}

static void on_scan_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE scan button clicked");
  ui_post_event(UI_EVENT_BLE_SCAN_REQUEST);
}

static void on_duration_btn_click(lv_event_t* e)
{
  int duration_s = (int)(intptr_t)lv_event_get_user_data(e);
  Serial.printf("PIXEL: BLE scan %ds requested\n", duration_s);
  switch (duration_s) {
    case 10:
      ui_post_event(UI_EVENT_BLE_DURATION_SELECTED_10S);
      break;
    case 20:
      ui_post_event(UI_EVENT_BLE_DURATION_SELECTED_20S);
      break;
    case 30:
      ui_post_event(UI_EVENT_BLE_DURATION_SELECTED_30S);
      break;
    default:
      break;
  }
}

static void on_cancel_btn_click(lv_event_t* e)
{
  (void)e;
  Serial.println("PIXEL: BLE scan canceled by user");
  ui_post_event(UI_EVENT_BLE_CANCEL);
}

static void set_top_band_state(top_band_state_t state)
{
  g_top_state = state;
  if (!g_duration_container || !g_idle_container || !g_scanning_container) return;

  switch (state) {
    case TOP_STATE_DURATION:
      lv_obj_add_flag(g_idle_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(g_scanning_container, LV_OBJ_FLAG_HIDDEN);
      ui_bottom_button_set("Cancel", on_cancel_btn_click);
      for (lv_obj_t* btn : g_duration_buttons) {
        if (btn) {
          lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
          lv_obj_clear_state(btn, LV_STATE_DISABLED);
        }
      }
      break;
    case TOP_STATE_SCANNING:
      lv_obj_add_flag(g_idle_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(g_scanning_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);
      ui_bottom_button_set("Cancel", on_cancel_btn_click);
      for (lv_obj_t* btn : g_duration_buttons) {
        if (btn) {
          lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
          lv_obj_add_state(btn, LV_STATE_DISABLED);
        }
      }
      break;
    case TOP_STATE_IDLE:
    default:
      lv_obj_clear_flag(g_idle_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(g_duration_container, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(g_scanning_container, LV_OBJ_FLAG_HIDDEN);
      if (g_status_label) {
        lv_label_set_text(g_status_label, "Press Scan to search for BLE devices.");
      }
      ui_bottom_button_restore();
      break;
  }
}

static void clear_device_list(void)
{
  if (!g_device_list) return;

  lv_obj_clean(g_device_list);
  memset(g_device_entries, 0, sizeof(g_device_entries));
  refresh_empty_state();
}

static ble_device_entry_t* find_entry_by_addr(const uint8_t* addr)
{
  if (!addr) return NULL;

  for (size_t i = 0; i < NETSEC_BLE_DEVICE_BUFFER_SIZE; ++i) {
    if (g_device_entries[i].in_use && memcmp(g_device_entries[i].mac_bytes, addr, sizeof(g_device_entries[i].mac_bytes)) == 0) {
      return &g_device_entries[i];
    }
  }

  return NULL;
}

static ble_device_entry_t* allocate_entry(const uint8_t* addr)
{
  if (!g_device_list || !addr) return NULL;

  for (size_t i = 0; i < NETSEC_BLE_DEVICE_BUFFER_SIZE; ++i) {
    ble_device_entry_t* entry = &g_device_entries[i];
    if (entry->in_use) continue;

    memset(entry, 0, sizeof(*entry));
    entry->in_use = true;
    memcpy(entry->mac_bytes, addr, sizeof(entry->mac_bytes));

    entry->row = lv_obj_create(g_device_list);
    lv_obj_set_size(entry->row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(entry->row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(entry->row, lv_color_hex(COLOR_SURFACE), 0);
    lv_obj_set_style_bg_opa(entry->row, LV_OPA_40, 0);
    lv_obj_set_style_border_width(entry->row, 0, 0);
    lv_obj_set_style_radius(entry->row, RADIUS_SMALL, 0);
    lv_obj_set_style_pad_all(entry->row, PAD_SMALL, 0);

    entry->label = lv_label_create(entry->row);
    lv_obj_add_style(entry->label, ui_get_style_label_normal(), 0);
    lv_obj_set_style_text_color(entry->label, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_set_width(entry->label, LV_PCT(100));
    lv_label_set_long_mode(entry->label, LV_LABEL_LONG_WRAP);

    refresh_empty_state();
    return entry;
  }

  return NULL;
}

static void upsert_device_row(const netsec_ble_device_t* device)
{
  if (!device || !g_device_list) return;

  ble_device_entry_t* entry = find_entry_by_addr(device->mac_bytes);
  if (!entry) {
    entry = allocate_entry(device->mac_bytes);
  }
  if (!entry || !entry->label) return;

  const char* mac = strlen(device->mac_str) ? device->mac_str : "--:--:--:--:--:--";

  const char* name = strlen(device->name) ? device->name : "(unknown)";
  lv_label_set_text_fmt(entry->label, "%s\n%s\nRSSI: %d dBm", name, mac, device->rssi);

  refresh_empty_state();
}

static void refresh_empty_state(void)
{
  if (!g_empty_label || !g_device_list) return;

  const char* text = g_has_scanned ? "No BLE devices found." : "Press Scan to search for BLE devices.";
  lv_label_set_text(g_empty_label, text);

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

static void start_scan_timer(uint32_t duration_ms)
{
  g_scan_remaining_ms = duration_ms;

  if (!g_scan_timer) {
    g_scan_timer = lv_timer_create(scan_timer_cb, 1000, NULL);
  } else {
    lv_timer_reset(g_scan_timer);
  }

  lv_timer_set_period(g_scan_timer, 1000);
  lv_timer_resume(g_scan_timer);
}

static void stop_scan_timer(void)
{
  if (g_scan_timer) {
    lv_timer_pause(g_scan_timer);
  }
  g_scan_remaining_ms = 0;
  update_scan_status_label();
}

static void update_scan_status_label(void)
{
  if (!g_status_label) return;

  if (g_scan_active) {
    if (g_scan_remaining_ms > 0) {
      uint32_t remaining_s = (g_scan_remaining_ms + 999) / 1000;
      lv_label_set_text_fmt(g_status_label, "Scanning (%lus)…", static_cast<unsigned long>(remaining_s));
    } else {
      lv_label_set_text(g_status_label, "Finishing scan…");
    }
  }
}

static void scan_timer_cb(lv_timer_t* timer)
{
  (void)timer;

  if (!g_scan_active) return;

  if (g_scan_remaining_ms <= 1000) {
    g_scan_remaining_ms = 0;
    stop_scan_timer();
    update_scan_status_label();
    return;
  }

  g_scan_remaining_ms -= 1000;
  update_scan_status_label();
}

