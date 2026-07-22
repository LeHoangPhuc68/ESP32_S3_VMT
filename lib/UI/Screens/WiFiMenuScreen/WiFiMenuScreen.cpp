#include "WiFiMenuScreen.h"

const MenuItem WiFiMenuScreen::Items_[
    WiFiMenuScreen::ItemCount] = {
    {
        "Wi-Fi Scanner",
        AppId::WiFiTools
    },
    {
        "Channel\nAnalyzer",
        AppId::ChannelAnalyzer
    },
    {
        "Packet\nMonitor",
        AppId::PacketMonitor
    }
};

bool WiFiMenuScreen::create(
    lv_obj_t *parent,
    const NavigationCallback scannerCallback,
    void *scannerContext,
    const NavigationCallback analyzerCallback,
    void *analyzerContext,
    const NavigationCallback packetMonitorCallback,
    void *packetMonitorContext,
    const NavigationCallback parentCallback,
    void *parentContext)
{
    if (parent == nullptr)
    {
        return false;
    }

    scannerCallback_ = scannerCallback;
    scannerContext_ = scannerContext;
    analyzerCallback_ = analyzerCallback;
    analyzerContext_ = analyzerContext;
    packetMonitorCallback_ = packetMonitorCallback;
    packetMonitorContext_ = packetMonitorContext;
    parentCallback_ = parentCallback;
    parentContext_ = parentContext;

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
        lv_color_hex(0x070A0F),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    if (!menuScreen_.create(
            root_,
            Items_,
            ItemCount,
            "WI-FI TOOLS",
            &WiFiMenuScreen::handleMenuSelection,
            this))
    {
        return false;
    }

    hide();

    return true;
}

bool WiFiMenuScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr);
}

void WiFiMenuScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    lv_obj_move_foreground(root_);

    menuScreen_.open();

    lv_obj_invalidate(root_);
}

void WiFiMenuScreen::hide()
{
    menuScreen_.close();

    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void WiFiMenuScreen::handleInput(
    const InputManager::Action action)
{
    if (action == InputManager::Action::Back)
    {
        requestParent();
        return;
    }

    menuScreen_.handleInput(action);
}

const char *WiFiMenuScreen::name() const
{
    return "Wi-Fi Menu";
}

void WiFiMenuScreen::handleMenuSelection(
    void *context,
    const std::uint8_t index,
    const MenuItem &item)
{
    if (
        context == nullptr ||
        index >= ItemCount)
    {
        return;
    }

    WiFiMenuScreen *screen =
        static_cast<WiFiMenuScreen *>(
            context);

    switch (item.appId)
    {
    case AppId::WiFiTools:
        screen->requestScanner();
        break;

    case AppId::ChannelAnalyzer:
        screen->requestAnalyzer();
        break;

    case AppId::PacketMonitor:
        screen->requestPacketMonitor();
        break;

    default:
        break;
    }
}

void WiFiMenuScreen::requestScanner()
{
    if (scannerCallback_ != nullptr)
    {
        scannerCallback_(
            scannerContext_);
    }
}

void WiFiMenuScreen::requestAnalyzer()
{
    if (analyzerCallback_ != nullptr)
    {
        analyzerCallback_(
            analyzerContext_);
    }
}

void WiFiMenuScreen::requestPacketMonitor()
{
    if (packetMonitorCallback_ != nullptr)
    {
        packetMonitorCallback_(
            packetMonitorContext_);
    }
}

void WiFiMenuScreen::requestParent()
{
    if (parentCallback_ != nullptr)
    {
        parentCallback_(
            parentContext_);
    }
}
