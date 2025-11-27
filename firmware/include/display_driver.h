/*
 * ARCHI - Display Driver Header (TFT_eSPI wrapper for LVGL)
 * 
 * Wraps TFT_eSPI and provides LVGL flush callback.
 * Handles display buffer allocation, DMA support, and SPI mutex for concurrency.
 */

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize display: allocate buffers, configure TFT_eSPI, register LVGL driver
// Must be called before lvgl_port_init().
void display_init(void);

// Flush a portion of the framebuffer to the display
// Called by LVGL via lv_disp_drv_t.flush_cb
void display_flush(struct _lv_disp_drv_t * disp_drv, const struct _lv_area_t * area, lv_color_t * color_p);

// Input read callback (touch)
// Called by LVGL via lv_indev_drv_t.read_cb
void display_touch_read(struct _lv_indev_drv_t * indev_drv, struct _lv_indev_data_t * data);

// Deinit display: cleanup buffers, disable TFT_eSPI
void display_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_DRIVER_H
