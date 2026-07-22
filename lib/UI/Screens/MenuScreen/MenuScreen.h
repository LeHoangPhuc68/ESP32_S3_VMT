#pragma once

#include <cstdint>

#include <lvgl.h>

#include "InputManager.h"
#include "../../../Core/Menu/MenuItem.h"

class MenuScreen final
{
public:
    using SelectionCallback = void (*)(
        void *context,
        std::uint8_t index,
        const MenuItem &item);

    MenuScreen() = default;

    bool create(
        lv_obj_t *parent,
        const MenuItem *items,
        std::uint8_t itemCount,
        const char *title,
        SelectionCallback selectionCallback,
        void *callbackContext);

    void open();

    void close();

    bool isVisible() const;

    void handleInput(
        InputManager::Action action);

    void showStatus(
        const char *text,
        std::uint32_t color);

    void hideStatus();

    std::uint8_t selectedIndex() const;

private:
    static constexpr std::uint8_t MaximumTileCount = 6;

    void createMenuCard(
        lv_obj_t *parent);

    void createHeader();

    void createTiles();

    void createControlLabel();

    void updateMenuDisplay();

    void selectCurrentMenuItem();

    bool isItemImplemented(
        std::uint8_t index) const;

    static const char *iconForApp(
        AppId appId);

    const MenuItem *items_ = nullptr;
    std::uint8_t itemCount_ = 0;
    const char *title_ = nullptr;

    SelectionCallback selectionCallback_ = nullptr;
    void *callbackContext_ = nullptr;

    lv_obj_t *menuCard_ = nullptr;
    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *statusLabel_ = nullptr;
    lv_obj_t *controlLabel_ = nullptr;

    lv_obj_t *tileContainers_[MaximumTileCount] = {};
    lv_obj_t *tileIconLabels_[MaximumTileCount] = {};
    lv_obj_t *tileTitleLabels_[MaximumTileCount] = {};
    lv_obj_t *tileStateLabels_[MaximumTileCount] = {};

    bool visible_ = false;
    std::uint8_t selectedMenuIndex_ = 0;
};
