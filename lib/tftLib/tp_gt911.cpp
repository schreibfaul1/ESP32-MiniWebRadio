// first release on 01/2025
// updated on Nov 23 2025

#include "tp_gt911.h"

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

// Code for GT911
TP_GT911::TP_GT911() {}

bool TP_GT911::begin(TwoWire* twi, uint8_t addr, uint16_t h_resolution, uint16_t v_resolution) {
    m_disp_h_resolution = h_resolution;
    m_disp_v_resolution = v_resolution;
    m_wire = twi; // I2C TwoWire Instance
    m_addr = addr;
    char buff[128] = {0};
    m_wire->setTimeOut(1000);
    m_wire->beginTransmission(m_addr);
    if (m_wire->endTransmission() == 0) {
        m_isInit = true;
        sprintf(buff, "TouchPad found at " ANSI_ESC_CYAN "0x%02X" ANSI_ESC_RESET, m_addr);
        tp_info(buff);
        readInfo(); // Need to get resolution to use rotation
        uint16_t x_resolution = readXResolution();
        uint16_t y_resolution = readYResolution();
        // log_i("x_resolution %i", x_resolution);
        // log_i("y_resolution %i", y_resolution);
        if ((m_disp_v_resolution == x_resolution && m_disp_h_resolution == y_resolution) || (m_disp_v_resolution == y_resolution && m_disp_h_resolution == x_resolution)) {
            sprintf(buff, "TP resolution " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET " x " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET, x_resolution, y_resolution);
            tp_info(buff);
        } else {
            sprintf(buff, "Touch resolution " ANSI_ESC_CYAN "%dx%d" ANSI_ESC_RESET " mapped to display " ANSI_ESC_CYAN "%dx%d" ANSI_ESC_RESET "", x_resolution, y_resolution, m_disp_h_resolution,
                    m_disp_v_resolution);
            tp_info(buff);
        }
        m_touch_h_resolution = x_resolution;
        m_touch_v_resolution = y_resolution;
        return true;
    }
    sprintf(buff, ANSI_ESC_RED "TouchPad not found at 0x%02X" ANSI_ESC_RESET, m_addr);
    tp_info(buff);
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
void TP_GT911::loop() {
    if (!m_isInit) return;
    static GTPoint  p, p1;
    static uint32_t ts = 0;

    uint8_t t = touched(); // number of touch points

    if (t == 1 && !m_f_isTouch) {
        p = getPoint(0);
        log_w("X: %d, Y: %d", p.x, p.y);
        tp_pressed(p.x, p.y);
        ts = millis();
        m_f_isTouch = true;
        return;
    }
    if (t == 1 && m_f_isTouch) {
        p1 = getPoint(0);
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
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t TP_GT911::readXResolution() {
    uint8_t buf[2];
    readBytes(0x8048, buf, 2);
    return buf[0] | (buf[1] << 8);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t TP_GT911::readYResolution() {
    uint8_t buf[2];
    readBytes(0x804A, buf, 2);
    return buf[0] | (buf[1] << 8);
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
    if (m_wire->endTransmission(false) != 0) return 0;

    if (m_wire->requestFrom(m_addr, (uint8_t)1) != 1) return 0;

    if (m_wire->available()) { return m_wire->read(); }
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
    if (m_wire->endTransmission(false) != 0) {
        return false; // ❌ Früher abbrechen bei Fehler
    }

    uint16_t index = 0;
    while (index < size) {
        uint8_t req = min((int)(size - index), (int)I2C_BUFFER_LENGTH);
        if (m_wire->requestFrom(m_addr, req) != req) {
            return false; // ❌ Prüfe ob alle Bytes empfangen wurden
        }
        while (m_wire->available() && index < size) { data[index++] = m_wire->read(); }
        // index++ entfernt - war der Bug!
    }

    m_wire->endTransmission(true); // Sende STOP
    return index == size;
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
        delay(5);
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
    char buff[128] = {0};
    sprintf(buff, "TP ProductID: " ANSI_ESC_CYAN "%s," ANSI_ESC_RESET " Firmware version: " ANSI_ESC_CYAN "%d," ANSI_ESC_RESET " VendorID: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET, m_info.productId,
            m_info.fwId, m_info.vendorId);
    tp_info(buff);
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
void TP_GT911::transform(uint16_t& x, uint16_t& y) {
    uint16_t touch_w = m_touch_h_resolution;
    uint16_t touch_h = m_touch_v_resolution;

    // 1️⃣ Mirror in native space
    if (m_mirror_h) x = touch_w - 1 - x;
    if (m_mirror_v) y = touch_h - 1 - y;

    uint16_t xr = x;
    uint16_t yr = y;

    int8_t r = m_rotation + 1;
    if (r == 4) r = 0;

    // 2️⃣ Rotation in native space
    switch (r) {
        case 0: // 0°
            break;

        case 1: // 90°
            xr = y;
            yr = touch_w - 1 - x;
            std::swap(touch_w, touch_h);
            break;

        case 2: // 180°
            xr = touch_w - 1 - x;
            yr = touch_h - 1 - y;
            break;

        case 3: // 270°
            xr = touch_h - 1 - y;
            yr = x;
            std::swap(touch_w, touch_h);
            break;
    }

    // 3️⃣ Scale
    x = (uint32_t)xr * (m_disp_h_resolution - 1) / (touch_w - 1);
    y = (uint32_t)yr * (m_disp_v_resolution - 1) / (touch_h - 1);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP_GT911::GTPoint TP_GT911::getPoint(uint8_t num) {

    uint16_t x, y;
    x = m_points[num].x;
    y = m_points[num].y;

    m_points[num].x = constrain(x, 0, m_disp_h_resolution - 1);
    m_points[num].y = constrain(y, 0, m_disp_v_resolution - 1);

    transform(x, y);

    m_points[num].x = x;
    m_points[num].y = y;
    return m_points[num];
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP_GT911::GTPoint* TP_GT911::getPoints() {
    uint16_t x = 0, y = 0;
    for (uint8_t num = 0; num < GT911_MAX_CONTACTS; num++) {

        transform(x, y);

        m_points[num].x = x;
        m_points[num].y = y;
    }
    return m_points;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
