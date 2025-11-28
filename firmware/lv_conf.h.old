#if 1
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* board config may or may not be available when LVGL core compiles */
#if defined(__has_include)
# if __has_include("board_config.h")
#  include "board_config.h"
# endif
#else
/* fallback: try include but suppress build-break if missing */
# include "board_config.h"
#endif

/* Display resolution: prefer DISP_* if available, otherwise safe defaults */
#ifndef LV_HOR_RES_MAX
 #ifdef DISP_HOR_RES
  #define LV_HOR_RES_MAX DISP_HOR_RES
 #else
  #define LV_HOR_RES_MAX 320
 #endif
#endif

#ifndef LV_VER_RES_MAX
 #ifdef DISP_VER_RES
  #define LV_VER_RES_MAX DISP_VER_RES
 #else
  #define LV_VER_RES_MAX 240
 #endif
#endif

/* Color settings */
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* Theme defaults */
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 0
#define LV_THEME_DEFAULT_PALETTE_PRIMARY LV_PALETTE_BLUE
#define LV_THEME_DEFAULT_PALETTE_SECONDARY LV_PALETTE_DEEP_PURPLE

/* Fonts */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_THEME_DEFAULT_FONT_SMALL &lv_font_montserrat_12
#define LV_THEME_DEFAULT_FONT_NORMAL &lv_font_montserrat_14
#define LV_THEME_DEFAULT_FONT_LARGE &lv_font_montserrat_16

/* Logging */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

#include "lv_conf_internal.h"

#endif /* LV_CONF_H */
#endif /*End of "Content enable"*/
