/*
 * PIXEL - UI Screens Public API
 * 
 * All screen creation and management functions.
 */

#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "lvgl.h"
#include "netsec_api.h"
#include "ui_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create main screen (pet display + button bands)
lv_obj_t* ui_build_main_screen(void);
void ui_main_screen_on_unload(void);
void ui_apply_bottom_button_for_screen(ui_screen_id_t screen_id);

// Create WiFi scan results screen
lv_obj_t* ui_build_wifi_screen(void);
void ui_wifi_on_unload(void);
void ui_wifi_handle_ap_found(const netsec_wifi_ap_t* ap);
void ui_wifi_handle_scan_done(void);

// Create BLE scan results screen
lv_obj_t* ui_build_ble_screen(void);
void ui_ble_on_unload(void);
lv_obj_t* ui_ble_get_scan_button(void);
void ui_ble_prepare_for_scan(uint32_t duration_ms);
void ui_ble_handle_device_found(const netsec_ble_device_t* device);
void ui_ble_handle_scan_started(const netsec_scan_summary_t* meta);
void ui_ble_handle_scan_completed(const netsec_scan_summary_t* meta);
void ui_ble_handle_scan_error_memory(void);
uint32_t ui_ble_get_last_scan_duration_ms(void);
void ui_ble_set_state_idle(void);
void ui_ble_set_state_choosing_duration(void);
void ui_ble_set_state_scanning(uint32_t duration_ms);
void ui_ble_show_scan_request(uint32_t duration_ms);
void ui_ble_cancel_scan(void);

// Create settings screen (placeholder)
lv_obj_t* ui_build_settings_screen(void);
void ui_settings_on_unload(void);

// Create monitoring screen (system stats)
lv_obj_t* ui_build_monitor_screen(void);
void ui_monitor_on_unload(void);

// Bottom button helpers shared across screens
void ui_bottom_button_set(const char* label, lv_event_cb_t handler);
void ui_bottom_button_restore(void);

#ifdef __cplusplus
}
#endif

#endif // UI_SCREENS_H
