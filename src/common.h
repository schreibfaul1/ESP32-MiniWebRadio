// created: 10.02.2022
// updated: 28.06.2026

#include "settings.h"

#if TFT_CONTROLLER == 0
    #define TFT_MODE_SPI
    #define TFT_LAYOUT_S
#endif

#if (TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5)
    #define TFT_MODE_SPI
    #define TFT_LAYOUT_M
#endif

#if (TFT_CONTROLLER == 7)
    #define TFT_MODE_RGB
    #define TFT_ALIGN_LANDSCAPE
    #define TFT_LAYOUT_L
#endif

#if (TFT_CONTROLLER == 8 || TFT_CONTROLLER == 9)
    #define TFT_MODE_DSI
    #define TFT_ALIGN_LANDSCAPE
    #define TFT_LAYOUT_XL
#endif

#if (TFT_CONTROLLER == 10)
    #define TFT_MODE_DSI
    #define TFT_ALIGN_PORTRAIT
    #define TFT_LAYOUT_L
#endif

#if (TP_CONTROLLER < 7)
    #define TP_MODE_XPT2046
#endif

#if (TP_CONTROLLER == 7)
    #define TP_MODE_GT911
#endif

#if (TP_CONTROLLER == 8)
    #define TP_MODE_FT6X63
#endif

#pragma once

#include "Audio.h"
#include "BH1750.h"
#include "DLNAClient.h"
#include "ESP32FtpServer.h"
#include "IR.h"
#include "SPIFFS.h"
#include "TCA9554.h"
#include "base64.h"
#include "driver/ledc.h"
#include "es8311.h"
#include "esp_log.h"
#include "esp_psram.h"
#include "kcx_bt_emitter.h"
#include "mbedtls/sha1.h"
#include "meteo.h"
#include "rtime.h"
#include "tft_common_defs.h"
#include "websrv.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FFat.h>
#include <FS.h>
#include <Preferences.h>
#include <SD_MMC.h>
#include <SPI.h>
#include <Ticker.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <Wire.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <vector>
#include <deque>
#include "mwr_src/function1.h"

Audio       audio;
Preferences pref;
WebSrv      webSrv;
WiFiMulti   wifiMulti;
RTIME       rtc;
Ticker      ticker100ms;
TwoWire     i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire     i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
SPIClass    spiBus(FSPI);


#include "tft_dsi.h"
#include "tft_rgb.h"
#include "tft_spi.h"
#include "tp_ft6x36.h"
#include "tp_gt911.h"
#include "tp_xpt2046.h"

#ifdef TFT_MODE_SPI // ⏹⏹⏹⏹
TFT_SPI  tft(spiBus, TFT_CS);
TFT_SPI& getTFT() {
    return tft;
}
#elif defined TFT_MODE_RGB // ⏹⏹⏹⏹
TFT_RGB  tft;
TFT_RGB& getTFT() {
    return tft;
}
#elif defined TFT_MODE_DSI // ⏹⏹⏹⏹
TFT_DSI  tft;
TFT_DSI& getTFT() {
    return tft;
}
#else
    #error "wrong TFT_CONTROLLER"
#endif

#ifdef TP_MODE_XPT2046 // ⏹⏹⏹⏹
TP_XPT2046  tp(spiBus, TP_CS);
TP_XPT2046& getTP() {
    return tp;
}
#elif defined TP_MODE_GT911  // ⏹⏹⏹⏹
TP_GT911  tp;
TP_GT911& getTP() {
    return tp;
}
#elif defined TP_MODE_FT6X63 // ⏹⏹⏹⏹
FT6x36  tp;
FT6x36& getTP() {
    return tp;
}
#else
    #error "wrong TP_CONTROLLER"
#endif

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  output on serial terminal
#define ANSI_ESC_RESET "\033[0m"

#define ANSI_ESC_BLACK        "\033[30m"
#define ANSI_ESC_RED          "\033[31m"
#define ANSI_ESC_GREEN        "\033[32m"
#define ANSI_ESC_YELLOW       "\033[33m"
#define ANSI_ESC_BLUE         "\033[34m"
#define ANSI_ESC_MAGENTA      "\033[35m"
#define ANSI_ESC_CYAN         "\033[36m"
#define ANSI_ESC_WHITE        "\033[37m"
#define ANSI_ESC_BG_BLACK     "\033[40m"
#define ANSI_ESC_BG_RED       "\033[41m"
#define ANSI_ESC_BG_GREEN     "\033[42m"
#define ANSI_ESC_BG_YELLOW    "\033[43m"
#define ANSI_ESC_BG_BLUE      "\033[44m"
#define ANSI_ESC_BG_MAGENTA   "\033[45m"
#define ANSI_ESC_BG_CYAN      "\033[46m"
#define ANSI_ESC_BG_WHITE     "\033[47m"
#define ANSI_ESC_GREY         "\033[90m"
#define ANSI_ESC_LIGHTRED     "\033[91m"
#define ANSI_ESC_LIGHTGREEN   "\033[92m"
#define ANSI_ESC_LIGHTYELLOW  "\033[93m"
#define ANSI_ESC_LIGHTBLUE    "\033[94m"
#define ANSI_ESC_LIGHTMAGENTA "\033[95m"
#define ANSI_ESC_LIGHTCYAN    "\033[96m"
#define ANSI_ESC_LIGHTGREY    "\033[97m"
#define ANSI_ESC_DARKRED      "\033[38;5;52m"
#define ANSI_ESC_DARKGREEN    "\033[38;5;22m"
#define ANSI_ESC_DARKYELLOW   "\033[38;5;136m"
#define ANSI_ESC_DARKBLUE     "\033[38;5;17m"
#define ANSI_ESC_DARKMAGENTA  "\033[38;5;53m"
#define ANSI_ESC_DARKCYAN     "\033[38;5;23m"
#define ANSI_ESC_DARKGREY     "\033[38;5;240m"
#define ANSI_ESC_BROWN        "\033[38;5;130m"
#define ANSI_ESC_ORANGE       "\033[38;5;214m"
#define ANSI_ESC_DARKORANGE   "\033[38;5;166m"
#define ANSI_ESC_LIGHTORANGE  "\033[38;5;215m"
#define ANSI_ESC_PURPLE       "\033[38;5;129m"
#define ANSI_ESC_PINK         "\033[38;5;213m"
#define ANSI_ESC_LIME         "\033[38;5;190m"
#define ANSI_ESC_NAVY         "\033[38;5;25m"
#define ANSI_ESC_AQUAMARINE   "\033[38;5;51m"
#define ANSI_ESC_LAVENDER     "\033[38;5;189m"
#define ANSI_ESC_LIGHTBROWN   "\033[38;2;210;180;140m"
#define ANSI_ESC_RESET        "\033[0m"
#define ANSI_ESC_BOLD         "\033[1m"
#define ANSI_ESC_FAINT        "\033[2m"
#define ANSI_ESC_ITALIC       "\033[3m"
#define ANSI_ESC_UNDERLINE    "\033[4m"
#define ANSI_ESC_BLINK        "\033[5m"
#define ANSI_ESC_INVERT       "\033[7m"
#define ANSI_ESC_STRIKE       "\033[9m"

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
struct _emojis {
    const char greenCircle[5] = {0xF0, 0x9F, 0x9F, 0xA2, 0x00};  // UTF-8: "🟢"
    const char yellowCircle[5] = {0xF0, 0x9F, 0x9F, 0xA1, 0x00}; // UTF-8: "🟡"
    const char redCircle[5] = {0xF0, 0x9F, 0x94, 0xB4, 0x00};    // UTF-8: "🔴"
    const char blueCircle[5] = {0xF0, 0x9F, 0x94, 0xB5, 0x00};   // UTF-8: "🔵"
    const char orangeCircle[5] = {0xF0, 0x9F, 0x9F, 0xA0, 0x00}; // UTF-8: "🟠"
    const char purpleCircle[5] = {0xF0, 0x9F, 0x9F, 0xA3, 0x00}; // UTF-8: "🟣"
    const char brownCircle[5] = {0xF0, 0x9F, 0x9F, 0xA4, 0x00};  // UTF-8: "🟤"
    const char greenSquare[5] = {0xF0, 0x9F, 0x9F, 0xA9, 0x00};  // UTF-8: "🟩"
    const char yellowSquare[5] = {0xF0, 0x9F, 0x9F, 0xA8, 0x00}; // UTF-8: "🟨"
    const char redSquare[5] = {0xF0, 0x9F, 0x9F, 0xA5, 0x00};    // UTF-8: "🟥"
    const char blueSquare[5] = {0xF0, 0x9F, 0x9F, 0xA6, 0x00};   // UTF-8: "🟦"
    const char orangeSquare[5] = {0xF0, 0x9F, 0x9F, 0xA7, 0x00}; // UTF-8: "🟧"
    const char purpleSquare[5] = {0xF0, 0x9F, 0x9F, 0xAA, 0x00}; // UTF-8: "🟪"
    const char brownSquare[5] = {0xF0, 0x9F, 0x9F, 0xAB, 0x00};  // UTF-8: "🟫"
} emoji;
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


// prototypes (main.cpp)
boolean      defaultsettings();
void         updateSettings();
void         fall_asleep();
void         wake_up(int8_t state, int8_t substate);
void         setRTC(ps_ptr<char> TZString);
boolean      isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime);
boolean      copySDtoFFat(const char* path);
void         showStationName();
void         showStreamTitle(ps_ptr<char> streamTitle);
void         showLogoAndStationName();
ps_ptr<char> getStationName();
ps_ptr<char> getLogoPath();
void         webSrv_send_station_items();
void         showFileLogo(int8_t state, int8_t subState);
void         showPlayerFileName(ps_ptr<char> fname);
void         show_DLNA_FileName(const char* fname);
void         showPlsFileNumber();
void         showAudioFileNumber();
void         display_sleeptime(int8_t ud = 0);
boolean      drawImage(ps_ptr<char> path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
boolean      isAudio(ps_ptr<char> path);
bool         connectToWiFi();
void         setWiFiCredentials(ps_ptr<char> ssid, ps_ptr<char> password);
ps_ptr<char> scaleImage(ps_ptr<char> path);
bool         detect_i2_c_devices(TwoWire* twi, int8_t sda, int8_t scl, i2c_items_s* i2c_items);
void         set_tft_items();
void         set_tp_items();
bool         init_SD_card();
void         setVolume(uint8_t vol);
uint8_t      downvolume();
uint8_t      upvolume();
void         setStation(ps_ptr<char> url, ps_ptr<char> extension = {});
void         setStation(uint16_t sta);
ps_ptr<char> getFlagPath(uint16_t station);
void         nextStation();
void         prevStation();
void         setStationByNumber(uint16_t staNr);
void         WEBSRV_onCommand(ps_ptr<char> cmd, ps_ptr<char> param, ps_ptr<char> arg);
void         WEBSRV_onRequest(ps_ptr<char> cmd, ps_ptr<char> param, ps_ptr<char> arg, ps_ptr<char> contentType, uint32_t contentLength);
void         WEBSRV_onDelete(ps_ptr<char> cmd, ps_ptr<char> param, ps_ptr<char> arg);
void         savefile(ps_ptr<char> fileName, uint32_t contentLength, ps_ptr<char> contenttype);
void         setI2STone();
ps_ptr<char> getI2STone();
void         SD_playFile(ps_ptr<char> pathWoFileName, ps_ptr<char> fileName);
void         SD_playFile(ps_ptr<char> path, uint32_t resumeFilePos = 0, bool showFN = true);
bool         SD_rename(ps_ptr<char> src, ps_ptr<char> dest);
bool         SD_newFolder(ps_ptr<char> folderPathName);
bool         SD_delete(ps_ptr<char> itemPath);
void         processPlaylist();
void         changeState(int8_t state, int8_t subState);
void         connecttohost(ps_ptr<char> host);
void         connecttoFS(const char* FS, ps_ptr<char> filename, uint32_t fileStartTime = 0);
void         stopSong();
void         placingGraphicObjects();
void         muteChanged(bool m);
void         setTimeCounter(uint8_t sec);
void         setTFTbrightness(uint8_t brightness);
ps_ptr<char> get_WiFi_PW(const char* ssid);
void         my_audio_info(Audio::msg_t m);
void         on_dlna_client(const DLNA_Client::msg_s& msg);
void         on_kcx_bt_emitter(const KCX_BT_Emitter::msg_s& msg);
void         on_websrv(const WebSrv::msg_s& msg);
void         on_meteo(const METEO::msg_s& msg);
void         tp_pressed(uint16_t x, uint16_t y);
void         tp_long_pressed(uint16_t x, uint16_t y);
void         tp_moved(uint16_t x, uint16_t y);
void         tp_released(uint16_t x, uint16_t y);
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline uint32_t simpleHash(ps_ptr<char> str) {
    if (str == NULL) return 0;
    uint32_t hash = 0;
    for (int32_t i = 0; i < str.strlen(); i++) {
        if (str[i] < 32) continue; // ignore control sign
        hash += (str[i] - 31) * i * 32;
    }
    return hash;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
inline int rfind(const char* str, char ch, int start = -1) { // same as indexof() burt from right to left
    if (!str) return -1;                                     // if str is NULL
    int len = strlen(str);
    if (start == -1 || start >= len) start = len - 1; // Default: Search from the end of the string

    for (int i = start; i >= 0; --i) {
        if (str[i] == ch) return i; // character found
    }
    return -1; // character not found
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    // --- Clamp Input ---
    if (x <= in_min) return out_min;
    if (x >= in_max) return out_max;

    // --- Normal map operation with 64-bit ---
    const int64_t run = int64_t(in_max) - int64_t(in_min);
    if (run == 0) {
        log_e("map(): Invalid range, %li == %li (min == max)", in_min, in_max);
        return out_min; // fallback
    }

    const int64_t rise = int64_t(out_max) - int64_t(out_min);
    const int64_t delta = int64_t(x) - int64_t(in_min);

    return int32_t((delta * rise) / run + out_min);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool setupBacklight(int pin, uint32_t freq_hz) {
    if (pin < 0) return false;

    ledc_channel_config_t ch =
        {.gpio_num = (gpio_num_t)pin, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_1, .intr_type = LEDC_INTR_DISABLE, .timer_sel = LEDC_TIMER_3, .duty = 0, .hpoint = 0};

    ledc_timer_config_t tmr = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_3,
        .freq_hz = freq_hz,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    // Timer zuerst initialisieren
    ledc_timer_config(&tmr);

    // Dann Channel anlegen
    ledc_channel_config(&ch);

    // Optional Helligkeit setzen
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 127);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    return true;
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
inline void x_ps_free(char** b) {
    if (*b) {
        free(*b);
        *b = NULL;
    }
}
inline void x_ps_free(unsigned char** b) {
    if (*b) {
        free(*b);
        *b = NULL;
    }
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// 📌📌📌   H A R D C O P Y    📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void writeDirect(File& file, int32_t width, int32_t height) {
    uint16_t row[width];
    for (int32_t y = height - 1; y >= 0; y--) {
        getTFT().readRect(0, y, width, 1, row);
        file.write((uint8_t*)row, width * sizeof(uint16_t));
    }
}

void writeRotated90(File& file, int32_t width, int32_t height) {
    uint16_t column[height];
    for (int32_t y = 0; y < width; y++) {
        getTFT().readRect(y, 0, 1, height, column);
        file.write((uint8_t*)column, height * sizeof(uint16_t));
    }
}

void writeRotated180(File& file, int32_t width, int32_t height) {
    uint16_t row[width];
    for (int32_t y = 0; y < height; y++) {
        getTFT().readRect(0, y, width, 1, row);
        std::reverse(row, row + width);
        file.write((uint8_t*)row, width * sizeof(uint16_t));
    }
}

void writeRotated270(File& file, int32_t width, int32_t height) {
    uint16_t column[height];
    for (int32_t y = width - 1; y >= 0; y--) {
        getTFT().readRect(y, 0, 1, height, column);
        std::reverse(column, column + height);
        file.write((uint8_t*)column, height * sizeof(uint16_t));
    }
}

void make_hardcopy_on_sd() {
    const uint8_t bmp320x240[70] = {
        0x42, 0x4D, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    (void)bmp320x240;

    const uint8_t bmp480x320[70] = {
        0x42, 0x4D, 0x46, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x40, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    (void)bmp480x320;

    const uint8_t bmp800x480[70] = {
        0x42, 0x4D, 0x46, 0xC4, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, // BM + File size 768070
        0x46, 0x00, 0x00, 0x00,                                     // Pixel data offset (70 bytes)
        0x28, 0x00, 0x00, 0x00,                                     // DIB header size
        0x20, 0x03, 0x00, 0x00,                                     // Width: 800
        0xE0, 0x01, 0x00, 0x00,                                     // Height: 480
        0x01, 0x00,                                                 // Color planes
        0x10, 0x00,                                                 // Bit count: 16 (RGB565)
        0x03, 0x00, 0x00, 0x00,                                     // Compression: BI_BITFIELDS (3)
        0x00, 0xB0, 0x04, 0x00,                                     // Image size (kept same, optional)
        0x23, 0x2E, 0x00, 0x00,                                     // X pixels per meter
        0x23, 0x2E, 0x00, 0x00,                                     // Y pixels per meter
        0x00, 0x00, 0x00, 0x00,                                     // Colors used
        0x00, 0x00, 0x00, 0x00,                                     // Important colors
        0x00, 0xF8, 0x00, 0x00,                                     // Red mask
        0xE0, 0x07, 0x00, 0x00,                                     // Green mask
        0x1F, 0x00, 0x00, 0x00,                                     // Blue mask
        0x00, 0x00, 0x00, 0x00                                      // Alpha mask (optional, empty)
    };
    (void)bmp800x480;

    const uint8_t bmp1024x600[70] = {
        0x42, 0x4D,             // 'BM'
        0x46, 0xC0, 0x12, 0x00, // File size: 1,228,870
        0x00, 0x00, 0x00, 0x00, // Reserved
        0x46, 0x00, 0x00, 0x00, // Pixel data offset (70)
        0x28, 0x00, 0x00, 0x00, // DIB header size (40)
        0x00, 0x04, 0x00, 0x00, // Width: 1024
        0x58, 0x02, 0x00, 0x00, // Height: 600
        0x01, 0x00,             // Planes
        0x10, 0x00,             // BitCount: 16 (RGB565)
        0x03, 0x00, 0x00, 0x00, // Compression: BI_BITFIELDS
        0x00, 0xC0, 0x12, 0x00, // Image size
        0x23, 0x2E, 0x00, 0x00, // X pixels per meter
        0x23, 0x2E, 0x00, 0x00, // Y pixels per meter
        0x00, 0x00, 0x00, 0x00, // Colors used
        0x00, 0x00, 0x00, 0x00, // Important colors
        0x00, 0xF8, 0x00, 0x00, // Red mask
        0xE0, 0x07, 0x00, 0x00, // Green mask
        0x1F, 0x00, 0x00, 0x00, // Blue mask
        0x00, 0x00, 0x00, 0x00  // Alpha mask
    };
    (void)bmp1024x600;

    File hc = SD_MMC.open("/hardcopy.bmp", "w", true);
#ifdef TFT_LAYOUT_S
    hc.write(bmp320x240, sizeof(bmp320x240));
    if (TFT_ROTATION == 0) writeDirect(hc, 320, 240);
    if (TFT_ROTATION == 1) writeRotated90(hc, 240, 320);
    if (TFT_ROTATION == 2) writeRotated180(hc, 320, 240);
    if (TFT_ROTATION == 3) writeRotated270(hc, 240, 320);
    hc.close();
#elifdef TFT_LAYOUT_M
    hc.write(bmp480x320, sizeof(bmp480x320));
    if (TFT_ROTATION == 0) writeDirect(hc, 480, 320);
    if (TFT_ROTATION == 1) writeRotated90(hc, 320, 480);
    if (TFT_ROTATION == 2) writeRotated180(hc, 480, 320);
    if (TFT_ROTATION == 3) writeRotated270(hc, 320, 480);
    hc.close();
#elifdef TFT_LAYOUT_L
    hc.write(bmp800x480, sizeof(bmp800x480));
    if (TFT_ROTATION == 0) writeDirect(hc, 800, 480);
    if (TFT_ROTATION == 1) writeRotated90(hc, 480, 800);
    if (TFT_ROTATION == 2) writeRotated180(hc, 800, 480);
    if (TFT_ROTATION == 3) writeRotated270(hc, 480, 800);
    hc.close();
#elifdef TFT_LAYOUT_XL
    hc.write(bmp1024x600, sizeof(bmp1024x600));
    if (TFT_ROTATION == 0) writeDirect(hc, 1024, 600);
    if (TFT_ROTATION == 1) writeRotated90(hc, 600, 1024);
    if (TFT_ROTATION == 2) writeRotated180(hc, 1024, 600);
    if (TFT_ROTATION == 3) writeRotated270(hc, 600, 1024);
    hc.close();
#else

#endif
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
