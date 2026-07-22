#include "DashboardScreen.h"

#include <Arduino.h>

namespace
{
    constexpr std::uint32_t ColorBackground =
        0x070A0F;

    constexpr std::uint32_t ColorSurface =
        0x111722;

    constexpr std::uint32_t ColorSurfaceBorder =
        0x263244;

    constexpr std::uint32_t ColorTextPrimary =
        0xF5F7FA;

    constexpr std::uint32_t ColorTextSecondary =
        0x9AA4B2;

    constexpr std::uint32_t ColorAccent =
        0x67E8F9;

    constexpr std::uint32_t ColorSuccess =
        0x86EFAC;
}

bool DashboardScreen::create(
    lv_obj_t *parent,
    const NavigationCallback navigationCallback,
    void *navigationContext)
{
    if (parent == nullptr)
    {
        return false;
    }

    /*
     * Không tạo lại object LVGL nếu screen đã tồn tại.
     */
    if (root_ != nullptr)
    {
        navigationCallback_ =
            navigationCallback;

        navigationContext_ =
            navigationContext;

        return true;
    }

    navigationCallback_ =
        navigationCallback;

    navigationContext_ =
        navigationContext;

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
        lv_color_hex(ColorBackground),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    createHeader();
    createSystemCard();
    createConnectivityCard();
    createFooter();

    /*
     * Dashboard được tạo sẵn nhưng chưa hiển thị.
     */
    hide();

    return
        titleLabel_ != nullptr &&
        subtitleLabel_ != nullptr &&
        systemCard_ != nullptr &&
        systemTitleLabel_ != nullptr &&
        systemValueLabel_ != nullptr &&
        connectivityCard_ != nullptr &&
        connectivityTitleLabel_ != nullptr &&
        connectivityValueLabel_ != nullptr &&
        footerLabel_ != nullptr;
}

bool DashboardScreen::create(
    lv_obj_t *parent)
{
    return create(
        parent,
        nullptr,
        nullptr);
}

void DashboardScreen::show()
{
    if (root_ == nullptr)
    {
        return;
    }

    lastUpdateTime_ = 0;

    lv_obj_remove_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);

    update();
}

void DashboardScreen::hide()
{
    if (root_ == nullptr)
    {
        return;
    }

    lv_obj_add_flag(
        root_,
        LV_OBJ_FLAG_HIDDEN);
}

void DashboardScreen::update()
{
    if (
        root_ == nullptr ||
        lv_obj_has_flag(
            root_,
            LV_OBJ_FLAG_HIDDEN))
    {
        return;
    }

    const unsigned long currentTime =
        millis();

    /*
     * Chỉ cập nhật label mỗi giây để tránh tạo
     * quá nhiều invalidation cho LVGL.
     */
    if (
        lastUpdateTime_ != 0 &&
        currentTime - lastUpdateTime_ < 1000)
    {
        return;
    }

    lastUpdateTime_ =
        currentTime;

    if (systemValueLabel_ != nullptr)
    {
        lv_label_set_text_fmt(
            systemValueLabel_,
            "UPTIME  %lu s\nHEAP    %u KB",
            currentTime / 1000UL,
            static_cast<unsigned int>(
                ESP.getFreeHeap() / 1024U));
    }

    if (connectivityValueLabel_ != nullptr)
    {
        lv_label_set_text(
            connectivityValueLabel_,
            "WI-FI   IDLE\nBLE     IDLE");
    }
}

void DashboardScreen::handleInput(
    const InputManager::Action action)
{
    switch (action)
    {
    case InputManager::Action::Back:
    {
        requestParent();
        break;
    }

    case InputManager::Action::Next:
    case InputManager::Action::Select:
    case InputManager::Action::Primary:
    case InputManager::Action::None:
    default:
    {
        /*
         * Dashboard hiện chưa có thành phần tương tác.
         */
        break;
    }
    }
}

const char *DashboardScreen::name() const
{
    return "Dashboard";
}

void DashboardScreen::createHeader()
{
    if (root_ == nullptr)
    {
        return;
    }

    titleLabel_ =
        lv_label_create(root_);

    if (titleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        titleLabel_,
        "DASHBOARD");

    lv_obj_set_style_text_font(
        titleLabel_,
        &lv_font_montserrat_18,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        titleLabel_,
        lv_color_hex(ColorTextPrimary),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        titleLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_align(
        titleLabel_,
        LV_ALIGN_TOP_LEFT,
        14,
        12);

    subtitleLabel_ =
        lv_label_create(root_);

    if (subtitleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        subtitleLabel_,
        "VMT SYSTEM OVERVIEW");

    lv_obj_set_style_text_font(
        subtitleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        subtitleLabel_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_set_style_text_letter_space(
        subtitleLabel_,
        1,
        LV_PART_MAIN);

    lv_obj_align_to(
        subtitleLabel_,
        titleLabel_,
        LV_ALIGN_OUT_BOTTOM_LEFT,
        0,
        2);
}

void DashboardScreen::createSystemCard()
{
    if (root_ == nullptr)
    {
        return;
    }

    systemCard_ =
        lv_obj_create(root_);

    if (systemCard_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        systemCard_,
        143,
        82);

    lv_obj_align(
        systemCard_,
        LV_ALIGN_BOTTOM_LEFT,
        10,
        -25);

    lv_obj_clear_flag(
        systemCard_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(
        systemCard_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        systemCard_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        systemCard_,
        lv_color_hex(ColorSurfaceBorder),
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        systemCard_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        systemCard_,
        12,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        systemCard_,
        10,
        LV_PART_MAIN);

    systemTitleLabel_ =
        lv_label_create(systemCard_);

    if (systemTitleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        systemTitleLabel_,
        "SYSTEM");

    lv_obj_set_style_text_font(
        systemTitleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        systemTitleLabel_,
        lv_color_hex(ColorSuccess),
        LV_PART_MAIN);

    lv_obj_align(
        systemTitleLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        0);

    systemValueLabel_ =
        lv_label_create(systemCard_);

    if (systemValueLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        systemValueLabel_,
        "UPTIME  0 s\nHEAP    -- KB");

    lv_obj_set_style_text_font(
        systemValueLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        systemValueLabel_,
        lv_color_hex(ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_set_style_text_line_space(
        systemValueLabel_,
        5,
        LV_PART_MAIN);

    lv_obj_align(
        systemValueLabel_,
        LV_ALIGN_BOTTOM_LEFT,
        0,
        0);
}

void DashboardScreen::createConnectivityCard()
{
    if (root_ == nullptr)
    {
        return;
    }

    connectivityCard_ =
        lv_obj_create(root_);

    if (connectivityCard_ == nullptr)
    {
        return;
    }

    lv_obj_set_size(
        connectivityCard_,
        143,
        82);

    lv_obj_align(
        connectivityCard_,
        LV_ALIGN_BOTTOM_RIGHT,
        -10,
        -25);

    lv_obj_clear_flag(
        connectivityCard_,
        LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(
        connectivityCard_,
        lv_color_hex(ColorSurface),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        connectivityCard_,
        LV_OPA_COVER,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        connectivityCard_,
        lv_color_hex(ColorSurfaceBorder),
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        connectivityCard_,
        1,
        LV_PART_MAIN);

    lv_obj_set_style_radius(
        connectivityCard_,
        12,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        connectivityCard_,
        10,
        LV_PART_MAIN);

    connectivityTitleLabel_ =
        lv_label_create(connectivityCard_);

    if (connectivityTitleLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        connectivityTitleLabel_,
        "CONNECTIVITY");

    lv_obj_set_style_text_font(
        connectivityTitleLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        connectivityTitleLabel_,
        lv_color_hex(ColorAccent),
        LV_PART_MAIN);

    lv_obj_align(
        connectivityTitleLabel_,
        LV_ALIGN_TOP_LEFT,
        0,
        0);

    connectivityValueLabel_ =
        lv_label_create(connectivityCard_);

    if (connectivityValueLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        connectivityValueLabel_,
        "WI-FI   IDLE\nBLE     IDLE");

    lv_obj_set_style_text_font(
        connectivityValueLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        connectivityValueLabel_,
        lv_color_hex(ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_set_style_text_line_space(
        connectivityValueLabel_,
        5,
        LV_PART_MAIN);

    lv_obj_align(
        connectivityValueLabel_,
        LV_ALIGN_BOTTOM_LEFT,
        0,
        0);
}

void DashboardScreen::createFooter()
{
    if (root_ == nullptr)
    {
        return;
    }

    footerLabel_ =
        lv_label_create(root_);

    if (footerLabel_ == nullptr)
    {
        return;
    }

    lv_label_set_text(
        footerLabel_,
        "BOOT HOLD  MAIN MENU");

    lv_obj_set_style_text_font(
        footerLabel_,
        &lv_font_montserrat_12,
        LV_PART_MAIN);

    lv_obj_set_style_text_color(
        footerLabel_,
        lv_color_hex(ColorTextSecondary),
        LV_PART_MAIN);

    lv_obj_align(
        footerLabel_,
        LV_ALIGN_BOTTOM_MID,
        0,
        -7);
}

void DashboardScreen::requestParent()
{
    if (navigationCallback_ == nullptr)
    {
        return;
    }

    navigationCallback_(
        navigationContext_);
}
