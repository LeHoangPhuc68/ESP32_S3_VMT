#include "WiFiScannerScreen.h"

#include "../../../Services/WiFi/WiFiScannerService.h"
#include "../../../Services/WiFi/WiFiSelectionService.h"

namespace
{
    constexpr std::uint32_t ColorBackground =
        0x070A0F;

    constexpr std::uint32_t ColorSurface =
        0x111722;

    constexpr std::uint32_t ColorSelected =
        0x183447;

    constexpr std::uint32_t ColorBorder =
        0x263244;

    constexpr std::uint32_t ColorTextPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorTextSecondary =
        0x9AA4B2;

    constexpr std::uint32_t ColorAccent =
        0x67E8F9;

    constexpr std::uint32_t ColorSuccess =
        0x86EFAC;

    constexpr std::uint32_t ColorError =
        0xFCA5A5;
}

bool WiFiScannerScreen::create(
    lv_obj_t *parent,
    const NavigationCallback detailCallback,
    void *detailContext,
    const NavigationCallback homeCallback,
    void *homeContext)
{
    if (parent == nullptr)
    {
        return false;
    }

    detailCallback_ =
        detailCallback;

    detailContext_ =
        detailContext;

    homeCallback_ =
        homeCallback;

    homeContext_ =
        homeContext;

    if (root_ != nullptr)
    {
        return true;
    }

    root_ =
        lv_obj_create(parent);

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
    createList();
    createFooter();

    hide();

    for (std::uint8_t row = 0;
         row < VisibleRows;
         ++row)
    {
        if (
            rowContainers_[row] == nullptr ||
            rowSsidLabels_[row] == nullptr ||
            rowInfoLabels_[row] == nullptr)
        {
            return false;
        }
    }

    return
        titleLabel_ != nullptr &&
        statusLabel_ != nullptr &&
        footerLabel_ != nullptr;
}

bool WiFiScannerScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
}

void WiFiScannerScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    /*
     * Chỉ scan khi chưa có kết quả.
     *
     * Quay lại từ AP Detail sẽ giữ nguyên danh sách.
     */
    if (
        WiFiScannerService::state() ==
            WiFiScannerService::State::Idle ||
        WiFiScannerService::state() ==
            WiFiScannerService::State::Failed)
    {
        selectedIndex_ =
            0;

        firstVisibleIndex_ =
            0;

        lastNetworkCount_ =
            0;

        startScan();
    }

    normalizeSelection();
    refresh();
}

void WiFiScannerScreen::hide()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiScannerScreen::update()
{
    if (
        root_ == nullptr ||
        lv_obj_has_flag(
            root_,
            LV_OBJ_FLAG_HIDDEN))
    {
        return;
    }

    WiFiScannerService::update();

    const bool scanning =
        WiFiScannerService::isScanning();

    const std::uint8_t networkCount =
        WiFiScannerService::networkCount();

    if (
        scanning != wasScanning_ ||
        networkCount != lastNetworkCount_)
    {
        wasScanning_ =
            scanning;

        lastNetworkCount_ =
            networkCount;

        normalizeSelection();
        refresh();
    }
}

void WiFiScannerScreen::handleInput(
    const InputManager::Action action)
{
    switch (action)
    {
    case InputManager::Action::Previous:
    {
        movePrevious();
        break;
    }

    case InputManager::Action::Next:
    {
        moveNext();
        break;
    }

    case InputManager::Action::Select:
    {
        openSelectedNetwork();
        break;
    }

    case InputManager::Action::Back:
    {
        startScan();
        refresh();
        break;
    }

    case InputManager::Action::Home:
    {
        requestHome();
        break;
    }

    case InputManager::Action::None:
    default:
    {
        break;
    }
    }
}

const char *WiFiScannerScreen::name() const
{
    return "Wi-Fi Scanner";
}

void WiFiScannerScreen::createHeader()
{
    titleLabel_ =
        lv_label_create(root_);

    if (titleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        titleLabel_,
        "WI-FI SCANNER");

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(
            ColorTextPrimary),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        titleLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_align(
        titleLabel_,
        LV_ALIGN_TOP_LEFT,
        10,
        8);

    statusLabel_ =
        lv_label_create(root_);

    if (statusLabel_ == nullptr)
    {
        return;
    }

    lv_obj_set_style_text_font(
        statusLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        statusLabel_,
        lv_color_hex(
            ColorAccent),
        LV_PART_MAIN);

    lv_obj_align(
        statusLabel_,
        LV_ALIGN_TOP_RIGHT,
        -10,
        11);
}

void WiFiScannerScreen::createList()
{
    constexpr std::int16_t RowStartY =
        32;

    constexpr std::int16_t RowHeight =
        27;

    constexpr std::int16_t RowGap =
        2;

    for (std::uint8_t row = 0;
         row < VisibleRows;
         ++row)
    {
        lv_obj_t *container =
            lv_obj_create(root_);

        rowContainers_[row] =
            container;

        if (container == nullptr)
        {
            return;
        }

        lv_obj_set_size(
            container,
            300,
            RowHeight);

        lv_obj_set_pos(
            container,
            10,
            RowStartY +
                row * (
                    RowHeight +
                    RowGap));

        lv_obj_clear_flag(
            container,
            LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_style_pad_all(
            container,
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            container,
            6,
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            container,
            lv_color_hex(
                ColorBorder),
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            container,
            1,
            LV_PART_MAIN);

        lv_obj_set_style_bg_color(
            container,
            lv_color_hex(
                ColorSurface),
            LV_PART_MAIN);

        lv_obj_set_style_bg_opa(
            container,
            LV_OPA_COVER,
            LV_PART_MAIN);

        rowSsidLabels_[row] =
            lv_label_create(container);

        if (rowSsidLabels_[row] == nullptr)
        {
            return;
        }

        lv_obj_set_width(
            rowSsidLabels_[row],
            180);

        lv_label_set_long_mode(
            rowSsidLabels_[row],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_font(
            rowSsidLabels_[row],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            rowSsidLabels_[row],
            lv_color_hex(
                ColorTextPrimary),
            LV_PART_MAIN);

        lv_obj_align(
            rowSsidLabels_[row],
            LV_ALIGN_LEFT_MID,
            8,
            0);

        rowInfoLabels_[row] =
            lv_label_create(container);

        if (rowInfoLabels_[row] == nullptr)
        {
            return;
        }

        lv_obj_set_style_text_font(
            rowInfoLabels_[row],
            &lv_font_montserrat_12,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            rowInfoLabels_[row],
            lv_color_hex(
                ColorTextSecondary),
            LV_PART_MAIN);

        lv_obj_align(
            rowInfoLabels_[row],
            LV_ALIGN_RIGHT_MID,
            -7,
            0);
    }
}

void WiFiScannerScreen::createFooter()
{
    footerLabel_ =
        lv_label_create(root_);

    if (footerLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        footerLabel_,
        "LEFT/RIGHT MOVE   SELECT OPEN   BACK RESCAN   HOME EXIT");

    lv_obj_set_style_text_font(
        footerLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        footerLabel_,
        lv_color_hex(
            ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        footerLabel_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -5);
}

void WiFiScannerScreen::startScan()
{
    if (WiFiScannerService::isScanning())
    {
        return;
    }

    selectedIndex_ =
        0;

    firstVisibleIndex_ =
        0;

    WiFiScannerService::startScan();

    wasScanning_ =
        WiFiScannerService::isScanning();

    lastNetworkCount_ =
        WiFiScannerService::networkCount();
}

void WiFiScannerScreen::refresh()
{
    updateHeader();
    updateRows();
    updateFooter();
}

void WiFiScannerScreen::updateHeader()
{
    if (statusLabel_ == nullptr)
    {
        return;
    }

    switch (WiFiScannerService::state())
    {
    case WiFiScannerService::State::Preparing:
    case WiFiScannerService::State::Scanning:
    {
        lv_label_set_text(
            statusLabel_,
            "SCANNING...");

        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorAccent),
            LV_PART_MAIN);

        break;
    }

    case WiFiScannerService::State::Complete:
    {
        lv_label_set_text_fmt(
            statusLabel_,
            "%u NETWORKS",
            static_cast<unsigned int>(
                WiFiScannerService::
                    networkCount()));

        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorSuccess),
            LV_PART_MAIN);

        break;
    }

    case WiFiScannerService::State::Failed:
    {
        lv_label_set_text(
            statusLabel_,
            "SCAN FAILED");

        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorError),
            LV_PART_MAIN);

        break;
    }

    case WiFiScannerService::State::Idle:
    default:
    {
        lv_label_set_text(
            statusLabel_,
            "READY");

        lv_obj_set_style_text_color(
            statusLabel_,
            lv_color_hex(
                ColorTextSecondary),
            LV_PART_MAIN);

        break;
    }
    }
}

void WiFiScannerScreen::updateRows()
{
    const std::uint8_t count =
        WiFiScannerService::networkCount();

    for (std::uint8_t row = 0;
         row < VisibleRows;
         ++row)
    {
        const std::uint8_t networkIndex =
            firstVisibleIndex_ + row;

        if (networkIndex >= count)
        {
            lv_obj_add_flag(
                rowContainers_[row],
                LV_OBJ_FLAG_HIDDEN);

            continue;
        }

        const WiFiScannerService::Network *network =
            WiFiScannerService::network(
                networkIndex);

        if (network == nullptr)
        {
            lv_obj_add_flag(
                rowContainers_[row],
                LV_OBJ_FLAG_HIDDEN);

            continue;
        }

        lv_obj_remove_flag(
            rowContainers_[row],
            LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text(
            rowSsidLabels_[row],
            network->ssid.c_str());

        lv_label_set_text_fmt(
            rowInfoLabels_[row],
            "%ld  CH%ld  %s",
            static_cast<long>(
                network->rssi),
            static_cast<long>(
                network->channel),
            WiFiScannerService::securityText(
                network->encryptionType));

        const bool selected =
            networkIndex ==
                selectedIndex_;

        lv_obj_set_style_bg_color(
            rowContainers_[row],
            lv_color_hex(
                selected
                    ? ColorSelected
                    : ColorSurface),
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            rowContainers_[row],
            lv_color_hex(
                selected
                    ? ColorAccent
                    : ColorBorder),
            LV_PART_MAIN);
    }
}

void WiFiScannerScreen::updateFooter()
{
    if (footerLabel_ == nullptr)
    {
        return;
    }

    if (WiFiScannerService::isScanning())
    {
        lv_label_set_text(
            footerLabel_,
            "SCANNING...   HOME EXIT");

        return;
    }

        lv_label_set_text(
            footerLabel_,
            "LEFT/RIGHT  MOVE   SELECT  DETAIL   BACK  EXIT");
}

void WiFiScannerScreen::movePrevious()
{
    const std::uint8_t count =
        WiFiScannerService::networkCount();

    if (
        count == 0 ||
        WiFiScannerService::isScanning())
    {
        return;
    }

    if (selectedIndex_ == 0)
    {
        selectedIndex_ =
            count - 1;
    }
    else
    {
        --selectedIndex_;
    }

    normalizeSelection();
    refresh();
}

void WiFiScannerScreen::moveNext()
{
    const std::uint8_t count =
        WiFiScannerService::networkCount();

    if (
        count == 0 ||
        WiFiScannerService::isScanning())
    {
        return;
    }

    selectedIndex_ =
        static_cast<std::uint8_t>(
            (selectedIndex_ + 1) %
            count);

    normalizeSelection();
    refresh();
}

void WiFiScannerScreen::normalizeSelection()
{
    const std::uint8_t count =
        WiFiScannerService::networkCount();

    if (count == 0)
    {
        selectedIndex_ =
            0;

        firstVisibleIndex_ =
            0;

        return;
    }

    if (selectedIndex_ >= count)
    {
        selectedIndex_ =
            count - 1;
    }

    if (selectedIndex_ < firstVisibleIndex_)
    {
        firstVisibleIndex_ =
            selectedIndex_;
    }

    const std::uint8_t lastVisibleIndex =
        firstVisibleIndex_ +
        VisibleRows - 1;

    if (selectedIndex_ > lastVisibleIndex)
    {
        firstVisibleIndex_ =
            selectedIndex_ -
            VisibleRows + 1;
    }

    const std::uint8_t maximumFirstIndex =
        count > VisibleRows
            ? count - VisibleRows
            : 0;

    if (firstVisibleIndex_ > maximumFirstIndex)
    {
        firstVisibleIndex_ =
            maximumFirstIndex;
    }
}

void WiFiScannerScreen::openSelectedNetwork()
{
    if (WiFiScannerService::isScanning())
    {
        return;
    }

    const WiFiScannerService::Network *network =
        WiFiScannerService::network(
            selectedIndex_);

    if (network == nullptr)
    {
        return;
    }

    if (!WiFiSelectionService::select(
            *network))
    {
        return;
    }

    if (detailCallback_ == nullptr)
    {
        return;
    }

    detailCallback_(
        detailContext_);
}

void WiFiScannerScreen::requestHome()
{
    if (homeCallback_ == nullptr)
    {
        return;
    }

    homeCallback_(
        homeContext_);
}