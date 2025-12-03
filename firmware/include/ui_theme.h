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

// Minimal color palette (24-bit RGB hex)
// If the background looks inverted (gray/white), enable TFT_INVERSION_ON in platformio.ini.
#define COLOR_BACKGROUND 0x000000  // Base background for full screens
#define COLOR_SURFACE    0x00FF00  // Slightly lifted surfaces (cards, lists, bands)
#define COLOR_ACCENT     0xAA33CC  // Neon accent for borders, highlights, focus
#define COLOR_TEXT       0x00FF00  // Primary text color; dim via opacity when needed
#define COLOR_GREEN      0xFF00FF  // Neon Green accent
#define COLOR_CPC_BLUE   0x0000AA  // Amstrad CPC blue for central areas
#define COLOR_CPC_YELLOW 0xFFFF00  // Amstrad CPC bright yellow for central text

// Typography
#define FONT_SIZE_SMALL   12
#define FONT_SIZE_NORMAL  14
#define FONT_SIZE_LARGE   18
#define FONT_SIZE_TITLE   24

// Spacing & dimensions
#define PAD_TINY   2
#define PAD_SMALL  4
#define PAD_NORMAL 6
#define PAD_LARGE  16

#define RADIUS_SMALL  3
#define RADIUS_NORMAL 6
#define RADIUS_LARGE  12

// Component dimensions
#define BUTTON_HEIGHT 32
#define BUTTON_WIDTH  80
#define BAND_HEIGHT   36

// Screen layout
#define MAIN_SCREEN_PET_SIZE 120
#define MAIN_SCREEN_STATUS_HEIGHT 20

// Initialize theme (call once at startup)
void ui_theme_init(void);

// Get/set common styles for reuse
lv_style_t* ui_get_style_btn_primary(void);
lv_style_t* ui_get_style_label_title(void);
lv_style_t* ui_get_style_label_normal(void);

#ifdef __cplusplus
}
#endif

#endif // UI_THEME_H
