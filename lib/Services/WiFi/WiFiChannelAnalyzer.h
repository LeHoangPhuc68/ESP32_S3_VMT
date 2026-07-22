#pragma once

#include <cstdint>

#include "WiFiScanSnapshot.h"

class WiFiChannelAnalyzer final
{
public:
    static constexpr std::uint8_t FirstChannel = 1;
    static constexpr std::uint8_t LastChannel = 13;
    static constexpr std::uint8_t ChannelCount =
        LastChannel - FirstChannel + 1;

    static constexpr std::int16_t NoSignalRssi = -127;

    struct ChannelMetrics
    {
        std::uint16_t interferenceScore = 0;
        std::int16_t strongestRssi = NoSignalRssi;
        std::int16_t averageRssi = NoSignalRssi;
        std::uint8_t apCount = 0;
        bool recommended = false;
    };

    bool analyze(
        const WiFiScanSnapshot &snapshot);

    void clear();

    bool hasAnalysis() const;

    const ChannelMetrics *metrics(
        std::uint8_t channel) const;

    std::uint8_t recommendedChannel() const;

    std::uint32_t generation() const;

    std::uint32_t completedAtMs() const;

private:
    static std::uint8_t overlapWeight(
        std::uint8_t distance);

    static std::uint16_t signalImpact(
        std::int32_t rssi);

    ChannelMetrics channels_[ChannelCount];

    bool hasAnalysis_ = false;

    std::uint8_t recommendedChannel_ = 0;

    std::uint32_t generation_ = 0;

    std::uint32_t completedAtMs_ = 0;
};
