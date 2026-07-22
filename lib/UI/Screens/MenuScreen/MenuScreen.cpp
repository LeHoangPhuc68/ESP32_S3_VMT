#include "MenuScreen.h"

#include "../../../Core/App/AppCatalog.h"

namespace
{
    constexpr std::uint32_t ColorTextPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorTextSecondary =
        0x9AA4B2;

    constexpr std::uint32_t ColorAccent =
        0x67E8F9;

    constexpr std::uint32_t ColorWarning =
        0xFCD34D;

    constexpr std::uint32_t ColorSurface =
        0x080B10;

    constexpr std::uint32_t ColorTile =
        0x111722;

    constexpr std::uint32_t ColorTileFocused =
        0x183447;

    constexpr std::uint32_t ColorBorder =
        0x263244;

    constexpr std::int32_t TileWidth = 96;
    constexpr std::int32_t TileHeight = 58;
    constexpr std::int32_t TileStartX = 10;
    constexpr std::int32_t TileStartY = 24;
    constexpr std::int32_t TileGapX = 6;
    constexpr std::int32_t TileGapY = 5;
    constexpr std::uint8_t TileColumns = 3;
}

bool MenuScreen::create(
    lv_obj_t *parent,
    const MenuItem *items,
    const std::uint8_t itemCount,
    const char *title,
    const SelectionCallback selectionCallback,
    void *callbackContext)
{
    if (
        parent == nullptr ||
        items == nullptr ||
        itemCount == 0 ||
        itemCount > MaximumTileCount ||
        title == nullptr)
    {
        return false;
    }

    if (menuCard_ != nullptr)
    {
        return true;
    }

    items_ = items;
    itemCount_ = itemCount;
    title_ = title;
    selectionCallback_ = selectionCallback;
    callbackContext_ = callbackContext;
    selectedMenuIndex_ = 0;
    visible_ = false;

    createMenuCard(parent);

    if (menuCard_ == nullptr)
    {
        return false;
    }

    createHeader();
    createTiles();
    createControlLabel();

    if (
        titleLabel_ == nullptr ||
        statusLabel_ == nullptr ||
        controlLabel_ == nullptr)
    {
        return false;
    }

    for (std::uint8_t index = 0;
         index < itemCount_;
         ++index)
    {
        if (
            tileContainers_[index] == nullptr ||
            tileIconLabels_[index] == nullptr ||
            tileTitleLabels_[index] == nullptr ||
            tileStateLabels_[index] == nullptr)
        {
            return false;
        }
    }

    close();

    return true;
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
    case InputManager::Action::Primary:
    {
        selectCurrentMenuItem();
        break;
    }

    case InputManager::Action::Back:
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

    lv_obj_move_foreground(
        statusLabel_);
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

void MenuScreen::createMenuCard(
    lv_obj_t *parent)
{
    menuCard_ =
        lv_obj_create(parent);

    if (menuCard_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        menuCard_,
        LV_PCT(100),
        LV_PCT(100));

    lv_obj_align(
        menuCard_,
        LV_ALIGN_CENTER,
        0,
        0);

    lv_obj_clear_flag(
        menuCard_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_radius(
        menuCard_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        menuCard_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        menuCard_,
        LV_OPA_90,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        menuCard_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        menuCard_,
        0,
        LV_PART_MAIN);
}

void MenuScreen::createHeader()
{
    if (menuCard_ == nullptr)
    {
        return;
    }

    titleLabel_ =
        lv_label_create(menuCard_);

    if (titleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        titleLabel_,
        title_);

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(ColorTextPrimary),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        titleLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_align(
        titleLabel_,
        LV_ALIGN_TOP_LEFT,
        10,
        5);

    statusLabel_ =
        lv_label_create(menuCard_);

    if (statusLabel_ == nullptr)
    {
        return;
    }

    lv_obj_set_width(
        statusLabel_,
        190);

    lv_label_set_long_mode(
        statusLabel_,
        LV_LABEL_LONG_DOT);

    lv_obj_set_style_text_align(
        statusLabel_,
        LV_TEXT_ALIGN_RIGHT,
        LV_PART_MAIN);

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
        LV_OPA_COVER,
        LV_PART_MAIN);

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -10,
        5);

    hideStatus();
}

void MenuScreen::createTiles()
{
    if (
        menuCard_ == nullptr ||
        items_ == nullptr)
    {
        return;
    }

    for (std::uint8_t index = 0;
         index < itemCount_;
         ++index)
    {
        const std::int32_t column =
            index % TileColumns;

        const std::int32_t row =
            index / TileColumns;

        lv_obj_t *tile =
            lv_obj_create(menuCard_);

        tileContainers_[index] =
            tile;

        if (tile == nullptr)
        {
            return;
        }

        lv_obj_set_size(
            tile,
            TileWidth,
            TileHeight);

        lv_obj_set_pos(
            tile,
            TileStartX +
                column *
                    (TileWidth + TileGapX),
            TileStartY +
                row *
                    (TileHeight + TileGapY));

        lv_obj_clear_flag(
            tile,
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            tile,
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            tile,
            8,
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            tile,
            LV_OPA_COVER,
            LV_PART_MAIN);

        tileIconLabels_[index] =
            lv_label_create(tile);

        tileTitleLabels_[index] =
            lv_label_create(tile);

        tileStateLabels_[index] =
            lv_label_create(tile);

        if (
            tileIconLabels_[index] == nullptr ||
            tileTitleLabels_[index] == nullptr ||
            tileStateLabels_[index] == nullptr)
        {
            return;
        }

        lv_label_set_text(
            tileIconLabels_[index],
            iconForApp(
                items_[index].appId));

        lv_obj_set_style_text_font(
            tileIconLabels_[index],
            &lv_font_montserrat_16,
            LV_PART_MAIN);

        lv_obj_align(
            tileIconLabels_[index],
            LV_ALIGN_TOP_MID,
            0,
            3);

        lv_obj_set_width(
            tileTitleLabels_[index],
            TileWidth - 6);

        lv_label_set_long_mode(
            tileTitleLabels_[index],
            LV_LABEL_LONG_DOT);

        lv_label_set_text(
            tileTitleLabels_[index],
            items_[index].title);

        lv_obj_set_style_text_align(
            tileTitleLabels_[index],
            LV_TEXT_ALIGN_CENTER,
            LV_PART_MAIN);

        lv_obj_set_style_text_font(
            tileTitleLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_align(
            tileTitleLabels_[index],
            LV_ALIGN_TOP_MID,
            0,
            23);

        lv_label_set_text(
            tileStateLabels_[index],
            isItemImplemented(index)
                ? ""
                : "SOON");

        lv_obj_set_style_text_font(
            tileStateLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            tileStateLabels_[index],
            lv_color_hex(ColorWarning),
            LV_PART_MAIN);

        lv_obj_align(
            tileStateLabels_[index],
            LV_ALIGN_BOTTOM_MID,
            0,
            -3);
    }
}

void MenuScreen::createControlLabel()
{
    if (menuCard_ == nullptr)
    {
        return;
    }

    controlLabel_ =
        lv_label_create(menuCard_);

    if (controlLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        controlLabel_,
        "KEY NEXT/HOLD OPEN   BOOT OPEN/HOLD BACK");

    lv_obj_set_style_text_font(
        controlLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        controlLabel_,
        lv_color_hex(ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        controlLabel_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -2);
}

void MenuScreen::updateMenuDisplay()
{
    if (
        items_ == nullptr ||
        itemCount_ == 0)
    {
        return;
    }

    if (selectedMenuIndex_ >= itemCount_)
    {
        selectedMenuIndex_ = 0;
    }

    for (std::uint8_t index = 0;
         index < itemCount_;
         ++index)
    {
        const bool focused =
            index == selectedMenuIndex_;

        const bool implemented =
            isItemImplemented(index);

        lv_obj_set_style_bg_color(
            tileContainers_[index],
            lv_color_hex(
                focused
                    ? ColorTileFocused
                    : ColorTile),
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            tileContainers_[index],
            lv_color_hex(
                focused
                    ? (implemented
                           ? ColorAccent
                           : ColorWarning)
                    : ColorBorder),
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            tileContainers_[index],
            focused
                ? 2
                : 1,
            LV_PART_MAIN);

        const std::uint32_t contentColor =
            implemented || focused
                ? ColorTextPrimary
                : ColorTextSecondary;

        lv_obj_set_style_text_color(
            tileIconLabels_[index],
            lv_color_hex(contentColor),
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            tileTitleLabels_[index],
            lv_color_hex(contentColor),
            LV_PART_MAIN);
    }

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

    if (selectionCallback_ != nullptr)
    {
        selectionCallback_(
            callbackContext_,
            selectedMenuIndex_,
            items_[selectedMenuIndex_]);
    }
}

bool MenuScreen::isItemImplemented(
    const std::uint8_t index) const
{
    return
        items_ != nullptr &&
        index < itemCount_ &&
        AppCatalog::isImplemented(
            items_[index].appId);
}

const char *MenuScreen::iconForApp(
    const AppId appId)
{
    switch (appId)
    {
    case AppId::Dashboard:
        return LV_SYMBOL_LIST;

    case AppId::WiFiTools:
        return LV_SYMBOL_WIFI;

    case AppId::Bluetooth:
        return LV_SYMBOL_BLUETOOTH;

    case AppId::USBTools:
        return LV_SYMBOL_USB;

    case AppId::System:
        return LV_SYMBOL_POWER;

    case AppId::Settings:
        return LV_SYMBOL_SETTINGS;

    case AppId::ChannelAnalyzer:
        return LV_SYMBOL_BARS;

    case AppId::PacketMonitor:
        return LV_SYMBOL_EYE_OPEN;

    case AppId::Count:
    case AppId::Invalid:
    default:
        return LV_SYMBOL_RIGHT;
    }
}
