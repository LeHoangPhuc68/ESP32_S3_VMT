#pragma once

#include <Arduino.h>
#include <cstdint>

class WiFiManager final
{
public:
    enum class Owner : std::uint8_t
    {
        None = 0,
        Scanner,
        SignalMonitor,
        PacketMonitor,
        ChannelAnalyzer
    };

    enum class State : std::uint8_t
    {
        Uninitialized = 0,
        Idle,
        Preparing,
        Scanning,
        Ready,
        Failed
    };

    static bool begin();
    static void update();

    static bool requestScan(
        Owner owner,
        std::uint8_t channel = 0,
        bool showHidden = true,
        std::uint16_t passiveDwellMs = 300);

    static bool cancel(Owner owner);
    static bool release(Owner owner);
    static void reset();

    static bool isInitialized();
    static bool isBusy();
    static bool isOwnedBy(Owner owner);

    static Owner owner();
    static State state();
    static const char *stateText();
    static std::int16_t lastScanCode();
    static std::int16_t resultCount();

    static String ssid(std::int16_t index);
    static String bssid(std::int16_t index);
    static std::int32_t rssi(std::int16_t index);
    static std::int32_t channel(std::int16_t index);
    static std::uint8_t encryptionType(std::int16_t index);

private:
    static constexpr std::uint32_t PrepareDelayMs = 180;
    static constexpr std::uint32_t ScanTimeoutMs = 15000;
    static constexpr std::uint32_t FailureGraceMs = 1200;

    static bool prepareRadio();
    static bool launchScan();
    static void finish(std::int16_t resultCount);
    static void fail(std::int16_t errorCode);
    static void clearDriverResults();

    static bool initialized_;
    static Owner owner_;
    static State state_;

    static std::uint8_t requestedChannel_;
    static bool showHidden_;
    static std::uint16_t passiveDwellMs_;

    static std::uint32_t stateStartedAt_;
    static std::uint32_t firstFailureAt_;

    static std::int16_t lastScanCode_;
    static std::int16_t resultCount_;
};
