#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#ifdef __cplusplus
extern "C" {
#endif

// NETSEC Module API
// NETSEC is responsible for implementing these functions.

/* WiFi AP result structure */
typedef struct {
    char ssid[33];              // SSID (null-terminated)
    int8_t rssi;                // RSSI signal strength
    uint8_t bssid[6];           // MAC address
    uint8_t channel;            // WiFi channel
} netsec_wifi_ap_t;

/* BLE device result structure */
typedef struct {
    char name[32];              // Device name (UTF-8, max 31 chars + NUL)
    char mac_str[18];           // Colon-separated MAC string ("AA:BB:CC:DD:EE:FF")
    uint8_t mac_bytes[6];       // Raw 6-byte MAC address
    int8_t rssi;                // RSSI signal strength
    uint32_t flags;             // Bitmask describing advertisement/properties
} netsec_ble_device_t;

/* Scan completion metadata shared across WiFi/BLE */
typedef struct {
    uint16_t item_count;        // Number of APs/devices reported during the scan
    uint32_t duration_ms;       // Wall-clock duration of the scan
    uint32_t timestamp_ms;      // Time when the scan finished (millis())
} netsec_scan_summary_t;

/* NETSEC result types pushed to netsec_result_queue */
typedef enum {
    NETSEC_RES_NONE = 0,
    NETSEC_RES_WIFI_AP,          // WiFi AP found
    NETSEC_RES_WIFI_SCAN_DONE,   // WiFi scan complete
    NETSEC_RES_BLE_SCAN_STARTED, // BLE scan started
    NETSEC_RES_BLE_DEVICE_FOUND, // BLE device found
    NETSEC_RES_BLE_SCAN_COMPLETED, // BLE scan complete (duration reached)
    NETSEC_RES_BLE_SCAN_CANCELED,  // BLE scan canceled by user
} netsec_result_type_t;

/* NETSEC result structure sent via queue */
typedef struct {
    netsec_result_type_t type;
    union {
        netsec_wifi_ap_t wifi_ap;
        netsec_ble_device_t ble_device;
        netsec_scan_summary_t scan_summary; // Populated for BLE/WiFi scan lifecycle events
    } data;
} netsec_result_t;

typedef enum {
    NETSEC_CMD_NONE = 0,
    NETSEC_CMD_WIFI_SCAN_START,
    NETSEC_CMD_WIFI_SCAN_STOP,
    NETSEC_CMD_BLE_SCAN_START,
    NETSEC_CMD_BLE_SCAN_STOP,
    NETSEC_CMD_BLE_SCAN_CANCEL = NETSEC_CMD_BLE_SCAN_STOP, // Alias for compatibility
} netsec_command_type_t;

typedef struct {
    netsec_command_type_t type;
    union {
        struct {
            uint32_t duration_ms;
        } ble_scan_start; // Duration for NETSEC_CMD_BLE_SCAN_START
    } data;
} netsec_command_t;

/**
 * Initialize NETSEC module.
 * @param result_queue: queue to push scan results to
 */
void netsec_init(QueueHandle_t result_queue);

/**
 * Start a WiFi scan (non-blocking).
 * Results are sent to netsec_result_queue.
 */
void netsec_start_wifi_scan(void);

/**
 * Stop an ongoing WiFi scan.
 */
void netsec_stop_wifi_scan(void);

/**
 * Start a BLE scan (non-blocking).
 * Results are sent to netsec_result_queue.
 */
void netsec_start_ble_scan(uint32_t duration_ms);

/**
 * Stop an ongoing BLE scan.
 */
void netsec_stop_ble_scan(void);

/**
 * Request handshake capture on a target AP.
 * Must check LAB_MODE_ENABLED before proceeding.
 * @param target: WiFi AP to target
 * @return: true if capture started, false if lab mode disabled or error
 */
bool netsec_request_handshake_capture(const netsec_wifi_ap_t* target);

// Global NETSEC command queue (created in system_init.cpp)

// NETSEC task entrypoint

#ifdef __cplusplus
}
#endif
