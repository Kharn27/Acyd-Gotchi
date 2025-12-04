#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// NETSEC Core API

#ifdef __cplusplus
extern "C" {
#endif

// Initialize NETSEC module
void netsec_init(QueueHandle_t result_queue);

// NETSEC task entrypoint
void netsec_task(void* pvParameters);

#ifdef __cplusplus
} // extern "C"
#endif
