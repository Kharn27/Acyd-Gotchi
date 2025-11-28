/*
 * ARCHI - Display Driver Header (TFT_eSPI wrapper)
 *
 * Pure hardware layer: no LVGL dependency and no knowledge of other drivers.
 */
#pragma once

#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <cstdint>
#include <cstddef>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the TFT display hardware (rotation, backlight, clear screen)
void display_hw_init(void);

// Deinitialize the display (e.g., turn off backlight)
void display_hw_deinit(void);

// Optional helpers for higher layers
void display_hw_set_rotation(uint8_t rotation);
void display_hw_set_backlight(bool on);

// Push a pixel buffer to a rectangular area on screen
void display_hw_push_pixels(int32_t x1, int32_t y1, uint32_t w, uint32_t h, const uint16_t* color_p);

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_DRIVER_H
