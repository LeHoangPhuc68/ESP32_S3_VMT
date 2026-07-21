#include "AppCatalog.h"

namespace AppCatalog
{
    const AppDefinition Definitions[] = {
        {
            AppId::Dashboard,
            "Dashboard",
            true,
            true,
            nullptr
        },
        {
            AppId::WiFiTools,
            "Wi-Fi Tools",
            true,
            true,
            nullptr
        },
        {
            AppId::Bluetooth,
            "Bluetooth",
            false,
            false,
            "Bluetooth  soon"
        },
        {
            AppId::USBTools,
            "USB Tools",
            false,
            false,
            "USB Tools  soon"
        },
        {
            AppId::System,
            "System",
            false,
            false,
            "System  soon"
        },
        {
            AppId::Settings,
            "Settings",
            false,
            false,
            "Settings  soon"
        }
    };

    const std::uint8_t DefinitionCount =
        static_cast<std::uint8_t>(
            sizeof(Definitions) /
            sizeof(Definitions[0]));

    const AppDefinition *find(
        const AppId appId)
    {
        for (std::uint8_t index = 0;
             index < DefinitionCount;
             ++index)
        {
            if (Definitions[index].id == appId)
            {
                return &Definitions[index];
            }
        }

        return nullptr;
    }

    bool isImplemented(
        const AppId appId)
    {
        const AppDefinition *definition =
            find(appId);

        return
            definition != nullptr &&
            definition->implemented;
    }

    const char *nameOf(
        const AppId appId)
    {
        const AppDefinition *definition =
            find(appId);

        if (definition == nullptr)
        {
            return nullptr;
        }

        return definition->name;
    }
}