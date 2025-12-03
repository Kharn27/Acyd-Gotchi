## Problème : Panic sur scan BLE (OOM contrôleur BT)

**Contexte**  
Carte ESP32 CYD (PlatformIO, Arduino). Lancement d’un scan BLE via l’UI → reboot avec Guru Meditation (StoreProhibited) dans la pile BT (bluedroid) lors de l’init BLE.

**Symptôme**  
- Crash immédiat au démarrage du scan BLE.  
- Backtrace (décodée) : `memset` → `bta_sys_init` → `btu_task_start_up` → `osi_thread_run`.  
- Après ajout d’une garde mémoire, le scan est avorté proprement : plus de crash, mais BLE indisponible si mémoire insuffisante.

**Décodage backtrace (addr2line)**  
- `0x40091a49`: `memset` (/newlib/memset.S)  
- `0x40169eac`: `bta_sys_init` (esp-idf components/bt/host/bluedroid/bta/sys/bta_sys_main.c:171)  
- `0x40149459`: `btu_task_start_up` (components/bt/host/bluedroid/stack/btu/btu_task.c:276)  
- `0x40164cef`: `osi_thread_run` (components/bt/common/osi/thread.c:165)

**Cause probable**  
OOM dans la pile BT (allocation NULL) pendant `BLEDevice::init`, aboutissant à un `memset` sur pointeur nul dans `bta_sys_init`.

**État après fix provisoire** (`firmware/src/netsec/netsec_ble.cpp`)  
- Release mémoire BT classique : `esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT)`.  
- Garde mémoire avant init BLE : log du heap 8-bit (`free` / `largest`) et seuil minimal ~70 KB.  
- Si insuffisant : log `[NETSEC:BLE] Not enough heap for BLE init, aborting scan` et on sort sans crasher.

**Logs récents**  
```
[NETSEC] BLE scan duration=10000 ms
[NETSEC:BLE] Heap free=61052 bytes, largest=55284 bytes
[NETSEC:BLE] Not enough heap for BLE init, aborting scan
```

**Impact**  
- Plus de panic, mais scan BLE impossible tant que la mémoire libre/contiguë reste sous le seuil. Pas de résultats BLE.

**Reproduction**  
1. Flasher firmware actuel.  
2. Depuis l’UI, lancer un scan BLE (10 s).  
3. Observer logs série (garde mémoire) et absence de scan.

**Pistes de résolution**  
1. Libérer de la RAM : réduire `NETSEC_TASK_STACK_SIZE` / `UI_TASK_STACK_SIZE` (board_config.h), réduire `LVGL_BUFFER_SIZE`, désactiver assets lourds (fonds/polices).  
2. Couper le Wi-Fi avant init BLE si non utilisé en parallèle (`WiFi.mode(WIFI_OFF)` / `esp_wifi_stop()`), puis rétablir après le scan.  
3. Basculer sur NimBLE (pile BLE plus légère) à moyen terme.  
4. Ajuster le seuil mémoire si on confirme qu’un bloc contigu ~55 KB suffit sur cet IDF/Arduino.

**Action minimale actuelle**  
La garde empêche le panic et avorte le scan si RAM insuffisante ; prévoir un message UI “mémoire insuffisante pour scan BLE” si on conserve ce comportement.
