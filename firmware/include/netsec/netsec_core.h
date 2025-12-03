#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// NETSEC Core API

// Initialize NETSEC module
void netsec_init(QueueHandle_t result_queue);

// NETSEC task entrypoint
void netsec_task(void* pvParameters);
