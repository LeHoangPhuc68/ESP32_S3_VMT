#include "WiFiChannelAnalyzerScreen.h"
#include <Arduino.h>

namespace
{
    constexpr std::uint32_t ColorBackground = 0x070A0F;
    constexpr std::uint32_t ColorSurface = 0x111722;
    constexpr std::uint32_t ColorSurfaceFocused = 0x183447;
    constexpr std::uint32_t ColorBorder = 0x263244;
    constexpr std::uint32_t ColorPrimary = 0xF5F7FA;
    constexpr std::uint32_t ColorSecondary = 0x9AA4B2;
    constexpr std::uint32_t ColorAccent = 0x67E8F9;
    constexpr std::uint32_t ColorSuccess = 0x86EFAC;
    constexpr std::uint32_t ColorWarning = 0xFDE68A;
    constexpr std::uint32_t ColorDanger = 0xFCA5A5;

    constexpr std::int32_t ChartWidth = 308;
    constexpr std::int32_t ChartHeight = 92;
    constexpr std::int32_t CellWidth = 22;
    constexpr std::int32_t CellHeight = 84;
    constexpr std::int32_t CellGap = 1;
    constexpr std::int32_t BarHeight = 45;
    constexpr std::int32_t BarTop = 17;
}

bool WiFiChannelAnalyzerScreen::create(
    lv_obj_t *parent,
    const NavigationCallback parentCallback,
    void *parentContext)
{
    if (parent == nullptr)
    {
        return false;
    }

    parentCallback_ = parentCallback;
    parentContext_ = parentContext;

    if (root_ != nullptr)
    {
        return true;
    }

    root_ = lv_obj_create(parent);

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
        lv_color_hex(ColorBackground),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    createHeader();
    createChart();
    createDetailView();
    createFooter();

    hide();

    if (
        titleLabel_ == nullptr ||
        statusLabel_ == nullptr ||
        recommendationLabel_ == nullptr ||
        snapshotLabel_ == nullptr ||
        chartPanel_ == nullptr ||
        selectionLabel_ == nullptr ||
        detailPanel_ == nullptr ||
        detailTitleLabel_ == nullptr ||
        detailCountLabel_ == nullptr ||
        detailStrongestLabel_ == nullptr ||
        detailAverageLabel_ == nullptr ||
        detailScoreLabel_ == nullptr ||
        detailRecommendationLabel_ == nullptr ||
        footerLabel_ == nullptr)
    {
        return false;
    }

    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        if (
            channelCells_[index] == nullptr ||
            barFills_[index] == nullptr ||
            apCountLabels_[index] == nullptr ||
            channelLabels_[index] == nullptr)
        {
            return false;
        }
    }

    return true;
}

bool WiFiChannelAnalyzerScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr);
}

void WiFiChannelAnalyzerScreen::show()
{
    Serial.println("[ANALYZER] show enter");

    if (root_ == nullptr)
    {
        Serial.println("[ANALYZER] root null");
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    lv_obj_move_foreground(root_);

    detailVisible_ = false;

    lv_obj_remove_flag(
        chartPanel_,
        LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(
        detailPanel_,
        LV_OBJ_FLAG_HIDDEN);

    const WiFiScanSnapshot &snapshot =
        WiFiScannerService::snapshot();

    if (snapshot.hasSuccessfulScan())
    {
        analyzer_.analyze(snapshot);
    }
    else
    {
        analyzer_.clear();
    }

    lastScannerState_ =
        WiFiScannerService::state();

    lastAgeRefreshAt_ = millis();

    refresh();
    lv_obj_invalidate(root_);

    Serial.println("[ANALYZER] show exit");
}

void WiFiChannelAnalyzerScreen::hide()
{
    WiFiScannerService::cancelScan();

    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiChannelAnalyzerScreen::update()
{
    static std::uint32_t lastDebugAt = 0;
    const std::uint32_t debugNow = millis();

    if (debugNow - lastDebugAt >= 1000U)
    {
        lastDebugAt = debugNow;
        Serial.println("[ANALYZER] update alive");
    }

    WiFiScannerService::update();

    const WiFiScanSnapshot &snapshot =
        WiFiScannerService::snapshot();

    bool analysisChanged = false;

    if (snapshot.hasSuccessfulScan())
    {
        analysisChanged =
            analyzer_.analyze(snapshot);
    }
    else if (analyzer_.hasAnalysis())
    {
        analyzer_.clear();
        analysisChanged = true;
    }

    const WiFiScannerService::State scannerState =
        WiFiScannerService::state();

    const bool stateChanged =
        scannerState != lastScannerState_;

    const std::uint32_t now = millis();
    const bool ageChanged =
        now - lastAgeRefreshAt_ >= 1000;

    if (analysisChanged)
    {
        lastScannerState_ = scannerState;
        lastAgeRefreshAt_ = now;
        refresh();
        return;
    }

    lastScannerState_ = scannerState;

    if (stateChanged || ageChanged)
    {
        if (ageChanged)
        {
            lastAgeRefreshAt_ = now;
        }

        refreshHeader();
    }
}

void WiFiChannelAnalyzerScreen::handleInput(
    const InputManager::Action action)
{
    Serial.printf(
        "[ANALYZER] input=%u\n",
        static_cast<unsigned int>(action));

    switch (action)
    switch (action)
    {
    case InputManager::Action::Next:
        moveNext();
        break;

    case InputManager::Action::Select:
        toggleDetail();
        break;

    case InputManager::Action::Primary:
        startScan();
        break;

    case InputManager::Action::Back:
        WiFiScannerService::cancelScan();
        requestParent();
        break;

    case InputManager::Action::None:
    default:
        break;
    }
}

const char *WiFiChannelAnalyzerScreen::name() const
{
    return "Channel Analyzer";
}

void WiFiChannelAnalyzerScreen::createHeader()
{
    titleLabel_ = lv_label_create(root_);
    statusLabel_ = lv_label_create(root_);
    recommendationLabel_ = lv_label_create(root_);
    snapshotLabel_ = lv_label_create(root_);

    if (
        titleLabel_ == nullptr ||
        statusLabel_ == nullptr ||
        recommendationLabel_ == nullptr ||
        snapshotLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        titleLabel_,
        "CHANNEL ANALYZER");

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(ColorPrimary),
        LV_PART_MAIN);

    lv_obj_set_pos(
        titleLabel_,
        7,
        3);

    lv_obj_set_width(
        statusLabel_,
        105);

    lv_obj_set_style_text_align(
        statusLabel_,
        LV_TEXT_ALIGN_RIGHT,
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        statusLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -7,
        3);

    lv_obj_set_style_text_font(
        recommendationLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        recommendationLabel_,
        lv_color_hex(ColorSuccess),
        LV_PART_MAIN);

    lv_obj_set_pos(
        recommendationLabel_,
        7,
        19);

    lv_obj_set_width(
        snapshotLabel_,
        165);

    lv_label_set_long_mode(
        snapshotLabel_,
        LV_LABEL_LONG_DOT);

    lv_obj_set_style_text_align(
        snapshotLabel_,
        LV_TEXT_ALIGN_RIGHT,
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        snapshotLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        snapshotLabel_,
        lv_color_hex(ColorSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        snapshotLabel_,
        LV_ALIGN_TOP_RIGHT,
        -7,
        19);
}

void WiFiChannelAnalyzerScreen::createChart()
{
    chartPanel_ = lv_obj_create(root_);

    if (chartPanel_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        chartPanel_,
        ChartWidth,
        ChartHeight);

    lv_obj_set_pos(
        chartPanel_,
        6,
        36);

    lv_obj_clear_flag(
        chartPanel_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        chartPanel_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        chartPanel_,
        5,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        chartPanel_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        chartPanel_,
        lv_color_hex(ColorBorder),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        chartPanel_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        chartPanel_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    const std::int32_t chartContentWidth =
        ChannelCount * CellWidth +
        (ChannelCount - 1) * CellGap;

    const std::int32_t firstCellX =
        (ChartWidth - chartContentWidth) / 2;

    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        lv_obj_t *cell = lv_obj_create(chartPanel_);
        channelCells_[index] = cell;

        if (cell == nullptr)
        {
            return;
        }

        lv_obj_set_size(
            cell,
            CellWidth,
            CellHeight);

        lv_obj_set_pos(
            cell,
            firstCellX +
                index * (CellWidth + CellGap),
            3);

        lv_obj_clear_flag(
            cell,
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            cell,
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            cell,
            3,
            LV_PART_MAIN);

        lv_obj_set_style_bg_color(
            cell,
            lv_color_hex(ColorSurface),
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            cell,
            LV_OPA_COVER,
            LV_PART_MAIN);

        barFills_[index] = lv_obj_create(cell);
        apCountLabels_[index] = lv_label_create(cell);
        channelLabels_[index] = lv_label_create(cell);

        if (
            barFills_[index] == nullptr ||
            apCountLabels_[index] == nullptr ||
            channelLabels_[index] == nullptr)
        {
            return;
        }

        lv_obj_set_size(
            barFills_[index],
            12,
            1);

        lv_obj_set_pos(
            barFills_[index],
            5,
            BarTop + BarHeight - 1);

        lv_obj_clear_flag(
            barFills_[index],
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            barFills_[index],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            barFills_[index],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            barFills_[index],
            2,
            LV_PART_MAIN);

        lv_obj_set_width(
            apCountLabels_[index],
            CellWidth);

        lv_obj_set_style_text_align(
            apCountLabels_[index],
            LV_TEXT_ALIGN_CENTER,
            LV_PART_MAIN);

        lv_obj_set_style_text_font(
            apCountLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            apCountLabels_[index],
            lv_color_hex(ColorSecondary),
            LV_PART_MAIN);

        lv_obj_set_pos(
            apCountLabels_[index],
            0,
            0);

        lv_obj_set_width(
            channelLabels_[index],
            CellWidth);

        lv_label_set_text_fmt(
            channelLabels_[index],
            "%u",
            static_cast<unsigned int>(
                index + WiFiChannelAnalyzer::FirstChannel));

        lv_obj_set_style_text_align(
            channelLabels_[index],
            LV_TEXT_ALIGN_CENTER,
            LV_PART_MAIN);

        lv_obj_set_style_text_font(
            channelLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_pos(
            channelLabels_[index],
            0,
            66);
    }

    selectionLabel_ = lv_label_create(root_);

    if (selectionLabel_ == nullptr)
    {
        return;
    }

    lv_obj_set_width(
        selectionLabel_,
        306);

    lv_obj_set_style_text_align(
        selectionLabel_,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        selectionLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        selectionLabel_,
        lv_color_hex(ColorPrimary),
        LV_PART_MAIN);

    lv_obj_set_pos(
        selectionLabel_,
        7,
        131);
}

void WiFiChannelAnalyzerScreen::createDetailView()
{
    detailPanel_ = lv_obj_create(root_);

    if (detailPanel_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        detailPanel_,
        ChartWidth,
        ChartHeight);

    lv_obj_set_pos(
        detailPanel_,
        6,
        36);

    lv_obj_clear_flag(
        detailPanel_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        detailPanel_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        detailPanel_,
        5,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        detailPanel_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        detailPanel_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        detailPanel_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        detailPanel_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    detailTitleLabel_ = lv_label_create(detailPanel_);
    detailCountLabel_ = lv_label_create(detailPanel_);
    detailStrongestLabel_ = lv_label_create(detailPanel_);
    detailAverageLabel_ = lv_label_create(detailPanel_);
    detailScoreLabel_ = lv_label_create(detailPanel_);
    detailRecommendationLabel_ = lv_label_create(detailPanel_);

    lv_obj_t *labels[] =
    {
        detailTitleLabel_,
        detailCountLabel_,
        detailStrongestLabel_,
        detailAverageLabel_,
        detailScoreLabel_,
        detailRecommendationLabel_
    };

    for (lv_obj_t *label : labels)
    {
        if (label == nullptr)
        {
            return;
        }

        lv_obj_set_style_text_font(
            label,
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            label,
            lv_color_hex(ColorPrimary),
            LV_PART_MAIN);
    }

    lv_obj_set_pos(detailTitleLabel_, 8, 5);
    lv_obj_set_pos(detailCountLabel_, 8, 29);
    lv_obj_set_pos(detailStrongestLabel_, 91, 29);
    lv_obj_set_pos(detailAverageLabel_, 199, 29);
    lv_obj_set_pos(detailScoreLabel_, 8, 55);
    lv_obj_set_pos(detailRecommendationLabel_, 124, 55);

    lv_obj_add_flag(
        detailPanel_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiChannelAnalyzerScreen::createFooter()
{
    footerLabel_ = lv_label_create(root_);

    if (footerLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        footerLabel_,
        "KEY NEXT/HOLD INFO  BOOT SCAN/HOLD BACK");

    lv_obj_set_style_text_font(
        footerLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        footerLabel_,
        lv_color_hex(ColorSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        footerLabel_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -3);
}

void WiFiChannelAnalyzerScreen::startScan()
{
    if (WiFiScannerService::isScanning())
    {
        return;
    }

    WiFiScannerService::startScan();

    lastScannerState_ =
        WiFiScannerService::state();

    refreshHeader();
    lv_obj_invalidate(root_);
}

void WiFiChannelAnalyzerScreen::refresh()
{
    refreshHeader();
    refreshChart();
    refreshSelection();
    refreshDetail();
}

void WiFiChannelAnalyzerScreen::refreshHeader()
{
    switch (WiFiScannerService::state())
    {
    case WiFiScannerService::State::Preparing:
    case WiFiScannerService::State::Scanning:
        lv_label_set_text(statusLabel_, "SCANNING");
        break;

    case WiFiScannerService::State::Complete:
        lv_label_set_text(statusLabel_, "READY");
        break;

    case WiFiScannerService::State::Failed:
        lv_label_set_text(statusLabel_, "FAILED");
        break;

    case WiFiScannerService::State::Idle:
    default:
        lv_label_set_text(
            statusLabel_,
            analyzer_.hasAnalysis()
                ? "CACHED"
                : "NO DATA");
        break;
    }

    updateStatusColor();

    if (!analyzer_.hasAnalysis())
    {
        lv_label_set_text(
            recommendationLabel_,
            "BEST CH --");

        lv_label_set_text(
            snapshotLabel_,
            "GEN --");

        return;
    }

    const WiFiScanSnapshot &snapshot =
        WiFiScannerService::snapshot();

    if (
        snapshot.hasSuccessfulScan() &&
        snapshot.generation() == analyzer_.generation() &&
        snapshot.count() == 0)
    {
        lv_label_set_text_fmt(
            recommendationLabel_,
            "DEFAULT CH %u",
            static_cast<unsigned int>(
                analyzer_.recommendedChannel()));
    }
    else
    {
        lv_label_set_text_fmt(
            recommendationLabel_,
            "BEST CH %u",
            static_cast<unsigned int>(
                analyzer_.recommendedChannel()));
    }

    const std::uint32_t ageSeconds =
        (millis() - analyzer_.completedAtMs()) /
        1000U;

    lv_label_set_text_fmt(
        snapshotLabel_,
        "GEN %lu  AGE %lus",
        static_cast<unsigned long>(
            analyzer_.generation()),
        static_cast<unsigned long>(
            ageSeconds));
}

void WiFiChannelAnalyzerScreen::refreshChart()
{
    std::uint16_t maximumScore = 0;

    if (analyzer_.hasAnalysis())
    {
        for (std::uint8_t channel =
                 WiFiChannelAnalyzer::FirstChannel;
             channel <= WiFiChannelAnalyzer::LastChannel;
             ++channel)
        {
            const WiFiChannelAnalyzer::ChannelMetrics *metrics =
                analyzer_.metrics(channel);

            if (
                metrics != nullptr &&
                metrics->interferenceScore > maximumScore)
            {
                maximumScore = metrics->interferenceScore;
            }
        }
    }

    for (std::uint8_t index = 0;
         index < ChannelCount;
         ++index)
    {
        const std::uint8_t channel =
            index + WiFiChannelAnalyzer::FirstChannel;

        const WiFiChannelAnalyzer::ChannelMetrics *metrics =
            analyzer_.metrics(channel);

        std::int32_t height = 1;
        std::uint32_t barColor = ColorSuccess;

        if (metrics != nullptr)
        {
            lv_label_set_text_fmt(
                apCountLabels_[index],
                "%u",
                static_cast<unsigned int>(
                    metrics->apCount));

            if (maximumScore > 0)
            {
                height =
                    2 +
                    ((BarHeight - 2) *
                     metrics->interferenceScore) /
                        maximumScore;

                const std::uint32_t relativeScore =
                    (static_cast<std::uint32_t>(
                         metrics->interferenceScore) *
                     100U) /
                    maximumScore;

                if (relativeScore > 66)
                {
                    barColor = ColorDanger;
                }
                else if (relativeScore > 33)
                {
                    barColor = ColorWarning;
                }
            }
        }
        else
        {
            lv_label_set_text(
                apCountLabels_[index],
                "-");
        }

        lv_obj_set_height(
            barFills_[index],
            height);

        lv_obj_set_y(
            barFills_[index],
            BarTop + BarHeight - height);

        lv_obj_set_style_bg_color(
            barFills_[index],
            lv_color_hex(barColor),
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            barFills_[index],
            metrics == nullptr ||
                    metrics->interferenceScore == 0
                ? LV_OPA_30
                : LV_OPA_COVER,
            LV_PART_MAIN);

        const bool selected =
            channel == selectedChannel_;

        const bool recommended =
            metrics != nullptr &&
            metrics->recommended;

        lv_obj_set_style_bg_color(
            channelCells_[index],
            lv_color_hex(
                selected
                    ? ColorSurfaceFocused
                    : ColorSurface),
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            channelCells_[index],
            selected || recommended
                ? 2
                : 1,
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            channelCells_[index],
            lv_color_hex(
                selected
                    ? ColorAccent
                    : (recommended
                           ? ColorSuccess
                           : ColorBorder)),
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            channelLabels_[index],
            lv_color_hex(
                recommended
                    ? ColorSuccess
                    : (selected
                           ? ColorAccent
                           : ColorPrimary)),
            LV_PART_MAIN);
    }
}

void WiFiChannelAnalyzerScreen::refreshSelection()
{
    const WiFiChannelAnalyzer::ChannelMetrics *metrics =
        analyzer_.metrics(selectedChannel_);

    if (metrics == nullptr)
    {
        lv_label_set_text_fmt(
            selectionLabel_,
            "CH %02u   AP --   INT --",
            static_cast<unsigned int>(
                selectedChannel_));

        return;
    }

    lv_label_set_text_fmt(
        selectionLabel_,
        "CH %02u   AP %u   INT %u",
        static_cast<unsigned int>(
            selectedChannel_),
        static_cast<unsigned int>(
            metrics->apCount),
        static_cast<unsigned int>(
            metrics->interferenceScore));
}

void WiFiChannelAnalyzerScreen::refreshDetail()
{
    const WiFiChannelAnalyzer::ChannelMetrics *metrics =
        analyzer_.metrics(selectedChannel_);

    lv_label_set_text_fmt(
        detailTitleLabel_,
        "CHANNEL %u DETAILS",
        static_cast<unsigned int>(
            selectedChannel_));

    if (metrics == nullptr)
    {
        lv_label_set_text(detailCountLabel_, "APS --");
        lv_label_set_text(detailStrongestLabel_, "STRONG --");
        lv_label_set_text(detailAverageLabel_, "AVG --");
        lv_label_set_text(detailScoreLabel_, "SCORE --");
        lv_label_set_text(
            detailRecommendationLabel_,
            "NO SNAPSHOT");
        return;
    }

    lv_label_set_text_fmt(
        detailCountLabel_,
        "APS %u",
        static_cast<unsigned int>(
            metrics->apCount));

    if (metrics->apCount == 0)
    {
        lv_label_set_text(
            detailStrongestLabel_,
            "STRONG --");

        lv_label_set_text(
            detailAverageLabel_,
            "AVG --");
    }
    else
    {
        lv_label_set_text_fmt(
            detailStrongestLabel_,
            "STRONG %d",
            static_cast<int>(
                metrics->strongestRssi));

        lv_label_set_text_fmt(
            detailAverageLabel_,
            "AVG %d",
            static_cast<int>(
                metrics->averageRssi));
    }

    lv_label_set_text_fmt(
        detailScoreLabel_,
        "SCORE %u",
        static_cast<unsigned int>(
            metrics->interferenceScore));

    lv_label_set_text(
        detailRecommendationLabel_,
        metrics->recommended
            ? "RECOMMENDED"
            : "NOT RECOMMENDED");

    lv_obj_set_style_text_color(
        detailRecommendationLabel_,
        lv_color_hex(
            metrics->recommended
                ? ColorSuccess
                : ColorSecondary),
        LV_PART_MAIN);
}

void WiFiChannelAnalyzerScreen::updateStatusColor()
{
    std::uint32_t color = ColorSecondary;

    switch (WiFiScannerService::state())
    {
    case WiFiScannerService::State::Preparing:
    case WiFiScannerService::State::Scanning:
        color = ColorAccent;
        break;

    case WiFiScannerService::State::Complete:
        color = ColorSuccess;
        break;

    case WiFiScannerService::State::Failed:
        color = ColorDanger;
        break;

    case WiFiScannerService::State::Idle:
    default:
        color = analyzer_.hasAnalysis()
            ? ColorSuccess
            : ColorSecondary;
        break;
    }

    lv_obj_set_style_text_color(
        statusLabel_,
        lv_color_hex(color),
        LV_PART_MAIN);
}

void WiFiChannelAnalyzerScreen::moveNext()
{
    selectedChannel_ =
        selectedChannel_ >= WiFiChannelAnalyzer::LastChannel
            ? WiFiChannelAnalyzer::FirstChannel
            : static_cast<std::uint8_t>(
                  selectedChannel_ + 1);

    refreshChart();
    refreshSelection();
    refreshDetail();
}

void WiFiChannelAnalyzerScreen::toggleDetail()
{
    detailVisible_ = !detailVisible_;

    if (detailVisible_)
    {
        refreshDetail();

        lv_obj_add_flag(
            chartPanel_,
            LV_OBJ_FLAG_HIDDEN);

        lv_obj_remove_flag(
            detailPanel_,
            LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(
            detailPanel_,
            LV_OBJ_FLAG_HIDDEN);

        lv_obj_remove_flag(
            chartPanel_,
            LV_OBJ_FLAG_HIDDEN);
    }
}

void WiFiChannelAnalyzerScreen::requestParent()
{
    if (parentCallback_ != nullptr)
    {
        parentCallback_(parentContext_);
    }
}
