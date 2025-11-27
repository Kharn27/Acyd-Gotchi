#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "system_init.h"
#include "tasks.h"
#include "board_config.h"
#include "ui_api.h"
#include "netsec_api.h"

// Global queue handles for inter-task communication
QueueHandle_t ui_event_queue = NULL;
QueueHandle_t netsec_command_queue = NULL;
QueueHandle_t netsec_result_queue = NULL;

// Forward declarations of task implementations (will be filled in later)
// These are weak symbols to allow PIXEL and NETSEC to override if not yet implemented.
extern void ui_task(void* pvParameters);
extern void netsec_task(void* pvParameters);

/**
 * ARCHI-internal: Initialize board-level hardware.
 * Placeholder for now; will expand with GPIO, SPI bus setup, etc.
 */
void archi_init_board(void) {
    Serial.println("[ARCHI] Initializing board hardware...");
    // TODO(ARCHI): GPIO, SPI bus, power management, etc.
    Serial.println("[ARCHI] Board hardware OK");
}

/**
 * Main system initialization.
 * Creates FreeRTOS queues and starts UI and NETSEC tasks.
 * LVGL initialization is deferred to the UI task (via lvgl_port_init).
 */
void system_init(void) {
    Serial.println("[SYSTEM] Initializing system...");
    
    // Initialize board hardware (GPIO, SPI, etc.)
    archi_init_board();
    
    // Create inter-task communication queues
    Serial.println("[SYSTEM] Creating queues...");
    ui_event_queue = xQueueCreate(16, sizeof(ui_event_t));
    netsec_command_queue = xQueueCreate(8, sizeof(uint32_t));  // Simplified: commands are uint32_t opcodes
    netsec_result_queue = xQueueCreate(32, sizeof(netsec_result_t));
    
    if (!ui_event_queue || !netsec_command_queue || !netsec_result_queue) {
        Serial.println("[ERROR] Failed to create queues!");
        return;
    }
    
    Serial.println("[SYSTEM] Queues created");
    
    // Initialize UI module (passes queue handle)
    ui_init(ui_event_queue);
    
    // Initialize NETSEC module (passes result queue handle)
    netsec_init(netsec_result_queue);
    
    // Create UI task
    Serial.println("[SYSTEM] Creating UI task...");
    BaseType_t ui_result = xTaskCreatePinnedToCore(
        ui_task,
        "UI",
        UI_TASK_STACK_SIZE,
        NULL,
        UI_TASK_PRIORITY,
        NULL,
        1  // Core 1 (other core for UI, core 0 for other tasks)
    );
    
    if (ui_result != pdPASS) {
        Serial.println("[ERROR] Failed to create UI task!");
        return;
    }
    
    // Create NETSEC task
    Serial.println("[SYSTEM] Creating NETSEC task...");
    BaseType_t netsec_result = xTaskCreatePinnedToCore(
        netsec_task,
        "NETSEC",
        NETSEC_TASK_STACK_SIZE,
        NULL,
        NETSEC_TASK_PRIORITY,
        NULL,
        0  // Core 0
    );
    
    if (netsec_result != pdPASS) {
        Serial.println("[ERROR] Failed to create NETSEC task!");
        return;
    }
    
    Serial.println("[SYSTEM] System initialized successfully");
}

// Weak task implementations (will be overridden by PIXEL and NETSEC modules)
__attribute__((weak)) void ui_task(void* pvParameters) {
    Serial.println("[UI] Task started (stub)");
    vTaskDelay(pdMS_TO_TICKS(10000));  // Sleep to avoid spinning
    vTaskDelete(NULL);
}

__attribute__((weak)) void netsec_task(void* pvParameters) {
    Serial.println("[NETSEC] Task started (stub)");
    vTaskDelay(pdMS_TO_TICKS(10000));  // Sleep to avoid spinning
    vTaskDelete(NULL);
}

// Weak UI API implementations (will be overridden by PIXEL)
__attribute__((weak)) void ui_init(QueueHandle_t ui_queue) {
    Serial.println("[UI] Init stub (PIXEL will implement)");
}

__attribute__((weak)) void ui_show_main_screen(void) {
    Serial.println("[UI] Show main screen stub");
}

__attribute__((weak)) void ui_show_wifi_screen(void) {
    Serial.println("[UI] Show WiFi screen stub");
}

__attribute__((weak)) void ui_show_ble_screen(void) {
    Serial.println("[UI] Show BLE screen stub");
}

__attribute__((weak)) void ui_update_pet(uint32_t delta_ms) {
    // Stub
}

// Weak NETSEC API implementations (will be overridden by NETSEC)
__attribute__((weak)) void netsec_init(QueueHandle_t result_queue) {
    Serial.println("[NETSEC] Init stub (NETSEC will implement)");
}

__attribute__((weak)) void netsec_start_wifi_scan(void) {
    Serial.println("[NETSEC] Start WiFi scan stub");
}

__attribute__((weak)) void netsec_stop_wifi_scan(void) {
    Serial.println("[NETSEC] Stop WiFi scan stub");
}

__attribute__((weak)) void netsec_start_ble_scan(void) {
    Serial.println("[NETSEC] Start BLE scan stub");
}

__attribute__((weak)) void netsec_stop_ble_scan(void) {
    Serial.println("[NETSEC] Stop BLE scan stub");
}

__attribute__((weak)) bool netsec_request_handshake_capture(const netsec_wifi_ap_t* target) {
    Serial.println("[NETSEC] Handshake capture stub (lab mode disabled)");
    return false;
}

