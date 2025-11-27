#ifndef NETSEC_WIFI_H
#define NETSEC_WIFI_H

#include <stdint.h>
#include "netsec_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Start an asynchronous WiFi scan. Results are posted to the provided queue via netsec_result_t entries.
void netsec_wifi_start_scan(void);

// Stop an ongoing WiFi scan (if supported)
void netsec_wifi_stop_scan(void);

// Convert low-level scan result into netsec_result_t and post to queue
void netsec_wifi_post_ap(const char* ssid, int32_t rssi, uint8_t channel, const uint8_t* bssid);

#ifdef __cplusplus
}
#endif

#endif // NETSEC_WIFI_H
