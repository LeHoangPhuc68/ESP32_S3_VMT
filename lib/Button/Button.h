#pragma once

#include <Arduino.h>

namespace Button
{
    enum class Id : uint8_t
    {
        Left = 0,
        Right
    };

    bool begin();

    void update();

    bool isPressed(Id id);

    bool wasPressed(Id id);

    bool wasReleased(Id id);

    bool wasClicked(Id id);

    bool wasLongPressed(Id id);
}
