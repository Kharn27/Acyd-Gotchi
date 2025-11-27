# Acyd-Gotchi  
Firmware modulaire pour ESP32-CYD (Cheap Yellow Display) combinant interface graphique LVGL, tactile XPT2046, affichage TFT_eSPI et moteur FreeRTOS.  
Projet visant à créer un Tamagotchi moderne, extensible, mêlant UI avancée, IA, stockage et modules réseau (WiFi/BLE scan).

---

## 🚀 Objectif du projet

Acyd-Gotchi est un firmware écrit pour l’ESP32-2432S028R (ESP32 CYD) qui combine :

- une **UI moderne** (LVGL),
- un **moteur de scènes**,
- un **Tamagotchi évolutif**,
- des **modules FreeRTOS** pour les tâches système,
- un futur **module NetSec** (scan WiFi/BLE, analyse, affichage temps réel).

Le but est de construire un firmware propre, modulaire, maintenable, entièrement piloté par un architecte IA (Codex / local agent), assisté de sous-agents spécialisés.

---

## 🧩 Architecture globale

L’architecture suit 4 principes :

1. **Modularité** → chaque domaine possède son fichier d’API + implémentation.
2. **FreeRTOS** → boucle LVGL séparée, module NetSec indépendant, system monitor.
3. **Communication inter-modules** via queues FreeRTOS.
4. **Indépendance du matériel** → drivers encapsulés dans `system_init.*`, `board_config.h`.

### Modules principaux

| Module | Description |
|--------|-------------|
| **system_init** | Init hardware (PSRAM, TFT, XPT2046, SPI, LVGL tick). |
| **ui_main** | UI principale, intégration LVGL, dispatch des scènes. |
| **tasks_impl** | FreeRTOS tasks : UI, NetSec, System Monitor. |
| **netsec_core** | (WIP) scans WiFi/BLE + systèmes événements. |
| **ui_api.h** | Interface que PIXEL (agent UI) utilise pour créer les écrans. |
| **netsec_api.h** | Interface que NETSEC (agent sécurité) utilise pour les scans. |
| **board_config.h** | Pins du TFT, tactile, SPI et options matérielles. |

---

## 🖼 Technologies utilisées

- **Framework** : Arduino (via PlatformIO)
- **RTOS** : FreeRTOS (inclus ESP32)
- **UI** : LVGL 9.x
- **Affichage** : TFT_eSPI (User_Setup.h fourni)
- **Tactile** : XPT2046_Touchscreen
- **Stockage** : SPIFFS / LittleFS (futur)
- **Hardware** : ESP32-2432S028R (CYD), écran ILI9341 320×240

---

## 🐣 Vision long terme (Tamagotchi-like)

Le pet possède :

- des besoins internes (faim, énergie, social, propreté, curiosité),
- une humeur globale,
- une petite **Utility AI** (choisir des actions selon les besoins),
- des animations simples,
- une **sauvegarde SPIFFS**,
- un cycle jour/nuit,
- la possibilité d’être étendu avec mini-jeux.

---

## 🛰 Vision long terme (module NetSec)

En parallèle du jeu :

- scan WiFi,
- scan BLE,
- affichage Live View,
- logs,
- vues statistiques,
- mode “analyse passive”.

Ce module tourne sur une task FreeRTOS indépendante.

---

## 🤖 Agents IA utilisés dans ce projet

Le projet est conçu pour fonctionner avec plusieurs agents :

### **THEBOSS**
Superviseur général :
- planifier le travail,
- créer les tâches,
- valider les merges,
- déléguer aux sous-agents.

### **ARCHI**
Agent architecte firmware :
- structure du code,
- définition des API,
- cohérence PlatformIO/LVGL/TFT,
- gestion RTOS.

### **PIXEL**
Agent UI/UX :
- LVGL,
- rendu,
- couleurs,
- composants.

### **NETSEC**
Agent sécurité :
- WiFi/BLE scans,
- analyse,
- datastructures.

Ces fichiers d’agents se trouvent dans `.github/agents/`.

---

## 📂 Structure du repository

```

Acyd-Gotchi/
│
├── firmware/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── system_init.cpp
│   │   ├── ui_main.cpp
│   │   ├── tasks_impl.cpp
│   │   ├── netsec_core.cpp
│   │   └── ...
│   ├── include/
│   │   ├── system_init.h
│   │   ├── ui_api.h
│   │   ├── netsec_api.h
│   │   ├── board_config.h
│   │   └── tasks.h
│   ├── lib/
│   ├── .pio/
│   ├── platformio.ini
│   └── User_Setup.h (TFT)
│
├── .github/
│   ├── agents/
│   │   ├── ARCHI.agent.md
│   │   ├── PIXEL.agent.md
│   │   ├── NETSEC.agent.md
│   │   └── THEBOSS.agent.md
│   └── workflows/
│
└── README.md   ← (ce fichier)

```

---

## 🧪 Build & Flash

### Compilation

```

cd firmware
pio run -e esp32-cyd

```

### Flash

```

pio run -e esp32-cyd -t upload

```

### Monitor

```

pio device monitor

```

---

## 📝 État actuel

- Architecture FreeRTOS créée
- LVGL initialisation en cours de stabilisation
- Drivers TFT_eSPI/XPT2046 en intégration
- API UI & NetSec prêtes
- Build encore instable (User_Setup / lv_conf)

---

## 🧠 Notes importantes pour Codex / Agents

- **Ne jamais réécrire massivement** sans raison validée par THEBOSS.
- Travailler **par tâches, par phases**, conformément au plan.
- Toujours partir du **code actuel du repo**, pas d’une reconstruction.
- Les modifications doivent rester **locales, sûres, et diffables**.
- Toute décision d’architecture majeure doit être documentée.

---

## 📜 Licence

MIT — libre utilisation.
