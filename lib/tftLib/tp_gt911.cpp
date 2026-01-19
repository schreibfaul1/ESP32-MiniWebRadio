// first release on 01/2025
// updated on Nov 23 2025

#include "tp_gt911.h"
#define ANSI_ESC_RED   "\033[31m"
#define ANSI_ESC_CYAN  "\033[36m"
#define ANSI_ESC_RESET "\033[0m"

// Code for GT911
TP_GT911::TP_GT911() {}

bool TP_GT911::begin(TwoWire* twi, uint8_t addr, uint16_t h_resolution, uint16_t v_resolution) {
    m_wire = twi; // I2C TwoWire Instance
    m_addr = addr;
    char buff[64] = {0};
    m_wire->beginTransmission(m_addr);
    if (m_wire->endTransmission() == 0) {
        m_isInit = true;
        sprintf(buff, "TouchPad found at " ANSI_ESC_CYAN "0x%02X" ANSI_ESC_RESET, m_addr);
        if (tp_info) tp_info(buff);
        readInfo(); // Need to get resolution to use rotation
        if(v_resolution != m_info.yResolution || h_resolution != m_info.xResolution) {
            sprintf(buff, "Incorrect preset resolution " ANSI_ESC_CYAN "%dx%d, found " ANSI_ESC_CYAN "%dx%d" ANSI_ESC_RESET, m_h_resolution, m_v_resolution, m_info.xResolution, m_info.yResolution);
            if (tp_info) tp_info(buff);
        }
        m_h_resolution = m_info.xResolution;
        m_v_resolution = m_info.yResolution;
        return true;
    }
    sprintf(buff, ANSI_ESC_RED "TouchPad not found at 0x%02X" ANSI_ESC_RESET, m_addr);
    if (tp_info)
        tp_info(buff);
    else
        log_e("TouchPad not Initialized at 0x%02X", m_addr);
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP_GT911::setRotation(uint8_t m) {
    m_rotation = m;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP_GT911::setMirror(bool h, bool v) {
    m_mirror_h = h;
    m_mirror_v = v;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP_GT911::setVersion(uint8_t v) {
    switch (v) {
        case GT911:
            m_version = GT911;
            break; // GT927, GT928, GT967, GT5688
                   // case 4: m_version = TP_ILI2510; break; // ILI9488
                   // case 5: m_version = TP_FT5406; break; // FT5446, FT6336U
    }
    char buff[64] = {0};
    sprintf(buff, "TP resolution " ANSI_ESC_CYAN "%dx%d" ANSI_ESC_RESET, m_h_resolution, m_v_resolution);
    if (tp_info) tp_info(buff);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP_GT911::loop() {
    if (!m_isInit) return;
    static GTPoint  p, p1;
    static uint32_t ts = 0;
    uint8_t         t = touched(); // number of touch points
    if (t == 1 && !m_f_isTouch) {
        p = getPoint(0);
        // log_w("X: %d, Y: %d", p.x, p.y);
        if (tp_pressed) tp_pressed(p.x, p.y);
        ts = millis();
        m_f_isTouch = true;
        return;
    }
    if (t == 1 && m_f_isTouch) {
        p1 = getPoint(0);
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
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP_GT911::getProductID() {
    if (!m_isInit) return false;
    char buf[64] = {0};
    strcpy(buf, "Product ID: " ANSI_ESC_CYAN);
    readBytes(GT911_REG_ID, (uint8_t*)buf + 17, 4);
    strcat(buf, ANSI_ESC_RESET);
    if (tp_info) tp_info(buf);
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP_GT911::reset() {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pinMode(m_intPin, OUTPUT);
    digitalWrite(m_intPin, LOW);
    pinMode(m_rstPin, OUTPUT);
    digitalWrite(m_rstPin, LOW);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(m_intPin, HIGH);
    pinMode(m_rstPin, INPUT);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(m_intPin, LOW);
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP_GT911::write(uint16_t reg, uint8_t data) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->write(data);
    return m_wire->endTransmission(true) == 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP_GT911::read(uint16_t reg) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->endTransmission(false);
    m_wire->requestFrom(m_addr, (uint8_t)1);
    while (m_wire->available()) { return m_wire->read(); }
    m_wire->endTransmission(true);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP_GT911::writeBytes(uint16_t reg, uint8_t* data, uint16_t size) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    for (uint16_t i = 0; i < size; i++) { m_wire->write(data[i]); }
    return m_wire->endTransmission(true) == 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP_GT911::readBytes(uint16_t reg, uint8_t* data, uint16_t size) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->endTransmission(false);

    uint16_t index = 0;
    while (index < size) {
        uint8_t req = _min(size - index, I2C_BUFFER_LENGTH);
        m_wire->requestFrom(m_addr, req);
        while (m_wire->available()) { data[index++] = m_wire->read(); }
        index++;
    }
    m_wire->endTransmission(true);
    return size == index - 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP_GT911::calcChecksum(uint8_t* buf, uint8_t len) {
    uint8_t ccsum = 0;
    for (uint8_t i = 0; i < len; i++) { ccsum += buf[i]; }

    return (~ccsum + 1) & 0xFF; // complement of checksum
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP_GT911::readChecksum() {
    return read(GT911_REG_CHECKSUM);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t TP_GT911::readTouches() {
    uint32_t timeout = millis() + 20;
    do {
        uint8_t flag = read(GT911_REG_COORD_ADDR);
        if ((flag & 0x80) && ((flag & 0x0F) < GT911_MAX_CONTACTS)) {
            write(GT911_REG_COORD_ADDR, 0);
            return flag & 0x0F;
        }
        delay(1);
    } while (millis() < timeout);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP_GT911::readTouchPoints() {
    bool result = readBytes(GT911_REG_COORD_ADDR + 1, (uint8_t*)m_points, sizeof(GTPoint) * GT911_MAX_CONTACTS);
    return result;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP_GT911::GTInfo* TP_GT911::readInfo() {
    readBytes(GT911_REG_DATA, (uint8_t*)&m_info, sizeof(m_info));
    return &m_info;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP_GT911::touched() {
    uint8_t contacts = 0;
    contacts = readTouches();
    if (contacts > 0) { readTouchPoints(); }
    return contacts;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP_GT911::GTPoint TP_GT911::getPoint(uint8_t num) {
    int x_new = 0, y_new = 0;
    if (m_mirror_h) m_points[num].x = m_h_resolution - m_points[num].x;
    if (m_mirror_v) m_points[num].y = m_v_resolution - m_points[num].y;

    switch (m_rotation) {
        case 0: // 90 degree
            x_new = m_points[num].y;
            y_new = m_h_resolution - m_points[num].x;
            break;
        case 1: return m_points[num]; // 0 degree, no change
        case 2: // -90 degree
            x_new = m_v_resolution - m_points[num].y;
            y_new = m_points[num].x;
            break;
        case 3: // -180 degree
            x_new = m_h_resolution - m_points[num].x;
            y_new = m_v_resolution - m_points[num].y;
            break;
    }
    m_points[num].x = x_new;
    m_points[num].y = y_new;
    return m_points[num];
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP_GT911::GTPoint* TP_GT911::getPoints() {
    int x_new = 0, y_new = 0;
    for (uint8_t num = 0; num < GT911_MAX_CONTACTS; num++) {
        if (m_mirror_h) m_points[num].x = m_h_resolution - m_points[num].x;
        if (m_mirror_v) m_points[num].y = m_v_resolution - m_points[num].y;
        switch (m_rotation) {
        case 0: // 90 degree
            x_new = m_points[num].y;
            y_new = m_h_resolution - m_points[num].x;
            break;
        case 1: // 0 degree, no change
            x_new = m_points[num].x;
            y_new = m_points[num].y;
            break;
        case 2: // -90 degree
            x_new = m_v_resolution - m_points[num].y;
            y_new = m_points[num].x;
            break;
        case 3: // -180 degree
            x_new = m_h_resolution - m_points[num].x;
            y_new = m_v_resolution - m_points[num].y;
            break;
        }
        m_points[num].x = x_new;
        m_points[num].y = y_new;
    }
    return m_points;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
