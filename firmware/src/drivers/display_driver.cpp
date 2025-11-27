#include "display_driver.h"
#include "board_config.h"
#include "touch_driver.h"
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <Arduino.h>

// Forward declaration for touch_read from touch_driver.h
// Forward declaration removed; using cyd_touch_* API

static TFT_eSPI tft = TFT_eSPI();

#ifndef LV_HOR_RES_MAX
#define LV_HOR_RES_MAX 320
#endif
#ifndef LV_VER_RES_MAX
#define LV_VER_RES_MAX 240
#endif

#define DRAW_BUF_LINES 10
static lv_disp_draw_buf_t draw_buf;
static lv_color_t draw_buf_1[LV_HOR_RES_MAX * DRAW_BUF_LINES];
static lv_disp_t* g_disp = NULL;
static lv_indev_t* g_indev_touch = NULL;

// LVGL flush callback
void display_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;
    uint32_t w = (uint32_t)(x2 - x1 + 1);
    uint32_t h = (uint32_t)(y2 - y1 + 1);

    // Push the pixels to the TFT
    tft.startWrite();
    tft.setAddrWindow(x1, y1, w, h);
    // lv_color_t is typically 16-bit on Arduino builds
    tft.pushColors((uint16_t*)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

// Touch read callback using touch_driver API
void display_touch_read(lv_indev_drv_t* indev_drv, lv_indev_data_t* data)
{
    (void)indev_drv;
    uint16_t x=0, y=0;
    bool pressed = cyd_touch_read(&x, &y);
    if (pressed) {
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void display_init(void)
{
    Serial.println("ARCHI: Display init (TFT_eSPI)");
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

#ifdef BACKLIGHT_PIN
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);
#endif

    // Initialize LVGL draw buffer
    lv_disp_draw_buf_init(&draw_buf, draw_buf_1, NULL, LV_HOR_RES_MAX * DRAW_BUF_LINES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;

    g_disp = lv_disp_drv_register(&disp_drv);

    // Register touch input
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    // Ensure touch driver is initialized before registering input
    cyd_touch_init();
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = display_touch_read;
    g_indev_touch = lv_indev_drv_register(&indev_drv);

    // Ensure byte swap matches LVGL color order
    tft.setSwapBytes(true);
}

void display_deinit(void)
{
#ifdef BACKLIGHT_PIN
    digitalWrite(BACKLIGHT_PIN, LOW);
#endif
}

lv_disp_t* display_get_disp(void) { return g_disp; }
lv_indev_t* display_get_indev_touch(void) { return g_indev_touch; }
