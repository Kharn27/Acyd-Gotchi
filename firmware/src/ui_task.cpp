/*
 * ARCHI - UI Task Implementation
 * 
 * High-priority FreeRTOS task for LVGL rendering loop.
 * Calls lv_timer_handler() periodically (~5-10 ms).
 * Runs on core 1 (higher priority for UI responsiveness).
 */

#include "tasks.h"
#include "ui_api.h"
#include "ui_screens.h"
#include "netsec_api.h"
#include "lvgl_port.h"

#include "lvgl.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <Arduino.h>

typedef enum {
  BLE_UI_STATE_IDLE = 0,
  BLE_UI_STATE_CHOOSING_DURATION,
  BLE_UI_STATE_SCANNING,
} ble_ui_state_t;

static ble_ui_state_t g_ble_ui_state = BLE_UI_STATE_IDLE;

static void ui_handle_ble_duration_selection(uint32_t duration_s)
{
  const uint32_t duration_ms = duration_s * 1000;
  ui_ble_show_scan_request(duration_ms);
  g_ble_ui_state = BLE_UI_STATE_SCANNING;

  if (netsec_command_queue) {
    netsec_command_t cmd = { NETSEC_CMD_NONE };
    cmd.type = NETSEC_CMD_BLE_SCAN_START;
    cmd.data.ble_scan_start.duration_ms = duration_ms;
    xQueueSend(netsec_command_queue, &cmd, 0);
  }
}

void ui_task(void * pvParameters)
{
  (void)pvParameters;
  
  Serial.printf("UI Task started on core %d, priority %d\n",
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

    // Handle NETSEC results (non-blocking)
    netsec_result_t netsec_res;
    while (xQueueReceive(netsec_result_queue, &netsec_res, 0) == pdTRUE) {
      switch (netsec_res.type) {
        case NETSEC_RES_WIFI_AP:
          ui_wifi_handle_ap_found(&netsec_res.data.wifi_ap);
          break;
        case NETSEC_RES_WIFI_SCAN_DONE:
          ui_wifi_handle_scan_done();
          break;
        case NETSEC_RES_BLE_SCAN_STARTED:
          ui_ble_handle_scan_started(&netsec_res.data.scan_summary);
          g_ble_ui_state = BLE_UI_STATE_SCANNING;
          break;
        case NETSEC_RES_BLE_DEVICE_FOUND:
          ui_ble_handle_device_found(&netsec_res.data.ble_device);
          break;
        case NETSEC_RES_BLE_SCAN_COMPLETED:
          ui_ble_handle_scan_completed(&netsec_res.data.scan_summary);
          g_ble_ui_state = BLE_UI_STATE_IDLE;
          break;
        default:
          break;
      }
    }

    // Handle UI events from queue (non-blocking)
    ui_event_t event;
    if (xQueueReceive(ui_event_queue, &event, 0) == pdTRUE) {
      ui_event_router_t router = ui_get_event_router();
      if (router) {
        router(event);
      }

      // Process UI event
      switch (event) {
        case UI_EVENT_BUTTON_WIFI:
          Serial.println("UI Event: WiFi button pressed");
          ui_show_wifi_screen();
          break;

        case UI_EVENT_BUTTON_BLE:
          Serial.println("UI Event: BLE button pressed");
          ui_show_ble_screen();
          ui_ble_set_state_idle();
          g_ble_ui_state = BLE_UI_STATE_IDLE;
          break;

        case UI_EVENT_BUTTON_MENU:
          Serial.println("UI Event: Menu button pressed");
          ui_show_settings_screen();
          break;

        case UI_EVENT_BLE_SCAN_REQUEST:
          Serial.println("UI Event: BLE scan request");
          ui_ble_set_state_choosing_duration();
          g_ble_ui_state = BLE_UI_STATE_CHOOSING_DURATION;
          break;

        case UI_EVENT_BLE_DURATION_SELECTED_10S:
          Serial.println("UI Event: BLE duration 10s selected");
          ui_handle_ble_duration_selection(10);
          break;

        case UI_EVENT_BLE_DURATION_SELECTED_20S:
          Serial.println("UI Event: BLE duration 20s selected");
          ui_handle_ble_duration_selection(20);
          break;

        case UI_EVENT_BLE_DURATION_SELECTED_30S:
          Serial.println("UI Event: BLE duration 30s selected");
          ui_handle_ble_duration_selection(30);
          break;

        case UI_EVENT_BLE_CANCEL:
          Serial.println("UI Event: BLE scan cancel requested");
          if (g_ble_ui_state == BLE_UI_STATE_SCANNING && netsec_command_queue) {
            netsec_command_t cmd = { .type = NETSEC_CMD_BLE_SCAN_CANCEL };
            xQueueSend(netsec_command_queue, &cmd, 0);
          }
          ui_ble_cancel_scan();
          g_ble_ui_state = BLE_UI_STATE_IDLE;
          break;

        case UI_EVENT_BLE_SCAN_DONE:
          Serial.println("UI Event: BLE scan done");
          ui_ble_handle_scan_completed(NULL);
          g_ble_ui_state = BLE_UI_STATE_IDLE;
          break;

        case UI_EVENT_UPDATE_PET:
          Serial.println("UI Event: Update pet");
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

