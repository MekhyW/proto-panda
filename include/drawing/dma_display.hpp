#pragma once
#include "config.hpp"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

class MatrixPanel_I2S_DMA_2 : public MatrixPanel_I2S_DMA{
    public:
        MatrixPanel_I2S_DMA_2(const HUB75_I2S_CFG &mxconfig):MatrixPanel_I2S_DMA(mxconfig){}

        inline void updateMatrixDMABuffer_2(uint16_t x, uint16_t y, uint8_t red, uint8_t green, uint8_t blue){
            updateMatrixDMABuffer(x, y, red, green, blue);
        }
};

class DMADisplay{
    public:
        static bool Start();
        static void DrawTestScreen();


        static MatrixPanel_I2S_DMA_2 *Display ;
    private:
        static HUB75_I2S_CFG mxconfig;
};



