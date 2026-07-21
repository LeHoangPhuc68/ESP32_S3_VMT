#include "AppManager.h"

#include "AppCatalog.h"

namespace
{
    constexpr std::uint32_t ColorSuccess =
        0x86EFAC;

    constexpr std::uint32_t ColorWarning =
        0xFCD34D;

    constexpr std::uint32_t ColorError =
        0xFCA5A5;
}

bool AppManager::initialized_ =
    false;

AppId AppManager::activeApp_ =
    AppId::Invalid;

AppManager::LaunchHandler
    AppManager::launchHandler_ =
        nullptr;

void *AppManager::launchContext_ =
    nullptr;

bool AppManager::begin()
{
    if (initialized_)
    {
        return true;
    }

    activeApp_ =
        AppId::Invalid;

    initialized_ =
        true;

    return true;
}

void AppManager::setLaunchHandler(
    const LaunchHandler launchHandler,
    void *launchContext)
{
    launchHandler_ =
        launchHandler;

    launchContext_ =
        launchContext;
}

AppLaunchResult AppManager::launch(
    const AppId appId)
{
    if (!initialized_)
    {
        if (!begin())
        {
            return {
                AppLaunchStatus::InvalidApp,
                "App manager error",
                false,
                ColorError
            };
        }
    }

    /*
     * AppManager không còn switch từng AppId.
     * Toàn bộ metadata được lấy từ AppCatalog.
     */
    const AppDefinition *definition =
        AppCatalog::find(appId);

    if (definition == nullptr)
    {
        return {
            AppLaunchStatus::InvalidApp,
            "Invalid app",
            false,
            ColorError
        };
    }

    /*
     * App chưa có screen thật.
     */
    if (!definition->implemented)
    {
        return {
            AppLaunchStatus::NotImplemented,
            definition->unavailableMessage,
            false,
            ColorWarning
        };
    }

    /*
     * App đã triển khai nhưng UI chưa đăng ký
     * launch handler.
     */
    if (launchHandler_ == nullptr)
    {
        return {
            AppLaunchStatus::InvalidApp,
            "Navigation error",
            false,
            ColorError
        };
    }

    /*
     * Yêu cầu lớp UI mở screen tương ứng.
     */
    if (!launchHandler_(
            launchContext_,
            appId))
    {
        return {
            AppLaunchStatus::InvalidApp,
            "App launch error",
            false,
            ColorError
        };
    }

    activeApp_ =
        appId;

    return {
        AppLaunchStatus::Launched,
        nullptr,
        definition->closeMenuOnLaunch,
        ColorSuccess
    };
}

AppId AppManager::activeApp()
{
    return activeApp_;
}

bool AppManager::isInitialized()
{
    return initialized_;
}