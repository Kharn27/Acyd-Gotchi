// User_Setup for ESP32-CYD (ESP32-2432S028R)
// Centralize TFT_eSPI pin mapping to align with board_config.h
// and avoid macro conflicts on TOUCH_CS.

#pragma once

// Display driver configuration
#define USER_SETUP_ID 2432
#define ST7789_DRIVER
#define TFT_RGB_ORDER TFT_RGB
#define TFT_WIDTH 240
#define TFT_HEIGHT 320
#define CGRAM_OFFSET

// Backlight control (optional)
#define TFT_BL 27
#define TFT_BACKLIGHT_ON HIGH

// SPI pins
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1

// Touch controller (XPT2046) on shared SPI bus
#define TOUCH_CS 33
#define TOUCH_IRQ 36

// SPI frequencies
#define SPI_FREQUENCY 40000000
#define SPI_TOUCH_FREQUENCY 2500000
