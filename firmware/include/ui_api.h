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
    UI_EVENT_BUTTON_MONITOR, // Top bar monitor/stats button
} ui_event_t;

typedef enum {
    UI_SCREEN_MAIN = 0,
    UI_SCREEN_WIFI,
    UI_SCREEN_BLE,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_MONITOR,
} ui_screen_id_t;

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
 * Navigate to a logical screen. The previous LVGL screen is destroyed
 * and the requested screen is rebuilt from scratch.
 */
void ui_navigate_to(ui_screen_id_t target);

/**
 * Return the currently active logical screen.
 */
ui_screen_id_t ui_get_current_screen(void);

/**
 * Update the pet animation (called periodically from ui_task).
 * @param delta_ms: milliseconds since last call
 */
void ui_update_pet(uint32_t delta_ms);
