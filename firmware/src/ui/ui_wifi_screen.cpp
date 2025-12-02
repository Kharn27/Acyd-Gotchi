/*
 * PIXEL - WiFi Scan Screen
 *
 * Displays list of detected WiFi networks with real-time updates from
 * netsec_result_queue.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define WIFI_AP_BUFFER_SIZE 32

typedef struct {
  bool in_use;
  uint8_t bssid[6];
  lv_obj_t* row;
  lv_obj_t* label;
} wifi_ap_entry_t;

static lv_obj_t* g_wifi_screen = NULL;
static lv_obj_t* g_wifi_list = NULL;
static lv_obj_t* g_wifi_empty_label = NULL;
static lv_obj_t* g_wifi_status_label = NULL;
static wifi_ap_entry_t g_wifi_entries[WIFI_AP_BUFFER_SIZE];

static void refresh_empty_state(void);
static wifi_ap_entry_t* find_entry_by_bssid(const uint8_t* bssid);
static wifi_ap_entry_t* allocate_entry(const uint8_t* bssid);
static void upsert_ap_row(const netsec_wifi_ap_t* ap);

lv_obj_t* ui_create_wifi_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "WiFi Networks");
  lv_obj_set_pos(title, PAD_NORMAL, PAD_LARGE);
  lv_obj_add_style(title, ui_get_style_label_title(), 0);

  // Scrollable list container
  g_wifi_list = lv_obj_create(scr);
  lv_obj_set_size(g_wifi_list, LV_HOR_RES - 2 * PAD_NORMAL,
                  LV_VER_RES - BAND_HEIGHT - 2 * PAD_LARGE);
  lv_obj_set_pos(g_wifi_list, PAD_NORMAL, BAND_HEIGHT + PAD_SMALL);
  lv_obj_set_style_bg_color(g_wifi_list, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(g_wifi_list, LV_OPA_20, 0);
  lv_obj_set_style_border_width(g_wifi_list, 0, 0);
  lv_obj_set_style_pad_all(g_wifi_list, PAD_SMALL, 0);
  lv_obj_set_style_pad_gap(g_wifi_list, PAD_SMALL, 0);
  lv_obj_set_style_radius(g_wifi_list, RADIUS_NORMAL, 0);
  lv_obj_set_scroll_dir(g_wifi_list, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(g_wifi_list, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_set_flex_flow(g_wifi_list, LV_FLEX_FLOW_COLUMN);

  // Empty state label
  g_wifi_empty_label = lv_label_create(scr);
  lv_label_set_text(g_wifi_empty_label, "No WiFi networks yet.");
  lv_obj_add_style(g_wifi_empty_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_wifi_empty_label, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_align_to(g_wifi_empty_label, g_wifi_list, LV_ALIGN_CENTER, 0, 0);

  // Status text
  g_wifi_status_label = lv_label_create(scr);
  lv_label_set_text(g_wifi_status_label, "Waiting for scan results…");
  lv_obj_add_style(g_wifi_status_label, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_wifi_status_label, lv_color_hex(COLOR_TEXT), 0);
  lv_obj_set_pos(g_wifi_status_label, PAD_NORMAL, LV_VER_RES - PAD_LARGE);

  refresh_empty_state();
  g_wifi_screen = scr;
  Serial.println("PIXEL: WiFi screen created");
  return scr;
}

void ui_wifi_handle_ap_found(const netsec_wifi_ap_t* ap)
{
  if (!ap) return;
  upsert_ap_row(ap);
}

void ui_wifi_handle_scan_done(void)
{
  if (g_wifi_status_label) {
    lv_label_set_text(g_wifi_status_label, "Scan complete.");
  }
}

static wifi_ap_entry_t* find_entry_by_bssid(const uint8_t* bssid)
{
  if (!bssid) return NULL;

  for (size_t i = 0; i < WIFI_AP_BUFFER_SIZE; ++i) {
    wifi_ap_entry_t* entry = &g_wifi_entries[i];
    if (entry->in_use && memcmp(entry->bssid, bssid, sizeof(entry->bssid)) == 0) {
      return entry;
    }
  }

  return NULL;
}

static wifi_ap_entry_t* allocate_entry(const uint8_t* bssid)
{
  if (!g_wifi_list || !bssid) return NULL;

  for (size_t i = 0; i < WIFI_AP_BUFFER_SIZE; ++i) {
    wifi_ap_entry_t* entry = &g_wifi_entries[i];
    if (entry->in_use) continue;

    memset(entry, 0, sizeof(*entry));
    entry->in_use = true;
    memcpy(entry->bssid, bssid, sizeof(entry->bssid));

    entry->row = lv_obj_create(g_wifi_list);
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

static void upsert_ap_row(const netsec_wifi_ap_t* ap)
{
  if (!ap || !g_wifi_list) return;

  wifi_ap_entry_t* entry = find_entry_by_bssid(ap->bssid);
  if (!entry) {
    entry = allocate_entry(ap->bssid);
  }
  if (!entry || !entry->label) return;

  char bssid[18];
  snprintf(bssid, sizeof(bssid), "%02X:%02X:%02X:%02X:%02X:%02X",
           ap->bssid[0], ap->bssid[1], ap->bssid[2],
           ap->bssid[3], ap->bssid[4], ap->bssid[5]);

  lv_label_set_text_fmt(entry->label, "%s\n%s\nRSSI: %d dBm | CH: %u",
                        ap->ssid, bssid, ap->rssi, ap->channel);

  if (g_wifi_status_label) {
    lv_label_set_text(g_wifi_status_label, "Scanning…");
  }
  refresh_empty_state();
}

static void refresh_empty_state(void)
{
  if (!g_wifi_empty_label || !g_wifi_list) return;

  bool has_entries = lv_obj_get_child_cnt(g_wifi_list) > 0;
  if (has_entries) {
    lv_obj_add_flag(g_wifi_empty_label, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(g_wifi_empty_label, LV_OBJ_FLAG_HIDDEN);
  }
}

