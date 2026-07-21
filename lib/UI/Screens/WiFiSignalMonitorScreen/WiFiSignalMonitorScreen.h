#pragma once

#include <cstdint>
#include <lvgl.h>

#include "../BaseScreen.h"
#include "../../../Services/WiFi/WiFiSignalMonitorService.h"

class WiFiSignalMonitorScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiSignalMonitorScreen() = default;

    bool create(
        lv_obj_t *parent,
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
    static constexpr std::uint8_t GraphBarCount =
        WiFiSignalMonitorService::HistorySize;

    void createHeader();

    void createGraph();

    void createStatistics();

    void createFooter();

    void refresh();

    void refreshGraph();

    void updateStatusColor();

    void requestBack();

    void requestHome();

    static std::int32_t graphHeightForRssi(
        std::int32_t rssi);

    lv_obj_t *root_ =
        nullptr;

    lv_obj_t *titleLabel_ =
        nullptr;

    lv_obj_t *ssidLabel_ =
        nullptr;

    lv_obj_t *statusLabel_ =
        nullptr;

    lv_obj_t *currentLabel_ =
        nullptr;

    lv_obj_t *qualityLabel_ =
        nullptr;

    lv_obj_t *trendLabel_ =
        nullptr;

    lv_obj_t *graphPanel_ =
        nullptr;

    lv_obj_t *graphBars_[GraphBarCount] =
        {};

    lv_obj_t *averageLabel_ =
        nullptr;

    lv_obj_t *minimumLabel_ =
        nullptr;

    lv_obj_t *maximumLabel_ =
        nullptr;

    lv_obj_t *rangeLabel_ =
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

    std::uint32_t lastRefreshAt_ =
        0;
};