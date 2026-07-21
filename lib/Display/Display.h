#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

namespace Display
{
    constexpr uint16_t NativeWidth = 170;
    constexpr uint16_t NativeHeight = 320;

    bool begin();

    void clear(uint16_t color = BLACK);

    void setBrightness(uint8_t brightness);

    uint16_t width();

    uint16_t height();

    void drawRgb565Bitmap(
        int16_t x,
        int16_t y,
        const uint16_t *pixels,
        uint16_t width,
        uint16_t height);

    Arduino_GFX &gfx();
}
