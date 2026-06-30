#pragma once
#include <stdint.h>
#include "tools/psrammap.hpp"
#include "tools/displays.hpp"
#include "tools/sectionview.hpp"

class BasicTexture{
    public:
        BasicTexture():pixels(nullptr),width(0),height(0),transparentColor(0){}
        BasicTexture(uint16_t w, uint16_t h);
        uint16_t *pixels;
        uint16_t width, height;
        uint16_t transparentColor;
};

class Sprite{
    public:
        Sprite():x(0),y(0),w(0),h(0),frames(),id(0),currentFrame(-1){};
        uint16_t x,y;
        uint16_t w,h;
        SectionMap<1> view;
        
        std::vector<BasicTexture*> frames;
        
        int id;
        int currentFrame;

        bool SetFrameId(int id){
            if (id < 0 || id > frames.size()){
                return false;
            }
            currentFrame = id;
            return true;
        }
        int GetFrameId(){
            return currentFrame;
        }
        int GetFrameCount(){
            return frames.size();
        }

        int GetId(){
            return id;
        };
        uint16_t GetWidth(int id){ 
            if (id < 0 || id > frames.size()){
                return 0;
            }
            return frames[id]->width;
        };
        uint16_t GetHeight(int id){
            if (id < 0 || id > frames.size()){
                return 0;
            } 
            return frames[id]->height;
        };

        void SetPosition(uint16_t xa, uint16_t ya){
            x = xa;
            y = ya;
        };

        void SetPixelColor(int id, uint16_t x, uint16_t y, uint16_t color);
        void SetTransparent(uint16_t c){
            if (id < 0 || id > frames.size()){
                return;
            }
            frames[id]->transparentColor = c;
        }

        void CropSprite(int srcX,  int srcY, int pw, int ph, bool fliph, bool flipv){
            w = w;
            h = h;
            if (view.count() == 0){
                view.addView(0, 0, srcX,  srcY, pw, ph, fliph, flipv);
                return;
            }
            view.setView(0, 0, 0, srcX,  srcY, pw, ph, fliph, flipv);
        }

        int CreateEmptySprite(uint16_t sizeX, uint16_t sizeY);

        int LoadFromPng(std::string name);

        void Draw(FlipConfig& flipSettings);
};

