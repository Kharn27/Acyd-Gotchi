/*
 * ARCHI - LVGL Port Implementation
 * 
 * Initializes LVGL on ESP32-CYD:
 * - Calls lv_init()
 * - Registers display and input drivers
 * - Sets up tick timer via esp_timer
 */
extern "C" {
  #include "lvgl.h"
}

#include "lvgl_port.h"
#include "display_driver.h"
#include "board_config.h"

//#include "lvgl.h"
#include "esp_timer.h"

#include <stdio.h>

// Static references for drivers
static lv_disp_t* g_disp = NULL;
static lv_indev_t* g_indev_touch = NULL;
static esp_timer_handle_t g_tick_timer = NULL;

// LVGL tick timer callback (1 ms tick)
static void lvgl_tick_timer_cb(void* arg) {
  // Avec LV_TICK_CUSTOM = 1, on peut appeler directement le handler
  // ici au lieu de lv_tick_inc().
  lv_timer_handler();
}

// LVGL tick setup via esp_timer
static void lvgl_setup_tick(void)
{
  esp_timer_create_args_t timer_args = {
    .callback = lvgl_tick_timer_cb,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "lvgl_tick"
  };
  
  if (esp_timer_create(&timer_args, &g_tick_timer) != ESP_OK) {
    printf("ERROR: Failed to create LVGL tick timer\n");
    return;
  }
  
  // Start timer to fire every 1 ms
  if (esp_timer_start_periodic(g_tick_timer, 1000) != ESP_OK) {
    printf("ERROR: Failed to start LVGL tick timer\n");
  }
}

void lvgl_port_init(void)
{
  // Initialize LVGL library
  lv_init();

  printf("ARCHI: Initializing display driver...\n");
  // Initialize display (TFT_eSPI wrapper)
  display_init();

  // Cache registered handles for consumers
  g_disp = display_get_disp();
  g_indev_touch = display_get_indev_touch();

  if (!g_disp || !g_indev_touch) {
    printf("ERROR: LVGL port missing disp/indev handles (check display_init)\n");
  }

  printf("ARCHI: Setting up LVGL tick timer...\n");
  // Setup 1 ms tick via esp_timer
  lvgl_setup_tick();

  printf("ARCHI: LVGL port initialized\n");
}

void lvgl_port_deinit(void)
{
  if (g_tick_timer) {
    esp_timer_stop(g_tick_timer);
    esp_timer_delete(g_tick_timer);
    g_tick_timer = NULL;
  }
  
  display_deinit();
  
  printf("ARCHI: LVGL port deinitialized\n");
}

lv_disp_t* lvgl_port_get_disp(void)
{
  return g_disp;
}

lv_indev_t* lvgl_port_get_indev_touch(void)
{
  return g_indev_touch;
}

// Weak functions: display_init must set these
void __attribute__((weak)) display_init(void) {}
void __attribute__((weak)) display_deinit(void) {}

