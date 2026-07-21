#include "Display.h"

namespace
{
    constexpr int8_t PinPower = 15;
    constexpr int8_t PinBacklight = 38;

    constexpr int8_t PinDc = 7;
    constexpr int8_t PinCs = 6;
    constexpr int8_t PinWr = 8;
    constexpr int8_t PinRd = 9;

    constexpr int8_t PinD0 = 39;
    constexpr int8_t PinD1 = 40;
    constexpr int8_t PinD2 = 41;
    constexpr int8_t PinD3 = 42;
    constexpr int8_t PinD4 = 45;
    constexpr int8_t PinD5 = 46;
    constexpr int8_t PinD6 = 47;
    constexpr int8_t PinD7 = 48;

    constexpr int8_t PinReset = 5;
    constexpr uint8_t DefaultRotation = 1;

    Arduino_DataBus *displayBus = new Arduino_ESP32PAR8Q(
        PinDc,
        PinCs,
        PinWr,
        PinRd,
        PinD0,
        PinD1,
        PinD2,
        PinD3,
        PinD4,
        PinD5,
        PinD6,
        PinD7);

    Arduino_GFX *displayGfx = new Arduino_ST7789(
        displayBus,
        PinReset,
        0,
        true,
        Display::NativeWidth,
        Display::NativeHeight,
        35,
        0,
        35,
        0);

    uint8_t currentBrightness = 255;

    void initializePower()
    {
        pinMode(PinPower, OUTPUT);
        digitalWrite(PinPower, HIGH);

        pinMode(PinBacklight, OUTPUT);
        digitalWrite(PinBacklight, LOW);
    }
}

bool Display::begin()
{
    initializePower();

    if (!displayGfx->begin())
    {
        return false;
    }

    displayGfx->setRotation(DefaultRotation);
    displayGfx->fillScreen(BLACK);

    setBrightness(currentBrightness);

    return true;
}

void Display::clear(const uint16_t color)
{
    displayGfx->fillScreen(color);
}

void Display::setBrightness(const uint8_t brightness)
{
    currentBrightness = brightness;

    digitalWrite(
        PinBacklight,
        brightness == 0 ? LOW : HIGH);
}

uint16_t Display::width()
{
    return static_cast<uint16_t>(displayGfx->width());
}

uint16_t Display::height()
{
    return static_cast<uint16_t>(displayGfx->height());
}

void Display::drawRgb565Bitmap(
    const int16_t x,
    const int16_t y,
    const uint16_t *pixels,
    const uint16_t width,
    const uint16_t height)
{
    if (pixels == nullptr || width == 0 || height == 0)
    {
        return;
    }

    displayGfx->draw16bitRGBBitmap(
        x,
        y,
        const_cast<uint16_t *>(pixels),
        width,
        height);
}

Arduino_GFX &Display::gfx()
{
    return *displayGfx;
}
