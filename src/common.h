// created: 10.Feb.2022
// updated: 06.Jum.2025

#pragma once
#pragma GCC optimize("Os") // optimize for code size
// clang-format off
#define _SSID                   "mySSID"                        // Your WiFi credentials here
#define _PW                     "myWiFiPassword"                // Or in textfile on SD-card
#define TFT_CONTROLLER          7                               // (0)ILI9341, (3)ILI9486, (4)ILI9488, (5)ST7796, (7) RGB display
#define DISPLAY_INVERSION       0                               // only SPI displays, (0) off (1) on
#define TFT_ROTATION            1                               // only SPI displays, 1 or 3 (landscape)
#define TFT_FREQUENCY           40000000                        // only SPI displays, 80000000, 40000000, 27000000, 20000000, 10000000
#define TP_VERSION              5                               // only SPI displays, (0)ILI9341, (3)ILI9486, (4)ILI9488, (5)ST7796, (7)GT911
#define TP_ROTATION             1                               // only SPI displays, 1 or 3 (landscape)
#define TP_H_MIRROR             0                               // only SPI displays, (0) default, (1) mirror up <-> down
#define TP_V_MIRROR             0                               // only SPI displays, (0) default, (1) mittor left <-> right
#define I2S_COMM_FMT            0                               // (0) MAX98357A PCM5102A CS4344, (1) LSBJ (Least Significant Bit Justified format) PT8211
#define SDMMC_FREQUENCY         80000000                        // 80000000 or 40000000 Hz
#define FTP_USERNAME            "esp32"                         // user and pw in FTP Client
#define FTP_PASSWORD            "esp32"
#define CONN_TIMEOUT            2500                            // unencrypted connection timeout in ms (http://...)
#define CONN_TIMEOUT_SSL        3500                            // encrypted connection timeout in ms (https://...)
#define WIFI_TX_POWER           5                               // 2 ... 21 (dBm) Adjust the WiFi transmission power to optimise power consumption or increase range, default: 5
#define LIST_TIMER              5                               // After this time (seconds), the display returns from the list view
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Arduino.h>
#include "esp_log.h"
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <Ticker.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <FS.h>
#include <FFat.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include <vector>
#include "index.h"
#include "index.js.h"
#include "websrv.h"
#include "rtime.h"
#include "IR.h"
#include "tft_spi.h"
#include "tft_rgb.h"
#include "tp_xpt2046.h"
#include "tp_gt911.h"
#include "SPIFFS.h"
#include "ESP32FtpServer.h"
#include "Audio.h"
#include "DLNAClient.h"
#include "KCX_BT_Emitter.h"
#include "BH1750.h"
#include "base64.h"
#include <mbedtls/aes.h>
#include "mbedtls/sha1.h"
#include <mbedtls/base64.h>
#include "psram_unique_ptr.hpp"




#if TFT_CONTROLLER < 7
    // Digital I/O used
        #define TFT_CS              8
        #define TFT_DC             12
        #define TFT_BL             10 // at -1 the brightness menu is not displayed
        #define TP_IRQ             39
        #define TP_CS              15
        #define SD_MMC_D0          11
        #define SD_MMC_CLK         13
        #define SD_MMC_CMD         14
        #define IR_PIN              4  // IR Receiver (if available)
        #define TFT_MOSI           18  // TFT and TP (FSPI)
        #define TFT_MISO            2  // TFT and TP (FSPI)
        #define TFT_SCK            17  // TFT and TP (FSPI)

        #define I2S_DOUT            9
        #define I2S_BCLK            3
        #define I2S_LRC             1
        #define I2S_MCLK            0

        #define AMP_ENABLED        -1  // control pin for extenal amplifier (if available)
        #define BT_EMITTER_RX      45  // TX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_TX      38  // RX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_LINK    19  // high if connected                     (-1 if not available)
        #define BT_EMITTER_MODE    20  // high transmit - low receive           (-1 if not available)
        #define BT_EMITTER_CONNECT 48  // high impulse -> awake after POWER_OFF (-1 if not available)

        #define I2C_SDA            41  // I2C, dala line for capacitive touchpad
        #define I2C_SCL            42  // I2C, clock line for capacitive touchpad
#endif

#if TFT_CONTROLLER == 7 // RGB display

const TFT_RGB::Pins RGB_PINS = {  // SUNTON 7"
    .b0 = 15,
    .b1 = 7,
    .b2 = 6,
    .b3 = 5,
    .b4 = 4,
    .g0 = 9,
    .g1 = 46,
    .g2 = 3,
    .g3 = 8,
    .g4 = 16,
    .g5 = 1,
    .r0 = 14,
    .r1 = 21,
    .r2 = 47,
    .r3 = 48,
    .r4 = 45,
    .hsync = 39,
    .vsync = 40,
    .de = 41,
    .pclk = 42,
    .bl = 2
};

const TFT_RGB::Timing RGB_TIMING = {
    .h_res = 800,
    .v_res = 480,
    .pixel_clock_hz = 10000000,
    .hsync_pulse_width = 30,
    .hsync_back_porch = 16,
    .hsync_front_porch = 210,
    .vsync_pulse_width = 13,
    .vsync_back_porch = 10,
    .vsync_front_porch = 22
};

#define TP_SDA 19
#define TP_SCL 20
#define TP_IRQ -1

#define SD_MMC_CMD         11
#define SD_MMC_CLK         12
#define SD_MMC_D0          13

#define I2C_MASTER_FREQ_HZ 400000 // 400 kHz I2C-frequency
#define GT911_I2C_ADDRESS  0x5D   // default I2C-address of GT911

#define I2S_DOUT           17
#define I2S_BCLK           0
#define I2S_LRC            18
#define I2S_MCLK           -1  // important!

#define IR_PIN             38  // IR Receiver (if available)
#define BT_EMITTER_RX      -1  // must be -1, not enough pins
#define BT_EMITTER_TX      -1  // must be -1, not enough pins
#define BT_EMITTER_LINK    -1  // must be -1, not enough pins
#define BT_EMITTER_MODE    -1  // must be -1, not enough pins
#define BT_EMITTER_CONNECT -1  // must be -1, not enough pins

#define TFT_BL              2  // same as RGB_PINS.bl
#define AMP_ENABLED        -1  // control pin for extenal amplifier (if available)

#define I2C_SDA            19  // I2C line, same as dala line for capacitive touchpad  (-1 if not used) can be used for brightness sensor
#define I2C_SCL            20  // I2C line, same as clock line for capacitive touchpad (-1 if not used) can be used for brightness sensor



#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// output on serial terminal
#define ANSI_ESC_RESET          "\033[0m"
#define ANSI_ESC_BLACK          "\033[30m"
#define ANSI_ESC_RED            "\033[31m"
#define ANSI_ESC_GREEN          "\033[32m"
#define ANSI_ESC_YELLOW         "\033[33m"
#define ANSI_ESC_BLUE           "\033[34m"
#define ANSI_ESC_MAGENTA        "\033[35m"
#define ANSI_ESC_CYAN           "\033[36m"
#define ANSI_ESC_WHITE          "\033[37m"

#define ANSI_ESC_GREY           "\033[90m"
#define ANSI_ESC_LIGHTRED       "\033[91m"
#define ANSI_ESC_LIGHTGREEN     "\033[92m"
#define ANSI_ESC_LIGHTYELLOW    "\033[93m"
#define ANSI_ESC_LIGHTBLUE      "\033[94m"
#define ANSI_ESC_LIGHTMAGENTA   "\033[95m"
#define ANSI_ESC_LIGHTCYAN      "\033[96m"
#define ANSI_ESC_LIGHTGREY      "\033[97m"

#define ANSI_ESC_DARKRED        "\033[38;5;52m"
#define ANSI_ESC_DARKGREEN      "\033[38;5;22m"
#define ANSI_ESC_DARKYELLOW     "\033[38;5;136m"
#define ANSI_ESC_DARKBLUE       "\033[38;5;17m"
#define ANSI_ESC_DARKMAGENTA    "\033[38;5;53m"
#define ANSI_ESC_DARKCYAN       "\033[38;5;23m"
#define ANSI_ESC_DARKGREY       "\033[38;5;240m"

#define ANSI_ESC_BROWN          "\033[38;5;130m"
#define ANSI_ESC_ORANGE         "\033[38;5;214m"
#define ANSI_ESC_DARKORANGE     "\033[38;5;166m"
#define ANSI_ESC_LIGHTORANGE    "\033[38;5;215m"
#define ANSI_ESC_PURPLE         "\033[38;5;129m"
#define ANSI_ESC_PINK           "\033[38;5;213m"
#define ANSI_ESC_LIME           "\033[38;5;190m"
#define ANSI_ESC_NAVY           "\033[38;5;25m"
#define ANSI_ESC_AQUAMARINE     "\033[38;5;51m"
#define ANSI_ESC_LAVENDER       "\033[38;5;189m"

#define ANSI_ESC_LIGHTBROWN     "\033[38;2;210;180;140m"

#define ANSI_ESC_RESET          "\033[0m"
#define ANSI_ESC_BOLD           "\033[1m"
#define ANSI_ESC_FAINT          "\033[2m"
#define ANSI_ESC_ITALIC         "\033[3m"
#define ANSI_ESC_UNDERLINE      "\033[4m"
#define ANSI_ESC_BLINK          "\033[5m"
#define ANSI_ESC_INVERT         "\033[7m"
#define ANSI_ESC_STRIKE         "\033[9m"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
struct _emojis{
    const char  greenCircle[5]   = {0xF0, 0x9F, 0x9F, 0xA2, 0x00};     // UTF-8: "🟢"
    const char  yellowCircle[5]  = {0xF0, 0x9F, 0x9F, 0xA1, 0x00};     // UTF-8: "🟡"
    const char  redCircle[5]     = {0xF0, 0x9F, 0x94, 0xB4, 0x00};     // UTF-8: "🔴"
    const char  blueCircle[5]    = {0xF0, 0x9F, 0x94, 0xB5, 0x00};     // UTF-8: "🔵"
    const char  orangeCircle[5]  = {0xF0, 0x9F, 0x9F, 0xA0, 0x00};     // UTF-8: "🟠"
    const char  purpleCircle[5]  = {0xF0, 0x9F, 0x9F, 0xA3, 0x00};     // UTF-8: "🟣"
    const char  brownCircle[5]   = {0xF0, 0x9F, 0x9F, 0xA4, 0x00};     // UTF-8: "🟤"
    const char  greenSquare[5]   = {0xF0, 0x9F, 0x9F, 0xA9, 0x00};     // UTF-8: "🟩"
    const char  yellowSquare[5]  = {0xF0, 0x9F, 0x9F, 0xA8, 0x00};     // UTF-8: "🟨"
    const char  redSquare[5]     = {0xF0, 0x9F, 0x9F, 0xA5, 0x00};     // UTF-8: "🟥"
    const char  blueSquare[5]    = {0xF0, 0x9F, 0x9F, 0xA6, 0x00};     // UTF-8: "🟦"
    const char  orangeSquare[5]  = {0xF0, 0x9F, 0x9F, 0xA7, 0x00};     // UTF-8: "🟧"
    const char  purpleSquare[5]  = {0xF0, 0x9F, 0x9F, 0xAA, 0x00};     // UTF-8: "🟪"
    const char  brownSquare[5]   = {0xF0, 0x9F, 0x9F, 0xAB, 0x00};     // UTF-8: "🟫"
} emoji;
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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

static bool _newLine = false;
extern SemaphoreHandle_t mutex_rtc;
extern RTIME rtc;

extern WebSrv webSrv;
extern std::vector<ps_ptr<char>> _logBuffer;
void SerialPrintfln(const char* fmt, ...){
    ps_ptr<char>myLog("myLog");
    if(_newLine){_newLine = false; myLog.assign("\n");} else{myLog.assign("");}
    rtc.hasValidTime()? myLog.append(rtc.gettime_s()) : myLog.append("00:00:00");
    myLog.append(" ");
    va_list args;
    va_start(args, fmt);
    myLog.appendf_va(fmt, args);
    va_end(args);
    myLog.append("\033[0m\r\n");
    Serial.printf("%s", myLog.c_get());
    _logBuffer.insert(_logBuffer.begin(), std:: move(myLog)); // send to webSrv in loop()
    myLog.reset();
}

void SerialPrintfcr(const char* fmt, ...){
    if(_logBuffer.size() == 1024) _logBuffer.pop_back();
    ps_ptr<char>myLog("myLog");
    rtc.hasValidTime()? myLog.assign(rtc.gettime_s()) : myLog.assign("00:00:00");
    myLog.append(" ");
    va_list args;
    va_start(args, fmt);
    myLog.appendf_va(fmt, args);
    va_end(args);
    myLog.append("\033[0m\r");
    Serial.printf("%s", myLog.c_get());
    _logBuffer.insert(_logBuffer.begin(), std:: move(myLog));// send to webSrv in loop()
    myLog.reset();
    _newLine = true;
}

int log_redirect_handler(const char *format, va_list args) {
    int len = vsnprintf(nullptr, 0, format, args) + 1;
    // Puffer für die formatierte Nachricht
    ps_ptr<char>log_buffer("log_buffer");
    log_buffer.alloc(len);
    vsnprintf(log_buffer.get(), len, format, args);
    if (len > 0) {
        int idx = log_buffer.index_of("ARDUINO:");
        if(idx > 0){
            char c = log_buffer[7];
            log_buffer.remove_before(idx, true);
            log_buffer.insert("    ", 8);
            if(c == 'E') log_buffer.insert(ANSI_ESC_RED, 10);
            if(c == 'W') log_buffer.insert(ANSI_ESC_YELLOW, 10);
            if(c == 'I') log_buffer.insert(ANSI_ESC_GREEN, 10);
            if(c == 'D') log_buffer.insert(ANSI_ESC_CYAN, 10);
            if(c == 'V') log_buffer.insert(ANSI_ESC_GREY, 10);
            log_buffer.truncate_at(log_buffer.strlen() -1); // remove '\n'
            SerialPrintfln("%s", log_buffer.c_get());
        }
        else{
            SerialPrintfln("%s", log_buffer.c_get());
        }
    }return 0;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
struct dlnaHistory {
    char*    objId = NULL;
    char*    name = NULL;
    uint16_t maxItems = 0;
};
struct releasedArg {
    char*   arg1 = NULL;
    char*   arg2 = NULL;
    char*   arg3 = NULL;
    int16_t val1 = 0;
    int16_t val2 = 0;
};
struct timecounter {
    uint8_t timer = 0;
    uint8_t factor = 2;
};
struct irButtons {
    int16_t val;
    char*   label;
};
typedef struct __settings{
    irButtons irbuttons[45];
    uint8_t numOfIrButtons = 0;
    char*   lastconnectedhost = NULL;
    char*   lastconnectedfile = NULL;
} settings_t;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// clang-format on
// prototypes (main.cpp)
boolean        defaultsettings();
void           updateSettings();
void           urldecode(char* str);
const char*    SD_stringifyDirContent(String path);
void           setTFTbrightness(uint8_t duty);
void           fall_asleep();
void           wake_up();
void           setRTC(const char* TZString);
boolean        isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime);
boolean        copySDtoFFat(const char* path);
void           showStreamTitle(const char* streamTitle);
void           showLogoAndStationName(bool force);
void           showFileLogo(uint8_t state);
void           showFileName(const char* fname);
void           showPlsFileNumber();
void           showAudioFileNumber();
void           display_sleeptime(int8_t ud = 0);
boolean        drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
boolean        isAudio(File file);
boolean        isAudio(const char* path);
boolean        isPlaylist(File file);
bool           connectToWiFi();
void           setWiFiCredentials(const char* ssid, const char* password);
const char*    scaleImage(const char* path);
void           setVolume(uint8_t vol);
uint8_t        downvolume();
uint8_t        upvolume();
void           setStation(uint16_t sta);
const char*    getFlagPath(uint16_t station);
void           nextStation();
void           prevStation();
void           setStationByNumber(uint16_t staNr);
void           StationsItems();
void           setStationViaURL(const char* url, const char* extension);
void           savefile(const char* fileName, uint32_t contentLength);
String         setI2STone();
void           SD_playFile(const char* pathWoFileName, const char* fileName);
void           SD_playFile(const char* path, uint32_t resumeFilePos = 0, bool showFN = true);
bool           SD_rename(const char* src, const char* dest);
bool           SD_newFolder(const char* folderPathName);
bool           SD_delete(const char* itemPath);
bool           preparePlaylistFromDLNAFolder();
bool           preparePlaylistFromFile(const char* path);
bool           preparePlaylistFromSDFolder(const char* path);
void           sortPlayListAlphabetical();
void           sortPlayListRandom();
void           processPlaylist(boolean first = false);
void           changeState(int32_t state);
void           connecttohost(const char* host);
void           connecttoFS(const char* FS, const char* filename, uint32_t fileStartTime = 0);
void           stopSong();
void IRAM_ATTR headphoneDetect();
void           placingGraphicObjects();
void           muteChanged(bool m);
void           BTpowerChanged(int8_t newState); // true -> power on, false -> power off
void           setTimeCounter(uint8_t sec);
const char*    getWiFiPW(const char* ssid);
bool           setWiFiPW(const char* ssid, const char* password);
void           my_audio_info(Audio::msg_t m);

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

inline void hardcopy() {
    const uint8_t bmp320x240[70] = {
        0x42, 0x4D, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    const uint8_t bmp480x320[70] = {
        0x42, 0x4D, 0x46, 0xB0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x40, 0x01,
        0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x04, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x23, 0x2E, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
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

    File hc = SD_MMC.open("/hardcopy.bmp", "w", true);
    if (TFT_CONTROLLER < 2) {
        hc.write(bmp320x240, sizeof(bmp320x240));
        uint16_t buff[320];
        for (int i = 240; i > 0; i--) {
            tft.readRect(0, i - 1, 320, 1, buff);
            hc.write((uint8_t*)buff, 320 * 2);
        }
        hc.close();
    } else if (TFT_CONTROLLER < 7) {
        hc.write(bmp480x320, sizeof(bmp480x320));
        uint16_t buff[480];
        for (int i = 320; i > 0; i--) {
            tft.readRect(0, i - 1, 480, 1, buff);
            hc.write((uint8_t*)buff, 480 * 2);
        }
        hc.close();
    } else {
        hc.write(bmp800x480, sizeof(bmp800x480));
        uint16_t buff[800];
        for (int i = 480; i > 0; i--) {
            tft.readRect(0, i - 1, 800, 1, buff);
            hc.write((uint8_t*)buff, 800 * 2);
        }
        hc.close();
    }
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
inline void vector_clear_and_shrink(vector<char*>& vec) {
    uint size = vec.size();
    for (int32_t i = 0; i < size; i++) { x_ps_free(&vec[i]); }
    vec.clear();
    vec.shrink_to_fit();
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class RegisterTable {
  public:
    virtual const char* getName() = 0;
    virtual bool        isEnabled() = 0;
    virtual void        disable() = 0;
    virtual bool        positionXY(uint16_t, uint16_t) = 0;
    virtual ~RegisterTable() {}
};
static std::vector<RegisterTable*> registertable_objects;
static void                        register_object(RegisterTable* obj) {
    registertable_objects.push_back(obj);
}
inline void get_registered_names() {
    for (auto obj : registertable_objects) {
        printf(ANSI_ESC_WHITE "    registered object:" ANSI_ESC_YELLOW " %-17s" ANSI_ESC_WHITE " is enabled: %-5s\n", obj->getName(), obj->isEnabled() ? ANSI_ESC_RED "yes" : ANSI_ESC_BLUE "no");
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
            strcat(objName, obj->getName());
        }
    }
    return objName;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class IR_buttons {
  private:
    settings_t* m_settings;
    uint8_t     m_numOfIrButtons = 0;

  public:
    IR_buttons(settings_t* s) {
        m_settings = s;
        m_numOfIrButtons = 0;
    }
    ~IR_buttons() {}

    int16_t hexStringToInt16(const char* str) {
        if (strcmp(str, "-1") == 0) { // Check if it's the special case '-1'
            return -1;                // Special case for unused keys
        }
        // Processing the hexadecimal value
        if (strlen(str) >= 3 && str[0] == '0' && tolower(str[1]) == 'x') {
            return (int16_t)strtol(str, NULL, 16); // Convert Hexadecimal Value
        }
        Serial.println("Invalid format."); // Error output if the format is not correct
        return 0;
    }

    const char* skipWhitespace(const char* str) { // Helper function: Skip spaces
        while (*str && isspace(*str)) { str++; }
        return str;
    }

    const char* validateAndExtractString(const char* ptr, char** dest) { // Error handling: Validation of quoted strings
        if (*ptr == '\"') {
            ptr++; // skip '"'
            const char* start = ptr;
            while (*ptr && *ptr != '\"') { ptr++; }
            if (*ptr == '\"') {
                *dest = strndup(start, ptr - start); // allocate mem for string
                return ptr + 1;                      // skip '"'
            } else {
                Serial.println("Error: Unterminated string.");
                return NULL;
            }
        } else {
            Serial.println("Error: Expected string.");
            return NULL;
        }
    }

    bool parseJSONString(const char* jsonString) { // Function to parse the JSON string
        const char* ptr = jsonString;
        uint8_t     buttonNr = 0;
        size_t      buttonIndex = 0;

        // Check if the JSON string starts with '['
        ptr = skipWhitespace(ptr);
        if (*ptr != '[') {
            Serial.println("Error: Expected '[' to start array.");
            return false;
        }
        ptr++; // Überspringe '['

        // Process each JSON object in the array
        char key[16];
        while (*ptr && *ptr != ']' && buttonIndex < 43) {
            ptr = skipWhitespace(ptr);
            if (*ptr == '{') {
                ptr++; // skip '{'
                int16_t val = 0xFF;
                char*   label = NULL;
                bool    validObject = false;

                while (*ptr && *ptr != '}') {
                    ptr = skipWhitespace(ptr);

                    // Schlüssel extrahieren
                    if (*ptr == '\"') {
                        ptr++; // skip '"'
                        const char* keyStart = ptr;
                        while (*ptr && *ptr != '\"') { ptr++; }
                        memset(key, 0, 16);
                        strncpy(key, keyStart, ptr - keyStart);
                        ptr++; // skip '"'
                        ptr = skipWhitespace(ptr);

                        if (*ptr == ':') {
                            ptr++; // skip ':'
                            ptr = skipWhitespace(ptr);
                            // Value based on key
                            if (key[0] == 'A') { // IR Address
                                buttonNr = 42;
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (key[0] == 'C') {
                                ; // IR command unused
                                buttonNr = 43;
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (isdigit(key[0])) { // Nummer, z.B. "0", "10"
                                buttonNr = atoi(key);
                                char* str = NULL;
                                ptr = validateAndExtractString(ptr, &str);
                                if (!ptr) return false;      // error found
                                val = hexStringToInt16(str); // Hex in uint8_t umwandeln
                                x_ps_free(&str);
                                validObject = true;
                            } else if (strcmp(key, "label") == 0) { // Label
                                ptr = validateAndExtractString(ptr, &label);
                                if (!ptr) return false; // error found
                            }
                        }
                    }

                    ptr = skipWhitespace(ptr);
                    if (*ptr == ',') {
                        ptr++; // skip ','
                    }
                }

                // Make sure both values are present
                if (validObject && label != NULL) {
                    m_settings->irbuttons[buttonNr].val = val;
                    m_settings->irbuttons[buttonNr].label = label;
                    // log_w("buttonNr %i, val %i, label %s", buttonNr, m_settings->irbuttons[buttonNr].val, m_settings->irbuttons[buttonNr].label);
                    buttonIndex++;
                } else {
                    Serial.println("Error: Invalid object, missing buttonNr or label.");
                    return false;
                }

                ptr = skipWhitespace(ptr);
                if (*ptr == '}') {
                    ptr++; // skip '}'
                }

                ptr = skipWhitespace(ptr);
                if (*ptr == ',') {
                    ptr++; // skip ','
                }
            } else {
                Serial.println("Error: Expected '{' to start an object.");
                return false;
            }
        }

        // Check that the array ends correctly with ']'
        ptr = skipWhitespace(ptr);
        if (*ptr != ']') {
            Serial.println("Error: Expected ']' to close array.");
            return false;
        }
        return true; // JSON parsed successfully
    }

    uint8_t loadButtonsFromJSON(const char* filename) { // Function to load the JSON data
        File file = SD_MMC.open(filename);
        if (!file) {
            Serial.println("Failed to open file");
            return false;
        }
        String jsonString;
        while (file.available()) { jsonString += (char)file.read(); }
        file.close();
        //    log_w("%s", jsonString.c_str());
        // JSON parsen
        if (!parseJSONString(jsonString.c_str())) {
            Serial.println("Failed to parse JSON.");
            return false;
        }
        // debug output
        m_numOfIrButtons = 0;
        while (true) {
            if (m_settings->irbuttons[m_numOfIrButtons].label == NULL) break;

            // if(m_settings->irbuttons[m_numOfIrButtons].val == -1) log_w("IR_buttonNr %02i, value -1,   label %s", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].label);
            //  else log_w("IR_buttonNr %02i, value 0x%02X, label %s", m_numOfIrButtons, m_settings->irbuttons[m_numOfIrButtons].val, m_settings->irbuttons[m_numOfIrButtons].label);
            m_numOfIrButtons++;
        }
        m_settings->numOfIrButtons = m_numOfIrButtons;
        return m_numOfIrButtons;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class SD_content {
  private:
    struct FileInfo {
        int32_t fileSize;
        char*   fileName;
        char*   filePath;

        FileInfo(int32_t fs, const char* fn, const char* fp) : fileSize(fs), fileName(x_ps_strdup(fn)), filePath(x_ps_strdup(fp)) {} // constructor
        ~FileInfo() {
            x_ps_free(&fileName);
            x_ps_free(&filePath);
        } // destructor
        FileInfo(const FileInfo& other) : fileSize(other.fileSize), fileName(x_ps_strdup(other.fileName)), filePath(x_ps_strdup(other.filePath)) {} // copy constructor
        FileInfo& operator=(const FileInfo& other) {                                                                                                // copy assignment
            if (this != &other) {
                fileSize = other.fileSize;
                x_ps_free(&fileName);
                x_ps_free(&filePath);
                fileName = x_ps_strdup(other.fileName);
                filePath = x_ps_strdup(other.filePath);
            }
            return *this;
        }
    };
    std::vector<FileInfo> m_files;

    File  m_masterFile;
    File  m_slaveFile;
    char* m_buff = NULL;
    char* m_lastConnectedFile = NULL;
    char* m_lastConnectedFolder = NULL;
    char* m_lastConnectedFileName = NULL;

  public:
    SD_content() {
        m_buff = x_ps_malloc(1024);
        freeFilesVector();
    }
    ~SD_content() {
        freeFilesVector();
        x_ps_free(&m_buff);
    }
    bool listFilesInDir(const char* path, boolean audioFilesOnly, boolean withoutDirs) {
        if (!m_buff) {
            log_e("oom");
            return false;
        }
        freeFilesVector();
        if (m_masterFile) m_masterFile.close();
        if (!SD_MMC.exists(path)) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
            return false;
        }
        m_masterFile = SD_MMC.open(path);
        if (!m_masterFile.isDirectory()) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
            m_masterFile.close();
            return false;
        }
        while (true) { // get content
            m_slaveFile = m_masterFile.openNextFile();
            if (!m_slaveFile) break;
            if (m_slaveFile.isDirectory()) {
                if (!withoutDirs) { // folder size is -1
                    char* path = x_ps_malloc(strlen(m_slaveFile.path()) + 3);
                    strcpy(path, (const char*)m_slaveFile.path());
                    strcat(path, "/"); // add '/'
                    m_files.emplace_back((int)-1, m_slaveFile.name(), path);
                    x_ps_free(&path);
                }
            } else {
                if (audioFilesOnly) {
                    if (endsWith(m_slaveFile.name(), ".mp3") || endsWith(m_slaveFile.name(), ".aac") || endsWith(m_slaveFile.name(), ".m4a") || endsWith(m_slaveFile.name(), ".wav") ||
                        endsWith(m_slaveFile.name(), ".flac") || endsWith(m_slaveFile.name(), ".m3u") || endsWith(m_slaveFile.name(), ".opus") || endsWith(m_slaveFile.name(), ".ogg")) {
                        m_files.emplace_back(m_slaveFile.size(), m_slaveFile.name(), m_slaveFile.path());
                    }
                } else {
                    m_files.emplace_back(m_slaveFile.size(), m_slaveFile.name(), m_slaveFile.path());
                }
            }
        }
        sort();
        m_masterFile.close();
        return true;
    }

    bool isDir(uint16_t idx) {
        if (idx >= m_files.size()) {
            log_e("idx %i is oor, max = %i", idx, m_files.size() - 1);
            return false;
        }
        if (m_files[idx].fileSize == -1) return true;
        return false;
    }
    size_t      getSize() { return m_files.size(); }
    const char* getColouredSStringByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            log_w("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            log_w("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        if (isDir(idx)) return m_files[idx].fileName;
        sprintf(m_buff, "%s" ANSI_ESC_YELLOW " %li", m_files[idx].fileName, m_files[idx].fileSize);
        return m_buff;
    }
    const char* getFileNameByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            log_w("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            log_w("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        return m_files[idx].fileName;
    }

    int32_t getFileSizeByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            log_w("m_files.size() is 0");
            return 0;
        }
        if (m_files.size() < idx + 1) {
            log_w("idx %i is oor, max = %i", idx, m_files.size());
            return 0;
        }
        return m_files[idx].fileSize;
    }

    const char* getFilePathByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            log_w("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            log_w("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFilePathByIndex(0) returns "/dir_a/"
            getFilePathByIndex(3) returns "/dir_a/dir_b/file_b"
            getFilePathByIndex(5) returns "/dir_a/file_d"
        */
        return m_files[idx].filePath;
    }

    const char* getFileFolderByIndex(uint16_t idx) {
        if (m_files.size() == 0) {
            log_w("m_files.size() is 0");
            return "";
        }
        if (m_files.size() < idx + 1) {
            log_w("idx %i is oor, max = %i", idx, m_files.size());
            return "";
        }
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getFileFolderByIndex(0) returns "/dir_a/"
            getFileFolderByIndex(1) returns "/dir_a/dir_b/"
            getFileFolderByIndex(5) returns "/dir_a/"
        */
        if (isDir(idx)) return m_files[idx].filePath;
        int lastSlashIndex = lastIndexOf(m_files[idx].filePath, '/');
        strcpy(m_buff, m_files[idx].filePath);
        m_buff[lastSlashIndex + 1] = '\0';
        return m_buff;
    }

    int16_t getIndexByName(const char* path) {
        /*
            dir_a
                dir_b
                    file_a
                    file_b
                file_c
                file_d

            getIndexByName("/dir_a") returns 0
            getIndexByName("/dir_a/dir_b/file_b") returns 3
            getIndexByName("/dir_a/dir_b/file_y") returns -1
        */
        if (!path) return -1;
        for (int i = 0; i < m_files.size(); i++) {
            if (strcmp((const char*)m_files[i].filePath, path) == 0) { return i; }
        }
        return -1;
    }

    uint16_t getNextAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.size() == 0) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        int16_t newIdx = currIdx;
        while (true) {
            newIdx++;
            if (newIdx >= m_files.size()) newIdx = 0;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!endsWith(m_files[newIdx].fileName, ".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    uint16_t getPrevAudioFile(uint16_t currIdx) { // assume listFilesInDir with "audioFilesOnly"
        if (m_files.size() == 0) return 0;
        if (currIdx >= m_files.size()) currIdx = m_files.size() - 1;
        int16_t newIdx = currIdx;
        while (true) {
            newIdx--;
            if (newIdx == -1) newIdx = m_files.size() - 1;
            if (newIdx == currIdx) break;                           // avoid an infinite loop
            if (!endsWith(m_files[newIdx].fileName, ".m3u")) break; // skip m3u files
        }
        return newIdx;
    }

    void setLastConnectedFile(const char* lastconnectedItem) {
        /*  lastconnectedItem                       m_lastConnectedFolder       m_lastConnectedFileName     m_lastConnectedFile
            "/audiofiles/wavfiles/chicken.wav"      "/audiofiles/wavfiles/"     "chicken.wav"               "/audiofiles/wavfiles/chicken.wav"
            "xyz/chicken.wav"                       "/audiofiles/"              ""                          "/audiofiles/"                      // does not start with "/"
            "/audiofiles/wavfiles/chickenwav"       "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // file has no extension
            "/chicken.wav"                          "/"                         "chicken.wav"               "/chicken.wav"                      // we have no folder
            "/audiofiles/wavfiles/"                 "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // we have no file
            "/audiofiles/wavfiles/.wav"             "/audiofiles/wavfiles/"     ""                          "/audiofiles/wavfiles/"             // file has no name
        */
        x_ps_free(&m_lastConnectedFileName);
        x_ps_free(&m_lastConnectedFolder);
        int posFirst = 0, posLast = 0, posDot = 0;
        if (!lastconnectedItem) { // guard, lastconnectedItem == NULL
            m_lastConnectedFileName = x_ps_strdup("");
            m_lastConnectedFolder = x_ps_strdup("/audiofiles/");
            goto exit;
        }
        posFirst = indexOf(lastconnectedItem, "/", 0);
        posLast = lastIndexOf(lastconnectedItem, '/');
        if (posFirst != 0) { // guard, does not start with /
            m_lastConnectedFileName = x_ps_strdup("");
            m_lastConnectedFolder = x_ps_strdup("/audiofiles/");
            goto exit;
        }
        if (posLast == 0) {
            m_lastConnectedFolder = x_ps_strdup("/");
        } // we have no folder name
        else {
            m_lastConnectedFolder = x_ps_strndup(lastconnectedItem, posLast + 1);
        }

        if (posLast == strlen(lastconnectedItem) - 1) {
            m_lastConnectedFileName = x_ps_strdup("");
        } // we have no file name
        else {
            m_lastConnectedFileName = x_ps_strdup(lastconnectedItem + posLast + 1);
        }

        // log_e("posFirst %i, posLast %i, m_lastConnectedFileName %s, m_lastConnectedFolder %s", posFirst, posLast, m_lastConnectedFileName, m_lastConnectedFolder);
        posDot = indexOf(m_lastConnectedFileName, ".", 0);
        if (posDot == -1) { // no extension
            x_ps_free(&m_lastConnectedFileName);
            m_lastConnectedFileName = x_ps_strdup("");
        }

    exit:
        x_ps_free(&m_lastConnectedFile);
        m_lastConnectedFile = x_ps_malloc(strlen(m_lastConnectedFolder) + strlen(m_lastConnectedFileName) + 1);
        strcpy(m_lastConnectedFile, m_lastConnectedFolder);
        strcat(m_lastConnectedFile, m_lastConnectedFileName);
        // log_e("lastconnectedItem %s", lastconnectedItem);
        // log_e("lastConnectedFile %s", m_lastConnectedFile);
        // log_e("m_lastConnectedFileName %s", m_lastConnectedFileName);
        // log_e("m_lastConnectedFolder %s", m_lastConnectedFolder);
        listFilesInDir(m_lastConnectedFolder, true, false);
        sort();
    }
    const char* getLastConnectedFolder() { return m_lastConnectedFolder; }

    const char* getLastConnectedFileName() { return m_lastConnectedFileName; }
    int         getPosByFileName(const char* fileName) {
        for (int i = 0; i < m_files.size(); i++) {
            if (!strcmp(m_files[i].fileName, fileName)) return i; // fileName e.g. "file.mp3"
        }
        return 0;
    }

  private:
    void freeFilesVector() { m_files.clear(); }

    void sort() {
        std::sort(m_files.begin(), m_files.end(), [](const FileInfo& a, const FileInfo& b) {
            // Zuerst nach Ordner vs. Datei sortieren
            if (a.fileSize == -1 && b.fileSize != -1) {
                return true; // a ist Ordner, b ist Datei
            }
            if (a.fileSize != -1 && b.fileSize == -1) {
                return false; // a ist Datei, b ist Ordner
            }
            // Wenn beide entweder Ordner oder beide Dateien sind, alphabetisch sortieren
            return strcmp(a.fileName, b.fileName) < 0;
        });
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class stationManagement {
  private:
    struct sta {
        std::vector<uint8_t>  fav;
        std::vector<uint16_t> favStaNr;
        std::vector<char*>    country;
        std::vector<char*>    name;
        std::vector<char*>    url;
    } m_stations;

    uint16_t  m_staCnt = 0;
    uint16_t  m_staFavCnt = 0;
    uint16_t* m_curStation = 0;

  public:
    stationManagement(uint16_t* curStation) {
        clearStations();
        m_curStation = curStation;
    }
    ~stationManagement() { clearStations(); }

  private:
    void clearStations() {
        vector_clear_and_shrink(m_stations.country);
        vector_clear_and_shrink(m_stations.name);
        vector_clear_and_shrink(m_stations.url);
        m_stations.fav.clear();
        m_stations.fav.shrink_to_fit();
        m_stations.favStaNr.clear();
        m_stations.favStaNr.shrink_to_fit();

        m_stations.country.push_back(x_ps_strdup("unknown"));
        m_stations.name.push_back(x_ps_strdup("unknown"));
        m_stations.url.push_back(x_ps_strdup("unknown"));
        m_stations.fav.push_back('0');
        m_stations.favStaNr.push_back(0);
    }

  public:
    void updateStationsList() {
        clearStations();
        uint8_t item = 0;
        m_staCnt = 0;
        m_staFavCnt = 0;
        if (!SD_MMC.exists("/stations.json")) { return; }
        char buff[1024];
        File file = SD_MMC.open("/stations.json");
        while (file.available()) {
            char c = file.read();
            if (c == '[' || c == ']' || c == ',' || c == '\n' || c == '\r') { continue; } // skip
            if (c == '\"') {                                                              // start of string
                int pos = file.readBytesUntil('\"', buff, 1024);
                buff[pos] = 0;

                if (item == 0) {
                    m_stations.fav.push_back(buff[0]);
                    m_staCnt++;
                    if (buff[0] == '*') {
                        m_staFavCnt++;
                        m_stations.favStaNr.push_back(m_staCnt);
                    }
                }
                if (item == 1) { m_stations.country.push_back(x_ps_strdup(buff)); }
                if (item == 2) { m_stations.name.push_back(x_ps_strdup(buff)); }
                if (item == 3) { m_stations.url.push_back(x_ps_strdup(buff)); }
                item++;
                if (item > 3) item = 0;
                if (m_staCnt > 999) break;
            }
        }
        file.close();
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumStations() { return m_staCnt; }
    //----------------------------------------------------------------------------------------------------------
    uint16_t getSumFavStations() { return m_staFavCnt; }

    //----------------------------------------------------------------------------------------------------------
    uint16_t nextStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)++;
        if (*m_curStation > m_staCnt) *m_curStation = 1;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t nextFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp++;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp > m_staCnt) tmp = 1;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevStation() {
        if (!m_staCnt) return 1;
        (*m_curStation)--;
        if (*m_curStation < 1) *m_curStation = m_staCnt;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t prevFavStation() {
        if (!m_staCnt) return 1;
        uint16_t cnt = 0;
        int16_t  tmp = (*m_curStation);
        while (true) {
            tmp--;
            cnt++;
            if (cnt > m_staCnt) break;
            if (tmp < 1) tmp = m_staCnt;
            if (m_stations.fav[tmp] == '*') {
                *m_curStation = tmp;
                break;
            }
        }
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    uint16_t setStationByNumber(uint16_t staNr) {
        if (!m_staCnt) return 1;
        if (staNr > m_staCnt)
            *m_curStation = m_staCnt;
        else
            *m_curStation = staNr;
        return *m_curStation;
    }
    //----------------------------------------------------------------------------------------------------------
    const char* getStationName(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.name[staNr]) return strdup("unknown");
        return m_stations.name[staNr];
    }
    char getStationFav(uint16_t staNr) { // 0 = not fav, * = fav, 1..3 = fav number (notused)
        if (staNr > m_staCnt) return '0';
        if (!m_stations.fav[staNr]) return '0';
        return m_stations.fav[staNr];
    }
    const char* getStationUrl(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.url[staNr]) return strdup("unknown");
        return m_stations.url[staNr];
    }
    const char* getStationCountry(uint16_t staNr) {
        if (staNr > m_staCnt) return strdup("unknown");
        if (!m_stations.country[staNr]) return strdup("unknown");
        return m_stations.country[staNr];
    }
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     G R A P H I C   O B J E C T S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

extern __attribute__((weak)) void graphicObjects_OnChange(const char* name, int32_t arg1);
extern __attribute__((weak)) void graphicObjects_OnClick(const char* name, uint8_t val);
extern __attribute__((weak)) void graphicObjects_OnRelease(const char* name, releasedArg ra);

extern SemaphoreHandle_t mutex_display;
extern SD_content        _SD_content;
class slider : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_val = 0;
    int16_t     m_minVal = 0;
    int16_t     m_maxVal = 0;
    uint16_t    m_leftStop = 0;
    uint16_t    m_rightStop = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_railColor = 0;
    uint32_t    m_spotColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_objectInit = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    uint8_t     m_railHigh = 0;
    uint16_t    m_middle_h = 0;
    uint16_t    m_spotPos = 0;
    uint8_t     m_spotRadius = 0;
    uint8_t     m_padding_left = 0;
    uint8_t     m_padding_right = 0;
    uint8_t     m_padding_top = 0;
    uint8_t     m_padding_bottom = 0;
    char*       m_name = NULL;
    releasedArg m_ra;

  public:
    slider(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("slider");
        m_railHigh = 6;
        m_spotRadius = 12;
        m_bgColor = TFT_BLACK;
        m_railColor = TFT_BEIGE;
        m_spotColor = TFT_RED;
    }
    ~slider() { m_objectInit = false; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom, int16_t minVal, int16_t maxVal) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_padding_right = paddig_right;
        m_padding_top = paddig_top;       // unused
        m_padding_bottom = paddig_bottom; // unused
        m_minVal = minVal;
        m_maxVal = maxVal;
        m_leftStop = m_x + m_padding_left + m_spotRadius + 10;                          // x pos left stop
        m_rightStop = m_x + m_w - m_padding_left - m_padding_right - m_spotRadius - 10; // x pos right stop
        m_enabled = false;
        m_middle_h = m_y + (m_h / 2);
        m_spotPos = (m_leftStop + m_rightStop) / 2; // in the middle
        m_objectInit = true;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    bool        positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;

        if (m_enabled) {
            if (x < m_leftStop) x = m_leftStop; // (x, y) is in range
            if (x > m_rightStop) x = m_rightStop;
            m_clicked = true;
            drawNewSpot(x);
        }
        if (!m_clicked) {
            if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        }
        if (!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val) {
        if (!m_objectInit) return;
        if (val < m_minVal) val = m_minVal;
        if (val > m_maxVal) val = m_maxVal;
        m_val = val;
        if (m_clicked) return;
        uint16_t spotPos = map_l(val, m_minVal, m_maxVal, m_leftStop, m_rightStop); // val -> x
        if (m_enabled)
            drawNewSpot(spotPos);
        else
            m_spotPos = spotPos;
    }
    int16_t getValue() { return m_val; }
    void    setNewMinMaxVal(int16_t minVal, int16_t maxVal) {
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        int x = m_x + m_padding_left;
        int y = m_middle_h - (m_railHigh / 2);
        int w = m_w - m_padding_left - m_padding_right;
        int h = m_railHigh;
        (void)h;
        int r = 2;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        tft.fillRoundRect(x, y, w, m_railHigh, r, m_railColor);
        drawNewSpot(m_spotPos);
    }
    void disable() {
        if (m_enabled) hide();
        m_enabled = false;
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }

  private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if (run == 0) {
            log_e("map(): Invalid input range, %li == %li (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewSpot(uint16_t xPos) {
        if (m_enabled) {
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_spotPos - m_spotRadius - 1, m_middle_h - m_spotRadius - 1, 2 * m_spotRadius + 2, 2 * m_spotRadius + 2);
                else
                    tft.copyFramebuffer(1, 0, m_spotPos - m_spotRadius - 1, m_middle_h - m_spotRadius - 1, 2 * m_spotRadius + 2, 2 * m_spotRadius + 2);
            } else {
                tft.fillRect(m_spotPos - m_spotRadius, m_middle_h - m_spotRadius, 2 * m_spotRadius, 2 * m_spotRadius + 1, m_bgColor);
            }
            tft.fillRect(m_spotPos - m_spotRadius - 1, m_middle_h - (m_railHigh / 2), 2 * m_spotRadius + 2, m_railHigh, m_railColor);
            tft.fillCircle(xPos, m_middle_h, m_spotRadius, m_spotColor);
        }
        m_spotPos = xPos;
        int32_t val = map_l(m_spotPos, m_leftStop, m_rightStop, m_minVal, m_maxVal); // xPos -> val
        m_ra.val1 = val;
        if (graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, val);
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class progressbar : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_val = 0;
    int16_t     m_minVal = 0;
    int16_t     m_maxVal = 0;
    int16_t     m_oldPos = 0;
    uint16_t    m_padding_left = 0;
    uint16_t    m_padding_right = 0;
    uint16_t    m_padding_top = 0;
    uint16_t    m_padding_bottom = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_frameColor = 0;
    uint32_t    m_railColorLeft = 0;
    uint32_t    m_railColorRight = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_objectInit = false;
    bool        m_backgroundTransparency = true;
    bool        m_saveBackground = false;
    uint8_t     m_railHigh = 0;
    char*       m_name = NULL;
    releasedArg m_ra;

  public:
    progressbar(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("progressbar");
        m_railHigh = 6;
        m_bgColor = TFT_BLACK;
        m_frameColor = TFT_WHITE;
        m_railColorLeft = TFT_RED;
        m_railColorRight = TFT_GREEN;
    }
    ~progressbar() { m_objectInit = false; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t padding_left, uint16_t padding_right, uint16_t padding_top, uint16_t padding_bottom, int16_t minVal, int16_t maxVal) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = padding_left;
        m_padding_right = padding_right;
        m_padding_top = padding_top;
        m_padding_bottom = padding_bottom;
        m_minVal = minVal;
        m_maxVal = maxVal;
        m_enabled = false;
        m_objectInit = true;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    bool        positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        m_ra.val1 = map_l(x, m_x + 1, m_x + m_w - 2, m_minVal, m_maxVal);
        m_ra.val2 = m_ra.val1 - m_val; // offset
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val) {
        if (!m_objectInit) return;
        if (val < m_minVal) val = m_minVal;
        if (val > m_maxVal) val = m_maxVal;
        m_val = val;
        if (m_clicked) return;
        if (m_enabled) drawChanges();
    }
    int16_t getValue() { return m_val; }
    void    setNewMinMaxVal(int16_t minVal, int16_t maxVal) {
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        tft.drawRect(m_x + m_padding_left, m_y, m_w - m_padding_left - m_padding_right, m_h, m_frameColor);
        drawNewValue();
        m_enabled = true;
    }
    void disable() { m_enabled = false; }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
    void reset() {
        m_val = m_minVal;
        tft.fillRect(m_x, m_y + 1, m_w - m_h - 1, m_h - 2, m_railColorRight);
    }

  private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if (run == 0) {
            log_e("map(): Invalid input range, %li == %li (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewValue() {
        int      x = m_x + 1 + m_padding_left;
        int      w = m_w - 1 - m_padding_right;
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, x, x + w);
        tft.fillRect(x, m_y + 1, pos, m_h - 2, m_railColorLeft);
        tft.fillRect(pos, m_y + 1, w - pos, m_h - 2, m_railColorRight);
        m_oldPos = pos;
        if (graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, m_val);
    }
    void drawChanges() {
        int      x = m_x + 1 + m_padding_left;
        int      w = m_w - 1 - m_padding_right;
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, x, x + w);
        if (pos > m_oldPos) { tft.fillRect(m_oldPos, m_y + 1, pos - m_oldPos, m_h - 2, m_railColorLeft); }
        if (pos < m_oldPos) { tft.fillRect(pos, m_y + 1, m_oldPos - pos, m_h - 2, m_railColorRight); }
        m_oldPos = pos;
        if (graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, m_val);
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbox : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_h_align = TFT_ALIGN_RIGHT;
    uint8_t     m_v_align = TFT_ALIGN_TOP;
    uint8_t     m_padding_left = 0;  // left margin
    uint8_t     m_paddig_right = 0;  // right margin
    uint8_t     m_paddig_top = 0;    // top margin
    uint8_t     m_paddig_bottom = 0; // bottom margin
    uint8_t     m_borderWidth = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_fgColor = 0;
    uint32_t    m_borderColor = 0;
    char*       m_text = NULL;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_autoSize = false;
    bool        m_narrow = false;
    bool        m_noWrap = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    releasedArg m_ra;

  public:
    textbox(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~textbox() {
        x_ps_free(&m_text);
        x_ps_free(&m_name);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (!m_text) { m_text = strdup(""); }
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        writeText(m_text);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = m_padding_left + m_borderWidth;
        m_paddig_right = m_paddig_right + m_borderWidth;
        m_paddig_top = m_paddig_top + m_borderWidth;
        m_paddig_bottom = m_paddig_bottom + m_borderWidth;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
    void setText(const char* txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        if (!txt) { txt = strdup(""); }
        x_ps_free(&m_text);
        m_text = x_ps_strdup(txt);
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    void setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }

    void writeText(const char* txt) {
        if (!txt) { txt = strdup(""); }
        if (txt != m_text) { // no self copy
            x_ps_free(&m_text);
            m_text = x_ps_strdup(txt);
        }
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            tft.setTextColor(m_fgColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (m_borderWidth > 0) { tft.drawRect(m_x, m_y, m_w, m_h, m_borderColor); }
            if (m_borderWidth > 1) { tft.drawRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_borderColor); }
            tft.writeText(m_text, x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, m_autoSize);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class inputbox : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_h_align = TFT_ALIGN_RIGHT;
    uint8_t     m_v_align = TFT_ALIGN_TOP;
    uint8_t     m_padding_left = 0;  // left margin
    uint8_t     m_paddig_right = 0;  // right margin
    uint8_t     m_paddig_top = 0;    // top margin
    uint8_t     m_paddig_bottom = 0; // bottom margin
    uint8_t     m_borderWidth = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_fgColor = 0;
    uint32_t    m_borderColor = 0;
    char*       m_text = NULL;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_autoSize = false;
    bool        m_narrow = false;
    bool        m_noWrap = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    releasedArg m_ra;

  public:
    inputbox(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~inputbox() {
        x_ps_free(&m_text);
        x_ps_free(&m_name);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (!m_text) { m_text = strdup(""); }
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        writeText(m_text);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = m_padding_left + m_borderWidth;
        m_paddig_right = m_paddig_right + m_borderWidth;
        m_paddig_top = m_paddig_top + m_borderWidth;
        m_paddig_bottom = m_paddig_bottom + m_borderWidth;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
    void setText(const char* txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        if (!txt) { txt = strdup(""); }
        x_ps_free(&m_text);
        m_text = x_ps_strdup(txt);
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    void setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }
    void writeText(const char* txt) {
        if (!txt) { txt = strdup(""); }
        if (txt != m_text) { // no self copy
            x_ps_free(&m_text);
            m_text = x_ps_strdup(txt);
        }
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            tft.setTextColor(m_fgColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (m_borderWidth > 0) { tft.drawRect(m_x, m_y, m_w, m_h, m_borderColor); }
            if (m_borderWidth > 1) { tft.drawRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_borderColor); }

            uint16_t lineLength = 0;
            uint16_t txtMaxWidth = w - 2 * h;
            uint16_t idx = 0;
            lineLength = tft.getLineLength(m_text, m_narrow);
            while (lineLength > txtMaxWidth) {
                lineLength = tft.getLineLength(m_text + idx, m_narrow);
                if (lineLength > txtMaxWidth) {
                    idx++;
                    if (idx > strlen(m_text)) break;
                }
            }
            tft.writeText(m_text + idx, x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, false);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbutton : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_r = 0; // radius round rect
    uint8_t     m_fontSize = 0;
    uint8_t     m_h_align = TFT_ALIGN_RIGHT;
    uint8_t     m_v_align = TFT_ALIGN_TOP;
    uint8_t     m_padding_left = 0;  // left margin
    uint8_t     m_paddig_right = 0;  // right margin
    uint8_t     m_paddig_top = 0;    // top margin
    uint8_t     m_paddig_bottom = 0; // bottom margin
    uint8_t     m_borderWidth = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_fgColor = 0;
    uint32_t    m_borderColor = 0;
    uint32_t    m_clickColor = 0;
    char*       m_text = NULL;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_autoSize = false;
    bool        m_narrow = false;
    bool        m_noWrap = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    releasedArg m_ra;

  public:
    textbutton(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~textbutton() {
        x_ps_free(&m_text);
        x_ps_free(&m_name);
    }
    void drawTriangeUp() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_h - m_paddig_bottom;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = x0 + (x1 - x0) / 2;
        int16_t  y2 = m_y + m_paddig_top;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeDown() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_paddig_top;
        int16_t  x2 = x0 + (x1 - x0) / 2;
        int16_t  y2 = m_y + m_h - m_paddig_bottom;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeLeft() {
        int16_t  x0 = m_x + m_w - m_paddig_right;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = m_x + m_padding_left;
        int16_t  y2 = y0 + (y1 - y0) / 2;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeRight() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_padding_left;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = m_x + m_w - m_paddig_right;
        int16_t  y2 = y0 + (y1 - y0) / 2;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom, uint8_t radius) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_r = radius;
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (!m_text) { m_text = strdup(""); }
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        writeText(m_text);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setClickColor(uint32_t color) { m_clickColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = max(m_padding_left, m_borderWidth);
        m_paddig_right = max(m_paddig_right, m_borderWidth);
        m_paddig_top = max(m_paddig_top, m_borderWidth);
        m_paddig_bottom = max(m_paddig_bottom, m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        writeText(m_text);
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;

        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        writeText(m_text);
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
    void setText(const char* txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        if (!txt) { txt = strdup(""); }
        x_ps_free(&m_text);
        m_text = x_ps_strdup(txt);
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    const char* getText() { return m_text; }
    void        setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }
    void writeText(const char* txt) {
        if (!txt) { txt = strdup(""); }
        if (txt != m_text) { // no self copy
            x_ps_free(&m_text);
            m_text = x_ps_strdup(txt);
        }
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            if (!m_clicked)
                tft.setTextColor(m_fgColor);
            else
                tft.setTextColor(m_clickColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (!m_clicked) {
                if (m_borderWidth > 0) { tft.drawRoundRect(m_x, m_y, m_w, m_h, m_r, m_borderColor); }
                if (m_borderWidth > 1) { tft.drawRoundRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_r, m_borderColor); }
            } else {
                if (m_borderWidth > 0) { tft.drawRoundRect(m_x, m_y, m_w, m_h, m_r, m_clickColor); }
                if (m_borderWidth > 1) { tft.drawRoundRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_r, m_clickColor); }
            }
            if (strcmp(m_text, "/l") == 0) {
                drawTriangeLeft();
            } else if (strcmp(m_text, "/r") == 0) {
                drawTriangeRight();
            } else if (strcmp(m_text, "/u") == 0) {
                drawTriangeUp();
            } else if (strcmp(m_text, "/d") == 0) {
                drawTriangeDown();
            } else
                tft.writeText(m_text, x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, m_autoSize);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class selectbox : public RegisterTable {

    /*    —————————————————————————————————————————————————————————————————————————————
         |                   textbox                               |  ⏬  |  ⏫  |idx |
          —————————————————————————————————————————————————————————————————————————————
    */
  private:
    int16_t            m_x = 0;
    int16_t            m_y = 0;
    int16_t            m_w = 0;
    int16_t            m_h = 0;
    uint8_t            m_fontSize = 0;
    uint8_t            m_padding_left = 0;  // left margin
    uint8_t            m_paddig_right = 0;  // right margin
    uint8_t            m_paddig_top = 0;    // top margin
    uint8_t            m_paddig_bottom = 0; // bottom margin
    uint8_t            m_borderWidth = 0;
    int8_t             m_idx = 0;
    uint32_t           m_bgColor = 0;
    uint32_t           m_fgColor = 0;
    uint32_t           m_borderColor = 0;
    char*              m_name = NULL;
    bool               m_enabled = false;
    bool               m_clicked = false;
    bool               m_autoSize = false;
    bool               m_narrow = false;
    bool               m_noWrap = false;
    bool               m_backgroundTransparency = false;
    bool               m_saveBackground = false;
    releasedArg        m_ra;
    textbox*           m_txt_select = new textbox("select_txtbox_ssid");
    textbutton*        m_txt_btn_down = new textbutton("select_txtbtn_down");
    textbutton*        m_txt_btn_up = new textbutton("select_txtbtn_up");
    textbox*           m_txt_btn_idx = new textbox("select_txtbox_idx");
    std::vector<char*> m_selContent;

  public:
    selectbox(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        setFontSize(fontSize);
    }
    ~selectbox() {
        vector_clear_and_shrink(m_selContent);
        x_ps_free(&m_name);
        delete m_txt_select;
        delete m_txt_btn_down;
        delete m_txt_btn_up;
        delete m_txt_btn_idx;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        if (m_w < 40) {
            log_e("width < 40px");
            return;
        } // width
        m_h = h;
        if (m_h < 10) {
            log_e("height < 10px");
            return;
        } // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_txt_select->begin(m_x, m_y, m_w - (m_h * 3), m_h, m_padding_left, m_paddig_right, m_paddig_top, m_paddig_bottom);
        m_txt_btn_down->begin(m_x + m_w - (m_h * 3), m_y, m_h, m_h, m_h / 5, m_h / 5, m_h / 5, m_h / 5, 0);
        m_txt_btn_up->begin(m_x + m_w - (m_h * 2), m_y, m_h, m_h, m_h / 5, m_h / 5, m_h / 5, m_h / 5, 0);
        m_txt_btn_idx->begin(m_x + m_w - (m_h * 1), m_y, m_h, m_h, m_padding_left, m_paddig_right, m_paddig_top, m_paddig_bottom);
        m_txt_btn_down->setClickColor(TFT_CYAN);
        m_txt_btn_down->setText("/d");
        m_txt_btn_up->setClickColor(TFT_CYAN);
        m_txt_btn_up->setText("/u");
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_txt_select->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_txt_btn_down->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_txt_btn_up->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_txt_btn_idx->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_txt_select->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_down->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_up->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_idx->show(m_backgroundTransparency, m_saveBackground);
        m_enabled = true;
        m_clicked = false;

        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        m_idx = 0;
        if (m_selContent.size() > 0) writeText(m_idx);
    }
    void hide() {
        m_enabled = false;
        m_txt_select->hide();
        m_txt_btn_down->hide();
        m_txt_btn_up->hide();
        m_txt_btn_idx->hide();
    }
    void disable() {
        m_enabled = false;
        m_txt_select->disable();
        m_txt_btn_down->disable();
        m_txt_btn_up->disable();
        m_txt_btn_idx->disable();
    }
    void enable() {
        m_enabled = true;
        m_txt_select->enable();
        m_txt_btn_down->enable();
        m_txt_btn_up->enable();
        m_txt_btn_idx->enable();
    }
    void setFontSize(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
        m_txt_select->setFont(m_fontSize);
        m_txt_btn_down->setFont(m_fontSize);
        m_txt_btn_up->setFont(m_fontSize);
        m_txt_btn_idx->setFont(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        m_txt_select->setTextColor(m_fgColor);
        m_txt_btn_down->setTextColor(m_fgColor);
        m_txt_btn_up->setTextColor(m_fgColor);
        m_txt_btn_idx->setTextColor(m_fgColor);
    }
    void setBGcolor(uint32_t color) {
        m_bgColor = color;
        m_txt_select->setBGcolor(m_bgColor);
        m_txt_btn_down->setBGcolor(m_bgColor);
        m_txt_btn_up->setBGcolor(m_bgColor);
        m_txt_btn_idx->setBGcolor(m_bgColor);
    }
    void setBorderColor(uint32_t color) {
        m_borderColor = color;
        m_txt_select->setBorderColor(m_borderColor);
        m_txt_btn_down->setBorderColor(m_borderColor);
        m_txt_btn_up->setBorderColor(m_borderColor);
        m_txt_btn_idx->setBorderColor(m_borderColor);
    }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_txt_select->setBorderWidth(m_borderWidth);
        m_txt_btn_down->setBorderWidth(m_borderWidth);
        m_txt_btn_up->setBorderWidth(m_borderWidth);
        m_txt_btn_idx->setBorderWidth(m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (m_txt_select->positionXY(x, y)) { ; }
        if (m_txt_btn_down->positionXY(x, y)) { ; }
        if (m_txt_btn_up->positionXY(x, y)) { ; }
        if (m_txt_btn_idx->positionXY(x, y)) { ; }
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        bool ret = false;
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_txt_select->released();
        if (m_txt_btn_down->released())
            if (m_idx < m_selContent.size() - 1) {
                m_idx++; /* log_e("btn_down %i/%i", m_idx, m_selContent.size()); */
                writeText(m_idx);
                ret = true;
            }
        if (m_txt_btn_up->released())
            if (m_idx > 0) {
                m_idx--; /* log_e("btn_up %i/%i",   m_idx, m_selContent.size()); */
                writeText(m_idx);
                ret = true;
            }
        m_txt_btn_idx->released();
        m_clicked = false;
        return ret;
    }
    void addText(const char* txt) {
        if (!txt) { return; }
        if (m_selContent.size() > 0) {
            for (uint8_t i = 0; i < m_selContent.size(); i++) {
                if (strcmp(txt, m_selContent[i]) == 0) {
                    //    log_w("addText: %s already in list", txt);
                    return;
                }
            }
        }
        m_selContent.push_back(x_ps_strdup(txt));
    }
    void clearText() { vector_clear_and_shrink(m_selContent); }
    void writeText(uint8_t idx) {
        char* txt = NULL;
        if (idx >= m_selContent.size()) {
            txt = strdup("");
        } else
            txt = m_selContent[idx];
        if (m_enabled) {
            // log_w("writeText: %s", txt);
            m_txt_select->setText(txt, m_narrow, m_noWrap);
            m_txt_select->show(m_backgroundTransparency, m_saveBackground);
            char c_idx[5] = {0};
            itoa(idx + 1, c_idx, 10);
            m_txt_btn_idx->setText(c_idx, m_narrow, m_noWrap);
            m_txt_btn_idx->show(m_backgroundTransparency, m_saveBackground);
        }
    }
    char* getSelectedText() {
        if (m_selContent.size() > 0) { return m_selContent[m_idx]; }
        return NULL;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class keyBoard : public RegisterTable { // show time "hh:mm:ss" e.g. in header
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_r = 0;
    uint8_t     m_padding_left = 0;
    uint8_t     m_paddig_right = 0;
    uint8_t     m_paddig_top = 0;
    uint8_t     m_paddig_bottom = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_val = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    char*       m_name = NULL;
    const char* m_txt = NULL;
    float       m_row1[12] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float       m_row2[11] = {1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.6};
    float       m_row3[11] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.1};
    const char  m_alpha1[12][4] = {"1..", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "BS"};
    const char  m_alpha2[11][4] = {"A..", "a", "s", "d", "f", "g", "h", "j", "k", "l", "RET"};
    const char  m_alpha3[11][5] = {"#..", ".", "z", "x", "c", "v", "b", "n", "m", "_", "   "};
    const char  m_Alpha1[12][4] = {"1..", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "BS"};
    const char  m_Alpha2[11][4] = {"a..", "A", "S", "D", "F", "G", "H", "J", "K", "L", "RET"};
    const char  m_Alpha3[11][5] = {"#..", ".", "Z", "X", "C", "V", "B", "N", "M", "_", "   "};
    const char  m_special1[12][4] = {"1..", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BS"};
    const char  m_special2[11][4] = {"a..", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "RET"};
    const char  m_special3[11][5] = {"#..", "*", "+", ",", "-", "*", "-", ".", "/", ":", "   "};
    const char  m_Special1[12][4] = {"1..", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BS"};
    const char  m_Special2[11][4] = {"a..", ";", "<", "=", ">", "?", "@", "[", "\\", "]", "RET"};
    const char  m_Special3[11][5] = {"#..", "^", "_", "`", "{", "|", "}", "~", "#", "$", "   "};
    uint32_t    m_color1[12] = {TFT_YELLOW,    TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY,
                                TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_YELLOW};
    uint32_t    m_color2[11] = {TFT_YELLOW, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_RED};
    uint32_t    m_color3[11] = {TFT_YELLOW, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY};
    uint32_t    m_bgColor = 0;
    uint32_t    m_fgColor = 0;
    uint32_t    m_clickColor = TFT_CYAN;
    textbutton* txt_btn_array = new textbutton[34]{textbutton("txt_btn0"),  textbutton("txt_btn1"),  textbutton("txt_btn2"),  textbutton("txt_btn3"),  textbutton("txt_btn4"),  textbutton("txt_btn5"),
                                                   textbutton("txt_btn6"),  textbutton("txt_btn7"),  textbutton("txt_btn8"),  textbutton("txt_btn9"),  textbutton("txt_btn10"), textbutton("txt_btn11"),
                                                   textbutton("txt_btn12"), textbutton("txt_btn13"), textbutton("txt_btn14"), textbutton("txt_btn15"), textbutton("txt_btn16"), textbutton("txt_btn17"),
                                                   textbutton("txt_btn18"), textbutton("txt_btn19"), textbutton("txt_btn20"), textbutton("txt_btn21"), textbutton("txt_btn22"), textbutton("txt_btn23"),
                                                   textbutton("txt_btn24"), textbutton("txt_btn25"), textbutton("txt_btn26"), textbutton("txt_btn27"), textbutton("txt_btn28"), textbutton("txt_btn29"),
                                                   textbutton("txt_btn30"), textbutton("txt_btn31"), textbutton("txt_btn32"), textbutton("txt_btn33")};

  public:
    keyBoard(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("timeString");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_fontSize = fontSize;
    }
    ~keyBoard() {
        x_ps_free(&m_name);
        delete[] txt_btn_array;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        uint8_t btnW = (m_w - (paddig_left + paddig_right)) / 12;
        uint8_t btnH = (m_h - (paddig_top + paddig_bottom)) / 3;
        uint8_t margin = btnW / 17;
        btnW -= margin;
        btnH -= margin;
        uint8_t  radius = btnW / 10;
        uint16_t posX = m_x + paddig_left;
        uint16_t posY = m_y + m_paddig_top;
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_w = 12 * btnW + 11 * margin + paddig_left + paddig_right; // recalculate width
        m_h = 3 * btnH + 2 * margin + paddig_top + paddig_bottom;   // recalculate high
        for (int i = 0; i < 12; i++) {                              // row 1
            txt_btn_array[i].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row1[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i].setBGcolor(m_bgColor);
            txt_btn_array[i].setTextColor(m_color1[i]);
            txt_btn_array[i].setBorderColor(m_color1[i]);
            txt_btn_array[i].setClickColor(m_clickColor);
            txt_btn_array[i].setBorderWidth(1);
            txt_btn_array[i].setFont(m_fontSize);
            txt_btn_array[i].setText(m_alpha1[i]);
            txt_btn_array[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row1[i] * btnW + margin;
        }
        posY += btnH + margin;
        posX = m_x;
        for (int i = 0; i < 11; i++) { // row 2
            txt_btn_array[i + 12].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row2[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i + 12].setBGcolor(m_bgColor);
            txt_btn_array[i + 12].setTextColor(m_color2[i]);
            txt_btn_array[i + 12].setBorderColor(m_color2[i]);
            txt_btn_array[i + 12].setClickColor(m_clickColor);
            txt_btn_array[i + 12].setBorderWidth(1);
            txt_btn_array[i + 12].setFont(m_fontSize);
            txt_btn_array[i + 12].setText(m_alpha2[i]);
            txt_btn_array[i + 12].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row2[i] * btnW + margin;
        }
        posY += btnH + margin;
        posX = m_x;
        for (int i = 0; i < 11; i++) { // row 3
            txt_btn_array[i + 23].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row3[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i + 23].setBGcolor(m_bgColor);
            txt_btn_array[i + 23].setTextColor(m_color3[i]);
            txt_btn_array[i + 23].setBorderColor(m_color3[i]);
            txt_btn_array[i + 23].setClickColor(m_clickColor);
            txt_btn_array[i + 23].setBorderWidth(1);
            txt_btn_array[i + 23].setFont(m_fontSize);
            txt_btn_array[i + 23].setText(m_alpha3[i]);
            txt_btn_array[i + 23].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row3[i] * btnW + margin;
        }
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        if (!m_backgroundTransparency) tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        for (int i = 0; i < 34; i++) { txt_btn_array[i].show(m_backgroundTransparency, m_saveBackground); }
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void    disable() { m_enabled = false; }
    void    enable() { m_enabled = true; }
    uint8_t getVal() { return m_val; }
    bool    positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        for (int i = 0; i < 34; i++) {
            if (txt_btn_array[i].positionXY(x, y)) m_txt = txt_btn_array[i].getText();
        }
        if (m_txt) {
            if (strcmp(m_txt, "BS") == 0) {
                m_val = 8;
            } // BS
            else if (strcmp(m_txt, "RET") == 0) {
                m_val = 13;
            } // CR
            else if (strcmp(m_txt, "   ") == 0) {
                m_val = 32;
            } // space
            else if (strcmp(m_txt, "1..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "a..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "A..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "#..") == 0) {
                m_val = 0;
            } else
                m_val = m_txt[0];
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_val);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        // if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        for (int i = 0; i < 34; i++) {
            if (txt_btn_array[i].released()) {
                if (strcmp(txt_btn_array[i].getText(), "A..") == 0) { // upcase
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_Alpha1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_Alpha2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_Alpha3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "a..") == 0) { // lowcase
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_alpha1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_alpha2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_alpha3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "1..") == 0) { // special
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_special1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_special2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_special3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "#..") == 0) { // Special
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_Special1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_Special2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_Special3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
            }
        }
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class wifiSettings : public RegisterTable {
    /*                —————————————————————————————————————————————————————————————————————————————
                     |                             selectbox (SSID)            |  ⏬  |  ⏫  |idx |
                      —————————————————————————————————————————————————————————————————————————————
                      —————————————————————————————————————————————————————————————————————————————
                     |                             inputbox (Password)                            |
                      —————————————————————————————————————————————————————————————————————————————
                      —————————————————————————————————————————————————————————————————————————————
                     |                                                                            |
                     |                                                                            |
                     |                                 keyBoard                                   |
                     |                                                                            |
                     |                                                                            |
                      —————————————————————————————————————————————————————————————————————————————
    */
  private:
    int16_t            m_x = 0;
    int16_t            m_y = 0;
    int16_t            m_w = 0;
    int16_t            m_h = 0;
    uint8_t            m_fontSize = 0;
    uint8_t            m_padding_left = 0;  // left margin
    uint8_t            m_paddig_right = 0;  // right margin
    uint8_t            m_paddig_top = 0;    // top margin
    uint8_t            m_paddig_bottom = 0; // bottom margin
    uint8_t            m_pwd_idx = 0;
    uint8_t            m_borderWidth = 0;
    uint32_t           m_bgColor = 0;
    uint32_t           m_fgColor = 0;
    uint32_t           m_borderColor = TFT_BLACK;
    char*              m_name = NULL;
    bool               m_enabled = false;
    bool               m_clicked = false;
    bool               m_autoSize = false;
    bool               m_narrow = false;
    bool               m_noWrap = false;
    bool               m_backgroundTransparency = false;
    bool               m_saveBackground = false;
    std::vector<char*> m_ssid;
    std::vector<char*> m_password;
    releasedArg        m_ra;
    selectbox*         m_sel_ssid = new selectbox("wifiSettings_selectbox_ssid", 0);
    inputbox*          m_in_password = new inputbox("wifiSettings_txtbox_pwd");
    keyBoard*          m_keyboard = new keyBoard("wifiSettings_keyBoard", 0);
    struct w_se {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winSelect;
    struct w_pwd {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winPWD;
    struct w_k {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winKeybrd;

  public:
    wifiSettings(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_LIGHTGREY;
        setFontSize(fontSize);
        m_in_password->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_in_password->setTextColor(m_fgColor);
        m_in_password->setBGcolor(m_bgColor);
        m_in_password->setBorderColor(m_borderColor);
        m_in_password->setBorderWidth(m_borderWidth);
        m_in_password->setFont(0); // auto size
    }
    ~wifiSettings() {
        x_ps_free(&m_name);
        vector_clear_and_shrink(m_password);
        delete m_sel_ssid;
        delete m_in_password;
        delete m_keyboard;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {

        if (w == 320) { // s 320x240
            m_winSelect.x = 10;
            m_winSelect.y = 40;
            m_winSelect.w = 300;
            m_winSelect.h = 28;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 10;
            m_winPWD.y = 80;
            m_winPWD.w = 300;
            m_winPWD.h = 28;
            m_winPWD.pl = 1;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 10;
            m_winKeybrd.y = 120;
            m_winKeybrd.w = 300;
            m_winKeybrd.h = 75;
            m_winKeybrd.pl = 1;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        } else if (w == 480) {  // m 480x320
            m_winSelect.x = 12;
            m_winSelect.y = 50;
            m_winSelect.w = 456;
            m_winSelect.h = 30;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 12;
            m_winPWD.y = 90;
            m_winPWD.w = 456;
            m_winPWD.h = 30;
            m_winPWD.pl = 1;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 12;
            m_winKeybrd.y = 160;
            m_winKeybrd.w = 456;
            m_winKeybrd.h = 114;
            m_winKeybrd.pl = 1;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        } else if (w == 800) {  // l 800x480
            m_winSelect.x = 82;
            m_winSelect.y = 70;
            m_winSelect.w = 636;
            m_winSelect.h = 50;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 82;
            m_winPWD.y = 130;
            m_winPWD.w = 636;
            m_winPWD.h = 50;
            m_winPWD.pl = 1;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 82;
            m_winKeybrd.y = 240;
            m_winKeybrd.w = 636;
            m_winKeybrd.h = 160;
            m_winKeybrd.pl = 1;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        }
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        if (m_w < 40) {
            log_e("width < 40px");
            return;
        } // width
        m_h = h;
        if (m_h < 10) {
            log_e("height < 10px");
            return;
        } // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_sel_ssid->begin(m_winSelect.x, m_winSelect.y, m_winSelect.w, m_winSelect.h, m_winSelect.pl, m_winSelect.pr, m_winSelect.pt, m_winSelect.pb);
        m_in_password->begin(m_winPWD.x, m_winPWD.y, m_winPWD.w, m_winPWD.h, m_winPWD.pl, m_winPWD.pr, m_winPWD.pt, m_winPWD.pb);
        m_keyboard->begin(m_winKeybrd.x, m_winKeybrd.y, m_winKeybrd.w, m_winKeybrd.h, m_winKeybrd.pl, m_winKeybrd.pr, m_winKeybrd.pt, m_winKeybrd.pb);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_in_password->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_sel_ssid->show(m_backgroundTransparency, m_saveBackground);
        m_in_password->show(m_backgroundTransparency, m_saveBackground);
        m_keyboard->show(m_backgroundTransparency, m_saveBackground);
        m_enabled = true;
        m_clicked = false;

        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
    }
    void hide() {
        m_enabled = false;
        m_sel_ssid->hide();
        m_in_password->hide();
        m_keyboard->hide();
    }
    void disable() {
        m_enabled = false;
        m_sel_ssid->disable();
        m_in_password->disable();
        m_keyboard->disable();
    }
    void enable() {
        m_enabled = true;
        m_sel_ssid->enable();
        m_in_password->enable();
        m_keyboard->enable();
    }
    void setFontSize(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
        m_sel_ssid->setFontSize(m_fontSize);
        m_in_password->setFont(m_fontSize);
        //    m_keyboard->setFontSize(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        m_sel_ssid->setTextColor(m_fgColor);
        m_in_password->setTextColor(m_fgColor);
        // m_keyboard->setTextColor(m_fgColor);
    }
    void setBGcolor(uint32_t color) {
        m_bgColor = color;
        m_sel_ssid->setBGcolor(m_bgColor);
        m_in_password->setBGcolor(m_bgColor);
        //    m_keyboard->setBGcolor(m_bgColor);
    }
    void setBorderColor(uint32_t color) {
        m_borderColor = color;
        m_sel_ssid->setBorderColor(m_borderColor);
        m_in_password->setBorderColor(m_borderColor);
    }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_sel_ssid->setBorderWidth(m_borderWidth);
        m_in_password->setBorderWidth(m_borderWidth);
        //    m_keyboard->setBorderWidth(m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (m_sel_ssid->positionXY(x, y)) { ; }
        if (m_in_password->positionXY(x, y)) { ; }
        if (m_keyboard->positionXY(x, y)) {
            // log_e("key pressed %i", m_keyboard->getVal());
            changePassword(m_keyboard->getVal(), m_pwd_idx);
            m_in_password->setText(m_password[m_pwd_idx]);
            m_in_password->show(m_backgroundTransparency, m_saveBackground);
        }
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        bool ret = false;
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (m_sel_ssid->released()) {
            const char* selTxt = m_sel_ssid->getSelectedText();
            if (selTxt) {
                for (int i = 0; i < m_ssid.size(); i++) {
                    if (strcmp(selTxt, m_ssid[i]) == 0) {
                        m_in_password->setText(m_password[i]);
                        m_in_password->show(m_backgroundTransparency, m_saveBackground);
                        m_pwd_idx = i;
                    }
                }
            }
            ret = true;
        }
        if (m_in_password->released()) { /*log_e("m_txt_password released")*/
            ;
        }
        if (m_keyboard->released()) { /*log_e("keyboard released")*/
            ;
        }
        if (m_keyboard->getVal() == 0x0D) {    // enter
            m_ra.arg1 = m_ssid[m_pwd_idx];     // ssid
            m_ra.arg2 = m_password[m_pwd_idx]; // password
            // log_w("enter pressed ssid %s, password %s", m_ssid[m_pwd_idx], m_password[m_pwd_idx]);
            if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        }

        m_clicked = false;
        return ret;
    }
    void addWiFiItems(const char* ssid, const char* pw) {
        if (!ssid) { ssid = strdup(""); }
        char* pwd = x_ps_calloc(64, sizeof(char)); // password max 63 chars
        if (!pw) {
            strcpy(pwd, "");
        } else {
            strcpy(pwd, pw);
        }
        m_sel_ssid->addText(ssid);
        m_ssid.push_back(strdup(ssid));
        if (m_ssid.size() == 1) {
            m_in_password->setText(pwd);
            m_in_password->show(m_backgroundTransparency, m_saveBackground);
        }
        m_password.push_back(strndup(pwd, strlen(pwd) + 1));
        x_ps_free(&pwd);
    }
    void clearText() {
        m_sel_ssid->clearText();
        vector_clear_and_shrink(m_password);
        vector_clear_and_shrink(m_ssid);
        m_in_password->setText("");
    }
    char* getSelectedText() {
        //    if(m_selContent.size() > 0){return m_selContent[m_idx];}
        return NULL;
    }

  private:
    void changePassword(char ch, uint8_t idx) {
        if (ch == 0x08) { // backspace
            if (m_password[idx][0] == '\0') return;
            m_password[idx][strlen(m_password[idx]) - 1] = '\0';
        } else if (ch == 0x0D) { // enter
            //    log_w("enter pressed");
        } else {
            if (strlen(m_password[idx]) < 63) {
                int len = strlen(m_password[idx]);
                m_password[idx][len] = ch;
                m_password[idx][len + 1] = '\0';
            }
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class timeString : public RegisterTable { // show time "hh:mm:ss" e.g. in header
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_h_align = TFT_ALIGN_CENTER;
    uint8_t     m_v_align = TFT_ALIGN_CENTER;
    uint32_t    m_bgColor = 0;
    uint32_t    m_fgColor = 0;
    uint32_t    m_borderColor = 0;
    char*       m_name = NULL;
    char        m_time[10] = "00:00:00";
    bool        m_enabled = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false;
    bool        m_clicked = false;
    releasedArg m_ra;
    textbox*    txt_time = new textbox[8]{textbox("txt_timeH10"), textbox("txt_timeH01"), textbox("txt_timeC1"),  textbox("txt_timeM10"),
                                          textbox("txt_timeM01"), textbox("txt_timeC2"),  textbox("txt_timeS10"), textbox("txt_timeS01")}; // time of the day
  public:
    timeString(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("timeString");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_fontSize = fontSize;
    }
    ~timeString() {
        x_ps_free(&m_name);
        delete[] txt_time;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t pl, uint16_t pr, uint16_t pt, uint16_t pb) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        uint8_t  w_digits = m_w / 7;
        uint8_t  w_colon = w_digits / 2;
        uint16_t xPos[8] = {
            static_cast<uint16_t>(m_x + pl + 0 * w_digits + 0 * w_colon), /* H10 */
            static_cast<uint16_t>(m_x + pl + 1 * w_digits + 0 * w_colon), /* H01 */
            static_cast<uint16_t>(m_x + pl + 2 * w_digits + 0 * w_colon), /* C1 */
            static_cast<uint16_t>(m_x + pl + 2 * w_digits + 1 * w_colon), /* M10 */
            static_cast<uint16_t>(m_x + pl + 3 * w_digits + 1 * w_colon), /* M01 */
            static_cast<uint16_t>(m_x + pl + 4 * w_digits + 1 * w_colon), /* C2 */
            static_cast<uint16_t>(m_x + pl + 4 * w_digits + 2 * w_colon), /* S10 */
            static_cast<uint16_t>(m_x + pl + 5 * w_digits + 2 * w_colon)  /* S01 */
        };
        uint8_t width[8] = {w_digits, w_digits, w_colon, w_digits, w_digits, w_colon, w_digits, w_digits};
        for (uint8_t i = 0; i < 8; i++) {
            txt_time[i].begin(xPos[i], m_y + pt, width[i], h, 0, 0, 0, 0);
            txt_time[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_time[i].setTextColor(m_fgColor);
            txt_time[i].setFont(m_fontSize);
        }
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        updateTime(m_time, true);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = size;
        m_fontSize = size;
        tft.setFont(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        for (uint8_t i = 0; i < 8; i++) { txt_time[i].setTextColor(m_fgColor); }
    }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void updateTime(const char* hl_time, bool complete = true) {
        if (!hl_time) return;
        if (strlen(hl_time) != 8) return;
        if (!m_enabled) return;
        memcpy(m_time, hl_time, 8);     // hhmmss
        static char oldtime[8] = {255}; // hhmmss
        tft.setFont(m_fontSize);
        tft.setTextColor(m_fgColor);
        if (complete == true) {
            if (m_backgroundTransparency) {
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            for (uint8_t i = 0; i < 8; i++) { oldtime[i] = 255; }
        }
        for (uint8_t i = 0; i < 8; i++) {
            if (oldtime[i] != m_time[i]) {
                char ch[2] = {0, 0};
                ch[0] = m_time[i];
                txt_time[i].setText(ch, true);
                txt_time[i].show(m_backgroundTransparency, m_saveBackground);
                oldtime[i] = m_time[i];
            }
        }
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button1state : public RegisterTable { // click button
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = 0;
    char*       m_defaultPicturePath = NULL;
    char*       m_clickedPicturePath = NULL;
    char*       m_inactivePicturePath = NULL;
    char*       m_alternativePicturePath = NULL; // e.g. IR select
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_backgroundTransparency = false;
    char*       m_name = NULL;
    releasedArg m_ra;

  public:
    button1state(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("button1state");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        setDefaultPicturePath(NULL);
        setClickedPicturePath(NULL);
        setInactivePicturePath(NULL);
        setAlternativePicturePath(NULL);
    }
    ~button1state() {
        x_ps_free(&m_defaultPicturePath);
        x_ps_free(&m_clickedPicturePath);
        x_ps_free(&m_inactivePicturePath);
        x_ps_free(&m_alternativePicturePath);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool backgroundTransparency = false) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_backgroundTransparency = backgroundTransparency;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            setInactive();
            return;
        }
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_enabled = true;
    }
    void hide() {
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void setInactive() {
        drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        m_enabled = false;
    }
    void showAlternativePic(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            setInactive();
            return;
        }
        drawImage(m_alternativePicturePath, m_x, m_y, m_w, m_h);
    }
    void showClickedPic() { drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h); }
    void setDefaultPicturePath(const char* path) {
        x_ps_free(&m_defaultPicturePath);
        if (path)
            m_defaultPicturePath = x_ps_strdup(path);
        else
            m_defaultPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedPicturePath(const char* path) {
        x_ps_free(&m_clickedPicturePath);
        if (path)
            m_clickedPicturePath = x_ps_strdup(path);
        else
            m_clickedPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path) {
        x_ps_free(&m_inactivePicturePath);
        if (path)
            m_inactivePicturePath = x_ps_strdup(path);
        else
            m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }
    void setAlternativePicturePath(const char* path) {
        x_ps_free(&m_alternativePicturePath);
        if (path)
            m_alternativePicturePath = x_ps_strdup(path);
        else
            m_alternativePicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) {
            drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button2state : public RegisterTable { // on off switch
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = 0;
    char*       m_offPicturePath = NULL;
    char*       m_onPicturePath = NULL;
    char*       m_clickedOffPicturePath = NULL;
    char*       m_clickedOnPicturePath = NULL;
    char*       m_inactivePicturePath = NULL;
    char*       m_alternativeOnPicturePath = NULL;
    char*       m_alternativeOffPicturePath = NULL;
    bool        m_enabled = false;
    bool        m_active = true;
    bool        m_clicked = false;
    bool        m_state = false;
    char*       m_name = NULL;
    releasedArg m_ra;

  public:
    button2state(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("button2state");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        setOffPicturePath(NULL);
        setOnPicturePath(NULL);
        setClickedOffPicturePath(NULL);
        setClickedOnPicturePath(NULL);
        setInactivePicturePath(NULL);
    }
    ~button2state() {
        x_ps_free(&m_offPicturePath);
        x_ps_free(&m_onPicturePath);
        x_ps_free(&m_clickedOffPicturePath);
        x_ps_free(&m_clickedOnPicturePath);
        x_ps_free(&m_inactivePicturePath);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_active = true;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show() {
        m_clicked = false;
        if (m_active) {
            if (m_state)
                drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
            m_enabled = true;
        } else {
            drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void showClickedPic() {
        if (m_state) {
            drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
        } else {
            drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void showAlternativePic() {
        if (m_state) {
            drawImage(m_alternativeOnPicturePath, m_x, m_y, m_w, m_h);
        } else {
            drawImage(m_alternativeOffPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void setAlternativeOnPicturePath(const char* path) {
        x_ps_free(&m_alternativeOnPicturePath);
        if (path)
            m_alternativeOnPicturePath = x_ps_strdup(path);
        else
            m_alternativeOnPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    void setAlternativeOffPicturePath(const char* path) {
        x_ps_free(&m_alternativeOffPicturePath);
        if (path)
            m_alternativeOffPicturePath = x_ps_strdup(path);
        else
            m_alternativeOffPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void setValue(bool val) {
        m_state = val;
        if (m_enabled) {
            if (m_state)
                drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    bool getValue() { return m_state; }
    void setOn() { m_state = true; }
    void setOff() { m_state = false; }
    void setActive(bool act) { m_active = act; }
    bool getActive() { return m_active; }

    void setOffPicturePath(const char* path) {
        x_ps_free(&m_offPicturePath);
        if (path)
            m_offPicturePath = x_ps_strdup(path);
        else
            m_offPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedOffPicturePath(const char* path) {
        x_ps_free(&m_clickedOffPicturePath);
        if (path)
            m_clickedOffPicturePath = x_ps_strdup(path);
        else
            m_clickedOffPicturePath = x_ps_strdup("clickedOffPicturePath is not set");
    }
    void setClickedOnPicturePath(const char* path) {
        x_ps_free(&m_clickedOnPicturePath);
        if (path)
            m_clickedOnPicturePath = x_ps_strdup(path);
        else
            m_clickedOnPicturePath = x_ps_strdup("clickedOnPicturePath is not set");
    }
    void setOnPicturePath(const char* path) {
        x_ps_free(&m_onPicturePath);
        if (path)
            m_onPicturePath = x_ps_strdup(path);
        else
            m_onPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path) {
        x_ps_free(&m_inactivePicturePath);
        if (path)
            m_inactivePicturePath = x_ps_strdup(path);
        else
            m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) {
            if (m_state)
                drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
            m_state = !m_state;
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (m_state)
            drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
        else
            drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class numbersBox : public RegisterTable { // range 000...999
  private:
    bool        m_enabled = false;
    uint8_t     m_segmWidth = 0;
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = 0;
    bool        m_clicked = false;
    releasedArg m_ra;
    char*       m_name = NULL;
    char        m_root[20] = "/digits_small/";
    char        m_numbers[4] = "000";

  public:
    numbersBox(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("numbersBox");

        if (TFT_CONTROLLER < 2) {
            m_segmWidth = 48;
        } else if (TFT_CONTROLLER < 7) {
            m_segmWidth = 64;
        } else
            m_segmWidth = 86;
    }
    ~numbersBox() { x_ps_free(&m_name); }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
    }
    const char* getName() { return m_name; }
    bool        show() {
        if (!m_enabled) return false;
        char path[50];
        for (uint8_t i = 0; i < 3; i++) {
            sprintf(path, "%s%csbl.jpg", m_root, m_numbers[i]);
            if (!drawImage(path, m_x + i * m_segmWidth, m_y)) return false;
            ;
        }
        return true;
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    bool isEnabled() { return m_enabled; }
    void setNumbers(uint16_t numbers) {
        if (numbers > 999) return;
        snprintf(m_numbers, sizeof(m_numbers), "%03u", numbers);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class pictureBox : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint16_t    m_image_w = 0;
    uint16_t    m_image_h = 0;
    uint8_t     m_padding_left = 0;  // left margin
    uint8_t     m_paddig_right = 0;  // right margin
    uint8_t     m_paddig_top = 0;    // top margin
    uint8_t     m_paddig_bottom = 0; // bottom margin
    uint32_t    m_bgColor = 0;
    char*       m_PicturePath = NULL;
    char*       m_altPicturePath = NULL;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_backgroundTransparency = false;
    bool        m_saveBackground = false; // is used and to draw further objects on this box
    releasedArg m_ra;

  public:
    pictureBox(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("pictureBox");
        setPicturePath(NULL);
        setAlternativPicturePath(NULL);
    }
    ~pictureBox() {
        x_ps_free(&m_name);
        x_ps_free(&m_PicturePath);
        x_ps_free(&m_altPicturePath);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_enabled = false;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    bool        show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        int x = m_x + m_padding_left;
        int y = m_y + m_paddig_top;
        int w = m_w - (m_paddig_right + m_padding_left);
        int h = m_h - (m_paddig_bottom + m_paddig_top);
        if (!GetImageSize(m_PicturePath)) {
            GetImageSize(m_altPicturePath);

            if (m_saveBackground) { tft.copyFramebuffer(1, 2, m_x, m_y, m_w, m_h); }
            if (m_backgroundTransparency) { tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h); }
            if (m_altPicturePath)
                m_enabled = drawImage(m_altPicturePath, x, y, w, h);
            else
                m_enabled = false;
            if (m_saveBackground) { tft.copyFramebuffer(0, 1, m_x, m_y, m_w, m_h); }
            return m_enabled;
        } else {
            if (m_saveBackground) { tft.copyFramebuffer(1, 2, m_x, m_y, m_w, m_h); }
            if (m_backgroundTransparency) { tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h); }
            m_enabled = drawImage(m_PicturePath, x, y, w, h);
            if (m_saveBackground) { tft.copyFramebuffer(0, 1, m_x, m_y, m_w, m_h); }
            return m_enabled;
        }
    }
    void hide() {
        if (m_saveBackground) {
            tft.copyFramebuffer(2, 1, m_x, m_y, m_w, m_h); // restore background
        }
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() {
        if (m_saveBackground) {
            tft.copyFramebuffer(2, 1, m_x, m_y, m_w, m_h); // restore background
        }
        m_enabled = false;
    }
    void enable() { m_enabled = true; }
    void setPicturePath(const char* path) {
        if (m_PicturePath) {
            x_ps_free(&m_PicturePath);
            m_PicturePath = NULL;
        }
        if (path)
            m_PicturePath = x_ps_strdup(path);
        else
            m_PicturePath = x_ps_strdup("picturePath is not set");
        if (path) { GetImageSize(path); }
    }
    void setAlternativPicturePath(const char* path) {
        x_ps_free(&m_altPicturePath);
        if (path) m_altPicturePath = x_ps_strdup(path);
        //    else m_altPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }

  private:
    bool GetImageSize(const char* picturePath) {
        if (!picturePath) return false;
        const char* scaledPicPath = scaleImage(picturePath);
        if (!SD_MMC.exists(scaledPicPath)) { /* log_w("file %s not exists, objName: %s", scaledPicPath, m_name)*/
            ;
            return false;
        }
        File file = SD_MMC.open(scaledPicPath, "r", false);
        if (file.size() < 24) {
            log_w("file %s is too small", scaledPicPath);
            file.close();
            return false;
        }
        char buf[8];
        file.readBytes(buf, 3);
        if ((buf[0] == 0xFF) && (buf[1] == 0xD8) && (buf[2] == 0xFF)) { // format jpeg
            int16_t c1, c2;
            while (true) {
                c1 = file.read();
                if (c1 == -1) {
                    log_w("sof marker in %s not found", scaledPicPath);
                    file.close();
                    return false;
                } // end of file reached
                if (c1 == 0xFF) {
                    c2 = file.read();
                    if (c2 == 0xC0) break;
                } // 0xFFC0 Marker found
            }
            file.readBytes(buf, 7);
            m_image_h = buf[3] * 256 + buf[4];
            m_image_w = buf[5] * 256 + buf[6];
            //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        if ((buf[0] == 'B') && (buf[1] == 'M') && (buf[2] == '6')) { // format bmp
            for (int i = 0; i < 15; i++) file.read();                // read 15 dummys
            m_image_w = file.read();                                 // pos 18
            m_image_w += (file.read() << 8);
            m_image_w += (file.read() << 16);
            m_image_w += (file.read() << 24);
            m_image_h = file.read(); // pos 22
            m_image_h += (file.read() << 8);
            m_image_h += (file.read() << 16);
            m_image_h += (file.read() << 24);
            //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        if ((buf[0] == 'G') && (buf[1] == 'I') && (buf[2] == 'F')) { // format gif
            for (int i = 0; i < 3; i++) file.read();                 // read 3 dummys
            m_image_w = file.read();                                 // pos 6
            m_image_w += (file.read() << 8);
            m_image_h = file.read(); // pos 8
            m_image_h += (file.read() << 8);
            //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        if ((buf[0] == 0x89) && (buf[1] == 'P') && (buf[2] == 'N')) { // format png
            for (int i = 0; i < 13; i++) file.read();                 // read 13 dummys
            m_image_w = file.read() << 24;                            // pos 16
            m_image_w += file.read() << 16;                           // pos 17
            m_image_w += file.read() << 8;                            // pos 18
            m_image_w += file.read();                                 // pos 19
            m_image_h = file.read() << 24;                            // pos 20
            m_image_h += file.read() << 16;                           // pos 21
            m_image_h += file.read() << 8;                            // pos 22
            m_image_h += file.read();                                 // pos 23

            // bitDepth  = header[24];  // Position 24 = Bit-Tiefe
            // colorType = header[25];  // Position 25 = Farbtyp
            // log_w("w %i, h %i", m_w, m_h);
            return true;
        }
        log_e("unknown picture format %s", picturePath);
        return false;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock24 : public RegisterTable { // draw a clock in 24h format
  private:
    pictureBox* pic_clock24_digitsH10 = new pictureBox("clock24_digitsH10");     // digits hour   * 10
    pictureBox* pic_clock24_digitsH01 = new pictureBox("clock24_digitsH01");     // digits hour   * 01
    pictureBox* pic_clock24_digitsM10 = new pictureBox("clock24_digitsM10");     // digits minute * 10
    pictureBox* pic_clock24_digitsM01 = new pictureBox("clock24_digitsM01");     // digits minute * 01
    pictureBox* pic_clock24_digitsColon = new pictureBox("clock24_digitsColon"); // digits colon
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 4;
        uint16_t w = 72;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     72 x 120 px
    struct w_h01 {
        uint16_t x = 76;
        uint16_t w = 72;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     72 x 120 px
    struct w_c {
        uint16_t x = 148;
        uint16_t w = 24;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         24 x 120 px
    struct w_m10 {
        uint16_t x = 172;
        uint16_t w = 72;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   72 x 120 px
    struct w_m01 {
        uint16_t x = 244;
        uint16_t w = 72;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   72 x 120 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 32;
        uint16_t w = 96;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     96 x 160 px
    struct w_h01 {
        uint16_t x = 128;
        uint16_t w = 96;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     96 x 160 px
    struct w_c {
        uint16_t x = 224;
        uint16_t w = 32;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         32 x 160 px
    struct w_m10 {
        uint16_t x = 256;
        uint16_t w = 96;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   96 x 160 px
    struct w_m01 {
        uint16_t x = 352;
        uint16_t w = 96;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   96 x 160 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#else
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 36;
        uint16_t w = 168;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10    168 x 260 px
    struct w_h01 {
        uint16_t x = 204;
        uint16_t w = 168;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01    168 x 260 px
    struct w_c {
        uint16_t x = 372;
        uint16_t w = 56;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         56 x 260 px
    struct w_m10 {
        uint16_t x = 428;
        uint16_t w = 168;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10  168 x 260 px
    struct w_m01 {
        uint16_t x = 596;
        uint16_t w = 168;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01  168 x 260 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    bool        m_backgroundTransparency = false;
    uint16_t    m_digitsYPos = 0;
    bool        m_showAll = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    uint8_t     m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg m_ra;

  public:
    imgClock24(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("imgClock24");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock24() {
        x_ps_free(&m_name);
        x_ps_free(&m_pathBuff);
        delete pic_clock24_digitsH10;
        delete pic_clock24_digitsH01;
        delete pic_clock24_digitsColon;
        delete pic_clock24_digitsM10;
        delete pic_clock24_digitsM01;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_digitsYPos = m_y + m_digitsYoffset;
        pic_clock24_digitsH10->begin(m_x + s_h10.x, m_digitsYPos, s_h10.w, s_h10.h, s_h10.pl, s_h10.pr, s_h10.pt, s_h10.pb);
        pic_clock24_digitsH01->begin(m_x + s_h01.x, m_digitsYPos, s_h01.w, s_h01.h, s_h01.pl, s_h01.pr, s_h01.pt, s_h01.pb);
        pic_clock24_digitsColon->begin(m_x + s_c.x, m_digitsYPos, s_c.w, s_c.h, s_c.pl, s_c.pr, s_c.pt, s_c.pb);
        pic_clock24_digitsM10->begin(m_x + s_m10.x, m_digitsYPos, s_m10.w, s_m10.h, s_m10.pl, s_m10.pr, s_m10.pt, s_m10.pb);
        pic_clock24_digitsM01->begin(m_x + s_m01.x, m_digitsYPos, s_m01.w, s_m10.h, s_m01.pl, s_m01.pr, s_m01.pt, s_m01.pb);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() {
        m_enabled = false;
        m_showAll = false;
    }
    bool isDisabled() { return !m_enabled; }
    bool enable() { return m_enabled = true; }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday) {
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_hour = minuteOfTheDay / 60;
        m_min = minuteOfTheDay % 60;
        m_weekday = weekday;
        if (m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t m_min) {
        static uint8_t oldTime[4];
        static bool    k = false;
        uint8_t        time[5];
        time[0] = m_hour / 10;
        time[1] = m_hour % 10;
        time[2] = m_min / 10;
        time[3] = m_min % 10;

        for (uint8_t i = 0; i < 4; i++) {
            if ((time[i] != oldTime[i]) || m_showAll) {
                sprintf(m_pathBuff, "/digits/sevenSegment/%igreen.jpg", time[i]);
                if (i == 0) {
                    pic_clock24_digitsH10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_clock24_digitsH01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_clock24_digitsM10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_clock24_digitsM01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            oldTime[i] = time[i];
        }

        k = !k;
        if (k) {
            pic_clock24_digitsColon->setPicturePath("/digits/sevenSegment/dgreen.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        } else {
            pic_clock24_digitsColon->setPicturePath("/digits/sevenSegment/egreen.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        }
        m_showAll = false;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_clicked = false;
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock24small : public RegisterTable { // draw a clock in 24h format
  private:
    pictureBox* pic_clock24_digitsH10 = new pictureBox("clock24_digitsH10");     // digits hour   * 10
    pictureBox* pic_clock24_digitsH01 = new pictureBox("clock24_digitsH01");     // digits hour   * 01
    pictureBox* pic_clock24_digitsM10 = new pictureBox("clock24_digitsM10");     // digits minute * 10
    pictureBox* pic_clock24_digitsM01 = new pictureBox("clock24_digitsM01");     // digits minute * 01
    pictureBox* pic_clock24_digitsColon = new pictureBox("clock24_digitsColon"); // digits colon
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_digitsYoffset = 3;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 0;
        uint16_t w = 48;
        uint16_t h = 72;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     72 x 120 px
    struct w_h01 {
        uint16_t x = 48;
        uint16_t w = 48;
        uint16_t h = 72;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     72 x 120 px
    struct w_c {
        uint16_t x = 96;
        uint16_t w = 16;
        uint16_t h = 72;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         24 x 120 px
    struct w_m10 {
        uint16_t x = 112;
        uint16_t w = 48;
        uint16_t h = 72;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   72 x 120 px
    struct w_m01 {
        uint16_t x = 160;
        uint16_t w = 48;
        uint16_t h = 72;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   72 x 120 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7
    uint16_t m_digitsYoffset = 3;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 0;
        uint16_t w = 64;
        uint16_t h = 96;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     96 x 160 px
    struct w_h01 {
        uint16_t x = 64;
        uint16_t w = 64;
        uint16_t h = 96;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     96 x 160 px
    struct w_c {
        uint16_t x = 128;
        uint16_t w = 32;
        uint16_t h = 96;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         32 x 160 px
    struct w_m10 {
        uint16_t x = 160;
        uint16_t w = 64;
        uint16_t h = 96;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   96 x 160 px
    struct w_m01 {
        uint16_t x = 224;
        uint16_t w = 64;
        uint16_t h = 96;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   96 x 160 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#else
    uint16_t m_digitsYoffset = 3;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 0;
        uint16_t w = 86;
        uint16_t h = 144;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10    168 x 260 px
    struct w_h01 {
        uint16_t x = 86;
        uint16_t w = 86;
        uint16_t h = 144;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01    168 x 260 px
    struct w_c {
        uint16_t x = 172;
        uint16_t w = 31;
        uint16_t h = 144;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         56 x 260 px
    struct w_m10 {
        uint16_t x = 203;
        uint16_t w = 86;
        uint16_t h = 144;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10  168 x 260 px
    struct w_m01 {
        uint16_t x = 289;
        uint16_t w = 86;
        uint16_t h = 144;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01  168 x 260 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    bool        m_backgroundTransparency = false;
    uint16_t    m_digitsYPos = 0;
    bool        m_showAll = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    uint8_t     m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg m_ra;

  public:
    imgClock24small(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("imgClock24");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock24small() {
        x_ps_free(&m_name);
        x_ps_free(&m_pathBuff);
        delete pic_clock24_digitsH10;
        delete pic_clock24_digitsH01;
        delete pic_clock24_digitsColon;
        delete pic_clock24_digitsM10;
        delete pic_clock24_digitsM01;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_digitsYPos = m_y + m_digitsYoffset;
        pic_clock24_digitsH10->begin(m_x + s_h10.x, m_digitsYPos, s_h10.w, s_h10.h, s_h10.pl, s_h10.pr, s_h10.pt, s_h10.pb);
        pic_clock24_digitsH01->begin(m_x + s_h01.x, m_digitsYPos, s_h01.w, s_h01.h, s_h01.pl, s_h01.pr, s_h01.pt, s_h01.pb);
        pic_clock24_digitsColon->begin(m_x + s_c.x, m_digitsYPos, s_c.w, s_c.h, s_c.pl, s_c.pr, s_c.pt, s_c.pb);
        pic_clock24_digitsM10->begin(m_x + s_m10.x, m_digitsYPos, s_m10.w, s_m10.h, s_m10.pl, s_m10.pr, s_m10.pt, s_m10.pb);
        pic_clock24_digitsM01->begin(m_x + s_m01.x, m_digitsYPos, s_m01.w, s_m10.h, s_m01.pl, s_m01.pr, s_m01.pt, s_m01.pb);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() {
        m_enabled = false;
        m_showAll = false;
    }
    bool isDisabled() { return !m_enabled; }
    bool enable() { return m_enabled = true; }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday) {
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_hour = minuteOfTheDay / 60;
        m_min = minuteOfTheDay % 60;
        m_weekday = weekday;
        if (m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t m_min) {
        static uint8_t oldTime[4];
        static bool    k = false;
        uint8_t        time[5];
        time[0] = m_hour / 10;
        time[1] = m_hour % 10;
        time[2] = m_min / 10;
        time[3] = m_min % 10;

        for (uint8_t i = 0; i < 4; i++) {
            if ((time[i] != oldTime[i]) || m_showAll) {
                sprintf(m_pathBuff, "/digits_small/%isrt.jpg", time[i]);
                if (i == 0) {
                    pic_clock24_digitsH10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_clock24_digitsH01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_clock24_digitsM10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_clock24_digitsM01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            oldTime[i] = time[i];
        }

        k = !k;
        if (k) {
            pic_clock24_digitsColon->setPicturePath("/digits_small/dsrt.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        } else {
            pic_clock24_digitsColon->setPicturePath("/digits_small/esrt.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        }
        m_showAll = false;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_clicked = false;
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock12 : public RegisterTable { // draw a clock in 12h format
  private:
    pictureBox* pic_clock12_digitsH10 = new pictureBox("clock12_digitsH10");       // digits hour   * 10
    pictureBox* pic_clock12_digitsH01 = new pictureBox("clock12_digitsH01");       // digits hour   * 01
    pictureBox* pic_clock12_digitsM10 = new pictureBox("clock12_digitsM10");       // digits minute * 10
    pictureBox* pic_clock12_digitsM01 = new pictureBox("clock12_digitsM01");       // digits minute * 01
    pictureBox* pic_clock12_digitsColon = new pictureBox("clock12_digitsColon");   // digits colon
    pictureBox* pic_clock12_digits_AM_PM = new pictureBox("clock12_digits_AM_PM"); // digits AM/PM
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint16_t    m_digitsYPos = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 0;
        uint16_t w = 56;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     56 x 120 px
    struct w_h01 {
        uint16_t x = 56;
        uint16_t w = 56;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     56 x 120 px
    struct w_c {
        uint16_t x = 112;
        uint16_t w = 40;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         40 x 120 px
    struct w_m10 {
        uint16_t x = 152;
        uint16_t w = 56;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   56 x 120 px
    struct w_m01 {
        uint16_t x = 208;
        uint16_t w = 56;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   56 x 120 px
    struct w_ap {
        uint16_t x = 264;
        uint16_t w = 56;
        uint16_t h = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_ap; // AM_PM         56 x 120 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 16;
        uint16_t w = 80;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     80 x 160 px
    struct w_h01 {
        uint16_t x = 96;
        uint16_t w = 80;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     80 x 160 px
    struct w_c {
        uint16_t x = 176;
        uint16_t w = 48;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         48 x 160 px
    struct w_m10 {
        uint16_t x = 224;
        uint16_t w = 80;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   80 x 160 px
    struct w_m01 {
        uint16_t x = 304;
        uint16_t w = 80;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   80 x 160 px
    struct w_ap {
        uint16_t x = 384;
        uint16_t w = 80;
        uint16_t h = 160;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_ap; // AM_PM         80 x 160 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#else
    uint16_t m_digitsYoffset = 30;
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 36;
        uint16_t w = 130;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10    130 x 260 px
    struct w_h01 {
        uint16_t x = 166;
        uint16_t w = 130;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01    130 x 260 px
    struct w_c {
        uint16_t x = 296;
        uint16_t w = 78;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         78 x 260 px
    struct w_m10 {
        uint16_t x = 374;
        uint16_t w = 130;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10  130 x 260 px
    struct w_m01 {
        uint16_t x = 504;
        uint16_t w = 130;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01  130 x 260 px
    struct w_ap {
        uint16_t x = 634;
        uint16_t w = 130;
        uint16_t h = 260;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_ap; // AM_PM        130 x 260 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    bool        m_showAll = false;
    bool        m_backgroundTransparency = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    uint8_t     m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg m_ra;

  public:
    imgClock12(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("imgClock12");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock12() {
        x_ps_free(&m_name);
        x_ps_free(&m_pathBuff);
        delete pic_clock12_digitsH10;
        delete pic_clock12_digitsH01;
        delete pic_clock12_digitsColon;
        delete pic_clock12_digitsM10;
        delete pic_clock12_digitsM01;
        delete pic_clock12_digits_AM_PM;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_digitsYPos = m_y + m_digitsYoffset;
        pic_clock12_digitsH10->begin(s_h10.x, m_digitsYPos, s_h10.w, s_h10.h, s_h10.pl, s_h10.pr, s_h10.pt, s_h10.pb);
        pic_clock12_digitsH01->begin(s_h01.x, m_digitsYPos, s_h01.w, s_h01.h, s_h01.pl, s_h01.pr, s_h01.pt, s_h01.pb);
        pic_clock12_digitsColon->begin(s_c.x, m_digitsYPos, s_c.w, s_c.h, s_c.pl, s_c.pr, s_c.pt, s_c.pb);
        pic_clock12_digitsM10->begin(s_m10.x, m_digitsYPos, s_m10.w, s_m10.h, s_m10.pl, s_m10.pr, s_m10.pt, s_m10.pb);
        pic_clock12_digitsM01->begin(s_m01.x, m_digitsYPos, s_m01.w, s_m10.h, s_m01.pl, s_m01.pr, s_m01.pt, s_m01.pb);
        pic_clock12_digits_AM_PM->begin(s_ap.x, m_digitsYPos, s_ap.w, s_ap.h, s_ap.pl, s_ap.pr, s_ap.pt, s_ap.pb);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() {
        m_enabled = false;
        m_showAll = false;
    }
    bool isDisabled() { return !m_enabled; }
    bool enable() { return m_enabled = true; }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday) {
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_hour = minuteOfTheDay / 60;
        m_min = minuteOfTheDay % 60;
        m_weekday = weekday;
        if (m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t m_min) {
        static uint8_t oldTime[4];
        static bool    k = false;
        uint8_t        time[5];
        time[0] = m_hour / 10;
        time[1] = m_hour % 10;
        time[2] = m_min / 10;
        time[3] = m_min % 10;

        bool        isPM = true;
        static bool isOldPM = false;
        for (uint8_t i = 0; i < 4; i++) {
            uint8_t hour = m_hour;
            if (hour > 0 && hour < 13)
                isPM = false;
            else
                (hour -= 12);
            time[0] = hour / 10;
            time[1] = hour % 10;
            if ((time[i] != oldTime[i]) || m_showAll) {
                sprintf(m_pathBuff, "/digits/foldedNumbers/%iwhite.jpg", time[i]);
                if (i == 0) {
                    pic_clock12_digitsH10->setPicturePath(m_pathBuff);
                    pic_clock12_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_clock12_digitsH01->setPicturePath(m_pathBuff);
                    pic_clock12_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_clock12_digitsM10->setPicturePath(m_pathBuff);
                    pic_clock12_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_clock12_digitsM01->setPicturePath(m_pathBuff);
                    pic_clock12_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            oldTime[i] = time[i];
        }
        if ((isPM != isOldPM) || m_showAll) {
            if (isPM) {
                pic_clock12_digits_AM_PM->setPicturePath("/digits/foldedNumbers/pmwhite.jpg");
                pic_clock12_digits_AM_PM->show(m_backgroundTransparency, false);
            } else {
                pic_clock12_digits_AM_PM->setPicturePath("/digits/foldedNumbers/amwhite.jpg");
                pic_clock12_digits_AM_PM->show(m_backgroundTransparency, false);
            }
            isOldPM = isPM;
        }

        k = !k;
        if (k) {
            pic_clock12_digitsColon->setPicturePath("/digits/foldedNumbers/dwhite.jpg");
            pic_clock12_digitsColon->show(m_backgroundTransparency, false);
        } else {
            pic_clock12_digitsColon->setPicturePath("/digits/foldedNumbers/ewhite.jpg");
            pic_clock12_digitsColon->show(m_backgroundTransparency, false);
        }

        m_showAll = false;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_clicked = false;
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class alarmClock : public RegisterTable { // draw a clock in 12 or 24h format
  private:
    pictureBox* pic_alarm_digitsH10 = new pictureBox("alarm_digitsH10");     // digits hour   * 10
    pictureBox* pic_alarm_digitsH01 = new pictureBox("alarm_digitsH01");     // digits hour   * 01
    pictureBox* pic_alarm_digitsM10 = new pictureBox("alarm_digitsM10");     // digits minute * 10
    pictureBox* pic_alarm_digitsM01 = new pictureBox("alarm_digitsM01");     // digits minute * 01
    pictureBox* pic_alarm_digitsColon = new pictureBox("alarm_digitsColon"); // digits colon
    textbox*    txt_alarm_days = new textbox[7]{textbox("txt_alarm_days0"), textbox("txt_alarm_days1"), textbox("txt_alarm_days2"), textbox("txt_alarm_days3"),
                                                textbox("txt_alarm_days4"), textbox("txt_alarm_days5"), textbox("txt_alarm_days6")}; // days of the week
    textbox*    txt_alarm_time = new textbox[7]{textbox("txt_alarm_time0"), textbox("txt_alarm_time1"), textbox("txt_alarm_time2"), textbox("txt_alarm_time3"),
                                                textbox("txt_alarm_time4"), textbox("txt_alarm_time5"), textbox("txt_alarm_time6")}; // time of the day

    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    uint16_t m_alarmdaysYPos = 0;
    uint16_t m_alarmtimeYPos = 0;
    uint16_t m_digitsYPos = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_alarmdaysXPos[7] = {2, 47, 92, 137, 182, 227, 272}; // same as altarmTimeXPos
    uint8_t  m_alarmdaysYoffset = 2;
    uint8_t  m_alarmdaysW = 44;
    uint8_t  m_alarmdaysH = 25;
    uint8_t  m_fontSize = 0; // auto
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 4;
        uint16_t w = 72;
        uint16_t h = 104;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10    72 x 104 px
    struct w_h01 {
        uint16_t x = 76;
        uint16_t w = 72;
        uint16_t h = 104;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01    72 x 104 px
    struct w_c {
        uint16_t x = 148;
        uint16_t w = 72;
        uint16_t h = 104;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon        24 x 104 px
    struct w_m10 {
        uint16_t x = 172;
        uint16_t w = 72;
        uint16_t h = 104;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10  72 x 104 px
    struct w_m01 {
        uint16_t x = 244;
        uint16_t w = 72;
        uint16_t h = 104;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01  72 x 104 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7
    uint16_t m_alarmdaysXPos[7] = {9, 75, 141, 207, 273, 339, 405};
    uint8_t  m_alarmdaysYoffset = 2;
    uint8_t  m_alarmdaysW = 65;
    uint8_t  m_alarmdaysH = 23;
    uint8_t  m_fontSize = 0; // auto
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 32;
        uint16_t w = 96;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10     96 x 150 px
    struct w_h01 {
        uint16_t x = 128;
        uint16_t w = 96;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01     96 x 150 px
    struct w_c {
        uint16_t x = 224;
        uint16_t w = 32;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         32 x 150 px
    struct w_m10 {
        uint16_t x = 255;
        uint16_t w = 96;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10   96 x 150 px
    struct w_m01 {
        uint16_t x = 352;
        uint16_t w = 96;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01   96 x 150 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#else // 800 x 480px
    uint16_t m_alarmdaysXPos[7] = {50, 150, 250, 350, 450, 550, 650};
    uint8_t  m_alarmdaysYoffset = 10;
    uint8_t  m_alarmdaysW = 100;
    uint8_t  m_alarmdaysH = 32;
    uint8_t  m_fontSize = 0; // auto
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_h10 {
        uint16_t x = 112;
        uint16_t w = 132;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h10; // Hour * 10    132 x 220 px
    struct w_h01 {
        uint16_t x = 244;
        uint16_t w = 132;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_h01; // Hour * 01    132 x 220 px
    struct w_c {
        uint16_t x = 376;
        uint16_t w = 80;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_c; // Colon         47 x 220 px
    struct w_m10 {
        uint16_t x = 423;
        uint16_t w = 132;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m10; // Minute * 10  132 x 220 px
    struct w_m01 {
        uint16_t x = 555;
        uint16_t w = 132;
        uint16_t h = 220;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_m01; // Minute * 01  132 x 220 px
    //-------------------------------------------------------------------------------------------------------------------------------------------------------
#endif

    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    bool        m_showAll = false;
    bool        m_backgroundTransparency = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    uint8_t*    m_alarmDays = NULL;
    int16_t*    m_alarmTime = NULL;
    uint8_t     m_min = 0, m_hour = 0, m_weekday = 0;
    int8_t      m_btnAlarmDay = -1;
    int8_t      m_btnAlarmTime = -1;
    int8_t      m_idx = 0;
    uint8_t     m_alarmDigits[4] = {0};
    const char* m_p1 = "/digits/sevenSegment/"; // path
    uint8_t     m_p1Len = 21;
    const char  m_WD[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    releasedArg m_ra;

  public:
    alarmClock(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("alarmClock");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        strcpy(m_pathBuff, m_p1);
    }
    ~alarmClock() {
        x_ps_free(&m_name);
        x_ps_free(&m_pathBuff);
        delete pic_alarm_digitsH10;
        delete pic_alarm_digitsH01;
        delete pic_alarm_digitsColon;
        delete pic_alarm_digitsM10;
        delete pic_alarm_digitsM01;
        delete[] txt_alarm_days;
        delete[] txt_alarm_time;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_alarmdaysYPos = m_y + m_alarmdaysYoffset; // m_y;
        m_alarmtimeYPos = m_alarmdaysYPos + m_alarmdaysH + 1;
        m_digitsYPos = m_alarmtimeYPos + m_alarmdaysH + 1;
        pic_alarm_digitsH10->begin(s_h10.x, m_digitsYPos, s_h10.w, s_h10.h, s_h10.pl, s_h10.pr, s_h10.pt, s_h10.pb);
        pic_alarm_digitsH01->begin(s_h01.x, m_digitsYPos, s_h01.w, s_h01.h, s_h01.pl, s_h01.pr, s_h01.pt, s_h01.pb);
        pic_alarm_digitsColon->begin(s_c.x, m_digitsYPos, s_c.w, s_c.h, s_c.pl, s_c.pr, s_c.pt, s_c.pb);
        pic_alarm_digitsM10->begin(s_m10.x, m_digitsYPos, s_m10.w, s_m10.h, s_m10.pl, s_m10.pr, s_m10.pt, s_m10.pb);
        pic_alarm_digitsM01->begin(s_m01.x, m_digitsYPos, s_m01.w, s_m10.h, s_m01.pl, s_m01.pr, s_m01.pt, s_m01.pb);
        for (uint8_t i = 0; i < 7; i++) {
            txt_alarm_days[i].begin(m_alarmdaysXPos[i], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, 0, 0, 0, 0);
            txt_alarm_days[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_alarm_days[i].setBorderWidth(1);
            txt_alarm_days[i].setFont(m_fontSize);
            txt_alarm_days[i].setText(m_WD[i]);
            txt_alarm_time[i].begin(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, 0, 0, 0, 0);
            txt_alarm_time[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_alarm_time[i].setBorderWidth(1);
            txt_alarm_time[i].setFont(m_fontSize);
        }
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        updateDigits();
        updateAlarmDaysAndTime();
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    void shiftRight() {
        m_idx++;
        if (m_idx == 4) m_idx = 0;
        m_showAll = true;
        updateDigits();
    }
    void shiftLeft() {
        m_idx--;
        if (m_idx == -1) m_idx = 0;
        m_showAll = true;
        updateDigits();
    }
    void digitUp() {
        if (m_idx == 0) { // 10h
            if (m_alarmDigits[0] == 2) return;
            if (m_alarmDigits[0] == 1 && m_alarmDigits[1] > 3) return;
            m_alarmDigits[0]++;
        }
        if (m_idx == 1) { // 1h
            if (m_alarmDigits[0] == 2 && m_alarmDigits[1] == 3) return;
            if (m_alarmDigits[1] == 9) return;
            m_alarmDigits[1]++;
        }
        if (m_idx == 2) { // 10m
            if (m_alarmDigits[2] == 5) return;
            m_alarmDigits[2]++;
        }
        if (m_idx == 3) { // 1m
            if (m_alarmDigits[3] == 9) return;
            m_alarmDigits[3]++;
        }
        m_showAll = true;
        updateDigits();
    }
    void digitDown() {
        if (m_idx == 0) { // 10h
            if (m_alarmDigits[0] == 0) return;
            m_alarmDigits[0]--;
        }
        if (m_idx == 1) { // 1h
            if (m_alarmDigits[1] == 0) return;
            m_alarmDigits[1]--;
        }
        if (m_idx == 2) { // 10m
            if (m_alarmDigits[2] == 0) return;
            m_alarmDigits[2]--;
        }
        if (m_idx == 3) { // 1m
            if (m_alarmDigits[3] == 0) return;
            m_alarmDigits[3]--;
        }
        m_showAll = true;
        updateDigits();
    }
    void setAlarmTimeAndDays(uint8_t* alarmDays, int16_t alarmTime[7]) {
        m_alarmTime = alarmTime;
        m_alarmDays = alarmDays;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        for (int i = 0; i < 7; i++) {
            if (txt_alarm_days[i].positionXY(x, y)) m_btnAlarmDay = i;
            if (txt_alarm_time[i].positionXY(x, y)) m_btnAlarmTime = i;
        }
        if (m_btnAlarmDay >= 0) alarmDaysPressed(m_btnAlarmDay);
        if (m_btnAlarmTime >= 0) alarmTimePressed(m_btnAlarmTime);
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        char hhmm[10] = "00:00";
        if (m_btnAlarmDay >= 0) { sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmDay] / 60, m_alarmTime[m_btnAlarmDay] % 60); }
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);

        if (m_btnAlarmDay >= 0) {
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmDay;
            *m_alarmDays ^= mask;      // toggle the bit
            if (*m_alarmDays & mask) { // is set
                txt_alarm_days[m_btnAlarmDay].setBorderColor(TFT_RED);
                txt_alarm_days[m_btnAlarmDay].setTextColor(TFT_RED);
                txt_alarm_days[m_btnAlarmDay].setText(m_WD[m_btnAlarmDay]);
                txt_alarm_days[m_btnAlarmDay].show(m_backgroundTransparency, false);
                txt_alarm_time[m_btnAlarmDay].setBorderColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmDay].setTextColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmDay].setText(hhmm);
                txt_alarm_time[m_btnAlarmDay].show(m_backgroundTransparency, false);
            } else { // bit is not set
                txt_alarm_days[m_btnAlarmDay].setBorderColor(TFT_DARKGREY);
                txt_alarm_days[m_btnAlarmDay].setTextColor(TFT_DARKGREY);
                txt_alarm_days[m_btnAlarmDay].setText(m_WD[m_btnAlarmDay]);
                txt_alarm_days[m_btnAlarmDay].show(m_backgroundTransparency, false);
                txt_alarm_time[m_btnAlarmDay].setBorderColor(TFT_DARKGREY);
                txt_alarm_time[m_btnAlarmDay].setTextColor(TFT_DARKGREY);
                txt_alarm_time[m_btnAlarmDay].setText("");
                txt_alarm_time[m_btnAlarmDay].show(m_backgroundTransparency, false);
            }
            m_btnAlarmDay = -1;
        }
        if (m_btnAlarmTime >= 0) {
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmTime;
            if (mask & *m_alarmDays) { // bit is set -> alarm is active for that day
                txt_alarm_time[m_btnAlarmTime].setBorderColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmTime].setTextColor(TFT_GREEN);
                m_alarmTime[m_btnAlarmTime] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60 + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
                char hhmm[10] = "00:00";
                sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmTime] / 60, m_alarmTime[m_btnAlarmTime] % 60);
                txt_alarm_time[m_btnAlarmTime].setText(hhmm);
                txt_alarm_time[m_btnAlarmTime].show(m_backgroundTransparency, false);
            }
            m_btnAlarmTime = -1;
        }
        m_clicked = false;
        return true;
    }

  private:
    void updateDigits() {
        static uint8_t m_oldAlarmDigits[4] = {0};
        for (uint8_t i = 0; i < 4; i++) {
            if (m_oldAlarmDigits[i] != m_alarmDigits[i] || m_showAll) {
                m_pathBuff[m_p1Len + 0] = m_alarmDigits[i] + 48;
                m_pathBuff[m_p1Len + 1] = '\0';

                if (i == m_idx) {
                    strcat(m_pathBuff, "orange.jpg");
                } else {
                    strcat(m_pathBuff, "red.jpg");
                }

                if (i == 0) {
                    pic_alarm_digitsH10->setPicturePath(m_pathBuff);
                    pic_alarm_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_alarm_digitsH01->setPicturePath(m_pathBuff);
                    pic_alarm_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_alarm_digitsM10->setPicturePath(m_pathBuff);
                    pic_alarm_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_alarm_digitsM01->setPicturePath(m_pathBuff);
                    pic_alarm_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            m_oldAlarmDigits[i] = m_alarmDigits[i];
        }
        if (m_showAll) {
            pic_alarm_digitsColon->setPicturePath("/digits/sevenSegment/dred.jpg");
            pic_alarm_digitsColon->show(m_backgroundTransparency, false);
        }
    }
    void updateAlarmDaysAndTime() {
        uint8_t  mask = 0b00000001;
        uint16_t color = TFT_BLACK;

        for (int i = 0; i < 7; i++) {
            // alarmDays
            if (*m_alarmDays & mask)
                color = TFT_RED;
            else
                color = TFT_DARKGREY;
            txt_alarm_days[i].setBorderColor(color);
            txt_alarm_days[i].setTextColor(color);
            txt_alarm_days[i].setText(m_WD[i]);
            txt_alarm_days[i].show(m_backgroundTransparency, false);
            char hhmm[10] = "00:00";
            sprintf(hhmm, "%02d:%02d", m_alarmTime[i] / 60, m_alarmTime[i] % 60);
            if (*m_alarmDays & mask) {
                txt_alarm_time[i].setBorderColor(TFT_GREEN);
                txt_alarm_time[i].setTextColor(TFT_GREEN);
                txt_alarm_time[i].setText(hhmm);
            } else {
                txt_alarm_time[i].setBorderColor(TFT_DARKGREY);
                txt_alarm_time[i].setTextColor(TFT_DARKGREY);
                txt_alarm_time[i].setText("");
            }
            txt_alarm_time[i].show(m_backgroundTransparency, false);
            mask <<= 1;
        }
    }

    void alarmDaysPressed(uint8_t idx) {
        txt_alarm_days[idx].setBorderColor(TFT_YELLOW);
        txt_alarm_days[idx].setTextColor(TFT_YELLOW);
        txt_alarm_days[idx].setText(m_WD[idx]);
        txt_alarm_days[idx].show(m_backgroundTransparency, false);
    }
    void alarmTimePressed(uint8_t idx) {
        uint8_t mask = 0b00000001;
        mask <<= idx;
        if (mask & *m_alarmDays) { // bit is set -> active
            m_alarmTime[idx] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60 + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
            char hhmm[10] = {0};
            sprintf(hhmm, "%02d:%02d", m_alarmTime[idx] / 60, m_alarmTime[idx] % 60);
            txt_alarm_time[idx].setBorderColor(TFT_YELLOW);
            txt_alarm_time[idx].setTextColor(TFT_YELLOW);
            txt_alarm_time[idx].setText(hhmm);
            tft.setTextColor(TFT_YELLOW);
            txt_alarm_time[idx].show(m_backgroundTransparency, false);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class uniList {

  private:
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    int32_t  m_nr[10] = {0};
    uint8_t  m_fontSize = 0;
    uint8_t  m_tftSize = 0;
    uint8_t  m_lineHight = 0;
    uint8_t  m_mode = 0;
    uint32_t m_bgColor = 0;
    uint8_t  m_indentContent = 0;
    uint8_t  m_indentDirectory = 0;
    char*    m_name = NULL;
    char*    m_buff = NULL;
    char*    m_txt[10] = {0};
    char*    m_ext1[10] = {0};
    char*    m_ext2[10] = {0};
    bool     m_enabled = false;

  public:
    uniList(const char* name) {
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("dlnaList");
        m_bgColor = TFT_BLACK;
    }
    ~uniList() {
        x_ps_free(&m_name);
        x_ps_free(&m_buff);
        for (int i = 0; i < 10; i++) {
            x_ps_free(&m_txt[i]);
            x_ps_free(&m_ext1[i]);
            x_ps_free(&m_ext2[i]);
        }
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fontSize) {
        m_x = x;               // x pos
        m_y = y;               // y pos
        m_w = w;               // width
        m_h = h;               // high
        m_fontSize = fontSize; // default font size
        m_enabled = false;
        m_lineHight = m_h / 10;
        m_buff = x_ps_malloc(1024);
    }
    void setMode(uint8_t mode, const char* tftSize, uint8_t fontSize) {
        if (mode == RADIO) { m_mode = RADIO; }
        if (mode == PLAYER) { m_mode = PLAYER; }
        if (mode == DLNA) { m_mode = DLNA; }
        m_fontSize = fontSize;
        if (strcmp(tftSize, "s") == 0) m_tftSize = 1;
        if (strcmp(tftSize, "m") == 0) m_tftSize = 2;
        if (strcmp(tftSize, "l") == 0) m_tftSize = 3;
        switch (m_tftSize) {
            case 1:
                if (m_mode == RADIO) {
                    m_indentDirectory = 15;
                    m_indentContent = 15;
                } // 320x240
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 15;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 15;
                }
                break;
            case 2:
                if (m_mode == RADIO) {
                    m_indentDirectory = 20;
                    m_indentContent = 20;
                } // 480x320
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 20;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 20;
                }
                break;
            case 3:
                if (m_mode == RADIO) {
                    m_indentDirectory = 30;
                    m_indentContent = 30;
                } // 800x480
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                break;
        }
    }
    void clearList() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        for (int i = 0; i < 10; i++) {
            x_ps_free(&m_txt[i]);
            x_ps_free(&m_ext1[i]);
            x_ps_free(&m_ext2[i]);
            m_nr[i] = -1;
        }
    }
    void drawLine(uint8_t pos, const char* txt, const char* ext1 = NULL, const char* ext2 = NULL, const char* color = ANSI_ESC_WHITE, int32_t nr = -1) {
        if (pos > 9) return;
        if (!txt) return;
        tft.setFont(m_fontSize);
        if (m_mode == RADIO) {
            sprintf(m_buff, ANSI_ESC_YELLOW "%03li %s%s", nr, color, txt);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
            }
            if (ext1) {
                x_ps_free(&m_ext1[pos]);
                m_ext1[pos] = strdup(ext1);
            }
            if (ext2) {
                x_ps_free(&m_ext2[pos]);
                m_ext2[pos] = strdup(ext2);
            }
            m_nr[pos] = nr;
        }
        if (m_mode == DLNA) {
            if (!txt) {
                log_e("txt is NULL");
                return;
            }
            if (!ext1)
                sprintf(m_buff, "%s%s", color, txt);
            else if (ext1[0] == '\0')
                sprintf(m_buff, "%s%s", color, txt);
            else
                sprintf(m_buff, "%s%s " ANSI_ESC_CYAN "(%s)", color, txt, ext1);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
                m_nr[pos] = 1;
            }
            if (ext1) {
                x_ps_free(&m_ext1[pos]);
                m_ext1[pos] = strdup(ext1);
            }
            if (ext2) {
                x_ps_free(&m_ext2[pos]);
                m_ext2[pos] = strdup(ext2);
            }
        }
        if (m_mode == PLAYER) {
            if (!txt) {
                log_e("txt is NULL");
                return;
            }
            if (!nr)
                sprintf(m_buff, "%s%s", color, txt);
            else
                sprintf(m_buff, "%s%s" ANSI_ESC_YELLOW " %li", color, txt, nr);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
                m_nr[pos] = nr;
            }
        }
        tft.writeText(m_buff, pos ? m_indentContent : m_indentDirectory, m_y + pos * m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
    }
    void drawPosInfo(int16_t firstVal, int16_t secondVal, int16_t total, const char* color) { // e.g. 1-9/65
        sprintf(m_buff, "%s%i-%i/%i", color, firstVal, secondVal, total);
        tft.writeText(m_buff, 0, m_y, m_w, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true);
    }
    void colourLine(uint8_t pos, const char* color = ANSI_ESC_WHITE) {
        if (pos > 9) return;
        tft.setFont(m_fontSize);
        if (m_mode == RADIO) { sprintf(m_buff, ANSI_ESC_YELLOW "%03li %s%s", m_nr[pos], color, m_txt[pos]); }
        if (m_mode == PLAYER) {
            if (m_nr[pos])
                sprintf(m_buff, "%s%s" ANSI_ESC_YELLOW " %li", color, m_txt[pos], m_nr[pos]); // file
            else
                sprintf(m_buff, "%s%s", color, m_txt[pos]); // directory
        }
        tft.writeText(m_buff, pos ? m_indentContent : m_indentDirectory, m_y + pos * m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
    }
    const char* getTxtByPos(uint8_t pos) { return m_txt[pos]; }
    int16_t     getNumberByPos(uint8_t pos) { return m_nr[pos]; }
    void        drawTriangeUp() {
        auto triangleUp = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + s, y + 0, x + 0, y + 2 * s, x + 2 * s, y + 2 * s, TFT_RED); };
        int  line = 1;
        if (m_mode == RADIO) line = 0;
        triangleUp(0, m_y + (line * m_lineHight), m_lineHight / 3.5);
    }
    void drawTriangeDown() {
        auto triangleDown = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + 0, y + 0, x + 2 * s, y + 0, x + s, y + 2 * s, TFT_RED); };
        triangleDown(0, m_y + (9 * m_lineHight), m_lineHight / 3.5);
    }
};
extern uniList myList;
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
  ———————————————————————————————————————————————————————
  | DLNA List                       Vol3    01:10:32    |           m_itemsListPos
  | Musik                                      1-9/6    |           <-- 0
  |   Videos(7)                                         |           <-- 1
  |   Interpreten(2)                                    |           <-- 2
  |   Alben                                             |           <-- 3
  |   Alle Titel(7)                                     |           <-- 4
  |   Genres                                            |           <-- 5
  |   Ordner(1)                                         |           <-- 6
  |   Wiedergabelisten                                  |           <-- 7
  |   Filme(23)                                         |           <-- 8
  |                                                     |           <-- 9
  | 003   0:00    128K              IP:192.168.178.24   |
  ———————————————————————————————————————————————————————
*/

class dlnaList : public RegisterTable {

  private:
    int16_t                                    m_x = 0;
    int16_t                                    m_y = 0;
    int16_t                                    m_w = 0;
    int16_t                                    m_h = 0;
    int16_t                                    m_oldX = 0;
    int16_t                                    m_oldY = 0;
    uint8_t*                                   m_dlnaLevel;
    uint8_t                                    m_fontSize = 0;
    uint8_t                                    m_lineHight = 0;
    uint8_t                                    m_browseOnRelease = 0;
    uint8_t                                    m_itemListPos = 0;
    int8_t                                     m_currDLNAsrvNr = -1;
    int16_t                                    m_currItemNr[10] = {0};
    int16_t                                    m_viewPoint = 0;
    uint16_t                                   m_dlnaMaxItems = 0;
    uint32_t                                   m_bgColor = 0;
    bool                                       m_enabled = false;
    bool                                       m_clicked = false;
    bool                                       m_state = false;
    bool                                       m_isAudio = false;
    bool                                       m_isURL = false;
    char*                                      m_name = NULL;
    char*                                      m_pathBuff = NULL;
    const char*                                m_chptr = NULL;
    char*                                      m_buff = NULL;
    const char*                                m_tftSize = NULL;
    const std::deque<DLNA_Client::dlnaServer>* m_dlnaServer;
    const std::deque<DLNA_Client ::srvItem>*   m_srvContent;
    DLNA_Client*                               m_dlna;
    dlnaHistory*                               m_dlnaHistory = NULL;
    releasedArg                                m_ra;

  public:
    dlnaList(const char* name, DLNA_Client* dlna, dlnaHistory* dh, uint8_t dhSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("dlnaList");
        m_buff = x_ps_malloc(512);
        m_dlna = dlna;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        m_ra.arg1 = NULL;
        m_ra.arg2 = NULL;
        m_ra.val1 = 0;
        m_ra.val2 = 0;
        m_dlnaHistory = dh;
        for (uint8_t i = 0; i < 10; i++) m_currItemNr[i] = 0;
    }
    ~dlnaList() {
        x_ps_free(&m_name);
        x_ps_free(&m_buff);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_fontSize = fontSize;
        m_enabled = false;
        m_lineHight = m_h / 10;
        m_tftSize = tftSize;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(int8_t number, const std::deque<DLNA_Client::dlnaServer>& dlnaServer, const std::deque<DLNA_Client::srvItem>& srvContent, uint8_t* dlnaLevel, uint16_t maxItems) {
        m_browseOnRelease = 0;
        m_dlnaServer = &dlnaServer;
        m_srvContent = &srvContent;
        m_dlnaLevel = dlnaLevel;
        if (m_dlnaLevel == 0) m_currDLNAsrvNr = number;
        m_clicked = false;
        m_enabled = true;
        m_dlnaMaxItems = maxItems;
        m_dlnaHistory[0].maxItems = m_dlna->getNrOfServers();
        dlnaItemsList();
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    bool positionXY(uint16_t x, uint16_t y) { // called every tine if x or y has changed
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_clicked == false) {
            m_oldX = x;
            m_oldY = y;
        }
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        hasReleased(x - m_x, y - m_y);
        m_clicked = false;

        if (m_chptr || (m_itemListPos == 0 && (*m_dlnaLevel) > 0)) {
            drawItem(m_itemListPos, true);
            m_chptr = NULL;
            vTaskDelay(300);
        }

        if (m_browseOnRelease == 0) { ; } // file
        if (m_browseOnRelease == 1) {
            (*m_dlnaLevel)++;
            m_dlna->browseServer(m_currDLNAsrvNr, "0", 0, 9);
        } // get serverlist
        if (m_browseOnRelease == 2) {
            (*m_dlnaLevel)--;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, 0, 9);
        } // previous level
        if (m_browseOnRelease == 3) {
            (*m_dlnaLevel)++;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, 0, 9);
        } // folder, next level
        if (m_browseOnRelease == 4) { m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9); } // scroll up / down

        m_browseOnRelease = 0;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_ra.val1 = 0;
        return true;
    }

  private:
    void dlnaItemsList() {
        uint8_t pos = 0;
        myList.setMode(DLNA, m_tftSize, m_fontSize);
        myList.clearList();
        myList.drawLine(0, m_dlnaHistory[*m_dlnaLevel].name, NULL, ANSI_ESC_ORANGE);
        tft.setTextColor(TFT_WHITE);
        for (pos = 1; pos < 10; pos++) {
            if (pos == 1 && m_viewPoint > 0) { myList.drawTriangeUp(); }
            if (pos == 9 && m_viewPoint + 9 < m_dlnaMaxItems - 1) { myList.drawTriangeDown(); }
            if (*m_dlnaLevel == 0 && pos > m_dlnaServer->size()) { /* log_e("pos too high %i", pos);*/
                break;
            } // guard
            if (*m_dlnaLevel > 0 && pos > m_srvContent->size()) { /* log_e("pos too high %i", pos);*/
                break;
            } // guard
            drawItem(pos);
        }
        sprintf(m_buff, "%i-%i/%i", m_viewPoint + 1, m_viewPoint + (pos - 1), m_dlnaMaxItems); // shows the current items pos e.g. "30-39/210"
        tft.setTextColor(TFT_ORANGE);
        tft.writeText(m_buff, 10, m_y, m_w - 10, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true);

        return;
    }

    void drawItem(int8_t pos, bool selectedLine = false) { // pos 0 is parent, pos 1...9 are itens, selectedLine means released (ok)
        if (pos < 0 || pos > 9) {
            log_e("pos oor %i", pos);
            return;
        } // guard
        if (*m_dlnaLevel == 0 && pos > m_dlnaServer->size()) { /* log_e("pos too high %i", pos);*/
            return;
        } // guard
        if (*m_dlnaLevel > 0 && pos > m_srvContent->size()) { /* log_e("pos too high %i", pos);*/
            return;
        } // guard
        char        extension[15] = {0};
        char        dummy[] = "";
        bool        isAudio = false;
        bool        isURL = false;
        const char *item = dummy, *duration = dummy, *itemURL = dummy, *color = ANSI_ESC_WHITE;
        (void)itemURL;
        int32_t itemSize = 0;
        int16_t childCount = 0;
        if (pos == 0) {
            if (pos + m_viewPoint == m_currItemNr[*m_dlnaLevel] + 1)
                color = ANSI_ESC_MAGENTA;
            else
                color = ANSI_ESC_ORANGE;
            if (selectedLine) color = ANSI_ESC_CYAN;
            myList.drawLine(pos, m_dlnaHistory[*m_dlnaLevel].name, "", "", color, 1);
            return;
        }
        if (*m_dlnaLevel == 0) {
            if (m_dlnaServer->at(pos - 1).friendlyName.c_get()) item = m_dlnaServer->at(pos - 1).friendlyName.c_get();
        } else {
            if (m_srvContent->at(pos - 1).title.c_get()) item = m_srvContent->at(pos - 1).title.c_get();
            itemSize = m_srvContent->at(pos - 1).itemSize;
            childCount = m_srvContent->at(pos - 1).childCount;
            if (m_srvContent->at(pos - 1).duration.c_get()) duration = m_srvContent->at(pos - 1).duration.c_get();
            isAudio = m_srvContent->at(pos - 1).isAudio;
            if (startsWith(m_srvContent->at(pos - 1).itemURL.c_get(), "http")) {
                isURL = true;
                itemURL = m_srvContent->at(pos - 1).itemURL.c_get();
            }
        }

        if ((pos - 1) + m_viewPoint == m_currItemNr[*m_dlnaLevel]) {
            color = ANSI_ESC_MAGENTA;
        } else if (isURL && isAudio) {
            color = ANSI_ESC_YELLOW;
        } else {
            color = ANSI_ESC_WHITE;
        }
        if (selectedLine) { color = ANSI_ESC_CYAN; }

        if (childCount) { sprintf(extension, "%i", childCount); }
        if (itemSize) { sprintf(extension, "%li", itemSize); }
        if (duration[0] != '?') {
            if (strcmp(duration, "0:00:00") != 0) sprintf(extension, "%s", duration);
        }
        myList.drawLine(pos, item, extension, itemURL, color, 1);
    }

    void hasReleased(uint16_t x, uint16_t y) {
        m_itemListPos = y / (m_h / 10);
        bool     guard1 = false;
        bool     guard2 = false;
        uint16_t itemURLsize = 0;
        uint16_t titleSize = 0;
        for (int i = 0; i < m_srvContent->size(); i++) {
            if (strcmp(m_srvContent->at(i).itemURL.c_get(), "?") != 0) itemURLsize++;
            if (strcmp(m_srvContent->at(i).title.c_get(), "?") != 0) titleSize++;
        }
        if (itemURLsize > (m_itemListPos - 1)) guard2 = true;
        bool guard3 = false;
        if (titleSize > (m_itemListPos - 1)) guard3 = true;
        uint16_t friendlyName = 0;
        for (int i = 0; i < m_dlnaServer->size(); i++) {
            if (strcmp(m_dlnaServer->at(i).friendlyName.c_get(), "?") != 0) friendlyName++;
        }
        if (friendlyName > (m_itemListPos - 1)) guard1 = true;

        if (m_oldY && (m_oldY + 7 * m_lineHight < y)) { // fast wipe down
            m_ra.val1 = 0;
            if (m_viewPoint == 0) goto exit;
            if (m_viewPoint > 36)
                m_viewPoint -= 36;
            else
                m_viewPoint = 0;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY + 1 * m_lineHight < y)) { // normal wipe down
            m_ra.val1 = 0;
            if (m_viewPoint == 0) goto exit;
            if (m_viewPoint > 9)
                m_viewPoint -= 9;
            else
                m_viewPoint = 0;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY - 8 * m_lineHight > y)) { // fast wipe up
            m_ra.val1 = 0;
            if (m_viewPoint + 9 >= m_dlnaMaxItems - 1) goto exit;
            int16_t diff = (m_dlnaMaxItems - 1) - (m_viewPoint + 9);
            if (diff >= 36)
                m_viewPoint += 36;
            else
                m_viewPoint += diff;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // normal wipe up
            m_ra.val1 = 0;
            if (m_viewPoint + 9 >= m_dlnaMaxItems - 1) goto exit;
            m_viewPoint += 9;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if (m_itemListPos == 0) { // previous level, content list
            if (*m_dlnaLevel == 0) { goto exit; }
            m_viewPoint = 0;
            m_browseOnRelease = 2;
            goto exit;
        }

        if (guard1) { // server list
            if (*m_dlnaLevel == 0) {
                m_chptr = m_dlnaServer->at(m_itemListPos - 1).friendlyName.c_get();
                m_currDLNAsrvNr = m_itemListPos - 1;
                m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
                x_ps_free(&m_dlnaHistory[(*m_dlnaLevel) + 1].name);
                if (m_dlnaServer->at(m_itemListPos - 1).friendlyName.c_get() == NULL) {
                    log_e("invalid pointer in dlna history");
                    m_dlnaHistory[(*m_dlnaLevel) + 1].name = strdup((char*)"dummy");
                    goto exit;
                }
                m_dlnaHistory[(*m_dlnaLevel) + 1].name = strdup(m_dlnaServer->at(m_itemListPos - 1).friendlyName.c_get());
                m_browseOnRelease = 1;
                goto exit;
            }
        }

        if (guard2) { // is file
            if (startsWith(m_srvContent->at(m_itemListPos - 1).itemURL.c_get(), "http")) {
                m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
                if (m_srvContent->at(m_itemListPos - 1).isAudio) {
                    sprintf(m_buff, "%s", m_srvContent->at(m_itemListPos - 1).title.c_get());
                    m_chptr = m_buff;
                    m_ra.arg1 = m_srvContent->at(m_itemListPos - 1).itemURL.get(); // url --> connecttohost()
                    m_ra.arg2 = m_srvContent->at(m_itemListPos - 1).title.get();   // filename --> showFileName()
                    if (m_ra.arg1 && m_ra.arg2) m_ra.val1 = 1;
                    m_browseOnRelease = 0;
                    goto exit;
                }
            }
        }

        if (guard3) { // is folder
            m_viewPoint = 0;
            sprintf(m_buff, "%s (%d)", m_srvContent->at(m_itemListPos - 1).title.c_get(), m_srvContent->at(m_itemListPos - 1).childCount);
            m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
            m_chptr = m_buff;
            x_ps_free(&m_dlnaHistory[(*m_dlnaLevel) + 1].objId);
            m_dlnaHistory[(*m_dlnaLevel) + 1].objId = strdup(m_srvContent->at(m_itemListPos - 1).objectId.c_get());
            x_ps_free(&m_dlnaHistory[(*m_dlnaLevel) + 1].name);
            m_dlnaHistory[(*m_dlnaLevel) + 1].name = strdup(m_srvContent->at(m_itemListPos - 1).title.c_get());
            m_browseOnRelease = 3;
            goto exit;
        }
        // log_i("at this position is nothing to do");
    exit:
        return;
    }

  public:
    void prevPage() { // from IR control
        if (m_viewPoint == 0) return;
        if (m_viewPoint > 9)
            m_viewPoint -= 9;
        else
            m_viewPoint = 0;
        if (m_currItemNr[*m_dlnaLevel] > 9) {
            m_currItemNr[*m_dlnaLevel] -= 9;
        } else {
            m_currItemNr[*m_dlnaLevel] = 0;
        }
        m_chptr = NULL;
        m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9);
        m_dlna->loop();
        while (m_dlna->getState() != m_dlna->IDLE) {
            m_dlna->loop();
            vTaskDelay(10);
        } // wait of browse rady
        m_srvContent = &m_dlna->getBrowseResult();
        dlnaItemsList();
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        return;
    }
    void nextPage() { // from IR control
        if (m_dlnaMaxItems <= m_viewPoint + 9) return;
        if (m_dlnaMaxItems - 9 > m_viewPoint) {
            m_viewPoint += 9;
        } else {
            m_viewPoint = m_dlnaMaxItems - 9;
        }
        if (m_dlnaMaxItems - 9 > m_currItemNr[*m_dlnaLevel]) {
            m_currItemNr[*m_dlnaLevel] += 9;
        } else {
            m_currItemNr[*m_dlnaLevel] = m_dlnaMaxItems - 1;
        }
        m_chptr = NULL;
        m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9);
        m_dlna->loop();
        while (m_dlna->getState() != m_dlna->IDLE) {
            m_dlna->loop();
            vTaskDelay(10);
        } // wait of browse rady
        m_srvContent = &m_dlna->getBrowseResult();
        dlnaItemsList();
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        return;
    }
    void prevItem() { // from IR control
        if (*m_dlnaLevel == 0 && m_currItemNr[*m_dlnaLevel] == 0) return;
        if (m_currItemNr[*m_dlnaLevel] < 0) return;
        if (m_currItemNr[*m_dlnaLevel] < m_viewPoint) {
            m_viewPoint -= 9;
            if (m_viewPoint < 0) m_viewPoint = 0;
            m_chptr = NULL;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse ready
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return;
        }
        m_currItemNr[*m_dlnaLevel]--;
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        drawItem(m_currItemNr[*m_dlnaLevel] + 1 - m_viewPoint + 1); // std colour
    }
    void nextItem() { // from IR control
        if (m_dlnaMaxItems == m_currItemNr[*m_dlnaLevel] - 1) return;
        if (m_currItemNr[*m_dlnaLevel] == m_dlnaMaxItems - 1) return;
        m_currItemNr[*m_dlnaLevel]++;
        if (m_currItemNr[*m_dlnaLevel] >= m_viewPoint + 9) {
            m_viewPoint += 9;
            m_chptr = NULL;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return;
        }
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        drawItem(m_currItemNr[*m_dlnaLevel] - 1 - m_viewPoint + 1); // std colour
    }
    const char* getSelectedURL() { // ok from IR
        if (*m_dlnaLevel == 0) {   //------------------------------------------------------------------------------------------------------- choose server
            // log_e("server %s", m_dlnaServer.friendlyName[m_currItemNr[0]]);
            m_chptr = m_dlnaServer->at(m_currItemNr[0]).friendlyName.c_get();
            m_currDLNAsrvNr = m_currItemNr[0];
            m_currItemNr[*m_dlnaLevel] = m_currItemNr[0];
            drawItem(m_currItemNr[*m_dlnaLevel] + m_viewPoint + 1, true); // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)++;
            x_ps_free(&m_dlnaHistory[*m_dlnaLevel].name);
            if (m_dlnaServer->at(m_currItemNr[0]).friendlyName.c_get() == NULL) {
                log_e("invalid pointer in dlna history");
                m_dlnaHistory[*m_dlnaLevel].name = strdup((char*)"dummy");
                return NULL;
            }
            m_dlnaHistory[*m_dlnaLevel].name = strdup(m_dlnaServer->at(m_currItemNr[0]).friendlyName.c_get());
            m_dlna->browseServer(m_currDLNAsrvNr, "0", 0, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            m_dlnaMaxItems = m_dlna->getTotalMatches();
            m_dlnaHistory[*m_dlnaLevel].maxItems = m_dlnaMaxItems; // level 1
            // log_e("m_dlnaMaxItems %i, level %i", m_dlnaMaxItems, (*m_dlnaLevel));
            dlnaItemsList();
            return NULL;
        }
        if (m_currItemNr[*m_dlnaLevel] + 1 == m_viewPoint) { // DLNA history, parent item ---------------------------------------------- back to parent
            // log_e("%s", m_dlnaHistory[*m_dlnaLevel].name);
            drawItem(0, true); // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)--;
            m_dlnaMaxItems = m_dlnaHistory[*m_dlnaLevel].maxItems;
            m_viewPoint = 0;
            m_currItemNr[*m_dlnaLevel] = 0;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return NULL;
        }
        if (strcmp(m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get(), "?") == 0) { // --------------------------------------- choose folder
            drawItem(m_currItemNr[*m_dlnaLevel] - m_viewPoint + 1, true);                                   // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)++;
            m_currItemNr[*m_dlnaLevel] = 0;
            x_ps_free(&m_dlnaHistory[*m_dlnaLevel].objId);
            m_dlnaHistory[*m_dlnaLevel].objId = strdup(m_srvContent->at(m_currItemNr[(*m_dlnaLevel) - 1] - m_viewPoint).objectId.c_get());
            x_ps_free(&m_dlnaHistory[*m_dlnaLevel].name);
            m_dlnaHistory[*m_dlnaLevel].name = strdup(m_srvContent->at(m_currItemNr[(*m_dlnaLevel) - 1] - m_viewPoint).title.c_get());
            m_viewPoint = 0;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, 0, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            m_dlnaMaxItems = m_dlna->getTotalMatches();
            m_dlnaHistory[*m_dlnaLevel].maxItems = m_dlnaMaxItems;
            // log_e("m_dlnaMaxItems %i, level %i", m_dlnaMaxItems, (*m_dlnaLevel)); // level 2, 3, 4...
            dlnaItemsList();
            if (!m_dlnaMaxItems) { // folder is empty
                m_currItemNr[*m_dlnaLevel]--;
                drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
            }
            return NULL;
        }
        if (startsWith(m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get(), "http") != 0) { // ---------------------------------- choose file
            drawItem(m_currItemNr[*m_dlnaLevel] - m_viewPoint + 1, true);                                          // make cyan
            vTaskDelay(300);
            return m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get();
        }
        return NULL;
    }
    const char* getSelectedTitle() { return m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).title.c_get(); }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
  ———————————————————————————————————————————————————————
  | Audio Files                     Vol11    15:32:18   |
  | /audiofiles/myPlaylist/                    1-9/11   |           <- parent folder  m_curAudioFileNr = -1
  |   320_test.mpr 9610227                              |           <--               m_curAudioFileNr - m_viewpos = 0
  |   If_I_Had_a_Chicken_mono_mp3 1591510               |           <--
  |   If_I_Had_a_Chicken_mono_16bit.wav 13257580        |           <--
  |   If_I_Had_a_Chicken_mono_8bit.wav 6628972          |           <--
  |   If_I_Had_a_Chicken_stereo_mp3 6012554             |           <--
  |   If_I_Had_a_Chicken_stereo_16bit.wav 26514608      |           <--
  |   If_I_Had_a_Chicken_stereo_8bit.wav 1327260        |           <--
  |   beep.mp3 75302                                    |           <--
  |     click.mp3 3360                                  |           <--               m_curAudioFileNr - m_viewpos = 8
  | 003   0:00    128K              IP:192.168.178.24   |
  ———————————————————————————————————————————————————————
*/

class fileList : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_oldX = 0;
    int16_t     m_oldY = 0;
    uint8_t     m_lineHight = 0;
    uint8_t     m_browseOnRelease = 0;
    uint16_t    m_fileListPos = 0;
    int16_t     m_curAudioFileNr = 0;
    uint16_t    m_viewPos = 0;
    uint8_t     m_fontSize = 0;
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    char*       m_name = NULL;
    char*       m_curAudioFolder = NULL;
    char*       m_curAudioPath = NULL;
    char*       m_curAudioName = NULL;
    char*       m_fileItemsPos = NULL;
    const char* m_tftSize = NULL;
    const char* m_rootColor = ANSI_ESC_LIGHTBROWN;
    const char* m_folderColor = ANSI_ESC_ORANGE;
    const char* m_fileColor = ANSI_ESC_WHITE;
    const char* m_selectColor = ANSI_ESC_CYAN;
    const char* m_irColor = ANSI_ESC_MAGENTA;
    const char* m_currentColor = ANSI_ESC_MAGENTA;
    releasedArg m_ra;

  public:
    fileList(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("fileList");
        m_fileItemsPos = x_ps_malloc(30);
        m_curAudioFolder = x_ps_malloc(1024);
        m_curAudioPath = x_ps_malloc(1024);
        m_curAudioName = x_ps_malloc(1024);
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_ra.arg1 = NULL;
        m_ra.arg2 = NULL;
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~fileList() {
        x_ps_free(&m_name);
        x_ps_free(&m_fileItemsPos);
        x_ps_free(&m_curAudioFolder); //   /audiofiles/folder1
        x_ps_free(&m_curAudioName);   //   song.mp3
        x_ps_free(&m_curAudioPath);   //   /audiofiles/folder1/song.mp3
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_enabled = false;
        m_tftSize = tftSize;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(const char* cur_AudioFolder, uint16_t curAudioFileNr) {
        m_browseOnRelease = 0;
        m_clicked = false;
        m_enabled = true;
        if (!cur_AudioFolder) {
            log_w("cur_AudioFolder set to /audiofiles/");
            strcpy(m_curAudioFolder, (char*)"/audiofiles/");
        } else if (cur_AudioFolder != m_curAudioFolder)
            strcpy(m_curAudioFolder, cur_AudioFolder); // cur_AudioFolder can have the same address as m_curAudioFolder
        m_curAudioFileNr = curAudioFileNr;
        _SD_content.listFilesInDir(m_curAudioFolder, true, false);
        if (m_curAudioFileNr >= _SD_content.getSize()) m_curAudioFileNr = _SD_content.getSize(); // guard
        m_viewPos = calculateDisplayStartPosition(_SD_content.getSize(), m_curAudioFileNr);      // calculate viewPos
        audioFileslist(m_viewPos);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_clicked == false) {
            m_oldX = x;
            m_oldY = y;
        }
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (!m_clicked) return false;

        hasReleased(x - m_x, y - m_y);
        m_clicked = false;
        int pos = (y - m_y) / m_lineHight;

        char* fileName = NULL;

        if (m_browseOnRelease == 1) {
            if (m_viewPos + 9 >= _SD_content.getSize()) {
                goto exit;
            } // wipe up
            else {
                m_viewPos += 9;
                m_ra.val1 = 1;
            }
            audioFileslist(m_viewPos);
        }
        if (m_browseOnRelease == 2) {
            if (m_viewPos == 0) { goto exit; } // wipe down
            if (m_viewPos > 9) {
                m_viewPos -= 9;
                m_ra.val1 = 1;
            } else {
                m_viewPos = 0;
                m_ra.val1 = 1;
            }
            audioFileslist(m_viewPos);
        }
        if (m_browseOnRelease == 3) {                                 // log_e("m_curAudioFolder = %s", m_curAudioFolder);                                         // previous folder
            if (!strcmp(m_curAudioFolder, "/audiofiles/")) goto exit; // is already the root
            myList.drawLine(pos, m_curAudioFolder, "", "", ANSI_ESC_CYAN, 1);
            int lastSlash = rfind(m_curAudioFolder, '/');
            if (lastSlash != -1) { // Look for the penultimate '/' before the position of the last
                int secondLastSlash = rfind(m_curAudioFolder, '/', lastSlash - 1);
                if (secondLastSlash != -1) m_curAudioFolder[secondLastSlash + 1] = '\0';
            }
            log_e("m_curAudioFolder = %s", m_curAudioFolder);
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            _SD_content.listFilesInDir(m_curAudioFolder, true, false);
            m_ra.val1 = 2;
            m_ra.val2 = m_curAudioFileNr;
            m_ra.arg1 = m_curAudioFolder;
        }
        if (m_browseOnRelease == 4) {
            m_viewPos += m_fileListPos; // next folder
            int16_t idx = m_viewPos - 1;
            myList.drawLine(pos, _SD_content.getColouredSStringByIndex(idx), "", "", ANSI_ESC_CYAN, 1);
            strcpy(m_curAudioFolder, _SD_content.getFilePathByIndex(idx));
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            _SD_content.listFilesInDir(m_curAudioFolder, true, false);
            m_ra.val1 = 2; // isfolder
            m_ra.val2 = m_viewPos;
            m_ra.arg1 = m_curAudioFolder;
        }
        if (m_browseOnRelease == 5) {
            m_viewPos += m_fileListPos; // play file
            myList.drawLine(pos, m_curAudioName, "", "", ANSI_ESC_CYAN, 1);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            m_ra.arg1 = m_curAudioFolder; // fileFolder
            m_ra.arg2 = m_curAudioName;   // fileName
            m_ra.arg3 = m_curAudioPath;   // filePath
            m_ra.val1 = 3;                // isfile
            m_ra.val2 = m_viewPos - 1;    // fileNr (is curAudioFileNr)
        }
    exit:
        m_browseOnRelease = 0;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        x_ps_free(&fileName);
        m_ra.val1 = 0;
        m_ra.arg1 = NULL;
        return true;
    }

    void prevPage() { // from IR control
        if (m_viewPos == 0) return;
        if (m_viewPos > 9) {
            m_viewPos -= 9;
            m_curAudioFileNr -= 9;
        } else {
            m_curAudioFileNr -= m_viewPos;
            m_viewPos = 0;
        }
        audioFileslist(m_viewPos);
        return;
    }
    void nextPage() { // from IR control
        if (m_viewPos + 9 <= _SD_content.getSize() - 1) {
            m_viewPos += 9;
            m_curAudioFileNr += 9;
            if (m_curAudioFileNr > _SD_content.getSize() - 1) m_curAudioFileNr = _SD_content.getSize() - 1;
            audioFileslist(m_viewPos);
            return;
        }
    }

    void prevFile() { // from IR control
        if (m_curAudioFileNr < 0) return;
        if (m_curAudioFileNr && m_curAudioFileNr - m_viewPos == 0) {
            if (m_viewPos >= 9)
                m_viewPos -= 9;
            else
                m_viewPos = 0;
            m_curAudioFileNr--;
            audioFileslist(m_viewPos);
            return;
        }
        int         pos = m_curAudioFileNr - m_viewPos + 1;
        const char* color = m_fileColor;                                // assume is file
        if (_SD_content.isDir(m_curAudioFileNr)) color = m_folderColor; // is folder
        myList.colourLine(pos, color);
        m_curAudioFileNr--;
        myList.colourLine(pos - 1, m_irColor);
    }
    void nextFile() { // from IR control
        if (m_curAudioFileNr == _SD_content.getSize() - 1) return;
        if (m_curAudioFileNr - m_viewPos == 8) {
            if (m_viewPos + 9 < _SD_content.getSize())
                m_viewPos += 9;
            else
                m_viewPos = _SD_content.getSize() - 1;
            m_curAudioFileNr++;
            audioFileslist(m_viewPos);
            return;
        }
        const char* color;
        int         pos = m_curAudioFileNr - m_viewPos + 1;
        if (m_curAudioFileNr == -1) {
            color = m_rootColor;
        } // is root dir
        else {
            if (_SD_content.isDir(m_curAudioFileNr))
                color = m_folderColor; // is folder
            else
                color = m_fileColor; // is file
        }
        myList.colourLine(pos, color);
        m_curAudioFileNr++;
        myList.colourLine(pos + 1, m_irColor);
    }
    const char* getSelectedFile() {
        if (m_curAudioFileNr == -1) {                                   // get parent folder
            if (!strcmp(m_curAudioFolder, "/audiofiles/")) return NULL; // is already the root
            myList.colourLine(m_y, m_selectColor);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            int lastSlash = rfind(m_curAudioFolder, '/');
            if (lastSlash != -1) { // Look for the penultimate '/' before the position of the last
                int secondLastSlash = rfind(m_curAudioFolder, '/', lastSlash - 1);
                if (secondLastSlash != -1) m_curAudioFolder[secondLastSlash + 1] = '\0'; // previous folder
            }
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            _SD_content.listFilesInDir(m_curAudioFolder, true, false);
            show(m_curAudioFolder, 0);
            return NULL;
        }
        if (_SD_content.isDir(m_curAudioFileNr)) { // is child folder
            myList.colourLine(m_y, m_selectColor);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            strcpy(m_curAudioPath, _SD_content.getFilePathByIndex(m_curAudioFileNr));
            show(m_curAudioPath, 0);
            return NULL;
        }
        myList.colourLine(m_y, m_selectColor);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return _SD_content.getFilePathByIndex(m_curAudioFileNr);
    }
    const char* getSelectedFileName() { return _SD_content.getFileNameByIndex(m_curAudioFileNr); }
    const char* getSelectedFilePath() {
        myList.colourLine(m_curAudioFileNr - m_viewPos + 1, m_selectColor);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return _SD_content.getFilePathByIndex(m_curAudioFileNr);
    }
    uint16_t getSelectedFileNr() { return m_curAudioFileNr; }

  private:
    void audioFileslist(uint16_t viewPos) {
        // guard -------------------------------------------------------------------------------------------------------------------------------------
        if (_SD_content.getSize() == 0) { ; }                                          // folder empty
        if (viewPos >= _SD_content.getSize()) { viewPos = _SD_content.getSize() - 1; } // viewPos too high
        //--------------------------------------------------------------------------------------------------------------------------------------------

        tft.setFont(m_fontSize);
        myList.setMode(PLAYER, m_tftSize, m_fontSize);
        myList.clearList();
        const char* color;

        color = m_folderColor;
        if (strcmp(m_curAudioFolder, "/audiofiles/") == 0) color = m_rootColor; // is root
        myList.drawLine(0, m_curAudioFolder, "", "", color, 0);
        color = m_fileColor;
        for (uint8_t pos = 1; pos < 10; pos++) {
            int idx = pos + viewPos - 1;
            if (pos == 1 && viewPos > 0 && _SD_content.getSize()) { myList.drawTriangeUp(); }
            if (pos == 9 && viewPos + 9 < _SD_content.getSize()) { myList.drawTriangeDown(); }
            if (viewPos + pos > _SD_content.getSize()) break;
            if (_SD_content.isDir(idx)) {
                if (idx == m_curAudioFileNr) {
                    color = m_currentColor;
                } // is current folder
                else {
                    color = m_folderColor;
                } // is folder
            } else {
                if (idx == m_curAudioFileNr) {
                    color = m_currentColor;
                } // current file
                else {
                    color = m_fileColor;
                } // is file
            }
            if (_SD_content.isDir(idx))
                myList.drawLine(pos, _SD_content.getFileNameByIndex(idx), "", "", color, 0);
            else
                myList.drawLine(pos, _SD_content.getFileNameByIndex(idx), "", "", color, _SD_content.getFileSizeByIndex(idx));
        }
        uint16_t firstVal = viewPos + 1;
        uint16_t secondVal = firstVal + 8;
        if (secondVal > _SD_content.getSize()) secondVal = _SD_content.getSize();
        myList.drawPosInfo(firstVal, secondVal, _SD_content.getSize(), ANSI_ESC_ORANGE); // shows the current items pos e.g. "30-39/210"
        return;
    }

    int calculateDisplayStartPosition(int list_size, int current_position) {
        // Calculate the theoretical starting position to get current_position to the middle
        int start_position = current_position - 4;
        // Make sure the starting position is not negative
        start_position = std::max(0, start_position);
        // Make sure the starting position doesn't go beyond the end of the list
        if (start_position + 9 > list_size) { start_position = std::max(0, list_size - 9); }
        return start_position;
    }

    void hasReleased(uint16_t x, uint16_t y) {
        m_fileListPos = y / (m_h / 10);

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // -------------------------------------- normal wipe up
            m_browseOnRelease = 1;
            goto exit;
        }

        if (m_oldY && (m_oldY + 2 * m_lineHight < y)) { // -------------------------------------- normal wipe down
            m_browseOnRelease = 2;
            goto exit;
        }

        if (m_fileListPos == 0) {                         //  ----------------------------------------------------------- previous folder
            if (lastIndexOf(m_curAudioFolder, '/') > 0) { // not the first '/'
                m_browseOnRelease = 3;
            }
        } else {
            if (m_fileListPos + m_viewPos > _SD_content.getSize()) goto exit; // ----------------- next folder
            int idx = m_viewPos + m_fileListPos - 1;
            if (_SD_content.isDir(idx)) {
                strcpy(m_curAudioName, "");
                strcpy(m_curAudioFolder, _SD_content.getFileFolderByIndex(idx));
                m_browseOnRelease = 4;
            } else { // -------------------------------------------------------------------------- playfile
                strcpy(m_curAudioName, _SD_content.getFileNameByIndex(idx));
                strcpy(m_curAudioFolder, _SD_content.getFileFolderByIndex(idx));
                strcpy(m_curAudioPath, _SD_content.getFilePathByIndex(idx));
                m_browseOnRelease = 5;
            }
        }
    exit:
        return;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
extern stationManagement staMgnt; /*
———————————————————————————————————————————————————————
| Stations List                   Vol3    01:16:32    |           m_stationListPos
| 017 BGRADIOk                                        |           <-- 0
| 018 knixx.fm                                        |           <-- 1
| 019 -0N-Chrismas on Radio                           |           <-- 2
| 020 BBC 6music                                      |           <-- 3
| 021 -0N-Movues on Radio                             |           <-- 4
| 022 -0N-Top40 on Radio                              |           <-- 5
| 023 Rockantenne Alternative (mp3)                   |           <-- 6
| 024 Gra Wroclaw                                     |           <-- 7
| 025 Classic EuroDisco                               |           <-- 8
| 026 Hit Radio FFH - Soundtrack (AAC+)               |           <-- 9
| 003   0:00    128K              IP:192.168.178.24   |
———————————————————————————————————————————————————————
*/
class stationsList : public RegisterTable {
  private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_oldX = 0;
    int16_t     m_oldY = 0;
    uint8_t     m_lineHight = 0;
    uint16_t    m_firstStationsLineNr = 0;
    uint16_t*   m_curSstationNr = NULL;
    uint16_t    m_curStaNrCpy = 0;
    uint8_t     m_browseOnRelease = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_stationListPos = 0;
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    char*       m_buff = NULL;
    releasedArg m_ra;
    const char* m_colorToDraw = NULL;
    const char* m_staNameToDraw = NULL;
    const char* m_tftSize = NULL;
    uint16_t    m_staNrToDraw = 0;

  public:
    stationsList(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("stationsList");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        m_ra.arg1 = NULL;
        m_ra.arg2 = NULL;
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~stationsList() {
        x_ps_free(&m_name);
        x_ps_free(&m_buff);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize, uint16_t* curStationNr) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_curSstationNr = curStationNr;
        m_enabled = false;
        m_tftSize = tftSize;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show() {
        m_clicked = false;
        m_enabled = true;
        m_browseOnRelease = 0;
        stationslist(true);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        hasClicked(x - m_x, y - m_y);
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;

        if (m_browseOnRelease == 1) {
            stationslist(false); // wipe up
        }
        if (m_browseOnRelease == 2) {
            stationslist(false); // wipe down
        }
        if (m_browseOnRelease == 3) {
            myList.getTxtByPos(m_stationListPos); // click
            myList.colourLine(m_stationListPos, ANSI_ESC_CYAN);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            m_ra.val1 = myList.getNumberByPos(m_stationListPos);
        }
        m_browseOnRelease = 0;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        x_ps_free(&m_buff);
        m_ra.val1 = 0;
        m_ra.arg1 = NULL;
        return true;
    }

  private:
    void stationslist(bool first) {
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if (first) {
            if (staMgnt.getSumStations() <= 10)
                m_firstStationsLineNr = 0;
            else if (*m_curSstationNr < 5)
                m_firstStationsLineNr = 0;
            else if (*m_curSstationNr + 5 <= staMgnt.getSumStations())
                m_firstStationsLineNr = *m_curSstationNr - 5;
            else
                m_firstStationsLineNr = staMgnt.getSumStations() - 10;
            m_curStaNrCpy = *m_curSstationNr;
            if (m_curStaNrCpy == 0) m_curStaNrCpy = 1;
        }
        myList.clearList();
        myList.setMode(RADIO, m_tftSize, m_fontSize);

        for (uint8_t pos = 0; pos < 10; pos++) {
            if (pos + m_firstStationsLineNr + 1 > staMgnt.getSumStations()) break;
            if (staMgnt.getStationFav(pos + m_firstStationsLineNr + 1) == '*')
                m_colorToDraw = ANSI_ESC_WHITE; // is fav station
            else
                m_colorToDraw = ANSI_ESC_GREY;                                                        // is not a fav station
            if ((pos + m_firstStationsLineNr + 1) == m_curStaNrCpy) m_colorToDraw = ANSI_ESC_MAGENTA; // is the current station

            m_staNameToDraw = staMgnt.getStationName(pos + m_firstStationsLineNr + 1); // the station name
            m_staNrToDraw = pos + m_firstStationsLineNr + 1;                           // the station number
            myList.drawLine(pos, m_staNameToDraw, NULL, NULL, m_colorToDraw, m_staNrToDraw);
            if (pos == 1 && m_firstStationsLineNr > 0 && staMgnt.getSumStations()) { myList.drawTriangeUp(); }
            if (pos == 9 && m_firstStationsLineNr + 10 < staMgnt.getSumStations()) { myList.drawTriangeDown(); }
        }
        xSemaphoreGive(mutex_display);
    }
    void hasClicked(uint16_t x, uint16_t y) {
        m_stationListPos = y / (m_h / 10);
        if (m_oldY && (m_oldY + 2 * m_lineHight < y)) { // wipe up
            if (m_browseOnRelease != 1) {
                m_browseOnRelease = 1;
                if (m_firstStationsLineNr == 0) {
                    m_browseOnRelease = 0;
                    return;
                } // nothing to do
                else if (m_firstStationsLineNr < 10)
                    m_firstStationsLineNr = 0;
                else
                    m_firstStationsLineNr -= 9;
            }
            return;
        }

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // wipe down
            if (m_browseOnRelease != 2) {
                m_browseOnRelease = 2;
                if (m_firstStationsLineNr + 10 >= staMgnt.getSumStations()) {
                    m_browseOnRelease = 0;
                    return;
                } // nothing to do
                else
                    m_firstStationsLineNr += 9;
            }
            return;
        }
        if (myList.getNumberByPos(m_stationListPos) == -1) return;
        if (m_oldX || m_oldY) return;
        m_oldX = x;
        m_oldY = y;
        m_browseOnRelease = 3; // pos has clicked
        return;
    }

  public:
    void prevPage() { // from IR control
        if (m_firstStationsLineNr == 0) {
            return;
        } // nothing to do
        else if (m_firstStationsLineNr < 10) {
            m_curStaNrCpy -= m_firstStationsLineNr;
            m_firstStationsLineNr = 0;
        } else {
            m_firstStationsLineNr -= 9;
            m_curStaNrCpy -= 9;
        }
        stationslist(false);
    }
    void nextPage() { // from IR control
        if (m_firstStationsLineNr + 10 >= staMgnt.getSumStations()) {
            m_browseOnRelease = 0;
            return;
        } // nothing to do
        else {
            m_firstStationsLineNr += 9;
            m_curStaNrCpy += 9;
            if (m_curStaNrCpy > staMgnt.getSumStations()) m_curStaNrCpy = staMgnt.getSumStations();
        }
        stationslist(false);
    }
    void prevStation() { // from IR control
        if (m_curStaNrCpy < 2) return;
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr - 1;
        if (pos < 0) return;
        if (pos == 0) { // prev page
            if (m_firstStationsLineNr > 8)
                m_firstStationsLineNr -= 9;
            else
                m_firstStationsLineNr = 0;
            m_curStaNrCpy--;
            stationslist(false);
            return;
        }
        myList.colourLine(pos, staMgnt.getStationFav(m_curStaNrCpy) == '*' ? ANSI_ESC_WHITE : ANSI_ESC_GREY);
        myList.colourLine(pos - 1, ANSI_ESC_MAGENTA);
        m_curStaNrCpy--;
    }
    void nextStation() { // from IR control
        if (m_curStaNrCpy >= staMgnt.getSumStations()) return;
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr - 1;
        if (pos > 9) return;
        if (pos == 9) { // next Page
            m_firstStationsLineNr += 9;
            m_curStaNrCpy++;
            stationslist(false);
            return;
        }
        myList.colourLine(pos, staMgnt.getStationFav(m_curStaNrCpy) == '*' ? ANSI_ESC_WHITE : ANSI_ESC_GREY);
        myList.colourLine(pos + 1, ANSI_ESC_MAGENTA);
        m_curStaNrCpy++;
    }
    uint16_t getSelectedStation() { // from IR control
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr;
        myList.colourLine(pos - 1, ANSI_ESC_CYAN);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return m_curStaNrCpy;
    }
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class vuMeter : public RegisterTable {
  private:
    uint16_t    m_x = 0;
    uint16_t    m_y = 0;
    uint16_t    m_w = 0;
    uint16_t    m_h = 0;
    uint32_t    m_bgColor = TFT_BLACK;
    uint32_t    m_frameColor = TFT_DARKGREY;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_backgroundTransparency = false;
    uint8_t     m_VUleftCh = 0;  // VU meter left channel
    uint8_t     m_VUrightCh = 0; // VU meter right channel
    releasedArg m_ra;
    uint8_t     m_segm_w = 0;
    uint8_t     m_segm_h = 0;
    uint8_t     m_frameSize = 1;
    uint16_t    m_real_w = 0;
    uint16_t    m_real_h = 0;

  public:
    vuMeter(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("vuMeter");
        m_bgColor = TFT_BLACK;
    }
    ~vuMeter() { x_ps_free(&m_name); }
    void begin(uint16_t x, uint16_t y, uint16_t real_w, uint16_t real_h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_real_w = real_w;
        m_real_h = real_h;
#if TFT_CONTROLLER < 2 // 320 x 240px
        m_segm_w = 9;
        m_segm_h = 7;
#else // 480 x 320px
        m_segm_w = 12;
        m_segm_h = 8;
#endif
        m_w = 2 * m_segm_w + 3 * m_frameSize;
        m_h = 12 * m_segm_h + 13 * m_frameSize;
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool backgroundTransparency = false) {
        m_backgroundTransparency = backgroundTransparency;
        m_enabled = true;
        m_clicked = false;
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_real_w, m_real_h);
        } else {
            tft.fillRect(m_x, m_y, m_real_w, m_real_h, m_bgColor);
        }
        tft.drawRect(m_x, m_y, m_w, m_h, m_frameColor);
        for (uint8_t i = 0; i < 12; i++) {
            drawRect(i, 0, 0);
            drawRect(i, 1, 0);
        }
        m_VUleftCh = 0;
        m_VUrightCh = 0;
    }
    void hide() {
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            m_enabled = false;
        }
    }
    void disable() { m_enabled = false; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void update(uint16_t vum) {
        if (!m_enabled) return;
        uint8_t left = map_l(vum >> 8, 0, 127, 0, 11);
        uint8_t right = map_l(vum & 0x00FF, 0, 127, 0, 11);

        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if (left > m_VUleftCh) {
            for (int32_t i = m_VUleftCh; i < left; i++) { drawRect(i, 1, 1); }
        }
        if (left < m_VUleftCh) {
            for (int32_t i = left; i < m_VUleftCh; i++) { drawRect(i, 1, 0); }
        }
        m_VUleftCh = left;

        if (right > m_VUrightCh) {
            for (int32_t i = m_VUrightCh; i < right; i++) { drawRect(i, 0, 1); }
        }
        if (right < m_VUrightCh) {
            for (int32_t i = right; i < m_VUrightCh; i++) { drawRect(i, 0, 0); }
        }
        m_VUrightCh = right;
        xSemaphoreGive(mutex_display);
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }

  private:
    void drawRect(uint8_t pos, uint8_t ch, bool br) {
        uint16_t color = 0;
        uint16_t y_end = m_y + m_h - m_frameSize - m_segm_h;
        uint16_t xPos = m_x + m_frameSize + ch * (m_segm_w + m_frameSize);
        uint16_t yPos = y_end - pos * (m_frameSize + m_segm_h);
        if (pos > 11) return;
        switch (pos) {
            case 0 ... 6: // green
                br ? color = TFT_GREEN : color = TFT_DARKGREEN;
                break;
            case 7 ... 9: // yellow
                br ? color = TFT_YELLOW : color = TFT_DARKYELLOW;
                break;
            case 10 ... 11: // red
                br ? color = TFT_RED : color = TFT_DARKRED;
                break;
        }
        tft.fillRect(xPos, yPos, m_segm_w, m_segm_h, color);
    };
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayHeader : public RegisterTable {
  private:
    textbox*    txt_Item = new textbox("header_Item");          // Radio, Player, Clock....
    pictureBox* pic_Speaker = new pictureBox("header_Speaker"); // loudspeaker symbol
    textbox*    txt_Volume = new textbox("header_Volume");      // volume
    pictureBox* pic_RSSID = new pictureBox("header_RSSID");     // RSSID symbol
    timeString* m_timeStringObject = NULL;
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int8_t      m_rssi = 0;
    int8_t      m_old_rssi = -1;
    uint8_t     m_fontSize = 0;
    uint8_t     m_volume = 0;
    uint32_t    m_bgColor = TFT_BLACK;
    char*       m_name = NULL;
    char*       m_item = NULL;
    char        m_time[10] = "00:00:00";
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_speakerOn = false;
    bool        m_backgroundTransparency = false;
    const char  m_rssiSymbol[5][18] = {"/common/RSSI0.png", "/common/RSSI1.png", "/common/RSSI2.png", "/common/RSSI3.png", "/common/RSSI4.png"};
    const char  m_speakerSymbol[2][25] = {"/common/Speaker_off.png", "/common/Speaker_on.png"};
    releasedArg m_ra;
    uint16_t    m_itemColor = TFT_GREENYELLOW;
    uint16_t    m_volumeColor = TFT_DEEPSKYBLUE;
    uint16_t    m_timeColor = TFT_GREENYELLOW;
#if TFT_CONTROLLER < 2 // 320 x 240px
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 165;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 165;
        uint16_t w = 30;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 25 x 20 px
    struct w_v {
        uint16_t x = 195;
        uint16_t w = 30;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 225;
        uint16_t w = 35;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 27 x 20 px
    struct w_t {
        uint16_t x = 260;
        uint16_t w = 60;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time;      // time object
#elif TFT_CONTROLLER < 7 // 480 x 320px
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 240;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 240;
        uint16_t w = 45;
        uint8_t  pl = 3;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 38 x 30 px
    struct w_v {
        uint16_t x = 285;
        uint16_t w = 50;
        uint8_t  pl = 10;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 335;
        uint16_t w = 45;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 39 x 30 px
    struct w_t {
        uint16_t x = 380;
        uint16_t w = 100;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time; // time object
#else                    // 800 x 480px
    //------------------------------------------------------------------------padding-left-right-top-bottom--------------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 400;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 400;
        uint16_t w = 60;
        uint8_t  pl = 1;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 57 x 46 px
    struct w_v {
        uint16_t x = 460;
        uint16_t w = 100;
        uint8_t  pl = 10;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 560;
        uint16_t w = 80;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 64 x 48 px
    struct w_t {
        uint16_t x = 640;
        uint16_t w = 160;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time; // time object
#endif
  public:
    displayHeader(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("displayHeader");
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
        m_timeStringObject = new timeString("timeString", m_fontSize);
    }
    ~displayHeader() {
        x_ps_free(&m_name);
        x_ps_free(&m_item);
        delete txt_Item;
        delete pic_Speaker;
        delete txt_Volume;
        delete pic_RSSID;
        delete m_timeStringObject;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        txt_Item->begin(s_Item.x, m_y, s_Item.w, m_h, s_Item.pl, s_Item.pr, s_Item.pt, s_Item.pb);
        pic_Speaker->begin(s_Speaker.x, m_y, s_Speaker.w, m_h, s_Speaker.pl, s_Speaker.pr, s_Speaker.pt, s_Speaker.pb);
        txt_Volume->begin(s_Volume.x, m_y, s_Volume.w, m_h, s_Volume.pl, s_Volume.pr, s_Volume.pt, s_Volume.pb);
        pic_RSSID->begin(s_RSSID.x, m_y, s_RSSID.w, m_h, s_RSSID.pl, s_RSSID.pr, s_RSSID.pt, s_RSSID.pb);
        m_timeStringObject->begin(s_time.x, m_y, s_time.w, m_h, s_time.pl, s_time.pr, s_time.pt, s_time.pb);

        txt_Item->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_Item->setTextColor(m_itemColor);
        txt_Item->setFont(m_fontSize); // 0 -> auto
        pic_Speaker->setPicturePath(m_speakerSymbol[0]);
        txt_Volume->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_Volume->setFont(m_fontSize); // 0 -> auto
        pic_RSSID->setPicturePath(m_rssiSymbol[0]);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool transparency = false) {
        m_backgroundTransparency = transparency;
        m_timeStringObject->show(m_backgroundTransparency, false);
        m_enabled = true;
        m_clicked = false;
        m_old_rssi = -1;
        if (m_item)
            updateItem(m_item);
        else
            updateItem("");
        speakerOnOff(m_speakerOn);
        updateVolume(m_volume);
        updateRSSI(m_rssi);
        updateTime(m_time, true);
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void enable() { m_enabled = true; }
    void disable() { m_enabled = false; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void updateItem(const char* hl_item) { // radio, clock, audioplayer...
        if (!m_enabled) return;
        if (!hl_item) {
            log_e("hl_item is NULL");
            return;
        }
        if (m_item && !strcmp(hl_item, m_item)) return; // nothing to do
        x_ps_free(&m_item);
        m_item = strdup(hl_item);
        txt_Item->setText(hl_item);
        txt_Item->show(m_backgroundTransparency, false);
    }
    void setItemColor(uint16_t itemColor) {
        m_itemColor = itemColor;
        txt_Item->setTextColor(m_itemColor);
    }

    void speakerOnOff(bool on) {
        m_speakerOn = on;
        if (!m_enabled) return;
        pic_Speaker->setPicturePath(m_speakerSymbol[m_speakerOn]);
        pic_Speaker->show(m_backgroundTransparency, false);
    }
    void updateVolume(uint8_t vol) {
        m_volume = vol;
        if (!m_enabled) return;
        char buff[15];
        itoa(m_volume, buff, 10);
        txt_Volume->setTextColor(m_volumeColor);
        txt_Volume->setText(buff);
        txt_Volume->show(m_backgroundTransparency, false);
    }

    void updateRSSI(int8_t rssi, bool show = false) {
        static int32_t old_rssi = -1;
        int8_t         new_rssi = -1;
        if (rssi >= 0) return;
        m_rssi = rssi;
        if (m_rssi < -1) new_rssi = 4;
        if (m_rssi < -50) new_rssi = 3;
        if (m_rssi < -65) new_rssi = 2;
        if (m_rssi < -75) new_rssi = 1;
        if (m_rssi < -85) new_rssi = 0;

        if (new_rssi != old_rssi) {
            old_rssi = new_rssi; // no need to draw a rssi icon if rssiRange has not changed
            if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO) {
                static int32_t tmp_rssi = 0;
                if ((abs(rssi - tmp_rssi) > 4)) { SerialPrintfln("WiFI_info:   RSSI is " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " dB", rssi); }
                tmp_rssi = rssi;
            }
            if (m_enabled) show = true;
        }
        if (show) {
            pic_RSSID->setPicturePath(m_rssiSymbol[new_rssi]);
            pic_RSSID->show(m_backgroundTransparency, false);
        }
    }
    void updateTime(const char* hl_time, bool complete = true) {
        if (!m_enabled) return;
        memcpy(m_time, hl_time, 8); // hhmmss
        m_timeStringObject->updateTime(m_time, false);
    }
    void setTimeColor(uint16_t timeColor) {
        m_timeColor = timeColor;
        m_timeStringObject->setTextColor(m_timeColor);
        updateTime(m_time, true);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayFooter : public RegisterTable {
  private:
    pictureBox* pic_Antenna = new pictureBox("footer_Antenna");     // antenna symbol
    textbox*    txt_StaNr = new textbox("footer_StaNr");            // station number
    pictureBox* pic_Flag = new pictureBox("footer_Flag");           // flag symbol
    pictureBox* pic_Hourglass = new pictureBox("footer_Hourglass"); // hourglass symbol
    textbox*    txt_OffTimer = new textbox("footer_OffTimer");      // off timer
    textbox*    txt_BitRate = new textbox("footer_BitRate");        // bit rate
    textbox*    txt_IpAddr = new textbox("footer_IPaddr");          // ip address
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint8_t     m_fontSize = 0;
    int8_t      m_timeCounter = 0;
    uint8_t     m_volume = 0;
    uint16_t    m_staNr = 0;
    uint16_t    m_offTime = 0;
    uint32_t    m_bitRate = 0;
    uint16_t    m_bgColor = TFT_BLACK;
    uint16_t    m_stationColor = TFT_LAVENDER;
    uint16_t    m_bitRateColor = TFT_LAVENDER;
    uint16_t    m_ipAddrColor = TFT_GREENYELLOW;
    char*       m_name = NULL;
    char*       m_ipAddr = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_backgroundTransparency = false;
    releasedArg m_ra;
    const char  m_stationSymbol[22] = "/common/Antenna.png";
    const char  m_hourGlassymbol[2][27] = {"/common/Hourglass_blue.png", "/common/Hourglass_red.png"};
#if TFT_CONTROLLER < 2 // 320 x 240px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 0;
        uint16_t w = 25;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 19 x 20 px
    struct w_s {
        uint16_t x = 25;
        uint16_t w = 32;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 57;
        uint16_t w = 40;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  33...40 x 20 px
    struct w_h {
        uint16_t x = 100;
        uint16_t w = 20;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   16 x 20 px
    struct w_o {
        uint16_t x = 122;
        uint16_t w = 35;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 158;
        uint16_t w = 42;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 200;
        uint16_t w = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7 // 480 x 320px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 1;
        uint16_t w = 30;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 29 x 30 px
    struct w_s {
        uint16_t x = 30;
        uint16_t w = 50;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 80;
        uint16_t w = 48;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 3;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  40...48 x 24 px
    struct w_h {
        uint16_t x = 132;
        uint16_t w = 24;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   23 x 30 px
    struct w_o {
        uint16_t x = 160;
        uint16_t w = 54;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 214;
        uint16_t w = 66;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 280;
        uint16_t w = 200;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#else                    // 800 x 480px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 0;
        uint16_t w = 51;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 47 x 48 px
    struct w_s {
        uint16_t x = 51;
        uint16_t w = 84;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 135;
        uint16_t w = 80;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 5;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  60...80 x 40 px
    struct w_h {
        uint16_t x = 225;
        uint16_t w = 40;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 3;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   35 x 44 px
    struct w_o {
        uint16_t x = 265;
        uint16_t w = 75;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 340;
        uint16_t w = 110;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 450;
        uint16_t w = 350;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#endif
  public:
    displayFooter(const char* name, uint8_t fontSize) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("displayFooter");
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
    }
    ~displayFooter() {
        x_ps_free(&m_name);
        x_ps_free(&m_ipAddr);
        delete pic_Antenna;
        delete txt_StaNr;
        delete pic_Flag;
        delete pic_Hourglass;
        delete txt_OffTimer;
        delete txt_BitRate;
        delete txt_IpAddr;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        pic_Antenna->begin(s_Antenna.x, m_y, s_Antenna.w, m_h, s_Antenna.pl, s_Antenna.pr, s_Antenna.pt, s_Antenna.pb);
        txt_StaNr->begin(s_StaNr.x, m_y, s_StaNr.w, m_h, s_StaNr.pl, s_StaNr.pr, s_StaNr.pt, s_StaNr.pb);
        pic_Flag->begin(s_Flag.x, m_y, s_Flag.w, m_h, s_Flag.pl, s_Flag.pr, s_Flag.pt, s_Flag.pb);
        pic_Hourglass->begin(s_Hourglass.x, m_y, s_Hourglass.w, m_h, s_Hourglass.pl, s_Hourglass.pr, s_Hourglass.pt, s_Hourglass.pb);
        txt_OffTimer->begin(s_OffTimer.x, m_y, s_OffTimer.w, m_h, s_OffTimer.pl, s_OffTimer.pr, s_OffTimer.pt, s_OffTimer.pb);
        txt_BitRate->begin(s_BitRate.x, m_y, s_BitRate.w, m_h, s_BitRate.pl, s_BitRate.pr, s_BitRate.pt, s_BitRate.pb);
        txt_IpAddr->begin(s_IPaddr.x, m_y, s_IPaddr.w, m_h, s_IPaddr.pl, s_IPaddr.pr, s_IPaddr.pt, s_IPaddr.pb);

        txt_StaNr->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_StaNr->setTextColor(m_stationColor);
        txt_StaNr->setFont(m_fontSize); // 0 -> auto
        txt_OffTimer->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_OffTimer->setFont(m_fontSize); // 0 -> auto
        txt_BitRate->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        txt_BitRate->setTextColor(m_bitRateColor);
        txt_BitRate->setBorderColor(m_bitRateColor);
        txt_BitRate->setBorderWidth(1);
        txt_BitRate->setFont(m_fontSize); // 0 -> auto
        txt_IpAddr->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        txt_IpAddr->setTextColor(m_ipAddrColor);
        txt_IpAddr->setFont(m_fontSize); // 0 -> auto
        pic_Antenna->setPicturePath(m_stationSymbol);
    }
    const char* getName() { return m_name; }
    bool        isEnabled() { return m_enabled; }
    void        show(bool transparency = false) {
        m_backgroundTransparency = transparency;
        m_enabled = true;
        m_clicked = false;
        pic_Antenna->show(m_backgroundTransparency, false);
        updateStation(m_staNr);
        updateOffTime(m_offTime);
        updateBitRate(m_bitRate);
        if (m_ipAddr)
            writeIpAddr(m_ipAddr);
        else
            writeIpAddr("");
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void enable() { m_enabled = true; }
    void disable() { m_enabled = false; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void updateStation(uint16_t staNr) {
        m_staNr = staNr;
        char buff[10];
        sprintf(buff, "%03d", m_staNr);
        txt_StaNr->setText(buff);
        txt_StaNr->show(m_backgroundTransparency, false);
    }
    void setStationNrColor(uint16_t stationColor) { m_stationColor = stationColor; }
    void updateFlag(const char* flag) {
        if (flag) {
            pic_Flag->setAlternativPicturePath("/flags/unknown.jpg");
            pic_Flag->setPicturePath(flag);
            pic_Flag->show(m_backgroundTransparency, false);
        } else {
            pic_Flag->hide();
        }
    }
    void updateOffTime(uint16_t offTime) {
        m_offTime = offTime;
        if (!m_enabled) return;
        char buff[15];
        sprintf(buff, "%d:%02d", m_offTime / 60, m_offTime % 60);
        if (m_offTime) {
            txt_OffTimer->setTextColor(TFT_RED);
            txt_OffTimer->setText(buff);
            txt_OffTimer->show(m_backgroundTransparency, false);
            pic_Hourglass->setPicturePath(m_hourGlassymbol[1]);
            pic_Hourglass->show(m_backgroundTransparency, false);
        } else {
            txt_OffTimer->setTextColor(TFT_DEEPSKYBLUE);
            txt_OffTimer->setText(buff);
            txt_OffTimer->show(m_backgroundTransparency, false);
            pic_Hourglass->setPicturePath(m_hourGlassymbol[0]);
            pic_Hourglass->show(m_backgroundTransparency, false);
        }
    }
    void updateTC(uint8_t timeCounter) {
        m_timeCounter = timeCounter;
        if (!m_enabled) return;
        if (!m_timeCounter) {
            updateBitRate(m_bitRate);
        } else {
            uint16_t x0 = s_BitRate.x;
            uint16_t x1x2 = round(s_BitRate.x + ((float)((s_BitRate.w) / 10) * timeCounter)) - 1;
            uint16_t y0y1 = m_y + m_h - 5;
            uint16_t y2 = round((m_y + m_h - 5) - ((float)(m_h - 6) / 10) * timeCounter);
            if (m_backgroundTransparency) {
                tft.copyFramebuffer(1, 0, s_BitRate.x, m_y, s_BitRate.w, m_h);
            } else {
                tft.fillRect(s_BitRate.x, m_y, s_BitRate.w, m_h, m_bgColor);
            }
            tft.fillTriangle(x0, y0y1, x1x2, y0y1, x1x2, y2, TFT_RED);
        }
    }

    void updateBitRate(uint32_t bitRate) {
        m_bitRate = bitRate / 1000; // KBit/s
        if (!m_enabled) return;
        char sbr[10];
        itoa(m_bitRate, sbr, 10);
        if (m_bitRate < 1000) {
            strcat(sbr, "K");
        } else {
            sbr[2] = sbr[1];
            sbr[1] = '.';
            sbr[3] = 'M';
            sbr[4] = '\0';
        }
        txt_BitRate->setText(sbr);
        txt_BitRate->show(m_backgroundTransparency, false);
    }
    void setBitRateColor(uint16_t bitRateColor) {
        m_bitRateColor = bitRateColor;
        txt_BitRate->setBorderColor(m_bitRateColor);
        txt_BitRate->setTextColor(m_bitRateColor);
    }
    void setIpAddr(const char* ipAddr) {
        if (!ipAddr) return;
        x_ps_free(&m_ipAddr);
        m_ipAddr = strdup(ipAddr);
    }
    void writeIpAddr(const char* ipAddr) {
        char myIP[30] = "IP:";
        strcat(myIP, ipAddr);
        txt_IpAddr->setText(myIP, true, true);
        txt_IpAddr->show(m_backgroundTransparency, false);
    }
    void setIpAddrColor(uint16_t ipAddrColor) {
        m_ipAddrColor = ipAddrColor;
        txt_IpAddr->setTextColor(m_ipAddrColor);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        uint8_t pos = 0;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, pos);
        pic_Antenna->positionXY(x, y); // transfer the position to the graphic objects
        txt_StaNr->positionXY(x, y);
        pic_Flag->positionXY(x, y);
        pic_Hourglass->positionXY(x, y);
        txt_OffTimer->positionXY(x, y);
        txt_BitRate->positionXY(x, y);
        txt_IpAddr->positionXY(x, y);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }

  private:
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void GetRunTimeStats(char* pcWriteBuffer) {
    TaskStatus_t* pxTaskStatusArray;
    UBaseType_t   uxArraySize;
    uint8_t       ulStatsAsPercentage;
    uint64_t      ulTotalRunTime;
    char          leftSpace[] = "             |";

    // Take a snapshot of the number of tasks in case it changes while this function is executing.
    uxArraySize = uxTaskGetNumberOfTasks();

    // Allocate a TaskStatus_t structure for each task.  An array could be allocated statically at compile time.
    pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    if (pxTaskStatusArray != NULL) {
        // Generate raw status information about each task.
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, (UBaseType_t)uxArraySize, &ulTotalRunTime);

        // For percentage calculations.
        ulTotalRunTime /= 100UL;

        char* tmpBuff = x_ps_malloc(100);
        strcpy(pcWriteBuffer, leftSpace);
        strcat(pcWriteBuffer, ANSI_ESC_YELLOW " TASKNAME            | RUNTIMECOUNTER | TOTALRUNTIME[%] | CORE | PRIO  |\n");
        strcat(pcWriteBuffer, leftSpace);
        strcat(pcWriteBuffer, "---------------------+----------------+-----------------+------+-------|\n");

        // Avoid divide by zero errors.
        if (ulTotalRunTime > 0) {
            // For each populated position in the pxTaskStatusArray array, format the raw data as human readable ASCII data
            for (int x = 0; x < uxArraySize; x++) {
                // What percentage of the total run time has the task used? This will always be rounded down to the nearest integer.
                // ulTotalRunTimeDiv100 has already been divided by 100.
                ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;
                memset(tmpBuff, 0x20, 100);
                memcpy(tmpBuff, pxTaskStatusArray[x].pcTaskName, strlen(pxTaskStatusArray[x].pcTaskName));
                tmpBuff[20] = '|';
                int8_t  core = (pxTaskStatusArray[x].xCoreID);
                uint8_t prio = (pxTaskStatusArray[x].uxBasePriority);
                if (ulStatsAsPercentage) {
                    sprintf(tmpBuff + 23, "%12lu  |       %02lu%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, (long unsigned int)ulStatsAsPercentage, core, prio);
                } else {
                    sprintf(tmpBuff + 23, "%12lu  |       <1%%       |%4i  |%5d  |", (long unsigned int)pxTaskStatusArray[x].ulRunTimeCounter, core, prio);
                }
                uint8_t i = 23;
                while (tmpBuff[i] == '0') {
                    tmpBuff[i] = ' ';
                    i++;
                }
                if (tmpBuff[45] == '0') tmpBuff[45] = ' ';
                strcat(pcWriteBuffer, leftSpace);
                strcat(pcWriteBuffer, " ");
                strcat(pcWriteBuffer, tmpBuff);
                strcat(pcWriteBuffer, "\n");
            }
            x_ps_free(&tmpBuff);
        }
        // The array is no longer needed, free the memory it consumes.
        vPortFree(pxTaskStatusArray);

#if TFT_CONTROLLER == 7
        extern uint64_t _totalRuntime;
        tmpBuff = x_ps_malloc(130);
        if (_totalRuntime > 0) {
            sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: %llu", leftSpace, _totalRuntime, tft.getVsyncCounter(), tft.getVsyncCounter() / _totalRuntime);
        } else {
            sprintf(tmpBuff, "%s" ANSI_ESC_LIGHTGREEN " time since start: %llus, VSYNCS: %llu  ==> fps: <1", leftSpace, _totalRuntime, tft.getVsyncCounter());
        }
        strcat(tmpBuff, "                                   ");
        tmpBuff[90] = '\0';
        strcat(tmpBuff, ANSI_ESC_YELLOW "|\n");
        strcat(pcWriteBuffer, tmpBuff);
        x_ps_free(&tmpBuff);
#endif
        strcat(pcWriteBuffer, "             |---------------------+----------------+-----------------+------+-------|\n");
    }
}

const char ir_buttons_json[] = "[{\"A\":\"0x00\",\"label\":\"IR address\"},"
                               "{\"C\":\"0x4a\",\"label\":\"IR command\"},"
                               "{\"0\":\"0x52\",\"label\":\"ZERO\"},"
                               "{\"10\":\"0x42\",\"label\":\"MUTE\"},"
                               "{\"20\":\"0x40\",\"label\":\"SLEEP\"},"
                               "{\"1\":\"0x16\",\"label\":\"ONE\"},"
                               "{\"11\":\"0x43\",\"label\":\"ARROW RIGHT\"},"
                               "{\"21\":\"0x4a\",\"label\":\"CANCEL\"},"
                               "{\"2\":\"0x19\",\"label\":\"TWO\"},"
                               "{\"12\":\"0x44\",\"label\":\"ARROW LEFT\"},"
                               "{\"22\":\"-1\",\"label\":\"-\"},"
                               "{\"3\":\"0x0d\",\"label\":\"THREE\"},"
                               "{\"13\":\"0x15\",\"label\":\"ARROW DOWN\"},"
                               "{\"4\":\"0x0c\",\"label\":\"FOUR\"},"
                               "{\"14\":\"0x46\",\"label\":\"ARROW UP\"},"
                               "{\"5\":\"0x18\",\"label\":\"FIVE\"},"
                               "{\"15\":\"0x4a\",\"label\":\"MODE\"},"
                               "{\"6\":\"0x5e\",\"label\":\"SIX\"},"
                               "{\"16\":\"0x40\",\"label\":\"OK\"},"
                               "{\"7\":\"0x08\",\"label\":\"SEVEN\"},"
                               "{\"17\":\"0x20\",\"label\":\"-\"},"
                               "{\"8\":\"0x1c\",\"label\":\"EIGHT\"},"
                               "{\"18\":\"-1\",\"label\":\"-\"},"
                               "{\"9\":\"0x5a\",\"label\":\"NINE\"},"
                               "{\"19\":\"-1\",\"label\":\"-\"}]";

const char stations_json[] = "[[\"*\",\"DE\",\"0N 70s\",\"http://0n-70s.radionetz.de:8000/0n-70s.mp3\"],"
                             "[\"*\",\"DE\",\"0N 80s\",\"http://0n-80s.radionetz.de:8000/0n-80s.mp3\"],"
                             "[\"*\",\"DE\",\"0N 90s\",\"http://0n-90s.radionetz.de:8000/0n-90s.mp3\"],"
                             "[\"*\",\"DE\",\"0N Charts\",\"http://0n-charts.radionetz.de:8000/0n-charts.mp3\"],"
                             "[\"*\",\"DE\",\"0N Dance\",\"http://0n-dance.radionetz.de:8000/0n-dance.mp3\"],"
                             "[\"*\",\"DE\",\"0N Disco\",\"http://0n-disco.radionetz.de:8000/0n-disco.mp3\"],"
                             "[\"*\",\"DE\",\"1000 Oldies\",\"http://c3.auracast.net:8010/stream\"],"
                             "[\"*\",\"DE\",\"Eurodance\",\"http://www.laut.fm/eurodance\"],"
                             "[\"\",\"DE\",\"extra-radio 88.0\",\"https://www.extra-radio.de/stream/listen.m3u\"],"
                             "[\"*\",\"DE\",\"Hitradio SKW\",\"http://server4.streamserver24.com:2199/tunein/hitradio.asx\"],"
                             "[\"*\",\"DE\",\"MacSlon's Irish Pub Radio\",\"http://macslons-irish-pub-radio.stream.laut.fm/macslons-irish-pub-radio\"],"
                             "[\"\",\"GR\",\"Άνοιξη 100.7\",\"http://solid1.streamupsolutions.com:55023/stream\"],"
                             "[\"\",\"RU\",\"НАШЕ Радио\",\"http://nashe1.hostingradio.ru/nashe-128.mp3\"],"
                             "[\"\",\"RU\",\"Радио Русские Песни\",\"http://listen.rusongs.ru/ru-mp3-128\"],"
                             "[\"\",\"BG\",\"Свежа България\",\"http://31.13.223.148:8000/fresh.mp3\"],"
                             "[\"\",\"CH\",\"SWISS POP\",\"https://stream.srg-ssr.ch/rsp/aacp_48.asx\"],"
                             "[\"\",\"BG\",\"BGRADIO\",\"http://play.global.audio/bgradio_low.ogg\"],"
                             "[\"\",\"DE\",\"knixx.fm\",\"http://s1.knixx.fm:5347/dein_webradio_vbr.opus\"],"
                             "[\"*\",\"DE\",\"- 0 N - Christmas on Radio\",\"https://0n-christmas.radionetz.de/0n-christmas.aac\"],"
                             "[\"*\",\"GB\",\"BBC 6music\",\"http://as-hls-ww-live.akamaized.net/pool_904/live/ww/bbc_6music/bbc_6music.isml/bbc_6music-audio=96000.norewind.m3u8\"],"
                             "[\"\",\"DE\",\"- 0 N - Movies on Radio\",\"https://0n-movies.radionetz.de/0n-movies.mp3\"],"
                             "[\"*\",\"DE\",\"- 0 N - Top 40 on Radio\",\"https://0n-top40.radionetz.de/0n-top40.mp3\"],"
                             "[\"\",\"DE\",\"ROCKANTENNE Alternative (mp3)\",\"https://stream.rockantenne.de/alternative/stream/mp3\"],"
                             "[\"\",\"PL\",\"Gra Wrocław\",\"http://rmfstream2.interia.pl:8000/radio_gra_wroc\"],"
                             "[\"*\",\"RU\",\"Classic EuroDisco Радио\",\"https://live.radiospinner.com/clsscrdsc-96\"],"
                             "[\"*\",\"DE\",\"Hit Radio FFH - Soundtrack (AAC+)\",\"http://streams.ffh.de/ffhchannels/aac/soundtrack.m3u\"]]";

const char timezones_json[] = "[[\"Africa/Accra\",\"GMT0\"],"
                              "[\"Africa/Addis_Ababa\",\"EAT-3\"],"
                              "[\"Africa/Algiers\",\"CET-1\"],"
                              "[\"Africa/Asmara\",\"EAT-3\"],"
                              "[\"Africa/Bamako\",\"GMT0\"],"
                              "[\"Africa/Bangui\",\"WAT-1\"],"
                              "[\"Africa/Banjul\",\"GMT0\"],"
                              "[\"Africa/Bissau\",\"GMT0\"],"
                              "[\"Africa/Blantyre\",\"CAT-2\"],"
                              "[\"Africa/Brazzaville\",\"WAT-1\"],"
                              "[\"Africa/Bujumbura\",\"CAT-2\"],"
                              "[\"Africa/Cairo\",\"EET-2\"],"
                              "[\"Africa/Casablanca\",\"<+01>-1\"],"
                              "[\"Africa/Ceuta\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Africa/Conakry\",\"GMT0\"],"
                              "[\"Africa/Dakar\",\"GMT0\"],"
                              "[\"Africa/Dar_es_Salaam\",\"EAT-3\"],"
                              "[\"Africa/Djibouti\",\"EAT-3\"],"
                              "[\"Africa/Douala\",\"WAT-1\"],"
                              "[\"Africa/El_Aaiun\",\"<+01>-1\"],"
                              "[\"Africa/Freetown\",\"GMT0\"],"
                              "[\"Africa/Gaborone\",\"CAT-2\"],"
                              "[\"Africa/Harare\",\"CAT-2\"],"
                              "[\"Africa/Johannesburg\",\"SAST-2\"],"
                              "[\"Africa/Juba\",\"CAT-2\"],"
                              "[\"Africa/Kampala\",\"EAT-3\"],"
                              "[\"Africa/Khartoum\",\"CAT-2\"],"
                              "[\"Africa/Kigali\",\"CAT-2\"],"
                              "[\"Africa/Kinshasa\",\"WAT-1\"],"
                              "[\"Africa/Lagos\",\"WAT-1\"],"
                              "[\"Africa/Libreville\",\"WAT-1\"],"
                              "[\"Africa/Lome\",\"GMT0\"],"
                              "[\"Africa/Luanda\",\"WAT-1\"],"
                              "[\"Africa/Lubumbashi\",\"CAT-2\"],"
                              "[\"Africa/Lusaka\",\"CAT-2\"],"
                              "[\"Africa/Malabo\",\"WAT-1\"],"
                              "[\"Africa/Maputo\",\"CAT-2\"],"
                              "[\"Africa/Maseru\",\"SAST-2\"],"
                              "[\"Africa/Mbabane\",\"SAST-2\"],"
                              "[\"Africa/Mogadishu\",\"EAT-3\"],"
                              "[\"Africa/Monrovia\",\"GMT0\"],"
                              "[\"Africa/Nairobi\",\"EAT-3\"],"
                              "[\"Africa/Ndjamena\",\"WAT-1\"],"
                              "[\"Africa/Niamey\",\"WAT-1\"],"
                              "[\"Africa/Nouakchott\",\"GMT0\"],"
                              "[\"Africa/Ouagadougou\",\"GMT0\"],"
                              "[\"Africa/Porto-Novo\",\"WAT-1\"],"
                              "[\"Africa/Sao_Tome\",\"GMT0\"],"
                              "[\"Africa/Tripoli\",\"EET-2\"],"
                              "[\"Africa/Tunis\",\"CET-1\"],"
                              "[\"Africa/Windhoek\",\"CAT-2\"],"
                              "[\"America/Adak\",\"HST10HDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Anchorage\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Anguilla\",\"AST4\"],"
                              "[\"America/Antigua\",\"AST4\"],"
                              "[\"America/Araguaina\",\"<-03>3\"],"
                              "[\"America/Argentina/Buenos_Aires\",\"<-03>3\"],"
                              "[\"America/Argentina/Catamarca\",\"<-03>3\"],"
                              "[\"America/Argentina/Cordoba\",\"<-03>3\"],"
                              "[\"America/Argentina/Jujuy\",\"<-03>3\"],"
                              "[\"America/Argentina/La_Rioja\",\"<-03>3\"],"
                              "[\"America/Argentina/Mendoza\",\"<-03>3\"],"
                              "[\"America/Argentina/Rio_Gallegos\",\"<-03>3\"],"
                              "[\"America/Argentina/Salta\",\"<-03>3\"],"
                              "[\"America/Argentina/San_Juan\",\"<-03>3\"],"
                              "[\"America/Argentina/San_Luis\",\"<-03>3\"],"
                              "[\"America/Argentina/Tucuman\",\"<-03>3\"],"
                              "[\"America/Argentina/Ushuaia\",\"<-03>3\"],"
                              "[\"America/Aruba\",\"AST4\"],"
                              "[\"America/Asuncion\",\"<-04>4<-03>,M10.1.0/0,M3.4.0/0\"],"
                              "[\"America/Atikokan\",\"EST5\"],"
                              "[\"America/Bahia\",\"<-03>3\"],"
                              "[\"America/Bahia_Banderas\",\"CST6\"],"
                              "[\"America/Barbados\",\"AST4\"],"
                              "[\"America/Belem\",\"<-03>3\"],"
                              "[\"America/Belize\",\"CST6\"],"
                              "[\"America/Blanc-Sablon\",\"AST4\"],"
                              "[\"America/Boa_Vista\",\"<-04>4\"],"
                              "[\"America/Bogota\",\"<-05>5\"],"
                              "[\"America/Boise\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Cambridge_Bay\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Campo_Grande\",\"<-04>4\"],"
                              "[\"America/Cancun\",\"EST5\"],"
                              "[\"America/Caracas\",\"<-04>4\"],"
                              "[\"America/Cayenne\",\"<-03>3\"],"
                              "[\"America/Cayman\",\"EST5\"],"
                              "[\"America/Chicago\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Chihuahua\",\"CST6\"],"
                              "[\"America/Costa_Rica\",\"CST6\"],"
                              "[\"America/Creston\",\"MST7\"],"
                              "[\"America/Cuiaba\",\"<-04>4\"],"
                              "[\"America/Curacao\",\"AST4\"],"
                              "[\"America/Danmarkshavn\",\"GMT0\"],"
                              "[\"America/Dawson\",\"MST7\"],"
                              "[\"America/Dawson_Creek\",\"MST7\"],"
                              "[\"America/Denver\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Detroit\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Dominica\",\"AST4\"],"
                              "[\"America/Edmonton\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Eirunepe\",\"<-05>5\"],"
                              "[\"America/El_Salvador\",\"CST6\"],"
                              "[\"America/Fortaleza\",\"<-03>3\"],"
                              "[\"America/Fort_Nelson\",\"MST7\"],"
                              "[\"America/Glace_Bay\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Godthab\",\"<-02>2\"],"
                              "[\"America/Goose_Bay\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Grand_Turk\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Grenada\",\"AST4\"],"
                              "[\"America/Guadeloupe\",\"AST4\"],"
                              "[\"America/Guatemala\",\"CST6\"],"
                              "[\"America/Guayaquil\",\"<-05>5\"],"
                              "[\"America/Guyana\",\"<-04>4\"],"
                              "[\"America/Halifax\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Havana\",\"CST5CDT,M3.2.0/0,M11.1.0/1\"],"
                              "[\"America/Hermosillo\",\"MST7\"],"
                              "[\"America/Indiana/Indianapolis\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Knox\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Marengo\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Petersburg\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Tell_City\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Vevay\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Vincennes\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Indiana/Winamac\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Inuvik\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Iqaluit\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Jamaica\",\"EST5\"],"
                              "[\"America/Juneau\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kentucky/Louisville\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kentucky/Monticello\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Kralendijk\",\"AST4\"],"
                              "[\"America/La_Paz\",\"<-04>4\"],"
                              "[\"America/Lima\",\"<-05>5\"],"
                              "[\"America/Los_Angeles\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Lower_Princes\",\"AST4\"],"
                              "[\"America/Maceio\",\"<-03>3\"],"
                              "[\"America/Managua\",\"CST6\"],"
                              "[\"America/Manaus\",\"<-04>4\"],"
                              "[\"America/Marigot\",\"AST4\"],"
                              "[\"America/Martinique\",\"AST4\"],"
                              "[\"America/Matamoros\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Mazatlan\",\"MST7\"],"
                              "[\"America/Menominee\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Merida\",\"CST6\"],"
                              "[\"America/Metlakatla\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Mexico_City\",\"CST6\"],"
                              "[\"America/Miquelon\",\"<-03>3<-02>,M3.2.0,M11.1.0\"],"
                              "[\"America/Moncton\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Monterrey\",\"CST6\"],"
                              "[\"America/Montevideo\",\"<-03>3\"],"
                              "[\"America/Montreal\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Montserrat\",\"AST4\"],"
                              "[\"America/Nassau\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/New_York\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nipigon\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nome\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Noronha\",\"<-02>2\"],"
                              "[\"America/North_Dakota/Beulah\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/North_Dakota/Center\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/North_Dakota/New_Salem\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Nuuk\",\"<-02>2\"],"
                              "[\"America/Ojinaga\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Panama\",\"EST5\"],"
                              "[\"America/Pangnirtung\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Paramaribo\",\"<-03>3\"],"
                              "[\"America/Phoenix\",\"MST7\"],"
                              "[\"America/Port-au-Prince\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Port_of_Spain\",\"AST4\"],"
                              "[\"America/Porto_Velho\",\"<-04>4\"],"
                              "[\"America/Puerto_Rico\",\"AST4\"],"
                              "[\"America/Punta_Arenas\",\"<-03>3\"],"
                              "[\"America/Rainy_River\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Rankin_Inlet\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Recife\",\"<-03>3\"],"
                              "[\"America/Regina\",\"CST6\"],"
                              "[\"America/Resolute\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Rio_Branco\",\"<-05>5\"],"
                              "[\"America/Santarem\",\"<-03>3\"],"
                              "[\"America/Santiago\",\"<-04>4<-03>,M9.1.6/24,M4.1.6/24\"],"
                              "[\"America/Santo_Domingo\",\"AST4\"],"
                              "[\"America/Sao_Paulo\",\"<-03>3\"],"
                              "[\"America/Scoresbysund\",\"<-01>1<+00>,M3.5.0/0,M10.5.0/1\"],"
                              "[\"America/Sitka\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/St_Barthelemy\",\"AST4\"],"
                              "[\"America/St_Johns\",\"NST3:30NDT,M3.2.0,M11.1.0\"],"
                              "[\"America/St_Kitts\",\"AST4\"],"
                              "[\"America/St_Lucia\",\"AST4\"],"
                              "[\"America/St_Thomas\",\"AST4\"],"
                              "[\"America/St_Vincent\",\"AST4\"],"
                              "[\"America/Swift_Current\",\"CST6\"],"
                              "[\"America/Tegucigalpa\",\"CST6\"],"
                              "[\"America/Thule\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"America/Thunder_Bay\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Tijuana\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Toronto\",\"EST5EDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Tortola\",\"AST4\"],"
                              "[\"America/Vancouver\",\"PST8PDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Whitehorse\",\"MST7\"],"
                              "[\"America/Winnipeg\",\"CST6CDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Yakutat\",\"AKST9AKDT,M3.2.0,M11.1.0\"],"
                              "[\"America/Yellowknife\",\"MST7MDT,M3.2.0,M11.1.0\"],"
                              "[\"Antarctica/Casey\",\"<+11>-11\"],"
                              "[\"Antarctica/Davis\",\"<+07>-7\"],"
                              "[\"Antarctica/DumontDUrville\",\"<+10>-10\"],"
                              "[\"Antarctica/Macquarie\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Antarctica/Mawson\",\"<+05>-5\"],"
                              "[\"Antarctica/McMurdo\",\"NZST-12NZDT,M9.5.0,M4.1.0/3\"],"
                              "[\"Antarctica/Palmer\",\"<-03>3\"],"
                              "[\"Antarctica/Rothera\",\"<-03>3\"],"
                              "[\"Antarctica/Syowa\",\"<+03>-3\"],"
                              "[\"Antarctica/Troll\",\"<+00>0<+02>-2,M3.5.0/1,M10.5.0/3\"],"
                              "[\"Antarctica/Vostok\",\"<+06>-6\"],"
                              "[\"Arctic/Longyearbyen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Asia/Aden\",\"<+03>-3\"],"
                              "[\"Asia/Almaty\",\"<+06>-6\"],"
                              "[\"Asia/Amman\",\"<+03>-3\"],"
                              "[\"Asia/Anadyr\",\"<+12>-12\"],"
                              "[\"Asia/Aqtau\",\"<+05>-5\"],"
                              "[\"Asia/Aqtobe\",\"<+05>-5\"],"
                              "[\"Asia/Ashgabat\",\"<+05>-5\"],"
                              "[\"Asia/Atyrau\",\"<+05>-5\"],"
                              "[\"Asia/Baghdad\",\"<+03>-3\"],"
                              "[\"Asia/Bahrain\",\"<+03>-3\"],"
                              "[\"Asia/Baku\",\"<+04>-4\"],"
                              "[\"Asia/Bangkok\",\"<+07>-7\"],"
                              "[\"Asia/Barnaul\",\"<+07>-7\"],"
                              "[\"Asia/Beirut\",\"EET-2EEST,M3.5.0/0,M10.5.0/0\"],"
                              "[\"Asia/Bishkek\",\"<+06>-6\"],"
                              "[\"Asia/Brunei\",\"<+08>-8\"],"
                              "[\"Asia/Chita\",\"<+09>-9\"],"
                              "[\"Asia/Choibalsan\",\"<+08>-8\"],"
                              "[\"Asia/Colombo\",\"<+0530>-5:30\"],"
                              "[\"Asia/Damascus\",\"<+03>-3\"],"
                              "[\"Asia/Dhaka\",\"<+06>-6\"],"
                              "[\"Asia/Dili\",\"<+09>-9\"],"
                              "[\"Asia/Dubai\",\"<+04>-4\"],"
                              "[\"Asia/Dushanbe\",\"<+05>-5\"],"
                              "[\"Asia/Famagusta\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Asia/Gaza\",\"EET-2EEST,M3.4.4/50,M10.4.4/50\"],"
                              "[\"Asia/Hebron\",\"EET-2EEST,M3.4.4/50,M10.4.4/50\"],"
                              "[\"Asia/Ho_Chi_Minh\",\"<+07>-7\"],"
                              "[\"Asia/Hong_Kong\",\"HKT-8\"],"
                              "[\"Asia/Hovd\",\"<+07>-7\"],"
                              "[\"Asia/Irkutsk\",\"<+08>-8\"],"
                              "[\"Asia/Jakarta\",\"WIB-7\"],"
                              "[\"Asia/Jayapura\",\"WIT-9\"],"
                              "[\"Asia/Jerusalem\",\"IST-2IDT,M3.4.4/26,M10.5.0\"],"
                              "[\"Asia/Kabul\",\"<+0430>-4:30\"],"
                              "[\"Asia/Kamchatka\",\"<+12>-12\"],"
                              "[\"Asia/Karachi\",\"PKT-5\"],"
                              "[\"Asia/Kathmandu\",\"<+0545>-5:45\"],"
                              "[\"Asia/Khandyga\",\"<+09>-9\"],"
                              "[\"Asia/Kolkata\",\"IST-5:30\"],"
                              "[\"Asia/Krasnoyarsk\",\"<+07>-7\"],"
                              "[\"Asia/Kuala_Lumpur\",\"<+08>-8\"],"
                              "[\"Asia/Kuching\",\"<+08>-8\"],"
                              "[\"Asia/Kuwait\",\"<+03>-3\"],"
                              "[\"Asia/Macau\",\"CST-8\"],"
                              "[\"Asia/Magadan\",\"<+11>-11\"],"
                              "[\"Asia/Makassar\",\"WITA-8\"],"
                              "[\"Asia/Manila\",\"PST-8\"],"
                              "[\"Asia/Muscat\",\"<+04>-4\"],"
                              "[\"Asia/Nicosia\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Asia/Novokuznetsk\",\"<+07>-7\"],"
                              "[\"Asia/Novosibirsk\",\"<+07>-7\"],"
                              "[\"Asia/Omsk\",\"<+06>-6\"],"
                              "[\"Asia/Oral\",\"<+05>-5\"],"
                              "[\"Asia/Phnom_Penh\",\"<+07>-7\"],"
                              "[\"Asia/Pontianak\",\"WIB-7\"],"
                              "[\"Asia/Pyongyang\",\"KST-9\"],"
                              "[\"Asia/Qatar\",\"<+03>-3\"],"
                              "[\"Asia/Qyzylorda\",\"<+05>-5\"],"
                              "[\"Asia/Riyadh\",\"<+03>-3\"],"
                              "[\"Asia/Sakhalin\",\"<+11>-11\"],"
                              "[\"Asia/Samarkand\",\"<+05>-5\"],"
                              "[\"Asia/Seoul\",\"KST-9\"],"
                              "[\"Asia/Shanghai\",\"CST-8\"],"
                              "[\"Asia/Singapore\",\"<+08>-8\"],"
                              "[\"Asia/Srednekolymsk\",\"<+11>-11\"],"
                              "[\"Asia/Taipei\",\"CST-8\"],"
                              "[\"Asia/Tashkent\",\"<+05>-5\"],"
                              "[\"Asia/Tbilisi\",\"<+04>-4\"],"
                              "[\"Asia/Tehran\",\"<+0330>-3:30\"],"
                              "[\"Asia/Thimphu\",\"<+06>-6\"],"
                              "[\"Asia/Tokyo\",\"JST-9\"],"
                              "[\"Asia/Tomsk\",\"<+07>-7\"],"
                              "[\"Asia/Ulaanbaatar\",\"<+08>-8\"],"
                              "[\"Asia/Urumqi\",\"<+06>-6\"],"
                              "[\"Asia/Ust-Nera\",\"<+10>-10\"],"
                              "[\"Asia/Vientiane\",\"<+07>-7\"],"
                              "[\"Asia/Vladivostok\",\"<+10>-10\"],"
                              "[\"Asia/Yakutsk\",\"<+09>-9\"],"
                              "[\"Asia/Yangon\",\"<+0630>-6:30\"],"
                              "[\"Asia/Yekaterinburg\",\"<+05>-5\"],"
                              "[\"Asia/Yerevan\",\"<+04>-4\"],"
                              "[\"Atlantic/Azores\",\"<-01>1<+00>,M3.5.0/0,M10.5.0/1\"],"
                              "[\"Atlantic/Bermuda\",\"AST4ADT,M3.2.0,M11.1.0\"],"
                              "[\"Atlantic/Canary\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Cape_Verde\",\"<-01>1\"],"
                              "[\"Atlantic/Faroe\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Madeira\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Atlantic/Reykjavik\",\"GMT0\"],"
                              "[\"Atlantic/South_Georgia\",\"<-02>2\"],"
                              "[\"Atlantic/Stanley\",\"<-03>3\"],"
                              "[\"Atlantic/St_Helena\",\"GMT0\"],"
                              "[\"Australia/Adelaide\",\"ACST-9:30ACDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Brisbane\",\"AEST-10\"],"
                              "[\"Australia/Broken_Hill\",\"ACST-9:30ACDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Currie\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Darwin\",\"ACST-9:30\"],"
                              "[\"Australia/Eucla\",\"<+0845>-8:45\"],"
                              "[\"Australia/Hobart\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Lindeman\",\"AEST-10\"],"
                              "[\"Australia/Lord_Howe\",\"<+1030>-10:30<+11>-11,M10.1.0,M4.1.0\"],"
                              "[\"Australia/Melbourne\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Australia/Perth\",\"AWST-8\"],"
                              "[\"Australia/Sydney\",\"AEST-10AEDT,M10.1.0,M4.1.0/3\"],"
                              "[\"Europe/Amsterdam\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Andorra\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Astrakhan\",\"<+04>-4\"],"
                              "[\"Europe/Athens\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Belgrade\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Berlin\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Bratislava\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Brussels\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Bucharest\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Budapest\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Busingen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Chisinau\",\"EET-2EEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Copenhagen\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Dublin\",\"IST-1GMT0,M10.5.0,M3.5.0/1\"],"
                              "[\"Europe/Gibraltar\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Guernsey\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Helsinki\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Isle_of_Man\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Istanbul\",\"<+03>-3\"],"
                              "[\"Europe/Jersey\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Kaliningrad\",\"EET-2\"],"
                              "[\"Europe/Kiev\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Kirov\",\"<+03>-3\"],"
                              "[\"Europe/Lisbon\",\"WET0WEST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Ljubljana\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/London\",\"GMT0BST,M3.5.0/1,M10.5.0\"],"
                              "[\"Europe/Luxembourg\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Madrid\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Malta\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Mariehamn\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Minsk\",\"<+03>-3\"],"
                              "[\"Europe/Monaco\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Moscow\",\"MSK-3\"],"
                              "[\"Europe/Oslo\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Paris\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Podgorica\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Prague\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Riga\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Rome\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Samara\",\"<+04>-4\"],"
                              "[\"Europe/San_Marino\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Sarajevo\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Saratov\",\"<+04>-4\"],"
                              "[\"Europe/Simferopol\",\"MSK-3\"],"
                              "[\"Europe/Skopje\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Sofia\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Stockholm\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Tallinn\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Tirane\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Ulyanovsk\",\"<+04>-4\"],"
                              "[\"Europe/Uzhgorod\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Vaduz\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vatican\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vienna\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Vilnius\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Volgograd\",\"<+03>-3\"],"
                              "[\"Europe/Warsaw\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Zagreb\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Europe/Zaporozhye\",\"EET-2EEST,M3.5.0/3,M10.5.0/4\"],"
                              "[\"Europe/Zurich\",\"CET-1CEST,M3.5.0,M10.5.0/3\"],"
                              "[\"Indian/Antananarivo\",\"EAT-3\"],"
                              "[\"Indian/Chagos\",\"<+06>-6\"],"
                              "[\"Indian/Christmas\",\"<+07>-7\"],"
                              "[\"Indian/Cocos\",\"<+0630>-6:30\"],"
                              "[\"Indian/Comoro\",\"EAT-3\"],"
                              "[\"Indian/Kerguelen\",\"<+05>-5\"],"
                              "[\"Indian/Mahe\",\"<+04>-4\"],"
                              "[\"Indian/Maldives\",\"<+05>-5\"],"
                              "[\"Indian/Mauritius\",\"<+04>-4\"],"
                              "[\"Indian/Mayotte\",\"EAT-3\"],"
                              "[\"Indian/Reunion\",\"<+04>-4\"],"
                              "[\"Pacific/Apia\",\"<+13>-13\"],"
                              "[\"Pacific/Auckland\",\"NZST-12NZDT,M9.5.0,M4.1.0/3\"],"
                              "[\"Pacific/Bougainville\",\"<+11>-11\"],"
                              "[\"Pacific/Chatham\",\"<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45\"],"
                              "[\"Pacific/Chuuk\",\"<+10>-10\"],"
                              "[\"Pacific/Easter\",\"<-06>6<-05>,M9.1.6/22,M4.1.6/22\"],"
                              "[\"Pacific/Efate\",\"<+11>-11\"],"
                              "[\"Pacific/Enderbury\",\"<+13>-13\"],"
                              "[\"Pacific/Fakaofo\",\"<+13>-13\"],"
                              "[\"Pacific/Fiji\",\"<+12>-12\"],"
                              "[\"Pacific/Funafuti\",\"<+12>-12\"],"
                              "[\"Pacific/Galapagos\",\"<-06>6\"],"
                              "[\"Pacific/Gambier\",\"<-09>9\"],"
                              "[\"Pacific/Guadalcanal\",\"<+11>-11\"],"
                              "[\"Pacific/Guam\",\"ChST-10\"],"
                              "[\"Pacific/Honolulu\",\"HST10\"],"
                              "[\"Pacific/Kiritimati\",\"<+14>-14\"],"
                              "[\"Pacific/Kosrae\",\"<+11>-11\"],"
                              "[\"Pacific/Kwajalein\",\"<+12>-12\"],"
                              "[\"Pacific/Majuro\",\"<+12>-12\"],"
                              "[\"Pacific/Marquesas\",\"<-0930>9:30\"],"
                              "[\"Pacific/Midway\",\"SST11\"],"
                              "[\"Pacific/Nauru\",\"<+12>-12\"],"
                              "[\"Pacific/Niue\",\"<-11>11\"],"
                              "[\"Pacific/Norfolk\",\"<+11>-11<+12>,M10.1.0,M4.1.0/3\"],"
                              "[\"Pacific/Noumea\",\"<+11>-11\"],"
                              "[\"Pacific/Pago_Pago\",\"SST11\"],"
                              "[\"Pacific/Palau\",\"<+09>-9\"],"
                              "[\"Pacific/Pitcairn\",\"<-08>8\"],"
                              "[\"Pacific/Pohnpei\",\"<+11>-11\"],"
                              "[\"Pacific/Port_Moresby\",\"<+10>-10\"],"
                              "[\"Pacific/Rarotonga\",\"<-10>10\"],"
                              "[\"Pacific/Saipan\",\"ChST-10\"],"
                              "[\"Pacific/Tahiti\",\"<-10>10\"],"
                              "[\"Pacific/Tarawa\",\"<+12>-12\"],"
                              "[\"Pacific/Tongatapu\",\"<+13>-13\"],"
                              "[\"Pacific/Wake\",\"<+12>-12\"],"
                              "[\"Pacific/Wallis\",\"<+12>-12\"],"
                              "[\"Etc/GMT\",\"GMT0\"],"
                              "[\"Etc/GMT-0\",\"GMT0\"],"
                              "[\"Etc/GMT-1\",\"<+01>-1\"],"
                              "[\"Etc/GMT-2\",\"<+02>-2\"],"
                              "[\"Etc/GMT-3\",\"<+03>-3\"],"
                              "[\"Etc/GMT-4\",\"<+04>-4\"],"
                              "[\"Etc/GMT-5\",\"<+05>-5\"],"
                              "[\"Etc/GMT-6\",\"<+06>-6\"],"
                              "[\"Etc/GMT-7\",\"<+07>-7\"],"
                              "[\"Etc/GMT-8\",\"<+08>-8\"],"
                              "[\"Etc/GMT-9\",\"<+09>-9\"],"
                              "[\"Etc/GMT-10\",\"<+10>-10\"],"
                              "[\"Etc/GMT-11\",\"<+11>-11\"],"
                              "[\"Etc/GMT-12\",\"<+12>-12\"],"
                              "[\"Etc/GMT-13\",\"<+13>-13\"],"
                              "[\"Etc/GMT-14\",\"<+14>-14\"],"
                              "[\"Etc/GMT0\",\"GMT0\"],"
                              "[\"Etc/GMT+0\",\"GMT0\"],"
                              "[\"Etc/GMT+1\",\"<-01>1\"],"
                              "[\"Etc/GMT+2\",\"<-02>2\"],"
                              "[\"Etc/GMT+3\",\"<-03>3\"],"
                              "[\"Etc/GMT+4\",\"<-04>4\"],"
                              "[\"Etc/GMT+5\",\"<-05>5\"],"
                              "[\"Etc/GMT+6\",\"<-06>6\"],"
                              "[\"Etc/GMT+7\",\"<-07>7\"],"
                              "[\"Etc/GMT+8\",\"<-08>8\"],"
                              "[\"Etc/GMT+9\",\"<-09>9\"],"
                              "[\"Etc/GMT+10\",\"<-10>10\"],"
                              "[\"Etc/GMT+11\",\"<-11>11\"],"
                              "[\"Etc/GMT+12\",\"<-12>12\"],"
                              "[\"Etc/UCT\",\"UTC0\"],"
                              "[\"Etc/UTC\",\"UTC0\"],"
                              "[\"Etc/Greenwich\",\"GMT0\"],"
                              "[\"Etc/Universal\",\"UTC0\"],"
                              "[\"Etc/Zulu\",\"UTC0\"]]";

const char aesKey[] = "mysecretkey12345";

const char voice_time_de[24][50] = {"Beim dritten Ton ist es genau Mitternacht",        "Beim dritten Ton ist es genau ein Uhr",
                                    "Beim dritten Ton ist es genau zwei Uhr",           "Beim dritten Ton ist es genau drei Uhr",
                                    "Beim dritten Ton ist es genau vier Uhr",           "Beim dritten Ton ist es genau fünf Uhr",
                                    "Beim dritten Ton ist es genau sechs Uhr",          "Beim dritten Ton ist es genau sieben Uhr",
                                    "Beim dritten Ton ist es genau acht Uhr",           "Beim dritten Ton ist es genau neun Uhr",
                                    "Beim dritten Ton ist es genau zehn Uhr",           "Beim dritten Ton ist es genau elf Uhr",
                                    "Beim dritten Ton ist es genau zwölf Uhr",          "Beim dritten Ton ist es genau dreizehn Uhr",
                                    "Beim dritten Ton ist es genau vierzehn Uhr",       "Beim dritten Ton ist es genau fünfzehn Uhr",
                                    "Beim dritten Ton ist es genau sechszehn Uhr",      "Beim dritten Ton ist es genau siebzehn Uhr",
                                    "Beim dritten Ton ist es genau achtzehn Uhr",       "Beim dritten Ton ist es genau neunzehn Uhr",
                                    "Beim dritten Ton ist es genau zwanzig Uhr",        "Beim dritten Ton ist es genau einundzwanzig Uhr",
                                    "Beim dritten Ton ist es genau zweiundzwanzig Uhr", "Beim dritten Ton ist es genau dreiundzwanzig Uhr"};

inline const char* aes_encrypt(const char* input) {
    static char* output = NULL;
    uint16_t     len = strlen(input) / 16;
    len++;
    x_ps_free(&output);
    output = (char*)x_ps_calloc((len * 16) + 1, 1);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char*)aesKey, 128);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, (const unsigned char*)input, (unsigned char*)output);
    mbedtls_aes_free(&aes);
    return output;
}

inline const char* aes_decrypt(const char* input) {
    static char* output = NULL;
    uint16_t     len = strlen(input) + 1;
    x_ps_free(&output);
    output = (char*)x_ps_calloc(len, 1);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, (const unsigned char*)aesKey, 128);
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, (const unsigned char*)input, (unsigned char*)output);
    mbedtls_aes_free(&aes);
    return output;
}

inline void encode_base64(unsigned char* input, size_t input_len) {
    // Buffer-Größe berechnen für Base64-Kodierung
    size_t         buffer_size = ((input_len + 2) / 3) * 4 + 1;
    unsigned char* output = (unsigned char*)malloc(buffer_size); // Dynamischer Buffer

    size_t output_len;
    int    ret = mbedtls_base64_encode(output, buffer_size, &output_len, input, input_len);

    if (ret == 0) {
        printf("Base64-Kodierung: %s\n", output);
    } else {
        printf("Fehler bei der Base64-Kodierung\n");
    }

    free(output); // Buffer freigeben
}

inline void decode_base64(unsigned char* input, size_t input_len) {
    size_t output_len;
    // Buffer-Größe berechnen für Base64-Dekodierung
    size_t         buffer_size = (input_len / 4) * 3 + 1;
    unsigned char* output = (unsigned char*)malloc(buffer_size); // Dynamischer Buffer

    // Dekodierung
    int ret = mbedtls_base64_decode((uint8_t*)output, sizeof(output), &output_len, (const unsigned char*)input, input_len);

    if (ret == 0) {
        printf("Dekodierter Text: %s\n", output);
    } else {
        printf("Fehler bei der Base64-Dekodierung\n");
    }
    free(output); // Buffer freigeben
}
// Macro for comfortable calls
#define MWR_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define MWR_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
