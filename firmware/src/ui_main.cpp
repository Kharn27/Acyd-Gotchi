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
static lv_obj_t* g_main_screen = NULL;
static lv_obj_t* g_wifi_screen = NULL;
static lv_obj_t* g_ble_screen = NULL;
static lv_obj_t* g_settings_screen = NULL;

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

void ui_show_main_screen(void)
{
  Serial.println("PIXEL: Showing main screen");
  
  if (!g_main_screen) {
    // Initialize theme first
    ui_theme_init();

    // Create main screen
    g_main_screen = ui_create_main_screen();

    // Load it
    ui_load_screen(g_main_screen);
  } else {
    ui_load_screen(g_main_screen);
  }

  ui_set_screen_state_to_main();
}

void ui_show_wifi_screen(void)
{
  Serial.println("PIXEL: Showing WiFi screen");
  
  if (!g_wifi_screen) {
    g_wifi_screen = ui_create_wifi_screen();
  }

  ui_set_screen_state_to_wifi();
  ui_load_screen(g_wifi_screen);
}

void ui_show_ble_screen(void)
{
  Serial.println("PIXEL: Showing BLE screen");
  
  if (!g_ble_screen) {
    g_ble_screen = ui_create_ble_screen();
  }

  ui_set_screen_state_to_ble();
  ui_load_screen(g_ble_screen);
}

void ui_show_settings_screen(void)
{
  Serial.println("PIXEL: Showing Settings screen");

  if (!g_settings_screen) {
    g_settings_screen = ui_create_settings_screen();
  }

  ui_set_screen_state_to_settings();
  ui_load_screen(g_settings_screen);
}

void ui_update_pet(uint32_t delta_ms)
{
  // Called periodically to update pet animation/state
  (void)delta_ms;
  
  // TODO: Update pet sprite/animation frame
  // For now, stub
}

