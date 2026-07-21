#pragma once

#include "../App/AppId.h"

/*
 * Dữ liệu của một mục menu.
 *
 * title:
 *   Nội dung hiển thị.
 *
 * appId:
 *   Ứng dụng được yêu cầu khi người dùng chọn mục.
 */
struct MenuItem
{
    const char *title;
    AppId appId;
};