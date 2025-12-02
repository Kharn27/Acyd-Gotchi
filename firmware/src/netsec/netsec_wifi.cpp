#include "netsec_wifi.h"
#include "netsec_api.h"
#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

// Forward declaration (ESP32 async scan callback)
#if defined(ARDUINO_ARCH_ESP32)
static void on_wifi_scan_done(WiFiEvent_t event, WiFiEventInfo_t info);

#endif

// Internal flag to avoid concurrent scans
static volatile bool wifi_scan_in_progress = false;
static uint32_t s_wifi_scan_start_ms = 0;
static uint16_t s_wifi_result_count = 0;

void netsec_wifi_start_scan(void)
{
  if (wifi_scan_in_progress) {
    Serial.println("[NETSEC:WIFI] Scan already in progress");
    return;
  }
  Serial.println("[NETSEC:WIFI] Starting WiFi scan (async)");
  wifi_scan_in_progress = true;
  s_wifi_result_count = 0;
  s_wifi_scan_start_ms = millis();
#if defined(ARDUINO_ARCH_ESP32)
  WiFi.onEvent(on_wifi_scan_done, ARDUINO_EVENT_WIFI_SCAN_DONE);
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

  if (s_wifi_result_count < UINT16_MAX) {
    ++s_wifi_result_count;
  }

  xQueueSend(netsec_result_queue, &res, 0);
}

// Callback: called when scan is done
#if defined(ARDUINO_ARCH_ESP32)
static void on_wifi_scan_done(WiFiEvent_t event, WiFiEventInfo_t info)
{
  uint32_t elapsed_ms = s_wifi_scan_start_ms ? (millis() - s_wifi_scan_start_ms) : 0;
  Serial.printf("[NETSEC:WIFI] Scan done in %lu ms, posting results\n", static_cast<unsigned long>(elapsed_ms));
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
    netsec_result_t done_evt = { .type = NETSEC_RES_WIFI_SCAN_DONE };
    done_evt.data.scan_summary.item_count = static_cast<uint16_t>(n);
    done_evt.data.scan_summary.duration_ms = elapsed_ms;
    done_evt.data.scan_summary.timestamp_ms = millis();
    xQueueSend(netsec_result_queue, &done_evt, 0);
  }
  WiFi.scanDelete();
  wifi_scan_in_progress = false;
}
#endif
