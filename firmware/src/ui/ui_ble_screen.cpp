/*
 * PIXEL - BLE Scan Screen
 * 
 * Displays list of detected BLE devices.
 * Placeholder for now.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>

static lv_obj_t* g_ble_screen = NULL;
static lv_obj_t* g_ble_scan_button = NULL;

static void on_scan_btn_click(lv_event_t* e);

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

  // Scan button
  g_ble_scan_button = lv_btn_create(band_top);
  lv_obj_set_size(g_ble_scan_button, 70, 30);
  lv_obj_add_style(g_ble_scan_button, ui_get_style_btn_primary(), 0);
  lv_obj_add_event_cb(g_ble_scan_button, on_scan_btn_click, LV_EVENT_CLICKED, NULL);

  lv_obj_t* label_scan = lv_label_create(g_ble_scan_button);
  lv_label_set_text(label_scan, "SCAN");
  lv_obj_center(label_scan);
  lv_obj_add_style(label_scan, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(label_scan, lv_color_hex(COLOR_TEXT), 0);

  // Title
  lv_obj_t* list_title = lv_label_create(scr);
  lv_label_set_text(list_title, "BLE Devices");
  lv_obj_set_pos(list_title, PAD_NORMAL, BAND_HEIGHT + PAD_SMALL);
  lv_obj_add_style(list_title, ui_get_style_label_title(), 0);

  // List container (placeholder)
  lv_obj_t* list = lv_list_create(scr);
  lv_obj_set_size(list, LV_HOR_RES - 2 * PAD_NORMAL,
                 LV_VER_RES - BAND_HEIGHT - 60 - 2 * PAD_LARGE);
  lv_obj_set_pos(list, PAD_NORMAL, BAND_HEIGHT + PAD_LARGE + 10);
  lv_obj_set_style_bg_color(list, lv_color_hex(COLOR_SURFACE), 0);
  
  // Add placeholder entries
  lv_list_add_btn(list, "📡", "Device 1 (scanning...)");
  lv_list_add_btn(list, "📡", "Device 2");
  
  // Status text
  lv_obj_t* status = lv_label_create(scr);
  lv_label_set_text(status, "Scanning...");
  lv_obj_set_pos(status, PAD_NORMAL, LV_VER_RES - PAD_LARGE);
  lv_obj_add_style(status, ui_get_style_label_normal(), 0);

  g_ble_screen = scr;
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
}

lv_obj_t* ui_create_settings_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  
  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "Settings");
  lv_obj_set_pos(title, PAD_NORMAL, PAD_LARGE);
  lv_obj_add_style(title, ui_get_style_label_title(), 0);
  
  // Placeholder content
  lv_obj_t* text = lv_label_create(scr);
  lv_label_set_text(text, "Settings coming soon!");
  lv_obj_set_pos(text, PAD_NORMAL, 60);
  lv_obj_add_style(text, ui_get_style_label_normal(), 0);

  Serial.println("PIXEL: Settings screen created");
  return scr;
}

