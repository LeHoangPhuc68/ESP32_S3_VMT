#pragma once

#include <cstdint>

enum class ScreenId : std::uint8_t
{
    Home = 0,
    Dashboard,
    WiFiScanner,
    WiFiAccessPoint,
    WiFiSignalMonitor,

    Count,
    Invalid = 0xFF
};