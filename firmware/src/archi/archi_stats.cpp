#include "archi/archi_stats.h"

#include <Arduino.h>
#include <string.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_heap_caps.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <SPIFFS.h>
#endif

static void archi_zero_heap_snapshot(archi_heap_snapshot_t* snapshot)
{
  if (snapshot) {
    memset(snapshot, 0, sizeof(*snapshot));
  }
}

bool archi_get_sys_stats(archi_sys_stats_t* out_stats)
{
  if (!out_stats) {
    return false;
  }

  memset(out_stats, 0, sizeof(*out_stats));

#if defined(ARDUINO_ARCH_ESP32)
  // Heap (8-bit capable) metrics
  out_stats->heap_free_bytes = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  out_stats->heap_largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  out_stats->heap_min_free_bytes = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);

  // Flash chip size (as reported by Arduino API)
  out_stats->flash_chip_size_bytes = ESP.getFlashChipSize();

  // PSRAM metrics (only if present)
  #if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT) || defined(CONFIG_SPIRAM)
    out_stats->psram_free_bytes = ESP.getFreePsram();
    out_stats->psram_total_bytes = ESP.getPsramSize();
  #endif

  // SPIFFS stats (attempt mount if not already mounted)
  if (SPIFFS.begin(false)) {
    out_stats->spiffs_mounted = true;
    out_stats->spiffs_total_bytes = SPIFFS.totalBytes();
    out_stats->spiffs_used_bytes = SPIFFS.usedBytes();
  }

  return true;
#else
  return false;
#endif
}

bool archi_get_heap_snapshot(archi_heap_snapshot_t* out_snapshot)
{
  if (!out_snapshot) {
    return false;
  }

  archi_zero_heap_snapshot(out_snapshot);

#if defined(ARDUINO_ARCH_ESP32)
  out_snapshot->free_8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  out_snapshot->largest_8bit = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
  out_snapshot->min_free_8bit = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
  out_snapshot->free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  out_snapshot->min_internal = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#if defined(MALLOC_CAP_SPIRAM)
  out_snapshot->free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  out_snapshot->largest_psram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
  out_snapshot->min_psram = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
#endif
  return true;
#else
  return false;
#endif
}

void archi_log_heap(const char* tag)
{
#if defined(ARDUINO_ARCH_ESP32)
  archi_heap_snapshot_t snap;
  if (!archi_get_heap_snapshot(&snap)) {
    return;
  }

  Serial.printf("[HEAP]%s free8=%luB largest8=%luB min8=%luB internal=%luB min_internal=%luB psram=%luB min_psram=%luB largest_psram=%luB\n",
                tag ? tag : "",
                static_cast<unsigned long>(snap.free_8bit),
                static_cast<unsigned long>(snap.largest_8bit),
                static_cast<unsigned long>(snap.min_free_8bit),
                static_cast<unsigned long>(snap.free_internal),
                static_cast<unsigned long>(snap.min_internal),
                static_cast<unsigned long>(snap.free_psram),
                static_cast<unsigned long>(snap.min_psram),
                static_cast<unsigned long>(snap.largest_psram));
#else
  (void)tag;
#endif
}
