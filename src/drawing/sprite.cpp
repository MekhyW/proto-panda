#include "drawing/sprite.hpp"
#include "tools/storage.hpp"
#include "tools/devices.hpp"


BasicTexture::BasicTexture(uint16_t sizeX, uint16_t sizeY){
    width = sizeX;
    height = sizeY;
    transparentColor = 0;
    pixels = (uint16_t*)ps_malloc(sizeof(uint16_t) * width * height);
    if (!pixels){
        return;
    }
}

int Sprite::CreateEmptySprite(uint16_t sizeX, uint16_t sizeY){
    BasicTexture* mem = (BasicTexture*)ps_malloc(sizeof(BasicTexture));
    if (!mem) {
        return -1;
    }
    #ifndef __INTELLISENSE__
    new (mem) BasicTexture(sizeX, sizeY);
    #endif

    if (mem->pixels == nullptr){
        heap_caps_free(mem);
        return -1;
    }
    int len = frames.size();
    frames.emplace_back(mem);

    if (w == 0){
        w = mem->width;
    }
    if (h == 0){
        h = mem->height;
    }
    return len;
}

int Sprite::LoadFromPng(std::string name){
    int rcError;
    size_t width_p, height_p;
    uint16_t *pixels = Storage::DecodePNG(name.c_str(), rcError, width_p, height_p);
    if (rcError != 0){
        return -1;
    }

    BasicTexture* mem = (BasicTexture*)ps_malloc(sizeof(BasicTexture));
    if (!mem) {
        heap_caps_free(mem);
        return -1;
    }
    #ifndef __INTELLISENSE__
    new (mem) BasicTexture();
    #endif

    mem->width = width_p;
    mem->height = height_p;
    mem->pixels = pixels;

    if (w == 0){
        w = mem->width;
    }
    if (h == 0){
        h = mem->height;
    }

    int len = frames.size();
    frames.emplace_back(mem);
    return len;
}

void Sprite::SetPixelColor(int idx, uint16_t x, uint16_t y, uint16_t color){
    if (idx < 0 || idx > frames.size()){
        return;
    }
    BasicTexture *tx = frames[idx];
    if (x >= tx->width){
        return;
    }
    if (y >= tx->height){
        return;
    }
    tx->pixels[x + y*tx->width] = color;
}

void Sprite::Draw(FlipConfig& flipSettings){
    if (currentFrame < 0 || currentFrame > frames.size()){
        return;
    }

    int targetW = w;
    int targetH = h;
    BasicTexture *tx = frames[currentFrame];
    if (targetW > tx->width){
        targetW = tx->width;
    }
    if (targetH > tx->height){
        targetH = tx->height;
    }
    for (int dy=0;dy<targetH;dy++){
        for (int dx=0;dx<targetW;dx++){

            int xIn;
            int yIn;
            if (!view.getPosition(dx, dy, xIn, yIn)){
                continue;;
            }
            if (xIn < 0 || xIn >= tx->width || yIn >= tx->height || yIn < 0){
                continue;
            }

            uint16_t color = tx->pixels[yIn * tx->width + xIn];
            if (color == tx->transparentColor){
                continue;
            }
            uint8_t r;
            uint8_t g;
            uint8_t b;
            BaseDisplay::color565to888(color, r,g,b);
            Devices::Display->setPixelWithFlip(x+dx, y+dy, r,g,b, flipSettings);            
        }
    }
}