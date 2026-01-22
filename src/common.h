// created: 10.02.2022
// updated: 23.11.2025

#pragma once

#include "Audio.h"
#include "BH1750.h"
#include "DLNAClient.h"
#include "ESP32FtpServer.h"
#include "IR.h"
#include "SPIFFS.h"
#include "base64.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "kcx_bt_emitter.h"
#include "mbedtls/sha1.h"
#include "rtime.h"
#include "settings.h"

#if TFT_CONTROLLER < 7
#include "tft_spi.h"
#elif TFT_CONTROLLER == 7
#include "tft_rgb.h"
#elif TFT_CONTROLLER == 8
#include "tft_dsi.h"
#endif

#include "tp_ft6x36.h"
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
    UNDEFINED = -1
};

enum ir_shift {
    IR_RIGHT = +100,
    IR_LEFT = -100,
    IR_UP = +101,
    IR_DOWN = - 101,
    IR_RESET = -127
};

static bool                     newLine = false;
extern SemaphoreHandle_t        mutex_rtc;
extern RTIME                    rtc;
extern WebSrv                   webSrv;
extern std::deque<ps_ptr<char>> s_logBuffer;

void SerialPrintfln(const char* fmt, ...) {
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
    const char* arg1 = NULL;
    const char* arg2 = NULL;
    ps_ptr<char> arg3 = "";
    int16_t     val1 = 0;
    int16_t     val2 = 0;
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
    ps_ptr<char> mode = {};
    ps_ptr<char> version = {};
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
void         fall_asleep();
void         wake_up();
void         setRTC(ps_ptr<char> TZString);
boolean      isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime);
boolean      copySDtoFFat(const char* path);
void         showStreamTitle(ps_ptr<char> streamTitle);
void         showLogoAndStationName(bool force);
void         showFileLogo(int8_t state, int8_t subState);
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
void         processPlaylist();
void         changeState(int8_t state, int8_t subState);
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
void         tp_pressed(uint16_t x, uint16_t y);
void         tp_long_pressed(uint16_t x, uint16_t y);
void         tp_moved(uint16_t x, uint16_t y);
void         tp_released(uint16_t x, uint16_t y);

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
    const char* pos = strstr(p, needle);
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
    int   count = 0;
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
inline int32_t clamp_min_max(int32_t val, int32_t min, int32_t max) {
    if (min >= max) {
        log_e("min >= max, min: %i, max %i", min, max);
        return val;
    }
    if (val < min) val = min;
    if (val > max) val = max;
    return val;
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

inline void setupBacklight(int pin, uint32_t freq_hz) {

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
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}

inline void setTFTbrightness(uint8_t duty) {
    if (TFT_BL >= 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }
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
#elif TFT_CONTROLLER == 7
extern TFT_RGB tft;
#elif TFT_CONTROLLER == 8
extern TFT_DSI tft;
#else
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
