#pragma once

#include <stdint-gcc.h>

struct Pins {
    gpio_num_t b0;
    gpio_num_t b1;
    gpio_num_t b2;
    gpio_num_t b3;
    gpio_num_t b4;
    gpio_num_t g0;
    gpio_num_t g1;
    gpio_num_t g2;
    gpio_num_t g3;
    gpio_num_t g4;
    gpio_num_t g5;
    gpio_num_t r0;
    gpio_num_t r1;
    gpio_num_t r2;
    gpio_num_t r3;
    gpio_num_t r4;
    gpio_num_t hsync;
    gpio_num_t vsync;
    gpio_num_t de;
    gpio_num_t pclk;
    gpio_num_t bl;
};

struct Timing {
    uint16_t h_res;
    uint16_t v_res;
    uint32_t pixel_clock_hz;
    float    pixel_clock_mhz;
    uint8_t  hsync_pulse_width;
    uint8_t  hsync_back_porch;
    uint16_t hsync_front_porch;
    uint8_t  vsync_pulse_width;
    uint8_t  vsync_back_porch;
    uint8_t  vsync_front_porch;
    float    lane_bit_rate_mbps;
};