
#include "tools/displays/ws2812.hpp"
#include "tools/oledscreen.hpp"

bool WS2812BDisplay::begin(){
    return true;
};


void WS2812BDisplay::flipDma(){

    FlipConfig f = FlipConfig();
    setPixelWithFlip(31, 7, 255, 0, 0,  f);
    setPixelWithFlip(16, 0, 0, 255, 0,  f);

    setPixelWithFlip(0, 0, 255, 0, 0,  f);
    setPixelWithFlip(1, 0, 0, 255, 0,  f);

    FastLED.show(); 
    FastLED.delay(5); 

};

void WS2812BDisplay::clearScreen(){
    for (int a=0;a<(m_width*m_height*m_panels);++a){
        m_leds[a] = CRGB(0,0,0);
    }
};

void WS2812BDisplay::setBrightness8(const uint8_t b){
    FastLED.setBrightness(b);
};

int WS2812BDisplay::GetLEDIndex(uint16_t x, uint16_t y) {
    int xIn = 0;
    int yIn = 0;
    if (!view.getPosition(x, y, xIn, yIn)) {
        return -1;
    }

    if (!mirrorHalf) {
        if (xIn >= m_realWidth) {
            return -1;
        }
    } else {
        if (xIn >= m_realWidth / 2) {
            return -1;
        }
    }
    if (yIn >= m_height) {
        return -1;
    }

    int matrixIndex = xIn / m_width;
    if (matrixIndex >= m_panels) {   
        return -1;
    }
    int xInPanel = xIn - matrixIndex * m_width;

    int localY;
    if (xInPanel % 2 == 0) {
        localY = yIn;
    } else {
        localY = (m_height - 1) - yIn;
    }

    int idx = (matrixIndex * m_width * m_height) + (xInPanel * m_height) + localY;


    if (idx < 0 || idx >= m_ledCount) {
        return -1;
    }
    return idx;
}

void WS2812BDisplay::setPixelWithFlip(int16_t xIn, int16_t yIn, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings){
    int idx;
    OledScreen::MarkPixel(xIn, yIn, red, green, blue);
    uint8_t ra = red;
    uint8_t ga = green;
    uint8_t ba = blue;


    BaseDisplay::reorder_rgb(flipSettings.modeLeft, &ra, &ga, &ba);


    if (!mirrorHalf){
        idx = GetLEDIndex(xIn, yIn);
        if (idx <= -1){
            return;
        }
        m_leds[idx] = CRGB(ra, ga, ba);
        return;
    }



    if (flipSettings.flipLeft){
        idx = GetLEDIndex((halfPosition-1)-xIn, yIn);
        if (idx <= -1){
            return;
        }
        m_leds[idx] = CRGB(ra, ga, ba);
    }else{
        idx = GetLEDIndex(xIn, yIn);
        if (idx <= -1){
            return;
        }
        m_leds[idx] = CRGB(ra, ga, ba);
    }

    
    uint8_t rb = red;
    uint8_t gb = green;
    uint8_t bb = blue;

    BaseDisplay::reorder_rgb(flipSettings.modeRight, &rb, &gb, &bb);

    if (flipSettings.flipRight){
        idx = GetLEDIndex( (halfPosition+halfPosition-1)-xIn, yIn);
        if (idx <= -1){
            return;
        }
        m_leds[idx] = CRGB(rb, gb, bb);
    }else{
        idx = GetLEDIndex(halfPosition-xIn, yIn);
        if (idx <= -1){
            return;
        }
        m_leds[idx] = CRGB(rb, gb, bb);
    }    
};
