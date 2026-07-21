#pragma once

#include <cstdint>

#include "../../Core/App/AppId.h"
#include "AppScreenBinding.h"
#include "ScreenId.h"

namespace AppScreenRegistry
{
    /*
     * Danh sách ánh xạ giữa AppId và ScreenId.
     *
     * Chỉ những ứng dụng đã có screen thật mới xuất hiện
     * trong danh sách này.
     */
    extern const AppScreenBinding Bindings[];

    /*
     * Số lượng binding trong registry.
     */
    extern const std::uint8_t BindingCount;

    /*
     * Tìm ScreenId tương ứng với AppId.
     *
     * Trả về true:
     *   Tìm thấy screen.
     *
     * Trả về false:
     *   App chưa có screen hoặc AppId không hợp lệ.
     */
    bool findScreen(
        AppId appId,
        ScreenId &screenId);

    /*
     * Kiểm tra một app đã có screen được đăng ký chưa.
     */
    bool hasScreen(
        AppId appId);
}