#include "WiFiAccessPointScreen.h"

#include "../../../Services/WiFi/WiFiScannerService.h"
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

    void styleValueLabel(
        lv_obj_t *label)
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
            lv_color_hex(
                ColorPrimary),
            LV_PART_MAIN);
    }
}

bool WiFiAccessPointScreen::create(
    lv_obj_t *parent,
    const NavigationCallback monitorCallback,
    void *monitorContext,
    const NavigationCallback backCallback,
    void *backContext,
    const NavigationCallback homeCallback,
    void *homeContext)
{
    if (parent == nullptr)
    {
        return false;
    }
    monitorCallback_ =
        monitorCallback;

    monitorContext_ =
        monitorContext;
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
    createContent();
    createFooter();

    hide();

    return
        titleLabel_ != nullptr &&
        ssidLabel_ != nullptr &&
        bssidLabel_ != nullptr &&
        rssiLabel_ != nullptr &&
        qualityLabel_ != nullptr &&
        channelLabel_ != nullptr &&
        frequencyLabel_ != nullptr &&
        securityLabel_ != nullptr &&
        hiddenLabel_ != nullptr &&
        signalTrack_ != nullptr &&
        signalFill_ != nullptr &&
        footerLabel_ != nullptr;
}

bool WiFiAccessPointScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
}

void WiFiAccessPointScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    refresh();
}

void WiFiAccessPointScreen::hide()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiAccessPointScreen::update()
{
    /*
     * AP Detail hiện dùng snapshot từ lần scan gần nhất,
     * chưa cần cập nhật liên tục.
     *
     * B12 Signal Monitor sẽ thực hiện RSSI realtime.
     */
}

void WiFiAccessPointScreen::handleInput(
    const InputManager::Action action)
{
    switch (action)
    {
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

    case InputManager::Action::Select:
    {
        requestMonitor();
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
WiFiAccessPointScreen::name() const
{
    return "AP Detail";
}

void WiFiAccessPointScreen::createHeader()
{
    titleLabel_ =
        lv_label_create(root_);

    if (titleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        titleLabel_,
        "ACCESS POINT");

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(
            ColorPrimary),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        titleLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_align(
        titleLabel_,
        LV_ALIGN_TOP_LEFT,
        10,
        7);
}

void WiFiAccessPointScreen::createContent()
{
    lv_obj_t *panel =
        lv_obj_create(root_);

    if (panel == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        panel,
        300,
        125);

    lv_obj_set_pos(
        panel,
        10,
        30);

    lv_obj_clear_flag(
        panel,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        panel,
        8,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        panel,
        7,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        panel,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        panel,
        lv_color_hex(
            ColorBorder),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        panel,
        lv_color_hex(
            ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        panel,
        LV_OPA_COVER,
        LV_PART_MAIN);

    ssidLabel_ =
        lv_label_create(panel);

    if (ssidLabel_ == nullptr)
    {
        return;
    }

    lv_obj_set_width(
        ssidLabel_,
        280);

    lv_label_set_long_mode(
        ssidLabel_,
        LV_LABEL_LONG_DOT);

    lv_obj_set_style_text_font(
        ssidLabel_,
        &lv_font_montserrat_16,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        ssidLabel_,
        lv_color_hex(
            ColorAccent),
        LV_PART_MAIN);

    lv_obj_align(
        ssidLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        0);

    bssidLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        bssidLabel_);

    lv_obj_set_style_text_color(
        bssidLabel_,
        lv_color_hex(
            ColorSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        bssidLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        22);

    signalTrack_ =
        lv_obj_create(panel);

    if (signalTrack_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        signalTrack_,
        280,
        8);

    lv_obj_set_pos(
        signalTrack_,
        0,
        43);

    lv_obj_clear_flag(
        signalTrack_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        signalTrack_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        signalTrack_,
        4,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        signalTrack_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        signalTrack_,
        lv_color_hex(
            ColorBorder),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        signalTrack_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    signalFill_ =
        lv_obj_create(signalTrack_);

    if (signalFill_ == nullptr)
    {
        return;
    }

    lv_obj_set_height(
        signalFill_,
        LV_PCT(100));

    lv_obj_set_width(
        signalFill_,
        0);

    lv_obj_align(
        signalFill_,
        LV_ALIGN_LEFT_MID,
        0,
        0);

    lv_obj_clear_flag(
        signalFill_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_pad_all(
        signalFill_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        signalFill_,
        4,
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        signalFill_,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        signalFill_,
        lv_color_hex(
            ColorSuccess),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        signalFill_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    rssiLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        rssiLabel_);

    lv_obj_align(
        rssiLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        59);

    qualityLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        qualityLabel_);

    lv_obj_align(
        qualityLabel_,
        LV_ALIGN_TOP_RIGHT,
        0,
        59);

    channelLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        channelLabel_);

    lv_obj_align(
        channelLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        80);

    frequencyLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        frequencyLabel_);

    lv_obj_align(
        frequencyLabel_,
        LV_ALIGN_TOP_RIGHT,
        0,
        80);

    securityLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        securityLabel_);

    lv_obj_align(
        securityLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        101);

    hiddenLabel_ =
        lv_label_create(panel);

    styleValueLabel(
        hiddenLabel_);

    lv_obj_align(
        hiddenLabel_,
        LV_ALIGN_TOP_RIGHT,
        0,
        101);
}

void WiFiAccessPointScreen::createFooter()
{
    footerLabel_ =
        lv_label_create(root_);

    if (footerLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        footerLabel_,
        "SELECT  MONITOR   BACK  LIST   HOME  EXIT");

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

void WiFiAccessPointScreen::refresh()
{
    const WiFiScannerService::Network *network =
        WiFiSelectionService::selected();

    if (network == nullptr)
    {
        lv_label_set_text(
            ssidLabel_,
            "NO AP SELECTED");

        lv_label_set_text(
            bssidLabel_,
            "--:--:--:--:--:--");

        lv_label_set_text(
            rssiLabel_,
            "RSSI --");

        lv_label_set_text(
            qualityLabel_,
            "SIGNAL --");

        lv_label_set_text(
            channelLabel_,
            "CH --");

        lv_label_set_text(
            frequencyLabel_,
            "-- MHz");

        lv_label_set_text(
            securityLabel_,
            "SECURITY --");

        lv_label_set_text(
            hiddenLabel_,
            "VISIBLE --");

        updateSignalBar(
            0);

        return;
    }

    const std::uint8_t quality =
        WiFiSelectionService::signalQuality();

    const std::uint16_t frequency =
        WiFiSelectionService::frequencyMHz();

    lv_label_set_text(
        ssidLabel_,
        network->ssid.c_str());

    lv_label_set_text(
        bssidLabel_,
        network->bssid.c_str());

    lv_label_set_text_fmt(
        rssiLabel_,
        "RSSI %ld dBm",
        static_cast<long>(
            network->rssi));

    lv_label_set_text_fmt(
        qualityLabel_,
        "SIGNAL %u%%",
        static_cast<unsigned int>(
            quality));

    lv_label_set_text_fmt(
        channelLabel_,
        "CHANNEL %ld",
        static_cast<long>(
            network->channel));

    if (frequency == 0)
    {
        lv_label_set_text(
            frequencyLabel_,
            "FREQ UNKNOWN");
    }
    else
    {
        lv_label_set_text_fmt(
            frequencyLabel_,
            "%u MHz",
            static_cast<unsigned int>(
                frequency));
    }

    lv_label_set_text_fmt(
        securityLabel_,
        "%s",
        WiFiScannerService::securityText(
            network->encryptionType));

    lv_label_set_text(
        hiddenLabel_,
        network->hidden
            ? "HIDDEN YES"
            : "HIDDEN NO");

    updateSignalBar(
        quality);
}

void WiFiAccessPointScreen::updateSignalBar(
    const std::uint8_t quality)
{
    if (signalFill_ == nullptr)
    {
        return;
    }

    /*
     * Track rộng 280 px.
     */
    const std::int32_t width =
        (280 *
         static_cast<std::int32_t>(
             quality)) /
        100;

    lv_obj_set_width(
        signalFill_,
        width);

    if (quality >= 70)
    {
        lv_obj_set_style_bg_color(
            signalFill_,
            lv_color_hex(
                ColorSuccess),
            LV_PART_MAIN);

        return;
    }

    if (quality >= 40)
    {
        lv_obj_set_style_bg_color(
            signalFill_,
            lv_color_hex(
                ColorWarning),
            LV_PART_MAIN);

        return;
    }

    lv_obj_set_style_bg_color(
        signalFill_,
        lv_color_hex(
            0xFCA5A5),
        LV_PART_MAIN);
}

void WiFiAccessPointScreen::requestBack()
{
    if (backCallback_ == nullptr)
    {
        return;
    }

    backCallback_(
        backContext_);
}

void WiFiAccessPointScreen::requestHome()
{
    if (homeCallback_ == nullptr)
    {
        return;
    }

    homeCallback_(
        homeContext_);
}
void WiFiAccessPointScreen::requestMonitor()
{
    if (monitorCallback_ == nullptr)
    {
        return;
    }

    if (!WiFiSelectionService::hasSelection())
    {
        return;
    }

    monitorCallback_(
        monitorContext_);
}