#pragma once

#include <cstdint>

#include <lvgl.h>

#include "../BaseScreen.h"
#include "../MenuScreen/MenuScreen.h"

class WiFiMenuScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiMenuScreen() = default;

    bool create(
        lv_obj_t *parent,
        NavigationCallback scannerCallback,
        void *scannerContext,
        NavigationCallback analyzerCallback,
        void *analyzerContext,
        NavigationCallback packetMonitorCallback,
        void *packetMonitorContext,
        NavigationCallback parentCallback,
        void *parentContext);

    bool create(
        lv_obj_t *parent) override;

    void show() override;

    void hide() override;

    void handleInput(
        InputManager::Action action) override;

    const char *name() const override;

private:
    static constexpr std::uint8_t ItemCount = 3;

    static const MenuItem Items_[ItemCount];

    static void handleMenuSelection(
        void *context,
        std::uint8_t index,
        const MenuItem &item);

    void requestScanner();

    void requestAnalyzer();

    void requestPacketMonitor();

    void requestParent();

    lv_obj_t *root_ = nullptr;

    MenuScreen menuScreen_;

    NavigationCallback scannerCallback_ = nullptr;
    void *scannerContext_ = nullptr;

    NavigationCallback analyzerCallback_ = nullptr;
    void *analyzerContext_ = nullptr;

    NavigationCallback packetMonitorCallback_ = nullptr;
    void *packetMonitorContext_ = nullptr;

    NavigationCallback parentCallback_ = nullptr;
    void *parentContext_ = nullptr;
};
