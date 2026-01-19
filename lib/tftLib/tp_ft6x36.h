// first releas on 11/2025
// updated on Nov 16 2025

#pragma once

#include <Arduino.h>
#include <Wire.h>

extern __attribute__((weak)) void tp_info(const char* info);
extern __attribute__((weak)) void tp_positionXY(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_moved(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released(uint16_t x, uint16_t y);

class FT6x36 {
  private:
    struct T_Point {
        uint16_t x;
        uint16_t y;
        uint16_t id;
    };

   public:
    static const uint8_t  FT6x36U_I2C_ADDRESS = 0x38;

  private:
    static const uint8_t FT6x36U_ADDR_DEVICE_MODE = 0x00;
    static const uint8_t FT6X36_ADDR_TOUCH_STATUS = 0x02;

    static const uint8_t FT6X36_ADDR_TOUCH1_WEIGHT = 0x07;
    static const uint8_t FT6X36_ADDR_TOUCH1_MISC = 0x08;
    static const uint8_t FT6X36_ADDR_TOUCH1_YH = 0x05;
    static const uint8_t FT6X36_ADDR_TOUCH1_YL = 0x06;
    static const uint8_t FT6X36_ADDR_TOUCH1_XH = 0x03;
    static const uint8_t FT6X36_ADDR_TOUCH1_XL = 0x04;

    static const uint8_t FT6X36_ADDR_TOUCH2_XH = 0x09;
    static const uint8_t FT6X36_ADDR_TOUCH2_XL = 0x0A;
    static const uint8_t FT6X36_ADDR_TOUCH2_YH = 0x0B;
    static const uint8_t FT6X36_ADDR_TOUCH2_YL = 0x0C;
    static const uint8_t FT6X36_ADDR_TOUCH2_WEIGHT = 0x0D;
    static const uint8_t FT6X36_ADDR_TOUCH2_MISC = 0x0E;

    static const uint8_t FT6X36_ADDR_LIBRARY_VERSION_H = 0xA1;
    static const uint8_t  FT6X36_ADDR_LIBRARY_VERSION_L = 0xA2;

    static const uint8_t FT6X36_ADDR_CHIP_ID = 0xA3;
    static const uint8_t FT6X36_ADDR_FIRMARE_ID = 0xA6;
    static const uint8_t FT6X36_ADDR_THRESHHOLD = 0x80;


  public:
    FT6x36();
    bool     begin(TwoWire* twi, uint8_t addr, uint16_t h_resolution, uint16_t v_resolution);
    bool     probe();
    void     loop();
    void     setRotation(uint8_t m);
    void     setMirror(bool h, bool v);
    bool     get_FT6x36_items();
    uint8_t  touched();
    T_Point  getPoint(uint8_t num);
    T_Point* getPoints();


  private:
    TwoWire* m_wire;
    int8_t   m_scl;
    int8_t   m_sda;
    uint8_t  m_addr;
    uint8_t  m_rotation = 0;
    uint16_t m_h_resolution = 0;
    uint16_t m_v_resolution = 0;
    bool     m_f_isTouch = false;
    bool     m_f_isLongPressed = false;
    bool     m_isInit = false;
    bool     m_mirror_h = false;
    bool     m_mirror_v = false;

  private:
    bool    write(uint8_t reg, uint8_t data);
    uint8_t read(uint8_t reg);
};