#include "MainMenu.h"

namespace MainMenu
{
    const MenuItem Items[] = {
        {
            "Dashboard",
            AppId::Dashboard
        },
        {
            "Wi-Fi Tools",
            AppId::WiFiTools
        },
        {
            "Bluetooth",
            AppId::Bluetooth
        },
        {
            "USB Tools",
            AppId::USBTools
        },
        {
            "System",
            AppId::System
        },
        {
            "Settings",
            AppId::Settings
        }};

    const std::uint8_t ItemCount =
        static_cast<std::uint8_t>(
            sizeof(Items) /
            sizeof(Items[0]));
}