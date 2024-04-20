#include "common.h"

class slider{
private:
    int16_t m_x = 0;
    int16_t m_y = 0;
    int16_t m_w = 0;
    int16_t m_h = 0;
    int16_t m_val = 0;
    uint32_t m_fgColor = 0;
    uint32_t m_bgColor = 0;
    uint32_t m_leftColor = 0;
    uint32_t m_rightColor = 0;
    uint32_t m_slideSpot = 0;
public:
    slider(){
        m_fgColor = TFT_LIGHTGREY;
        m_bgColor = TFT_BLACK;
        m_leftColor = TFT_RED;
        m_rightColor = TFT_GREEN;
        m_slideSpot = TFT_RED;
    }
    ~slider(){

    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, int16_t minVal, int16_t maxVal){
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;
    }
    bool setValue(int16_t val){
        m_val = val;
        return true; // in range?
    }
    int16_t getValue(){
        return m_val;
    }
    void show(){

    }
    void hide(){

    }

};


