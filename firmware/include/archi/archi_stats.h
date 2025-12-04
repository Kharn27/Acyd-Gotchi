#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t heap_free_bytes;            // Current free 8-bit heap
    uint32_t heap_largest_free_block;    // Largest allocatable 8-bit block
    uint32_t heap_min_free_bytes;        // Low-water mark of 8-bit heap
    uint32_t flash_chip_size_bytes;      // Reported flash size
    bool     spiffs_mounted;             // SPIFFS mount status
    uint32_t spiffs_total_bytes;         // SPIFFS total size
    uint32_t spiffs_used_bytes;          // SPIFFS used size
    uint32_t psram_free_bytes;           // PSRAM free (0 if not available)
    uint32_t psram_total_bytes;          // PSRAM total (0 if not available)
} archi_sys_stats_t;

/**
 * Populate system stats (heap/flash/FS/PSRAM when available).
 * @param out_stats destination structure
 * @return true on success, false on invalid args or unsupported platform
 */
bool archi_get_sys_stats(archi_sys_stats_t* out_stats);

#ifdef __cplusplus
} // extern "C"
#endif

