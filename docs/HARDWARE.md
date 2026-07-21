# Hardware

## Primary board

- Board: LilyGO T-Display S3
- MCU: ESP32-S3
- PlatformIO board ID: `lilygo-t-display-s3`
- PlatformIO environment: `lilygo`
- Framework: Arduino
- USB CDC on boot: enabled
- Serial monitor baud rate: 115200
- Upload speed: 921600

## Display and UI

- Display integration: Arduino GFX Library
- UI framework: LVGL 9.5.0
- LVGL configuration: `include/lv_conf.h`
- Display implementation: `lib/Display/`
- UI implementation: `lib/UI/`

Exact panel dimensions, rotation, pins, and bus configuration must be taken from the current `Display` and pin configuration source. Do not guess or duplicate them in new code.

## Storage

- Filesystem: LittleFS
- Partition table: `huge_app.csv`
- Runtime files: `data/`

Current data files:

- `data/config.json`
- `data/theme.json`
- `data/wifi.json`

## Input

Input is abstracted by `InputManager` into:

- Previous
- Next
- Select
- Back
- Home

Physical button mapping belongs in the input and pin configuration code, not individual screens.

## Built-in radios

The ESP32-S3 provides:

- 2.4 GHz Wi-Fi
- Bluetooth Low Energy

Radio operations must be coordinated to avoid lifecycle and coexistence conflicts.

## Optional module placeholders

The repository currently contains module classes for:

- CC1101
- NRF24
- LoRa
- GPS
- RTC

Their presence in source does not guarantee that hardware is connected or fully implemented. A feature must detect or initialize the actual module before reporting it as available.

## Pin ownership

Authoritative pin assignments live in:

```text
include/pins.h
```

Rules:

- do not hard-code board pins in service or screen files;
- document shared SPI or I2C buses;
- prevent two modules from driving the same chip-select line;
- validate boot-strapping and USB-related pins before assigning external hardware;
- preserve display and button pin assignments.

## Hardware validation

Firmware changes involving hardware must be tested on the real board for:

- cold boot
- repeated reset
- display initialization
- button input
- USB serial output
- LittleFS access
- radio start/stop cycles
- transition between Wi-Fi and BLE features
- external module detection, when applicable
