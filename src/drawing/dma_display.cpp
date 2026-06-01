#include "tools/dma_display.hpp"
#include "tools/logger.hpp"

SPIClass spi3(FSPI);

bool MAX7219Display::begin(){

    pinMode(m_csPin, OUTPUT);
    digitalWrite(m_csPin, LOW);

    spi3.setFrequency(SPI_CLOCK_DIV2);
    spi3.setDataMode(SPI_MODE0);
    spi3.setBitOrder(MSBFIRST);
    spi3.begin(m_clockPin, -1, m_dataInPin, m_csPin);

    clearScreen();

    for(uint8_t i = 0; i < 20; i++) initialize();
     setBrightness8(32);


    updateMatrixDMABuffer_2(1, 2, 255, 255, 255);

    flipDma();
    Logger::Error("send shit");

    return true;
}
void MAX7219Display::setBrightness8(const uint8_t b){
    transferAll(0x0a, b);
    return;
}

void MAX7219Display::clearScreen(){
    memset(m_frameBuffer, 0, sizeof(uint8_t) * 8 * 8 * m_panels);
    return;
}

void MAX7219Display::transferAll(uint8_t address, uint8_t data) {
    digitalWrite(m_csPin, LOW);
    for (int i = 0; i <= m_panels; i++) {  // reverse order
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

void MAX7219Display::updateMatrixDMABuffer_2(uint16_t xIn, uint16_t yIn, uint8_t red, uint8_t green, uint8_t blue) {

    int x = xIn;
    int y = yIn;

    y = y - 18;
    
    if (y >= 8 || x >= m_lenght) return;
    if (y < 0|| x < 0) return;

    uint16_t bufferIndex = y + (x / 8) * 8;
    uint8_t bitPosition = 7 - (x % 8);

    if (red > 128 || green > 128 || blue > 128) {
        m_frameBuffer[bufferIndex] |= (1 << bitPosition);
    } else {
        m_frameBuffer[bufferIndex] &= ~(1 << bitPosition);
    }
}

void MAX7219Display::flipDma() {
    for (uint8_t row = 0; row < 8; row++) {
        uint8_t reg = row + 1;
        digitalWrite(m_csPin, LOW);
        
        for (int panel = 0; panel < m_panels; panel++) {  // 0 → n-1 instead of n-1 → 0
            uint16_t bufferIndex = row + panel * 8;
            transfer(reg, m_frameBuffer[bufferIndex]);
        }
        
        digitalWrite(m_csPin, HIGH);
    }
}