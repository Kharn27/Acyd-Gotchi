#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

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
    char name[32];              // Device name (null-terminated)
    int8_t rssi;                // RSSI signal strength
    uint8_t addr[6];            // BLE MAC address
} netsec_ble_device_t;

/* NETSEC result types pushed to netsec_result_queue */
typedef enum {
    NETSEC_RES_NONE = 0,
    NETSEC_RES_WIFI_AP,        // WiFi AP found
    NETSEC_RES_WIFI_SCAN_DONE,  // WiFi scan complete
    NETSEC_RES_BLE_DEVICE,     // BLE device found
    NETSEC_RES_BLE_SCAN_DONE,   // BLE scan complete
} netsec_result_type_t;

/* NETSEC result structure sent via queue */
typedef struct {
    netsec_result_type_t type;
    union {
        netsec_wifi_ap_t wifi_ap;
        netsec_ble_device_t ble_device;
    } data;
} netsec_result_t;

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
void netsec_start_ble_scan(void);

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
#ifdef __cplusplus
extern "C" {
#endif

#include <freertos/queue.h>

// Global NETSEC command queue (created in system_init.cpp)
extern QueueHandle_t netsec_command_queue;

// NETSEC task entrypoint
void netsec_task(void* pvParameters);

#ifdef __cplusplus
}
#endif
