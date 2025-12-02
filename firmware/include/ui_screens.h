/*
 * PIXEL - UI Screens Public API
 * 
 * All screen creation and management functions.
 */

#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create main screen (pet display + button bands)
lv_obj_t* ui_create_main_screen(void);

// Create WiFi scan results screen
lv_obj_t* ui_create_wifi_screen(void);

// Create BLE scan results screen
lv_obj_t* ui_create_ble_screen(void);
lv_obj_t* ui_ble_get_scan_button(void);

// Create settings screen (placeholder)
lv_obj_t* ui_create_settings_screen(void);

// Load a screen (switch display to given object)
void ui_load_screen(lv_obj_t* screen);

// Get current active screen
lv_obj_t* ui_get_active_screen(void);

// Update bottom button state depending on active screen
void ui_set_screen_state_to_main(void);
void ui_set_screen_state_to_wifi(void);
void ui_set_screen_state_to_ble(void);

#ifdef __cplusplus
}
#endif

#endif // UI_SCREENS_H
