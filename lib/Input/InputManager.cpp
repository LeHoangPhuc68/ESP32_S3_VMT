#include "InputManager.h"

#include "Button.h"

namespace
{
    /*
     * Giữ cả hai nút 600 ms để về Home.
     *
     * Không đặt quá ngắn vì người dùng có thể vô tình
     * nhấn chồng hai nút khi thao tác nhanh.
     */
    constexpr uint32_t HomeChordTimeMs = 600;

    bool bothButtonsTracking = false;
    bool homeActionTriggered = false;
    bool ignoreUntilReleased = false;

    uint32_t bothButtonsStartTime = 0;
}

bool InputManager::begin()
{
    bothButtonsTracking = false;
    homeActionTriggered = false;
    ignoreUntilReleased = false;
    bothButtonsStartTime = 0;

    return Button::begin();
}

InputManager::Action InputManager::update()
{
    Button::update();

    const bool leftPressed =
        Button::isPressed(Button::Id::Left);

    const bool rightPressed =
        Button::isPressed(Button::Id::Right);

    const bool bothPressed =
        leftPressed && rightPressed;

    /*
     * Sau khi tạo sự kiện Home, bỏ qua mọi event
     * cho đến khi cả hai nút được thả.
     */
    if (ignoreUntilReleased)
    {
        if (!leftPressed && !rightPressed)
        {
            ignoreUntilReleased = false;
            bothButtonsTracking = false;
            homeActionTriggered = false;
        }

        return Action::None;
    }

    if (bothPressed)
    {
        if (!bothButtonsTracking)
        {
            bothButtonsTracking = true;
            homeActionTriggered = false;
            bothButtonsStartTime = millis();
        }

        if (
            !homeActionTriggered &&
            millis() - bothButtonsStartTime >= HomeChordTimeMs)
        {
            homeActionTriggered = true;
            ignoreUntilReleased = true;

            return Action::Home;
        }

        return Action::None;
    }

    if (bothButtonsTracking)
    {
        bothButtonsTracking = false;
        homeActionTriggered = false;
    }

    /*
     * Giữ phải: Select
     */
    if (Button::wasLongPressed(Button::Id::Right))
    {
        return Action::Select;
    }

    /*
     * Giữ trái: Back
     */
    if (Button::wasLongPressed(Button::Id::Left))
    {
        return Action::Back;
    }

    /*
     * Click trái: mục trước
     */
    if (Button::wasClicked(Button::Id::Left))
    {
        return Action::Previous;
    }

    /*
     * Click phải: mục tiếp theo
     */
    if (Button::wasClicked(Button::Id::Right))
    {
        return Action::Next;
    }

    return Action::None;
}
