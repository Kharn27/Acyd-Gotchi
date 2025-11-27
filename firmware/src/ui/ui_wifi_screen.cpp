/*
 * PIXEL - WiFi Scan Screen
 * 
 * Displays list of detected WiFi networks.
 * Placeholder for now.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <stdio.h>

static lv_obj_t* g_wifi_screen = NULL;

lv_obj_t* ui_create_wifi_screen(void)
{
  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BG_MAIN), 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  
  // Title
  lv_obj_t* title = lv_label_create(scr);
  lv_label_set_text(title, "WiFi Networks");
  lv_obj_set_pos(title, PAD_NORMAL, PAD_LARGE);
  lv_obj_add_style(title, ui_get_style_label_title(), 0);
  
  // List container (placeholder)
  lv_obj_t* list = lv_list_create(scr);
  lv_obj_set_size(list, LV_HOR_RES - 2 * PAD_NORMAL, 
                 LV_VER_RES - 60 - 2 * PAD_LARGE);
  lv_obj_set_pos(list, PAD_NORMAL, PAD_LARGE + 30);
  lv_obj_set_style_bg_color(list, lv_color_hex(COLOR_BG_DARK), 0);
  
  // Add placeholder entries
  lv_list_add_btn(list, "📶", "Network 1 (scanning...)");
  lv_list_add_btn(list, "📶", "Network 2");
  
  // Status text
  lv_obj_t* status = lv_label_create(scr);
  lv_label_set_text(status, "Scanning...");
  lv_obj_set_pos(status, PAD_NORMAL, LV_VER_RES - 30);
  lv_obj_add_style(status, ui_get_style_label_normal(), 0);
  
  g_wifi_screen = scr;
  printf("PIXEL: WiFi screen created\n");
  return scr;
}

