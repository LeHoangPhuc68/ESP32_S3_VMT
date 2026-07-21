#pragma once

#include "AppId.h"
#include "AppLaunchResult.h"

class AppManager final
{
public:
    /*
     * Callback do lớp giao diện đăng ký.
     *
     * Callback trả về true nếu screen/app tương ứng
     * đã được mở thành công.
     */
    using LaunchHandler =
        bool (*)(void *context, AppId appId);

    static bool begin();

    /*
     * Đăng ký bộ điều hướng app.
     *
     * AppManager không include UIManager nên Core
     * không phụ thuộc trực tiếp vào UI.
     */
    static void setLaunchHandler(
        LaunchHandler launchHandler,
        void *launchContext);

    /*
     * Yêu cầu khởi chạy ứng dụng.
     */
    static AppLaunchResult launch(
        AppId appId);

    static AppId activeApp();

    static bool isInitialized();

private:
    static bool initialized_;

    static AppId activeApp_;

    static LaunchHandler launchHandler_;
    static void *launchContext_;
};