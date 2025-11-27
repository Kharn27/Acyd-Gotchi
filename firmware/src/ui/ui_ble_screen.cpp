/*
 * PIXEL - BLE Scan Screen
 * 
 * Displays list of detected BLE devices.
 * Placeholder for now.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <stdio.h>

static lv_obj_t* g_ble_screen = NULL;

lv_obj_t* ui_create_ble_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_size(scr, DISP_HOR_RES, DISP_VER_RES);
  
  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "BLE Devices");
  lv_obj_set_pos(title, PAD_NORMAL, PAD_LARGE);
  lv_obj_add_style(title, ui_get_style_label_title(), 0);
  
  // List container (placeholder)
  lv_obj_t* list = lv_list_create(scr);
  lv_obj_set_size(list, DISP_HOR_RES - 2 * PAD_NORMAL, 
                 DISP_VER_RES - 60 - 2 * PAD_LARGE);
  lv_obj_set_pos(list, PAD_NORMAL, PAD_LARGE + 30);
  lv_obj_set_style_bg_color(list, lv_color_hex(COLOR_BG_DARK), 0);
  
  // Add placeholder entries
  lv_list_add_btn(list, "📡", "Device 1 (scanning...)");
  lv_list_add_btn(list, "📡", "Device 2");
  
  // Status text
  lv_obj_t* status = lv_label_create(scr);
  lv_label_set_text(status, "Scanning...");
  lv_obj_set_pos(status, PAD_NORMAL, DISP_VER_RES - 30);
  lv_obj_add_style(status, ui_get_style_label_normal(), 0);
  
  g_ble_screen = scr;
  printf("PIXEL: BLE screen created\n");
  return scr;
}

lv_obj_t* ui_create_settings_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_size(scr, DISP_HOR_RES, DISP_VER_RES);
  
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
  
  printf("PIXEL: Settings screen created\n");
  return scr;
}

