#include "WiFiSignalMonitorService.h"

#include "WiFiManager.h"

bool WiFiSignalMonitorService::running_ = false;
WiFiSignalMonitorService::State WiFiSignalMonitorService::state_ =
    WiFiSignalMonitorService::State::Idle;
WiFiScannerService::Network WiFiSignalMonitorService::target_;
std::int32_t WiFiSignalMonitorService::history_[HistorySize] = {};
std::uint8_t WiFiSignalMonitorService::historyCount_ = 0;
std::uint8_t WiFiSignalMonitorService::historyWriteIndex_ = 0;
std::int32_t WiFiSignalMonitorService::currentRssi_ = -100;
std::int32_t WiFiSignalMonitorService::minimumRssi_ = -100;
std::int32_t WiFiSignalMonitorService::maximumRssi_ = -100;
std::int32_t WiFiSignalMonitorService::averageRssi_ = -100;
std::uint32_t WiFiSignalMonitorService::nextScanAt_ = 0;
std::uint32_t WiFiSignalMonitorService::successfulScans_ = 0;
std::uint32_t WiFiSignalMonitorService::missedScans_ = 0;
std::int16_t WiFiSignalMonitorService::lastScanCode_ = 0;

bool WiFiSignalMonitorService::start(
    const WiFiScannerService::Network &target)
{
    stop();

    if (target.bssid.length() == 0)
    {
        state_ = State::Failed;
        lastScanCode_ = -200;
        return false;
    }

    if (!WiFiManager::begin())
    {
        state_ = State::Failed;
        lastScanCode_ = -201;
        return false;
    }

    target_ = target;
    historyCount_ = 0;
    historyWriteIndex_ = 0;
    currentRssi_ = target.rssi;
    minimumRssi_ = target.rssi;
    maximumRssi_ = target.rssi;
    averageRssi_ = target.rssi;
    successfulScans_ = 0;
    missedScans_ = 0;
    lastScanCode_ = 0;

    for (std::uint8_t index = 0; index < HistorySize; ++index)
    {
        history_[index] = -100;
    }

    addSample(target.rssi);

    running_ = true;
    state_ = State::Ready;
    nextScanAt_ = millis();

    Serial.printf(
        "[SignalMonitor] Started: %s %s CH %ld\n",
        target_.ssid.c_str(),
        target_.bssid.c_str(),
        static_cast<long>(target_.channel));

    return true;
}

void WiFiSignalMonitorService::stop()
{
    if (WiFiManager::isOwnedBy(WiFiManager::Owner::SignalMonitor))
    {
        WiFiManager::cancel(WiFiManager::Owner::SignalMonitor);
    }

    running_ = false;
    state_ = State::Idle;
    nextScanAt_ = 0;
    lastScanCode_ = 0;
}

void WiFiSignalMonitorService::update()
{
    if (!running_)
    {
        return;
    }

    WiFiManager::update();

    if (WiFiManager::isOwnedBy(WiFiManager::Owner::SignalMonitor))
    {
        lastScanCode_ = WiFiManager::lastScanCode();

        switch (WiFiManager::state())
        {
        case WiFiManager::State::Preparing:
            state_ = State::Preparing;
            return;

        case WiFiManager::State::Scanning:
            state_ = State::Scanning;
            return;

        case WiFiManager::State::Ready:
            consumeResults(WiFiManager::resultCount());
            WiFiManager::release(WiFiManager::Owner::SignalMonitor);
            return;

        case WiFiManager::State::Failed:
            state_ = State::Failed;
            nextScanAt_ = millis() + RetryDelayMs;
            WiFiManager::release(WiFiManager::Owner::SignalMonitor);
            return;

        case WiFiManager::State::Idle:
        case WiFiManager::State::Uninitialized:
        default:
            break;
        }
    }

    const std::uint32_t now = millis();

    if (static_cast<std::int32_t>(now - nextScanAt_) >= 0)
    {
        if (!requestSample())
        {
            state_ = State::Failed;
            nextScanAt_ = now + RetryDelayMs;
        }
    }
}

void WiFiSignalMonitorService::requestImmediateSample()
{
    if (!running_)
    {
        return;
    }

    if (WiFiManager::isOwnedBy(WiFiManager::Owner::SignalMonitor))
    {
        return;
    }

    nextScanAt_ = millis();
}

bool WiFiSignalMonitorService::isRunning()
{
    return running_;
}

WiFiSignalMonitorService::State WiFiSignalMonitorService::state()
{
    return state_;
}

const char *WiFiSignalMonitorService::stateText()
{
    switch (state_)
    {
    case State::Idle:
        return "IDLE";
    case State::Preparing:
        return "PREPARING";
    case State::Scanning:
        return "SCANNING";
    case State::Ready:
        return "TRACKING";
    case State::NotFound:
        return "AP LOST";
    case State::Failed:
        return "RETRYING";
    default:
        return "UNKNOWN";
    }
}

const WiFiScannerService::Network *WiFiSignalMonitorService::target()
{
    return running_ ? &target_ : nullptr;
}

bool WiFiSignalMonitorService::hasSample()
{
    return historyCount_ > 0;
}

std::int32_t WiFiSignalMonitorService::currentRssi()
{
    return currentRssi_;
}

std::int32_t WiFiSignalMonitorService::minimumRssi()
{
    return minimumRssi_;
}

std::int32_t WiFiSignalMonitorService::maximumRssi()
{
    return maximumRssi_;
}

std::int32_t WiFiSignalMonitorService::averageRssi()
{
    return averageRssi_;
}

std::int32_t WiFiSignalMonitorService::rangeDb()
{
    return hasSample() ? maximumRssi_ - minimumRssi_ : 0;
}

std::uint8_t WiFiSignalMonitorService::signalQuality()
{
    if (!hasSample() || currentRssi_ <= -100)
    {
        return 0;
    }

    if (currentRssi_ >= -50)
    {
        return 100;
    }

    return static_cast<std::uint8_t>(2 * (currentRssi_ + 100));
}

const char *WiFiSignalMonitorService::qualityText()
{
    if (!hasSample())
    {
        return "UNKNOWN";
    }

    if (currentRssi_ >= -55)
    {
        return "EXCELLENT";
    }
    if (currentRssi_ >= -65)
    {
        return "GOOD";
    }
    if (currentRssi_ >= -75)
    {
        return "FAIR";
    }
    return "WEAK";
}

WiFiSignalMonitorService::Trend WiFiSignalMonitorService::trend()
{
    if (historyCount_ < 6)
    {
        return Trend::Unknown;
    }

    const std::uint8_t count = historyCount_ < 10 ? historyCount_ : 10;
    const std::uint8_t half = count / 2;
    const std::uint8_t start = historyCount_ - count;
    std::int32_t olderSum = 0;
    std::int32_t newerSum = 0;

    for (std::uint8_t index = 0; index < count; ++index)
    {
        const std::int32_t value = sample(start + index);

        if (index < half)
        {
            olderSum += value;
        }
        else
        {
            newerSum += value;
        }
    }

    const std::int32_t olderAverage = olderSum / half;
    const std::int32_t newerAverage = newerSum / (count - half);
    const std::int32_t difference = newerAverage - olderAverage;

    if (difference >= 3)
    {
        return Trend::Stronger;
    }
    if (difference <= -3)
    {
        return Trend::Weaker;
    }
    return Trend::Stable;
}

const char *WiFiSignalMonitorService::trendText()
{
    switch (trend())
    {
    case Trend::Stronger:
        return "GETTING STRONGER";
    case Trend::Weaker:
        return "GETTING WEAKER";
    case Trend::Stable:
        return "STABLE";
    case Trend::Unknown:
    default:
        return "COLLECTING";
    }
}

std::uint8_t WiFiSignalMonitorService::sampleCount()
{
    return historyCount_;
}

std::int32_t WiFiSignalMonitorService::sample(const std::uint8_t index)
{
    if (index >= historyCount_)
    {
        return -100;
    }

    std::uint8_t physicalIndex = index;

    if (historyCount_ == HistorySize)
    {
        physicalIndex = static_cast<std::uint8_t>(
            (historyWriteIndex_ + index) % HistorySize);
    }

    return history_[physicalIndex];
}

std::uint32_t WiFiSignalMonitorService::successfulScans()
{
    return successfulScans_;
}

std::uint32_t WiFiSignalMonitorService::missedScans()
{
    return missedScans_;
}

std::int16_t WiFiSignalMonitorService::lastScanCode()
{
    return lastScanCode_;
}

bool WiFiSignalMonitorService::requestSample()
{
    const std::uint8_t channel =
        target_.channel >= 1 && target_.channel <= 14
            ? static_cast<std::uint8_t>(target_.channel)
            : 0;

    if (!WiFiManager::requestScan(
            WiFiManager::Owner::SignalMonitor,
            channel,
            true,
            300))
    {
        return false;
    }

    state_ = State::Preparing;
    return true;
}

void WiFiSignalMonitorService::consumeResults(
    const std::int16_t resultCount)
{
    bool found = false;
    std::int32_t strongestRssi = -127;

    for (std::int16_t index = 0; index < resultCount; ++index)
    {
        if (!bssidMatches(WiFiManager::bssid(index), target_.bssid))
        {
            continue;
        }

        const std::int32_t value = WiFiManager::rssi(index);

        if (!found || value > strongestRssi)
        {
            found = true;
            strongestRssi = value;
        }
    }

    const std::uint32_t now = millis();

    if (found)
    {
        addSample(strongestRssi);
        ++successfulScans_;
        state_ = State::Ready;
        nextScanAt_ = now + SampleIntervalMs;
        return;
    }

    ++missedScans_;
    state_ = State::NotFound;
    nextScanAt_ = now + RetryDelayMs;
}

void WiFiSignalMonitorService::addSample(const std::int32_t rssi)
{
    history_[historyWriteIndex_] = rssi;
    historyWriteIndex_ = static_cast<std::uint8_t>(
        (historyWriteIndex_ + 1) % HistorySize);

    if (historyCount_ < HistorySize)
    {
        ++historyCount_;
    }

    currentRssi_ = rssi;
    recalculateStatistics();
}

void WiFiSignalMonitorService::recalculateStatistics()
{
    if (historyCount_ == 0)
    {
        currentRssi_ = -100;
        minimumRssi_ = -100;
        maximumRssi_ = -100;
        averageRssi_ = -100;
        return;
    }

    std::int32_t total = 0;
    minimumRssi_ = sample(0);
    maximumRssi_ = sample(0);

    for (std::uint8_t index = 0; index < historyCount_; ++index)
    {
        const std::int32_t value = sample(index);
        total += value;

        if (value < minimumRssi_)
        {
            minimumRssi_ = value;
        }
        if (value > maximumRssi_)
        {
            maximumRssi_ = value;
        }
    }

    averageRssi_ = total / static_cast<std::int32_t>(historyCount_);
}

bool WiFiSignalMonitorService::bssidMatches(
    const String &left,
    const String &right)
{
    return left.equalsIgnoreCase(right);
}
