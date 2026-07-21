#include "WiFiScannerService.h"

#include <WiFi.h>

#include "WiFiManager.h"

bool WiFiScannerService::initialized_ = false;
WiFiScannerService::State WiFiScannerService::state_ =
    WiFiScannerService::State::Idle;
WiFiScannerService::Network WiFiScannerService::networks_[
    WiFiScannerService::MaxNetworks];
std::uint8_t WiFiScannerService::networkCount_ = 0;
std::int16_t WiFiScannerService::lastScanCode_ = 0;

bool WiFiScannerService::begin()
{
    if (initialized_)
    {
        return true;
    }

    if (!WiFiManager::begin())
    {
        state_ = State::Failed;
        lastScanCode_ = -100;
        return false;
    }

    initialized_ = true;
    state_ = State::Idle;
    lastScanCode_ = 0;
    clearNetworks();
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

    if (!WiFiManager::requestScan(
            WiFiManager::Owner::Scanner,
            0,
            true,
            300))
    {
        state_ = State::Failed;
        lastScanCode_ = -110;
        return false;
    }

    /*
     * Không xóa danh sách cũ tại đây.
     * Trong lúc rescan, UI vẫn có thể hiển thị kết quả trước đó.
     * Danh sách chỉ được thay thế khi scan mới hoàn tất.
     */
    state_ = State::Preparing;
    lastScanCode_ = 0;

    Serial.println("[WiFiScanner] Rescan started");
    return true;
}

void WiFiScannerService::update()
{
    WiFiManager::update();

    if (!WiFiManager::isOwnedBy(WiFiManager::Owner::Scanner))
    {
        return;
    }

    lastScanCode_ = WiFiManager::lastScanCode();

    switch (WiFiManager::state())
    {
    case WiFiManager::State::Preparing:
        state_ = State::Preparing;
        break;

    case WiFiManager::State::Scanning:
        state_ = State::Scanning;
        break;

    case WiFiManager::State::Ready:
    {
        collectResults(WiFiManager::resultCount());
        sortBySignalStrength();
        state_ = State::Complete;
        WiFiManager::release(WiFiManager::Owner::Scanner);
        break;
    }

    case WiFiManager::State::Failed:
        state_ = State::Failed;
        WiFiManager::release(WiFiManager::Owner::Scanner);
        break;

    case WiFiManager::State::Idle:
    case WiFiManager::State::Uninitialized:
    default:
        break;
    }
}

void WiFiScannerService::clear()
{
    if (WiFiManager::isOwnedBy(WiFiManager::Owner::Scanner))
    {
        WiFiManager::cancel(WiFiManager::Owner::Scanner);
    }

    clearNetworks();
    state_ = State::Idle;
    lastScanCode_ = 0;
}

WiFiScannerService::State WiFiScannerService::state()
{
    return state_;
}

bool WiFiScannerService::isScanning()
{
    return state_ == State::Preparing || state_ == State::Scanning;
}

std::uint8_t WiFiScannerService::networkCount()
{
    return networkCount_;
}

const WiFiScannerService::Network *WiFiScannerService::network(
    const std::uint8_t index)
{
    return index < networkCount_ ? &networks_[index] : nullptr;
}

const char *WiFiScannerService::stateText()
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

const char *WiFiScannerService::securityText(
    const std::uint8_t encryptionType)
{
    switch (encryptionType)
    {
    case WIFI_AUTH_OPEN:
        return "OPEN";
    case WIFI_AUTH_WEP:
        return "WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WPA";
    case WIFI_AUTH_WPA2_PSK:
        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2-E";
#if defined(WIFI_AUTH_WPA3_PSK)
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3";
#endif
#if defined(WIFI_AUTH_WPA2_WPA3_PSK)
    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2/3";
#endif
#if defined(WIFI_AUTH_WAPI_PSK)
    case WIFI_AUTH_WAPI_PSK:
        return "WAPI";
#endif
    default:
        return "SECURED";
    }
}

std::int16_t WiFiScannerService::lastScanCode()
{
    return lastScanCode_;
}

void WiFiScannerService::collectResults(const std::int16_t resultCount)
{
    Network freshNetworks[MaxNetworks];
    std::uint8_t freshCount = 0;

    for (std::int16_t index = 0;
         index < resultCount && freshCount < MaxNetworks;
         ++index)
    {
        Network &network = freshNetworks[freshCount];
        network.ssid = WiFiManager::ssid(index);
        network.bssid = WiFiManager::bssid(index);
        network.rssi = WiFiManager::rssi(index);
        network.channel = WiFiManager::channel(index);
        network.encryptionType = WiFiManager::encryptionType(index);
        network.hidden = network.ssid.length() == 0;

        if (network.hidden)
        {
            network.ssid = "<HIDDEN>";
        }

        ++freshCount;
    }

    clearNetworks();
    networkCount_ = freshCount;

    for (std::uint8_t index = 0; index < freshCount; ++index)
    {
        networks_[index] = freshNetworks[index];
    }

    Serial.printf(
        "[WiFiScanner] Stored %u networks\n",
        static_cast<unsigned int>(networkCount_));
}

void WiFiScannerService::sortBySignalStrength()
{
    for (std::uint8_t left = 0; left < networkCount_; ++left)
    {
        for (std::uint8_t right = left + 1; right < networkCount_; ++right)
        {
            if (networks_[right].rssi > networks_[left].rssi)
            {
                const Network temporary = networks_[left];
                networks_[left] = networks_[right];
                networks_[right] = temporary;
            }
        }
    }
}

void WiFiScannerService::clearNetworks()
{
    for (std::uint8_t index = 0; index < MaxNetworks; ++index)
    {
        networks_[index] = Network();
    }

    networkCount_ = 0;
}
