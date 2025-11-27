/*
 * PIXEL - UI Main Implementation
 * 
 * UI module entry point: initializes theme, creates screens, manages state.
 */

#include "ui_api.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "lvgl.h"

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static QueueHandle_t g_ui_queue = NULL;
static lv_obj_t* g_main_screen = NULL;
static lv_obj_t* g_wifi_screen = NULL;
static lv_obj_t* g_ble_screen = NULL;

// Implementation of ui_api.h functions
void ui_init(QueueHandle_t ui_queue)
{
  g_ui_queue = ui_queue;
  printf("PIXEL: UI module initialized\n");
}

void ui_show_main_screen(void)
{
  printf("PIXEL: Showing main screen\n");
  
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
}

void ui_show_wifi_screen(void)
{
  printf("PIXEL: Showing WiFi screen\n");
  
  if (!g_wifi_screen) {
    g_wifi_screen = ui_create_wifi_screen();
  }
  
  ui_load_screen(g_wifi_screen);
}

void ui_show_ble_screen(void)
{
  printf("PIXEL: Showing BLE screen\n");
  
  if (!g_ble_screen) {
    g_ble_screen = ui_create_ble_screen();
  }
  
  ui_load_screen(g_ble_screen);
}

void ui_update_pet(uint32_t delta_ms)
{
  // Called periodically to update pet animation/state
  (void)delta_ms;
  
  // TODO: Update pet sprite/animation frame
  // For now, stub
}

