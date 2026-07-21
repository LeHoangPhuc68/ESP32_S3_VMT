# AGENTS.md

## Project identity

VMT is a modular firmware project for the LilyGO T-Display S3 (ESP32-S3). It uses the Arduino framework, PlatformIO, LVGL 9, Arduino GFX, ArduinoJson, and LittleFS.

The repository must remain buildable with:

```bash
pio run -e lilygo
```

## Mandatory workflow

Before changing code:

1. Read this file.
2. Read `docs/ARCHITECTURE.md`.
3. Read the files directly involved in the requested task.
4. Inspect call sites before changing any public interface.
5. Check `docs/ROADMAP.md` to avoid conflicting with planned architecture.

After changing code:

1. Run `pio run -e lilygo`.
2. Fix all compile errors introduced by the change.
3. Review the diff for unrelated edits.
4. Summarize changed files, behavior, remaining risks, and hardware tests required.
5. Do not commit or push unless explicitly requested.

## Non-negotiable rules

- Preserve existing working features unless the task explicitly replaces them.
- Do not invent hardware capabilities that are not present in `docs/HARDWARE.md`.
- Do not add offensive Wi-Fi or Bluetooth features such as deauthentication, beacon spam, credential capture, or unauthorized interference.
- Do not introduce placeholder implementations, fake success paths, dead menu entries, or empty screens.
- Do not silently swallow errors. Log failures with enough context for serial debugging.
- Do not add secrets, Wi-Fi passwords, access tokens, private keys, or personal credentials to the repository.
- Do not edit generated files under `.pio/`.
- Keep changes scoped to the requested task.
- Avoid broad rewrites unless the task explicitly requires a refactor.
- Prefer deterministic, non-blocking state machines over long delays in runtime code.
- Avoid heap churn in update loops and rendering paths.
- Respect ESP32-S3 memory limits.
- Public API changes require updating all call sites and documentation in the same change.

## Architecture boundaries

The current top-level responsibilities are:

- `src/`: application entry point and top-level boot flow.
- `lib/Core/`: application catalog, app lifecycle, events, logging, shared state, and menus.
- `lib/Display/`: display hardware abstraction.
- `lib/Input/`: input translation into semantic actions.
- `lib/Services/`: long-lived feature logic such as Wi-Fi and BLE.
- `lib/Modules/`: optional external hardware modules.
- `lib/UI/`: navigation, screens, widgets, theme, and rendering.
- `lib/Assets/`: fonts, icons, images, and generated image data.
- `include/`: board configuration, pins, LVGL configuration, version, and secret templates.
- `data/`: LittleFS runtime configuration assets.

Dependency direction:

```text
src -> Core / Display / Input / UI
UI -> Core / Services / Assets / Input
Services -> Core and platform APIs
Modules -> module interfaces and device drivers
Core -> no dependency on UI
```

Additional rules:

- Core must not include or depend directly on UI classes.
- Services must not render LVGL objects or navigate screens.
- UI screens may call service APIs but must not directly own radio drivers.
- Input code reports semantic actions; screens decide how actions are interpreted.
- Hardware-specific ESP-IDF or Arduino driver calls should be centralized behind the appropriate manager or service.
- Only one component may own a shared radio resource at a time.

## Wi-Fi rules

Current Wi-Fi code is under `lib/Services/WiFi/`.

Target architecture:

- `WiFiManager` owns radio mode, scan lifecycle, channel control, promiscuous mode, cancellation, and recovery.
- Scanner, signal monitor, packet monitor, and future analyzers request radio work from `WiFiManager`.
- UI reads service state and results; UI does not call `WiFi`, `esp_wifi_*`, or low-level scan APIs directly.
- Shared radio state must prevent overlapping operations.

Until `WiFiManager` is fully introduced, do not increase direct Wi-Fi driver usage in additional files.

## BLE rules

Current BLE code is under `lib/Services/BLE/`.

- BLE scanning must remain passive.
- Keep scan lifecycle and device cache outside UI screens.
- Avoid blocking scans from the UI thread.
- Device detail screens must use copied/stable data, not dangling pointers into temporary scan results.
- Any BLE API choice must match the installed Arduino-ESP32 framework version.

## UI rules

- All screens derive from `BaseScreen`.
- Screen transitions go through the existing navigation/UI management layer.
- Reusable visual elements belong in `lib/UI/Widgets/`.
- Reusable colors, fonts, and styles belong in `lib/UI/Theme/`.
- Do not duplicate layout constants across multiple screens when a shared constant is appropriate.
- Avoid doing radio scans, file I/O, or other long operations inside LVGL callbacks.
- Update methods must be fast and non-blocking.

## C++ style

- Language standard: GNU++17.
- Use `#pragma once` in headers.
- One primary class per header/source pair.
- Prefer `enum class` over unscoped enums.
- Use fixed-width integer types for stored state and protocol-facing values.
- Mark classes `final` when inheritance is not intended.
- Mark single-argument constructors `explicit`.
- Use `const` and `constexpr` wherever appropriate.
- Avoid macros for typed constants.
- Avoid owning raw pointers. If a raw pointer is required, clearly document ownership and lifetime.
- Keep functions focused and small enough to review.
- Use early returns for invalid state and failures.
- Do not use exceptions or RTTI unless the project configuration is intentionally changed.
- Do not reformat unrelated files.

## Logging

- Use the existing logging approach when possible.
- Prefix service logs consistently, for example `[WiFiScanner]` or `[BLEScanner]`.
- Log state transitions, start/stop events, timeout conditions, and driver error codes.
- Avoid flooding Serial from every update loop iteration.

## Testing and validation

Minimum software validation:

```bash
pio run -e lilygo
```

When relevant, also perform:

```bash
pio run -e lilygo -t clean
pio run -e lilygo
```

Hardware-dependent work must end with a test checklist. The agent cannot claim hardware behavior is verified unless it was actually tested on a LilyGO T-Display S3.

Required hardware test categories when applicable:

- boot and display initialization
- button navigation
- screen enter/exit behavior
- repeated scan/rescan
- return navigation
- radio operation after switching between features
- memory stability after repeated use
- serial log review

## Git rules

- Never force-push.
- Never rewrite published history.
- Never commit generated `.pio/` output.
- Never push without explicit user approval.
- Prefer one logical change per commit.
- Use Conventional Commits:

```text
feat(wifi): add centralized radio manager
fix(ui): restore scanner return navigation
refactor(ble): separate scan cache from screen state
docs(core): document service ownership rules
chore(ci): add PlatformIO build workflow
```

## Definition of done

A task is complete only when:

- requested behavior is implemented;
- affected public APIs and call sites are consistent;
- the project builds with `pio run -e lilygo`;
- no unrelated regressions are visible in the diff;
- documentation is updated when architecture or behavior changed;
- hardware-only validation is clearly listed instead of falsely claimed.
