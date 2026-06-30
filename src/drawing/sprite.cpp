#include "drawing/sprite.hpp"
#include "tools/storage.hpp"
#include "tools/devices.hpp"


bool Sprite::Init(uint16_t sizeX, uint16_t sizeY){
    width = sizeX;
    height = sizeY;
    pixels = (uint16_t*)ps_malloc(sizeof(uint16_t) * width * height);
    if (!pixels){
        return false;
    }
    return true;
}

bool Sprite::LoadSpriteFromPng(std::string name){
    if (pixels){
        heap_caps_free(pixels);
        pixels = nullptr;
    } 
    int rcError;
    size_t width_p, height_p;
    pixels = Storage::DecodePNG(name.c_str(), rcError, width_p, height_p);
    if (rcError != 0){
        return false;
    }

    width = width_p; 
    height = height_p;

    return true;
}

void Sprite::SetPixelColor(uint16_t x, uint16_t y, uint16_t color){
    if (x >= width){
        return;
    }
    if (y >= height){
        return;
    }
    pixels[x + y*width] = color;
}

void Sprite::Draw(FlipConfig& flipSettings){
    int pixelId = 0;
    for (int dy=0;dy<width;dy++){
        for (int dx=0;dx<width;dx++){
            uint16_t color = pixels[pixelId];
            pixelId++;
            if (color == transparentColor){
                continue;
            }
            uint8_t r;
            uint8_t g;
            uint8_t b;
            BaseDisplay::color565to888(color, r,g,b);
            Devices::Display->setPixelWithFlip(dx, dy, r,g,b, flipSettings);            
        }
    }
}