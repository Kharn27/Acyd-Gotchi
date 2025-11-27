/*
 * ARCHI - LVGL Port Header
 * 
 * Provides initialization and management of LVGL on ESP32-CYD.
 * Handles display driver registration, input device setup, and tick timer.
 */

#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize LVGL subsystem: display driver, input device, tick timer
void lvgl_port_init(void);

// Deinit LVGL and cleanup resources
void lvgl_port_deinit(void);

// Get the registered display object (useful for UI layer)
lv_disp_t* lvgl_port_get_disp(void);

// Get the registered input device (touch) object
lv_indev_t* lvgl_port_get_indev_touch(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_PORT_H
