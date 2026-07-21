# VMT Roadmap

## Status legend

- `[x]` present in the current repository
- `[~]` partially implemented or requires stabilization
- `[ ]` planned

## Foundation

- [x] PlatformIO project for LilyGO T-Display S3
- [x] Arduino framework and GNU++17
- [x] LVGL UI foundation
- [x] Display abstraction
- [x] Semantic input actions
- [x] App catalog and app manager
- [x] Screen navigation layer
- [x] Shared widgets and theme layer
- [x] LittleFS data directory
- [ ] Continuous integration build
- [ ] Release packaging and version automation
- [ ] Host-testable unit tests for pure logic

## Wi-Fi

- [x] Access point scanner service
- [x] Scanner screen
- [x] Selected access point service
- [x] Access point detail screen
- [x] Signal monitor service and screen
- [~] Packet monitor foundation
- [x] Centralized `WiFiManager`
- [x] Radio ownership arbitration
- [ ] Shared bounded scan cache
- [ ] Reliable repeated rescan and recovery tests
- [ ] Channel analyzer
- [ ] Offline OUI/vendor lookup
- [ ] Favorite access points stored in NVS or LittleFS
- [ ] Configurable auto-refresh interval
- [ ] Passive packet statistics and protocol summaries

## Bluetooth Low Energy

- [~] BLE service foundation
- [~] BLE scanner foundation
- [ ] BLE scanner screen
- [ ] Stable device record/cache
- [ ] BLE device detail screen
- [ ] Manufacturer data decoding
- [ ] Service UUID summary
- [ ] RSSI history/monitor
- [ ] Scan lifecycle coordination with Wi-Fi

## External modules

- [~] Common module interface and manager
- [~] CC1101 module placeholder
- [~] NRF24 module placeholder
- [~] LoRa module placeholder
- [~] GPS module placeholder
- [~] RTC module placeholder
- [ ] Runtime module detection
- [ ] Shared SPI bus arbitration
- [ ] Module settings and diagnostics UI

## System features

- [ ] Persistent settings service
- [ ] Theme configuration loading
- [ ] Device information screen
- [ ] Storage diagnostics
- [ ] Power and battery policy
- [ ] Safe sleep/wake flow
- [ ] OTA update strategy
- [ ] Crash diagnostics and reset reason display

## Documentation and release quality

- [x] Agent development instructions
- [x] Architecture documentation
- [x] Coding style documentation
- [x] Hardware documentation
- [x] Contribution workflow
- [ ] Public license decision
- [ ] Screenshots and demo media
- [ ] Stable release checklist
- [ ] Changelog
- [ ] Tagged releases

## Milestone order

### M1 — Stabilize current firmware

- establish CI;
- preserve a known-good build;
- document manual hardware smoke tests;
- remove dead or misleading placeholders from menus.

### M2 — Wi-Fi service completion

- introduce `WiFiManager`;
- centralize radio ownership;
- refactor scanner and signal monitor;
- stabilize repeated scan/rescan;
- add shared scan cache;
- finish passive packet statistics.

### M3 — BLE completion

- stabilize BLE service and scanner;
- add BLE UI and device detail;
- add manufacturer/service decoding;
- coordinate Wi-Fi/BLE lifecycle.

### M4 — External module infrastructure

- finalize module detection and lifecycle;
- define shared bus ownership;
- bring up NRF24, CC1101, and other supported hardware one module at a time.

### M5 — Product polish

- settings persistence;
- power management;
- diagnostics;
- documentation, screenshots, releases, and changelog.
