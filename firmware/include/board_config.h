// Board: ESP32-CYD with TFT display + touch
// This file centralizes hardware pin configuration and constraints.

#pragma once

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

#define TOUCH_CS_PIN   33

// Ne redéfinir TOUCH_CS que si la lib TFT_eSPI ne l'a pas déjà fait
#ifndef TOUCH_CS
  #define TOUCH_CS TOUCH_CS_PIN
#endif

// Mode mock display (pas de vrai TFT) – par défaut désactivé
#ifndef MOCK_TFT_ESPI
  #define MOCK_TFT_ESPI 0
#endif

// Unified resolution macros used across UI and drivers
#define DISP_HOR_RES DISPLAY_WIDTH
#define DISP_VER_RES DISPLAY_HEIGHT

/* Touch (XPT2046) */
// #define TOUCH_CS_PIN   15
// #define TOUCH_IRQ_PIN  33

// Legacy-compatible aliases used by drivers
// #define TOUCH_CS   TOUCH_CS_PIN
// #define TOUCH_IRQ  TOUCH_IRQ_PIN

// Basic calibration bounds (can be refined after hardware validation)
#define TS_MINX 200
#define TS_MAXX 3800
#define TS_MINY 200
#define TS_MAXY 3800

/* FreeRTOS task configuration */
#define UI_TASK_STACK_SIZE    (8 * 1024)      // 8 KB for UI task
#define NETSEC_TASK_STACK_SIZE (12 * 1024)    // 12 KB for network task
#define UI_TASK_PRIORITY       3
#define NETSEC_TASK_PRIORITY   2

/* LVGL buffer config (will be refined in ARCHI init) */
#define LVGL_BUFFER_SIZE (320 * 240 / 8)  // Conservative: ~9 KB

/* Lab mode gate (security flag, defaults to false) */
#define LAB_MODE_ENABLED  0

#ifndef MOCK_TFT_ESPI
  #define MOCK_TFT_ESPI 0
#endif

#ifndef MOCK_TOUCH
  #define MOCK_TOUCH 0
#endif
