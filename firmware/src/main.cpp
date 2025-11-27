#include <Arduino.h>
#include "system_init.h"
#include "board_config.h"

void setup() {
    // Initialize serial logging
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n\n=== Acyd-Gotchi Boot ===");
    Serial.println("Initializing system...");
    
    // Initialize board, LVGL, FreeRTOS infrastructure
    system_init();
    
    Serial.println("System init complete. Tasks running.");
}

void loop() {
    // In Arduino+FreeRTOS, the main loop is managed by FreeRTOS scheduler.
    // This function can remain mostly empty or perform global housekeeping.
    // Typically: nothing, or vTaskDelay(portMAX_DELAY).
    vTaskDelay(pdMS_TO_TICKS(1000));
}
