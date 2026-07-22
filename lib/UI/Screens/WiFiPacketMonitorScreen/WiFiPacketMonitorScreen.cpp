#include "WiFiPacketMonitorScreen.h"

#include <Arduino.h>

#include "../../../Services/WiFi/PacketMonitor.h"

namespace
{
    constexpr std::uint32_t ColorBackground = 0x070A0F;
    constexpr std::uint32_t ColorSurface = 0x111722;
    constexpr std::uint32_t ColorBorder = 0x263244;
    constexpr std::uint32_t ColorPrimary = 0xF5F7FA;
    constexpr std::uint32_t ColorSecondary = 0x9AA4B2;
    constexpr std::uint32_t ColorAccent = 0x67E8F9;
    constexpr std::uint32_t ColorSuccess = 0x86EFAC;
    constexpr std::uint32_t ColorWarning = 0xFDE68A;
    constexpr std::uint32_t ColorDanger = 0xFCA5A5;

    constexpr std::int32_t BodyX = 6;
    constexpr std::int32_t BodyY = 43;
    constexpr std::int32_t BodyWidth = 308;
    constexpr std::int32_t BodyHeight = 94;
    constexpr std::int32_t BodyColumnWidth = 143;
    constexpr std::int32_t BodyLeftX = 8;
    constexpr std::int32_t BodyRightX = 157;
}

bool WiFiPacketMonitorScreen::create(
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

    if (
        !createHeader() ||
        !createSummary() ||
        !createBodyPanels() ||
        !createFooter())
    {
        return false;
    }

    setDetailVisible(false);

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    return true;
}

bool WiFiPacketMonitorScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr);
}

void WiFiPacketMonitorScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    detailVisible_ = false;
    setDetailVisible(false);
    lastRefreshAt_ = 0;

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    // Ensure this persistent screen root is above all other screen roots.
    lv_obj_move_foreground(root_);

    refresh();

    // Force initial dashboard rendering immediately.
    lv_obj_invalidate(root_);
}

void WiFiPacketMonitorScreen::hide()
{
    PacketMonitor::stop();

    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiPacketMonitorScreen::update()
{
    PacketMonitor::update();

    const std::uint32_t now = millis();

    if (
        now - lastRefreshAt_ <
        RefreshIntervalMs)
    {
        return;
    }

    lastRefreshAt_ = now;
    refresh();
}

void WiFiPacketMonitorScreen::handleInput(
    const InputManager::Action action)
{
    switch (action)
    {
    case InputManager::Action::Next:
        selectNextChannel();
        break;

    case InputManager::Action::Select:
        toggleDetail();
        break;

    case InputManager::Action::Primary:
        startOrStop();
        break;

    case InputManager::Action::Back:
        PacketMonitor::stop();
        requestParent();
        break;

    case InputManager::Action::None:
    default:
        break;
    }
}

const char *WiFiPacketMonitorScreen::name() const
{
    return "Packet Monitor";
}

bool WiFiPacketMonitorScreen::createHeader()
{
    titleLabel_ = lv_label_create(root_);
    statusLabel_ = lv_label_create(root_);

    if (
        titleLabel_ == nullptr ||
        statusLabel_ == nullptr)
    {
        return false;
    }

    lv_label_set_text(
        titleLabel_,
        "PACKET MONITOR");

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
        8,
        3);

    lv_obj_set_width(
        statusLabel_,
        130);

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

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -8,
        3);

    return true;
}

bool WiFiPacketMonitorScreen::createSummary()
{
    channelLabel_ = lv_label_create(root_);
    ppsLabel_ = lv_label_create(root_);
    elapsedLabel_ = lv_label_create(root_);

    if (
        channelLabel_ == nullptr ||
        ppsLabel_ == nullptr ||
        elapsedLabel_ == nullptr)
    {
        return false;
    }

    lv_obj_set_width(
        channelLabel_,
        65);

    lv_obj_set_style_text_font(
        channelLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        channelLabel_,
        lv_color_hex(ColorPrimary),
        LV_PART_MAIN);

    lv_obj_set_pos(
        channelLabel_,
        8,
        21);

    lv_obj_set_width(
        ppsLabel_,
        145);

    lv_obj_set_style_text_align(
        ppsLabel_,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        ppsLabel_,
        &lv_font_montserrat_18,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        ppsLabel_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_set_pos(
        ppsLabel_,
        75,
        18);

    lv_obj_set_width(
        elapsedLabel_,
        88);

    lv_obj_set_style_text_align(
        elapsedLabel_,
        LV_TEXT_ALIGN_RIGHT,
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        elapsedLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        elapsedLabel_,
        lv_color_hex(ColorSecondary),
        LV_PART_MAIN);

    lv_obj_set_pos(
        elapsedLabel_,
        224,
        22);

    return true;
}

bool WiFiPacketMonitorScreen::createBodyPanels()
{
    overviewPanel_ = lv_obj_create(root_);
    detailPanel_ = lv_obj_create(root_);

    if (
        overviewPanel_ == nullptr ||
        detailPanel_ == nullptr)
    {
        return false;
    }

    lv_obj_t *panels[] =
    {
        overviewPanel_,
        detailPanel_
    };

    for (lv_obj_t *panel : panels)
    {
        lv_obj_set_size(
            panel,
            BodyWidth,
            BodyHeight);

        lv_obj_set_pos(
            panel,
            BodyX,
            BodyY);

        lv_obj_clear_flag(
            panel,
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            panel,
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            panel,
            5,
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            panel,
            1,
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            panel,
            lv_color_hex(ColorBorder),
            LV_PART_MAIN);

        lv_obj_set_style_bg_color(
            panel,
            lv_color_hex(ColorSurface),
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            panel,
            LV_OPA_COVER,
            LV_PART_MAIN);
    }

    for (std::uint8_t row = 0;
         row < 5;
         ++row)
    {
        overviewLabels_[row] =
            lv_label_create(overviewPanel_);

        overviewLabels_[row + 5] =
            lv_label_create(overviewPanel_);

        if (
            overviewLabels_[row] == nullptr ||
            overviewLabels_[row + 5] == nullptr)
        {
            return false;
        }

        lv_obj_set_pos(
            overviewLabels_[row],
            BodyLeftX,
            5 + row * 17);

        lv_obj_set_pos(
            overviewLabels_[row + 5],
            BodyRightX,
            5 + row * 17);
    }

    for (std::uint8_t index = 0;
         index < OverviewLabelCount;
         ++index)
    {
        lv_obj_set_width(
            overviewLabels_[index],
            BodyColumnWidth);

        lv_label_set_long_mode(
            overviewLabels_[index],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_font(
            overviewLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            overviewLabels_[index],
            lv_color_hex(ColorPrimary),
            LV_PART_MAIN);
    }

    for (std::uint8_t row = 0;
         row < 3;
         ++row)
    {
        detailLabels_[row] =
            lv_label_create(detailPanel_);

        detailLabels_[row + 3] =
            lv_label_create(detailPanel_);

        if (
            detailLabels_[row] == nullptr ||
            detailLabels_[row + 3] == nullptr)
        {
            return false;
        }

        lv_obj_set_pos(
            detailLabels_[row],
            BodyLeftX,
            13 + row * 25);

        lv_obj_set_pos(
            detailLabels_[row + 3],
            BodyRightX,
            13 + row * 25);
    }

    for (std::uint8_t index = 0;
         index < DetailLabelCount;
         ++index)
    {
        lv_obj_set_width(
            detailLabels_[index],
            BodyColumnWidth);

        lv_label_set_long_mode(
            detailLabels_[index],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_font(
            detailLabels_[index],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            detailLabels_[index],
            lv_color_hex(ColorPrimary),
            LV_PART_MAIN);
    }

    return true;
}

bool WiFiPacketMonitorScreen::createFooter()
{
    footerLabel_ = lv_label_create(root_);

    if (footerLabel_ == nullptr)
    {
        return false;
    }

    lv_obj_set_width(
        footerLabel_,
        312);

    lv_obj_set_style_text_align(
        footerLabel_,
        LV_TEXT_ALIGN_CENTER,
        LV_PART_MAIN);

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
        -2);

    return true;
}

void WiFiPacketMonitorScreen::selectNextChannel()
{
    if (
        PacketMonitor::state() !=
        PacketMonitor::State::Stopped)
    {
        return;
    }

    const std::uint8_t current =
        PacketMonitor::channel();

    const std::uint8_t next =
        current < FirstChannel ||
                current >= LastChannel
            ? FirstChannel
            : static_cast<std::uint8_t>(
                  current + 1);

    PacketMonitor::setChannel(next);
    refresh();
}

void WiFiPacketMonitorScreen::toggleDetail()
{
    setDetailVisible(!detailVisible_);
    refresh();
}

void WiFiPacketMonitorScreen::startOrStop()
{
    switch (PacketMonitor::state())
    {
    case PacketMonitor::State::Running:
    case PacketMonitor::State::Starting:
        PacketMonitor::stop();
        break;

    case PacketMonitor::State::Stopped:
    case PacketMonitor::State::Error:
        PacketMonitor::start();
        break;

    case PacketMonitor::State::Stopping:
    default:
        break;
    }

    refresh();
}

void WiFiPacketMonitorScreen::refresh()
{
    refreshHeader();
    refreshSummary();
    refreshOverview();
    refreshDetail();
    refreshFooter();
}

void WiFiPacketMonitorScreen::refreshHeader()
{
    if (
        PacketMonitor::state() ==
        PacketMonitor::State::Error)
    {
        lv_label_set_text_fmt(
            statusLabel_,
            "ERROR %d",
            static_cast<int>(
                PacketMonitor::lastErrorCode()));
    }
    else
    {
        const char *stateText =
            PacketMonitor::stateText();

        lv_label_set_text(
            statusLabel_,
            stateText != nullptr
                ? stateText
                : "UNKNOWN");
    }

    std::uint32_t color = ColorSecondary;

    switch (PacketMonitor::state())
    {
    case PacketMonitor::State::Running:
        color = ColorSuccess;
        break;

    case PacketMonitor::State::Starting:
        color = ColorAccent;
        break;

    case PacketMonitor::State::Stopping:
        color = ColorWarning;
        break;

    case PacketMonitor::State::Error:
        color = ColorDanger;
        break;

    case PacketMonitor::State::Stopped:
    default:
        color = ColorSecondary;
        break;
    }

    lv_obj_set_style_text_color(
        statusLabel_,
        lv_color_hex(color),
        LV_PART_MAIN);
}

void WiFiPacketMonitorScreen::refreshSummary()
{
    const PacketMonitor::Statistics &statistics =
        PacketMonitor::statistics();

    lv_label_set_text_fmt(
        channelLabel_,
        "CH %02u",
        static_cast<unsigned int>(
            PacketMonitor::channel()));

    lv_label_set_text_fmt(
        ppsLabel_,
        "%lu PPS",
        static_cast<unsigned long>(
            statistics.packetsPerSecond));

    const std::uint32_t elapsedSeconds =
        statistics.elapsedMs / 1000U;

    const std::uint32_t hours =
        elapsedSeconds / 3600U;

    const std::uint32_t minutes =
        (elapsedSeconds / 60U) % 60U;

    const std::uint32_t seconds =
        elapsedSeconds % 60U;

    lv_label_set_text_fmt(
        elapsedLabel_,
        "%02lu:%02lu:%02lu",
        static_cast<unsigned long>(hours),
        static_cast<unsigned long>(minutes),
        static_cast<unsigned long>(seconds));
}

void WiFiPacketMonitorScreen::refreshOverview()
{
    const PacketMonitor::Statistics &statistics =
        PacketMonitor::statistics();

    lv_label_set_text_fmt(
        overviewLabels_[0],
        "TOTAL %lu",
        static_cast<unsigned long>(
            statistics.totalPackets));

    lv_label_set_text_fmt(
        overviewLabels_[1],
        "MGMT %lu",
        static_cast<unsigned long>(
            statistics.managementPackets));

    lv_label_set_text_fmt(
        overviewLabels_[2],
        "CTRL %lu",
        static_cast<unsigned long>(
            statistics.controlPackets));

    lv_label_set_text_fmt(
        overviewLabels_[3],
        "DATA %lu",
        static_cast<unsigned long>(
            statistics.dataPackets));

    lv_label_set_text_fmt(
        overviewLabels_[5],
        "BEACON %lu",
        static_cast<unsigned long>(
            statistics.beaconFrames));

    lv_label_set_text_fmt(
        overviewLabels_[6],
        "PROBE REQ %lu",
        static_cast<unsigned long>(
            statistics.probeRequestFrames));

    lv_label_set_text_fmt(
        overviewLabels_[7],
        "DEAUTH %lu",
        static_cast<unsigned long>(
            statistics.deauthenticationFrames));

    lv_label_set_text_fmt(
        overviewLabels_[8],
        "DISASSOC %lu",
        static_cast<unsigned long>(
            statistics.disassociationFrames));

    if (statistics.totalPackets == 0)
    {
        lv_label_set_text(
            overviewLabels_[4],
            "AVG RSSI --");

        lv_label_set_text(
            overviewLabels_[9],
            "STRONG RSSI --");
    }
    else
    {
        lv_label_set_text_fmt(
            overviewLabels_[4],
            "AVG RSSI %ld",
            static_cast<long>(
                statistics.averageRssi));

        lv_label_set_text_fmt(
            overviewLabels_[9],
            "STRONG RSSI %ld",
            static_cast<long>(
                statistics.strongestRssi));
    }
}

void WiFiPacketMonitorScreen::refreshDetail()
{
    const PacketMonitor::Statistics &statistics =
        PacketMonitor::statistics();

    lv_label_set_text_fmt(
        detailLabels_[0],
        "PROBE RESP %lu",
        static_cast<unsigned long>(
            statistics.probeResponseFrames));

    lv_label_set_text_fmt(
        detailLabels_[1],
        "ASSOC REQ %lu",
        static_cast<unsigned long>(
            statistics.associationRequestFrames));

    lv_label_set_text_fmt(
        detailLabels_[2],
        "ASSOC RESP %lu",
        static_cast<unsigned long>(
            statistics.associationResponseFrames));

    lv_label_set_text_fmt(
        detailLabels_[3],
        "INTERVAL %lums",
        static_cast<unsigned long>(
            statistics.intervalMs));

    lv_label_set_text_fmt(
        detailLabels_[4],
        "MALFORMED %lu",
        static_cast<unsigned long>(
            statistics.malformedPackets));

    lv_label_set_text_fmt(
        detailLabels_[5],
        "OVERFLOW %lu",
        static_cast<unsigned long>(
            statistics.overflowPackets));
}

void WiFiPacketMonitorScreen::refreshFooter()
{
    const PacketMonitor::State state =
        PacketMonitor::state();

    if (
        state == PacketMonitor::State::Running ||
        state == PacketMonitor::State::Starting)
    {
        lv_label_set_text(
            footerLabel_,
            "KEY --/HOLD INFO  BOOT STOP/HOLD BACK");
    }
    else if (state == PacketMonitor::State::Stopping)
    {
        lv_label_set_text(
            footerLabel_,
            "KEY --/HOLD INFO  BOOT WAIT/HOLD BACK");
    }
    else
    {
        lv_label_set_text(
            footerLabel_,
            "KEY CH/HOLD INFO  BOOT START/HOLD BACK");
    }
}

void WiFiPacketMonitorScreen::setDetailVisible(
    const bool visible)
{
    detailVisible_ = visible;

    if (
        overviewPanel_ == nullptr ||
        detailPanel_ == nullptr)
    {
        return;
    }

    if (detailVisible_)
    {
        lv_obj_add_flag(
            overviewPanel_,
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
            overviewPanel_,
            LV_OBJ_FLAG_HIDDEN);
    }
}

void WiFiPacketMonitorScreen::requestParent()
{
    if (parentCallback_ != nullptr)
    {
        parentCallback_(parentContext_);
    }
}
