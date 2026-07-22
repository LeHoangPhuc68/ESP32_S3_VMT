#include "WiFiSelectionService.h"

bool WiFiSelectionService::hasSelection_ =
    false;

WiFiScanEntry
    WiFiSelectionService::selected_;

bool WiFiSelectionService::select(
    const WiFiScanEntry &network)
{
    selected_ =
        network;

    hasSelection_ =
        true;

    return true;
}

bool WiFiSelectionService::hasSelection()
{
    return hasSelection_;
}

const WiFiScanEntry *
WiFiSelectionService::selected()
{
    if (!hasSelection_)
    {
        return nullptr;
    }

    return &selected_;
}

void WiFiSelectionService::clear()
{
    selected_ = WiFiScanEntry();

    hasSelection_ =
        false;
}

std::uint8_t
WiFiSelectionService::signalQuality()
{
    if (!hasSelection_)
    {
        return 0;
    }

    const std::int32_t rssi =
        selected_.rssi;

    if (rssi <= -100)
    {
        return 0;
    }

    if (rssi >= -50)
    {
        return 100;
    }

    return static_cast<std::uint8_t>(
        2 * (rssi + 100));
}

std::uint16_t
WiFiSelectionService::frequencyMHz()
{
    if (!hasSelection_)
    {
        return 0;
    }

    const std::int32_t channel =
        selected_.channel;

    /*
     * ESP32-S3 chỉ sử dụng Wi-Fi 2.4 GHz.
     */
    if (
        channel >= 1 &&
        channel <= 13)
    {
        return static_cast<std::uint16_t>(
            2412 +
            ((channel - 1) * 5));
    }

    if (channel == 14)
    {
        return 2484;
    }

    return 0;
}
