#pragma once

#include <cstdint>

#include <lvgl.h>

#include "../BaseScreen.h"
#include "../../../Services/WiFi/WiFiChannelAnalyzer.h"
#include "../../../Services/WiFi/WiFiScannerService.h"

class WiFiChannelAnalyzerScreen final : public BaseScreen
{
public:
    using NavigationCallback =
        void (*)(void *context);

    WiFiChannelAnalyzerScreen() = default;

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
    static constexpr std::uint8_t ChannelCount =
        WiFiChannelAnalyzer::ChannelCount;

    void createHeader();

    void createChart();

    void createDetailView();

    void createFooter();

    void startScan();

    void refresh();

    void refreshHeader();

    void refreshChart();

    void refreshSelection();

    void refreshDetail();

    void updateStatusColor();

    void moveNext();

    void toggleDetail();

    void requestParent();

    lv_obj_t *root_ = nullptr;

    lv_obj_t *titleLabel_ = nullptr;

    lv_obj_t *statusLabel_ = nullptr;

    lv_obj_t *recommendationLabel_ = nullptr;

    lv_obj_t *snapshotLabel_ = nullptr;

    lv_obj_t *chartPanel_ = nullptr;

    lv_obj_t *channelCells_[ChannelCount] = {};

    lv_obj_t *barFills_[ChannelCount] = {};

    lv_obj_t *apCountLabels_[ChannelCount] = {};

    lv_obj_t *channelLabels_[ChannelCount] = {};

    lv_obj_t *selectionLabel_ = nullptr;

    lv_obj_t *detailPanel_ = nullptr;

    lv_obj_t *detailTitleLabel_ = nullptr;

    lv_obj_t *detailCountLabel_ = nullptr;

    lv_obj_t *detailStrongestLabel_ = nullptr;

    lv_obj_t *detailAverageLabel_ = nullptr;

    lv_obj_t *detailScoreLabel_ = nullptr;

    lv_obj_t *detailRecommendationLabel_ = nullptr;

    lv_obj_t *footerLabel_ = nullptr;

    NavigationCallback parentCallback_ = nullptr;

    void *parentContext_ = nullptr;

    WiFiChannelAnalyzer analyzer_;

    WiFiScannerService::State lastScannerState_ =
        WiFiScannerService::State::Idle;

    std::uint8_t selectedChannel_ =
        WiFiChannelAnalyzer::FirstChannel;

    bool detailVisible_ = false;

    std::uint32_t lastAgeRefreshAt_ = 0;
};
