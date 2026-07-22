#include "WiFiScannerService.h"

#include "WiFiManager.h"

bool WiFiScannerService::initialized_ =
    false;

WiFiScannerService::State
    WiFiScannerService::state_ =
        WiFiScannerService::State::Idle;

WiFiScanSnapshot
    WiFiScannerService::snapshot_;

WiFiScannerService::Network
    WiFiScannerService::stagedNetworks_[
        WiFiScannerService::MaxNetworks];

std::uint8_t
    WiFiScannerService::stagedNetworkCount_ =
        0;

std::int16_t
    WiFiScannerService::lastScanCode_ =
        0;

bool WiFiScannerService::begin()
{
    if (initialized_)
    {
        return true;
    }

    if (!WiFiManager::begin())
    {
        failScan(InitializationError);
        return false;
    }

    initialized_ = true;
    state_ = State::Idle;
    lastScanCode_ = 0;
    snapshot_.clear();

    Serial.println("[WiFiScanner] Ready");
    return true;
}

bool WiFiScannerService::startScan()
{
    if (!initialized_ && !begin())
    {
        return false;
    }

    if (isScanning())
    {
        return false;
    }

    constexpr WiFiManager::Owner Owner = WiFiManager::Owner::Scanner;

    if (!WiFiManager::acquire(Owner))
    {
        failScan(OwnershipUnavailableError);
        return false;
    }

    if (!WiFiManager::requestScan(Owner, 0, true, 300))
    {
        const std::int16_t managerCode = WiFiManager::lastScanCode();
        WiFiManager::release(Owner);
        failScan(managerCode != 0 ? managerCode : ScanRequestError);
        return false;
    }

    state_ = State::Preparing;
    lastScanCode_ = 0;

    Serial.println("[WiFiScanner] Scan started");
    return true;
}

void WiFiScannerService::update()
{
    if (!isScanning())
    {
        return;
    }

    constexpr WiFiManager::Owner Owner = WiFiManager::Owner::Scanner;

    if (!WiFiManager::isOwnedBy(Owner))
    {
        failScan(OwnershipLostError);
        return;
    }

    WiFiManager::update();
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
        stageResults(WiFiManager::resultCount());

        if (!WiFiManager::release(Owner))
        {
            stagedNetworkCount_ = 0;
            failScan(OwnershipLostError);
            return;
        }

        publishStagedResults();
        state_ = State::Complete;
        return;

    case WiFiManager::State::Failed:
    {
        const std::int16_t managerCode = WiFiManager::lastScanCode();
        WiFiManager::release(Owner);
        failScan(managerCode != 0 ? managerCode : ScanRequestError);
        return;
    }

    case WiFiManager::State::Idle:
    case WiFiManager::State::Uninitialized:
    default:
        WiFiManager::release(Owner);
        failScan(OwnershipLostError);
        return;
    }
}

void WiFiScannerService::cancelScan()
{
    constexpr WiFiManager::Owner Owner = WiFiManager::Owner::Scanner;
    const bool wasScanning = isScanning();

    if (!WiFiManager::isOwnedBy(Owner))
    {
        if (wasScanning)
        {
            failScan(OwnershipLostError);
        }

        return;
    }

    const bool cancelled = WiFiManager::cancel(Owner);
    const bool released = WiFiManager::release(Owner);

    if (!cancelled || !released)
    {
        if (wasScanning)
        {
            failScan(OwnershipLostError);
        }

        return;
    }

    if (!wasScanning)
    {
        return;
    }

    state_ = networkCount() > 0 ? State::Complete : State::Idle;
    lastScanCode_ = 0;
    Serial.println("[WiFiScanner] Scan cancelled");
}

void WiFiScannerService::clear()
{
    cancelScan();
    stagedNetworkCount_ = 0;
    snapshot_.clear();
    state_ = State::Idle;
    lastScanCode_ = 0;
}

WiFiScannerService::State
WiFiScannerService::state()
{
    return state_;
}

bool WiFiScannerService::isScanning()
{
    return
        state_ == State::Preparing ||
        state_ == State::Scanning;
}

std::uint8_t
WiFiScannerService::networkCount()
{
    return snapshot_.count();
}

const WiFiScannerService::Network *
WiFiScannerService::network(
    const std::uint8_t index)
{
    return snapshot_.entry(index);
}

const WiFiScanSnapshot &
WiFiScannerService::snapshot()
{
    return snapshot_;
}

const char *
WiFiScannerService::stateText()
{
    switch (state_)
    {
    case State::Idle:
        return "IDLE";

    case State::Preparing:
        return "PREPARING";

    case State::Scanning:
        return "SCANNING";

    case State::Complete:
        return "COMPLETE";

    case State::Failed:
        return "FAILED";

    default:
        return "UNKNOWN";
    }
}

const char *
WiFiScannerService::securityText(
    const std::uint8_t encryptionType)
{
    return WiFiManager::securityText(
        encryptionType);
}

std::int16_t
WiFiScannerService::lastScanCode()
{
    return lastScanCode_;
}

void WiFiScannerService::failScan(const std::int16_t errorCode)
{
    state_ = State::Failed;
    lastScanCode_ = errorCode;

    Serial.printf(
        "[WiFiScanner] Failed: code=%d managerState=%s owner=%u\n",
        static_cast<int>(errorCode),
        WiFiManager::stateText(),
        static_cast<unsigned int>(WiFiManager::owner()));
}

void WiFiScannerService::stageResults(
    const std::int16_t resultCount)
{
    stagedNetworkCount_ = 0;

    for (std::int16_t index = 0;
         index < resultCount && stagedNetworkCount_ < MaxNetworks;
         ++index)
    {
        Network &network =
            stagedNetworks_[stagedNetworkCount_];

        network.ssid = WiFiManager::ssid(index);
        network.bssid = WiFiManager::bssid(index);
        network.rssi = WiFiManager::rssi(index);
        network.channel = WiFiManager::channel(index);
        network.encryptionType = WiFiManager::encryptionType(index);
        network.hidden = network.ssid.length() == 0;

        if (network.hidden)
        {
            network.ssid = "<hidden>";
        }

        ++stagedNetworkCount_;
    }

    sortBySignalStrength(stagedNetworkCount_);
}

void WiFiScannerService::publishStagedResults()
{
    const std::uint8_t publishedCount =
        stagedNetworkCount_;

    for (std::uint8_t index = 0;
         index < publishedCount;
         ++index)
    {
        snapshot_.entryForPublication(index) =
            stagedNetworks_[index];
    }

    snapshot_.publish(publishedCount, millis());
    stagedNetworkCount_ = 0;

    Serial.printf(
        "[WiFiScanner] Published generation=%lu networks=%u\n",
        static_cast<unsigned long>(snapshot_.generation()),
        static_cast<unsigned int>(publishedCount));
}

void WiFiScannerService::sortBySignalStrength(
    const std::uint8_t count)
{
    if (count < 2)
    {
        return;
    }

    for (std::uint8_t index = 1;
         index < count;
         ++index)
    {
        Network current =
            stagedNetworks_[index];

        std::int16_t position =
            static_cast<std::int16_t>(
                index) - 1;

        while (
            position >= 0 &&
            stagedNetworks_[position].rssi <
                current.rssi)
        {
            stagedNetworks_[position + 1] =
                stagedNetworks_[position];

            --position;
        }

        stagedNetworks_[position + 1] =
            current;
    }
}
