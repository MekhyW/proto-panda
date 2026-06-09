#pragma once
#include <cstdint>
#include <cstring>
#include <Arduino.h>

#define MAX_VIEWS 32

class SectionMap {
public:

    struct View {
        int dispX, dispY;
        int srcX,  srcY;
        int w,     h;
    };

    SectionMap() : m_count(0) {}

    void addView(int dispX, int dispY,
                    int srcX,  int srcY,
                    int w,     int h) {
        if (m_count < MAX_VIEWS) {
            View& v = m_views[m_count++];
            v.dispX = srcX;   v.dispY = srcY;
            v.srcX  = dispX;  v.srcY  = dispY; 
            v.w     = w;      v.h     = h;
        }
    }

    inline bool getPosition(int x, int y,
                            int& tx, int& ty) const {
        if (m_count == 0){
            tx = x;
            ty = y;
            return true;
        }
        const View* v   = m_views;
        const View* end = v + m_count;
        for (; v != end; ++v) {
            int dx = ((int)x) - v->dispX;
            int dy = ((int)y) - v->dispY;
            if (dx < v->w && dy < v->h) {
                tx = v->srcX + dx;
                ty = v->srcY + dy;
                return true;
            }
        }
        return false;
    }

    void clear()       { m_count = 0; }
    int  count() const { return m_count; }

private:
    View m_views[MAX_VIEWS];
    int  m_count;
};