// first releas on 11/2025
// updated on Nov 16 2025

#include "tp_ft6x36.h"

__attribute__((weak)) void tp_info(const char* info) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

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

#define ANSI_ESC_RED   "\033[31m"
#define ANSI_ESC_CYAN  "\033[36m"
#define ANSI_ESC_RESET "\033[0m"

FT6x36::FT6x36() {}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool FT6x36::begin(TwoWire* twi, uint8_t addr, uint16_t h_resolution, uint16_t v_resolution) {
    m_wire = twi; // I2C TwoWire Instance
    m_wire->setTimeOut(1000);
    m_addr = addr;
    if (probe()) {
        m_isInit = true;
        char buff[64] = {0};
        sprintf(buff, "TouchPad found at " ANSI_ESC_CYAN "0x%02X" ANSI_ESC_RESET, m_addr);
        tp_info(buff);
        write(FT6x36U_ADDR_DEVICE_MODE, 0);
        write(FT6X36_ADDR_THRESHHOLD, 22);
        m_h_resolution = h_resolution;
        m_v_resolution = v_resolution;
        return true;
    }
    m_wire->flush();
    log_e(ANSI_ESC_RED "TouchPad not Initialized at 0x%02X" ANSI_ESC_RESET, m_addr);
    return false;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool FT6x36::probe() {

    // Select CHIP_ID register
    m_wire->beginTransmission(m_addr);
    m_wire->write(FT6X36_ADDR_CHIP_ID);

    // No STOP to avoid bus disruptions
    uint8_t err = m_wire->endTransmission(false);

    if (err != 0) {
        log_e("err %i", err);
        // 1 = data too long
        // 2 = NACK on address
        // 3 = NACK on data
        // 4 = other error
        return false;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    // Read 1 byte
    if (m_wire->requestFrom((int)m_addr, 1) != 1) {
        // log_e("Device does not respond");
        return false;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    uint8_t chip_id = m_wire->read();
    log_d("chip_id %i", chip_id);
    // FT6336 / FT6X36 gültige IDs:
    // 0x36 = FT6x36
    // 0x64 = FT6336U
    if (chip_id == 0x36 || chip_id == 0x64) { return true; }

    return false;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool FT6x36::get_FT6x36_items() {
    if (!m_isInit) return false;
    bool     ret = false;
    char     buff[256] = {0};
    uint8_t  chipID = 0, lv_l = 0, lv_h = 0, FW_ID = 0;
    uint16_t Library_Vers = 0;
    ret = read(FT6X36_ADDR_CHIP_ID, chipID);
    if(!ret) {log_e("error while read FT6X36_ADDR_CHIP_ID"); return false;}
    ret = read(FT6X36_ADDR_LIBRARY_VERSION_H, lv_h);
    if(!ret) {log_e("error while read FT6X36_ADDR_LIBRARY_VERSION_H"); return false;}
    ret = read(FT6X36_ADDR_LIBRARY_VERSION_L, lv_l);
    if(!ret) {log_e("error while read FT6X36_ADDR_LIBRARY_VERSION_L"); return false;}
    Library_Vers = (lv_h << 8) + lv_l;
    ret = read(FT6X36_ADDR_FIRMARE_ID, FW_ID);
    if(!ret) {log_e("error while read FT6X36_ADDR_FIRMARE_ID"); return false;}
    sprintf(buff, "FT6x36 Chip ID: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET ", Library Version: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET ", Firmware Version: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET, chipID,
            Library_Vers, FW_ID);
    tp_info(buff);
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void FT6x36::loop() {
    if (!m_isInit) return;
    static T_Point  p, p1;
    static uint32_t ts = 0;
    uint8_t         t = touched();

    if (t == 1 && !m_f_isTouch) {
        p = getPoint(t);
        // log_w("X: %d, Y: %d", p.x, p.y);
        tp_pressed(p.x, p.y);
        ts = millis();
        m_f_isTouch = true;
        return;
    }
    if (t == 1 && m_f_isTouch) {
        p1 = getPoint(t);
        if (p1.x != p.x || p1.y != p.y) {
            p = p1;
            tp_moved(p.x, p.y);
            return;
        }
        // fall through
    }
    if (t == 1 && m_f_isTouch && (millis() > ts + 2000) && !m_f_isLongPressed) {
        m_f_isLongPressed = true;
        tp_long_pressed(p.x, p.y);
        ts = millis() + 10000;
        return;
    }
    if (t == 0 && m_f_isTouch && !m_f_isLongPressed) {
        tp_released(p.x, p.y);
        m_f_isTouch = false;
        return;
    }
    if (t == 0 && m_f_isLongPressed) {
        m_f_isLongPressed = false;
        tp_long_released(p.x, p.y);
        m_f_isTouch = false;
        return;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void FT6x36::setRotation(uint8_t m) {
    m_rotation = m;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void FT6x36::setMirror(bool h, bool v) {
    m_mirror_h = h;
    m_mirror_v = v;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t FT6x36::touched() {
    if (!m_isInit) return 0;
    uint8_t s = 0;
    read(FT6X36_ADDR_TOUCH_STATUS, s); // return 0, 1 or 2
    return s;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
FT6x36::T_Point FT6x36::getPoint(uint8_t num) {
    T_Point points;
    bool    ret = false;
    uint8_t id = 0, x_h = 0, x_l = 0, y_h = 0, y_l = 0;
    if (!m_isInit) return {0, 0, 0};
    if (num != 1) return {0, 0, 0};
    if (num == 1) {
        ret = read(FT6X36_ADDR_TOUCH1_WEIGHT, id);
        if (!ret) {
            log_e("error while reading FT6X36_ADDR_TOUCH1_WEIGHT");
            return points;
        }
        ret = read(FT6X36_ADDR_TOUCH1_XH, x_h);
        if (!ret) {
            log_e("error while reading FT6X36_ADDR_TOUCH1_XH");
            return points;
        }
        ret = read(FT6X36_ADDR_TOUCH1_XL, x_l);
        if (!ret) {
            log_e("error while reading FT6X36_ADDR_TOUCH1_XL");
            return points;
        }
        ret = read(FT6X36_ADDR_TOUCH1_YH, y_h);
        if (!ret) {
            log_e("error while reading FT6X36_ADDR_TOUCH1_YH");
            return points;
        }
        ret = read(FT6X36_ADDR_TOUCH1_YL, y_l);
        if (!ret) {
            log_e("error while reading FT6X36_ADDR_TOUCH1_YL");
            return points;
        }
        points.id = id >> 4;
        points.x = ((x_h & 0x0F) << 8) + x_l;
        points.y = ((y_h & 0x0F) << 8) + y_l;
    }

    if (m_rotation == 0) { // 0°
        if (m_mirror_v)
            points.x = m_h_resolution - points.x;
        else {}
        if (m_mirror_h)
            points.y = m_v_resolution - points.y;
        else {}
    } else if (m_rotation == 1) { // 90°
        uint16_t tmp = points.x;
        if (m_mirror_v)
            points.x = m_h_resolution - points.y;
        else
            points.x = points.y;
        if (m_mirror_h)
            points.y = tmp;
        else
            points.y = m_v_resolution - tmp;
    } else if (m_rotation == 2) { // 180°
        if (m_mirror_v) {
        } else
            points.x = m_v_resolution - points.x;
        if (m_mirror_h) {
        } else
            points.y = m_h_resolution - points.y;
    } else if (m_rotation == 3) { // 270°
        uint16_t tmp = points.x;
        if (m_mirror_v)
            points.x = points.y;
        else
            points.x = m_h_resolution - points.y;
        if (m_mirror_h)
            points.y = m_v_resolution - tmp;
        else
            points.y = tmp;
    }

    log_d("x %i, y %i", points.x, points.y);
    return points;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool FT6x36::write(uint8_t reg, uint8_t data) {
    if (!m_isInit) return false;
    if(!m_wire->availableForWrite()) log_e("not available for write");
    bool ret = false;
    m_wire->beginTransmission(m_addr);
    ret = m_wire->write(reg);
    if(!ret) log_e("write register error");
    m_wire->write(data);
    if(!ret) log_e("write data error");
    return m_wire->endTransmission(true) == 0;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool FT6x36::read(uint8_t reg, uint8_t& value) {
    if (!m_isInit) return false;
    bool ret = false;

    m_wire->beginTransmission(m_addr);
    ret = m_wire->write(reg);

    if(!ret) log_e("write register error in fkt. read");

    if (m_wire->endTransmission(false) != 0) { log_e("transmission error"); return false; }

    if (m_wire->requestFrom(m_addr, (uint8_t)1) != 1) { log_e("request error, reg %i", reg); return false; }
    value = m_wire->read();
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
