#pragma once

#include <stdint-gcc.h>

struct Pins {
    int8_t b0;
    int8_t b1;
    int8_t b2;
    int8_t b3;
    int8_t b4;
    int8_t g0;
    int8_t g1;
    int8_t g2;
    int8_t g3;
    int8_t g4;
    int8_t g5;
    int8_t r0;
    int8_t r1;
    int8_t r2;
    int8_t r3;
    int8_t r4;
    int8_t hsync;
    int8_t vsync;
    int8_t de;
    int8_t pclk;
    int8_t bl;
};

struct Timing {
    uint16_t h_res;
    uint16_t v_res;
    uint32_t pixel_clock_hz;
    uint8_t  hsync_pulse_width;
    uint8_t  hsync_back_porch;
    uint16_t hsync_front_porch;
    uint8_t  vsync_pulse_width;
    uint8_t  vsync_back_porch;
    uint8_t  vsync_front_porch;
};