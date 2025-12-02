/*
 * ARCHI - Touch Driver Implementation (XPT2046 wrapper)
 *
 * Pure hardware touch interface. Converts raw readings to display space
 * and exposes non-blocking accessors to higher layers.
 */

#include "touch_driver.h"
#include "board_config.h"

#include <Arduino.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33


// Touch state machine
typedef struct {
  uint16_t x;
  uint16_t y;
  bool pressed;
} touch_state_t;

static touch_state_t g_touch_state = {0, 0, false};
static SemaphoreHandle_t g_touch_mutex = NULL;
static SPIClass touchscreenSPI = SPIClass(VSPI);
static XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

// Utility: map function (Arduino-style) with clamping
static uint16_t map_value(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);

void cyd_touch_init(void)
{
  Serial.println("ARCHI: Touch init (XPT2046)");
  
  // Create touch mutex
  if (!g_touch_mutex) {
    g_touch_mutex = xSemaphoreCreateMutex();
    if (!g_touch_mutex) {
      Serial.println("ERROR: Failed to create touch mutex");
      return;
    }
  }
  
  // *** IMPORTANT *** : initialiser le SPI du touch sur les bons pins
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);

  // Initialize XPT2046
  if (!ts.begin(touchscreenSPI)) {
    Serial.println("ERROR: XPT2046 touchscreen begin failed");
    return;
  }
  Serial.println("ARCHI: XPT2046 touchscreen initialized");
}

bool cyd_touch_read(uint16_t * x, uint16_t * y)
{
  bool pressed = false;
  
  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Convert raw touch coordinates to display coordinates
    // These values depend on your display rotation and calibration
    uint16_t tx = map_value(p.x, TS_MINX, TS_MAXX, 0, DISP_HOR_RES - 1);
    uint16_t ty = map_value(p.y, TS_MINY, TS_MAXY, 0, DISP_VER_RES - 1);
    
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
  
  if (xSemaphoreTake(g_touch_mutex, pdMS_TO_TICKS(10))) {
    *x = g_touch_state.x;
    *y = g_touch_state.y;
    pressed = g_touch_state.pressed;
    xSemaphoreGive(g_touch_mutex);
  }
  
  return pressed;
}

void cyd_touch_deinit(void)
{
  Serial.println("ARCHI: Touch deinit");
  
  if (g_touch_mutex) {
    vSemaphoreDelete(g_touch_mutex);
    g_touch_mutex = NULL;
  }
}

// Utility: map function (Arduino-style)
static uint16_t map_value(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  if (x < in_min) x = in_min;
  if (x > in_max) x = in_max;
  return (uint16_t)((uint32_t)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

