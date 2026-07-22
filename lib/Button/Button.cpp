#include "Button.h"

namespace
{
    constexpr uint8_t PinLeft = 0;
    constexpr uint8_t PinRight = 14;

    /* Centralized, non-blocking physical-button timing. */
    constexpr uint32_t DebounceTimeMs = 25;

    /*
     * A long press is emitted once after 200 ms. The release path suppresses
     * the short-click event after this threshold has been reached.
     */
    constexpr uint32_t LongPressTimeMs = 225;

    struct ButtonState
    {
        uint8_t pin;

        bool stablePressed;
        bool lastRawPressed;

        bool pressedEvent;
        bool releasedEvent;
        bool clickedEvent;
        bool longPressedEvent;

        bool longPressTriggered;

        uint32_t rawChangeTime;
        uint32_t pressStartTime;
    };

    ButtonState leftButton{
        PinLeft,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        0,
        0};

    ButtonState rightButton{
        PinRight,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        0,
        0};

    ButtonState &getButtonState(const Button::Id id)
    {
        return id == Button::Id::Left
                   ? leftButton
                   : rightButton;
    }

    bool readPressed(const uint8_t pin)
    {
        return digitalRead(pin) == LOW;
    }

    void clearEvents(ButtonState &button)
    {
        button.pressedEvent = false;
        button.releasedEvent = false;
        button.clickedEvent = false;
        button.longPressedEvent = false;
    }

    void initializeButton(ButtonState &button)
    {
        pinMode(button.pin, INPUT_PULLUP);

        const bool pressed =
            readPressed(button.pin);

        button.stablePressed = pressed;
        button.lastRawPressed = pressed;

        button.rawChangeTime = millis();
        button.pressStartTime = millis();

        button.longPressTriggered = false;

        clearEvents(button);
    }

    void updateButton(ButtonState &button)
    {
        clearEvents(button);

        const uint32_t now = millis();

        const bool rawPressed =
            readPressed(button.pin);

        if (rawPressed != button.lastRawPressed)
        {
            button.lastRawPressed = rawPressed;
            button.rawChangeTime = now;
        }

        const bool debounceFinished =
            now - button.rawChangeTime >= DebounceTimeMs;

        if (
            debounceFinished &&
            rawPressed != button.stablePressed)
        {
            button.stablePressed = rawPressed;

            if (button.stablePressed)
            {
                button.pressedEvent = true;
                button.pressStartTime = now;
                button.longPressTriggered = false;
            }
            else
            {
                button.releasedEvent = true;

                /*
                 * Chỉ tạo click nếu trước đó chưa kích hoạt
                 * long press.
                 */
                if (!button.longPressTriggered)
                {
                    button.clickedEvent = true;
                }

                button.longPressTriggered = false;
            }
        }

        if (
            button.stablePressed &&
            !button.longPressTriggered &&
            now - button.pressStartTime >= LongPressTimeMs)
        {
            button.longPressTriggered = true;
            button.longPressedEvent = true;
        }
    }
}

bool Button::begin()
{
    initializeButton(leftButton);
    initializeButton(rightButton);

    return true;
}

void Button::update()
{
    updateButton(leftButton);
    updateButton(rightButton);
}

bool Button::isPressed(const Id id)
{
    return getButtonState(id).stablePressed;
}

bool Button::wasPressed(const Id id)
{
    return getButtonState(id).pressedEvent;
}

bool Button::wasReleased(const Id id)
{
    return getButtonState(id).releasedEvent;
}

bool Button::wasClicked(const Id id)
{
    return getButtonState(id).clickedEvent;
}

bool Button::wasLongPressed(const Id id)
{
    return getButtonState(id).longPressedEvent;
}
