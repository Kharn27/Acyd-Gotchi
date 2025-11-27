# ARCHI - UI Smoke Test Plan

## Objectif
Valider que le système ARCHI (LVGL + drivers hardware) initialise correctement et que les tâches FreeRTOS s'exécutent sans bloquer.

## Prérequis
- ESP32-CYD assemblé et connecté (USB pour serial monitor)
- PlatformIO installé et configuré
- Brochage TFT_eSPI et XPT2046 correspont à `User_Setup.h`

## Build & Flash

### Avec mock drivers (toujours compile)
```bash
cd firmware
# Activer le mode mock (pas de TFT_eSPI/XPT2046 en compilant)
# Ajouter `-DDISPLAY_MOCK=1` à platformio.ini build_flags si TFT_eSPI échoue
pio run -e esp32-cyd
pio run -e esp32-cyd --target upload
```

### Avec TFT_eSPI réel (si compilation passe)
```bash
pio run -e esp32-cyd
pio run -e esp32-cyd --target upload
```

## Serial Output Expected

### Boot (devrait voir cet ordre)
```
[SYSTEM] Initializing system...
[ARCHI] Initializing board hardware...
[ARCHI] Board hardware OK
[SYSTEM] Creating queues...
[SYSTEM] Queues created
[UI] Init stub (PIXEL will implement)
[NETSEC] Init stub (NETSEC will implement)
[SYSTEM] Creating UI task...
[SYSTEM] Creating NETSEC task...
[SYSTEM] System initialized successfully
UI Task started on core 1, priority X
ARCHI: Initializing display driver...
ARCHI: Display init (MOCK=0 or 1)
ARCHI: Display buffers allocated (xxxx bytes each)
ARCHI: LVGL display driver registered
ARCHI: LVGL input device (touch) registered
ARCHI: Setting up LVGL tick timer...
ARCHI: LVGL port initialized
[UI] Show main screen stub
```

## Tests Manuels

### 1. Display Initialization
- [ ] Boot sequence complète sans crash (Guru Meditation)
- [ ] "Display buffers allocated" message visible
- [ ] "LVGL display driver registered" présent
- [ ] No error messages about TFT_eSPI or buffer allocation

### 2. Task Scheduling
- [ ] "UI Task started on core 1" appears
- [ ] Task priorities printed correctly
- [ ] No watchdog/WDT reset during boot

### 3. Touch Controller (si matériel réel)
- [ ] "LVGL input device (touch) registered" visible
- [ ] Si XPT2046 présent: pas d'erreur "XPT2046 touchscreen begin failed"
- [ ] En mode MOCK: "Running in MOCK touch mode" affiché

### 4. LVGL Tick Timer
- [ ] "LVGL tick timer..." messages present
- [ ] Pas d'erreur "Failed to create LVGL tick timer"

### 5. No Blocking
- [ ] UI task tourne sans arrêt (loop toutes les ~5 ms)
- [ ] Serial output steady, pas de stalls
- [ ] NETSEC task runs without blocking UI

## Fallback : Mode MOCK

Si `TFT_eSPI` ou `XPT2046` ne compilent pas :

1. Ajouter à `platformio.ini`:
```ini
build_flags =
  -DDISPLAY_MOCK=1
  -DTOUCH_MOCK=1
  -I${PROJECT_DIR}
```

2. Recompiler — les drivers et display_init vont utiliser des implémentations fictives
3. Valider que le boot réussit et LVGL init complète
4. Correction des dépendances peut attendre (PIXEL peut avancer sans affichage réel)

## Prochaines Étapes (Post-Smoke Test)

- [ ] Si boot OK: passer à Step3 (PIXEL - screens LVGL)
- [ ] Si TFT_eSPI/touch encore en erreur: activer MOCK et continuer
- [ ] Ajouter logs pour tracer appels flush/touch_read
- [ ] Ajouter tests perf: memoria heap/PSRAM, CPU load par task

## Références
- `firmware/include/lvgl_port.h` — API LVGL
- `firmware/include/display_driver.h` — Flush callback
- `firmware/include/touch_driver.h` — Touch read
- `firmware/src/ui_task.cpp` — Task loop

