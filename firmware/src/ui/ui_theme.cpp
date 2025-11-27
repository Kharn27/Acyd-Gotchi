/*
 * PIXEL - UI Theme Implementation
 * 
 * Initializes and manages LVGL styles.
 */

#include "ui_theme.h"
#include "lvgl.h"

// Static style objects
static lv_style_t style_btn_primary;
static lv_style_t style_btn_secondary;
static lv_style_t style_label_title;
static lv_style_t style_label_normal;

void ui_theme_init(void)
{
  // Initialize primary button style
  lv_style_init(&style_btn_primary);
  lv_style_set_bg_color(&style_btn_primary, lv_color_hex(COLOR_PRIMARY));
  lv_style_set_text_color(&style_btn_primary, lv_color_hex(COLOR_TEXT_PRIMARY));
  lv_style_set_border_width(&style_btn_primary, 1);
  lv_style_set_border_color(&style_btn_primary, lv_color_hex(COLOR_BORDER));
  lv_style_set_radius(&style_btn_primary, RADIUS_NORMAL);
  lv_style_set_pad_all(&style_btn_primary, PAD_NORMAL);
  
  // Initialize secondary button style
  lv_style_init(&style_btn_secondary);
  lv_style_set_bg_color(&style_btn_secondary, lv_color_hex(COLOR_SURFACE));
  lv_style_set_text_color(&style_btn_secondary, lv_color_hex(COLOR_TEXT_PRIMARY));
  lv_style_set_border_width(&style_btn_secondary, 1);
  lv_style_set_border_color(&style_btn_secondary, lv_color_hex(COLOR_BORDER));
  lv_style_set_radius(&style_btn_secondary, RADIUS_SMALL);
  lv_style_set_pad_all(&style_btn_secondary, PAD_SMALL);
  
  // Initialize title label style
  lv_style_init(&style_label_title);
  lv_style_set_text_color(&style_label_title, lv_color_hex(COLOR_TEXT_PRIMARY));
  lv_style_set_text_font(&style_label_title, &lv_font_montserrat_20);
  
  // Initialize normal label style
  lv_style_init(&style_label_normal);
  lv_style_set_text_color(&style_label_normal, lv_color_hex(COLOR_TEXT_SECONDARY));
  lv_style_set_text_font(&style_label_normal, &lv_font_montserrat_14);
}

lv_style_t* ui_get_style_btn_primary(void)
{
  return &style_btn_primary;
}

lv_style_t* ui_get_style_btn_secondary(void)
{
  return &style_btn_secondary;
}

lv_style_t* ui_get_style_label_title(void)
{
  return &style_label_title;
}

lv_style_t* ui_get_style_label_normal(void)
{
  return &style_label_normal;
}

