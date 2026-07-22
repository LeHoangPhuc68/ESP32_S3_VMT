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
  - Next
  - Select
  - Primary
  - Back

Input must not decide application behavior.

The two physical buttons use one consistent mapping:

| Physical input | Semantic action | Meaning |
| --- | --- | --- |
| KEY short | `Next` | Move focus to the next selectable item |
| KEY long | `Select` | Open, select, or confirm the focused item |
| BOOT short | `Primary` | Run the current screen's contextual primary action |
| BOOT long | `Back` | Return exactly one screen or menu level |

Debounce and long-press detection are non-blocking and centralized in the button driver. A long press is emitted once while held and suppresses the release click, so one physical press produces one semantic action. There is no two-button Home chord.

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

Navigation uses a fixed parent map rather than an unbounded history stack:

```text
Home landing
  -> Main menu grid
     -> Dashboard
     -> Wi-Fi menu
        -> Wi-Fi Scanner
           -> Access Point detail
              -> Signal Monitor
        -> Channel Analyzer
        -> Packet Monitor
```

The Back parents are explicit: the main menu closes to Home, Dashboard and the Wi-Fi menu return to the main menu, Scanner, Channel Analyzer, Packet Monitor, and Signal Monitor return to the Wi-Fi menu, and AP detail returns to Scanner. Back at the Home landing is a no-op. Repeated navigation therefore cannot grow a stack or underflow. Normal `ScreenManager` transitions always hide the departing screen first, preserving feature cleanup such as Scanner and Channel Analyzer refresh cancellation and Signal Monitor or Packet Monitor stop/release.

The main menu is a 3-by-2 icon grid sized for the 320-by-170 landscape display. All six top-level functions remain visible at once and KEY advances focus in row-major order. Implemented tiles open with KEY long or BOOT short. Unimplemented tiles remain visible with dimmed styling and a `SOON` marker; activation shows the existing lightweight availability message without navigating to an empty screen.

## Wi-Fi architecture

The scanner, signal monitor, and passive Packet Monitor use `WiFiManager` as the single radio owner. Channel Analyzer is a read-only consumer of completed Scanner snapshots and requests refreshes through `WiFiScannerService`; it does not acquire a second radio owner:

```text
Channel Analyzer UI --refresh--> WiFiScannerService --> WiFiManager
                                             |              |
                                             v              v
                                  WiFiScanSnapshot    Arduino WiFi / esp_wifi
                                             |
                                             v
                                  WiFiChannelAnalyzer

WiFiSignalMonitorService ---------------------------> WiFiManager
PacketMonitor --------------------------------------> WiFiManager
```

`WiFiManager` responsibilities:

- initialize and shut down the radio
- own station/promiscuous mode transitions
- own scan lifecycle
- own promiscuous callback installation, filtering, channel selection, and cleanup
- arbitrate scan ownership
- cancel and recover timed-out operations
- expose value-copy result access while an owner holds completed scan data
- prevent overlapping operations

Feature services explicitly acquire ownership, request work, copy bounded results, and release ownership. Cancellation stops and cleans the current operation while retaining ownership; release performs the same cleanup and relinquishes ownership. Both operations are idempotent for the owning service. Feature services remain responsible for feature-specific interpretation, not low-level driver control.

Ownership arbitration is cooperative and is currently expected to be called from the main application loop. `WiFiManager` does not synchronize concurrent calls from multiple FreeRTOS tasks and is not thread-safe for that use. Calling `acquire()` again for the current owner succeeds idempotently, but acquisition is not reference-counted; one matching `release()` relinquishes ownership.

Completed full Scanner results use one service-owned snapshot:

```text
WiFiManager Ready driver results
  -> WiFiScannerService bounded staging copy and sort
  -> WiFiManager release and driver-result cleanup
  -> WiFiScanSnapshot completed publication
  -> UI / Selection read const entries
```

`WiFiScanEntry` is the canonical copied AP record. `WiFiScanSnapshot` has a fixed capacity of 16 entries plus success status, count, generation and `millis()` completion time. Snapshot data is exposed read-only and contains owned `String` values and copied scalars only. A successful publication, including a zero-result success, increments generation and replaces the previous snapshot as one logical main-loop operation after ownership release succeeds. Failed, rejected and cancelled scans, including release failure, leave the prior successful publication unchanged. Entry pointers are service-owned and remain valid only until the next successful Scanner publication or explicit clear.

Selection and Signal Monitor use copied canonical entries for stable targets across navigation. Signal Monitor continues to inspect all transient channel-scan results through `WiFiManager` while it owns the radio; it does not publish those sampling scans into the Scanner snapshot. This preserves the full Scanner list and avoids losing a target that falls beyond the bounded snapshot capacity. Packet Monitor acquires its dedicated owner but does not consume or publish scan snapshots. The existing Channel Analyzer owner identity remains reserved, but the implemented analyzer deliberately does not use it because its refresh is a normal Scanner operation owned by `WiFiScannerService`.

### Channel Analyzer

`WiFiChannelAnalyzer` synchronously derives a fixed 13-entry metrics array for 2.4 GHz channels 1 through 13 from a successful `WiFiScanSnapshot`. It retains no snapshot entry pointers, SSIDs, BSSIDs or driver data. For each channel it records the APs observed exactly on that channel, strongest and average RSSI, a bounded interference score, and whether the channel is the single recommendation. Channels outside 1 through 13 are ignored. The analysis object occupies approximately 116 bytes of persistent C++ state; its temporary RSSI sums use 52 bytes of stack while rebuilding.

For each observed AP, RSSI is clamped to -100 through -30 dBm and converted to an impact from 10 through 80:

```text
impact = 10 + (clamp(RSSI, -100, -30) + 100)
overlapWeight(distance) = 5 - distance, for distance 0 through 4
overlapWeight(distance) = 0, for distance 5 or greater
channelScore = sum(impact * overlapWeight)
```

The 5/4/3/2/1 overlap weights make nearby primary channels contribute progressively less than an AP on the candidate channel. With at most 16 snapshot entries, the score cannot exceed 6400 and fits in `uint16_t`. Lower is better. The unrestricted minimum is found first, with the lower channel winning a tie. The best of channels 1, 6 and 11 is preferred only when its score is within `max(20, 15% of the unrestricted minimum)` of that minimum; ties among those preferred channels resolve in 1, 6, 11 order. A successful zero-result snapshot therefore produces zero metrics and deterministically recommends channel 1 as a no-evidence default.

This recommendation is a bounded heuristic, not a guarantee. The snapshot contains at most 16 copied scan records and has no channel width, secondary-channel, airtime or utilization information, so displayed AP counts mean APs observed in that bounded snapshot rather than authoritative RF totals. Regional channel availability also remains a deployment consideration.

On entry, Channel Analyzer displays an existing successful snapshot immediately and starts a Scanner refresh only when no successful snapshot exists. BOOT short requests a refresh through `WiFiScannerService`. The previous analysis remains visible while scanning and after a failed refresh; a new analysis is committed only when snapshot generation changes after a successful Scanner publication. Successful zero-result publications are valid updates. Leaving the screen cancels any active Scanner operation and releases ownership through the existing Scanner service cleanup path.

The screen shows all 13 channels at once with AP-count labels and interference bars. KEY short advances the selected channel with wraparound, KEY long toggles its bounded detail view, BOOT short refreshes, and BOOT long cancels active work and returns one level to the Wi-Fi menu.

### Passive Packet Monitor

Packet Monitor observes traffic on one manually selected 2.4 GHz channel from 1 through 13 using ESP32-S3 promiscuous receive mode. `PacketMonitor` acquires its dedicated manager owner before start. `WiFiManager` alone prepares station mode, selects the channel, configures the management/control/data receive filter, installs the callback thunk, enables promiscuous mode, neutralizes callback dispatch on stop, restores the idle radio state, and releases ownership. Start and stop are bounded; any partial failure runs the same cleanup path. A release failure invokes manager-owned power-off recovery. If even that bounded recovery cannot prove the radio inactive, the manager deliberately remains failed and owned instead of exposing an uncertain radio to another feature.

The feature is passive receive and aggregate analysis only. It performs no frame injection, raw 802.11 transmission, attack behavior, credential parsing, payload retention, packet display, PCAP capture, filesystem logging, or external-module access. It validates that at least the two-byte frame-control field is present and uses explicit masks instead of C bitfields to classify management, control, and data frames. Management counters cover beacons, probe requests/responses, association requests/responses, disassociation, and observed deauthentication frames.

The driver callback exposes only a transient frame view and updates a fixed POD interval accumulator. A short FreeRTOS critical section protects only counter/RSSI transfer; the callback performs no UI, logging, allocation, filesystem, delay, scan, or radio-control work. The main loop drains the accumulator every 250 ms, computes packets per second with unsigned wrap-safe timing, and maintains session-wide average and strongest RSSI. Counters reset for each start and remain visible after stop. Saturation increments a fixed overflow-event diagnostic instead of wrapping silently. No task, packet queue, payload buffer, or growing history is added. The linked ESP32-S3 image assigns 178 bytes to service/counter state, approximately 25 bytes to manager callback dispatch, and 120 bytes to the screen C++ instance, for about 323 bytes of fixed C++ RAM excluding bounded LVGL objects and the existing Wi-Fi stack.

Channel changes are permitted only while stopped. KEY short advances the channel with 13-to-1 wraparound and is ignored while running; KEY long toggles a compact fixed detail panel; BOOT short starts/stops; BOOT long performs full stop/release and returns exactly one level to the Wi-Fi menu. Automatic hopping, PCAP and microSD logging are explicitly outside this milestone.

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
