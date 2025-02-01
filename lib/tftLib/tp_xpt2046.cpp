#include "tp_xpt2046.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        XPT2046
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————



// Code für Touchpad mit XPT2046
TP_XPT2046::TP_XPT2046(SPIClass &spi, int csPin) : spi_TP(spi){
    _TP_CS = csPin;
    _rotation = 0;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::begin(uint8_t IRQ){
    _TP_IRQ = IRQ;
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
    spi_TP.beginTransaction(TP_SPI); // Prevent other SPI users
    digitalWrite(_TP_CS, 0);
    spi_TP.write(set_val);
    get_val = spi_TP.transfer16(0);
    digitalWrite(_TP_CS, 1);
    spi_TP.endTransaction(); // Allow other SPI users
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
                if (tp_released)
                    tp_released(x1, y1);
                else if (tp_long_released)
                    tp_long_released(x1, y1);
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

    switch(v) {
        case 0: TP_vers = TP_ILI9341_0; break;
        case 1: TP_vers = TP_ILI9341_1; break;
        case 2: TP_vers = TP_HX8347D_0; break;
        case 3: TP_vers = TP_ILI9486_0; break;
        case 4: TP_vers = TP_ILI9488_0; break;
        case 5: TP_vers = TP_ST7796_0; break;
        default: TP_vers = TP_ILI9341_0; break;
    }

    if(TP_vers == TP_ILI9341_0) { // ILI9341 display
        Xmax = 1913;              // Values Calibration
        Xmin = 150;
        Ymax = 1944;
        Ymin = 220;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if(TP_vers == TP_ILI9341_1) { // ILI9341 display for RaspberryPI  #70
        Xmax = 1940;
        Xmin = 90;
        Ymax = 1864;
        Ymin = 105;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if(TP_vers == TP_HX8347D_0) { // Waveshare HX8347D display
        Xmax = 1850;
        Xmin = 170;
        Ymax = 1880;
        Ymin = 140;
        xFaktor = float(Xmax - Xmin) / HX8347D_WIDTH;
        yFaktor = float(Ymax - Ymin) / HX8347D_HEIGHT;
    }
    if(TP_vers == TP_ILI9486_0) { // ILI9486 display for RaspberryPI
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9486_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9486_HEIGHT;
    }
    if(TP_vers == TP_ILI9488_0) { // ILI9488 display
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9488_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9488_HEIGHT;
    }
    if(TP_vers == TP_ST7796_0) { // ST7796 4" display
        Xmax = 1922;
        Xmin = 103;
        Ymax = 1950;
        Ymin = 110;
        xFaktor = float(Xmax - Xmin) / ST7796_WIDTH;
        yFaktor = float(Ymax - Ymin) / ST7796_HEIGHT;
    }
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

    if((_x < Xmin) || (_x > Xmax)) return false; // outside the display
    _x = Xmax - _x;
    _x /= xFaktor;

    if((_y < Ymin) || (y > Ymax)) return false; // outside the display
    _y = Ymax - _y;
    _y /= yFaktor;

    if(m_mirror_h) {
        switch(TP_vers) {
            case TP_ILI9341_0: // ILI9341
                _y = ILI9341_HEIGHT - _y;
                break;
            case TP_ILI9341_1: _y = ILI9341_HEIGHT - _y; break;
            case TP_HX8347D_0: _y = HX8347D_HEIGHT - _y; break;
            case TP_ILI9486_0: _y = ILI9486_HEIGHT - _y; break;
            case TP_ILI9488_0: _y = ILI9488_HEIGHT - _y; break;
            case TP_ST7796_0: _y = ST7796_HEIGHT - _y;
            default: break;
        }
    }

    if(m_mirror_v) {
        switch(TP_vers) {
            case TP_ILI9341_0: // ILI9341
                _x = ILI9341_WIDTH - _x;
                break;
            case TP_ILI9341_1: _x = ILI9341_WIDTH - _x; break;
            case TP_HX8347D_0: _x = HX8347D_WIDTH - _x; break;
            case TP_ILI9486_0: _x = ILI9486_WIDTH - _x; break;
            case TP_ILI9488_0: _x = ILI9488_WIDTH - _x; break;
            case TP_ST7796_0: _x = ST7796_WIDTH - _x; break;
            default: break;
        }
    }

    // log_i("_x %i, _y %i", _x, _y);
    x = _x;
    y = _y;

    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_0) { // 320px x 240px
        if(_rotation == 0) { y = ILI9341_HEIGHT - y; }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
            x = ILI9341_HEIGHT - x;
        }
        if(_rotation == 2) { x = ILI9341_WIDTH - x; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_1) { // 320px x 240px
        if(_rotation == 0) {
            y = ILI9341_HEIGHT - y;
            x = ILI9341_WIDTH - x;
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            x = ILI9341_HEIGHT - x;
        }
        if(_rotation == 2) { ; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
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
            y = HX8347D_WIDTH - tmpxy;
            if(x > HX8347D_HEIGHT - 1) x = 0;
            if(y > HX8347D_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) {
            x = HX8347D_WIDTH - x;
            y = HX8347D_HEIGHT - y;
            if(x > HX8347D_WIDTH - 1) x = 0;
            if(y > HX8347D_HEIGHT - 1) y = 0;
        }
        if(_rotation == TP_ILI9486_0) {
            tmpxy = y;
            y = x;
            x = HX8347D_HEIGHT - tmpxy;
            if(x > HX8347D_HEIGHT - 1) x = 0;
            if(y > HX8347D_WIDTH - 1) y = 0;
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
            y = ILI9486_WIDTH - tmpxy;
            if(x > ILI9486_HEIGHT - 1) x = 0;
            if(y > ILI9486_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) {
            x = ILI9486_WIDTH - x;
            y = ILI9486_HEIGHT - y;
            if(x > ILI9486_WIDTH - 1) x = 0;
            if(y > ILI9486_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) {
            tmpxy = y;
            y = x;
            x = ILI9486_HEIGHT - tmpxy;
            if(x > ILI9486_HEIGHT - 1) x = 0;
            if(y > ILI9486_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9488_0) { // ILI 9488 Display V1.0, 480px x 320px
        if(_rotation == 0) { x = ILI9488_WIDTH - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > ILI9488_HEIGHT - 1) x = 0;
            if(y > ILI9488_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if(x > ILI9488_WIDTH - 1) x = 0;
            if(y > ILI9488_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = ILI9488_HEIGHT - y;
            y = ILI9488_WIDTH - tmpxy;
            if(x > ILI9488_HEIGHT - 1) x = 0;
            if(y > ILI9488_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ST7796_0) { // ST7796 Display V1.1, 480px x 320px
        if(_rotation == 0) { x = ST7796_WIDTH - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > ST7796_HEIGHT - 1) x = 0;
            if(y > ST7796_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if(x > ST7796_WIDTH - 1) x = 0;
            if(y > ST7796_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = ST7796_HEIGHT - y;
            y = ST7796_WIDTH - tmpxy;
            if(x > ST7796_HEIGHT - 1) x = 0;
            if(y > ST7796_WIDTH - 1) y = 0;
        }
    }
    //    log_i("TP_vers %d, Rotation %d, X = %i, Y = %i",TP_vers, _rotation, x, y);
    return true;
}
