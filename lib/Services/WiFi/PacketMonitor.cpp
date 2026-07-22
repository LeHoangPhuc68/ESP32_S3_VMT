#include "PacketMonitor.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <limits>

namespace
{
    portMUX_TYPE statisticsMux =
        portMUX_INITIALIZER_UNLOCKED;

    constexpr std::uint16_t FrameTypeMask = 0x000C;
    constexpr std::uint8_t FrameTypeShift = 2;
    constexpr std::uint16_t FrameSubtypeMask = 0x00F0;
    constexpr std::uint8_t FrameSubtypeShift = 4;
    constexpr std::uint16_t ProtocolVersionMask = 0x0003;

    constexpr std::uint8_t ManagementType = 0;
    constexpr std::uint8_t ControlType = 1;
    constexpr std::uint8_t DataType = 2;

    constexpr std::uint8_t AssociationRequestSubtype = 0;
    constexpr std::uint8_t AssociationResponseSubtype = 1;
    constexpr std::uint8_t ProbeRequestSubtype = 4;
    constexpr std::uint8_t ProbeResponseSubtype = 5;
    constexpr std::uint8_t BeaconSubtype = 8;
    constexpr std::uint8_t DisassociationSubtype = 10;
    constexpr std::uint8_t DeauthenticationSubtype = 12;

    constexpr std::int16_t InitializationError = -300;
    constexpr std::int16_t OwnershipError = -301;
    constexpr std::int16_t InvalidChannelError = -302;
    constexpr std::int16_t StartError = -303;
    constexpr std::int16_t StopError = -304;
    constexpr std::int16_t ReleaseError = -305;

    void incrementCallbackCounter(
        std::uint32_t &counter,
        std::uint32_t &overflowCounter)
    {
        if (counter < UINT32_MAX)
        {
            ++counter;
            return;
        }

        if (overflowCounter < UINT32_MAX)
        {
            ++overflowCounter;
        }
    }
}

bool PacketMonitor::initialized_ = false;
bool PacketMonitor::acceptingFrames_ = false;
PacketMonitor::State PacketMonitor::state_ =
    PacketMonitor::State::Stopped;
std::uint8_t PacketMonitor::channel_ =
    PacketMonitor::FirstChannel;
std::int16_t PacketMonitor::lastErrorCode_ = 0;
PacketMonitor::Statistics PacketMonitor::statistics_;
PacketMonitor::CallbackStatistics
    PacketMonitor::callbackStatistics_;
std::int64_t PacketMonitor::sessionRssiSum_ = 0;
std::uint64_t PacketMonitor::sessionRssiSamples_ = 0;
std::uint32_t PacketMonitor::sessionStartedAt_ = 0;
std::uint32_t PacketMonitor::lastUpdateAt_ = 0;

bool PacketMonitor::begin()
{
    if (initialized_)
    {
        return true;
    }

    if (!WiFiManager::begin())
    {
        fail(InitializationError, "manager initialization");
        return false;
    }

    initialized_ = true;
    state_ = State::Stopped;
    channel_ = FirstChannel;
    lastErrorCode_ = 0;
    resetSession();

    Serial.println("[PacketMonitor] Ready");
    return true;
}

void PacketMonitor::update()
{
    if (state_ != State::Running)
    {
        return;
    }

    constexpr WiFiManager::Owner Owner =
        WiFiManager::Owner::PacketMonitor;

    if (
        !WiFiManager::isOwnedBy(Owner) ||
        WiFiManager::state() !=
            WiFiManager::State::Monitoring)
    {
        const bool ownerStillHeld =
            WiFiManager::isOwnedBy(Owner);

        setAcceptingFrames(false);

        if (ownerStillHeld)
        {
            stop();
        }
        else
        {
            consumeCallbackStatistics(millis());
        }

        fail(OwnershipError, "running ownership lost");
        return;
    }

    const std::uint32_t now = millis();

    if (now - lastUpdateAt_ < UpdateIntervalMs)
    {
        return;
    }

    consumeCallbackStatistics(now);
}

bool PacketMonitor::start()
{
    constexpr WiFiManager::Owner Owner =
        WiFiManager::Owner::PacketMonitor;

    if (state_ == State::Running)
    {
        if (
            WiFiManager::isOwnedBy(Owner) &&
            WiFiManager::state() ==
                WiFiManager::State::Monitoring)
        {
            return true;
        }

        setAcceptingFrames(false);
        fail(OwnershipError, "running ownership lost");
        return false;
    }

    if (state_ == State::Starting)
    {
        return true;
    }

    if (state_ == State::Stopping)
    {
        return false;
    }

    if (!initialized_ && !begin())
    {
        return false;
    }

    if (
        channel_ < FirstChannel ||
        channel_ > LastChannel)
    {
        fail(InvalidChannelError, "invalid channel");
        return false;
    }

    state_ = State::Starting;
    lastErrorCode_ = 0;
    resetSession();

    if (!WiFiManager::acquire(Owner))
    {
        fail(OwnershipError, "radio ownership unavailable");
        return false;
    }

    const std::uint32_t startedAt = millis();
    sessionStartedAt_ = startedAt;
    lastUpdateAt_ = startedAt;
    setAcceptingFrames(true);

    if (!WiFiManager::startPromiscuous(
            Owner,
            channel_,
            &PacketMonitor::handleFrame,
            nullptr))
    {
        const std::int16_t managerError =
            WiFiManager::lastRadioCode();

        setAcceptingFrames(false);

        const bool stopped =
            WiFiManager::stopPromiscuous(Owner);

        const bool released =
            WiFiManager::release(Owner);

        if (!stopped || !released)
        {
            WiFiManager::reset();
        }

        fail(
            managerError != 0
                ? managerError
                : StartError,
            "capture start");
        return false;
    }

    state_ = State::Running;

    Serial.printf(
        "[PacketMonitor] Capture started: channel=%u\n",
        static_cast<unsigned int>(channel_));
    return true;
}

bool PacketMonitor::stop()
{
    constexpr WiFiManager::Owner Owner =
        WiFiManager::Owner::PacketMonitor;

    setAcceptingFrames(false);

    if (
        state_ == State::Stopped &&
        !WiFiManager::isOwnedBy(Owner))
    {
        return true;
    }

    if (
        state_ == State::Error &&
        !WiFiManager::isOwnedBy(Owner))
    {
        return false;
    }

    const bool operationExpected =
        state_ == State::Running ||
        state_ == State::Starting ||
        state_ == State::Stopping;

    const bool ownershipHeld =
        WiFiManager::isOwnedBy(Owner);

    state_ = State::Stopping;

    bool stopped =
        !operationExpected || ownershipHeld;

    if (ownershipHeld)
    {
        stopped =
            WiFiManager::stopPromiscuous(Owner);
    }

    const std::uint32_t now = millis();
    consumeCallbackStatistics(now);
    statistics_.packetsPerSecond = 0;
    statistics_.elapsedMs =
        now - sessionStartedAt_;

    bool released = true;

    if (WiFiManager::isOwnedBy(Owner))
    {
        released =
            WiFiManager::release(Owner);
    }

    std::int16_t managerError =
        WiFiManager::lastRadioCode();

    bool recovered = true;

    if (!released)
    {
        WiFiManager::reset();
        recovered =
            !WiFiManager::isOwnedBy(Owner);
    }

    if (!stopped || !released || !recovered)
    {
        if (managerError == 0)
        {
            managerError =
                WiFiManager::lastRadioCode();
        }

        fail(
            managerError != 0
                ? managerError
                : (!released
                       ? ReleaseError
                       : StopError),
            "capture stop");
        return false;
    }

    state_ = State::Stopped;
    lastErrorCode_ = 0;

    Serial.println("[PacketMonitor] Capture stopped");
    return true;
}

bool PacketMonitor::setChannel(
    const std::uint8_t channel)
{
    if (
        channel < FirstChannel ||
        channel > LastChannel)
    {
        lastErrorCode_ = InvalidChannelError;
        Serial.printf(
            "[PacketMonitor] Invalid channel: %u\n",
            static_cast<unsigned int>(channel));
        return false;
    }

    if (state_ != State::Stopped)
    {
        Serial.printf(
            "[PacketMonitor] Channel change rejected: state=%s\n",
            stateText());
        return false;
    }

    channel_ = channel;
    lastErrorCode_ = 0;
    return true;
}

std::uint8_t PacketMonitor::channel()
{
    return channel_;
}

PacketMonitor::State PacketMonitor::state()
{
    return state_;
}

const char *PacketMonitor::stateText()
{
    switch (state_)
    {
    case State::Stopped:
        return "STOPPED";

    case State::Starting:
        return "STARTING";

    case State::Running:
        return "RUNNING";

    case State::Stopping:
        return "STOPPING";

    case State::Error:
        return "ERROR";

    default:
        return "UNKNOWN";
    }
}

const PacketMonitor::Statistics &
PacketMonitor::statistics()
{
    return statistics_;
}

std::int16_t PacketMonitor::lastErrorCode()
{
    return lastErrorCode_;
}

void PacketMonitor::handleFrame(
    void *context,
    const WiFiManager::FrameView &frame)
{
    (void)context;

    if (frame.type == WiFiManager::FrameType::Misc)
    {
        return;
    }

    bool malformed =
        frame.payload == nullptr ||
        frame.length < 2 ||
        frame.rxState != 0;

    std::uint8_t parsedType = 0;
    std::uint8_t parsedSubtype = 0;

    if (!malformed)
    {
        const std::uint16_t frameControl =
            static_cast<std::uint16_t>(
                frame.payload[0]) |
            static_cast<std::uint16_t>(
                frame.payload[1])
                << 8;

        malformed =
            (frameControl & ProtocolVersionMask) != 0;

        parsedType = static_cast<std::uint8_t>(
            (frameControl & FrameTypeMask) >>
            FrameTypeShift);

        parsedSubtype = static_cast<std::uint8_t>(
            (frameControl & FrameSubtypeMask) >>
            FrameSubtypeShift);

        switch (parsedType)
        {
        case ManagementType:
            malformed = malformed ||
                frame.type !=
                WiFiManager::FrameType::Management;
            break;

        case ControlType:
            malformed = malformed ||
                frame.type !=
                WiFiManager::FrameType::Control;
            break;

        case DataType:
            malformed = malformed ||
                frame.type !=
                WiFiManager::FrameType::Data;
            break;

        default:
            malformed = true;
            break;
        }
    }

    portENTER_CRITICAL(&statisticsMux);

    if (!acceptingFrames_)
    {
        portEXIT_CRITICAL(&statisticsMux);
        return;
    }

    if (malformed)
    {
        incrementCallbackCounter(
            callbackStatistics_.malformedPackets,
            callbackStatistics_.overflowPackets);

        portEXIT_CRITICAL(&statisticsMux);
        return;
    }

    incrementCallbackCounter(
        callbackStatistics_.totalPackets,
        callbackStatistics_.overflowPackets);

    switch (parsedType)
    {
    case ManagementType:
        incrementCallbackCounter(
            callbackStatistics_.managementPackets,
            callbackStatistics_.overflowPackets);

        switch (parsedSubtype)
        {
        case AssociationRequestSubtype:
            incrementCallbackCounter(
                callbackStatistics_.associationRequestFrames,
                callbackStatistics_.overflowPackets);
            break;

        case AssociationResponseSubtype:
            incrementCallbackCounter(
                callbackStatistics_.associationResponseFrames,
                callbackStatistics_.overflowPackets);
            break;

        case ProbeRequestSubtype:
            incrementCallbackCounter(
                callbackStatistics_.probeRequestFrames,
                callbackStatistics_.overflowPackets);
            break;

        case ProbeResponseSubtype:
            incrementCallbackCounter(
                callbackStatistics_.probeResponseFrames,
                callbackStatistics_.overflowPackets);
            break;

        case BeaconSubtype:
            incrementCallbackCounter(
                callbackStatistics_.beaconFrames,
                callbackStatistics_.overflowPackets);
            break;

        case DisassociationSubtype:
            incrementCallbackCounter(
                callbackStatistics_.disassociationFrames,
                callbackStatistics_.overflowPackets);
            break;

        case DeauthenticationSubtype:
            incrementCallbackCounter(
                callbackStatistics_.deauthenticationFrames,
                callbackStatistics_.overflowPackets);
            break;

        default:
            break;
        }
        break;

    case ControlType:
        incrementCallbackCounter(
            callbackStatistics_.controlPackets,
            callbackStatistics_.overflowPackets);
        break;

    case DataType:
        incrementCallbackCounter(
            callbackStatistics_.dataPackets,
            callbackStatistics_.overflowPackets);
        break;

    default:
        break;
    }

    if (callbackStatistics_.rssiSamples < UINT32_MAX)
    {
        if (
            callbackStatistics_.rssiSamples == 0 ||
            frame.rssi >
                callbackStatistics_.strongestRssi)
        {
            callbackStatistics_.strongestRssi =
                frame.rssi;
        }

        callbackStatistics_.rssiSum += frame.rssi;
        ++callbackStatistics_.rssiSamples;
    }
    else
    {
        incrementCallbackCounter(
            callbackStatistics_.overflowPackets,
            callbackStatistics_.overflowPackets);
    }

    portEXIT_CRITICAL(&statisticsMux);
}

void PacketMonitor::resetSession()
{
    setAcceptingFrames(false);

    portENTER_CRITICAL(&statisticsMux);
    callbackStatistics_ = CallbackStatistics();
    portEXIT_CRITICAL(&statisticsMux);

    statistics_ = Statistics();
    sessionRssiSum_ = 0;
    sessionRssiSamples_ = 0;

    const std::uint32_t now = millis();
    sessionStartedAt_ = now;
    lastUpdateAt_ = now;
}

void PacketMonitor::setAcceptingFrames(
    const bool accepting)
{
    portENTER_CRITICAL(&statisticsMux);
    acceptingFrames_ = accepting;
    portEXIT_CRITICAL(&statisticsMux);
}

void PacketMonitor::consumeCallbackStatistics(
    const std::uint32_t now)
{
    CallbackStatistics pending;

    portENTER_CRITICAL(&statisticsMux);
    pending = callbackStatistics_;
    callbackStatistics_ = CallbackStatistics();
    portEXIT_CRITICAL(&statisticsMux);

    const std::uint32_t interval =
        now - lastUpdateAt_;

    addCounter(
        statistics_.totalPackets,
        pending.totalPackets);
    addCounter(
        statistics_.managementPackets,
        pending.managementPackets);
    addCounter(
        statistics_.controlPackets,
        pending.controlPackets);
    addCounter(
        statistics_.dataPackets,
        pending.dataPackets);
    addCounter(
        statistics_.beaconFrames,
        pending.beaconFrames);
    addCounter(
        statistics_.probeRequestFrames,
        pending.probeRequestFrames);
    addCounter(
        statistics_.probeResponseFrames,
        pending.probeResponseFrames);
    addCounter(
        statistics_.associationRequestFrames,
        pending.associationRequestFrames);
    addCounter(
        statistics_.associationResponseFrames,
        pending.associationResponseFrames);
    addCounter(
        statistics_.deauthenticationFrames,
        pending.deauthenticationFrames);
    addCounter(
        statistics_.disassociationFrames,
        pending.disassociationFrames);
    addCounter(
        statistics_.malformedPackets,
        pending.malformedPackets);
    addCounter(
        statistics_.overflowPackets,
        pending.overflowPackets);

    if (pending.rssiSamples > 0)
    {
        const bool firstRssiSamples =
            sessionRssiSamples_ == 0;

        const bool sampleCountSafe =
            sessionRssiSamples_ <=
            static_cast<std::uint64_t>(
                std::numeric_limits<std::int64_t>::max()) -
                    pending.rssiSamples;

        const bool sumSafe =
            pending.rssiSum >= 0
                ? sessionRssiSum_ <=
                      std::numeric_limits<std::int64_t>::max() -
                          pending.rssiSum
                : sessionRssiSum_ >=
                      std::numeric_limits<std::int64_t>::min() -
                          pending.rssiSum;

        if (sampleCountSafe && sumSafe)
        {
            sessionRssiSamples_ +=
                pending.rssiSamples;

            sessionRssiSum_ +=
                pending.rssiSum;

            statistics_.averageRssi =
                static_cast<std::int16_t>(
                    sessionRssiSum_ /
                    static_cast<std::int64_t>(
                        sessionRssiSamples_));

            if (
                firstRssiSamples ||
                pending.strongestRssi >
                    statistics_.strongestRssi)
            {
                statistics_.strongestRssi =
                    pending.strongestRssi;
            }
        }
        else
        {
            recordOverflow();
        }
    }

    statistics_.intervalMs = interval;
    statistics_.elapsedMs =
        now - sessionStartedAt_;

    if (interval > 0)
    {
        const std::uint64_t scaledPackets =
            static_cast<std::uint64_t>(
                pending.totalPackets) *
            1000ULL;

        const std::uint64_t rate =
            scaledPackets / interval;

        statistics_.packetsPerSecond =
            rate > UINT32_MAX
                ? UINT32_MAX
                : static_cast<std::uint32_t>(rate);
    }
    else
    {
        statistics_.packetsPerSecond = 0;
    }

    lastUpdateAt_ = now;
}

void PacketMonitor::addCounter(
    std::uint32_t &target,
    const std::uint32_t increment)
{
    if (increment == 0)
    {
        return;
    }

    if (target <= UINT32_MAX - increment)
    {
        target += increment;
        return;
    }

    target = UINT32_MAX;
    recordOverflow();
}

void PacketMonitor::recordOverflow(
    const std::uint32_t increment)
{
    if (increment == 0)
    {
        return;
    }

    if (
        statistics_.overflowPackets <=
        UINT32_MAX - increment)
    {
        statistics_.overflowPackets += increment;
    }
    else
    {
        statistics_.overflowPackets = UINT32_MAX;
    }
}

void PacketMonitor::fail(
    const std::int16_t errorCode,
    const char *context)
{
    lastErrorCode_ = errorCode;
    state_ = State::Error;
    statistics_.packetsPerSecond = 0;

    Serial.printf(
        "[PacketMonitor] %s failed: code=%d\n",
        context != nullptr
            ? context
            : "operation",
        static_cast<int>(errorCode));
}
