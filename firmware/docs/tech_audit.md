# Acyd-Gotchi Firmware – Audit Technique (Step 1)

## Synthèse rapide
- Projet PlatformIO Arduino pour ESP32 CYD (profil `esp32dev` par défaut).
- Stack active : FreeRTOS + LVGL + TFT_eSPI + XPT2046, avec drivers d'affichage/tactile opérationnels et buffers LVGL persistants.
- Les API UI/NETSEC ont été alignées (événements `UI_EVENT_*`, structures `netsec_result_t` cohérentes), mais il reste quelques verrous de compilation et du câblage inter-tâches à compléter.

## Points bloquants (compilation / exécution)
1. **Headers NETSEC hors chemin d'inclusion** : `netsec_core.cpp` et les modules WiFi/BLE incluent `"netsec_core.h"` / `"netsec_wifi.h"` / `"netsec_ble.h"` sans préfixe alors que les headers résident dans `include/netsec/`. Avec l'include path actuel, la compilation échoue par fichier introuvable. 【F:src/netsec_core.cpp†L6-L12】【F:src/netsec/netsec_wifi.cpp†L1-L4】【F:include/netsec/netsec_core.h†L1-L16】
2. **Boucle NETSEC sans producteurs** : `netsec_task` lit `netsec_command_queue` mais aucune tâche n'y pousse de commandes ; l'initialisation NETSEC stocke un `result_queue` local qui n'est jamais utilisé (les posts passent par le global). La tâche tourne à vide et les scans ne démarrent jamais. 【F:src/netsec_core.cpp†L41-L79】【F:src/system_init.cpp†L30-L63】

## Problèmes importants (structure / stabilité)
1. **Configuration PlatformIO générique** : board `esp32dev`, partitions/PSRAM non configurées ; dépend d'un `User_Setup.h` manuel et n'assure pas la compatibilité CYD par défaut. 【F:platformio.ini†L1-L27】【F:User_Setup.h†L1-L24】
2. **Drivers NETSEC minimalistes** : WiFi/BLE publient des résultats via queues mais ne gèrent ni arrêt de scan, ni lab mode global dans les callbacks, ni débit de queue (pas de backpressure). 【F:src/netsec/netsec_wifi.cpp†L11-L55】【F:src/netsec/netsec_ble.cpp†L11-L43】
3. **Pipeline UI ↔︎ NETSEC incomplet** : la UI poste bien des événements utilisateurs, mais aucune traduction en opcodes NETSEC ni consommation des `netsec_result_t` pour afficher des résultats (écrans WiFi/BLE purement statiques). 【F:src/ui/ui_main_screen.cpp†L159-L196】【F:src/ui_task.cpp†L31-L68】【F:include/netsec_api.h†L30-L62】

## Risques / dette long terme
1. **Séparation des responsabilités encore floue** : ARCHI a centralisé l'affichage/tactile, mais NETSEC manipule toujours directement WiFi/BLE sans façade mockable, et UI conserve des dépendances globales (`ui_event_queue`). 【F:src/drivers/display_driver.cpp†L12-L80】【F:src/netsec/netsec_wifi.cpp†L16-L55】【F:src/ui_task.cpp†L31-L68】
2. **Stratégie mémoire non validée** : buffers LVGL statiques (~10 lignes) et stack tasks dimensionnée mais sans contrôle PSRAM/heap ; aucun monitoring ou fallback en cas de RAM insuffisante. 【F:src/drivers/display_driver.cpp†L12-L46】【F:include/board_config.h†L37-L44】
3. **Absence de modes mock/test** : les flags `DISPLAY_MOCK`/`MOCK_TOUCH` existent dans les build flags, mais aucune implémentation de drivers mock pour CI ou développement sans matériel. 【F:platformio.ini†L14-L20】【F:src/drivers/display_driver.cpp†L1-L71】

## Plan de correction proposé
- 🔴 **Critique (bloc compilation / fonctionnement)**
  1. Ajouter `include/netsec` au chemin d'inclusion ou référencer explicitement `netsec/netsec_*.h` dans les sources NETSEC pour rétablir la compilation.
  2. Brancher la pipeline UI→NETSEC : mapper les événements boutons vers des opcodes `netsec_command_queue` et consommer `netsec_result_queue` pour alimenter les écrans WiFi/BLE (même en mode mock).
- 🟠 **Important (stabilité/structure)**
  1. Spécialiser `platformio.ini` pour la CYD (pins, partitions flash/PSRAM) et documenter l'usage de `User_Setup.h`/`lv_conf.h`.
  2. Étoffer NETSEC : gestion d'arrêt/timeout de scan, garde `LAB_MODE_ENABLED` dans les callbacks, et protection de la queue contre la saturation.
  3. Introduire des drivers mock activables via `build_flags` pour valider LVGL/queues sans matériel.
- 🟢 **Architecture long terme**
  1. Documenter la cartographie CYD complète (SPI, backlight, touch IRQ) et publier une stratégie mémoire (buffers en PSRAM, tailles dynamiques ajustables).
  2. Encapsuler WiFi/BLE derrière des façades testables pour permettre des scénarios de lab sécurisés et du test automatique.
  3. Formaliser les API inter-modules (`ui_api.h`, `netsec_api.h`, `tasks.h`) dans une documentation courte pour limiter les globals et préparer l'intégration Step 2/3.
