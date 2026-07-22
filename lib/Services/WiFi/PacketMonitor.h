#pragma once

#include <cstdint>

#include "WiFiManager.h"

class PacketMonitor final
{
public:
    static constexpr std::uint8_t FirstChannel = 1;
    static constexpr std::uint8_t LastChannel = 13;
    static constexpr std::int16_t NoSignalRssi = -127;

    enum class State : std::uint8_t
    {
        Stopped = 0,
        Starting,
        Running,
        Stopping,
        Error
    };

    struct Statistics
    {
        std::uint32_t packetsPerSecond = 0;
        std::uint32_t totalPackets = 0;
        std::uint32_t managementPackets = 0;
        std::uint32_t controlPackets = 0;
        std::uint32_t dataPackets = 0;
        std::uint32_t beaconFrames = 0;
        std::uint32_t probeRequestFrames = 0;
        std::uint32_t probeResponseFrames = 0;
        std::uint32_t associationRequestFrames = 0;
        std::uint32_t associationResponseFrames = 0;
        std::uint32_t deauthenticationFrames = 0;
        std::uint32_t disassociationFrames = 0;
        std::uint32_t malformedPackets = 0;
        std::uint32_t overflowPackets = 0;
        std::uint32_t intervalMs = 0;
        std::uint32_t elapsedMs = 0;
        std::int16_t averageRssi = NoSignalRssi;
        std::int16_t strongestRssi = NoSignalRssi;
    };

    static bool begin();
    static void update();

    static bool start();
    static bool stop();

    static bool setChannel(std::uint8_t channel);
    static std::uint8_t channel();

    static State state();
    static const char *stateText();
    static const Statistics &statistics();
    static std::int16_t lastErrorCode();

private:
    struct CallbackStatistics
    {
        std::uint32_t totalPackets = 0;
        std::uint32_t managementPackets = 0;
        std::uint32_t controlPackets = 0;
        std::uint32_t dataPackets = 0;
        std::uint32_t beaconFrames = 0;
        std::uint32_t probeRequestFrames = 0;
        std::uint32_t probeResponseFrames = 0;
        std::uint32_t associationRequestFrames = 0;
        std::uint32_t associationResponseFrames = 0;
        std::uint32_t deauthenticationFrames = 0;
        std::uint32_t disassociationFrames = 0;
        std::uint32_t malformedPackets = 0;
        std::uint32_t overflowPackets = 0;
        std::int64_t rssiSum = 0;
        std::uint32_t rssiSamples = 0;
        std::int16_t strongestRssi = NoSignalRssi;
    };

    static constexpr std::uint32_t UpdateIntervalMs = 250;

    static void handleFrame(
        void *context,
        const WiFiManager::FrameView &frame);

    static void resetSession();
    static void setAcceptingFrames(bool accepting);
    static void consumeCallbackStatistics(std::uint32_t now);
    static void addCounter(
        std::uint32_t &target,
        std::uint32_t increment);
    static void recordOverflow(std::uint32_t increment = 1);
    static void fail(std::int16_t errorCode, const char *context);

    static bool initialized_;
    static bool acceptingFrames_;
    static State state_;
    static std::uint8_t channel_;
    static std::int16_t lastErrorCode_;
    static Statistics statistics_;
    static CallbackStatistics callbackStatistics_;
    static std::int64_t sessionRssiSum_;
    static std::uint64_t sessionRssiSamples_;
    static std::uint32_t sessionStartedAt_;
    static std::uint32_t lastUpdateAt_;
};
