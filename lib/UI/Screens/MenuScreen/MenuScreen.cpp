#include "MenuScreen.h"

namespace
{
    constexpr std::uint32_t ColorTextPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorTextSecondary =
        0x9AA4B2;

    constexpr std::uint32_t ColorAccent =
        0x67E8F9;

    constexpr std::uint32_t ColorSurface =
        0x080B10;
}

bool MenuScreen::create(
    lv_obj_t *parent,
    const MenuItem *items,
    const std::uint8_t itemCount,
    const SelectionCallback selectionCallback,
    void *callbackContext)
{
    if (
        parent == nullptr ||
        items == nullptr ||
        itemCount == 0)
    {
        return false;
    }

    /*
     * Nếu menu đã được tạo thì không tạo lại object LVGL.
     */
    if (
        menuCard_ != nullptr &&
        statusLabel_ != nullptr)
    {
        return true;
    }

    items_ = items;
    itemCount_ = itemCount;

    selectionCallback_ =
        selectionCallback;

    callbackContext_ =
        callbackContext;

    selectedMenuIndex_ = 0;
    visible_ = false;

    createStatusLabel(parent);
    createMenuCard(parent);

    return
        menuCard_ != nullptr &&
        menuItemLabel_ != nullptr &&
        menuPositionLabel_ != nullptr &&
        menuControlLabel_ != nullptr &&
        statusLabel_ != nullptr;
}

void MenuScreen::open()
{
    if (
        menuCard_ == nullptr ||
        items_ == nullptr ||
        itemCount_ == 0)
    {
        return;
    }

    visible_ = true;

    lv_obj_remove_flag(
        menuCard_,
        LV_OBJ_FLAG_HIDDEN);

    updateMenuDisplay();
}

void MenuScreen::close()
{
    if (menuCard_ == nullptr)
    {
        return;
    }

    visible_ = false;

    lv_obj_add_flag(
        menuCard_,
        LV_OBJ_FLAG_HIDDEN);

    hideStatus();
}

bool MenuScreen::isVisible() const
{
    return visible_;
}

void MenuScreen::handleInput(
    const InputManager::Action action)
{
    if (
        action == InputManager::Action::None ||
        !visible_ ||
        items_ == nullptr ||
        itemCount_ == 0)
    {
        return;
    }

    switch (action)
    {
    case InputManager::Action::Previous:
    {
        if (selectedMenuIndex_ == 0)
        {
            selectedMenuIndex_ =
                itemCount_ - 1;
        }
        else
        {
            --selectedMenuIndex_;
        }

        updateMenuDisplay();
        break;
    }

    case InputManager::Action::Next:
    {
        selectedMenuIndex_ =
            static_cast<std::uint8_t>(
                (selectedMenuIndex_ + 1) %
                itemCount_);

        updateMenuDisplay();
        break;
    }

    case InputManager::Action::Select:
    {
        selectCurrentMenuItem();
        break;
    }

    case InputManager::Action::Back:
    case InputManager::Action::Home:
    {
        close();
        break;
    }

    case InputManager::Action::None:
    default:
    {
        break;
    }
    }
}

void MenuScreen::showStatus(
    const char *text,
    const std::uint32_t color)
{
    if (
        statusLabel_ == nullptr ||
        text == nullptr)
    {
        return;
    }

    lv_label_set_text(
        statusLabel_,
        text);

    lv_obj_set_style_text_color(
        statusLabel_,
        lv_color_hex(color),
        LV_PART_MAIN);

    lv_obj_remove_flag(
        statusLabel_,
        LV_OBJ_FLAG_HIDDEN);
}

void MenuScreen::hideStatus()
{
    if (statusLabel_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        statusLabel_,
        LV_OBJ_FLAG_HIDDEN);
}

std::uint8_t MenuScreen::selectedIndex() const
{
    return selectedMenuIndex_;
}

void MenuScreen::createStatusLabel(
    lv_obj_t *parent)
{
    if (parent == nullptr)
    {
        return;
    }

    statusLabel_ =
        lv_label_create(parent);

    if (statusLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        statusLabel_,
        "");

    lv_obj_set_style_text_font(
        statusLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        statusLabel_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        statusLabel_,
        LV_OPA_70,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        statusLabel_,
        6,
        LV_PART_MAIN);

    lv_obj_set_style_pad_left(
        statusLabel_,
        7,
        LV_PART_MAIN);

    lv_obj_set_style_pad_right(
        statusLabel_,
        7,
        LV_PART_MAIN);

    lv_obj_set_style_pad_top(
        statusLabel_,
        3,
        LV_PART_MAIN);

    lv_obj_set_style_pad_bottom(
        statusLabel_,
        3,
        LV_PART_MAIN);

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -7,
        7);

    hideStatus();
}

void MenuScreen::createMenuCard(
    lv_obj_t *parent)
{
    if (
        parent == nullptr ||
        items_ == nullptr ||
        itemCount_ == 0)
    {
        return;
    }

    menuCard_ =
        lv_obj_create(parent);

    if (menuCard_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        menuCard_,
        286,
        74);

    lv_obj_align(
        menuCard_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -8);

    lv_obj_clear_flag(
        menuCard_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(
        menuCard_,
        16,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        menuCard_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        menuCard_,
        LV_OPA_80,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        menuCard_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_set_style_border_opa(
        menuCard_,
        LV_OPA_50,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        menuCard_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        menuCard_,
        0,
        LV_PART_MAIN);

    /*
     * Tên mục đang chọn.
     */
    menuItemLabel_ =
        lv_label_create(menuCard_);

    if (menuItemLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        menuItemLabel_,
        items_[selectedMenuIndex_].title);

    lv_obj_set_style_text_font(
        menuItemLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        menuItemLabel_,
        lv_color_hex(ColorTextPrimary),
        LV_PART_MAIN);

    lv_obj_align(
        menuItemLabel_,
        LV_ALIGN_TOP_LEFT,
        15,
        11);

    /*
     * Số thứ tự mục hiện tại.
     */
    menuPositionLabel_ =
        lv_label_create(menuCard_);

    if (menuPositionLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text_fmt(
        menuPositionLabel_,
        "%u  /  %u",
        static_cast<unsigned int>(
            selectedMenuIndex_ + 1),
        static_cast<unsigned int>(
            itemCount_));

    lv_obj_set_style_text_font(
        menuPositionLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        menuPositionLabel_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_align(
        menuPositionLabel_,
        LV_ALIGN_TOP_RIGHT,
        -14,
        13);

    /*
     * Chú thích điều khiển.
     */
    menuControlLabel_ =
        lv_label_create(menuCard_);

    if (menuControlLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        menuControlLabel_,
        "LEFT / RIGHT     HOLD: SELECT");

    lv_obj_set_style_text_font(
        menuControlLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        menuControlLabel_,
        lv_color_hex(ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        menuControlLabel_,
        LV_ALIGN_BOTTOM_LEFT,
        15,
        -10);

    lv_obj_add_flag(
        menuCard_,
        LV_OBJ_FLAG_HIDDEN);
}

void MenuScreen::updateMenuDisplay()
{
    if (
        items_ == nullptr ||
        itemCount_ == 0 ||
        menuItemLabel_ == nullptr ||
        menuPositionLabel_ == nullptr)
    {
        return;
    }

    /*
     * Bảo vệ index trong trường hợp dữ liệu menu
     * thay đổi về sau.
     */
    if (selectedMenuIndex_ >= itemCount_)
    {
        selectedMenuIndex_ = 0;
    }

    lv_label_set_text(
        menuItemLabel_,
        items_[selectedMenuIndex_].title);

    lv_label_set_text_fmt(
        menuPositionLabel_,
        "%u  /  %u",
        static_cast<unsigned int>(
            selectedMenuIndex_ + 1),
        static_cast<unsigned int>(
            itemCount_));

    /*
     * Giữ nguyên hành vi cũ:
     * chuyển mục sẽ ẩn thông báo trước đó.
     */
    hideStatus();
}

void MenuScreen::selectCurrentMenuItem()
{
    if (
        items_ == nullptr ||
        itemCount_ == 0 ||
        selectedMenuIndex_ >= itemCount_)
    {
        return;
    }

    /*
     * MenuScreen không còn biết Dashboard,
     * Wi-Fi, Bluetooth hay "soon".
     *
     * Nó chỉ báo mục được chọn cho lớp bên ngoài.
     */
    if (selectionCallback_ != nullptr)
    {
        selectionCallback_(
            callbackContext_,
            selectedMenuIndex_,
            items_[selectedMenuIndex_]);
    }
}