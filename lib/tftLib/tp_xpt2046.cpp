// first release on 09/2019
// updated on Feb 02 2025

#include "tp_xpt2046.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        XPT2046
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Code für Touchpad mit XPT2046
TP_XPT2046::TP_XPT2046(SPIClass &spi, int csPin){
    _TP_CS = csPin;
    _rotation = 0;
    spi_TP = &spi;
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::begin(uint8_t IRQ, uint16_t h_resolution, uint16_t v_resolution) {
    _TP_IRQ = IRQ;
    m_h_resolution = h_resolution;
    m_v_resolution = v_resolution;
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
    pinMode(_TP_IRQ, INPUT);
    TP_SPI = SPISettings(200000, MSBFIRST, SPI_MODE0);
    _rotation = 0;
    TP_Send(0xD0);
    TP_Send(0x90); // Remove any blockage
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TP_XPT2046::TP_Send(uint8_t set_val) {
    uint16_t get_val;
    spi_TP->beginTransaction(TP_SPI); // Prevent other SPI users
    digitalWrite(_TP_CS, 0);
    spi_TP->write(set_val);
    get_val = spi_TP->transfer16(0);
    digitalWrite(_TP_CS, 1);
    spi_TP->endTransaction(); // Allow other SPI users
    return get_val >> 4;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::loop() {
    static uint16_t x1 = 0, y1 = 0;
    static uint16_t x2 = 0, y2 = 0;
    if (!digitalRead(_TP_IRQ)) {
        if (!read_TP(x, y)) { return; }

        if (x != x1 && y != y1) {
            if (tp_moved) tp_moved(x, y);
        }

        {
            x1 = x;
            y1 = y;
        }

        if (f_loop) {
            f_loop = false;
            // log_i("tp_pressed x=%d, y=%d", x, y);
            if (tp_pressed) tp_pressed(x, y);
            x2 = x;
            y2 = y;
            m_pressingTime = millis();
            m_f_isPressing = true;
        } else {
            if (m_f_isPressing) {
                if (m_pressingTime + 2000 < millis()) {
                    m_f_isPressing = false;
                    if (tp_long_pressed) tp_long_pressed(x2, y2);
                    m_f_longPressed = true;
                }
            }
        }
    } else {
        if (f_loop == false) {
            // log_i("tp_released");
            if (m_f_longPressed) {
                m_f_longPressed = false;
                if (tp_long_released) tp_long_released(x1, y1);
            } else {
                if (tp_released) tp_released(x1, y1);
            }
            f_loop = true;
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::setRotation(uint8_t m) { _rotation = m; }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::setMirror(bool h, bool v) {
    m_mirror_h = h;
    m_mirror_v = v;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::setVersion(uint8_t v) {

    switch (v) {
        case 0: // ILI9341 display
            TP_vers = TP_ILI9341_0;
            Xmax = 1913; // Values Calibration
            Xmin = 150;
            Ymax = 1944;
            Ymin = 220;
            break;
        case 1: // ILI9341 display for RaspberryPI  #70
            TP_vers = TP_ILI9341_1;
            Xmax = 1940;
            Xmin = 90;
            Ymax = 1864;
            Ymin = 105;
            break;
        case 2: // Waveshare HX8347D display
            TP_vers = TP_HX8347D_0;
            Xmax = 1850;
            Xmin = 170;
            Ymax = 1880;
            Ymin = 140;
            break;
        case 3: // ILI9486 display for RaspberryPI
            TP_vers = TP_ILI9486_0;
            Xmax = 1922;
            Xmin = 140;
            Ymax = 1930;
            Ymin = 125;
            break;
        case 4: // ILI9488 display
            TP_vers = TP_ILI9488_0;
            Xmax = 1922;
            Xmin = 140;
            Ymax = 1930;
            Ymin = 125;
            break;
        case 5: // ST7796 4" display
            TP_vers = TP_ST7796_0;
            Xmax = 1922;
            Xmin = 103;
            Ymax = 1950;
            Ymin = 110;
            break;
    }
    xFaktor = float(Xmax - Xmin) / m_v_resolution;
    yFaktor = float(Ymax - Ymin) / m_h_resolution;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TP_XPT2046::read_TP(uint16_t& x, uint16_t& y) {
    uint32_t _y = 0;
    uint32_t _x = 0;
    uint16_t tmpxy;
    uint8_t  i = 0;

    if(digitalRead(_TP_IRQ)) return false; // TP_XPT2046 pressed?

    for(i = 0; i < 100; i++) {
        _x += TP_Send(0xD0); // x
        _y += TP_Send(0x90); // y
    }

    if(digitalRead(_TP_IRQ)) return false; // TP_XPT2046 must remain pressed as long as the measurement is running

    _x /= 100;
    _y /= 100;

    // log_w("_x %i, _y %i", _x, _y);

    if((_x < Xmin) || (_x > Xmax)) { return false; } // outside the display
    _x = Xmax - _x;
    _x /= xFaktor;

    if((_y < Ymin) || (_y > Ymax)) { return false; } // outside the display
    _y = Ymax - _y;
    _y /= yFaktor;

    if (m_mirror_h) { _y = m_h_resolution - _y; }
    if (m_mirror_v) { _x = m_v_resolution - _x; }

    // log_i("_x %i, _y %i", _x, _y);
    x = _x;
    y = _y;

    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_0) { // 320px x 240px
        if(_rotation == 0) { y = m_h_resolution - y; }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = m_v_resolution - y;
            x = m_h_resolution - x;
        }
        if(_rotation == 2) { x = m_v_resolution - x; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_1) { // 320px x 240px
        if(_rotation == 0) {
            y = m_h_resolution - y;
            x = m_v_resolution - x;
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            x = m_h_resolution - x;
        }
        if(_rotation == 2) { ; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = m_v_resolution - y;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_HX8347D_0) { // 320px x 240px
        if(_rotation == 0) {
            ; // do nothing
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = m_v_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
        if(_rotation == 2) {
            x = m_v_resolution - x;
            y = m_h_resolution - y;
            if(x > m_v_resolution - 1) x = 0;
            if(y > m_h_resolution - 1) y = 0;
        }
        if(_rotation == TP_ILI9486_0) {
            tmpxy = y;
            y = x;
            x = m_h_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9486_0) { // 480px x 320px
        if(_rotation == 0) {
            ; // do nothing
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = m_v_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
        if(_rotation == 2) {
            x = m_v_resolution - x;
            y = m_h_resolution - y;
            if(x > m_v_resolution - 1) x = 0;
            if(y > m_h_resolution - 1) y = 0;
        }
        if(_rotation == 3) {
            tmpxy = y;
            y = x;
            x = m_h_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9488_0) { // ILI 9488 Display V1.0, 480px x 320px
        if(_rotation == 0) { x = m_v_resolution - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = m_h_resolution - y;
            if(x > m_v_resolution - 1) x = 0;
            if(y > m_h_resolution - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = m_h_resolution - y;
            y = m_v_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ST7796_0) { // ST7796 Display V1.1, 480px x 320px
        if(_rotation == 0) { x = m_v_resolution - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = m_h_resolution - y;
            if(x > m_v_resolution - 1) x = 0;
            if(y > m_h_resolution - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = m_h_resolution - y;
            y = m_v_resolution - tmpxy;
            if(x > m_h_resolution - 1) x = 0;
            if(y > m_v_resolution - 1) y = 0;
        }
    }
    //    log_i("TP_vers %d, Rotation %d, X = %i, Y = %i",TP_vers, _rotation, x, y);
    return true;
}
