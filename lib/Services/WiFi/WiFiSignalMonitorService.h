#pragma once

#include <Arduino.h>
#include <cstdint>

#include "WiFiScannerService.h"

class WiFiSignalMonitorService final
{
public:
    static constexpr std::uint8_t HistorySize = 32;

    enum class State : std::uint8_t
    {
        Idle = 0,
        Preparing,
        Scanning,
        Ready,
        NotFound,
        Failed
    };

    enum class Trend : std::uint8_t
    {
        Unknown = 0,
        Stable,
        Stronger,
        Weaker
    };

    static bool start(
        const WiFiScannerService::Network &target);

    static void stop();

    static void update();

    static void requestImmediateSample();

    static bool isRunning();

    static State state();

    static const char *stateText();

    static const WiFiScannerService::Network *target();

    static bool hasSample();

    static std::int32_t currentRssi();

    static std::int32_t minimumRssi();

    static std::int32_t maximumRssi();

    static std::int32_t averageRssi();

    static std::int32_t rangeDb();

    static std::uint8_t signalQuality();

    static const char *qualityText();

    static Trend trend();

    static const char *trendText();

    static std::uint8_t sampleCount();

    /*
     * index = 0 là mẫu cũ nhất.
     */
    static std::int32_t sample(
        std::uint8_t index);

    static std::uint32_t successfulScans();

    static std::uint32_t missedScans();

    static std::int16_t lastScanCode();

private:
    static constexpr std::uint32_t PrepareDelayMs =
        150;

    static constexpr std::uint32_t SampleIntervalMs =
        1500;

    static constexpr std::uint32_t ScanTimeoutMs =
        10000;

    static constexpr std::uint32_t RetryDelayMs =
        1000;

    static bool prepareRadio();

    static bool launchScan();

    static void processResults(
        std::int16_t resultCount);

    static void addSample(
        std::int32_t rssi);

    static void recalculateStatistics();

    static bool bssidMatches(
        const String &left,
        const String &right);

    static bool running_;

    static State state_;

    static WiFiScannerService::Network target_;

    static std::int32_t history_[HistorySize];

    static std::uint8_t historyCount_;

    static std::uint8_t historyWriteIndex_;

    static std::int32_t currentRssi_;

    static std::int32_t minimumRssi_;

    static std::int32_t maximumRssi_;

    static std::int32_t averageRssi_;

    static std::uint32_t stateStartedAt_;

    static std::uint32_t nextScanAt_;

    static std::uint32_t successfulScans_;

    static std::uint32_t missedScans_;

    static std::int16_t lastScanCode_;
};