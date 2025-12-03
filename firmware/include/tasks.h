#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Task prototypes and inter-task communication handles.
// Tasks are started in system_init.cpp.

/* Queue depth configuration (kept here so both tasks and producers share the same limits) */
#define UI_EVENT_QUEUE_LENGTH       32
#define NETSEC_COMMAND_QUEUE_LENGTH 12
#define NETSEC_RESULT_QUEUE_LENGTH  96

/* UI task: handles LVGL event loop and display updates */
void ui_task(void* pvParameters);

/* NETSEC task: handles WiFi/BLE scanning and network operations (non-blocking) */
void netsec_task(void* pvParameters);

/* Queue handles for inter-task communication */
extern QueueHandle_t ui_event_queue;          // UI posts events from buttons
extern QueueHandle_t netsec_command_queue;    // UI sends commands to NETSEC
extern QueueHandle_t netsec_result_queue;     // NETSEC sends scan results back
