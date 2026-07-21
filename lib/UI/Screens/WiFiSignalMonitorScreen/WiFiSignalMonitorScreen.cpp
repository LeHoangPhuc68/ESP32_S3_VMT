#include "WiFiSignalMonitorScreen.h"

#include "../../../Services/WiFi/WiFiSelectionService.h"

namespace
{
    constexpr std::uint32_t ColorBackground =
        0x070A0F;

    constexpr std::uint32_t ColorSurface =
        0x111722;

    constexpr std::uint32_t ColorBorder =
        0x263244;

    constexpr std::uint32_t ColorPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorSecondary =
        0x9AA4B2;

    constexpr std::uint32_t ColorAccent =
        0x67E8F9;

    constexpr std::uint32_t ColorSuccess =
        0x86EFAC;

    constexpr std::uint32_t ColorWarning =
        0xFDE68A;

    constexpr std::uint32_t ColorDanger =
        0xFCA5A5;

    constexpr std::int32_t GraphHeight =
        48;

    constexpr std::int32_t GraphBarWidth =
        7;

    constexpr std::int32_t GraphBarGap =
        2;
}

bool WiFiSignalMonitorScreen::create(
    lv_obj_t *parent,
    const NavigationCallback backCallback,
    void *backContext,
    const NavigationCallback homeCallback,
    void *homeContext)
{
    if (parent == nullptr)
    {
        return false;
    }

    backCallback_ =
        backCallback;

    backContext_ =
        backContext;

    homeCallback_ =
        homeCallback;

    homeContext_ =
        homeContext;

    if (root_ != nullptr)
    {
        return true;
    }

    root_ =
        lv_obj_create(
            parent);

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
        lv_color_hex(
            ColorBackground),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    createHeader();
    createGraph();
    createStatistics();
    createFooter();

    hide();

    return
        titleLabel_ != nullptr &&
        ssidLabel_ != nullptr &&
        statusLabel_ != nullptr &&
        currentLabel_ != nullptr &&
        qualityLabel_ != nullptr &&
        trendLabel_ != nullptr &&
        graphPanel_ != nullptr &&
        averageLabel_ != nullptr &&
        minimumLabel_ != nullptr &&
        maximumLabel_ != nullptr &&
        rangeLabel_ != nullptr &&
        footerLabel_ != nullptr;
}

bool WiFiSignalMonitorScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
}

void WiFiSignalMonitorScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    const WiFiScannerService::Network *selected =
        WiFiSelectionService::selected();

    if (selected != nullptr)
    {
        WiFiSignalMonitorService::start(
            *selected);
    }

    lastRefreshAt_ =
        0;

    refresh();
}

void WiFiSignalMonitorScreen::hide()
{
    WiFiSignalMonitorService::stop();

    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiSignalMonitorScreen::update()
{
    WiFiSignalMonitorService::update();

    const std::uint32_t now =
        millis();

    if (
        now - lastRefreshAt_ <
        150)
    {
        return;
    }

    lastRefreshAt_ =
        now;

    refresh();
}

void WiFiSignalMonitorScreen::handleInput(
    const InputManager::Action action)
{
    switch (action)
    {
    case InputManager::Action::Select:
    {
        WiFiSignalMonitorService::
            requestImmediateSample();

        break;
    }

    case InputManager::Action::Back:
    {
        requestBack();
        break;
    }

    case InputManager::Action::Home:
    {
        requestHome();
        break;
    }

    case InputManager::Action::Previous:
    case InputManager::Action::Next:
    case InputManager::Action::None:
    default:
    {
        break;
    }
    }
}

const char *
WiFiSignalMonitorScreen::name() const
{
    return "Signal Monitor";
}

void WiFiSignalMonitorScreen::createHeader()
{
    titleLabel_ =
        lv_label_create(
            root_);

    lv_label_set_text(
        titleLabel_,
        "SIGNAL MONITOR");

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(
            ColorPrimary),
        LV_PART_MAIN);

    lv_obj_align(
        titleLabel_,
        LV_ALIGN_TOP_LEFT,
        8,
        4);

    statusLabel_ =
        lv_label_create(
            root_);

    lv_label_set_text(
        statusLabel_,
        "IDLE");

    lv_obj_set_style_text_font(
        statusLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -8,
        5);

    ssidLabel_ =
        lv_label_create(
            root_);

    lv_obj_set_width(
        ssidLabel_,
        195);

    lv_label_set_long_mode(
        ssidLabel_,
        LV_LABEL_LONG_DOT);

    lv_obj_set_style_text_font(
        ssidLabel_,
        &lv_font_montserrat_14,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        ssidLabel_,
        lv_color_hex(
            ColorAccent),
        LV_PART_MAIN);

    lv_obj_set_pos(
        ssidLabel_,
        8,
        21);

    currentLabel_ =
        lv_label_create(
            root_);

    lv_obj_set_style_text_font(
        currentLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        currentLabel_,
        lv_color_hex(
            ColorPrimary),
        LV_PART_MAIN);

    lv_obj_align(
        currentLabel_,
        LV_ALIGN_TOP_RIGHT,
        -8,
        20);

    qualityLabel_ =
        lv_label_create(
            root_);

    lv_obj_set_style_text_font(
        qualityLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        qualityLabel_,
        lv_color_hex(
            ColorSecondary),
        LV_PART_MAIN);

    lv_obj_set_pos(
        qualityLabel_,
        8,
        38);

    trendLabel_ =
        lv_label_create(
            root_);

    lv_obj_set_style_text_font(
        trendLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        trendLabel_,
        lv_color_hex(
            ColorSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        trendLabel_,
        LV_ALIGN_TOP_RIGHT,
        -8,
        39);
}

void WiFiSignalMonitorScreen::createGraph()
{
    graphPanel_ =
        lv_obj_create(
            root_);

    lv_obj_set_size(
        graphPanel_,
        304,
        55);

    lv_obj_set_pos(
        graphPanel_,
        8,
        52);

    lv_obj_clear_flag(
        graphPanel_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        graphPanel_,
        4,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        graphPanel_,
        5,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        graphPanel_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        graphPanel_,
        lv_color_hex(
            ColorBorder),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        graphPanel_,
        lv_color_hex(
            ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        graphPanel_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    for (std::uint8_t index = 0;
         index < GraphBarCount;
         ++index)
    {
        graphBars_[index] =
            lv_obj_create(
                graphPanel_);

        lv_obj_set_size(
            graphBars_[index],
            GraphBarWidth,
            1);

        lv_obj_set_pos(
            graphBars_[index],
            index *
                (GraphBarWidth +
                 GraphBarGap),
            GraphHeight - 1);

        lv_obj_clear_flag(
            graphBars_[index],
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            graphBars_[index],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            graphBars_[index],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            graphBars_[index],
            1,
            LV_PART_MAIN);

        lv_obj_set_style_bg_color(
            graphBars_[index],
            lv_color_hex(
                ColorAccent),
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            graphBars_[index],
            LV_OPA_COVER,
            LV_PART_MAIN);
    }
}

void WiFiSignalMonitorScreen::createStatistics()
{
    averageLabel_ =
        lv_label_create(
            root_);

    minimumLabel_ =
        lv_label_create(
            root_);

    maximumLabel_ =
        lv_label_create(
            root_);

    rangeLabel_ =
        lv_label_create(
            root_);

    lv_obj_t *labels[] =
    {
        averageLabel_,
        minimumLabel_,
        maximumLabel_,
        rangeLabel_
    };

    for (lv_obj_t *label : labels)
    {
        lv_obj_set_style_text_font(
            label,
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            label,
            lv_color_hex(
                ColorPrimary),
            LV_PART_MAIN);
    }

    lv_obj_set_pos(
        averageLabel_,
        8,
        113);

    lv_obj_set_pos(
        minimumLabel_,
        88,
        113);

    lv_obj_set_pos(
        maximumLabel_,
        168,
        113);

    lv_obj_set_pos(
        rangeLabel_,
        248,
        113);
}

void WiFiSignalMonitorScreen::createFooter()
{
    footerLabel_ =
        lv_label_create(
            root_);

    lv_label_set_text(
        footerLabel_,
        "SELECT SAMPLE   BACK DETAIL   HOME EXIT");

    lv_obj_set_style_text_font(
        footerLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        footerLabel_,
        lv_color_hex(
            ColorSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        footerLabel_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -5);
}

void WiFiSignalMonitorScreen::refresh()
{
    const WiFiScannerService::Network *target =
        WiFiSignalMonitorService::target();

    lv_label_set_text(
        statusLabel_,
        WiFiSignalMonitorService::stateText());

    updateStatusColor();

    if (target == nullptr)
    {
        lv_label_set_text(
            ssidLabel_,
            "NO AP SELECTED");

        lv_label_set_text(
            currentLabel_,
            "-- dBm");

        lv_label_set_text(
            qualityLabel_,
            "SIGNAL --");

        lv_label_set_text(
            trendLabel_,
            "NO TARGET");

        lv_label_set_text(
            averageLabel_,
            "AVG --");

        lv_label_set_text(
            minimumLabel_,
            "MIN --");

        lv_label_set_text(
            maximumLabel_,
            "MAX --");

        lv_label_set_text(
            rangeLabel_,
            "RNG --");

        refreshGraph();

        return;
    }

    lv_label_set_text(
        ssidLabel_,
        target->ssid.c_str());

    lv_label_set_text_fmt(
        currentLabel_,
        "%ld dBm",
        static_cast<long>(
            WiFiSignalMonitorService::
                currentRssi()));

    lv_label_set_text_fmt(
        qualityLabel_,
        "%s  %u%%",
        WiFiSignalMonitorService::
            qualityText(),
        static_cast<unsigned int>(
            WiFiSignalMonitorService::
                signalQuality()));

    const WiFiSignalMonitorService::Trend trend =
        WiFiSignalMonitorService::trend();

    switch (trend)
    {
    case WiFiSignalMonitorService::Trend::Stronger:
    {
        lv_label_set_text(
            trendLabel_,
            "+ GETTING STRONGER");

        lv_obj_set_style_text_color(
            trendLabel_,
            lv_color_hex(
                ColorSuccess),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::Trend::Weaker:
    {
        lv_label_set_text(
            trendLabel_,
            "- GETTING WEAKER");

        lv_obj_set_style_text_color(
            trendLabel_,
            lv_color_hex(
                ColorDanger),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::Trend::Stable:
    {
        lv_label_set_text(
            trendLabel_,
            "= STABLE");

        lv_obj_set_style_text_color(
            trendLabel_,
            lv_color_hex(
                ColorAccent),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::Trend::Unknown:
    default:
    {
        lv_label_set_text(
            trendLabel_,
            "COLLECTING");

        lv_obj_set_style_text_color(
            trendLabel_,
            lv_color_hex(
                ColorSecondary),
            LV_PART_MAIN);

        break;
    }
    }

    lv_label_set_text_fmt(
        averageLabel_,
        "AVG %ld",
        static_cast<long>(
            WiFiSignalMonitorService::
                averageRssi()));

    lv_label_set_text_fmt(
        minimumLabel_,
        "MIN %ld",
        static_cast<long>(
            WiFiSignalMonitorService::
                minimumRssi()));

    lv_label_set_text_fmt(
        maximumLabel_,
        "MAX %ld",
        static_cast<long>(
            WiFiSignalMonitorService::
                maximumRssi()));

    lv_label_set_text_fmt(
        rangeLabel_,
        "RNG %ld",
        static_cast<long>(
            WiFiSignalMonitorService::
                rangeDb()));

    refreshGraph();
}

void WiFiSignalMonitorScreen::refreshGraph()
{
    const std::uint8_t count =
        WiFiSignalMonitorService::
            sampleCount();

    /*
     * Căn các mẫu sang phải để mẫu mới nhất nằm
     * ở cuối biểu đồ.
     */
    const std::uint8_t emptyBars =
        GraphBarCount - count;

    for (std::uint8_t index = 0;
         index < GraphBarCount;
         ++index)
    {
        if (index < emptyBars)
        {
            lv_obj_set_height(
                graphBars_[index],
                1);

            lv_obj_set_y(
                graphBars_[index],
                GraphHeight - 1);

            lv_obj_set_style_bg_opa(
                graphBars_[index],
                LV_OPA_20,
                LV_PART_MAIN);

            continue;
        }

        const std::uint8_t sampleIndex =
            index - emptyBars;

        const std::int32_t rssi =
            WiFiSignalMonitorService::sample(
                sampleIndex);

        const std::int32_t height =
            graphHeightForRssi(
                rssi);

        lv_obj_set_height(
            graphBars_[index],
            height);

        lv_obj_set_y(
            graphBars_[index],
            GraphHeight - height);

        lv_obj_set_style_bg_opa(
            graphBars_[index],
            LV_OPA_COVER,
            LV_PART_MAIN);

        if (rssi >= -60)
        {
            lv_obj_set_style_bg_color(
                graphBars_[index],
                lv_color_hex(
                    ColorSuccess),
                LV_PART_MAIN);
        }
        else if (rssi >= -75)
        {
            lv_obj_set_style_bg_color(
                graphBars_[index],
                lv_color_hex(
                    ColorWarning),
                LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_bg_color(
                graphBars_[index],
                lv_color_hex(
                    ColorDanger),
                LV_PART_MAIN);
        }
    }
}

void WiFiSignalMonitorScreen::updateStatusColor()
{
    switch (WiFiSignalMonitorService::state())
    {
    case WiFiSignalMonitorService::State::Ready:
    {
        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorSuccess),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::State::Preparing:
    case WiFiSignalMonitorService::State::Scanning:
    {
        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorAccent),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::State::NotFound:
    case WiFiSignalMonitorService::State::Failed:
    {
        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorDanger),
            LV_PART_MAIN);

        break;
    }

    case WiFiSignalMonitorService::State::Idle:
    default:
    {
        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorSecondary),
            LV_PART_MAIN);

        break;
    }
    }
}

void WiFiSignalMonitorScreen::requestBack()
{
    if (backCallback_ != nullptr)
    {
        backCallback_(
            backContext_);
    }
}

void WiFiSignalMonitorScreen::requestHome()
{
    if (homeCallback_ != nullptr)
    {
        homeCallback_(
            homeContext_);
    }
}

std::int32_t
WiFiSignalMonitorScreen::graphHeightForRssi(
    std::int32_t rssi)
{
    if (rssi < -100)
    {
        rssi =
            -100;
    }

    if (rssi > -35)
    {
        rssi =
            -35;
    }

    const std::int32_t normalized =
        rssi + 100;

    std::int32_t height =
        2 +
        ((GraphHeight - 2) *
         normalized) /
            65;

    if (height < 1)
    {
        height =
            1;
    }

    if (height > GraphHeight)
    {
        height =
            GraphHeight;
    }

    return height;
}