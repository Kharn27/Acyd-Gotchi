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

static QueueHandle_t g_ui_queue = NULL;
static ui_event_router_t g_event_router = NULL;
static lv_obj_t* g_active_screen = NULL;
static ui_screen_id_t g_current_screen = UI_SCREEN_MAIN;
static bool g_theme_ready = false;

static lv_obj_t* ui_build_screen(ui_screen_id_t id);
static void ui_cleanup_screen(ui_screen_id_t id);

// Implementation of ui_api.h functions
void ui_init(QueueHandle_t ui_queue)
{
  g_ui_queue = ui_queue;
  if (!g_theme_ready) {
    ui_theme_init();
    g_theme_ready = true;
  }
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

bool ui_request_navigation(ui_screen_id_t target)
{
  switch (target) {
    case UI_SCREEN_MAIN:
      return ui_post_event(UI_EVENT_BACK);
    case UI_SCREEN_WIFI:
      return ui_post_event(UI_EVENT_BUTTON_WIFI);
    case UI_SCREEN_BLE:
      return ui_post_event(UI_EVENT_BUTTON_BLE);
    case UI_SCREEN_SETTINGS:
      return ui_post_event(UI_EVENT_BUTTON_MENU);
    case UI_SCREEN_MONITOR:
      return ui_post_event(UI_EVENT_BUTTON_MONITOR);
    default:
      return false;
  }
}

ui_screen_id_t ui_get_current_screen(void)
{
  return g_current_screen;
}

void ui_navigate_to(ui_screen_id_t target)
{
  Serial.printf("PIXEL: navigate %d -> %d\n", g_current_screen, target);

  if (!g_theme_ready) {
    ui_theme_init();
    g_theme_ready = true;
  }

  lv_obj_t* old_screen = g_active_screen;
  ui_cleanup_screen(g_current_screen);
  g_active_screen = NULL;

  if (old_screen) {
    lv_obj_del(old_screen);
  }

  lv_obj_t* new_screen = ui_build_screen(target);
  if (!new_screen) {
    Serial.println("PIXEL: failed to build target screen");
    return;
  }

  lv_scr_load(new_screen);

  g_active_screen = new_screen;
  g_current_screen = target;
  ui_apply_bottom_button_for_screen(target);

#ifdef UI_DEBUG_MEM
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  printf("[UI] free=%lu used=%lu max_used=%lu frag=%d%%\n",
         (unsigned long)mon.free_size,
         (unsigned long)(mon.total_size - mon.free_size),
         (unsigned long)mon.max_used,
         mon.frag_pct);
#endif
}

void ui_update_pet(uint32_t delta_ms)
{
  (void)delta_ms;
}

static lv_obj_t* ui_build_screen(ui_screen_id_t id)
{
  switch (id) {
    case UI_SCREEN_MAIN:
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

static void ui_cleanup_screen(ui_screen_id_t id)
{
  switch (id) {
    case UI_SCREEN_MAIN:
      ui_main_screen_on_unload();
      break;
    case UI_SCREEN_WIFI:
      ui_wifi_on_unload();
      break;
    case UI_SCREEN_BLE:
      ui_ble_on_unload();
      break;
    case UI_SCREEN_SETTINGS:
      ui_settings_on_unload();
      break;
    case UI_SCREEN_MONITOR:
      ui_monitor_on_unload();
      break;
    default:
      break;
  }
}

