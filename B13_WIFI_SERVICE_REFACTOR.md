# B13 - WiFi service refactor

## Implemented

- Added `WiFiManager` as the only owner of the ESP32 Wi-Fi scan/radio API.
- Added scan ownership for Scanner, Signal Monitor, Packet Monitor and Channel Analyzer.
- Centralized async scan state, timeout, failure grace period, cancellation and result cleanup.
- Refactored `WiFiScannerService` to use `WiFiManager`.
- Restored manual rescan on the Wi-Fi Scanner screen:
  - Back: rescan
  - Home: exit
- Scanner keeps the previous list visible while a new scan is running and replaces it only after success.
- Refactored `WiFiSignalMonitorService` to use channel-limited scans through `WiFiManager`.
- Signal Monitor no longer changes Wi-Fi mode or deletes scan data directly.

## Radio ownership rule

Only `WiFiManager.cpp` may call:

- `WiFi.mode()`
- `WiFi.disconnect()`
- `WiFi.scanNetworks()`
- `WiFi.scanComplete()`
- `WiFi.scanDelete()`
- `esp_wifi_scan_stop()`

## Input mapping: Wi-Fi Scanner

- Previous / Next: move selection
- Select: open AP detail
- Back: rescan
- Home: return home
