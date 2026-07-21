#include "ScreenManager.h"

bool ScreenManager::initialized_ =
    false;

BaseScreen *ScreenManager::screens_[
    ScreenManager::ScreenCount] = {};

BaseScreen *ScreenManager::currentScreen_ =
    nullptr;

ScreenId ScreenManager::currentScreenId_ =
    ScreenId::Invalid;

bool ScreenManager::begin()
{
    if (initialized_)
    {
        return true;
    }

    for (std::uint8_t index = 0;
         index < ScreenCount;
         ++index)
    {
        screens_[index] = nullptr;
    }

    currentScreen_ =
        nullptr;

    currentScreenId_ =
        ScreenId::Invalid;

    initialized_ =
        true;

    return true;
}

bool ScreenManager::registerScreen(
    const ScreenId screenId,
    BaseScreen &screen)
{
    if (!initialized_)
    {
        if (!begin())
        {
            return false;
        }
    }

    if (!isValidScreenId(screenId))
    {
        return false;
    }

    const std::uint8_t index =
        static_cast<std::uint8_t>(
            screenId);

    screens_[index] =
        &screen;

    return true;
}

bool ScreenManager::show(
    const ScreenId screenId)
{
    if (!initialized_)
    {
        return false;
    }

    BaseScreen *screen =
        getScreen(screenId);

    if (screen == nullptr)
    {
        return false;
    }

    if (!show(*screen))
    {
        return false;
    }

    currentScreenId_ =
        screenId;

    return true;
}

bool ScreenManager::show(
    BaseScreen &screen)
{
    if (!initialized_)
    {
        return false;
    }

    /*
     * Screen đã hiện tại thì chỉ bảo đảm nó đang visible.
     */
    if (currentScreen_ == &screen)
    {
        currentScreen_->show();

        return true;
    }

    /*
     * Ẩn màn hình cũ.
     */
    if (currentScreen_ != nullptr)
    {
        currentScreen_->hide();
    }

    /*
     * Chuyển sang màn hình mới.
     */
    currentScreen_ =
        &screen;

    currentScreen_->show();

    /*
     * Khi gọi bằng BaseScreen trực tiếp thì thử tìm lại
     * ScreenId tương ứng trong registry.
     */
    currentScreenId_ =
        ScreenId::Invalid;

    for (std::uint8_t index = 0;
         index < ScreenCount;
         ++index)
    {
        if (screens_[index] == currentScreen_)
        {
            currentScreenId_ =
                static_cast<ScreenId>(
                    index);

            break;
        }
    }

    return true;
}

BaseScreen *ScreenManager::currentScreen()
{
    return currentScreen_;
}

ScreenId ScreenManager::currentScreenId()
{
    return currentScreenId_;
}

BaseScreen *ScreenManager::getScreen(
    const ScreenId screenId)
{
    if (
        !initialized_ ||
        !isValidScreenId(screenId))
    {
        return nullptr;
    }

    const std::uint8_t index =
        static_cast<std::uint8_t>(
            screenId);

    return screens_[index];
}

bool ScreenManager::isInitialized()
{
    return initialized_;
}

bool ScreenManager::isValidScreenId(
    const ScreenId screenId)
{
    const std::uint8_t value =
        static_cast<std::uint8_t>(
            screenId);

    return value < ScreenCount;
}