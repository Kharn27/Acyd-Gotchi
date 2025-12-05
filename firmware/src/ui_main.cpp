/*
 * PIXEL - UI Main Implementation
 * 
 * UI module entry point: initializes theme, creates screens, manages state.
 */

#include "ui_api.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#ifndef UI_DEBUG_MEM
#define UI_DEBUG_MEM 0
#endif

static QueueHandle_t g_ui_queue = NULL;
static ui_event_router_t g_event_router = NULL;
static UiScreenId g_current_screen = UI_SCREEN_MAIN;
static lv_obj_t* g_active_screen = NULL;
static bool g_theme_initialized = false;

static const char* screen_id_to_str(UiScreenId id)
{
  switch (id) {
    case UI_SCREEN_MAIN: return "MAIN";
    case UI_SCREEN_WIFI: return "WIFI";
    case UI_SCREEN_BLE: return "BLE";
    case UI_SCREEN_SETTINGS: return "SETTINGS";
    case UI_SCREEN_MONITOR: return "MONITOR";
    default: return "UNKNOWN";
  }
}

static void ensure_theme_initialized(void)
{
  if (!g_theme_initialized) {
    ui_theme_init();
    g_theme_initialized = true;
  }
}

static lv_obj_t* build_screen(UiScreenId id)
{
  switch (id) {
    case UI_SCREEN_MAIN:
      ensure_theme_initialized();
      return ui_build_main_screen();
    case UI_SCREEN_WIFI:
      return ui_build_wifi_screen();
    case UI_SCREEN_BLE:
      return ui_build_ble_screen();
    case UI_SCREEN_SETTINGS:
      return ui_build_settings_screen();
    case UI_SCREEN_MONITOR:
      return ui_build_monitor_screen();
    default:
      return NULL;
  }
}

static void apply_screen_state(UiScreenId id)
{
  switch (id) {
    case UI_SCREEN_MAIN:
      ui_set_screen_state_to_main();
      break;
    case UI_SCREEN_WIFI:
      ui_set_screen_state_to_wifi();
      break;
    case UI_SCREEN_BLE:
      ui_set_screen_state_to_ble();
      break;
    case UI_SCREEN_SETTINGS:
      ui_set_screen_state_to_settings();
      break;
    case UI_SCREEN_MONITOR:
      ui_set_screen_state_to_monitor();
      break;
    default:
      break;
  }
}

static void teardown_screen(UiScreenId id)
{
  switch (id) {
    case UI_SCREEN_MAIN:
      ui_teardown_main_screen();
      break;
    case UI_SCREEN_BLE:
      ui_teardown_ble_screen();
      break;
    case UI_SCREEN_MONITOR:
      ui_teardown_monitor_screen();
      break;
    default:
      break;
  }
}

static ui_event_t event_for_screen(UiScreenId target)
{
  switch (target) {
    case UI_SCREEN_MAIN: return UI_EVENT_NAVIGATE_MAIN;
    case UI_SCREEN_WIFI: return UI_EVENT_NAVIGATE_WIFI;
    case UI_SCREEN_BLE: return UI_EVENT_NAVIGATE_BLE;
    case UI_SCREEN_SETTINGS: return UI_EVENT_NAVIGATE_SETTINGS;
    case UI_SCREEN_MONITOR: return UI_EVENT_NAVIGATE_MONITOR;
    default: return UI_EVENT_NONE;
  }
}

// Implementation of ui_api.h functions
void ui_init(QueueHandle_t ui_queue)
{
  g_ui_queue = ui_queue;
  g_current_screen = UI_SCREEN_MAIN;
  g_active_screen = NULL;
  g_theme_initialized = false;
  Serial.println("PIXEL: UI module initialized");
}

void ui_register_event_router(ui_event_router_t router)
{
  g_event_router = router;
}

ui_event_router_t ui_get_event_router(void)
{
  return g_event_router;
}

bool ui_post_event(ui_event_t event)
{
  if (!g_ui_queue) {
    Serial.println("PIXEL: UI event queue not initialized");
    return false;
  }

  BaseType_t result = xQueueSend(g_ui_queue, &event, 0);
  if (result != pdTRUE) {
    Serial.println("PIXEL: Failed to post UI event");
    return false;
  }

  return true;
}

bool ui_request_navigation(UiScreenId target)
{
  ui_event_t nav_event = event_for_screen(target);
  if (nav_event == UI_EVENT_NONE) {
    return false;
  }

  return ui_post_event(nav_event);
}

void ui_navigate_to(UiScreenId target)
{
  const UiScreenId previous = g_current_screen;
  Serial.printf("UI: navigate %s -> %s\n", screen_id_to_str(previous), screen_id_to_str(target));

  lv_obj_t* previous_screen = g_active_screen;
  if (previous_screen && previous != target) {
    teardown_screen(previous);
  }
  lv_obj_t* new_screen = build_screen(target);
  if (!new_screen) {
    Serial.println("UI: failed to build target screen");
    return;
  }

  g_active_screen = new_screen;
  lv_scr_load(new_screen);
  apply_screen_state(target);

  if (previous_screen && previous_screen != new_screen) {
    lv_obj_del(previous_screen);
    Serial.println("UI: destroyed previous screen");
  }

  g_current_screen = target;

#if UI_DEBUG_MEM
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Serial.printf("[UI] free=%lu used=%lu max_used=%lu frag=%d%%\n",
                (unsigned long)mon.free_size,
                (unsigned long)(mon.total_size - mon.free_size),
                (unsigned long)mon.max_used,
                mon.frag_pct);
#endif
}

void ui_update_pet(uint32_t delta_ms)
{
  // Called periodically to update pet animation/state
  (void)delta_ms;
  
  // TODO: Update pet sprite/animation frame
  // For now, stub
}

UiScreenId ui_get_current_screen(void)
{
  return g_current_screen;
}

