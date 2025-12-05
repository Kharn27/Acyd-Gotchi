/*
 * PIXEL - Monitor Screen
 *
 * Displays basic system metrics (heap, flash, SPIFFS, PSRAM) with a lightweight
 * layout and periodic refresh.
 */

#include "ui_screens.h"
#include "ui_theme.h"
#include "archi/archi_stats.h"
#include "lvgl.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>
static lv_obj_t* g_label_heap = NULL;
static lv_obj_t* g_label_heap_min = NULL;
static lv_obj_t* g_label_internal_heap = NULL;
static lv_obj_t* g_label_flash = NULL;
static lv_obj_t* g_label_spiffs = NULL;
static lv_obj_t* g_label_psram = NULL;
static lv_timer_t* g_refresh_timer = NULL;

static void monitor_refresh_cb(lv_timer_t* timer);
static void on_monitor_screen_delete(lv_event_t* e);

lv_obj_t* ui_create_monitor_screen(void)
{
  if (g_refresh_timer) {
    lv_timer_del(g_refresh_timer);
    g_refresh_timer = NULL;
  }

  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_size(scr, LV_HOR_RES, LV_VER_RES);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

  // Top band
  lv_obj_t* band_top = lv_obj_create(scr);
  lv_obj_set_size(band_top, LV_HOR_RES, BAND_HEIGHT);
  lv_obj_set_pos(band_top, 0, 0);
  lv_obj_set_style_bg_color(band_top, lv_color_hex(COLOR_SURFACE), 0);
  lv_obj_set_style_bg_opa(band_top, LV_OPA_30, 0);
  lv_obj_set_style_border_width(band_top, 0, 0);
  lv_obj_set_style_pad_all(band_top, PAD_SMALL, 0);
  lv_obj_set_flex_flow(band_top, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(band_top, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t* title = lv_label_create(band_top);
  lv_label_set_text(title, "Monitor");
  lv_obj_add_style(title, ui_get_style_label_title(), 0);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_TEXT), 0);

  // Content
  lv_obj_t* content = lv_obj_create(scr);
  lv_obj_set_size(content, LV_HOR_RES, LV_VER_RES - BAND_HEIGHT);
  lv_obj_set_pos(content, 0, BAND_HEIGHT);
  lv_obj_set_style_bg_color(content, lv_color_hex(COLOR_BACKGROUND), 0);
  lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(content, 0, 0);
  lv_obj_set_style_pad_all(content, PAD_NORMAL, 0);
  lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

  g_label_heap = lv_label_create(content);
  lv_obj_add_style(g_label_heap, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_heap, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_heap, "Heap: --");

  g_label_heap_min = lv_label_create(content);
  lv_obj_add_style(g_label_heap_min, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_heap_min, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_heap_min, "Heap min: --");

  g_label_internal_heap = lv_label_create(content);
  lv_obj_add_style(g_label_internal_heap, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_internal_heap, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_internal_heap, "Internal heap: --");

  g_label_flash = lv_label_create(content);
  lv_obj_add_style(g_label_flash, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_flash, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_flash, "Flash: --");

  g_label_spiffs = lv_label_create(content);
  lv_obj_add_style(g_label_spiffs, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_spiffs, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_spiffs, "SPIFFS: --");

  g_label_psram = lv_label_create(content);
  lv_obj_add_style(g_label_psram, ui_get_style_label_normal(), 0);
  lv_obj_set_style_text_color(g_label_psram, lv_color_hex(COLOR_TEXT), 0);
  lv_label_set_text(g_label_psram, "PSRAM: --");

  // Refresh timer (1s)
  g_refresh_timer = lv_timer_create(monitor_refresh_cb, 1000, NULL);
  monitor_refresh_cb(NULL);

  lv_obj_add_event_cb(scr, on_monitor_screen_delete, LV_EVENT_DELETE, NULL);
  return scr;
}

static void monitor_refresh_cb(lv_timer_t* timer)
{
  (void)timer;

  if (!g_label_heap) return;

  archi_sys_stats_t stats;
  archi_heap_snapshot_t heap_snap;
  bool has_sys = archi_get_sys_stats(&stats);
  bool has_heap = archi_get_heap_snapshot(&heap_snap);

  if (has_heap) {
    lv_label_set_text_fmt(
        g_label_heap,
        "8-bit heap free: %u B (largest %u B)",
        static_cast<unsigned>(heap_snap.free_8bit),
        static_cast<unsigned>(heap_snap.largest_8bit));

    lv_label_set_text_fmt(
        g_label_heap_min,
        "8-bit heap min free: %u B",
        static_cast<unsigned>(heap_snap.min_free_8bit));

    lv_label_set_text_fmt(
        g_label_internal_heap,
        "Internal heap free: %u B (min %u B)",
        static_cast<unsigned>(heap_snap.free_internal),
        static_cast<unsigned>(heap_snap.min_internal));
  } else {
    lv_label_set_text(g_label_heap, "8-bit heap: N/A");
    lv_label_set_text(g_label_heap_min, "8-bit heap min: N/A");
    lv_label_set_text(g_label_internal_heap, "Internal heap: N/A");
  }

  if (has_sys) {
    lv_label_set_text_fmt(
        g_label_flash,
        "Flash size: %u KB",
        static_cast<unsigned>(stats.flash_chip_size_bytes / 1024));
  } else {
    lv_label_set_text(g_label_flash, "Flash: N/A");
  }

  if (has_sys && stats.spiffs_mounted && stats.spiffs_total_bytes > 0) {
    uint32_t free_bytes = stats.spiffs_total_bytes - stats.spiffs_used_bytes;
    lv_label_set_text_fmt(
        g_label_spiffs,
        "SPIFFS: %u/%u KB used (free %u KB)",
        static_cast<unsigned>(stats.spiffs_used_bytes / 1024),
        static_cast<unsigned>(stats.spiffs_total_bytes / 1024),
        static_cast<unsigned>(free_bytes / 1024));
  } else {
    lv_label_set_text(g_label_spiffs, "SPIFFS: not mounted");
  }

  if (has_sys && stats.psram_total_bytes > 0) {
    if (has_heap && (heap_snap.free_psram > 0 || heap_snap.min_psram > 0)) {
      lv_label_set_text_fmt(
          g_label_psram,
          "PSRAM: %u/%u KB free (min %u KB, largest %u B)",
          static_cast<unsigned>(heap_snap.free_psram / 1024),
          static_cast<unsigned>(stats.psram_total_bytes / 1024),
          static_cast<unsigned>(heap_snap.min_psram / 1024),
          static_cast<unsigned>(heap_snap.largest_psram));
    } else {
      lv_label_set_text_fmt(
          g_label_psram,
          "PSRAM: %u/%u KB free",
          static_cast<unsigned>(stats.psram_free_bytes / 1024),
          static_cast<unsigned>(stats.psram_total_bytes / 1024));
    }
  } else {
    lv_label_set_text(g_label_psram, "PSRAM: not available");
  }
}

static void on_monitor_screen_delete(lv_event_t* e)
{
  (void)e;

  g_label_heap = NULL;
  g_label_heap_min = NULL;
  g_label_internal_heap = NULL;
  g_label_flash = NULL;
  g_label_spiffs = NULL;
  g_label_psram = NULL;

  if (g_refresh_timer) {
    lv_timer_del(g_refresh_timer);
    g_refresh_timer = NULL;
  }
}

