#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// UI Module API
// PIXEL is responsible for implementing these functions.

/* UI event types posted by buttons to ui_event_queue */
typedef enum {
    UI_EV_NONE = 0,
    UI_EV_WIFI_OPEN,        // Top-left button: open WiFi screen
    UI_EV_BT_OPEN,          // Top-middle button: open Bluetooth screen
    UI_EV_SELECT_ENTRY,     // User selected an entry (WiFi AP or BLE device)
    UI_EV_BACK,             // Back button or escape
} ui_event_t;

/**
 * Initialize UI module.
 * Must be called from ui_task context once LVGL is ready.
 * @param ui_queue: queue to post UI events to
 */
void ui_init(QueueHandle_t ui_queue);

/**
 * Display the main screen (pet + button bands).
 * Called after ui_init().
 */
void ui_show_main_screen(void);

/**
 * Display the WiFi scan results screen.
 * Called when user taps WiFi button.
 */
void ui_show_wifi_screen(void);

/**
 * Display the Bluetooth scan results screen.
 * Called when user taps Bluetooth button.
 */
void ui_show_ble_screen(void);

/**
 * Update the pet animation (called periodically from ui_task).
 * @param delta_ms: milliseconds since last call
 */
void ui_update_pet(uint32_t delta_ms);
