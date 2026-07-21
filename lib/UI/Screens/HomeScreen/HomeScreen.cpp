#include "HomeScreen.h"

#include "../../../Core/App/AppManager.h"
#include "../../../Core/Menu/MainMenu.h"
#include "wallpaper.h"

namespace
{
    constexpr std::uint32_t ColorTextPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorSurface =
        0x080B10;
}

bool HomeScreen::create(lv_obj_t *parent)
{
    if (parent == nullptr)
    {
        return false;
    }

    /*
     * Không tạo lại screen nếu đã khởi tạo.
     */
    if (root_ != nullptr)
    {
        return true;
    }

    root_ =
        lv_obj_create(parent);

    if (root_ == nullptr)
    {
        return false;
    }

    lv_obj_set_size(
        root_,
        LV_PCT(100),
        LV_PCT(100));

    lv_obj_align(
        root_,
        LV_ALIGN_CENTER,
        0,
        0);

    lv_obj_clear_flag(
        root_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        root_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        root_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        root_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        root_,
        lv_color_hex(0x000000),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    createWallpaper();
    createBrandLabel();

    if (!menuScreen_.create(
            root_,
            MainMenu::Items,
            MainMenu::ItemCount,
            &HomeScreen::handleMenuSelection,
            this))
    {
        return false;
    }

    return
        wallpaperImage_ != nullptr &&
        brandLabel_ != nullptr;
}

void HomeScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void HomeScreen::hide()
{
    if (root_ == nullptr)
    {
        return;
    }

    menuScreen_.close();

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void HomeScreen::update()
{
    /*
     * HomeScreen hiện chưa có tác vụ định kỳ.
     */
}

void HomeScreen::handleInput(
    const InputManager::Action action)
{
    if (action == InputManager::Action::None)
    {
        return;
    }

    /*
     * Home luôn đóng menu.
     */
    if (action == InputManager::Action::Home)
    {
        menuScreen_.close();
        return;
    }

    /*
     * Giữ nguyên hành vi:
     *
     * Khi menu chưa mở:
     * - Next mở menu;
     * - Select mở menu.
     */
    if (!menuScreen_.isVisible())
    {
        if (
            action == InputManager::Action::Next ||
            action == InputManager::Action::Select)
        {
            menuScreen_.open();
        }

        return;
    }

    /*
     * Khi menu đang mở, MenuScreen xử lý điều hướng.
     */
    menuScreen_.handleInput(action);
}

const char *HomeScreen::name() const
{
    return "Home";
}

void HomeScreen::createWallpaper()
{
    if (root_ == nullptr)
    {
        return;
    }

    wallpaperImage_ =
        lv_image_create(root_);

    if (wallpaperImage_ == nullptr)
    {
        return;
    }

    lv_image_set_src(
        wallpaperImage_,
        &wallpaper);

    lv_obj_align(
        wallpaperImage_,
        LV_ALIGN_CENTER,
        0,
        0);

    lv_obj_clear_flag(
        wallpaperImage_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_clear_flag(
        wallpaperImage_,
        LV_OBJ_FLAG_CLICKABLE);

    lv_obj_move_background(
        wallpaperImage_);
}

void HomeScreen::createBrandLabel()
{
    if (root_ == nullptr)
    {
        return;
    }

    brandLabel_ =
        lv_label_create(root_);

    if (brandLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        brandLabel_,
        "VMT OS");

    lv_obj_set_style_text_font(
        brandLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        brandLabel_,
        lv_color_hex(ColorTextPrimary),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        brandLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        brandLabel_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        brandLabel_,
        LV_OPA_60,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        brandLabel_,
        6,
        LV_PART_MAIN);

    lv_obj_set_style_pad_left(
        brandLabel_,
        7,
        LV_PART_MAIN);

    lv_obj_set_style_pad_right(
        brandLabel_,
        7,
        LV_PART_MAIN);

    lv_obj_set_style_pad_top(
        brandLabel_,
        3,
        LV_PART_MAIN);

    lv_obj_set_style_pad_bottom(
        brandLabel_,
        3,
        LV_PART_MAIN);

    lv_obj_align(
        brandLabel_,
        LV_ALIGN_TOP_LEFT,
        7,
        7);
}

void HomeScreen::handleMenuSelection(
    void *context,
    const std::uint8_t index,
    const MenuItem &item)
{
    if (context == nullptr)
    {
        return;
    }

    HomeScreen *homeScreen =
        static_cast<HomeScreen *>(context);

    homeScreen->onMenuItemSelected(
        index,
        item);
}

void HomeScreen::onMenuItemSelected(
    const std::uint8_t index,
    const MenuItem &item)
{
    /*
     * Index hiện không còn quyết định app nào được mở.
     * Giữ tham số để callback MenuScreen vẫn tổng quát.
     */
    (void)index;

    const AppLaunchResult result =
        AppManager::launch(item.appId);

    if (result.closeMenu)
    {
        /*
         * close() cũng ẩn status cũ trước khi status
         * mới được hiển thị.
         */
        menuScreen_.close();
    }

    if (result.message != nullptr)
    {
        menuScreen_.showStatus(
            result.message,
            result.statusColor);
    }
}