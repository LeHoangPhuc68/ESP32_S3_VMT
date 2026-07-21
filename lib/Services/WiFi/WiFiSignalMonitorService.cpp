#include "WiFiSignalMonitorService.h"

#include <WiFi.h>
#include <esp_wifi.h>

bool WiFiSignalMonitorService::running_ =
    false;

WiFiSignalMonitorService::State
    WiFiSignalMonitorService::state_ =
        WiFiSignalMonitorService::State::Idle;

WiFiScannerService::Network
    WiFiSignalMonitorService::target_;

std::int32_t
    WiFiSignalMonitorService::history_[
        WiFiSignalMonitorService::HistorySize] =
        {};

std::uint8_t
    WiFiSignalMonitorService::historyCount_ =
        0;

std::uint8_t
    WiFiSignalMonitorService::historyWriteIndex_ =
        0;

std::int32_t
    WiFiSignalMonitorService::currentRssi_ =
        -100;

std::int32_t
    WiFiSignalMonitorService::minimumRssi_ =
        -100;

std::int32_t
    WiFiSignalMonitorService::maximumRssi_ =
        -100;

std::int32_t
    WiFiSignalMonitorService::averageRssi_ =
        -100;

std::uint32_t
    WiFiSignalMonitorService::stateStartedAt_ =
        0;

std::uint32_t
    WiFiSignalMonitorService::nextScanAt_ =
        0;

std::uint32_t
    WiFiSignalMonitorService::successfulScans_ =
        0;

std::uint32_t
    WiFiSignalMonitorService::missedScans_ =
        0;

std::int16_t
    WiFiSignalMonitorService::lastScanCode_ =
        0;

bool WiFiSignalMonitorService::start(
    const WiFiScannerService::Network &target)
{
    stop();

    if (target.bssid.length() == 0)
    {
        Serial.println(
            "[SignalMonitor] Invalid target BSSID");

        state_ =
            State::Failed;

        lastScanCode_ =
            -200;

        return false;
    }

    target_ =
        target;

    for (std::uint8_t index = 0;
         index < HistorySize;
         ++index)
    {
        history_[index] =
            -100;
    }

    historyCount_ =
        0;

    historyWriteIndex_ =
        0;

    currentRssi_ =
        target.rssi;

    minimumRssi_ =
        target.rssi;

    maximumRssi_ =
        target.rssi;

    averageRssi_ =
        target.rssi;

    successfulScans_ =
        0;

    missedScans_ =
        0;

    lastScanCode_ =
        0;

    if (!prepareRadio())
    {
        state_ =
            State::Failed;

        lastScanCode_ =
            -201;

        return false;
    }

    /*
     * Dùng RSSI từ màn hình scanner làm mẫu đầu tiên.
     */
    addSample(
        target.rssi);

    running_ =
        true;

    state_ =
        State::Preparing;

    stateStartedAt_ =
        millis();

    nextScanAt_ =
        millis();

    Serial.printf(
        "[SignalMonitor] Started: %s %s CH %ld\n",
        target_.ssid.c_str(),
        target_.bssid.c_str(),
        static_cast<long>(
            target_.channel));

    return true;
}

void WiFiSignalMonitorService::stop()
{
    if (
        state_ == State::Preparing ||
        state_ == State::Scanning)
    {
        esp_wifi_scan_stop();
    }

    WiFi.scanDelete();

    running_ =
        false;

    state_ =
        State::Idle;

    stateStartedAt_ =
        millis();

    nextScanAt_ =
        0;

    lastScanCode_ =
        0;
}

void WiFiSignalMonitorService::update()
{
    if (!running_)
    {
        return;
    }

    const std::uint32_t now =
        millis();

    if (
        state_ == State::Ready ||
        state_ == State::NotFound ||
        state_ == State::Failed)
    {
        if (
            static_cast<std::int32_t>(
                now - nextScanAt_) >= 0)
        {
            if (!prepareRadio())
            {
                state_ =
                    State::Failed;

                nextScanAt_ =
                    now + RetryDelayMs;

                return;
            }

            state_ =
                State::Preparing;

            stateStartedAt_ =
                now;
        }

        return;
    }

    if (state_ == State::Preparing)
    {
        if (
            now - stateStartedAt_ <
            PrepareDelayMs)
        {
            return;
        }

        if (!launchScan())
        {
            state_ =
                State::Failed;

            nextScanAt_ =
                now + RetryDelayMs;
        }

        return;
    }

    if (state_ != State::Scanning)
    {
        return;
    }

    const std::int16_t result =
        WiFi.scanComplete();

    lastScanCode_ =
        result;

    if (result == WIFI_SCAN_RUNNING)
    {
        if (
            now - stateStartedAt_ >
            ScanTimeoutMs)
        {
            Serial.println(
                "[SignalMonitor] Scan timeout");

            esp_wifi_scan_stop();
            WiFi.scanDelete();

            state_ =
                State::Failed;

            lastScanCode_ =
                -202;

            nextScanAt_ =
                now + RetryDelayMs;
        }

        return;
    }

    if (result == WIFI_SCAN_FAILED)
    {
        Serial.println(
            "[SignalMonitor] Scan failed");

        WiFi.scanDelete();

        state_ =
            State::Failed;

        nextScanAt_ =
            now + RetryDelayMs;

        return;
    }

    if (result < 0)
    {
        if (
            now - stateStartedAt_ >
            ScanTimeoutMs)
        {
            WiFi.scanDelete();

            state_ =
                State::Failed;

            nextScanAt_ =
                now + RetryDelayMs;
        }

        return;
    }

    processResults(
        result);
}

void WiFiSignalMonitorService::requestImmediateSample()
{
    if (!running_)
    {
        return;
    }

    if (
        state_ == State::Preparing ||
        state_ == State::Scanning)
    {
        return;
    }

    nextScanAt_ =
        millis();
}

bool WiFiSignalMonitorService::isRunning()
{
    return running_;
}

WiFiSignalMonitorService::State
WiFiSignalMonitorService::state()
{
    return state_;
}

const char *
WiFiSignalMonitorService::stateText()
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

const WiFiScannerService::Network *
WiFiSignalMonitorService::target()
{
    if (!running_)
    {
        return nullptr;
    }

    return &target_;
}

bool WiFiSignalMonitorService::hasSample()
{
    return historyCount_ > 0;
}

std::int32_t
WiFiSignalMonitorService::currentRssi()
{
    return currentRssi_;
}

std::int32_t
WiFiSignalMonitorService::minimumRssi()
{
    return minimumRssi_;
}

std::int32_t
WiFiSignalMonitorService::maximumRssi()
{
    return maximumRssi_;
}

std::int32_t
WiFiSignalMonitorService::averageRssi()
{
    return averageRssi_;
}

std::int32_t
WiFiSignalMonitorService::rangeDb()
{
    if (!hasSample())
    {
        return 0;
    }

    return
        maximumRssi_ -
        minimumRssi_;
}

std::uint8_t
WiFiSignalMonitorService::signalQuality()
{
    if (!hasSample())
    {
        return 0;
    }

    if (currentRssi_ <= -100)
    {
        return 0;
    }

    if (currentRssi_ >= -50)
    {
        return 100;
    }

    return static_cast<std::uint8_t>(
        2 * (currentRssi_ + 100));
}

const char *
WiFiSignalMonitorService::qualityText()
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

WiFiSignalMonitorService::Trend
WiFiSignalMonitorService::trend()
{
    if (historyCount_ < 6)
    {
        return Trend::Unknown;
    }

    const std::uint8_t count =
        historyCount_ < 10
            ? historyCount_
            : 10;

    const std::uint8_t half =
        count / 2;

    std::int32_t olderSum =
        0;

    std::int32_t newerSum =
        0;

    const std::uint8_t start =
        historyCount_ - count;

    for (std::uint8_t index = 0;
         index < count;
         ++index)
    {
        const std::int32_t value =
            sample(
                start + index);

        if (index < half)
        {
            olderSum +=
                value;
        }
        else
        {
            newerSum +=
                value;
        }
    }

    const std::int32_t olderAverage =
        olderSum /
        static_cast<std::int32_t>(
            half);

    const std::int32_t newerAverage =
        newerSum /
        static_cast<std::int32_t>(
            count - half);

    const std::int32_t difference =
        newerAverage -
        olderAverage;

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

const char *
WiFiSignalMonitorService::trendText()
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

std::uint8_t
WiFiSignalMonitorService::sampleCount()
{
    return historyCount_;
}

std::int32_t
WiFiSignalMonitorService::sample(
    const std::uint8_t index)
{
    if (index >= historyCount_)
    {
        return -100;
    }

    /*
     * Khi buffer chưa đầy, mẫu đầu nằm tại index 0.
     * Khi buffer đầy, writeIndex trỏ tới mẫu cũ nhất.
     */
    std::uint8_t physicalIndex =
        index;

    if (historyCount_ == HistorySize)
    {
        physicalIndex =
            static_cast<std::uint8_t>(
                (historyWriteIndex_ + index) %
                HistorySize);
    }

    return history_[physicalIndex];
}

std::uint32_t
WiFiSignalMonitorService::successfulScans()
{
    return successfulScans_;
}

std::uint32_t
WiFiSignalMonitorService::missedScans()
{
    return missedScans_;
}

std::int16_t
WiFiSignalMonitorService::lastScanCode()
{
    return lastScanCode_;
}

bool WiFiSignalMonitorService::prepareRadio()
{
    if (WiFi.getMode() != WIFI_STA)
    {
        WiFi.mode(
            WIFI_OFF);

        delay(50);

        if (!WiFi.mode(
                WIFI_STA))
        {
            Serial.println(
                "[SignalMonitor] Cannot enter STA mode");

            return false;
        }
    }

    WiFi.setSleep(
        false);

    WiFi.disconnect(
        false,
        false);

    esp_wifi_scan_stop();

    WiFi.scanDelete();

    return true;
}

bool WiFiSignalMonitorService::launchScan()
{
    /*
     * Chỉ scan channel của AP mục tiêu.
     *
     * Nhanh hơn quét toàn bộ 2.4 GHz và phù hợp
     * với Signal Monitor active-scan phiên bản đầu.
     */
    std::uint8_t channel =
        0;

    if (
        target_.channel >= 1 &&
        target_.channel <= 14)
    {
        channel =
            static_cast<std::uint8_t>(
                target_.channel);
    }

    const std::int16_t result =
        WiFi.scanNetworks(
            true,
            true,
            false,
            300,
            channel);

    lastScanCode_ =
        result;

    Serial.printf(
        "[SignalMonitor] Scan start: result=%d channel=%u\n",
        static_cast<int>(
            result),
        static_cast<unsigned int>(
            channel));

    if (result == WIFI_SCAN_RUNNING)
    {
        state_ =
            State::Scanning;

        stateStartedAt_ =
            millis();

        return true;
    }

    if (result >= 0)
    {
        processResults(
            result);

        return true;
    }

    /*
     * Recovery khi driver từ chối khởi động scan.
     */
    if (result == WIFI_SCAN_FAILED)
    {
        WiFi.disconnect(
            false,
            false);

        delay(80);

        const std::int16_t retryResult =
            WiFi.scanNetworks(
                true,
                true,
                false,
                300,
                channel);

        lastScanCode_ =
            retryResult;

        if (retryResult == WIFI_SCAN_RUNNING)
        {
            state_ =
                State::Scanning;

            stateStartedAt_ =
                millis();

            return true;
        }

        if (retryResult >= 0)
        {
            processResults(
                retryResult);

            return true;
        }
    }

    return false;
}

void WiFiSignalMonitorService::processResults(
    const std::int16_t resultCount)
{
    bool found =
        false;

    std::int32_t strongestRssi =
        -127;

    for (std::int16_t index = 0;
         index < resultCount;
         ++index)
    {
        const String bssid =
            WiFi.BSSIDstr(
                index);

        if (!bssidMatches(
                bssid,
                target_.bssid))
        {
            continue;
        }

        const std::int32_t rssi =
            WiFi.RSSI(
                index);

        if (
            !found ||
            rssi > strongestRssi)
        {
            strongestRssi =
                rssi;

            found =
                true;
        }
    }

    WiFi.scanDelete();

    const std::uint32_t now =
        millis();

    if (found)
    {
        addSample(
            strongestRssi);

        ++successfulScans_;

        state_ =
            State::Ready;

        nextScanAt_ =
            now + SampleIntervalMs;

        Serial.printf(
            "[SignalMonitor] RSSI %ld dBm\n",
            static_cast<long>(
                strongestRssi));

        return;
    }

    ++missedScans_;

    state_ =
        State::NotFound;

    nextScanAt_ =
        now + RetryDelayMs;

    Serial.printf(
        "[SignalMonitor] Target not found, miss=%lu\n",
        static_cast<unsigned long>(
            missedScans_));
}

void WiFiSignalMonitorService::addSample(
    const std::int32_t rssi)
{
    history_[historyWriteIndex_] =
        rssi;

    historyWriteIndex_ =
        static_cast<std::uint8_t>(
            (historyWriteIndex_ + 1) %
            HistorySize);

    if (historyCount_ < HistorySize)
    {
        ++historyCount_;
    }

    currentRssi_ =
        rssi;

    recalculateStatistics();
}

void WiFiSignalMonitorService::recalculateStatistics()
{
    if (historyCount_ == 0)
    {
        currentRssi_ =
            -100;

        minimumRssi_ =
            -100;

        maximumRssi_ =
            -100;

        averageRssi_ =
            -100;

        return;
    }

    std::int32_t total =
        0;

    minimumRssi_ =
        sample(0);

    maximumRssi_ =
        sample(0);

    for (std::uint8_t index = 0;
         index < historyCount_;
         ++index)
    {
        const std::int32_t value =
            sample(
                index);

        total +=
            value;

        if (value < minimumRssi_)
        {
            minimumRssi_ =
                value;
        }

        if (value > maximumRssi_)
        {
            maximumRssi_ =
                value;
        }
    }

    averageRssi_ =
        total /
        static_cast<std::int32_t>(
            historyCount_);
}

bool WiFiSignalMonitorService::bssidMatches(
    const String &left,
    const String &right)
{
    return left.equalsIgnoreCase(
        right);
}