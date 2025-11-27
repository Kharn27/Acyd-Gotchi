/*
 * ARCHI - Touch Driver Header (XPT2046 wrapper for LVGL)
 * 
 * Non-blocking touch controller interface.
 * Reads XPT2046 and converts coordinates for LVGL input device.
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <cstdint>
#include <stdbool.h>
#include "board_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize touch controller: configure SPI, calibration if needed
void touch_init(void);

// Read touch data (non-blocking)
// Returns true if touch is pressed, false otherwise
// x, y: output coordinates (in display pixel space)
bool touch_read(uint16_t * x, uint16_t * y);

// Deinit touch controller
void touch_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // TOUCH_DRIVER_H
