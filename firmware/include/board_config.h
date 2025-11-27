#pragma once

// Board: ESP32-CYD with TFT display + touch
// This file centralizes hardware pin configuration and constraints.

/* Display (TFT_eSPI) */
#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

/* Touch (XPT2046) */
#define TOUCH_CS_PIN   15
#define TOUCH_IRQ_PIN  33

/* FreeRTOS task configuration */
#define UI_TASK_STACK_SIZE    (8 * 1024)      // 8 KB for UI task
#define NETSEC_TASK_STACK_SIZE (12 * 1024)    // 12 KB for network task
#define UI_TASK_PRIORITY       3
#define NETSEC_TASK_PRIORITY   2

/* LVGL buffer config (will be refined in ARCHI init) */
#define LVGL_BUFFER_SIZE (320 * 240 / 8)  // Conservative: ~9 KB

/* Lab mode gate (security flag, defaults to false) */
#define LAB_MODE_ENABLED  0
