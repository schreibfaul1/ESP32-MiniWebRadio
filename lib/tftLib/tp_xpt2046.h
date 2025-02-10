// first release on 09/2019
// updated on Feb 02 2025

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        T O U C H P A N E L
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        XPT2046
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

//Calibration
//x,y | Ux,Uy  0  ,0     | 1913,1940
//x,y | Ux,Uy  240,0     |  150,1940
//x,y | Ux,Uy  0  ,320   | 1913, 220
//x,y | Ux,Uy  240,320   |  150, 220
// the outcome of this is   x: (1913-150)/240 = 7,3458mV pixel
//                          y: (1944-220)/320 = 5,3875mV pixel

#include "Arduino.h"
#include <SPI.h>

extern __attribute__((weak)) void tp_moved(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y);

class TP_XPT2046{
  private:
    const uint16_t ILI9341_WIDTH  = 240;
    const uint16_t ILI9341_HEIGHT = 320;
    const uint16_t HX8347D_WIDTH  = 240;
    const uint16_t HX8347D_HEIGHT = 320;
    const uint16_t ILI9486_WIDTH  = 320;
    const uint16_t ILI9486_HEIGHT = 480;
    const uint16_t ILI9488_WIDTH  = 320;
    const uint16_t ILI9488_HEIGHT = 480;
    const uint16_t ST7796_WIDTH   = 320;
    const uint16_t ST7796_HEIGHT  = 480;

    SPIClass* spi_TP; // use in class TP_XPT2046
  public:
    TP_XPT2046(SPIClass &spi, int csPin);
    void     begin(uint8_t IRQ);
    void     loop();
    void     setVersion(uint8_t v);
    void     setRotation(uint8_t m);
    void     setMirror(bool h, bool v);
  private:
    uint16_t TP_Send(uint8_t set_val);
    bool     read_TP(uint16_t& x, uint16_t& y);
    SPISettings TP_SPI;
    uint8_t     _TP_CS, _TP_IRQ;
    uint16_t    x=0, y=0;
    uint8_t     _rotation;
    bool        f_loop=false;
    bool        m_mirror_h = false;
    bool        m_mirror_v = false;

    //const uint8_t TP_Dummy=0x80; //nur Startbit für XPT2046
    float xFaktor;
    float yFaktor;

    uint16_t Xmax=1913; // default, will be overwritten in constructor
    uint16_t Xmin=150;
    uint16_t Ymax=1944;
    uint16_t Ymin=220;
    uint8_t  TP_vers = 0;

    uint32_t m_pressingTime = 0;
    bool     m_f_isPressing = false;
    bool     m_f_longPressed = false;

    enum tpVers{TP_ILI9341_0 = 0, TP_ILI9341_1 = 1, TP_HX8347D_0 = 2, TP_ILI9486_0 = 3, TP_ILI9488_0 = 4, TP_ST7796_0 = 5};

  public:

};
