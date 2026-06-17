#include "tools/displays.hpp"
#pragma once


class MAX7219Display : public BaseDisplay {
public:
    MAX7219Display(uint32_t panels, uint8_t horizontal_panel_count, int csPin, int dataInPin, int clockPin) {
        if (horizontal_panel_count == 0){
            horizontal_panel_count = panels;
        }
        m_frameBuffer = new uint8_t[MAX7219_SIZE * MAX7219_SIZE * panels];
        m_width = MAX7219_SIZE * horizontal_panel_count; 
        m_height = MAX7219_SIZE * (panels / horizontal_panel_count);
        m_panels = panels;
        m_lenght = MAX7219_SIZE * horizontal_panel_count;
        m_horizontalPanels = horizontal_panel_count;
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
    uint32_t m_width, m_height, m_panels, m_lenght, m_horizontalPanels;
    int m_csPin, m_dataInPin, m_clockPin;
};
