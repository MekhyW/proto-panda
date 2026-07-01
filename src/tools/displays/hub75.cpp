
#include "tools/displays/hub75.hpp"

void Hub75Display::setPixelWithFlip(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue, FlipConfig& flipSettings) {
    if (!mirrorHalf){
        int xIn;
        int yIn;
        if (!view.getPosition(x, y, xIn, yIn)){
            return;
        }
        matrix->updateMatrixDMABuffer_2(xIn, yIn, red, green, blue);
        return;
    }

    uint8_t ra = red;
    uint8_t ga = green;
    uint8_t ba = blue;

    uint8_t rb = red;
    uint8_t gb = green;
    uint8_t bb = blue;

    BaseDisplay::reorder_rgb(flipSettings.modeLeft, &ra, &ga, &ba);
    

    if (flipSettings.flipLeft){
        int xIn;
        int yIn;
        if (!view.getPosition((halfPosition-1)-x, y, xIn, yIn)){
            return;
        }
        matrix->updateMatrixDMABuffer_2(xIn, yIn, ra, ga, ba);
    }else{
        int xIn;
        int yIn;
        if (!view.getPosition(x, y, xIn, yIn)){
            return;
        }
        matrix->updateMatrixDMABuffer_2(xIn, yIn, ra, ga, ba);
    }

    BaseDisplay::reorder_rgb(flipSettings.modeRight, &rb, &gb, &bb);

    if (flipSettings.flipRight){
        int xIn;
        int yIn;
        if (!view.getPosition((halfPosition+halfPosition-1)-x, y, xIn, yIn)){
            return;
        }
        matrix->updateMatrixDMABuffer_2(xIn, yIn, rb, gb, bb);
    }else{
        int xIn;
        int yIn;
        if (!view.getPosition((halfPosition)+x, y, xIn, yIn)){
            return;
        }
        matrix->updateMatrixDMABuffer_2(xIn, yIn, rb, gb, bb);
    }
    

}