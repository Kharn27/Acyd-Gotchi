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

// Post BLE device found to the result queue
void netsec_ble_post_device(const char* name, int rssi, const uint8_t* addr, uint8_t addr_len);

#ifdef __cplusplus
}
#endif

#endif // NETSEC_BLE_H
