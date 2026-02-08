#pragma once
#include "tft_structures.h"

#define _SSID             "mySSID"         // Your WiFi credentials here
#define _PW               "myWiFiPassword" // Or in textfile on SD-card
#define TFT_CONTROLLER    7                // (0)ILI9341[320x240], (3)ILI9486[480x320], (4)ILI9488[480x320], (5)ST7796[480x320], (7)RGB display[800x480], (8)DSI display[1024x600]
#define DISPLAY_INVERSION 0                // only SPI displays, (0) off (1) on
#define TFT_ROTATION      1                // only SPI displays, 1 or 3 (landscape)
#define TFT_FREQUENCY     40000000         // only SPI displays, 80000000, 40000000, 27000000, 20000000, 10000000
#define TP_CONTROLLER     7                // (0)ILI9341, (3)ILI9486, (4)ILI9488, (5)ST7796, (7)GT911, (8)FT6x63
#define TP_ROTATION       1                // 1 or 3 (landscape)
#define TP_H_MIRROR       0                // (0) default, (1) mirror up <-> down
#define TP_V_MIRROR       0                // (0) default, (1) mirror left <-> right
#define LIGHT_SENSOR      1                // (0) none, (1) BH1750
#define I2S_COMM_FMT      0                // (0) MAX98357A PCM5102A CS4344, (1) LSBJ (Least Significant Bit Justified format) PT8211
#define SDMMC_FREQUENCY   80000000         // 80000000 or 40000000 Hz
#define FTP_USERNAME      "esp32"          // user name in FTP Client
#define FTP_PASSWORD      "esp32"          // pw in FTP Client
#define CONN_TIMEOUT      2500             // unencrypted connection timeout in ms (http://...)
#define CONN_TIMEOUT_SSL  3500             // encrypted connection timeout in ms (https://...)
#define WIFI_TX_POWER     5                // 2 ... 21 (dBm) Adjust the WiFi transmission power to optimise power consumption or increase range, default: 5
#define LIST_TIMER        5                // After this time (seconds), the display returns from the list view

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DISPLAY [320x240] OR [480x320] TOUCHPAD-CONTROLLER XPT2046 OR FT6x36   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#if TFT_CONTROLLER < 7
    #if CONFIG_IDF_TARGET_ESP32S3
    // Digital I/O used
        #define TFT_CS             8
        #define TFT_DC             12
        #define TFT_BL             10 // at -1 the brightness menu is not displayed
        #define TP_IRQ             39
        #define TP_CS              15
        #define SD_MMC_D0          11
        #define SD_MMC_CLK         13
        #define SD_MMC_CMD         14
        #define IR_PIN             4  // IR Receiver (if available)
        #define TFT_MOSI           18 // TFT and TP (FSPI)
        #define TFT_MISO           2  // TFT and TP (FSPI)
        #define TFT_SCK            17 // TFT and TP (FSPI)
        #define I2S_DOUT           9
        #define I2S_BCLK           3
        #define I2S_LRC            1
        #define I2S_MCLK           0
        #define BT_EMITTER_RX      45 // TX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_TX      38 // RX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_MODE    20 // high transmit - low receive           (-1 if not available)
        #define BT_EMITTER_CONNECT 48 // high impulse -> awake after POWER_OFF (-1 if not available)
        #define I2C_SDA            41 // I2C, dala line for capacitive touchpadand and light sensor (-1 if not available)
        #define I2C_SCL            42 // I2C, clock line for capacitive touchpadand and light sensor (-1 if not available)
        #define AMP_ENABLED        -1 // onboard amplifier (-1 if not available)
    #endif                            // CONFIG_IDF_TARGET_ESP32S3

    #if CONFIG_IDF_TARGET_ESP32P4
        #define TFT_CS             26
        #define TFT_DC             3
        #define TFT_BL             4 // at -1 the brightness menu is not displayed
        #define TP_IRQ             5
        #define TP_CS              2
        #define SD_MMC_D0          39
        #define SD_MMC_D1          40
        #define SD_MMC_D2          41
        #define SD_MMC_D3          42
        #define SD_MMC_CLK         43
        #define SD_MMC_CMD         44
        #define IR_PIN             32 // IR Receiver (if available)
        #define TFT_MOSI           20 // TFT and TP (FSPI)
        #define TFT_MISO           21 // TFT and TP (FSPI)
        #define TFT_SCK            22 // TFT and TP (FSPI)
        #define I2S_DOUT           23
        #define I2S_BCLK           24
        #define I2S_LRC            25
        #define I2S_MCLK           -1
        #define BT_EMITTER_RX      27 // TX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_TX      28 // RX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_MODE    29 // high transmit - low receive           (-1 if not available)
        #define BT_EMITTER_CONNECT 30 // high impulse -> awake after POWER_OFF (-1 if not available)
        #define I2C_SDA            7  // I2C, dala line for capacitive touchpad and light sensor (-1 if not available)
        #define I2C_SCL            8  // I2C, clock line for capacitive touchpad and light sensor (-1 if not available)
        #define AMP_ENABLED        33 // onboard amplifier (-1 if not available)
    // free pins 46, 47, 48, 49, 50, 51, 52
    #endif // CONFIG_IDF_TARGET_ESP32P4
#endif     // TFT_CONTROLLER < 7

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DISPLAY [800x480] ESP32-S§ SUNTON   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if 1                            // 0 deactivated, 1 activated
    #if TFT_CONTROLLER == 7      // RGB display
const Pins RGB_PINS = { // SUNTON 7"
    .b0 = 15, .b1 = 7,  .b2 = 6,  .b3 = 5,  .b4 = 4,  .g0 = 9,     .g1 = 46,    .g2 = 3,  .g3 = 8,    .g4 = 16, .g5 = 1,
    .r0 = 14, .r1 = 21, .r2 = 47, .r3 = 48, .r4 = 45, .hsync = 39, .vsync = 40, .de = 41, .pclk = 42, .bl = 2};

const Timing RGB_TIMING = {.h_res = 800,
                           .v_res = 480,
                           .pixel_clock_hz = 10000000,
                           .hsync_pulse_width = 30,
                           .hsync_back_porch = 16,
                           .hsync_front_porch = 210,
                           .vsync_pulse_width = 13,
                           .vsync_back_porch = 10,
                           .vsync_front_porch = 22};

        #define TP_IRQ             -1
        #define SD_MMC_CMD         11
        #define SD_MMC_CLK         12
        #define SD_MMC_D0          13
        #define GT911_I2C_ADDRESS  0x5D // default I2C-address of GT911
        #define I2S_DOUT           17
        #define I2S_BCLK           0
        #define I2S_LRC            18
        #define I2S_MCLK           -1 // important!
        #define IR_PIN             38 // IR Receiver (if available)
        #define BT_EMITTER_RX      -1 // must be -1, not enough pins
        #define BT_EMITTER_TX      -1 // must be -1, not enough pins
        #define BT_EMITTER_MODE    -1 // must be -1, not enough pins
        #define BT_EMITTER_CONNECT -1 // must be -1, not enough pins
        #define TFT_BL             2  // same as RGB_PINS.bl
        #define I2C_SDA            19 // I2C, data line for capacitive touchpad and light sensor (-1 if not available)
        #define I2C_SCL            20 // I2C, clock line for capacitive touchpad and light sensor (-1 if not available)
        #define AMP_ENABLED        -1 // onboard amplifier (-1 if not available)
    #endif
#endif
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DISPLAY [800x480] ESP32-S3 ELECROW 5"   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if 0                       // 0 deactivated, 1 activated
    #if TFT_CONTROLLER == 7 // RGB display
const Pins RGB_PINS = { // ELECROW 5"
    .b0 = 8,  .b1 = 3,  .b2 = 46, .b3 = 9,  .b4 = 1,  .g0 = 5,     .g1 = 6,     .g2 = 7,  .g3 = 15,  .g4 = 16, .g5 = 4,
    .r0 = 45, .r1 = 48, .r2 = 47, .r3 = 21, .r4 = 14, .hsync = 39, .vsync = 41, .de = 40, .pclk = 0, .bl = 2};

const Timing RGB_TIMING = {.h_res = 800,
                                    .v_res = 480,
                                    .pixel_clock_hz = 11000000,
                                    .hsync_pulse_width = 4,
                                    .hsync_back_porch = 43,
                                    .hsync_front_porch = 8,
                                    .vsync_pulse_width = 4,
                                    .vsync_back_porch = 12,
                                    .vsync_front_porch = 8};

        #define TP_IRQ     -1
        #define SD_MMC_CMD 11
        #define SD_MMC_CLK 12
        #define SD_MMC_D0  13

        #define I2C_MASTER_FREQ_HZ 400000 // 400 kHz I2C-frequency
        #define GT911_I2C_ADDRESS  0x5D   // default I2C-address of GT911

        #define I2S_DOUT 17
        #define I2S_BCLK 42
        #define I2S_LRC  18
        #define I2S_MCLK -1 // important!

        #define IR_PIN             38 // IR Receiver (if available)
        #define BT_EMITTER_RX      -1 // must be -1, not enough pins
        #define BT_EMITTER_TX      -1 // must be -1, not enough pins
        #define BT_EMITTER_LINK    -1 // must be -1, not enough pins
        #define BT_EMITTER_MODE    -1 // must be -1, not enough pins
        #define BT_EMITTER_CONNECT -1 // must be -1, not enough pins

        #define TFT_BL      2  // same as RGB_PINS.bl
        #define AMP_ENABLED -1 // control pin for extenal amplifier (if available)

        #define I2C_SDA 19 // dala line for capacitive touchpadand and light sensor  (-1 if not used)
        #define I2C_SCL 20 // clock line for capacitive touchpad  and light sensor (-1 if not used)

    #endif
#endif
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DISPLAY [800x480] ESP32-S3 ELECROW 7"   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if 0                       // 0 deactivated, 1 activated
    #if TFT_CONTROLLER == 7 // RGB display
const Pins RGB_PINS = { // ELECROW 7"
    .b0 = 15, .b1 = 7,  .b2 = 6,  .b3 = 5,  .b4 = 4,  .g0 = 9,     .g1 = 46,    .g2 = 3,  .g3 = 8,   .g4 = 16, .g5 = 1,
    .r0 = 14, .r1 = 21, .r2 = 47, .r3 = 48, .r4 = 45, .hsync = 39, .vsync = 40, .de = 41, .pclk = 0, .bl = 2};

const Timing RGB_TIMING = {.h_res = 800,
                                    .v_res = 480,
                                    .pixel_clock_hz = 11000000,
                                    .hsync_pulse_width = 4,
                                    .hsync_back_porch = 160,
                                    .hsync_front_porch = 1,
                                    .vsync_pulse_width = 4,
                                    .vsync_back_porch = 140,
                                    .vsync_front_porch = 1};

        #define TP_IRQ             -1
        #define SD_MMC_CMD         11
        #define SD_MMC_CLK         12
        #define SD_MMC_D0          13
        #define GT911_I2C_ADDRESS  0x14 // default I2C-address of GT911
        #define I2S_DOUT           17
        #define I2S_BCLK           42
        #define I2S_LRC            18
        #define I2S_MCLK           -1 // important!
        #define IR_PIN             38 // IR Receiver (if available)
        #define BT_EMITTER_RX      -1 // must be -1, not enough pins
        #define BT_EMITTER_TX      -1 // must be -1, not enough pins
        #define BT_EMITTER_LINK    -1 // must be -1, not enough pins
        #define BT_EMITTER_MODE    -1 // must be -1, not enough pins
        #define BT_EMITTER_CONNECT -1 // must be -1, not enough pins
        #define TFT_BL             2  // same as RGB_PINS.bl
        #define AMP_ENABLED        -1 // control pin for extenal amplifier (if available)
        #define I2C_SDA            19 // I2C, data line for capacitive touchpad and light sensor (-1 if not available)
        #define I2C_SCL            20 // I2C, clock line for capacitive touchpad and light sensor (-1 if not available)
    #endif
#endif
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DISPLAY [800x480] ESP32-S3 WAVESHARE 7"   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if 0                       // 0 deactivated, 1 activated
    #if TFT_CONTROLLER == 7 // RGB display
const Pins RGB_PINS = { // WAVESHARE
    .b0 = 14, .b1 = 38, .b2 = 18, .b3 = 17, .b4 = 10, .g0 = 39,    .g1 = 0,    .g2 = 45, .g3 = 48,  .g4 = 47, .g5 = 21,
    .r0 = 1,  .r1 = 2,  .r2 = 42, .r3 = 41, .r4 = 40, .hsync = 46, .vsync = 3, .de = 5,  .pclk = 7, .bl = -1};

const Timing RGB_TIMING = {.h_res = 800,
                                    .v_res = 480,
                                    .pixel_clock_hz = 13000000,
                                    .hsync_pulse_width = 1,
                                    .hsync_back_porch = 1,
                                    .hsync_front_porch = 1,
                                    .vsync_pulse_width = 4,
                                    .vsync_back_porch = 4,
                                    .vsync_front_porch = 4};

        #define TP_IRQ -1

        #define SD_MMC_CMD 11
        #define SD_MMC_CLK 12
        #define SD_MMC_D0  13

        #define GT911_I2C_ADDRESS 0x14 // default I2C-address of GT911

        #define I2S_DOUT 19
        #define I2S_BCLK 20
        #define I2S_LRC  15
        #define I2S_MCLK -1 // important, don't change!

        #define IR_PIN             6  // IR Receiver (if available)
        #define BT_EMITTER_RX      -1 // must be -1, not enough pins
        #define BT_EMITTER_TX      -1 // must be -1, not enough pins
        #define BT_EMITTER_LINK    -1 // must be -1, not enough pins
        #define BT_EMITTER_MODE    -1 // must be -1, not enough pins
        #define BT_EMITTER_CONNECT -1 // must be -1, not enough pins

        #define TFT_BL      -1 // same as RGB_PINS.bl
        #define AMP_ENABLED -1 // control pin for extenal amplifier (if available)

        #define I2C_SDA 8 // I2C dala line for capacitive touchpad  (-1 if not used)
        #define I2C_SCL 9 // I2C clock line for capacitive touchpad (-1 if not used)
    #endif
#endif
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌  DSI-DISPLAY [1024x600] ESP32-P4 WAVESHARE 7"   📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if 1                       // 0 deactivated, 1 activated
    #if TFT_CONTROLLER == 8 // RGB display

const Timing DSI_TIMING = {.h_res = 1024,
                           .v_res = 600,
                           .pixel_clock_hz = 52000000,
                           .hsync_pulse_width = 10,
                           .hsync_back_porch = 160,
                           .hsync_front_porch = 160,
                           .vsync_pulse_width = 1,
                           .vsync_back_porch = 23,
                           .vsync_front_porch = 22};

        #define TP_IRQ -1

        #define SD_MMC_D0  39
        #define SD_MMC_D1  40
        #define SD_MMC_D2  41
        #define SD_MMC_D3  42
        #define SD_MMC_CLK 43
        #define SD_MMC_CMD 44

        #define GT911_I2C_ADDRESS 0x5D // default I2C-address of GT911

        #define I2S_DOUT 2
        #define I2S_BCLK 3
        #define I2S_LRC  4
        #define I2S_MCLK -1 // important, don't change!

        #define IR_PIN             5  // IR Receiver (if available)
        #define BT_EMITTER_RX      28
        #define BT_EMITTER_TX      29
        #define BT_EMITTER_LINK    30
        #define BT_EMITTER_MODE    31
        #define BT_EMITTER_CONNECT 34

        #define LCD_RESET   33
        #define TFT_BL      32 // same as RGB_PINS.bl
        #define AMP_ENABLED -1 // control pin for extenal amplifier (if available)

        #define I2C_SDA 7 // I2C dala line for capacitive touchpad
        #define I2C_SCL 8 // I2C clock line for capacitive touchpad
    #endif
#endif