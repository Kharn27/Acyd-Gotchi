# PIXEL - UI Design & Implementation

## Overview

The Acyd-Gotchi UI is built with **LVGL 8** on the ESP32-CYD 240x320 color display.

### Design Philosophy
- **Tamagotchi-like**: Central pet display with minimal, clear interface
- **Non-blocking**: All UI operations fit in ~5ms LVGL tick
- **Memory-efficient**: Reusable styles, lazy screen creation
- **Themeable**: Centralized color palette and typography

---

## Architecture

### Theme System (`ui_theme.h` / `ui_theme.cpp`)
- Centralized color palette (dark theme, cyan primary, orange accent)
- Reusable LVGL `lv_style_t` objects for buttons, labels
- Typography: Small (12px), Normal (14px), Large (18px), Title (24px)
- Spacing constants: Tiny (2px), Small (4px), Normal (8px), Large (16px)

### Screen Management (`ui_screens.h`)
- Main screen: Pet display (120x120 px) + status bar + 2 button bands
- WiFi/BLE screens: List-based (placeholder, updated by NETSEC)
- Settings screen: Stub for future settings

### Implementation Modules
- **ui_main.cpp**: Module entry, screen creation on-demand
- **ui_main_screen.cpp**: Main screen layout with pet and buttons
- **ui_wifi_screen.cpp**: WiFi network list
- **ui_ble_screen.cpp**: BLE device list + settings

---

## Layout

### Main Screen (240x320)
```
┌─────────────────────────────┐
│  WiFi │ BLE │ ⚙ Settings    │ 40px top band
├─────────────────────────────┤
│                             │
│         [🐛 PET]            │ 120px pet display (centered)
│       120x120 px            │
│                             │
├─────────────────────────────┤
│  Acyd | Health: 100% | 😊   │ Status bar
├─────────────────────────────┤
│  OK       Back      Menu     │ 40px bottom band
└─────────────────────────────┘
```

### Button Bands
- **Top**: Narrow icons (40x30 px) for WiFi/BLE/Settings
- **Bottom**: Wide action buttons (80x32 px) for OK/Back/Menu
- All buttons use flexbox layout for responsive spacing

---

## Event Flow

1. User taps button on screen → LVGL callback triggered
2. Callback posts `UI_EVENT_*` to `ui_event_queue`
3. UI task (`ui_task.cpp`) receives event in main loop
4. Task calls `ui_show_wifi_screen()` or similar
5. Screen loads and displays

Example:
```
User taps "WiFi" button
  ↓
on_wifi_btn_click() callback
  ↓
Post UI_EVENT_BUTTON_WIFI to queue
  ↓
ui_task receives event
  ↓
ui_show_wifi_screen()
  ↓
WiFi list displayed
```

---

## Interaction with Other Modules

### ARCHI (Display/Touch)
- PIXEL relies on `lvgl_port.h` (LVGL initialization)
- Uses `display_driver.h` (flush callback) — automatic
- Uses `touch_driver.h` (XPT2046) via LVGL input device

### NETSEC
- PIXEL provides screens for WiFi/BLE results
- NETSEC updates `lv_list` with scan results (future)
- Pet mood/state updated based on NETSEC events

---

## Memory & Performance

### Buffer Usage
- LVGL draw buffer: ~19 KB (240 * 40 * 2 bytes, double-buffered)
- Screen objects: ~1-2 KB per screen (theme styles reused)
- Total: ~30 KB for 3 screens + theme

### CPU Load
- UI tick: 5 ms period (200 Hz)
- `lv_task_handler()` completes in <1 ms (mock/no-op display)
- Actual display flush (SPI) is async (handled by ARCHI)

### Future Optimizations
- Compress pet sprites (1-4 bit color)
- Use PSRAM for large image buffers
- Partial screen updates (only dirty regions)

---

## File Structure

```
firmware/
├── include/
│   ├── ui_theme.h          # Color, fonts, spacing defs
│   ├── ui_screens.h        # Screen creation API
│   └── ui_api.h            # (from earlier) UI module API
├── src/
│   ├── ui_main.cpp         # Module entry & screen management
│   └── ui/
│       ├── ui_theme.cpp    # Theme initialization
│       ├── ui_main_screen.cpp  # Main Tamagotchi screen
│       ├── ui_wifi_screen.cpp  # WiFi list
│       └── ui_ble_screen.cpp   # BLE list + settings
```

---

## Next Steps (PIXEL Future)

1. **Pet Sprites**: Add simple bitmap or generated graphics for pet
2. **Animations**: Smooth transitions between screens (fade, slide)
3. **Touch Feedback**: Visual feedback for button presses (invertion, color change)
4. **Dynamic Updates**: NETSEC populates WiFi/BLE lists in real-time
5. **Status Indicators**: Display WiFi/BLE scan status, pet health bars

