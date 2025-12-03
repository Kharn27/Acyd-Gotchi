#include "netsec_ble.h"
#include "netsec_api.h"
#include <Arduino.h>
#include <string>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <BLEDevice.h>
#include <BLEScan.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_heap_caps.h>
#endif

static BLEScan* s_ble_scan = nullptr;
static bool s_ble_scan_running = false;
static bool s_ble_cancel_requested = false;
static TaskHandle_t s_ble_scan_task = nullptr;
static TimerHandle_t s_ble_scan_timer = nullptr;
static netsec_ble_device_t s_ble_devices[NETSEC_BLE_DEVICE_BUFFER_SIZE];
static size_t s_ble_device_write_idx = 0;
static uint32_t s_ble_scan_start_ms = 0;
static uint16_t s_ble_devices_reported = 0;
static bool s_bt_classic_mem_released = false;
static constexpr size_t NETSEC_BLE_MIN_HEAP_BYTES = 70 * 1024;

static bool netsec_ble_check_heap(void) {
#if defined(ARDUINO_ARCH_ESP32)
  const size_t free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  const size_t largest_8bit = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  Serial.printf("[NETSEC:BLE] Heap free=%u bytes, largest=%u bytes\n",
                static_cast<unsigned>(free_8bit),
                static_cast<unsigned>(largest_8bit));
  if (free_8bit < NETSEC_BLE_MIN_HEAP_BYTES || largest_8bit < (NETSEC_BLE_MIN_HEAP_BYTES / 2)) {
    Serial.println("[NETSEC:BLE] Not enough heap for BLE init, aborting scan");
    return false;
  }
#endif
  return true;
}

enum {
  NETSEC_BLE_NOTIFY_CANCEL = 1 << 0,
};

static const char* netsec_ble_result_type_str(netsec_result_type_t type) {
  switch (type) {
    case NETSEC_RES_BLE_SCAN_STARTED: return "BLE_SCAN_STARTED";
    case NETSEC_RES_BLE_DEVICE_FOUND: return "BLE_DEVICE_FOUND";
    case NETSEC_RES_BLE_SCAN_COMPLETED: return "BLE_SCAN_COMPLETED";
    case NETSEC_RES_BLE_SCAN_CANCELED: return "BLE_SCAN_CANCELED";
    case NETSEC_RES_WIFI_AP: return "WIFI_AP";
    case NETSEC_RES_WIFI_SCAN_DONE: return "WIFI_SCAN_DONE";
    default: return "UNKNOWN";
  }
}

static void netsec_ble_post_scan_event(netsec_result_type_t type, uint16_t device_count, uint32_t duration_ms) {
  extern QueueHandle_t netsec_result_queue;
  if (!netsec_result_queue) return;

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = type;
  res.data.scan_summary.item_count = device_count;
  res.data.scan_summary.duration_ms = duration_ms;
  res.data.scan_summary.timestamp_ms = millis();
  BaseType_t queued = xQueueSend(netsec_result_queue, &res, 0);
  Serial.printf("[NETSEC:BLE] Event queued: %s count=%u duration=%lu ms ts=%lu (%s)\n",
                netsec_ble_result_type_str(type),
                static_cast<unsigned>(device_count),
                static_cast<unsigned long>(duration_ms),
                static_cast<unsigned long>(res.data.scan_summary.timestamp_ms),
                queued == pdTRUE ? "ok" : "queue full");
}

static void netsec_ble_finalize_scan(bool canceled) {
#if defined(ARDUINO_ARCH_ESP32)
  if (s_ble_scan) {
    s_ble_scan->stop();
    s_ble_scan->clearResults();
  }
#endif

  uint32_t elapsed_ms = s_ble_scan_start_ms ? (millis() - s_ble_scan_start_ms) : 0;
  netsec_result_type_t evt_type = canceled ? NETSEC_RES_BLE_SCAN_CANCELED : NETSEC_RES_BLE_SCAN_COMPLETED;
  netsec_ble_post_scan_event(evt_type, s_ble_devices_reported, elapsed_ms);
  Serial.printf("[NETSEC:BLE] Scan %s: %u devices in %lu ms\n",
                canceled ? "canceled" : "completed",
                static_cast<unsigned>(s_ble_devices_reported),
                static_cast<unsigned long>(elapsed_ms));

  s_ble_scan_running = false;
  s_ble_cancel_requested = false;
  s_ble_scan_task = nullptr;
  s_ble_scan_start_ms = 0;
  s_ble_device_write_idx = 0;

  if (s_ble_scan_timer) {
    xTimerStop(s_ble_scan_timer, 0);
    xTimerDelete(s_ble_scan_timer, 0);
    s_ble_scan_timer = nullptr;
  }
}

#if defined(ARDUINO_ARCH_ESP32)
static void netsec_ble_timeout_cb(TimerHandle_t xTimer) {
  (void)xTimer;
  Serial.println("[NETSEC:BLE] Scan timeout reached, signaling cancel");
  if (s_ble_scan_task) {
    s_ble_scan_running = false;
    xTaskNotify(s_ble_scan_task, NETSEC_BLE_NOTIFY_CANCEL, eSetBits);
  }
}

static void netsec_ble_scan_task(void* pvParameters) {
  uint32_t duration_ms = reinterpret_cast<uint32_t>(pvParameters);
  const TickType_t stop_tick = xTaskGetTickCount() + pdMS_TO_TICKS(duration_ms);
  bool canceled = false;

  s_ble_scan_running = true;
  Serial.printf("[NETSEC:BLE] Starting BLE scan for %lu ms\n", static_cast<unsigned long>(duration_ms));

  while (s_ble_scan_running && xTaskGetTickCount() < stop_tick) {
    // Slice the scan to allow cancellation checks without long blocking calls.
    TickType_t remaining_ticks = stop_tick - xTaskGetTickCount();
    uint32_t slice_ms = pdTICKS_TO_MS(remaining_ticks);
    if (slice_ms > 500) {
      slice_ms = 500;
    }
    uint32_t slice_s = (slice_ms + 999) / 1000;
    if (slice_s == 0) {
      slice_s = 1;
    }

    BLEScanResults results = s_ble_scan->start(slice_s, false);
    const int found = results.getCount();
    for (int i = 0; i < found; ++i) {
      BLEAdvertisedDevice dev = results.getDevice(i);
      std::string name = dev.haveName() ? dev.getName() : std::string("");
      uint32_t flags = static_cast<uint32_t>(dev.getAddressType());
      netsec_ble_post_device(
          name.c_str(),
          dev.getRSSI(),
          reinterpret_cast<const uint8_t*>(dev.getAddress().getNative()),
          flags);
    }
    s_ble_scan->clearResults();
    Serial.printf("[NETSEC:BLE] Scan slice: %d advs, total reported=%u\n",
                  found,
                  static_cast<unsigned>(s_ble_devices_reported));

    uint32_t notify_value = 0;
    if (xTaskNotifyWait(0, NETSEC_BLE_NOTIFY_CANCEL, &notify_value, 0) == pdTRUE) {
      if (notify_value & NETSEC_BLE_NOTIFY_CANCEL) {
        Serial.println("[NETSEC:BLE] Cancel notification received");
        canceled = s_ble_cancel_requested;
        break;
      }
    }
  }

  if (!canceled && s_ble_cancel_requested) {
    canceled = true;
  }

  netsec_ble_finalize_scan(canceled);
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

  if (!netsec_ble_check_heap()) {
    return;
  }

  if (!s_ble_scan) {
    Serial.println("[NETSEC:BLE] Initializing BLE stack");
#if defined(ARDUINO_ARCH_ESP32)
    if (!s_bt_classic_mem_released) {
      esp_err_t rel = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
      Serial.printf("[NETSEC:BLE] Release classic BT mem: %s\n", rel == ESP_OK ? "ok" : "error");
      s_bt_classic_mem_released = true;
    }
#endif
    BLEDevice::init("");
    s_ble_scan = BLEDevice::getScan();
    if (!s_ble_scan) {
      Serial.println("[NETSEC:BLE] BLEDevice::getScan() returned null, aborting scan");
      return;
    }
    s_ble_scan->setActiveScan(true);
    s_ble_scan->setInterval(100);
    s_ble_scan->setWindow(99);
  }
  s_ble_device_write_idx = 0;
  s_ble_devices_reported = 0;
  s_ble_scan_start_ms = millis();
  s_ble_scan_running = true;
  s_ble_cancel_requested = false;
  netsec_ble_post_scan_event(NETSEC_RES_BLE_SCAN_STARTED, 0, duration_ms);

  if (s_ble_scan_timer) {
    xTimerStop(s_ble_scan_timer, 0);
    xTimerDelete(s_ble_scan_timer, 0);
  }
  s_ble_scan_timer = xTimerCreate("ble_scan_timeout",
                                  pdMS_TO_TICKS(duration_ms),
                                  pdFALSE,
                                  nullptr,
                                  netsec_ble_timeout_cb);
  if (!s_ble_scan_timer) {
    Serial.println("[NETSEC:BLE] Failed to create scan timeout timer");
  } else if (xTimerStart(s_ble_scan_timer, 0) != pdPASS) {
    Serial.println("[NETSEC:BLE] Failed to start scan timeout timer");
  } else {
    Serial.println("[NETSEC:BLE] Scan timeout timer armed");
  }

  BaseType_t task_status = xTaskCreatePinnedToCore(
      netsec_ble_scan_task,
      "ble_scan",
      4096,
      reinterpret_cast<void*>(duration_ms),
      1,
      &s_ble_scan_task,
      0);
  if (task_status != pdPASS || !s_ble_scan_task) {
    Serial.println("[NETSEC:BLE] Failed to create scan task");
    netsec_ble_finalize_scan(true);
  } else {
    Serial.println("[NETSEC:BLE] BLE scan task created");
  }
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
  if (s_ble_scan_timer) {
    xTimerStop(s_ble_scan_timer, 0);
  }
  s_ble_scan_running = false;
  s_ble_cancel_requested = true;
  if (s_ble_scan_task) {
    xTaskNotify(s_ble_scan_task, NETSEC_BLE_NOTIFY_CANCEL, eSetBits);
    Serial.println("[NETSEC:BLE] Cancel signal sent to scan task");
  }
  // Cleanup will be performed by the scan task to avoid race conditions.
#endif
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
    memcpy(device_slot->mac_bytes, addr, sizeof(device_slot->mac_bytes));
    snprintf(device_slot->mac_str, sizeof(device_slot->mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             device_slot->mac_bytes[0], device_slot->mac_bytes[1], device_slot->mac_bytes[2],
             device_slot->mac_bytes[3], device_slot->mac_bytes[4], device_slot->mac_bytes[5]);
  }
  device_slot->rssi = static_cast<int8_t>(rssi);
  device_slot->flags = flags;

  Serial.printf("[NETSEC:BLE] Device: %s | RSSI %d | name '%s'\n",
                strlen(device_slot->mac_str) ? device_slot->mac_str : "<unknown>",
                device_slot->rssi,
                strlen(device_slot->name) ? device_slot->name : "");

  netsec_result_t res;
  memset(&res, 0, sizeof(res));
  res.type = NETSEC_RES_BLE_DEVICE_FOUND;
  res.data.ble_device = *device_slot;

  if (s_ble_devices_reported < UINT16_MAX) {
    ++s_ble_devices_reported;
  }

  BaseType_t queued = xQueueSend(netsec_result_queue, &res, 0);
  if (queued != pdTRUE) {
    Serial.println("[NETSEC:BLE] Failed to queue device event (queue full?)");
  }
}
