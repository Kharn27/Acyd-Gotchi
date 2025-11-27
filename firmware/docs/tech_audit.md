# Acyd-Gotchi Firmware – Audit Technique (Step 0)

## Synthèse rapide
- Projet PlatformIO Arduino pour ESP32 CYD (carté comme `esp32dev`).
- Empilement prévu : FreeRTOS + LVGL + TFT_eSPI + XPT2046.
- Plusieurs incohérences bloquantes entre API déclarées et implémentations (UI/NETSEC/ARCHI).

## Points bloquants (compilation / exécution)
1. **Constantes d’affichage/tactile absentes ou divergentes** : le code utilise `DISP_HOR_RES`, `DISP_VER_RES`, `TOUCH_CS`, `TS_MINX`, `TS_MAXX`, etc. qui ne sont définies nulle part (`board_config.h` expose `DISPLAY_WIDTH/HEIGHT` et `TOUCH_CS_PIN`). 【F:src/drivers/display_init.cpp†L36-L77】【F:src/drivers/touch_driver.cpp†L15-L64】【F:include/board_config.h†L6-L20】
2. **Definitions d’événements UI incohérentes** : `ui_api.h` expose `UI_EV_*` alors que `ui_task.cpp` consomme `UI_EVENT_*` et `ui_main_screen.cpp` envoie aussi `UI_EVENT_*`. Compilation impossible faute d’enum commun. 【F:include/ui_api.h†L8-L31】【F:src/ui_task.cpp†L32-L55】【F:src/ui/ui_main_screen.cpp†L66-L96】
3. **API NETSEC en conflit** : `netsec_api.h` définit `netsec_result_t` avec union `wifi_ap/ble_device`, mais `netsec_ble.cpp` référence `NETSEC_RESULT_BLE_DEVICE` et des champs `res.ble.*` inexistants. 【F:include/netsec_api.h†L10-L46】【F:src/netsec/netsec_ble.cpp†L29-L53】
4. **Types ESP32 WiFi events incorrects** : `netsec_wifi.cpp` utilise `system_event_id_t/system_event_info_t` (ESP-IDF) avec `WiFi.onEvent`, ce qui ne compile pas avec l’API Arduino actuelle. 【F:src/netsec/netsec_wifi.cpp†L1-L46】
5. **Buffers LVGL non persistants** : `display_init` déclare `lv_disp_draw_buf_t draw_buf` en variable locale puis enregistre son pointeur, provoquant un use-after-return. 【F:src/drivers/display_init.cpp†L68-L90】
6. **g_disp/g_indev non initialisés dans lvgl_port** : les getters retournent toujours `NULL`, car `display_init` ne remonte pas les pointeurs. 【F:src/archi/lvgl_port.cpp†L15-L70】
7. **Définitions multiples de tasks** : `system_init.cpp` fournit des stubs faibles, `tasks_impl.cpp` implémente `ui_task/netsec_task`, et `netsec_core.cpp` redéfinit `netsec_task`. Risque d’erreur d’édition de liens et comportement imprévisible. 【F:src/system_init.cpp†L16-L85】【F:src/tasks_impl.cpp†L12-L42】【F:src/netsec_core.cpp†L40-L67】
8. **Headers manquants** : `netsec_core.h` est inclus mais inexistant, bloquant la compilation. 【F:src/netsec_core.cpp†L6-L11】

## Problèmes importants (structure / stabilité)
1. **Configuration PlatformIO générique** : board `esp32dev` au lieu d’un profil CYD, aucune partition flash/PSRAM, pas de `User_Setup.h` ou `lv_conf.h` fournis, risque d’échec TFT_eSPI/LVGL et de mapping pins incorrect. 【F:platformio.ini†L1-L33】
2. **Driver display/touch incomplet** : callbacks LVGL stub (flush/touch), absence de calibration et de mutex partagé avec le tactile, pas de gestion DMA ni orientation précise CYD. 【F:src/drivers/display_init.cpp†L92-L129】【F:src/drivers/touch_driver.cpp†L65-L103】
3. **Queue et API non alignées avec la navigation prévue** : `ui_event_queue` reçoit des événements sans mapping aux actions netsec (start scan, etc.), et `netsec_command_queue` n’est jamais alimentée. 【F:src/system_init.cpp†L30-L76】【F:src/ui_task.cpp†L43-L55】【F:src/netsec_core.cpp†L40-L67】
4. **Initialisation LVGL partielle** : `lvgl_port_init` n’appelle pas `lv_disp_drv_register` directement et ne capture pas le retour du driver créé dans `display_init`, pas de gestion de fréquence `lv_timer_handler`, tick 1ms lancé sans contrôle de charge. 【F:src/archi/lvgl_port.cpp†L24-L70】
5. **Dépendances BLE/WiFi non encapsulées** : Netsec manipule directement WiFi/BLE sans garde de “lab mode” ni isolation des callbacks/queues, rendant difficile le test ou le mock. 【F:src/netsec_core.cpp†L20-L67】【F:src/netsec/netsec_wifi.cpp†L1-L46】

## Risques / dette long terme
1. **Séparation des responsabilités floue** : ARCHI (drivers), PIXEL (UI), NETSEC (scans) s’entrecroisent (UI envoie événements via globals, drivers exposent globals), ce qui compliquera les évolutions Step 2/3.
2. **Absence de configuration matérielle centralisée** : pins SPI/tactile/SD non documentés, aucune table de routage CYD ; l’usage de macros divergentes multiplie les risques d’erreur.
3. **Pas de stratégie mémoire** : allocations dynamiques pour buffers LVGL sans PSRAM/heap monitoring ; absence de double buffering calibré ou de réduction de consommation RAM.

## Plan de correction proposé
- 🔴 **Critique (bloc compilation)**
  1. Unifier les constantes d’affichage/tactile (exposer `DISP_HOR_RES/VER_RES`, `TOUCH_CS`, limites TS_* dans `board_config.h` ou `User_Setup.h`) et aligner tout le code dessus.
  2. Harmoniser l’API UI : une seule enum d’événements, utilisée par `ui_task`, `ui_main_screen` et `ui_api.h`; supprimer doublons de tâches.
  3. Réparer NETSEC API : aligner `netsec_result_t` et les producteurs BLE/WiFi, créer le header `netsec_core.h` et choisir une seule implémentation de `netsec_task`.
  4. Corriger `netsec_wifi.cpp` pour l’API Arduino (handlers `WiFi.onEvent` modernes) ou stubber en Step 1.
  5. Rendre le buffer LVGL persistant (buffer statique ou global) et raccorder `lvgl_port` aux handles `lv_disp_t*/lv_indev_t*`.
- 🟠 **Important (stabilité/structure)**
  1. Ajouter `User_Setup.h` (TFT_eSPI) et `lv_conf.h` spécifiques CYD via `platformio.ini`/`build_flags`; définir board ou `board_build` adaptés (pins, PSRAM, partitions).
  2. Finaliser drivers display/touch : flush réel, calibration XPT2046, mutex partagé, orientation CYD, option mock via flags.
  3. Définir protocole de queues : événements UI → commandes NETSEC (opcodes), résultats NETSEC → UI (structs), avec timeouts non bloquants.
  4. Initialiser LVGL proprement : cadence `lv_timer_handler` (~5–10 ms), tick 1 ms conditionné, stockage du display/indev dans `lvgl_port`.
- 🟢 **Architecture long terme**
  1. Documenter la cartographie CYD (pins SPI, backlight, touch IRQ) et la politique mémoire (buffers en PSRAM, tailles dynamiques).
  2. Séparer clairement responsabilités : ARCHI (drivers/port), PIXEL (screens), NETSEC (scans + lab gating) avec headers API stables (`ui_api.h`, `netsec_api.h`, `tasks.h`).
  3. Préparer des modes mock/test (DISPLAY_MOCK/TOUCH_MOCK) activables dans `platformio.ini` pour CI et développement sans matériel.

## Feuille de route immédiate (proposition)
1. Valider les conventions d’API (events UI, queues, opcodes NETSEC) et renommer les constantes d’écran/tactile de manière unifiée.
2. Ajouter les fichiers de configuration manquants (`User_Setup.h`, `lv_conf.h`) et ajuster `platformio.ini` au profil CYD.
3. Simplifier les tâches : une seule implémentation `ui_task` (boucle LVGL + dispatch events) et `netsec_task` (lecture commandes + mocks scan) compatible Step 1.
4. Sécuriser les drivers : buffers LVGL persistants, callbacks touch calibrés, mutex commun SPI/touch, pointeurs renvoyés à `lvgl_port`.
5. Ensuite seulement, ouvrir Step 2 (affichage réel) puis Step 3 (screens interactifs) et Step 4 (scans async lab-mode).

Merci de confirmer ces axes avant d’assigner ARCHI/PIXEL/NETSEC ou d’ouvrir des PRs correctives.
