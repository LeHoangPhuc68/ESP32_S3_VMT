#pragma once

#include <Arduino.h>

namespace InputManager
{
    enum class Action : uint8_t
    {
        None = 0,
        Next,
        Select,
        Primary,
        Back
    };

    bool begin();

    Action update();
}
