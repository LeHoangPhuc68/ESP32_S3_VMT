#pragma once

#include <lvgl.h>

#include "../BaseScreen.h"

class WiFiAccessPointScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiAccessPointScreen() = default;

    bool create(
        lv_obj_t *parent,
        NavigationCallback monitorCallback,
        void *monitorContext,
        NavigationCallback backCallback,
        void *backContext,
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
    void createHeader();

    void createContent();

    void createFooter();

    void refresh();

    void updateSignalBar(
        std::uint8_t quality);

    void requestBack();

    void requestHome();

    void requestMonitor();

    NavigationCallback monitorCallback_ =
        nullptr;

    void *monitorContext_ =
        nullptr;

    lv_obj_t *root_ =
        nullptr;

    lv_obj_t *titleLabel_ =
        nullptr;

    lv_obj_t *ssidLabel_ =
        nullptr;

    lv_obj_t *bssidLabel_ =
        nullptr;

    lv_obj_t *rssiLabel_ =
        nullptr;

    lv_obj_t *qualityLabel_ =
        nullptr;

    lv_obj_t *channelLabel_ =
        nullptr;

    lv_obj_t *frequencyLabel_ =
        nullptr;

    lv_obj_t *securityLabel_ =
        nullptr;

    lv_obj_t *hiddenLabel_ =
        nullptr;

    lv_obj_t *signalTrack_ =
        nullptr;

    lv_obj_t *signalFill_ =
        nullptr;

    lv_obj_t *footerLabel_ =
        nullptr;

    NavigationCallback backCallback_ =
        nullptr;

    void *backContext_ =
        nullptr;

    NavigationCallback homeCallback_ =
        nullptr;

    void *homeContext_ =
        nullptr;
};