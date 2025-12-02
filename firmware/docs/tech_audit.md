# Acyd-Gotchi Firmware – Audit Technique (état stabilisé)

## Ce qui fonctionne aujourd'hui

- **Boot & init système** : le démarrage crée les trois queues FreeRTOS (événements UI, commandes NETSEC, résultats NETSEC) puis lance les tasks `ui_task` et `netsec_task` épinglées sur les deux cœurs avec les priorités prévues. Les stubs faibles restent là en secours, mais PIXEL/NETSEC fournissent leurs implémentations réelles. 【F:src/system_init.cpp†L12-L103】
- **ARCHI – Affichage & tactile** :
  - LVGL est initialisé proprement via `lvgl_port_init`, qui enregistre le driver écran, l’input tactile et démarre un tick `esp_timer` à 1 ms. 【F:src/archi/lvgl_port.cpp†L39-L94】
  - Le driver TFT_eSPI configure un buffer persistant (10 lignes), branche `flush_cb`, met en place l’indev tactile LVGL en s’appuyant sur le wrapper XPT2046, et allume le rétro-éclairage. 【F:src/drivers/display_driver.cpp†L12-L88】
  - Le driver tactile XPT2046 initialise le contrôleur, effectue un mapping des coordonnées et protège l’état via un mutex. 【F:src/drivers/touch_driver.cpp†L17-L83】
- **PIXEL – UI LVGL** :
  - L’initialisation PIXEL stocke la queue UI puis applique un thème simple avant de créer/charger le main screen. 【F:src/ui_main.cpp†L15-L43】
  - `ui_task` tourne sur core 1, appelle `lv_timer_handler` toutes les 5 ms et route les événements issus de la queue vers les écrans (main/WiFi/BLE/Settings). 【F:src/ui_task.cpp†L17-L55】
  - Les écrans principaux (main, WiFi, BLE, Settings) sont créés et chargés sans warnings LVGL ; ils exposent des boutons fonctionnels qui postent des événements dans la queue UI. 【F:src/ui/ui_main_screen.cpp†L23-L140】【F:src/ui/ui_wifi_screen.cpp†L16-L43】【F:src/ui/ui_ble_screen.cpp†L16-L47】【F:src/ui/ui_settings_screen.cpp†L13-L33】
- **NETSEC – Squelette** :
  - Le module initialise le WiFi en STA, offre des API start/stop pour WiFi/BLE et une boucle `netsec_task` consommant une queue de commandes basiques (opcodes 1–4). 【F:src/netsec_core.cpp†L13-L62】【F:src/netsec_core.cpp†L71-L105】
  - Des callbacks WiFi/BLE existent : scan async WiFi avec publication de résultats dans `netsec_result_queue`, début d’implémentation BLE basé sur `BLEDevice`. 【F:src/netsec/netsec_wifi.cpp†L14-L72】【F:src/netsec/netsec_ble.cpp†L11-L48】

## Limites / dettes techniques restantes

- **UI/UX** : mise en page basique (fonds unis, polices par défaut LVGL), pas de véritable retour d’état ni d’animations de pet ; les écrans WiFi/BLE/Settings n’affichent que des listes factices ou un placeholder sans interaction avec les scans. 【F:src/ui/ui_main_screen.cpp†L78-L138】【F:src/ui/ui_wifi_screen.cpp†L29-L41】【F:src/ui/ui_ble_screen.cpp†L29-L41】【F:src/ui/ui_settings_screen.cpp†L13-L33】
- **Touch** : calibration statique (bornes `TS_MIN*` génériques) et absence de feedback visuel ou de test long terme sur la stabilité du tick/handler. 【F:include/board_config.h†L31-L44】【F:src/drivers/touch_driver.cpp†L17-L83】
- **Pipeline UI ↔ NETSEC** : les boutons postent bien des événements, mais aucun mapping n’envoie encore de commandes vers `netsec_command_queue`, et les résultats WiFi/BLE poussés dans la queue ne sont pas consommés par l’UI. 【F:src/ui_task.cpp†L30-L55】【F:src/netsec/netsec_wifi.cpp†L42-L66】【F:src/netsec_core.cpp†L71-L105】
- **NETSEC** : logique encore très stub (pas de timeout/stop réels de scan WiFi, BLE non connecté aux résultats, handshake capture non implémenté). 【F:src/netsec/netsec_wifi.cpp†L34-L72】【F:src/netsec/netsec_ble.cpp†L32-L48】【F:src/netsec_core.cpp†L50-L68】
- **Robustesse & instrumentation** : pas de monitoring de mémoire/tick LVGL à long terme, ni de drivers mock pour tests sans matériel. Les paramètres PlatformIO restent simples mais non documentés côté partitions/PSRAM. 【F:src/archi/lvgl_port.cpp†L73-L94】【F:platformio.ini†L1-L24】

## Roadmap courte (proposée)

1. **Phase 1 – UI principale fonctionnelle (PIXEL)**
   - Nettoyer le thème (fonds neutres, marges, contrastes lisibles) et ajouter 2–3 boutons interactifs (WiFi, BLE, About) avec états pressés.
   - Dans `ui_task`, mapper les événements boutons vers des commandes envoyées sur `netsec_command_queue` et prévoir un handler de résultats pour rafraîchir les listes WiFi/BLE.

2. **Phase 2 – Touch & interactions (ARCHI + PIXEL)**
   - Affiner la calibration (`TS_MIN*`) après mesures terrain et ajouter un retour visuel sur la pression (changement de style bouton).
   - Vérifier la stabilité du tick LVGL et des buffers sur des sessions prolongées ; instrumenter quelques logs/metrics légers.

3. **Phase 3 – NETSEC mock réaliste (NETSEC + PIXEL)**
   - Implémenter des scans mock (WiFi/BLE) qui publient des listes factices régulières via `netsec_result_queue`.
   - Côté UI, afficher ces résultats dans les écrans listes et offrir une navigation simple (sélection, retour main screen).
   - Formaliser les structures d’événements/commandes dans `netsec_api.h` pour préparer l’intégration réelle.

4. **Phase 4 – Vers des scans réels**
   - Étendre les stubs : gestion de durée/annulation de scan, publication d’erreurs/états, et gating LAB_MODE pour toute action sensible (handshake capture, BLE pairing).
   - Documenter partitions/PSRAM si nécessaire et ajouter des hooks de supervision (compteurs d’allocation, stats tick) pour sécuriser les longues sessions.

## TODO synthétique par domaine

- **ARCHI** : valider calibration tactile, surveiller la stabilité du tick LVGL (esp_timer + task UI) et documenter la config PlatformIO/partition adaptée CYD.
- **PIXEL** : améliorer le thème, connecter la queue UI aux commandes NETSEC et consommer les résultats pour lister WiFi/BLE ; ajouter feedback tactile/état pressé.
- **NETSEC** : compléter la boucle commande→scan→résultat (mock d’abord), gérer timeout/stop WiFi, intégrer BLE avec publication d’événements, et définir les structures d’API pour les futures opérations réelles.
