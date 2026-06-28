#pragma once


#include "tools/sectionview.hpp"
#include "tools/config_default.hpp"



enum ColorMode{
    COLOR_MODE_RGB,
    COLOR_MODE_RBG,
    COLOR_MODE_GRB,
    COLOR_MODE_GBR,
    COLOR_MODE_BRG,
    COLOR_MODE_BGR,
    
};

class FlipConfig {
    public:
        FlipConfig():flipLeft(false),flipRight(true),modeLeft(COLOR_MODE_RGB),modeRight(COLOR_MODE_RGB){}
        FlipConfig(bool left, bool right, ColorMode cLeft, ColorMode cRight):flipLeft(left),flipRight(right),modeLeft(cLeft),modeRight(cRight){}
        bool flipLeft;
        bool flipRight;
        ColorMode modeLeft;
        ColorMode modeRight;

    static FlipConfig DefaultFlipConfig;
};


class BaseDisplay {
public:
    virtual ~BaseDisplay() {}
    
    // Core display methods - empty implementations
    virtual bool begin() { return false; }
    virtual void draw() {}
    virtual void flipDma() {}
    virtual void clearScreen() {}
    virtual void setBrightness8(const uint8_t b) {}
    virtual void setPixelWithFlip(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings) {}
    
    // Graphics methods from Adafruit_GFX - empty implementations
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) {}
    virtual void startWrite() {}
    virtual void endWrite() {}
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {}
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {}
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    virtual void fillScreen(uint16_t color) {}
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {}
    virtual void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {}
    virtual void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {}
    virtual void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {}
    virtual void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {}
    virtual void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {}
    virtual void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {}
    virtual void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {}
    virtual void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) {}
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {}
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) {}
    virtual void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {}
    virtual void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) {}
    virtual void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) {}
    virtual void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) {}
    virtual void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h) {}
    virtual void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {}
    virtual void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) {}
    virtual void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) {}
    virtual void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h) {}
    virtual void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {}
    virtual void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {}
    virtual void setTextSize(uint8_t s) {}
    virtual void setTextSize(uint8_t sx, uint8_t sy) {}
    virtual void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {}
    virtual void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {}
    virtual void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {}
    
    // Additional helper methods
    virtual void drawIcon(int *ico, int16_t x, int16_t y, int16_t cols, int16_t rows) {}
    
    // Brightness control
    void setBrightnessExt(const uint8_t b) { external_brightness = b; }
    uint8_t getBrightnessExt() { return external_brightness; }

    static inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b){
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    static uint16_t color444(uint8_t r, uint8_t g, uint8_t b) { return color565(r * 17, g * 17, b * 17); }

    static void color565to888(const uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
        r = (color >> 8) & 0xf8;
        g = (color >> 3) & 0xfc;
        b = (color << 3);
        r |= r >> 5;
        g |= g >> 6;
        b |= b >> 5;
    }

    static void reorder_rgb(ColorMode mode, uint8_t *r, uint8_t *g, uint8_t *b){
        uint8_t auxr = *r;
        uint8_t auxb = *b;
        uint8_t auxg = *g;
        switch (mode)
        {
        case COLOR_MODE_RGB:
            break;
        case COLOR_MODE_RBG:
            *b = auxg;
            *g = auxb;
            break;
        case COLOR_MODE_GRB:
            *r = auxg;
            *g = auxr;
            break;
        case COLOR_MODE_GBR:
            *g = auxr;
            *b = auxg;
            *r = auxb;
            break;
        case COLOR_MODE_BRG:
            *b = auxr;
            *r = auxg;
            *g = auxb;
            break;
        case COLOR_MODE_BGR:
            *b = auxr;
            *r = auxb;
            break;
        default:
            break;
        }
    }


    
    bool mirrorHalf;
    SectionMap view;
            
protected:
    uint32_t halfPosition;
    uint8_t external_brightness = 0;
};




class EmptyDisplay : public BaseDisplay {
public:
    EmptyDisplay() {}
};