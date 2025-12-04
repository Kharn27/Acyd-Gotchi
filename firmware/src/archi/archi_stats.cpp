#include "archi/archi_stats.h"

#include <Arduino.h>
#include <string.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_heap_caps.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <SPIFFS.h>
#endif

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

