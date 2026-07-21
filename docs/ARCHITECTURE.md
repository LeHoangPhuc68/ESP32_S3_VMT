# VMT Architecture

## Purpose

VMT is a modular handheld firmware for the LilyGO T-Display S3. The architecture is designed to keep hardware access, service state, application lifecycle, and UI rendering separate so that Wi-Fi, BLE, and external radio modules can evolve without forcing broad rewrites.

## Runtime overview

```text
setup()
  -> Display::begin()
  -> InputManager::begin()
  -> UIManager::begin()

loop()
  -> InputManager::update()
  -> UIManager::handleInput()
  -> UIManager::update()
```

The top-level loop remains intentionally small. Feature-specific state machines must live in services, modules, or screens instead of accumulating in `src/main.cpp`.

## Layer responsibilities

### Core

Location: `lib/Core/`

Responsibilities:

- application identifiers and catalog
- application launch lifecycle
- menu definitions
- shared events and state
- logging contracts

Core must not depend directly on LVGL screens or concrete UI classes.

### Display

Location: `lib/Display/`

Responsibilities:

- panel initialization
- display flushing and hardware interaction
- LVGL display integration

### Input

Location: `lib/Input/`

Responsibilities:

- read physical buttons
- debounce input
- translate input into semantic actions:
  - Previous
  - Next
  - Select
  - Back
  - Home

Input must not decide application behavior.

### Services

Location: `lib/Services/`

Responsibilities:

- own long-lived feature state
- coordinate asynchronous operations
- expose stable, UI-independent APIs
- centralize access to shared radios and platform APIs

Services must not create LVGL objects or control screen navigation.

### Modules

Location: `lib/Modules/`

Responsibilities:

- represent optional hardware such as CC1101, NRF24, LoRa, GPS, and RTC
- implement the common `IModule` contract
- expose presence, initialization, and lifecycle behavior

A module must not be reported as operational unless hardware detection and initialization actually succeed.

### UI

Location: `lib/UI/`

Responsibilities:

- screen navigation
- screen lifecycle
- LVGL object ownership
- widgets
- theme and layout
- translating semantic input into screen behavior

UI may consume services but must not become the owner of Wi-Fi, BLE, storage, or external module drivers.

### Assets

Location: `lib/Assets/` and `assets_src/`

Responsibilities:

- generated images
- icons
- fonts
- asset lookup and lifecycle

Generated C assets should be reproducible from source images through the existing script flow.

## Navigation

Navigation is organized around:

- `AppId`: logical applications
- `ScreenId`: concrete UI destinations
- `AppManager`: application launch state without direct UI dependency
- `ScreenManager`: concrete screen transitions
- `UIManager`: wiring between application requests, screen registration, and input dispatch

The callback boundary in `AppManager` must be preserved so `Core` remains decoupled from `UI`.

## Wi-Fi architecture

The scanner and signal monitor use `WiFiManager` as the single radio and scan owner. Packet monitoring and channel analysis remain planned and must use the same boundary when implemented:

```text
WiFiScannerService ------\
WiFiSignalMonitorService --+--> WiFiManager --> Arduino WiFi / esp_wifi
PacketMonitor ------------/
ChannelAnalyzer ----------/
```

`WiFiManager` responsibilities:

- initialize and shut down the radio
- own station/promiscuous mode transitions
- own scan lifecycle
- arbitrate scan ownership
- cancel and recover timed-out operations
- expose value-copy result access while an owner holds completed scan data
- prevent overlapping operations

Feature services explicitly acquire ownership, request work, copy bounded results, and release ownership. Cancellation stops and cleans the current operation while retaining ownership; release performs the same cleanup and relinquishes ownership. Both operations are idempotent for the owning service. Feature services remain responsible for feature-specific interpretation, not low-level driver control.

## BLE target architecture

```text
BLEScannerScreen
      |
      v
BLEScanner / BLEService
      |
      v
Arduino-ESP32 BLE APIs
```

Expected separation:

- BLE service owns initialization and radio lifecycle.
- Scanner owns scan state and device collection.
- UI owns selection, rendering, and navigation.
- Device records exposed to UI must remain valid for the screen lifetime.

## State machine rules

Long-running actions must use explicit states, for example:

```text
Idle -> Preparing -> Running -> Complete
                    |          |
                    v          v
                  Failed     Cancelled
```

Rules:

- state transitions occur in service `update()` functions;
- UI input callbacks only request operations;
- timeouts use `millis()` and must tolerate wraparound through unsigned subtraction;
- blocking waits and repeated long `delay()` calls are prohibited in normal runtime paths;
- error codes are retained for diagnostics.

## Data ownership

- A class that creates an LVGL object owns its destruction or screen cleanup.
- Service-owned result buffers must not be exposed as pointers that become invalid without a documented state transition.
- Selection services may copy stable records needed across screen transitions.
- Shared caches require a clear single owner.
- Static storage is acceptable for bounded embedded data when ownership is explicit.

## Configuration and secrets

- public configuration belongs in `include/config.h`, `include/pins.h`, or LittleFS data files;
- private values belong in a local ignored `include/secrets.h` based on `include/secrets.example.h`;
- source code must compile without real credentials where possible;
- no secret may be committed.

## Extension strategy

New functionality should usually be added in this order:

1. define the domain record and service state;
2. implement hardware/service logic;
3. add bounded cache or selection state if needed;
4. register the application and screen;
5. implement UI;
6. add build validation and hardware test checklist;
7. update roadmap and documentation.
