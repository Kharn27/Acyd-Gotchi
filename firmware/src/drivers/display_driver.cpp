#include "display_driver.h"
#include "board_config.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

void display_hw_init(void)
{
    Serial.println("ARCHI: Display init (TFT_eSPI)");
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    // On s'assure que le swap hardware est désactivé par défaut,
    // on le gérera manuellement lors du push
    tft.setSwapBytes(false);

#ifdef BACKLIGHT_PIN
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, HIGH);
#endif
}

void display_hw_deinit(void)
{
#ifdef BACKLIGHT_PIN
    digitalWrite(BACKLIGHT_PIN, LOW);
#endif
}

void display_hw_set_rotation(uint8_t rotation)
{
    tft.setRotation(rotation);
}

void display_hw_set_backlight(bool on)
{
#ifdef BACKLIGHT_PIN
    digitalWrite(BACKLIGHT_PIN, on ? HIGH : LOW);
#else
    (void)on;
#endif
}

void display_hw_push_pixels(int32_t x1, int32_t y1, uint32_t w, uint32_t h, const uint16_t* color_p)
{
    tft.startWrite();
    tft.setAddrWindow(x1, y1, w, h);
    // tft.pushPixels(const_cast<uint16_t*>(color_p), w * h);
    tft.pushColors(const_cast<uint16_t*>(color_p), w * h, true);
    tft.endWrite();
}
