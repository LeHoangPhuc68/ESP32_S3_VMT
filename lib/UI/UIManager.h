#pragma once

#include "../Core/App/AppId.h"
#include "../Input/InputManager.h"

#include "Navigation/ScreenId.h"
#include "Screens/BaseScreen.h"

class UIManager final
{
public:
    static bool begin();

    static void update();

    static void handleInput(
        InputManager::Action action);

    static bool showScreen(
        ScreenId screenId);

    static bool showScreen(
        BaseScreen &screen);

    static bool showHome();

    static bool showApp(
        AppId appId);

    static BaseScreen *currentScreen();

    static ScreenId currentScreenId();
    

private:
    static bool handleAppLaunch(
        void *context,
        AppId appId);

    static void handleReturnHome(
        void *context);

    static void handleShowWiFiAccessPoint(
        void *context);

    static void handleReturnWiFiScanner(
        void *context);
        
    static void handleShowWiFiSignalMonitor(
    void *context);

    static void handleReturnWiFiAccessPoint(
        void *context);
};