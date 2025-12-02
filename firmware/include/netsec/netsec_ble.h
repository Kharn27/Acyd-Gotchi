#ifndef NETSEC_BLE_H
#define NETSEC_BLE_H

#include <stdint.h>
#include "netsec_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start asynchronous BLE scan
void netsec_ble_start_scan(uint32_t duration_ms);

// Stop BLE scan
void netsec_ble_stop_scan(void);

// Maximum number of cached BLE devices per scan cycle
#define NETSEC_BLE_DEVICE_BUFFER_SIZE 16

// Post BLE device found to the result queue (reuses an internal circular buffer)
void netsec_ble_post_device(const char* name, int rssi, const uint8_t* addr, uint32_t flags);

// Check whether a BLE scan task is currently running
bool netsec_ble_is_scanning(void);

#ifdef __cplusplus
}
#endif

#endif // NETSEC_BLE_H
