#include "display_driver.h"
#include "board_config.h" // Assure-toi que BACKLIGHT_PIN y est défini (21)

#include <Arduino.h>
#include <TFT_eSPI.h>

static TFT_eSPI tft = TFT_eSPI();

// --- CONFIGURATION PWM ---
// Si BACKLIGHT_PIN n'est pas défini, on force la 21
#ifndef BACKLIGHT_PIN
  #define BACKLIGHT_PIN 21
#endif

#define BL_CHANNEL 0
#define BL_FREQ    5000
#define BL_RES     8 
// VALEUR CRITIQUE : 40 (environ 15% de puissance)
// Si avec ça c'est toujours "éblouissant", c'est qu'il y a un souci hard.
#define BL_VAL     180 

void display_hw_init(void)
{
    Serial.println("ARCHI: Display init (TFT_eSPI + LEDC Low)");
    tft.init();
    tft.setRotation(1);
    tft.setSwapBytes(false); // On laisse pushColors gérer
    tft.fillScreen(TFT_BLACK);

    // --- CONFIG PWM ---
    // 1. Setup du canal
    ledcSetup(BL_CHANNEL, BL_FREQ, BL_RES);
    
    // 2. Attachement à la PIN 21
    ledcAttachPin(BACKLIGHT_PIN, BL_CHANNEL);
    
    // 3. Écriture de la valeur (40/255)
    ledcWrite(BL_CHANNEL, BL_VAL); 
}

void display_hw_deinit(void)
{
    ledcWrite(BL_CHANNEL, 0);
}

void display_hw_set_rotation(uint8_t rotation)
{
    tft.setRotation(rotation);
}

void display_hw_set_backlight(bool on)
{
    // On/Off via PWM
    ledcWrite(BL_CHANNEL, on ? BL_VAL : 0);
}

void display_hw_push_pixels(int32_t x1, int32_t y1, uint32_t w, uint32_t h, const uint16_t* color_p)
{
    tft.startWrite();
    tft.setAddrWindow(x1, y1, w, h);
    // Swap activé pour corriger les couleurs
    tft.pushColors(const_cast<uint16_t*>(color_p), w * h, true);
    tft.endWrite();
}