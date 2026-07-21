#pragma once

#include <cstdint>

/*
 * Kết quả trả về sau khi AppManager nhận yêu cầu
 * khởi chạy một ứng dụng.
 */
enum class AppLaunchStatus : std::uint8_t
{
    Launched = 0,
    NotImplemented,
    InvalidApp
};

struct AppLaunchResult
{
    AppLaunchStatus status =
        AppLaunchStatus::InvalidApp;

    const char *message = nullptr;

    /*
     * true:
     *   Menu cần đóng sau khi chọn.
     *
     * false:
     *   Menu tiếp tục hiển thị.
     */
    bool closeMenu = false;

    /*
     * Mã màu RGB dùng cho status label.
     */
    std::uint32_t statusColor = 0xFFFFFF;
};