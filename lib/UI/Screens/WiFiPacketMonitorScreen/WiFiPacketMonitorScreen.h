#pragma once

#include <cstdint>

#include <lvgl.h>

#include "../BaseScreen.h"

class WiFiPacketMonitorScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiPacketMonitorScreen() = default;

    bool create(
        lv_obj_t *parent,
        NavigationCallback parentCallback,
        void *parentContext);

    bool create(
        lv_obj_t *parent) override;

    void show() override;

    void hide() override;

    void update() override;

    void handleInput(
        InputManager::Action action) override;

    const char *name() const override;

private:
    static constexpr std::uint8_t FirstChannel = 1;
    static constexpr std::uint8_t LastChannel = 13;
    static constexpr std::uint8_t OverviewLabelCount = 10;
    static constexpr std::uint8_t DetailLabelCount = 6;
    static constexpr std::uint32_t RefreshIntervalMs = 150;

    bool createHeader();

    bool createSummary();

    bool createBodyPanels();

    bool createFooter();

    void selectNextChannel();

    void toggleDetail();

    void startOrStop();

    void refresh();

    void refreshHeader();

    void refreshSummary();

    void refreshOverview();

    void refreshDetail();

    void refreshFooter();

    void setDetailVisible(bool visible);

    void requestParent();

    lv_obj_t *root_ = nullptr;

    lv_obj_t *titleLabel_ = nullptr;
    lv_obj_t *statusLabel_ = nullptr;

    lv_obj_t *channelLabel_ = nullptr;
    lv_obj_t *ppsLabel_ = nullptr;
    lv_obj_t *elapsedLabel_ = nullptr;

    lv_obj_t *overviewPanel_ = nullptr;
    lv_obj_t *overviewLabels_[OverviewLabelCount] = {};

    lv_obj_t *detailPanel_ = nullptr;
    lv_obj_t *detailLabels_[DetailLabelCount] = {};

    lv_obj_t *footerLabel_ = nullptr;

    NavigationCallback parentCallback_ = nullptr;
    void *parentContext_ = nullptr;

    std::uint32_t lastRefreshAt_ = 0;
    bool detailVisible_ = false;
};
