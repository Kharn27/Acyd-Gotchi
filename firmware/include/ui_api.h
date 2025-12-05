#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// UI Module API
// PIXEL is responsible for implementing these functions.

/* Logical identifiers for screens. Navigation is expressed via this enum,
 * never via cached lv_obj_t* pointers. */
typedef enum {
    UI_SCREEN_MAIN = 0,
    UI_SCREEN_WIFI,
    UI_SCREEN_BLE,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_MONITOR,
} UiScreenId;

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
    UI_EVENT_NAVIGATE_MAIN,
    UI_EVENT_NAVIGATE_WIFI,
    UI_EVENT_NAVIGATE_BLE,
    UI_EVENT_NAVIGATE_SETTINGS,
    UI_EVENT_NAVIGATE_MONITOR,
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
 * Request a navigation from any producer context. The navigation will be
 * executed inside ui_task when the event is drained.
 * @param target: logical screen identifier
 */
bool ui_request_navigation(UiScreenId target);

/**
 * Synchronously navigate to a target screen from within ui_task.
 * Destroys the current screen and builds the new one.
 */
void ui_navigate_to(UiScreenId target);

/**
 * Get the currently displayed logical screen.
 */
UiScreenId ui_get_current_screen(void);

/**
 * Update the pet animation (called periodically from ui_task).
 * @param delta_ms: milliseconds since last call
 */
void ui_update_pet(uint32_t delta_ms);
