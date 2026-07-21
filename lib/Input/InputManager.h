#pragma once

#include <Arduino.h>

namespace InputManager
{
    enum class Action : uint8_t
    {
        None = 0,
        Previous,
        Next,
        Select,
        Back,
        Home
    };

    bool begin();

    Action update();
}
