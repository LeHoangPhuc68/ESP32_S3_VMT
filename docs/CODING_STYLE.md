# Coding Style

## Language and toolchain

- Framework: Arduino
- Build system: PlatformIO
- C++ standard: GNU++17
- Target environment: `lilygo`
- Compiler warnings: `-Wall -Wextra`

## Files

- Use `.h` and `.cpp` pairs for non-trivial classes.
- Use `#pragma once` in headers.
- Keep one primary class per pair.
- File and class names use PascalCase.
- Functions and variables use camelCase.
- Compile-time constants use PascalCase or an existing local convention consistently.
- Private static members use the existing trailing underscore convention.

## Formatting

- Four spaces for indentation.
- Braces on their own lines, matching the current repository style.
- Keep line lengths readable on a normal editor window.
- Separate logical sections with blank lines, not decorative banners.
- Avoid formatting unrelated code in feature commits.

## Types

- Prefer `std::uint8_t`, `std::int16_t`, `std::uint32_t`, and similar fixed-width types for stored state.
- Prefer `enum class` with an explicit underlying type for compact state.
- Prefer `const char *` for static labels and `String` only where Arduino integration or dynamic text is genuinely useful.
- Avoid implicit narrowing conversions.
- Use `static_cast` for explicit conversions.

## Classes

- Mark non-inheritable classes `final`.
- Delete copy/move operations when an object owns a unique hardware resource and copying would be unsafe.
- Prefer static service classes only when there is intentionally one system-wide instance.
- Prefer ordinary instances where multiple independent objects may be useful.
- Keep constructors lightweight; perform fallible hardware work in `begin()`.

## Functions

- Use early returns for invalid state and errors.
- Keep update loops non-blocking.
- Do not perform expensive scans, file operations, or network work directly in UI event callbacks.
- Return meaningful success/failure values.
- Preserve driver error codes where practical.
- Do not claim success before an operation is complete.

## Memory

- Avoid dynamic allocation in frequently called paths.
- Bound scan result arrays and caches.
- Avoid repeated `String` concatenation inside loops.
- Store immutable labels in flash-compatible static storage when appropriate.
- Do not retain pointers to temporary BLE or Wi-Fi result objects.
- Check object lifetime when navigating between screens.

## Timing

Use unsigned elapsed-time checks:

```cpp
const std::uint32_t now = millis();

if (now - startedAt_ >= TimeoutMs)
{
    // timeout
}
```

Do not compare absolute future timestamps in a way that breaks on `millis()` rollover.

## Logging

Good:

```cpp
Serial.printf(
    "[WiFiScanner] Scan failed: %d\n",
    static_cast<int>(errorCode));
```

Avoid:

```cpp
Serial.println("error");
```

Logs should identify the subsystem, operation, and useful error value.

## Comments

Comments should explain:

- why a workaround exists;
- hardware or framework behavior;
- ownership and lifetime assumptions;
- non-obvious timing or state decisions.

Comments should not restate obvious code.

## Public APIs

- Document initialization requirements.
- Document whether returned pointers remain valid and for how long.
- Document whether a call is synchronous or asynchronous.
- Document state requirements and failure behavior.
- Update all callers in the same change when an API changes.

## UI

- Screen classes own their LVGL objects.
- Shared visual patterns belong in widgets or theme helpers.
- Screen input handlers request service operations; they do not block waiting for completion.
- Always define clear Back and Home behavior.
- Preserve the current input action vocabulary.

## Commits

Use Conventional Commits with a useful scope:

```text
feat(wifi): add scan ownership arbitration
fix(ble): retain selected device data safely
refactor(core): decouple app launch from UI manager
docs(architecture): define radio ownership
chore(ci): build firmware on pull requests
```
