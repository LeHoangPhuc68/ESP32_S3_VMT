#pragma once

#include <Arduino.h>
#include <cstdint>

struct WiFiScanEntry
{
    String ssid;
    String bssid;

    std::int32_t rssi = 0;
    std::int32_t channel = 0;

    std::uint8_t encryptionType = 0;

    bool hidden = false;
};

class WiFiScannerService;

class WiFiScanSnapshot final
{
public:
    static constexpr std::uint8_t Capacity = 16;

    enum class Status : std::uint8_t
    {
        Empty = 0,
        Success
    };

    Status status() const;

    bool hasSuccessfulScan() const;

    std::uint8_t count() const;

    std::uint32_t generation() const;

    std::uint32_t completedAtMs() const;

    const WiFiScanEntry *entry(
        std::uint8_t index) const;

private:
    friend class WiFiScannerService;

    WiFiScanEntry &entryForPublication(
        std::uint8_t index);

    void publish(
        std::uint8_t count,
        std::uint32_t completedAtMs);

    void clear();

    Status status_ = Status::Empty;

    std::uint8_t count_ = 0;

    std::uint32_t generation_ = 0;

    std::uint32_t completedAtMs_ = 0;

    WiFiScanEntry entries_[Capacity];
};
