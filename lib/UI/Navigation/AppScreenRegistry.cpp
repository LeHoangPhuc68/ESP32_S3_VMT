#include "AppScreenRegistry.h"

namespace AppScreenRegistry
{
    const AppScreenBinding Bindings[] = {
        {
            AppId::Dashboard,
            ScreenId::Dashboard
        },
        {
            AppId::WiFiTools,
            ScreenId::WiFiScanner
        }
    };

    const std::uint8_t BindingCount =
        static_cast<std::uint8_t>(
            sizeof(Bindings) /
            sizeof(Bindings[0]));

    bool findScreen(
        const AppId appId,
        ScreenId &screenId)
    {
        for (std::uint8_t index = 0;
             index < BindingCount;
             ++index)
        {
            if (Bindings[index].appId == appId)
            {
                screenId =
                    Bindings[index].screenId;

                return true;
            }
        }

        screenId =
            ScreenId::Invalid;

        return false;
    }

    bool hasScreen(
        const AppId appId)
    {
        ScreenId screenId =
            ScreenId::Invalid;

        return findScreen(
            appId,
            screenId);
    }
}