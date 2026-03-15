// first release on 09/2019
// updated on Feb 02 2025

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//         T O U C H P A N E L
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//         XPT2046
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Calibration
// x,y | Ux,Uy  0  ,0     | 1913,1940
// x,y | Ux,Uy  240,0     |  150,1940
// x,y | Ux,Uy  0  ,320   | 1913, 220
// x,y | Ux,Uy  240,320   |  150, 220
//  the outcome of this is   x: (1913-150)/240 = 7,3458mV pixel
//                           y: (1944-220)/320 = 5,3875mV pixel

#include "Arduino.h"
#include <SPI.h>

extern __attribute__((weak)) void tp_moved(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y);

class TP_XPT2046 {
  private:
    SPIClass* spi_TP; // use in class TP_XPT2046
  public:
    TP_XPT2046(SPIClass& spi, int csPin);
    void begin(uint8_t IRQ, uint16_t h_resolution, uint16_t v_resolution);
    void loop();
    void setSize(uint8_t s);
    void setRotation(uint8_t m);

  private:
    uint16_t    TP_Send(uint8_t set_val);
    inline void mapRotation(uint8_t rot, int32_t srcX, int32_t srcY, int32_t& dstX, int32_t& dstY) const;
    bool        read_TP(int16_t& x, int16_t& y);
    SPISettings TP_SPI;
    uint8_t     _TP_CS, _TP_IRQ;
    int16_t     x = 0, y = 0;
    uint16_t    m_h_res = 320, m_v_res = 240;
    uint8_t     m_rotation;
    bool        f_loop = false;
    uint8_t     TP_vers = 0;
    uint32_t    m_pressingTime = 0;
    bool        m_f_isPressing = false;
    bool        m_f_longPressed = false;

    struct cal {
        int32_t xMax;
        int32_t xMin;
        int32_t yMax;
        int32_t yMin;
    };

    const cal m_cal_28 = {.xMax = 1923, .xMin = 155, .yMax = 1890, .yMin = 140}; // 2.8 inch
    const cal m_cal_35 = {.xMax = 1970, .xMin = 114, .yMax = 1923, .yMin = 96}; // 3.5 inch
    const cal m_cal_40 = {.xMax = 1946, .xMin = 110, .yMax = 1907, .yMin = 108}; // 4.0 inch

    cal m_cal = {0};
};
