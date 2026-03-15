// first release on 09/2019
// updated on Feb 02 2025

#include "tp_xpt2046.h"

__attribute__((weak)) void tp_moved(uint16_t x, uint16_t y) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

__attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

__attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

__attribute__((weak)) void tp_released(uint16_t x, uint16_t y) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

__attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//         XPT2046
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Code für Touchpad mit XPT2046
TP_XPT2046::TP_XPT2046(SPIClass& spi, int csPin) {
    _TP_CS = csPin;
    m_rotation = 0;
    spi_TP = &spi;
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::begin(uint8_t IRQ, uint16_t h_resolution, uint16_t v_resolution) {
    _TP_IRQ = IRQ;
    m_h_res = h_resolution; // horizontal
    m_v_res = v_resolution; // vertical
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
    pinMode(_TP_IRQ, INPUT);
    TP_SPI = SPISettings(200000, MSBFIRST, SPI_MODE0);
    m_rotation = 0;
    TP_Send(0xD0);
    TP_Send(0x90); // Remove any blockage
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::loop() {
    static uint16_t x1 = 0, y1 = 0;
    static uint16_t x2 = 0, y2 = 0;
    if (!digitalRead(_TP_IRQ)) {
        if (!read_TP(x, y)) { return; }

        if (x < 0 || y < 0) log_w("x %i, y %i", x, y);
        if (x > m_h_res) log_w("%s%i: x %i, m_h_resolution %i", __FILE__, __LINE__, x, m_h_res);
        if (y > m_v_res) log_w("%s%i: y %i, m_v_resolution %i", __FILE__, __LINE__, y, m_v_res);

        if (x != x1 && y != y1) { tp_moved(x, y); }

        {
            x1 = x;
            y1 = y;
        }

        if (f_loop) {
            f_loop = false;
            // log_i("tp_pressed x=%d, y=%d", x, y);
            tp_pressed(x, y);
            x2 = x;
            y2 = y;
            m_pressingTime = millis();
            m_f_isPressing = true;
        } else {
            if (m_f_isPressing) {
                if (m_pressingTime + 2000 < millis()) {
                    m_f_isPressing = false;
                    tp_long_pressed(x2, y2);
                    m_f_longPressed = true;
                }
            }
        }
    } else {
        if (f_loop == false) {
            // log_i("tp_released");
            if (m_f_longPressed) {
                m_f_longPressed = false;
                tp_long_released(x1, y1);
            } else {
                tp_released(x1, y1);
            }
            f_loop = true;
        }
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::setRotation(uint8_t m) {
    m_rotation = m;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TP_XPT2046::setSize(uint8_t s) {
    switch (s) {
        case 0: m_cal = m_cal_28; break; // 2.8 inch
        case 1: m_cal = m_cal_35; break; // 3.5 inch
        case 2: m_cal = m_cal_40; break; // 4.0 inch
        default: m_cal = m_cal_40; break;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
inline void TP_XPT2046::mapRotation(uint8_t rot, int32_t srcX, int32_t srcY, int32_t& dstX, int32_t& dstY) const {
    switch (rot & 3) {
        case 0:
            dstX = srcX;
            dstY = srcY;
            break;

        case 1: // 90° CW
            dstX = m_v_res - 1 - srcY;
            dstY = srcX;
            break;

        case 2: // 180°
            dstX = m_h_res - 1 - srcX;
            dstY = m_v_res - 1 - srcY;
            break;

        case 3: // 270° CW
            dstX = srcY;
            dstY = m_h_res - 1 - srcX;
            break;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TP_XPT2046::read_TP(int16_t& x, int16_t& y) {

    int32_t read_x = 0, read_y = 0;
    if (digitalRead(_TP_IRQ)) return false; // TP_XPT2046 pressed?
    for (int i = 0; i < 128; i++) {
        read_x += TP_Send(0x90); // x
        read_y += TP_Send(0xD0); // y
    }
    read_x /= 128;
    read_y /= 128;
    if (digitalRead(_TP_IRQ)) return false; // TP_XPT2046 must remain pressed as long as the measurement is running


    // log_e("x %i, y %i", read_x, read_y);

    int32_t clip_x = max(m_cal.xMin, min(m_cal.xMax, read_x));
    int32_t clip_y = max(m_cal.yMin, min(m_cal.yMax, read_y));

    int32_t src_x = map(clip_x, m_cal.xMin, m_cal.xMax, 0, m_h_res - 1);
    int32_t src_y = map(clip_y, m_cal.yMin, m_cal.yMax, 0, m_v_res - 1);

    int32_t dst_x = 0;
    int32_t dst_y = 0;
    mapRotation(m_rotation, src_x, src_y, dst_x, dst_y);

    x = dst_x;
    y = dst_y;

    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
