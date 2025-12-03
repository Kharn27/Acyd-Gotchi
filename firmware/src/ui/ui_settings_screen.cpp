/*
 * PIXEL - Settings Screen
 *
 * Placeholder for settings UI.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <Arduino.h>

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

