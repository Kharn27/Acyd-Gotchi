// NETSEC Module - Core implementation
// Bridges the high-level netsec API to concrete wifi/ble modules and runs
// a simple command loop in the `netsec_task`.

#include <Arduino.h>
#include "netsec_core.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#endif
#include "netsec_api.h"
#include "netsec_wifi.h"
#include "netsec_ble.h"
#include "board_config.h"

static QueueHandle_t local_result_queue = NULL;

static const char* netsec_cmd_str(netsec_command_type_t type) {
    switch (type) {
        case NETSEC_CMD_WIFI_SCAN_START: return "WIFI_SCAN_START";
        case NETSEC_CMD_WIFI_SCAN_STOP: return "WIFI_SCAN_STOP";
        case NETSEC_CMD_BLE_SCAN_START: return "BLE_SCAN_START";
        case NETSEC_CMD_BLE_SCAN_STOP: return "BLE_SCAN_STOP";
        case NETSEC_CMD_BLE_SCAN_CANCEL: return "BLE_SCAN_CANCEL";
       default: return "UNKNOWN";
    }
}

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

void netsec_start_ble_scan(uint32_t duration_ms) {
    Serial.printf("[NETSEC] BLE scan requested for %lu ms\n", static_cast<unsigned long>(duration_ms));
    netsec_ble_start_scan(duration_ms);
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
// Commands are posted to `netsec_command_queue` as netsec_command_t structures.
extern QueueHandle_t netsec_command_queue;
void netsec_task(void* pvParameters) {
    (void)pvParameters;
    Serial.println("[NETSEC] Task started");
    netsec_command_t cmd;
    for (;;) {
        if (xQueueReceive(netsec_command_queue, &cmd, pdMS_TO_TICKS(1000)) == pdTRUE) {
            Serial.printf("[NETSEC] Command received: %s\n", netsec_cmd_str(cmd.type));
            switch (cmd.type) {
                case NETSEC_CMD_WIFI_SCAN_START:
                    netsec_start_wifi_scan();
                    break;
                case NETSEC_CMD_WIFI_SCAN_STOP:
                    netsec_stop_wifi_scan();
                    break;
                case NETSEC_CMD_BLE_SCAN_START:
                    if (netsec_ble_is_scanning()) {
                        Serial.println("[NETSEC] BLE scan already running, stopping before restart");
                        netsec_stop_ble_scan();
                    }
                    Serial.printf("[NETSEC] BLE scan duration=%lu ms\n",
                                  static_cast<unsigned long>(cmd.data.ble_scan_start.duration_ms));
                    netsec_start_ble_scan(cmd.data.ble_scan_start.duration_ms);
                    break;
                case NETSEC_CMD_BLE_SCAN_CANCEL:
                    if (netsec_ble_is_scanning()) {
                        netsec_stop_ble_scan();
                    } else {
                        Serial.println("[NETSEC] BLE scan stop requested but no scan active");
                    }
                    break;
                default:
                    Serial.printf("[NETSEC] Unknown command %u\n", cmd.type);
                    break;
            }
        }
        // Periodically could check scan results and post to local_result_queue
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
