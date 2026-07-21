#pragma once

#include "../../Core/App/AppId.h"
#include "ScreenId.h"

/*
 * Liên kết một ứng dụng với màn hình chính của nó.
 *
 * AppId:
 *   Định danh ứng dụng ở tầng Core.
 *
 * ScreenId:
 *   Định danh màn hình ở tầng UI.
 */
struct AppScreenBinding
{
    AppId appId;
    ScreenId screenId;
};