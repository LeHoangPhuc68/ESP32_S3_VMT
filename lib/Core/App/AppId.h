#pragma once

#include <cstdint>

/*
 * Định danh duy nhất cho từng ứng dụng/tính năng.
 *
 * Menu chỉ giữ AppId, không trực tiếp biết cách
 * khởi chạy ứng dụng.
 */
enum class AppId : std::uint8_t
{
    Dashboard = 0,
    WiFiTools,
    Bluetooth,
    USBTools,
    System,
    Settings,
    ChannelAnalyzer,
    PacketMonitor,

    Count,
    Invalid = 0xFF
};
