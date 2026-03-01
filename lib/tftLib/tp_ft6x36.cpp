// first releas on 11/2025
// updated on Nov 16 2025

#include "tp_ft6x36.h"
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
        char buff[30] = {0};
        sprintf(buff, "TouchPad found at " ANSI_ESC_CYAN "0x%02X" ANSI_ESC_RESET, m_addr);
        if (tp_info) tp_info(buff);
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
    char     buff[256] = {0};
    uint8_t  chipID = read(FT6X36_ADDR_CHIP_ID);
    uint16_t Library_Vers = read(FT6X36_ADDR_LIBRARY_VERSION_H) << 8;
    Library_Vers += read(FT6X36_ADDR_LIBRARY_VERSION_L);
    uint8_t FW_ID = read(FT6X36_ADDR_FIRMARE_ID);
    sprintf(buff, "FT6x36 Chip ID: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET ", Library Version: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET ", Firmware Version: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET, chipID,
            Library_Vers, FW_ID);
    if (tp_info) tp_info(buff);
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
        if (tp_pressed) tp_pressed(p.x, p.y);
        ts = millis();
        m_f_isTouch = true;
        return;
    }
    if (t == 1 && m_f_isTouch) {
        p1 = getPoint(t);
        if (p1.x != p.x || p1.y != p.y) {
            p = p1;
            if (tp_moved) tp_moved(p.x, p.y);
            return;
        }
        // fall through
    }
    if (t == 1 && m_f_isTouch && (millis() > ts + 2000) && !m_f_isLongPressed) {
        m_f_isLongPressed = true;
        if (tp_long_pressed) tp_long_pressed(p.x, p.y);
        ts = millis() + 10000;
        return;
    }
    if (t == 0 && m_f_isTouch && !m_f_isLongPressed) {
        if (tp_released) tp_released(p.x, p.y);
        m_f_isTouch = false;
        return;
    }
    if (t == 0 && m_f_isLongPressed) {
        m_f_isLongPressed = false;
        if (tp_long_released) tp_long_released(p.x, p.y);
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
    return read(FT6X36_ADDR_TOUCH_STATUS); // return 0, 1 or 2
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
FT6x36::T_Point FT6x36::getPoint(uint8_t num) {
    T_Point points;
    if (!m_isInit) return {0, 0, 0};
    if (num != 1) return {0, 0, 0};
    if (num == 1) {
        points.id = read(FT6X36_ADDR_TOUCH1_WEIGHT >> 4);
        points.x = (read(FT6X36_ADDR_TOUCH1_XH) & 0x0F) << 8;
        points.x += (read(FT6X36_ADDR_TOUCH1_XL));
        points.y = (read(FT6X36_ADDR_TOUCH1_YH) & 0x0F) << 8;
        points.y += (read(FT6X36_ADDR_TOUCH1_YL));
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
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg);
    m_wire->write(data);
    return m_wire->endTransmission(true) == 0;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t FT6x36::read(uint8_t reg) {
    if (!m_isInit) return 0;
    m_wire->beginTransmission(m_addr);
    m_wire->write((uint8_t)reg);
    m_wire->endTransmission();
    m_wire->requestFrom(m_addr, (uint8_t)1);
    uint8_t ret = m_wire->read();
    m_wire->endTransmission(true);
    return ret;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
