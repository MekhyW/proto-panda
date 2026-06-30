#pragma once
#include <stdint.h>
#include "tools/psrammap.hpp"
#include "tools/displays.hpp"

class Sprite{
    public:
        Sprite():x(0),y(0),width(0),height(0),pixels(nullptr),transparentColor(0),id(0){};
        uint16_t x,y;
        uint16_t width, height;
        uint16_t *pixels;
        uint16_t transparentColor;
        int id;

        int GetId(){
            return id;
        }

        void SetPosition(uint16_t xa, uint16_t ya){
            x = xa;
            y = ya;
        };

        void SetPixelColor(uint16_t x, uint16_t y, uint16_t color);
        void SetTransparent(uint16_t c){
            transparentColor = c;
        }

        bool Init(uint16_t sizeX, uint16_t sizeY);

        bool LoadSpriteFromPng(std::string name);

        void Draw(FlipConfig& flipSettings);
};

