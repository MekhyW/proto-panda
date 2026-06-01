#pragma once
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

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

class MatrixPanel_I2S_DMA2 : public MatrixPanel_I2S_DMA{
    public:
        MatrixPanel_I2S_DMA2(const HUB75_I2S_CFG &mxconfig):MatrixPanel_I2S_DMA(mxconfig){}
  
        virtual void flipDma(){ flipDMABuffer();};
        inline void updateMatrixDMABuffer_2(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){
            updateMatrixDMABuffer(x, y, red, green, blue);
        }
};

// Base class with empty implementations (not abstract)
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
            
protected:
    uint32_t halfPosition;
    uint8_t external_brightness = 0;
};

// HUB75 Display wrapper (using composition instead of inheritance)
class Hub75Display : public BaseDisplay {
private:
    MatrixPanel_I2S_DMA2* matrix;
    HUB75_I2S_CFG config;
    
public:
    Hub75Display(const HUB75_I2S_CFG &mxconfig) : config(mxconfig), matrix(nullptr) {}
    
    ~Hub75Display() {
        delete matrix;
        matrix = nullptr;
    }
    
    bool begin() override {
        mirrorHalf = true;
        matrix = new MatrixPanel_I2S_DMA2(config);
        halfPosition = (config.mx_width * config.chain_length)/2;
        return matrix->begin();
    }
    
    void draw() override {
        matrix->flipDMABuffer();
    }
    
    void flipDma() override {
        matrix->flipDMABuffer();
    }
    
    void clearScreen() override {
        matrix->fillScreen(0);
    }
    
    void setBrightness8(const uint8_t b) override {
        matrix->setBrightness8(b);
    }
    
    void setPixelWithFlip(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings) override;
    
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        matrix->drawPixel(x, y, color);
    }
    
    void startWrite() override {
        matrix->startWrite();
    }
    
    void endWrite() override {
        matrix->endWrite();
    }
    
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override {
        matrix->drawFastVLine(x, y, h, color);
    }
    
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override {
        matrix->drawFastHLine(x, y, w, color);
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        matrix->fillRect(x, y, w, h, color);
    }
    
    void fillScreen(uint16_t color) override {
        matrix->fillScreen(color);
    }
    
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override {
        matrix->drawLine(x0, y0, x1, y1, color);
    }
    
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override {
        matrix->drawRect(x, y, w, h, color);
    }
    
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override {
        matrix->drawCircle(x0, y0, r, color);
    }
    
    void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) override {
        matrix->drawCircleHelper(x0, y0, r, cornername, color);
    }
    
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override {
        matrix->fillCircle(x0, y0, r, color);
    }
    
    void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) override {
        matrix->fillCircleHelper(x0, y0, r, cornername, delta, color);
    }
    
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override {
        matrix->drawTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override {
        matrix->fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    
    void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override {
        matrix->drawRoundRect(x0, y0, w, h, radius, color);
    }
    
    void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color) override {
        matrix->fillRoundRect(x0, y0, w, h, radius, color);
    }
    
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override {
        matrix->drawBitmap(x, y, bitmap, w, h, color);
    }
    
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg) override {
        matrix->drawBitmap(x, y, bitmap, w, h, color, bg);
    }
    
    void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) override {
        matrix->drawXBitmap(x, y, bitmap, w, h, color);
    }
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h) override {
        matrix->drawGrayscaleBitmap(x, y, bitmap, w, h);
    }
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h) override {
        matrix->drawGrayscaleBitmap(x, y, bitmap, w, h);
    }
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override {
        matrix->drawGrayscaleBitmap(x, y, bitmap, mask, w, h);
    }
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override {
        matrix->drawGrayscaleBitmap(x, y, bitmap, mask, w, h);
    }
    
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) override {
        matrix->drawRGBBitmap(x, y, bitmap, w, h);
    }
    
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) override {
        matrix->drawRGBBitmap(x, y, bitmap, w, h);
    }
    
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h) override {
        matrix->drawRGBBitmap(x, y, bitmap, mask, w, h);
    }
    
    void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h) override {
        matrix->drawRGBBitmap(x, y, bitmap, mask, w, h);
    }
    
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override {
        matrix->drawChar(x, y, c, color, bg, size);
    }
    
    void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) override {
        matrix->drawChar(x, y, c, color, bg, size_x, size_y);
    }
    
    void setTextSize(uint8_t s) override {
        matrix->setTextSize(s);
    }
    
    void setTextSize(uint8_t sx, uint8_t sy) override {
        matrix->setTextSize(sx, sy);
    }
    
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override {
        matrix->getTextBounds(string, x, y, x1, y1, w, h);
    }
    
    void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override {
        matrix->getTextBounds(s, x, y, x1, y1, w, h);
    }
    
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) override {
        matrix->getTextBounds(str, x, y, x1, y1, w, h);
    }
    
    void drawIcon(int *ico, int16_t x, int16_t y, int16_t cols, int16_t rows) override {
        // Implement icon drawing if needed
    }

};

#define MAX7219_SIZE 8

// MAX7219Display implementation
class MAX7219Display : public BaseDisplay {
public:
    MAX7219Display(uint32_t panels, int csPin, int dataInPin, int clockPin) {
        m_frameBuffer = new uint8_t[MAX7219_SIZE * MAX7219_SIZE * panels];
        m_width = MAX7219_SIZE * panels;
        m_height = MAX7219_SIZE;
        m_panels = panels;
        m_lenght = MAX7219_SIZE * panels;
        m_csPin = csPin;
        m_dataInPin = dataInPin;
        m_clockPin = clockPin;
        halfPosition = m_lenght/2;
        mirrorHalf = false;
    }
    
    ~MAX7219Display() {
        delete[] m_frameBuffer;
    }


    
    bool begin() override;
    void draw() override { flipDma(); }
    void flipDma() override;
    void clearScreen() override;
    void setBrightness8(const uint8_t b) override;
    void setPixelWithFlip(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings) override;
    void startWrite() override {}
    void endWrite() override {}

    
    // Override only what you need, the rest will use BaseDisplay's empty implementations
    
private:
    void transfer(uint8_t address, uint8_t data);
    void initialize();
    void transferAll(uint8_t address, uint8_t data);
    void setPixelAt(uint16_t xIn, uint16_t yIn, uint8_t color);
    
    uint8_t *m_frameBuffer;
    uint32_t m_width, m_height, m_panels, m_lenght;
    int m_csPin, m_dataInPin, m_clockPin;
};

class EmptyDisplay : public BaseDisplay {
public:
    EmptyDisplay() {}
};