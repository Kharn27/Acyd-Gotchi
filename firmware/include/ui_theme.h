/*
 * PIXEL - UI Theme
 * 
 * Centralized theme for consistent styling across all screens.
 * Colors, font sizes, padding, border radius.
 */

#ifndef UI_THEME_H
#define UI_THEME_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Color palette (RGB565 on ESP32-CYD)
#define COLOR_BG_DARK     0x1062  // Dark background
#define COLOR_BG_MAIN     0x2945  // Main background
#define COLOR_SURFACE     0x4A4F  // Elevated surface
#define COLOR_PRIMARY     0x049D  // Bright cyan (highlights)
#define COLOR_ACCENT      0xFD20  // Orange/red accent
#define COLOR_TEXT_PRIMARY 0xFFFF  // White
#define COLOR_TEXT_SECONDARY 0xBDF7 // Light gray
#define COLOR_BORDER      0x39E7  // Teal border
#define COLOR_SUCCESS     0x07E0  // Green
#define COLOR_ERROR       0xF800  // Red

// Typography
#define FONT_SIZE_SMALL   12
#define FONT_SIZE_NORMAL  14
#define FONT_SIZE_LARGE   18
#define FONT_SIZE_TITLE   24

// Spacing & dimensions
#define PAD_TINY   2
#define PAD_SMALL  4
#define PAD_NORMAL 8
#define PAD_LARGE  16

#define RADIUS_SMALL  3
#define RADIUS_NORMAL 6
#define RADIUS_LARGE  12

// Component dimensions
#define BUTTON_HEIGHT 32
#define BUTTON_WIDTH  80
#define BAND_HEIGHT   40

// Screen layout
#define MAIN_SCREEN_PET_SIZE 120
#define MAIN_SCREEN_STATUS_HEIGHT 20

// Initialize theme (call once at startup)
void ui_theme_init(void);

// Get/set common styles for reuse
lv_style_t* ui_get_style_btn_primary(void);
lv_style_t* ui_get_style_btn_secondary(void);
lv_style_t* ui_get_style_label_title(void);
lv_style_t* ui_get_style_label_normal(void);

#ifdef __cplusplus
}
#endif

#endif // UI_THEME_H
