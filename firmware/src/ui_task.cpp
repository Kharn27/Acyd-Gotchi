/*
 * ARCHI - UI Task Implementation
 * 
 * High-priority FreeRTOS task for LVGL rendering loop.
 * Calls lv_timer_handler() periodically (~5-10 ms).
 * Runs on core 1 (higher priority for UI responsiveness).
 */

#include "tasks.h"
#include "ui_api.h"
#include "lvgl_port.h"

#include "lvgl.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdio.h>

void ui_task(void * pvParameters)
{
  (void)pvParameters;
  
  printf("UI Task started on core %d, priority %d\n", 
         xPortGetCoreID(), uxTaskPriorityGet(NULL));
  
  // Initialize LVGL and drivers
  lvgl_port_init();
  
  // Show main screen
  ui_show_main_screen();
  
  // Task loop: call lv_timer_handler() every ~5 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(5);  // 5 ms period
  
  while (1) {
    // Process LVGL internal timers and redraw
    lv_timer_handler();
    
    // Handle UI events from queue (non-blocking)
    ui_event_t event;
    if (xQueueReceive(ui_event_queue, &event, 0) == pdTRUE) {
      // Process UI event
      switch (event) {
        case UI_EVENT_BUTTON_WIFI:
          printf("UI Event: WiFi button pressed\n");
          ui_show_wifi_screen();
          break;
        
        case UI_EVENT_BUTTON_BLE:
          printf("UI Event: BLE button pressed\n");
          ui_show_ble_screen();
          break;
        
        case UI_EVENT_UPDATE_PET:
          printf("UI Event: Update pet\n");
          // Pet state updated, LVGL will redraw on next cycle
          break;
        
        default:
          break;
      }
    }
    
    // Delay until next cycle
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

