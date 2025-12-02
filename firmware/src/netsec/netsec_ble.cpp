#include "netsec_ble.h"
#include "netsec_api.h"
#include <Arduino.h>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <BLEDevice.h>
#include <BLEScan.h>
#endif

static BLEScan* s_ble_scan = nullptr;
static bool s_ble_scan_running = false;
static TaskHandle_t s_ble_scan_task = nullptr;
static netsec_ble_device_t s_ble_devices[NETSEC_BLE_DEVICE_BUFFER_SIZE];
static size_t s_ble_device_write_idx = 0;
static uint32_t s_ble_scan_start_ms = 0;
static uint16_t s_ble_devices_reported = 0;

#if defined(ARDUINO_ARCH_ESP32)
class NetsecBLEAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice* device) override {
    std::string name = device->haveName() ? device->getName() : std::string("");
    netsec_ble_post_device(
        name.c_str(),
        device->getRSSI(),
        reinterpret_cast<const uint8_t*>(device->getAddress().getNative()),
        device->getAdvType());
  }
};
#endif

static void netsec_ble_post_scan_done(uint16_t device_count, uint32_t duration_ms) {
  extern QueueHandle_t netsec_result_queue;
  if (!netsec_result_queue) return;

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = NETSEC_RES_BLE_SCAN_DONE;
  res.data.scan_summary.item_count = device_count;
  res.data.scan_summary.duration_ms = duration_ms;
  res.data.scan_summary.timestamp_ms = millis();
  xQueueSend(netsec_result_queue, &res, 0);
}

#if defined(ARDUINO_ARCH_ESP32)
static void netsec_ble_scan_task(void* pvParameters) {
  uint32_t duration_ms = reinterpret_cast<uint32_t>(pvParameters);
  uint32_t duration_s = (duration_ms + 999) / 1000;
  if (duration_s == 0) {
    duration_s = 1;
  }

  s_ble_scan_running = true;
  Serial.printf("[NETSEC:BLE] Starting BLE scan for %lu ms\n", static_cast<unsigned long>(duration_ms));

  BLEScanResults results = s_ble_scan->start(duration_s, false);
  (void)results;

  uint32_t elapsed_ms = s_ble_scan_start_ms ? (millis() - s_ble_scan_start_ms) : 0;
  netsec_ble_post_scan_done(s_ble_devices_reported, elapsed_ms);

  s_ble_scan_running = false;
  s_ble_scan_task = nullptr;
  vTaskDelete(NULL);
}
#endif

bool netsec_ble_is_scanning(void) {
#if defined(ARDUINO_ARCH_ESP32)
  return s_ble_scan_running || (s_ble_scan_task != nullptr);
#else
  return false;
#endif
}

void netsec_ble_start_scan(uint32_t duration_ms)
{
#if defined(ARDUINO_ARCH_ESP32)
  if (s_ble_scan_running) {
    Serial.println("[NETSEC:BLE] Scan already running, restarting");
    netsec_ble_stop_scan();
  }

  if (!s_ble_scan) {
    BLEDevice::init("");
    s_ble_scan = BLEDevice::getScan();
    s_ble_scan->setActiveScan(true);
    s_ble_scan->setInterval(100);
    s_ble_scan->setWindow(99);
    s_ble_scan->setAdvertisedDeviceCallbacks(new NetsecBLEAdvertisedDeviceCallbacks(), true);
  }
  if (s_ble_scan_task) {
    // Wait for previous task slot to be cleaned up
    while (s_ble_scan_task) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
  s_ble_devices_reported = 0;
  s_ble_scan_start_ms = millis();
  s_ble_scan_running = true;
  xTaskCreatePinnedToCore(
      netsec_ble_scan_task,
      "ble_scan",
      4096,
      reinterpret_cast<void*>(duration_ms),
      1,
      &s_ble_scan_task,
      0);
#else
  Serial.println("[NETSEC:BLE] BLE not supported on this platform (mock)");
#endif
}

void netsec_ble_stop_scan(void)
{
#if defined(ARDUINO_ARCH_ESP32)
  if (!s_ble_scan_running && !s_ble_scan_task) {
    Serial.println("[NETSEC:BLE] Stop requested but no scan running");
    return;
  }
  Serial.println("[NETSEC:BLE] Stop BLE scan");
  if (s_ble_scan) s_ble_scan->stop();
  if (s_ble_scan_task) {
    // Give some time for the scan task to exit cleanly
    while (s_ble_scan_task) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
#endif
  s_ble_scan_running = false;
}

void netsec_ble_post_device(const char* name, int rssi, const uint8_t* addr, uint32_t flags)
{
  extern QueueHandle_t netsec_result_queue;
  if (!netsec_result_queue) return;

  netsec_ble_device_t* device_slot = &s_ble_devices[s_ble_device_write_idx];
  s_ble_device_write_idx = (s_ble_device_write_idx + 1) % NETSEC_BLE_DEVICE_BUFFER_SIZE;

  memset(device_slot, 0, sizeof(*device_slot));
  strncpy(device_slot->name, name ? name : "", sizeof(device_slot->name) - 1);
  if (addr) {
    memcpy(device_slot->addr, addr, sizeof(device_slot->addr));
  }
  device_slot->rssi = static_cast<int8_t>(rssi);
  device_slot->flags = flags;

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = NETSEC_RES_BLE_DEVICE_FOUND;
  res.data.ble_device = *device_slot;

  if (s_ble_devices_reported < UINT16_MAX) {
    ++s_ble_devices_reported;
  }

  xQueueSend(netsec_result_queue, &res, 0);
}

