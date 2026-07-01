#include "tools/displays/max7219.hpp"
#include <SPI.h>

SPIClass spi3(FSPI);

bool MAX7219Display::begin(){
    pinMode(m_csPin, OUTPUT);
    digitalWrite(m_csPin, LOW);

    spi3.setFrequency(SPI_CLOCK_DIV2);
    spi3.setDataMode(SPI_MODE0);
    spi3.setBitOrder(MSBFIRST);
    spi3.begin(m_clockPin, -1, m_dataInPin, m_csPin);

    clearScreen();

    for(uint8_t i = 0; i < 20; i++){
        initialize();
    }
    
    setBrightness8(0);

    flipDma();
    transferAll(0x0a, 64);
    return true;
}

void MAX7219Display::setBrightness8(const uint8_t b){
    //transferAll(0x0a, b);
    return;
}

void MAX7219Display::clearScreen(){
    memset(m_frameBuffer, 0, sizeof(uint8_t) * 8 * 8 * m_panels);
    return;
}

void MAX7219Display::transferAll(uint8_t address, uint8_t data) {
    digitalWrite(m_csPin, LOW);
    for (int i = 0; i < m_panels; i++) {
        transfer(address, data);
    }
    digitalWrite(m_csPin, HIGH);
}

void MAX7219Display::initialize() {
    transferAll(0x0b, 0x07);  // scan limit
    transferAll(0x09, 0x00);  // decode mode
    transferAll(0x0c, 0x01);  // shutdown
    transferAll(0x0f, 0x00);  // display test
}

void MAX7219Display::transfer(uint8_t address, uint8_t data) {
    spi3.transfer(address);
    spi3.transfer(data);
}

void MAX7219Display::setPixelWithFlip(int16_t xIn, int16_t yIn, uint8_t red, uint8_t green, uint8_t blue, FlipConfig& flipSettings) {
    uint8_t color = 0;
    if (red > 128 || green > 128 || blue > 128) {
        color = 1;
    }

    if (!mirrorHalf){
        setPixelAt(xIn, yIn, color);
        return;
    }

    if (xIn >= halfPosition){
        return;
    }

    int x = xIn;
    int y = yIn;
    if (!view.getPosition(xIn, y, x, y)){
        return;
    }
    
    if (flipSettings.flipLeft){
        setPixelAt((halfPosition-1)-x, y, color);
    }else{
        setPixelAt(x, y, color);
    }

    if (flipSettings.flipRight){
        setPixelAt((m_lenght-1)-x, y, color);
    }else{
        setPixelAt((halfPosition)+x, y, color);
    }
}

void MAX7219Display::setPixelAt(uint16_t x, uint16_t y, uint8_t color) {


    // Check bounds
    if (x >= m_width || y >= m_height) {
        return;
    }
       
    // Calculate which panel this pixel belongs to (2D layout)
    uint16_t panelRow = y / MAX7219_SIZE;
    uint16_t panelCol = x / MAX7219_SIZE;
    uint16_t panelIndex = (panelRow * m_horizontalPanels) + panelCol;
    
    // Calculate position within the panel
    uint16_t xInPanel = x % MAX7219_SIZE;
    uint16_t yInPanel = y % MAX7219_SIZE;
    
    // Buffer index: row within panel + (panelIndex * 8 rows)
    uint16_t bufferIndex = yInPanel + (panelIndex * MAX7219_SIZE);
    uint8_t bitPosition = 7 - xInPanel;

    if (color != 0) {
        m_frameBuffer[bufferIndex] |= (1 << bitPosition);
    } else {
        m_frameBuffer[bufferIndex] &= ~(1 << bitPosition);
    }
}

void MAX7219Display::flipDma() {
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t reg = row + 1;
        digitalWrite(m_csPin, LOW);
        for (int panel = 0; panel < m_panels; panel++) {
            uint16_t bufferIndex = row + panel * 8;
            transfer(reg, m_frameBuffer[bufferIndex]);
        }
        digitalWrite(m_csPin, HIGH);
    }
}