#include "UIManager.h"

#include <Arduino.h>
#include <cstdint>
#include <lvgl.h>

#include "../Core/App/AppManager.h"
#include "../Display/Display.h"
#include "../Services/WiFi/WiFiScannerService.h"

#include "Navigation/AppScreenRegistry.h"
#include "Navigation/ScreenManager.h"

#include "Screens/DashboardScreen/DashboardScreen.h"
#include "Screens/HomeScreen/HomeScreen.h"
#include "Screens/WiFiScannerScreen/WiFiScannerScreen.h"
#include "Screens/WiFiAccessPointScreen/WiFiAccessPointScreen.h"
#include "Screens/WiFiSignalMonitorScreen/WiFiSignalMonitorScreen.h"

namespace
{
    constexpr std::uint16_t DrawBufferLines =
        20;

    lv_display_t *lvglDisplay =
        nullptr;

    alignas(4) std::uint8_t drawBuffer[
        Display::NativeHeight *
        DrawBufferLines *
        sizeof(std::uint16_t)];

    HomeScreen homeScreen;

    DashboardScreen dashboardScreen;

    WiFiScannerScreen wifiScannerScreen;
    
    WiFiAccessPointScreen wifiAccessPointScreen;

    WiFiSignalMonitorScreen wifiSignalMonitorScreen;

    std::uint32_t getTickMilliseconds()
    {
        return millis();
    }

    void flushDisplay(
        lv_display_t *display,
        const lv_area_t *area,
        std::uint8_t *pixelMap)
    {
        if (display == nullptr)
        {
            return;
        }

        if (
            area == nullptr ||
            pixelMap == nullptr)
        {
            lv_display_flush_ready(
                display);

            return;
        }

        const std::int32_t width =
            area->x2 - area->x1 + 1;

        const std::int32_t height =
            area->y2 - area->y1 + 1;

        if (
            width <= 0 ||
            height <= 0)
        {
            lv_display_flush_ready(
                display);

            return;
        }

        Display::drawRgb565Bitmap(
            static_cast<std::int16_t>(
                area->x1),
            static_cast<std::int16_t>(
                area->y1),
            reinterpret_cast<const std::uint16_t *>(
                pixelMap),
            static_cast<std::uint16_t>(
                width),
            static_cast<std::uint16_t>(
                height));

        lv_display_flush_ready(
            display);
    }
}

bool UIManager::begin()
{
    if (lvglDisplay != nullptr)
    {
        return true;
    }

    lv_init();

    lv_tick_set_cb(
        getTickMilliseconds);

    lvglDisplay =
        lv_display_create(
            Display::width(),
            Display::height());

    if (lvglDisplay == nullptr)
    {
        return false;
    }

    lv_display_set_color_format(
        lvglDisplay,
        LV_COLOR_FORMAT_RGB565);

    lv_display_set_buffers(
        lvglDisplay,
        drawBuffer,
        nullptr,
        sizeof(drawBuffer),
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_flush_cb(
        lvglDisplay,
        flushDisplay);

    lv_obj_t *root =
        lv_screen_active();

    if (root == nullptr)
    {
        return false;
    }

    lv_obj_set_style_bg_color(
        root,
        lv_color_hex(0x000000),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        root,
        LV_OPA_COVER,
        LV_PART_MAIN);

    lv_obj_clear_flag(
        root,
        LV_OBJ_FLAG_SCROLLABLE);

    /*
     * Khởi tạo service trước khi screen sử dụng nó.
     */
    if (!WiFiScannerService::begin())
    {
        return false;
    }

    /*
     * Tạo screen.
     */
    if (!homeScreen.create(root))
    {
        return false;
    }

    if (!dashboardScreen.create(
            root,
            &UIManager::handleReturnHome,
            nullptr))
    {
        return false;
    }

    if (!wifiScannerScreen.create(
        root,
        &UIManager::handleShowWiFiAccessPoint,
        nullptr,
        &UIManager::handleReturnHome,
        nullptr))
    {
        return false;
    }

    if (!wifiAccessPointScreen.create(
            root,
            &UIManager::handleShowWiFiSignalMonitor,
            nullptr,
            &UIManager::handleReturnWiFiScanner,
            nullptr,
            &UIManager::handleReturnHome,
            nullptr))
    {
        return false;
    }

    if (!wifiSignalMonitorScreen.create(
            root,
            &UIManager::handleReturnWiFiAccessPoint,
            nullptr,
            &UIManager::handleReturnHome,
            nullptr))
    {
        return false;
    }

    /*
     * Khởi tạo và đăng ký screen.
     */
    if (!ScreenManager::begin())
    {
        return false;
    }

    if (!ScreenManager::registerScreen(
            ScreenId::Home,
            homeScreen))
    {
        return false;
    }

    if (!ScreenManager::registerScreen(
            ScreenId::Dashboard,
            dashboardScreen))
    {
        return false;
    }

    if (!ScreenManager::registerScreen(
            ScreenId::WiFiScanner,
            wifiScannerScreen))
    {
        return false;
    }

    if (!ScreenManager::registerScreen(
        ScreenId::WiFiAccessPoint,
        wifiAccessPointScreen))
    {
        return false;
    }

        if (!ScreenManager::registerScreen(
            ScreenId::WiFiSignalMonitor,
            wifiSignalMonitorScreen))
    {
        return false;
    }

    /*
     * AppManager điều phối AppId sang UI.
     */
    if (!AppManager::begin())
    {
        return false;
    }

    AppManager::setLaunchHandler(
        &UIManager::handleAppLaunch,
        nullptr);

    return showHome();
}

void UIManager::update()
{
    BaseScreen *screen =
        ScreenManager::currentScreen();

    if (screen != nullptr)
    {
        screen->update();
    }

    const std::uint32_t waitTime =
        lv_timer_handler();

    if (waitTime == 0)
    {
        delay(1);
        return;
    }

    delay(
        waitTime > 5
            ? 5
            : waitTime);
}

void UIManager::handleInput(
    const InputManager::Action action)
{
    if (action == InputManager::Action::None)
    {
        return;
    }

    BaseScreen *screen =
        ScreenManager::currentScreen();

    if (screen == nullptr)
    {
        return;
    }

    screen->handleInput(
        action);
}

bool UIManager::showScreen(
    const ScreenId screenId)
{
    return ScreenManager::show(
        screenId);
}

bool UIManager::showScreen(
    BaseScreen &screen)
{
    return ScreenManager::show(
        screen);
}

bool UIManager::showHome()
{
    return showScreen(
        ScreenId::Home);
}

bool UIManager::showApp(
    const AppId appId)
{
    ScreenId screenId =
        ScreenId::Invalid;

    if (!AppScreenRegistry::findScreen(
            appId,
            screenId))
    {
        return false;
    }

    return showScreen(
        screenId);
}

BaseScreen *UIManager::currentScreen()
{
    return ScreenManager::currentScreen();
}

ScreenId UIManager::currentScreenId()
{
    return ScreenManager::currentScreenId();
}

bool UIManager::handleAppLaunch(
    void *context,
    const AppId appId)
{
    (void)context;

    return showApp(
        appId);
}

void UIManager::handleReturnHome(
    void *context)
{
    (void)context;

    showHome();
}

void UIManager::handleShowWiFiAccessPoint(
    void *context)
{
    (void)context;

    showScreen(
        ScreenId::WiFiAccessPoint);
}

void UIManager::handleReturnWiFiScanner(
    void *context)
{
    (void)context;

    showScreen(
        ScreenId::WiFiScanner);
}

void UIManager::handleShowWiFiSignalMonitor(
    void *context)
{
    (void)context;

    showScreen(
        ScreenId::WiFiSignalMonitor);
}

void UIManager::handleReturnWiFiAccessPoint(
    void *context)
{
    (void)context;

    showScreen(
        ScreenId::WiFiAccessPoint);
}