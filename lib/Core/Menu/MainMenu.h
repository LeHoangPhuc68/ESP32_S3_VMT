#pragma once

#include <cstdint>

#include "MenuItem.h"

namespace MainMenu
{
    /*
     * Index chỉ dùng khi cần truy cập vị trí menu.
     *
     * Việc khởi chạy ứng dụng không còn dựa trên index,
     * mà sử dụng MenuItem::appId.
     */
    enum ItemIndex : std::uint8_t
    {
        Dashboard = 0,
        WiFiTools,
        Bluetooth,
        USBTools,
        System,
        Settings
    };

    extern const MenuItem Items[];

    extern const std::uint8_t ItemCount;
}