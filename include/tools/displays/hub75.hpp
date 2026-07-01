#include "tools/displays.hpp"
#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

class MatrixPanel_I2S_DMA2 : public MatrixPanel_I2S_DMA{
    public:
        MatrixPanel_I2S_DMA2(const HUB75_I2S_CFG &mxconfig):MatrixPanel_I2S_DMA(mxconfig){}
  
        virtual void flipDma(){ flipDMABuffer();};
        inline void updateMatrixDMABuffer_2(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){
            updateMatrixDMABuffer(x, y, red, green, blue);
        }
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
    
    void setPixelWithFlip(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue,  FlipConfig &flipSettings) override;
    
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

