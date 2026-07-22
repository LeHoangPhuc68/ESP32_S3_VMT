#pragma once

#include <cstdint>

#include <lvgl.h>

#include "../BaseScreen.h"
#include "../MenuScreen/MenuScreen.h"
#include "../../../Core/Menu/MenuItem.h"

class HomeScreen final : public BaseScreen
{
public:
    HomeScreen() = default;

    bool create(lv_obj_t *parent) override;

    void show() override;
    void hide() override;
    void update() override;

    void handleInput(
        InputManager::Action action) override;

    const char *name() const override;

    void showMainMenu();

private:
    void createWallpaper();
    void createBrandLabel();

    /*
     * Callback static nhận sự kiện từ MenuScreen.
     */
    static void handleMenuSelection(
        void *context,
        std::uint8_t index,
        const MenuItem &item);

    /*
     * Xử lý yêu cầu mở ứng dụng thông qua AppManager.
     */
    void onMenuItemSelected(
        std::uint8_t index,
        const MenuItem &item);

    lv_obj_t *root_ = nullptr;
    lv_obj_t *wallpaperImage_ = nullptr;
    lv_obj_t *brandLabel_ = nullptr;

    MenuScreen menuScreen_;
};
