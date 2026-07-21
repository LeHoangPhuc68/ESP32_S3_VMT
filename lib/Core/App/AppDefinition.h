#pragma once

#include "AppId.h"

/*
 * Metadata mô tả một ứng dụng trong hệ thống.
 *
 * AppDefinition không chứa code giao diện và cũng
 * không trực tiếp khởi chạy ứng dụng.
 */
struct AppDefinition
{
    /*
     * Định danh duy nhất của ứng dụng.
     */
    AppId id;

    /*
     * Tên hiển thị và phục vụ debug.
     */
    const char *name;

    /*
     * true:
     *   Ứng dụng đã có screen và có thể khởi chạy.
     *
     * false:
     *   Ứng dụng mới chỉ là placeholder.
     */
    bool implemented;

    /*
     * Có đóng menu sau khi khởi chạy thành công không.
     */
    bool closeMenuOnLaunch;

    /*
     * Thông báo hiển thị khi ứng dụng chưa được
     * triển khai.
     */
    const char *unavailableMessage;
};