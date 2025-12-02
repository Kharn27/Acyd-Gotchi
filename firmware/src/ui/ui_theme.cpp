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
  // Force LVGL default theme to dark mode with neon green primary accents.
  lv_disp_t* disp = lv_disp_get_default();
  if (disp) {
    lv_theme_t* theme = lv_theme_default_init(disp,
                                             lv_color_hex(COLOR_ACCENT),
                                             lv_color_hex(COLOR_ACCENT),
                                             true, /* dark mode */
                                             &lv_font_unscii_8);
    lv_disp_set_theme(disp, theme);
  }

  // Initialize primary button style
  lv_style_init(&style_btn_primary);
  lv_style_set_bg_color(&style_btn_primary, lv_color_hex(COLOR_ACCENT));
  lv_style_set_bg_opa(&style_btn_primary, LV_OPA_30);
  lv_style_set_text_color(&style_btn_primary, lv_color_hex(COLOR_TEXT));
  lv_style_set_text_font(&style_btn_primary, &lv_font_unscii_8);
  lv_style_set_border_width(&style_btn_primary, 2);
  lv_style_set_border_color(&style_btn_primary, lv_color_hex(COLOR_ACCENT));
  lv_style_set_radius(&style_btn_primary, RADIUS_NORMAL);
  lv_style_set_pad_all(&style_btn_primary, PAD_NORMAL);
  lv_style_set_shadow_color(&style_btn_primary, lv_color_hex(COLOR_ACCENT));
  lv_style_set_shadow_width(&style_btn_primary, 10);
  lv_style_set_shadow_opa(&style_btn_primary, LV_OPA_40);

  // Initialize secondary button style
  lv_style_init(&style_btn_secondary);
  lv_style_set_bg_color(&style_btn_secondary, lv_color_hex(COLOR_SURFACE));
  lv_style_set_bg_opa(&style_btn_secondary, LV_OPA_COVER);
  lv_style_set_text_color(&style_btn_secondary, lv_color_hex(COLOR_TEXT));
  lv_style_set_text_font(&style_btn_secondary, &lv_font_unscii_8);
  lv_style_set_border_width(&style_btn_secondary, 1);
  lv_style_set_border_color(&style_btn_secondary, lv_color_hex(COLOR_ACCENT));
  lv_style_set_radius(&style_btn_secondary, RADIUS_SMALL);
  lv_style_set_pad_all(&style_btn_secondary, PAD_SMALL);
  lv_style_set_shadow_opa(&style_btn_secondary, LV_OPA_TRANSP);

  // Initialize title label style
  lv_style_init(&style_label_title);
  lv_style_set_text_color(&style_label_title, lv_color_hex(COLOR_TEXT));
  lv_style_set_text_font(&style_label_title, &lv_font_unscii_16);

  // Initialize normal label style
  lv_style_init(&style_label_normal);
  lv_style_set_text_color(&style_label_normal, lv_color_hex(COLOR_TEXT));
  lv_style_set_text_opa(&style_label_normal, LV_OPA_60);
  lv_style_set_text_font(&style_label_normal, &lv_font_unscii_8);
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

