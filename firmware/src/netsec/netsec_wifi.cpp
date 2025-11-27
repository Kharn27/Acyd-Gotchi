#include "netsec_wifi.h"
#include "netsec_api.h"
#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// Forward declaration
static void on_wifi_scan_done(system_event_id_t event, system_event_info_t info);

// Internal flag to avoid concurrent scans
static volatile bool wifi_scan_in_progress = false;

// ESP32: WiFi event handler for scan done
#if defined(ARDUINO_ARCH_ESP32)
static void WiFiEvent(WiFiEvent_t event, system_event_info_t info) {
  if (event == SYSTEM_EVENT_SCAN_DONE) {
    on_wifi_scan_done(event, info);
  }
}
#endif

void netsec_wifi_start_scan(void)
{
  if (wifi_scan_in_progress) {
    Serial.println("[NETSEC:WIFI] Scan already in progress");
    return;
  }
  Serial.println("[NETSEC:WIFI] Starting WiFi scan (async)");
  wifi_scan_in_progress = true;
#if defined(ARDUINO_ARCH_ESP32)
  WiFi.onEvent(WiFiEvent);
  WiFi.scanNetworks(true, true);
#else
  WiFi.scanNetworks();
  // TODO: handle scan done for ESP8266
#endif
}

void netsec_wifi_stop_scan(void)
{
  Serial.println("[NETSEC:WIFI] Stop WiFi scan (not implemented)");
  // ESP32 does not provide a direct stop for async scan; rely on scan completion
}

void netsec_wifi_post_ap(const char* ssid, int32_t rssi, uint8_t channel, const uint8_t* bssid)
{
  extern QueueHandle_t netsec_result_queue;
  if (!netsec_result_queue) return;

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = NETSEC_RES_WIFI_AP;
  strncpy(res.data.wifi_ap.ssid, ssid, sizeof(res.data.wifi_ap.ssid)-1);
  res.data.wifi_ap.rssi = rssi;
  res.data.wifi_ap.channel = channel;
  if (bssid) memcpy(res.data.wifi_ap.bssid, bssid, 6);

  xQueueSend(netsec_result_queue, &res, 0);
}

// Callback: called when scan is done
static void on_wifi_scan_done(system_event_id_t event, system_event_info_t info)
{
  Serial.println("[NETSEC:WIFI] Scan done, posting results");
  int n = WiFi.scanComplete();
  for (int i = 0; i < n; ++i) {
    netsec_wifi_post_ap(
      WiFi.SSID(i).c_str(),
      WiFi.RSSI(i),
      WiFi.channel(i),
      WiFi.BSSID(i)
    );
  }
  // Post scan done event
  extern QueueHandle_t netsec_result_queue;
  if (netsec_result_queue) {
    netsec_result_t done_evt = {0};
    done_evt.type = NETSEC_RES_WIFI_SCAN_DONE;
    xQueueSend(netsec_result_queue, &done_evt, 0);
  }
  WiFi.scanDelete();
  wifi_scan_in_progress = false;
}
