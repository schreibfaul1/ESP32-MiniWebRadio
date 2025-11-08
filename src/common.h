// created: 10.02.2022
// updated: 07.11.2025

#pragma once

#define _SSID             "mySSID"         // Your WiFi credentials here
#define _PW               "myWiFiPassword" // Or in textfile on SD-card
#define TFT_CONTROLLER    5                // (0)ILI9341, (3)ILI9486, (4)ILI9488, (5)ST7796, (7) RGB display
#define DISPLAY_INVERSION 0                // only SPI displays, (0) off (1) on
#define TFT_ROTATION      1                // only SPI displays, 1 or 3 (landscape)
#define TFT_FREQUENCY     80000000         // only SPI displays, 80000000, 40000000, 27000000, 20000000, 10000000
#define TP_VERSION        5                // only SPI displays, (0)ILI9341, (3)ILI9486, (4)ILI9488, (5)ST7796, (7)GT911
#define TP_ROTATION       1                // only SPI displays, 1 or 3 (landscape)
#define TP_H_MIRROR       0                // only SPI displays, (0) default, (1) mirror up <-> down
#define TP_V_MIRROR       0                // only SPI displays, (0) default, (1) mittor left <-> right
#define I2S_COMM_FMT      0                // (0) MAX98357A PCM5102A CS4344, (1) LSBJ (Least Significant Bit Justified format) PT8211
#define SDMMC_FREQUENCY   80000000         // 80000000 or 40000000 Hz
#define FTP_USERNAME      "esp32"          // user name in FTP Client
#define FTP_PASSWORD      "esp32"          // pw in FTP Client
#define CONN_TIMEOUT      2500             // unencrypted connection timeout in ms (http://...)
#define CONN_TIMEOUT_SSL  3500             // encrypted connection timeout in ms (https://...)
#define WIFI_TX_POWER     5                // 2 ... 21 (dBm) Adjust the WiFi transmission power to optimise power consumption or increase range, default: 5
#define LIST_TIMER        5                // After this time (seconds), the display returns from the list view
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include "Audio.h"
#include "BH1750.h"
#include "DLNAClient.h"
#include "ESP32FtpServer.h"
#include "IR.h"
#include "SPIFFS.h"
#include "base64.h"
#include "esp_log.h"
#include "kcx_bt_emitter.h"
#include "mbedtls/sha1.h"
#include "rtime.h"
#include "tft_rgb.h"
#include "tft_spi.h"
#include "tp_gt911.h"
#include "tp_xpt2046.h"
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
        #define I2C_SDA            41 // I2C, dala line for capacitive touchpad
        #define I2C_SCL            42 // I2C, clock line for capacitive touchpad
    #endif
    #if CONFIG_IDF_TARGET_ESP32P4
        #define TFT_CS             26
        #define TFT_DC             3
        #define TFT_BL             4 // at -1 the brightness menu is not displayed
        #define TP_IRQ             5
        #define TP_CS              2
        #define SD_MMC_D0          39 // also SDIO to ESP32-C6, do not change this pin
        #define SD_MMC_D1          40 // also SDIO to ESP32-C6, do not change this pin
        #define SD_MMC_D2          41 // also SDIO to ESP32-C6, do not change this pin
        #define SD_MMC_D3          42 // also SDIO to ESP32-C6, do not change this pin
        #define SD_MMC_CLK         43 // also SDIO to ESP32-C6, do not change this pin
        #define SD_MMC_CMD         44 // also SDIO to ESP32-C6, do not change this pin
        #define IR_PIN             32  // IR Receiver (if available)
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
        #define I2C_SDA            7  // I2C, dala line for capacitive touchpad
        #define I2C_SCL            8  // I2C, clock line for capacitive touch
    // free pins 32, 33, 46, 47, 48, 49, 50, 51, 52
    #endif // CONFIG_IDF_TARGET_ESP32P4
#endif     // CONFIG_IDF_TARGET_ESP32S3

#if TFT_CONTROLLER == 7 // RGB display
    #if CONFIG_IDF_TARGET_ESP32S3
const TFT_RGB::Pins RGB_PINS = { // SUNTON 7"
    .b0 = 15, .b1 = 7,  .b2 = 6,  .b3 = 5,  .b4 = 4,  .g0 = 9,     .g1 = 46,    .g2 = 3,  .g3 = 8,    .g4 = 16, .g5 = 1,
    .r0 = 14, .r1 = 21, .r2 = 47, .r3 = 48, .r4 = 45, .hsync = 39, .vsync = 40, .de = 41, .pclk = 42, .bl = 2};

const TFT_RGB::Timing RGB_TIMING = {.h_res = 800,
                                    .v_res = 480,
                                    .pixel_clock_hz = 10000000,
                                    .hsync_pulse_width = 30,
                                    .hsync_back_porch = 16,
                                    .hsync_front_porch = 210,
                                    .vsync_pulse_width = 13,
                                    .vsync_back_porch = 10,
                                    .vsync_front_porch = 22};

        #define TP_SDA             19
        #define TP_SCL             20
        #define TP_IRQ             -1
        #define SD_MMC_CMD         11
        #define SD_MMC_CLK         12
        #define SD_MMC_D0          13
        #define I2C_MASTER_FREQ_HZ 400000 // 400 kHz I2C-frequency
        #define GT911_I2C_ADDRESS  0x5D   // default I2C-address of GT911
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
        #define I2C_SDA            -1 //   19 // I2C line, same as dala line for capacitive touchpad  (-1 if not used) can be used for brightness sensor
        #define I2C_SCL            -1 //   20 // I2C line, same as clock line for capacitive touchpad (-1 if not used) can be used for brightness sensor
    #endif                            // CONFIG_IDF_TARGET_ESP32S3
    #if CONFIG_IDF_TARGET_ESP32P4
    // todo
    #endif
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

enum status {
    NONE = 0,
    RADIO = 1,
    PLAYER = 2,
    DLNA = 3,
    CLOCK = 4,
    BRIGHTNESS = 5,
    ALARMCLOCK = 6,
    SLEEPTIMER = 7,
    STATIONSLIST = 8,
    AUDIOFILESLIST = 9,
    DLNAITEMSLIST = 10,
    BLUETOOTH = 11,
    EQUALIZER = 12,
    SETTINGS = 13,
    IR_SETTINGS = 14,
    RINGING = 15,
    WIFI_SETTINGS = 16,
    UNDEFINED = 255
};

static bool                     newLine = false;
extern SemaphoreHandle_t        mutex_rtc;
extern RTIME                    rtc;
extern WebSrv                   webSrv;
extern std::deque<ps_ptr<char>> s_logBuffer;
void                            SerialPrintfln(const char* fmt, ...) {
    ps_ptr<char> myLog;
    if (newLine) {
        newLine = false;
        myLog.assign("\n");
    } else {
        myLog.assign("");
    }
    rtc.hasValidTime() ? myLog.append(rtc.gettime_s()) : myLog.append("00:00:00");
    myLog.append(" ");
    va_list args;
    va_start(args, fmt);
    myLog.appendf_va(fmt, args);
    va_end(args);
    myLog.append("\033[0m\r\n");
    Serial.printf("%s", myLog.c_get());
    s_logBuffer.insert(s_logBuffer.begin(), std::move(myLog)); // send to webSrv in loop()
    myLog.reset();
}

void SerialPrintfcr(const char* fmt, ...) {
    if (s_logBuffer.size() == 1024) s_logBuffer.pop_back();
    ps_ptr<char> myLog;
    rtc.hasValidTime() ? myLog.assign(rtc.gettime_s()) : myLog.assign("00:00:00");
    myLog.append(" ");
    va_list args;
    va_start(args, fmt);
    myLog.appendf_va(fmt, args);
    va_end(args);
    myLog.append("\033[0m\r");
    Serial.printf("%s", myLog.c_get());
    s_logBuffer.insert(s_logBuffer.begin(), std::move(myLog)); // send to webSrv in loop()
    myLog.reset();
    newLine = true;
}

int log_redirect_handler(const char* format, va_list args) {
    int len = vsnprintf(nullptr, 0, format, args) + 1;
    // Puffer für die formatierte Nachricht
    ps_ptr<char> log_buffer;
    log_buffer.alloc(len);
    vsnprintf(log_buffer.get(), len, format, args);
    if (len > 0) {
        int idx = log_buffer.index_of("ARDUINO:");
        if (idx > 0) {
            char c = log_buffer[7];
            log_buffer.remove_before(idx, true);
            log_buffer.insert("    ", 8);
            if (c == 'E') log_buffer.insert(ANSI_ESC_RED, 10);
            if (c == 'W') log_buffer.insert(ANSI_ESC_YELLOW, 10);
            if (c == 'I') log_buffer.insert(ANSI_ESC_GREEN, 10);
            if (c == 'D') log_buffer.insert(ANSI_ESC_CYAN, 10);
            if (c == 'V') log_buffer.insert(ANSI_ESC_GREY, 10);
            log_buffer.truncate_at(log_buffer.strlen() - 1); // remove '\n'
            SerialPrintfln("%s", log_buffer.c_get());
        } else {
            SerialPrintfln("%s", log_buffer.c_get());
        }
    }
    return 0;
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
struct dlnaHistory_s {
    ps_ptr<char> objId;
    ps_ptr<char> name;
    uint16_t     maxItems = 0;
};
struct releasedArg {
    char*   arg1 = NULL;
    char*   arg2 = NULL;
    char*   arg3 = NULL;
    int16_t val1 = 0;
    int16_t val2 = 0;
};
struct timecounter_s {
    uint8_t timer = 0;
    uint8_t factor = 2;
};
struct irButtons {
    int16_t val;
    char*   label;
};
struct settings_s {
    irButtons    irbuttons[45];
    uint8_t      numOfIrButtons = 0;
    ps_ptr<char> lastconnectedhost = {};
    ps_ptr<char> lastconnectedfile = {};
};

struct volume_s {
    uint8_t cur_volume = 21;
    uint8_t ringVolume = 21;
    uint8_t volumeAfterAlarm = 12;
    uint8_t volumeSteps = 21;
    uint8_t volumeCurve = 1;
};

struct bt_emitter_s {
    bool         found = false;
    bool         connect = false;
    bool         power_state = false;
    bool         play = true; // play: true, pause: false
    uint8_t      volume = 0;
    ps_ptr<char> mode;
    ps_ptr<char> version;
};

struct tone_s {
    int16_t LP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t BP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t HP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t BAL = 0; // -16...0....+16         audioI2S
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// prototypes (main.cpp)
bool         SD_MMC_exists(const char* path);
boolean      defaultsettings();
void         updateSettings();
void         urldecode(char* str);
void         setTFTbrightness(uint8_t duty);
void         fall_asleep();
void         wake_up();
void         setRTC(ps_ptr<char> TZString);
boolean      isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime);
boolean      copySDtoFFat(const char* path);
void         showStreamTitle(ps_ptr<char> streamTitle);
void         showLogoAndStationName(bool force);
void         showFileLogo(uint8_t state);
void         showFileName(const char* fname);
void         showPlsFileNumber();
void         showAudioFileNumber();
void         display_sleeptime(int8_t ud = 0);
boolean      drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
boolean      isAudio(File file);
boolean      isAudio(const char* path);
boolean      isPlaylist(File file);
bool         connectToWiFi();
void         setWiFiCredentials(const char* ssid, const char* password);
ps_ptr<char> scaleImage(ps_ptr<char> path);
void         setVolume(uint8_t vol);
uint8_t      downvolume();
uint8_t      upvolume();
void         setStation(uint16_t sta);
const char*  getFlagPath(uint16_t station);
void         nextStation();
void         prevStation();
void         setStationByNumber(uint16_t staNr);
void         StationsItems();
void         setStationViaURL(const char* url, const char* extension);
void         savefile(ps_ptr<char> fileName, uint32_t contentLength, ps_ptr<char> contenttype);
ps_ptr<char> setI2STone();
void         SD_playFile(ps_ptr<char> pathWoFileName, const char* fileName);
void         SD_playFile(ps_ptr<char> path, uint32_t resumeFilePos = 0, bool showFN = true);
bool         SD_rename(const char* src, const char* dest);
bool         SD_newFolder(const char* folderPathName);
bool         SD_delete(const char* itemPath);
bool         preparePlaylistFromDLNAFolder();
bool         preparePlaylistFromFile(const char* path);
bool         preparePlaylistFromSDFolder(const char* path);
void         sortPlayListAlphabetical();
void         sortPlayListRandom();
void         processPlaylist(boolean first = false);
void         changeState(int32_t state);
void         connecttohost(ps_ptr<char> host);
void         connecttoFS(const char* FS, const char* filename, uint32_t fileStartTime = 0);
void         stopSong();
void         placingGraphicObjects();
void         muteChanged(bool m);
void         setTimeCounter(uint8_t sec);
ps_ptr<char> get_WiFi_PW(const char* ssid);
void         my_audio_info(Audio::msg_t m);
void         on_dlna_client(const DLNA_Client::msg_s& msg);
void         on_kcx_bt_emitter(const KCX_BT_Emitter::msg_s& msg);
void         on_websrv(const WebSrv::msg_s& msg);

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline const char* byte_to_binary(int8_t x) { // e.g. alarmdays
    static char b[9];
    b[0] = '\0';

    int32_t z;
    for (z = 128; z > 0; z >>= 1) { strcat(b, ((x & z) == z) ? "1" : "0"); }
    return b;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline uint32_t simpleHash(const char* str) {
    if (str == NULL) return 0;
    uint32_t hash = 0;
    for (int32_t i = 0; i < strlen(str); i++) {
        if (str[i] < 32) continue; // ignore control sign
        hash += (str[i] - 31) * i * 32;
    }
    return hash;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t str2int(const char* str) {
    int32_t len = strlen(str);
    if (len > 0) {
        for (int32_t i = 0; i < len; i++) {
            if (!isdigit(str[i])) {
                log_e("NaN");
                return 0;
            }
        }
        return stoi(str);
    }
    return 0;
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* int2str(int32_t val) {
    static char ret[12];
    itoa(val, ret, 10);
    return ret;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void trim(char* s) {
    // fb   trim in place
    char* pe;
    char* p = s;
    while (isspace(*p)) p++; // left
    pe = p;                  // right
    while (*pe != '\0') pe++;
    do { pe--; } while ((pe > p) && isspace(*pe));
    if (p == s) {
        *++pe = '\0';
    } else { // move
        while (p <= pe) *s++ = *p++;
        *s = '\0';
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline bool startsWith(const char* base, const char* searchString) {
    if (base == NULL) {
        log_e("base = NULL");
        return false;
    } // guard
    if (searchString == NULL) {
        log_e("searchString == NULL");
        return false;
    } // guard
    if (strlen(searchString) > strlen(base)) return false;
    char c;
    while ((c = *searchString++) != '\0')
        if (c != *base++) return false;
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline bool endsWith(const char* base, const char* searchString) {
    if (base == NULL) {
        log_e("base = NULL");
        return false;
    } // guard
    if (searchString == NULL) {
        log_e("searchString == NULL");
        return false;
    } // guard
    int32_t slen = strlen(searchString);
    if (slen == 0) return false;
    const char* p = base + strlen(base);
    //  while(p > base && isspace(*p)) p--;  // rtrim
    p -= slen;
    if (p < base) return false;
    return (strncmp(p, searchString, slen) == 0);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t indexOf(const char* haystack, const char* needle, int32_t startIndex) {
    const char* p = haystack;
    for (; startIndex > 0; startIndex--)
        if (*p++ == '\0') return -1;
    char* pos = strstr(p, needle);
    if (pos == nullptr) return -1;
    return pos - haystack;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
inline int32_t lastIndexOf(const char* haystack, const char needle) {
    const char* p = strrchr(haystack, needle);
    return (p ? p - haystack : -1);
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
inline int replacestr(char* line, const char* search, const char* replace) { /* returns number of strings replaced.*/
    int   count;
    char* sp; // start of pattern
    // printf("replacestr(%s, %s, %s)\n", line, search, replace);
    if ((sp = strstr(line, search)) == NULL) { return (0); }
    count = 1;
    int sLen = strlen(search);
    int rLen = strlen(replace);
    if (sLen > rLen) {
        // move from right to left
        char* src = sp + sLen;
        char* dst = sp + rLen;
        while ((*dst = *src) != '\0') {
            dst++;
            src++;
        }
    } else if (sLen < rLen) {
        // move from left to right
        int   tLen = strlen(sp) - sLen;
        char* stop = sp + rLen;
        char* src = sp + sLen + tLen;
        char* dst = sp + rLen + tLen;
        while (dst >= stop) {
            *dst = *src;
            dst--;
            src--;
        }
    }
    memcpy(sp, replace, rLen);
    count += replacestr(sp + rLen, search, replace);
    return (count);
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_malloc(uint16_t len) {
    char* ps_str = NULL;
    if (psramFound()) { ps_str = (char*)ps_malloc(len); }
    if (!ps_str) { ps_str = (char*)malloc(len); }
    if (!ps_str) { log_e("oom"); }
    return ps_str;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_calloc(uint16_t len, uint8_t size) {
    char* ps_str = NULL;
    if (psramFound()) { ps_str = (char*)ps_calloc(len, size); }
    if (!ps_str) { ps_str = (char*)calloc(len, size); }
    if (!ps_str) { log_e("oom"); }
    return ps_str;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_strdup(const char* str) {
    if (!str) {
        log_e("str is NULL");
        return NULL;
    }
    char* ps_str = NULL;
    if (psramFound()) { ps_str = (char*)ps_malloc(strlen(str) + 1); }
    if (!ps_str) { ps_str = (char*)malloc(strlen(str) + 1); }
    if (!ps_str) {
        log_e("oom");
        return NULL;
    }
    strcpy(ps_str, str);
    return ps_str;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_strndup(const char* str, uint16_t n) { // with '\0' termination
    if (!str) {
        log_e("str is NULL");
        return NULL;
    }
    char* ps_str = NULL;
    if (psramFound()) { ps_str = (char*)ps_malloc(n + 1); }
    if (!ps_str) { ps_str = (char*)malloc(n + 1); }
    if (!ps_str) {
        log_e("oom");
        return NULL;
    }
    strncpy(ps_str, str, n);
    ps_str[n] = '\0';
    return ps_str;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int find_first_of(const char* source, const char* delimiters, int start = 0) {
    for (int i = start; source[i] != '\0'; ++i) {     // search at start
        for (int j = 0; delimiters[j] != '\0'; ++j) { // search delimiters
            if (source[i] == delimiters[j]) {
                return i; // position of first found delimiter
            }
        }
    }
    return -1; // not found
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int16_t strlenUTF8(const char* str) { // returns only printable glyphs, all ASCII and UTF-8 until 0xDFBD
    if (str == NULL) return -1;
    uint16_t idx = 0;
    uint16_t cnt = 0;
    while (*(str + idx) != '\0') {
        if ((*(str + idx) < 0xC0) && (*(str + idx) > 0x1F)) cnt++;
        if ((*(str + idx) == 0xE2) && (*(str + idx + 1) == 0x80)) cnt++; // general punctuation
        idx++;
    }
    return cnt;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    const int32_t run = in_max - in_min;
    if (run == 0) {
        log_e("map(): Invalid input range, %li == %li (min == max)", in_min, in_max);
        return -1; // AVR returns -1, SAM returns 0
    }
    const int32_t rise = out_max - out_min;
    const int32_t delta = x - in_min;
    return round((delta * rise) / run + out_min);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void SerialPrintflnCut(const char* item, const char* color, const char* str) {
    uint8_t maxLength = 100;
    if (strlen(str) > maxLength) {
        String f = str;
        SerialPrintfln("%s%s%s ... %s", item, color, f.substring(0, maxLength - 25).c_str(), f.substring(f.length() - 20, f.length()).c_str());
    } else {
        SerialPrintfln("%s%s%s", item, color, str);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if TFT_CONTROLLER < 7 // ⏹⏹⏹⏹
extern TFT_SPI tft;
#else
extern TFT_RGB tft;
#endif

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
inline void vector_clear_and_shrink(vector<char*>& vec) {
    uint size = vec.size();
    for (int32_t i = 0; i < size; i++) { x_ps_free(&vec[i]); }
    vec.clear();
    vec.shrink_to_fit();
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class RegisterTable {
  public:
    virtual ps_ptr<char> getName() = 0;
    virtual bool         isEnabled() = 0;
    virtual void         disable() = 0;
    virtual bool         positionXY(uint16_t, uint16_t) = 0;
    virtual ~RegisterTable() {}
};
static std::vector<RegisterTable*> registertable_objects;
static void                        register_object(RegisterTable* obj) {
    registertable_objects.push_back(obj);
}
inline void get_registered_names() {
    for (auto obj : registertable_objects) {
        printf(ANSI_ESC_WHITE "    registered object:" ANSI_ESC_YELLOW " %-17s" ANSI_ESC_WHITE " is enabled: %-5s" ANSI_ESC_RESET, "\n", obj->getName().c_get(),
               obj->isEnabled() ? ANSI_ESC_RED "yes" : ANSI_ESC_BLUE "no");
    }
}
inline void disableAllObjects() {
    for (auto obj : registertable_objects) { obj->disable(); }
}
inline const char* isObjectClicked(uint16_t x, uint16_t y) {
    static char objName[100];
    objName[0] = '\0';
    for (auto obj : registertable_objects) {
        if (obj->isEnabled() && obj->positionXY(x, y)) {
            if (strlen(objName) > 0) strcat(objName, ", ");
            strcat(objName, obj->getName().get());
        }
    }
    return objName;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Macro for comfortable calls
#define MWR_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
