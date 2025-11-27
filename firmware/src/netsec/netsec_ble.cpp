#include "netsec_ble.h"
#include "netsec_api.h"
#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <BLEDevice.h>
#include <BLEScan.h>
#endif

static BLEScan* s_ble_scan = nullptr;

void netsec_ble_start_scan(void)
{
  Serial.println("[NETSEC:BLE] Starting BLE scan");
#if defined(ARDUINO_ARCH_ESP32)
  if (!s_ble_scan) {
    BLEDevice::init("");
    s_ble_scan = BLEDevice::getScan();
    s_ble_scan->setActiveScan(true);
    s_ble_scan->setInterval(100);
    s_ble_scan->setWindow(99);
  }
  // Start scan asynchronously for 5 seconds and register callback via results (blocking call returns results)
  // We use start(5, false) to run non-blocking on ESP32 BLE stack
  s_ble_scan->start(5, false);
#else
  Serial.println("[NETSEC:BLE] BLE not supported on this platform (mock)");
#endif
}

void netsec_ble_stop_scan(void)
{
  Serial.println("[NETSEC:BLE] Stop BLE scan (noop)");
#if defined(ARDUINO_ARCH_ESP32)
  if (s_ble_scan) s_ble_scan->stop();
#endif
}

void netsec_ble_post_device(const char* name, int rssi, const uint8_t* addr, uint8_t addr_len)
{
  extern QueueHandle_t netsec_result_queue;
  if (!netsec_result_queue) return;

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = NETSEC_RES_BLE_DEVICE;
  strncpy(res.data.ble_device.name, name ? name : "", sizeof(res.data.ble_device.name) - 1);
  res.data.ble_device.rssi = rssi;
  res.data.ble_device.addr_len = addr_len > sizeof(res.data.ble_device.addr) ? sizeof(res.data.ble_device.addr) : addr_len;
  if (addr && res.data.ble_device.addr_len) {
    memcpy(res.data.ble_device.addr, addr, res.data.ble_device.addr_len);
  }

  xQueueSend(netsec_result_queue, &res, 0);
}

