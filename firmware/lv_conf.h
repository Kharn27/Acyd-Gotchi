#if 1
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include "board_config.h"

/* Display resolution */
#define LV_HOR_RES_MAX DISP_HOR_RES
#define LV_VER_RES_MAX DISP_VER_RES

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
