#pragma once

#include <cstdint>

enum class ScreenId : std::uint8_t
{
    Home = 0,
    Dashboard,
    WiFiMenu,
    WiFiScanner,
    WiFiAccessPoint,
    WiFiSignalMonitor,
    WiFiChannelAnalyzer,
    WiFiPacketMonitor,

    Count,
    Invalid = 0xFF
};
