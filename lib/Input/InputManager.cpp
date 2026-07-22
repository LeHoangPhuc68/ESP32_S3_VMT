#include "InputManager.h"

#include "Button.h"

bool InputManager::begin()
{
    return Button::begin();
}

InputManager::Action InputManager::update()
{
    Button::update();

    /*
     * GPIO14 / KEY long press: select or confirm.
     */
    if (Button::wasLongPressed(Button::Id::Right))
    {
        return Action::Select;
    }

    /*
     * GPIO0 / BOOT long press: navigate back one level.
     */
    if (Button::wasLongPressed(Button::Id::Left))
    {
        return Action::Back;
    }

    /*
     * GPIO14 / KEY short press: advance focus.
     */
    if (Button::wasClicked(Button::Id::Right))
    {
        return Action::Next;
    }

    /*
     * GPIO0 / BOOT short press: contextual primary action.
     */
    if (Button::wasClicked(Button::Id::Left))
    {
        return Action::Primary;
    }

    return Action::None;
}
