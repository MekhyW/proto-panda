#include "tools/displays.hpp"
#pragma once

#include <FastLED.h>


class WS2812BDisplay : public BaseDisplay {
public:
    WS2812BDisplay(uint16_t w, uint16_t h, uint16_t p, CRGB *leds) {
        mirrorHalf = false;
        m_leds = leds;
        m_width = w;
        m_height = h;
        m_panels = p;
        m_realWidth = p*w;
        halfPosition = m_realWidth/2;
    }
    
    ~WS2812BDisplay() {
    }
    
    bool begin() override;
    void draw() override { flipDma(); }
    void flipDma() override;
    void clearScreen() override;
    void setBrightness8(const uint8_t b) override;
    void setPixelWithFlip(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings) override;
    void startWrite() override {}
    void endWrite() override {}


private:
    int GetLEDIndex(uint16_t x, uint16_t y);
    uint32_t m_width, m_height, m_panels, m_realWidth;
    CRGB *m_leds;
};

