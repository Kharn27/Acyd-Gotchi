/*
 * ARCHI - Touch Driver Implementation (XPT2046 wrapper)
 * 
 * Non-blocking touch interface for LVGL.
 * Provides touch coordinate reading and conversion to display space.
 */

#include "touch_driver.h"
#include "board_config.h"

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Try to include XPT2046 library; if not available, use mock
// Determine MOCK_TOUCH (0 = real driver, 1 = mock) in a predictable way.
// Priority:
// 1) build flag  (preferred)
// 2) TOUCH_MOCK defined -> force mock
// 3) auto-detect by checking for XPT2046_Touchscreen.h

#ifndef MOCK_TOUCH
  #ifdef TOUCH_MOCK
    #define MOCK_TOUCH 1
  #else
    #if defined(__has_include)
      #if __has_include(XPT2046_Touchscreen.h)
        #include XPT2046_Touchscreen.h
        #define MOCK_TOUCH 0
      #else
        #define MOCK_TOUCH 1
      #endif
    #else
      #define MOCK_TOUCH 1
    #endif
  #endif
#endif


// Touch state machine
typedef struct {
  uint16_t x;
  uint16_t y;
  bool pressed;
} touch_state_t;

static touch_state_t g_touch_state = {0, 0, false};
static SemaphoreHandle_t g_touch_mutex = NULL;

#if !MOCK_TOUCH
  static XPT2046_Touchscreen ts(TOUCH_CS);
#endif

void touch_init(void)
{
  printf("ARCHI: Touch init (MOCK=%d)\n", MOCK_TOUCH ? 1 : 0);
  
  // Create touch mutex
  if (!g_touch_mutex) {
    g_touch_mutex = xSemaphoreCreateMutex();
    if (!g_touch_mutex) {
      printf("ERROR: Failed to create touch mutex\n");
      return;
    }
  }
  
#ifndef MOCK_TOUCH
  // Initialize XPT2046
  if (!ts.begin()) {
    printf("ERROR: XPT2046 touchscreen begin failed\n");
    return;
  }
  printf("ARCHI: XPT2046 touchscreen initialized\n");
#else
  printf("ARCHI: Running in MOCK touch mode\n");
#endif
}

bool touch_read(uint16_t * x, uint16_t * y)
{
  bool pressed = false;
  
#ifndef MOCK_TOUCH
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    
    // Convert raw touch coordinates to display coordinates
    // These values depend on your display rotation and calibration
    uint16_t tx = map(p.x, TS_MINX, TS_MAXX, 0, DISP_HOR_RES);
    uint16_t ty = map(p.y, TS_MINY, TS_MAXY, 0, DISP_VER_RES);
    
    if (xSemaphoreTake(g_touch_mutex, pdMS_TO_TICKS(10))) {
      g_touch_state.x = tx;
      g_touch_state.y = ty;
      g_touch_state.pressed = true;
      xSemaphoreGive(g_touch_mutex);
    }
    pressed = true;
  } else {
    if (xSemaphoreTake(g_touch_mutex, pdMS_TO_TICKS(10))) {
      g_touch_state.pressed = false;
      xSemaphoreGive(g_touch_mutex);
    }
  }
#else
  // Mock: always not pressed
  if (xSemaphoreTake(g_touch_mutex, pdMS_TO_TICKS(10))) {
    g_touch_state.pressed = false;
    xSemaphoreGive(g_touch_mutex);
  }
#endif
  
  if (xSemaphoreTake(g_touch_mutex, pdMS_TO_TICKS(10))) {
    *x = g_touch_state.x;
    *y = g_touch_state.y;
    pressed = g_touch_state.pressed;
    xSemaphoreGive(g_touch_mutex);
  }
  
  return pressed;
}

void touch_deinit(void)
{
  printf("ARCHI: Touch deinit\n");
  
  if (g_touch_mutex) {
    vSemaphoreDelete(g_touch_mutex);
    g_touch_mutex = NULL;
  }
}

// Utility: map function (Arduino-style)
static uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

