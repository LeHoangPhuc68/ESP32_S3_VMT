# ESP32_S3_VMT

VMT is a modular handheld firmware project for the LilyGO T-Display S3 (ESP32-S3). It is built with PlatformIO, the Arduino framework, LVGL, Arduino GFX, ArduinoJson, and LittleFS.

## Current foundation

The repository currently includes:

- display initialization and LVGL integration;
- semantic button input;
- application catalog and application manager;
- screen navigation and shared UI widgets;
- Wi-Fi scanner, access point selection/detail, and signal monitoring foundations;
- BLE service and scanner foundations;
- optional module abstractions for CC1101, NRF24, LoRa, GPS, and RTC;
- LittleFS configuration data.

## Build

Requirements:

- VS Code
- PlatformIO extension
- LilyGO T-Display S3

Build command:

```bash
pio run -e lilygo
```

Upload command:

```bash
pio run -e lilygo -t upload
```

Serial monitor:

```bash
pio device monitor -b 115200
```

## Project structure

```text
assets_src/      source images used for generated assets
data/            LittleFS files
include/         board configuration, pins, LVGL config, version, secrets template
lib/Assets/      generated and managed UI assets
lib/Core/        application lifecycle, catalog, events, shared state, menus
lib/Display/     display hardware abstraction
lib/Input/       button input and semantic actions
lib/Modules/     optional external hardware modules
lib/Services/    Wi-Fi and BLE feature logic
lib/UI/          navigation, screens, widgets, theme, UI manager
scripts/         build helper scripts
src/             firmware entry point
test/            test placeholders and future host-testable logic
```

## Architecture rules

- Core does not depend directly on UI.
- Services do not create LVGL objects or navigate screens.
- UI consumes services but should not own low-level radio drivers.
- Hardware APIs should be centralized behind managers or services.
- Runtime work should be non-blocking and state-driven.
- `main` should remain buildable.

See:

- `AGENTS.md`
- `docs/ARCHITECTURE.md`
- `docs/CODING_STYLE.md`
- `docs/HARDWARE.md`
- `docs/ROADMAP.md`
- `docs/CODEX_WORKFLOW.md`
- `CONTRIBUTING.md`

## Secrets

Copy `include/secrets.example.h` to a local `include/secrets.h` only when private values are required. Never commit real credentials, API tokens, private keys, or passwords.

## Safety scope

VMT focuses on device diagnostics, passive scanning, monitoring, and authorized experimentation. Offensive Wi-Fi or Bluetooth interference features are outside the project scope.

## Project status

The firmware is under active development. Source presence does not mean every external module or feature is fully implemented or hardware-validated. Consult `docs/ROADMAP.md` for status and planned work.
