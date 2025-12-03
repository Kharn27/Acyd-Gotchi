#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// UI Module API
// PIXEL is responsible for implementing these functions.

/* UI event types posted by buttons to ui_event_queue */
typedef enum {
    UI_EVENT_NONE = 0,
    UI_EVENT_BUTTON_WIFI,    // Top bar WiFi button
    UI_EVENT_BUTTON_BLE,     // Top bar BLE button
    UI_EVENT_BUTTON_MENU,    // Bottom bar menu button
    UI_EVENT_BLE_SCAN_REQUEST, // User tapped BLE scan entrypoint
    UI_EVENT_BLE_DURATION_SELECTED_10S,
    UI_EVENT_BLE_DURATION_SELECTED_20S,
    UI_EVENT_BLE_DURATION_SELECTED_30S,
    UI_EVENT_BLE_CANCEL,     // User canceled a pending scan
    UI_EVENT_BLE_SCAN_DONE,  // Scan finished notification
    UI_EVENT_SELECT_ENTRY,   // Generic list selection
    UI_EVENT_BACK,           // Back or escape navigation
    UI_EVENT_UPDATE_PET,     // Periodic pet refresh
} ui_event_t;

typedef void (*ui_event_router_t)(ui_event_t event);

/**
 * Initialize UI module.
 * Must be called from ui_task context once LVGL is ready.
 * @param ui_queue: queue to post UI events to
 */
void ui_init(QueueHandle_t ui_queue);

/**
 * Register a router to be notified of UI events popped by ui_task.
 * Allows other modules to react without ui_task depending on them.
 */
void ui_register_event_router(ui_event_router_t router);

/**
 * Get the currently registered event router (may be NULL).
 */
ui_event_router_t ui_get_event_router(void);

/**
 * Post an event to the UI event queue (non-blocking).
 * @param event: event to send
 * @return true if enqueued, false otherwise
 */
bool ui_post_event(ui_event_t event);

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
 * Display the Settings screen.
 * Called when user taps Menu.
 */
void ui_show_settings_screen(void);

/**
 * Update the pet animation (called periodically from ui_task).
 * @param delta_ms: milliseconds since last call
 */
void ui_update_pet(uint32_t delta_ms);
