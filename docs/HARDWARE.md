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

Input is abstracted by `InputManager` into four semantic actions:

| Physical button | Press | Action | UI behavior |
| --- | --- | --- | --- |
| KEY (GPIO14) | Short | `Next` | Move focus to the next selectable item |
| KEY (GPIO14) | Long | `Select` | Select, open, or confirm the focused item |
| BOOT (GPIO0) | Short | `Primary` | Run the screen's contextual primary action |
| BOOT (GPIO0) | Long | `Back` | Return one screen or menu level |

The button driver uses a centralized 25 ms debounce interval and a 200 ms long-press threshold. Detection uses `millis()` without blocking delays. A generated long-press event suppresses the short-click event on release.

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
