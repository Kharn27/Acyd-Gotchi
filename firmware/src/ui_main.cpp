/*
 * PIXEL - UI Main Implementation
 * 
 * UI module entry point: initializes theme, creates screens, manages state.
 */

#include "ui_api.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"
#include "archi/archi_stats.h"

#include <Arduino.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static QueueHandle_t g_ui_queue = NULL;
static ui_event_router_t g_event_router = NULL;
static bool g_theme_initialized = false;
static ui_screen_id_t g_current_screen = UI_SCREEN_MAIN;

static lv_obj_t* ui_build_screen(ui_screen_id_t target);
static lv_obj_t* ui_teardown_current_screen(void);

// Implementation of ui_api.h functions
void ui_init(QueueHandle_t ui_queue)
{
  g_ui_queue = ui_queue;
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

static lv_obj_t* ui_build_screen(ui_screen_id_t target)
{
  switch (target) {
    case UI_SCREEN_MAIN:
      return ui_create_main_screen();
    case UI_SCREEN_WIFI:
      return ui_create_wifi_screen();
    case UI_SCREEN_BLE:
      return ui_create_ble_screen();
    case UI_SCREEN_SETTINGS:
      return ui_create_settings_screen();
    case UI_SCREEN_MONITOR:
      return ui_create_monitor_screen();
    default:
      return NULL;
  }
}

// Remove the currently active screen to free LVGL allocations before building the next one.
// A tiny placeholder keeps LVGL happy while the new screen is constructed.
static lv_obj_t* ui_teardown_current_screen(void)
{
  lv_obj_t* old_screen = lv_scr_act();

  lv_obj_t* placeholder = lv_obj_create(NULL);
  if (!placeholder) {
    Serial.println("PIXEL: Failed to allocate placeholder screen for teardown");
    return NULL;
  }

  lv_obj_remove_style_all(placeholder);
  lv_obj_set_size(placeholder, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(placeholder, LV_OBJ_FLAG_SCROLLABLE);
  lv_scr_load(placeholder);

  if (old_screen && old_screen != placeholder) {
    lv_obj_del(old_screen);
  }

  // Run LVGL once to ensure deletion callbacks fire and memory is reclaimed.
  lv_timer_handler();
  return placeholder;
}

void ui_navigate_to(ui_screen_id_t target)
{
  if (!g_theme_initialized) {
    ui_theme_init();
    g_theme_initialized = true;
  }

  ui_screen_id_t previous = g_current_screen;
  archi_log_heap(" [UI] pre-nav");

  // Free current screen before constructing the next one to lower peak usage.
  lv_obj_t* placeholder = ui_teardown_current_screen();
  if (placeholder) {
    archi_log_heap(" [UI] post-teardown");
  } else {
    Serial.println("PIXEL: Navigation teardown skipped (placeholder alloc failed)");
  }

  lv_obj_t* new_screen = ui_build_screen(target);
  if (!new_screen) {
    Serial.println("PIXEL: Failed to build target screen");
    if (placeholder) {
      lv_obj_t* fallback = ui_build_screen(previous);
      if (fallback) {
        lv_scr_load(fallback);
        if (placeholder != fallback) {
          lv_obj_del(placeholder);
        }
        g_current_screen = previous;
        ui_set_bottom_bar_for_screen(previous);
      } else {
        lv_scr_load(placeholder);
      }
    }
    return;
  }

  Serial.printf("PIXEL: navigate %d -> %d\n", previous, target);
  lv_scr_load(new_screen);

  if (placeholder && placeholder != new_screen) {
    lv_obj_del(placeholder);
  }

  g_current_screen = target;
  ui_set_bottom_bar_for_screen(target);

  archi_log_heap(" [UI] post-load");

#ifdef UI_DEBUG_MEM
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  Serial.printf("[UI] free=%lu used=%lu max_used=%lu frag=%d%%\n",
                (unsigned long)mon.free_size,
                (unsigned long)(mon.total_size - mon.free_size),
                (unsigned long)mon.max_used,
                mon.frag_pct);
#endif
}

ui_screen_id_t ui_get_current_screen(void)
{
  return g_current_screen;
}

void ui_update_pet(uint32_t delta_ms)
{
  // Called periodically to update pet animation/state
  (void)delta_ms;
  
  // TODO: Update pet sprite/animation frame
  // For now, stub
}

