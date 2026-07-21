#pragma once

#include <Arduino.h>
#include <cstdint>

class WiFiScannerService final
{
public:
    static constexpr std::uint8_t MaxNetworks = 16;

    enum class State : std::uint8_t
    {
        Idle = 0,
        Preparing,
        Scanning,
        Complete,
        Failed
    };

    struct Network
    {
        String ssid;
        String bssid;
        std::int32_t rssi = 0;
        std::int32_t channel = 0;
        std::uint8_t encryptionType = 0;
        bool hidden = false;
    };

    static bool begin();
    static bool startScan();
    static void update();
    static void clear();

    static State state();
    static bool isScanning();
    static std::uint8_t networkCount();
    static const Network *network(std::uint8_t index);
    static const char *stateText();
    static const char *securityText(std::uint8_t encryptionType);
    static std::int16_t lastScanCode();

private:
    static void collectResults(std::int16_t resultCount);
    static void sortBySignalStrength();
    static void clearNetworks();

    static bool initialized_;
    static State state_;
    static Network networks_[MaxNetworks];
    static std::uint8_t networkCount_;
    static std::int16_t lastScanCode_;
};
