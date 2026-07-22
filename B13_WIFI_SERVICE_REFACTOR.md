# B13 - WiFi service refactor

## Implemented

- Added `WiFiManager` as the only owner of the ESP32 Wi-Fi scan/radio API.
- Added active ownership identities for Scanner, Signal Monitor, and Packet Monitor.
- Implemented Packet Monitor as a passive, single-channel consumer of the manager-owned promiscuous receive mode.
- Implemented Channel Analyzer as a read-only consumer of the shared Scanner snapshot. Its existing owner identity remains reserved and unused because analyzer refreshes use the Scanner service ownership flow.
- Centralized async scan state, timeout, failure grace period, cancellation and result cleanup.
- Refactored `WiFiScannerService` to use `WiFiManager`.
- Cancels and releases an in-progress scanner operation when its screen is hidden.
- Scanner keeps the previous list visible while a new scan is running and replaces it only after success.
- Refactored `WiFiSignalMonitorService` to use channel-limited scans through `WiFiManager`.
- Signal Monitor no longer changes Wi-Fi mode or deletes scan data directly.
- Added the M2.2.1 shared completed-scan snapshot foundation.

## Radio ownership rule

Only `WiFiManager.cpp` may include the Arduino/ESP-IDF Wi-Fi driver headers or call Wi-Fi driver APIs, including:

- `WiFi.mode()`
- `WiFi.disconnect()`
- `WiFi.scanNetworks()`
- `WiFi.scanComplete()`
- `WiFi.scanDelete()`
- `esp_wifi_scan_stop()`
- `esp_wifi_set_channel()`
- `esp_wifi_set_promiscuous()`
- `esp_wifi_set_promiscuous_rx_cb()`
- `esp_wifi_set_promiscuous_filter()`

Ownership arbitration is cooperative and is currently expected to run from the main application loop. `WiFiManager` does not synchronize concurrent calls from multiple FreeRTOS tasks and is not thread-safe for that use. `acquire()` is idempotent when called again by the current owner, but ownership is not reference-counted: one matching `release()` relinquishes it.

## Shared completed scan snapshot

`WiFiScanEntry` is the canonical completed-scan record. Its SSID and BSSID are owned `String` values, and its RSSI, channel, authentication type and hidden flag are copied scalars. It does not retain driver-owned pointers, references or views.

`WiFiScannerService` owns one `WiFiScanSnapshot` with a fixed capacity of 16 entries. Consumers receive only a const snapshot reference or const entry pointers. An entry pointer remains valid until the next successful Scanner publication or an explicit `WiFiScannerService::clear()`.

Scanner publication is ordered as follows:

```text
WiFiManager reports Ready
  -> WiFiScannerService copies and sorts bounded results in staging storage
  -> Scanner releases WiFiManager ownership
  -> WiFiManager deletes the driver result buffer
  -> staged entries and snapshot metadata are published
```

Snapshot status is `Empty` or `Success`; current-attempt failures remain in `WiFiScannerService::State` and are not published over prior data. If ownership release fails, staging is discarded and the prior snapshot remains unchanged, including its entries, count, timestamp, status and generation. Generation starts at zero and increments once for every successful Scanner scan publication, including a successful scan with zero results. It is a 32-bit change counter that wraps naturally, so consumers compare it for change rather than ordering. Start, rejection, failure and cancellation do not change the published snapshot or generation, so the previous successful list remains readable. Explicit clear marks the snapshot empty while retaining the generation counter. `completedAtMs()` uses `millis()` as a wrap-safe age basis when consumers subtract it from the current time.

Logical publication relies on the cooperative main-loop contract: readers cannot run while the Scanner service is copying and committing a result. The snapshot does not add FreeRTOS synchronization and remains unsafe for concurrent access from multiple tasks.

Selection and Signal Monitor store copied `WiFiScanEntry` targets. Signal Monitor's channel-limited sampling scans remain transient manager operations and do not publish into the shared Scanner snapshot; publishing those scans could replace the previous full Scanner list, and truncating their result search could miss the target AP. Packet Monitor does not read or publish scan snapshots.

## Channel Analyzer snapshot consumer

Channel Analyzer does not access `WiFiManager`, Arduino Wi-Fi, or ESP-IDF APIs directly. It requests refreshes through `WiFiScannerService`, which acquires and releases the existing Scanner owner, then analyzes only the completed const `WiFiScanSnapshot`. The reserved Channel Analyzer owner identity is not acquired, avoiding a second owner for the same refresh.

The analyzer holds 13 fixed channel metrics and no scan-entry pointers or copied network names. For every snapshot AP on channels 1 through 13 it records exact-channel count, strongest RSSI and average RSSI. Its interference score adds a signal impact of `10 + clamp(RSSI + 100, 0, 70)` multiplied by overlap weights 5, 4, 3, 2 and 1 at channel distances 0, 1, 2, 3 and 4; APs five or more channels away do not contribute. With the 16-entry snapshot limit, the maximum score is 6400.

The lowest score is the baseline recommendation. The best of channels 1, 6 and 11 replaces it only when within `max(20, 15% of the baseline score)`, providing a deterministic preference for common non-overlapping 20 MHz choices. Ties are deterministic. A successful zero-result scan recommends channel 1 as a no-evidence default. Results are advisory: the bounded snapshot cannot represent authoritative AP totals, channel utilization, channel width or all regional constraints.

An unchanged generation is not reanalyzed. A successful new generation, including zero results, replaces the metrics once. Cached analysis remains visible during refresh and after failure. If no successful snapshot exists on entry, the screen starts a Scanner scan. BOOT short refreshes; leaving or BOOT long cancels through `WiFiScannerService` and returns to the Wi-Fi menu. KEY short selects channels 1 through 13 with wraparound and KEY long toggles the fixed detail view.

The analysis model uses approximately 116 bytes of persistent C++ state plus 52 bytes of temporary stack during a rebuild. It adds no task, mutex, unbounded container, retained driver reference, persistence, or background scanner.

## Passive Packet Monitor

Packet Monitor acquires the dedicated `WiFiManager::Owner::PacketMonitor` identity before the manager prepares station mode, selects one channel from 1 through 13, installs its receive thunk, and enables promiscuous receive mode. Stop and every screen-exit path first neutralize callback dispatch, disable promiscuous mode, restore the manager's idle radio state, and request ownership release. Partial start and stop failures use the same bounded cleanup path so Scanner, Signal Monitor, and Channel Analyzer can subsequently use the radio. If both normal cleanup and manager-owned power-off recovery fail, the manager retains failed ownership rather than allowing another feature to use an uncertain radio state.

The receive path is passive-only. It never transmits raw 802.11 frames and implements no injection, deauthentication transmission, beacon/probe spam, credential capture, replay, jamming, or attack automation. It classifies only the frame-control type and selected management subtypes: association request/response, probe request/response, beacon, disassociation, and deauthentication. Control and data frames are counted by type. Frames shorter than the two-byte frame-control field or carrying invalid receive metadata are rejected as malformed. Packet payloads are not retained, displayed, parsed as application data, or written to storage.

The manager-owned callback thunk forwards one transient, non-owning frame view to `PacketMonitor`. The feature callback validates and classifies the frame, then updates a fixed POD accumulator under a short ESP32 critical section. It performs no LVGL, navigation, logging, `String` construction, allocation, filesystem access, delay, scan, or radio configuration. The main loop consumes and resets the accumulator at a bounded 250 ms interval; packets per second are derived from that interval with wrap-safe unsigned `millis()` subtraction. Average RSSI is session-wide, and all session counters reset on each successful start while final values remain visible after stop. Saturation is counted in the overflow diagnostic rather than wrapping silently.

Channel selection is fixed for a capture session; there is no automatic hopping. While stopped, KEY short advances channels 1 through 13 with wraparound. KEY short is ignored while running, KEY long toggles the fixed detail view, BOOT short starts or stops capture, and BOOT long performs full cleanup before returning one level to the Wi-Fi menu. The implementation uses fixed counters only and adds no worker task, unbounded queue, packet history, or per-packet allocation. The linked ESP32-S3 image reports 178 bytes of Packet Monitor service/counter state, about 25 bytes of manager callback-dispatch state, and a 120-byte screen C++ instance (approximately 323 bytes of fixed C++ RAM total, excluding bounded LVGL object allocations and the framework Wi-Fi stack). Automatic channel hopping, PCAP capture, microSD logging, and external radio modules are outside this milestone.

## Input mapping: Wi-Fi Scanner

- KEY short / `Next`: move selection to the next AP
- KEY long / `Select`: open the focused AP detail
- BOOT short / `Primary`: start a manual rescan and remain on the Scanner
- BOOT long / `Back`: cancel any active scan, release ownership, and return to the Wi-Fi menu
