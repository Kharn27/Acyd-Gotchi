// NETSEC Module - Core implementation
// Bridges the high-level netsec API to concrete wifi/ble modules and runs
// a simple command loop in the `netsec_task`.

#include <Arduino.h>
#include "netsec_core.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif
#include "netsec_wifi.h"
#include "netsec_ble.h"
#include "board_config.h"

static QueueHandle_t local_result_queue = NULL;

void netsec_init(QueueHandle_t result_queue) {
    Serial.println("[NETSEC] Network security module initialized");
    local_result_queue = result_queue;
    // Early init for WiFi stack if needed
#if defined(ARDUINO_ARCH_ESP32)
    // Ensure WiFi is in STA mode for scanning
    WiFi.mode(WIFI_MODE_STA);
    WiFi.disconnect();
#endif
}

// High-level API: start/stop delegated to netsec_wifi/netsec_ble modules
void netsec_start_wifi_scan(void) {
    Serial.println("[NETSEC] WiFi scan requested");
    netsec_wifi_start_scan();
}

void netsec_stop_wifi_scan(void) {
    Serial.println("[NETSEC] WiFi scan stop requested");
    netsec_wifi_stop_scan();
}

void netsec_start_ble_scan(void) {
    Serial.println("[NETSEC] BLE scan requested");
    netsec_ble_start_scan();
}

void netsec_stop_ble_scan(void) {
    Serial.println("[NETSEC] BLE scan stop requested");
    netsec_ble_stop_scan();
}

bool netsec_request_handshake_capture(const netsec_wifi_ap_t* target) {
    if (!LAB_MODE_ENABLED) {
        Serial.println("[NETSEC] Handshake capture rejected: lab mode disabled");
        return false;
    }

    Serial.print("[NETSEC] Handshake capture requested for SSID: ");
    Serial.println(target->ssid);
    // TODO: implement controlled handshake capture (lab only)
    return true;
}

// netsec_task: simple command processor (start/stop scans)
// Commands are posted to `netsec_command_queue` as uint32_t opcodes.
extern QueueHandle_t netsec_command_queue;
void netsec_task(void* pvParameters) {
    (void)pvParameters;
    Serial.println("[NETSEC] Task started");
    uint32_t cmd;
    for (;;) {
        if (xQueueReceive(netsec_command_queue, &cmd, pdMS_TO_TICKS(1000)) == pdTRUE) {
            switch (cmd) {
                case 1: // START_WIFI_SCAN
                    netsec_start_wifi_scan();
                    break;
                case 2: // STOP_WIFI_SCAN
                    netsec_stop_wifi_scan();
                    break;
                case 3: // START_BLE_SCAN
                    netsec_start_ble_scan();
                    break;
                case 4: // STOP_BLE_SCAN
                    netsec_stop_ble_scan();
                    break;
                default:
                    Serial.printf("[NETSEC] Unknown command %u\n", cmd);
                    break;
            }
        }
        // Periodically could check scan results and post to local_result_queue
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
