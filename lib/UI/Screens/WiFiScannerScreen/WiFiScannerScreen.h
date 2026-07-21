#pragma once

#include <cstdint>

#include <lvgl.h>

#include "../BaseScreen.h"

class WiFiScannerScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiScannerScreen() = default;

    bool create(
        lv_obj_t *parent,
        NavigationCallback detailCallback,
        void *detailContext,
        NavigationCallback homeCallback,
        void *homeContext);

    bool create(
        lv_obj_t *parent) override;

    void show() override;

    void hide() override;

    void update() override;

    void handleInput(
        InputManager::Action action) override;

    const char *name() const override;

private:
    static constexpr std::uint8_t VisibleRows =
        4;

    void createHeader();

    void createList();

    void createFooter();

    void startScan();

    void refresh();

    void updateHeader();

    void updateRows();

    void updateFooter();

    void movePrevious();

    void moveNext();

    void normalizeSelection();

    void openSelectedNetwork();

    void requestHome();

    lv_obj_t *root_ =
        nullptr;

    lv_obj_t *titleLabel_ =
        nullptr;

    lv_obj_t *statusLabel_ =
        nullptr;

    lv_obj_t *rowContainers_[VisibleRows] =
        {};

    lv_obj_t *rowSsidLabels_[VisibleRows] =
        {};

    lv_obj_t *rowInfoLabels_[VisibleRows] =
        {};

    lv_obj_t *footerLabel_ =
        nullptr;

    NavigationCallback detailCallback_ =
        nullptr;

    void *detailContext_ =
        nullptr;

    NavigationCallback homeCallback_ =
        nullptr;

    void *homeContext_ =
        nullptr;

    std::uint8_t selectedIndex_ =
        0;

    std::uint8_t firstVisibleIndex_ =
        0;

    std::uint8_t lastNetworkCount_ =
        0;

    bool wasScanning_ =
        false;
};