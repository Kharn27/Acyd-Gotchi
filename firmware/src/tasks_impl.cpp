#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "tasks.h"
#include "ui_api.h"
#include "netsec_api.h"

/**
 * UI Task: Runs LVGL event loop and displays main screen.
 * Must call lv_task_handler() periodically (~5-10 ms).
 */
void ui_task(void* pvParameters) {
    Serial.println("[UI_TASK] Started");
    
    // Show main screen after LVGL is initialized
    ui_show_main_screen();
    
    // Main event loop
    uint32_t last_pet_update = millis();
    
    while (1) {
        // Call LVGL task handler (low-level event processing)
        // TODO(PIXEL): Uncomment when LVGL is initialized
        // lv_task_handler();
        
        // Update pet animation at ~30 Hz (every 33ms)
        uint32_t now = millis();
        if (now - last_pet_update > 33) {
            uint32_t delta = now - last_pet_update;
            ui_update_pet(delta);
            last_pet_update = now;
        }
        
        // Small delay to prevent task starvation
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/**
 * NETSEC Task: Runs network security operations (WiFi/BLE scans, captures).
 * Non-blocking; operates via queue commands and pushes results to queue.
 */
void netsec_task(void* pvParameters) {
    Serial.println("[NETSEC_TASK] Started");
    
    // Main event loop
    while (1) {
        // TODO(NETSEC): Poll for commands in netsec_command_queue
        // TODO(NETSEC): Perform WiFi/BLE scans asynchronously
        // TODO(NETSEC): Push results to netsec_result_queue
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Periodic check
    }
}
