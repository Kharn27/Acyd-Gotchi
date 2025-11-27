/*
 * ARCHI - Display Driver Header (TFT_eSPI wrapper for LVGL)
 */
#pragma once

extern "C" {
  #include "lvgl.h"
}

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <cstdint>
#include <cstddef>

struct _lv_disp_t;
struct _lv_indev_t;

#ifdef __cplusplus
extern "C" {
#endif

void display_init(void);
void display_flush(struct _lv_disp_drv_t * disp_drv, const struct _lv_area_t * area, lv_color_t * color_p);
void display_touch_read(struct _lv_indev_drv_t * indev_drv, struct _lv_indev_data_t * data);
void display_deinit(void);

struct _lv_disp_t* display_get_disp(void);
struct _lv_indev_t* display_get_indev_touch(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_DRIVER_H