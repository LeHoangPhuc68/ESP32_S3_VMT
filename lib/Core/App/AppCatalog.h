#pragma once

#include <cstdint>

#include "AppDefinition.h"

namespace AppCatalog
{
    /*
     * Danh sách toàn bộ ứng dụng được hệ thống biết đến.
     */
    extern const AppDefinition Definitions[];

    /*
     * Số lượng ứng dụng hợp lệ trong catalog.
     */
    extern const std::uint8_t DefinitionCount;

    /*
     * Tìm metadata theo AppId.
     *
     * Trả về nullptr nếu AppId không tồn tại.
     */
    const AppDefinition *find(
        AppId appId);

    /*
     * Kiểm tra một app đã được triển khai chưa.
     */
    bool isImplemented(
        AppId appId);

    /*
     * Lấy tên ứng dụng.
     *
     * Trả về nullptr nếu AppId không hợp lệ.
     */
    const char *nameOf(
        AppId appId);
}