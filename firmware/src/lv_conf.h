#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* ==========================================
   ACTIVER LA CONFIGURATION
   ========================================== */
#define LV_CONF_SKIP 0  /* Doit être à 0 pour que ce fichier soit pris en compte */

/* ==========================================
   AFFICHAGE ET COULEURS
   ========================================== */
/* Pour ESP32 CYD, on est en 16 bits (RGB565) */
#define LV_COLOR_DEPTH 16

/* SWAP DES BYTES : CRUCIAL POUR TFT_eSPI
 * Si vos couleurs sont bizarres (glitchy), changez ceci de 1 à 0 ou inversement.
 * Généralement 0 pour TFT_eSPI si on utilise tft.pushColors() correctement,
 * mais 1 permet souvent d'optimiser le transfert SPI. */
#define LV_COLOR_16_SWAP 0

/* ==========================================
   MÉMOIRE
   ========================================== */
/* 0: Utiliser l'allocateur interne de LVGL (plus simple pour débuter)
 * 1: Utiliser malloc/free de l'ESP32 */
#define LV_MEM_CUSTOM 0

/* Taille du tas (heap) interne à LVGL en Kilooctets.
 * 48Ko est confortable pour un ESP32 sans PSRAM. */
#if LV_MEM_CUSTOM == 0
    #define LV_MEM_SIZE (48U * 1024U)
#endif

/* ==========================================
   TICK TIMER (IMPORTANT POUR ARDUINO)
   ========================================== */
/* Utilise la fonction millis() d'Arduino automatiquement pour gérer le temps.
 * Plus besoin d'appeler lv_tick_inc() dans loop() */
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/* ==========================================
   WIDGETS ET THEMES
   ========================================== */
#define LV_USE_LABEL 1
#define LV_USE_BTN 1
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_SLIDER 1
#define LV_USE_CHECKBOX 1
#define LV_USE_SWITCH 1

/* Thème par défaut */
#define LV_THEME_DEFAULT_DARK 1
#define LV_THEME_DEFAULT_GROW 1
#define LV_THEME_DEFAULT_TRANSITION_TIME 80

/* ==========================================
   POLICES (FONTS)
   ========================================== */
/* Activez uniquement celles dont vous avez besoin pour économiser la mémoire Flash */

/* --- POLICES MODERNES (Désactivées pour look Hacker/Terminal) --- */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_16 0

/* --- POLICES BITMAP / TERMINAL (Activées) --- */
/* UNSCII 8px pour les logs/debug (très petit) */
#define LV_FONT_UNSCII_8  1
/* UNSCII 16px pour le texte principal (Pet, Menus) - Look Retro */
#define LV_FONT_UNSCII_16 1

/* Police par défaut : UNSCII 16px (Style Terminal) 
 * Note : Définir ici évite les erreurs de syntaxe '&' dans platformio.ini sur Linux/Mac */
#define LV_FONT_DEFAULT &lv_font_unscii_16

/* ==========================================
   LOGGING (DEBUG)
   ========================================== */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN /* Affiche seulement les erreurs/warnings */

/* ==========================================
   AUTRES
   ========================================== */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0

#endif /* LV_CONF_H */