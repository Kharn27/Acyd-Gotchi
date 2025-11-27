/*
 * ARCHI - Display Driver Implementation (TFT_eSPI wrapper for LVGL)
 * 
 * Allocates display buffers, registers LVGL display driver,
 * and provides flush callback.
 * 
 * NOTE: If TFT_eSPI compilation fails due to missing User_Setup.h,
 * this file provides a MOCK implementation that compiles but does nothing.
 * Use DISPLAY_MOCK or set MOCK_DISPLAY=1 in platformio.ini to enable mock mode.
 */

#include "display_driver.h"
#include "board_config.h"
#include "touch_driver.h"
#include "lvgl.h"

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Try to include TFT_eSPI; if not available, mock implementation
#ifdef DISPLAY_MOCK
  #define MOCK_TFT_ESPI 1
#endif

#ifndef MOCK_TFT_ESPI
  #include "TFT_eSPI.h"
  static TFT_eSPI tft;
#endif

// Display buffer configuration
#define DISPLAY_BUF_SIZE (DISP_HOR_RES * 40 * sizeof(lv_color_t))  // 40 lines

// Static display buffer (allocated in RAM or PSRAM)
static lv_color_t* g_display_buf1 = NULL;
static lv_color_t* g_display_buf2 = NULL;

// LVGL draw buffer must persist after registration
static lv_disp_draw_buf_t g_draw_buf;

// SPI mutex for display/touch concurrency
static SemaphoreHandle_t g_spi_mutex = NULL;

// LVGL display driver
static lv_disp_drv_t g_disp_drv;
static lv_indev_drv_t g_indev_drv;

// Registered LVGL handles
static lv_disp_t* g_disp = NULL;
static lv_indev_t* g_indev = NULL;

// Forward declarations
static void display_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static void display_touch_read_cb(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);

void display_init(void)
{
  printf("ARCHI: Display init (MOCK=%d)\n", MOCK_TFT_ESPI ? 1 : 0);
  
  // Create SPI mutex
  if (!g_spi_mutex) {
    g_spi_mutex = xSemaphoreCreateMutex();
    if (!g_spi_mutex) {
      printf("ERROR: Failed to create SPI mutex\n");
      return;
    }
  }
  
#ifndef MOCK_TFT_ESPI
  // Initialize TFT_eSPI
  tft.init();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(TFT_BLACK);
  printf("ARCHI: TFT_eSPI initialized\n");
#else
  printf("ARCHI: Running in MOCK mode (no actual display I/O)\n");
#endif
  
  // Allocate display buffers
  g_display_buf1 = (lv_color_t *)malloc(DISPLAY_BUF_SIZE);
  if (!g_display_buf1) {
    printf("ERROR: Failed to allocate display buffer 1\n");
    return;
  }
  
  g_display_buf2 = (lv_color_t *)malloc(DISPLAY_BUF_SIZE);
  if (!g_display_buf2) {
    printf("ERROR: Failed to allocate display buffer 2\n");
    free(g_display_buf1);
    g_display_buf1 = NULL;
    return;
  }
  
  printf("ARCHI: Display buffers allocated (%u bytes each)\n", DISPLAY_BUF_SIZE);
  
  // Initialize LVGL display driver
  lv_disp_draw_buf_init(&g_draw_buf, g_display_buf1, g_display_buf2, DISP_HOR_RES * 40);
  
  lv_disp_drv_init(&g_disp_drv);
  g_disp_drv.hor_res = DISP_HOR_RES;
  g_disp_drv.ver_res = DISP_VER_RES;
  g_disp_drv.flush_cb = display_flush_cb;
  g_disp_drv.draw_buf = &g_draw_buf;

  g_disp = lv_disp_drv_register(&g_disp_drv);
  if (!g_disp) {
    printf("ERROR: Failed to register LVGL display driver\n");
    return;
  }
  
  printf("ARCHI: LVGL display driver registered\n");
  
  // Initialize LVGL input device (touch)
  lv_indev_drv_init(&g_indev_drv);
  g_indev_drv.type = LV_INDEV_TYPE_POINTER;
  g_indev_drv.read_cb = display_touch_read_cb;

  // Initialize touch controller before registering input device
  cyd_touch_init();

  g_indev = lv_indev_drv_register(&g_indev_drv);
  if (!g_indev) {
    printf("ERROR: Failed to register LVGL input device\n");
    return;
  }
  
  printf("ARCHI: LVGL input device (touch) registered\n");
}

void display_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  // This is called by LVGL to flush a portion of framebuffer
  // Implementation stub for now; real DMA/SPI logic would go here
  
  (void)disp_drv;
  (void)area;
  (void)color_p;
  
#ifndef MOCK_TFT_ESPI
  // In real implementation:
  // 1. Acquire SPI mutex
  // 2. Call tft.setAddrWindow() with area coordinates
  // 3. Write pixel data via pushImage/pushPixels
  // 4. Release SPI mutex
  // 5. Call lv_disp_flush_ready()
  
  if (xSemaphoreTake(g_spi_mutex, pdMS_TO_TICKS(100))) {
    // TODO: actual SPI write
    xSemaphoreGive(g_spi_mutex);
  }
#endif
  
  lv_disp_flush_ready(disp_drv);
}

void display_touch_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
  // Stub for touch read
  (void)indev_drv;

  uint16_t x = 0, y = 0;
  bool pressed = cyd_touch_read(&x, &y);

  data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  data->point.x = x;
  data->point.y = y;
}

void display_deinit(void)
{
  printf("ARCHI: Display deinit\n");
  
  if (g_display_buf1) {
    free(g_display_buf1);
    g_display_buf1 = NULL;
  }
  
  if (g_display_buf2) {
    free(g_display_buf2);
    g_display_buf2 = NULL;
  }
  
  if (g_spi_mutex) {
    vSemaphoreDelete(g_spi_mutex);
    g_spi_mutex = NULL;
  }

  cyd_touch_deinit();
}

struct _lv_disp_t* display_get_disp(void)
{
  return g_disp;
}

struct _lv_indev_t* display_get_indev_touch(void)
{
  return g_indev;
}

// Callback stubs for LVGL
static void display_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  display_flush(disp_drv, area, color_p);
}

static void display_touch_read_cb(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
  display_touch_read(indev_drv, data);
}

