// created: 10.Feb.2022
// updated: 06.Aug 2024

#pragma once
#pragma GCC optimize("Os") // optimize for code size
// clang-format off
#define _SSID                   "mySSID"                        // Your WiFi credentials here
#define _PW                     "myWiFiPassword"                // Or in textfile on SD-card
#define DECODER                 1                               // (1)MAX98357A PCM5102A CS4344... (2)AC101, (3)ES8388
#define TFT_CONTROLLER          5                               // (0)ILI9341, (1)HX8347D, (2)ILI9486a, (3)ILI9486b, (4)ILI9488, (5)ST7796, (6)ST7796RPI
#define DISPLAY_INVERSION       0                               // (0) off (1) on
#define TFT_ROTATION            1                               // 1 or 3 (landscape)
#define TFT_FREQUENCY           40000000                        // 80000000, 40000000, 27000000, 20000000, 10000000
#define TP_VERSION              5                               // (0)ILI9341, (1)ILI9341RPI, (2)HX8347D, (3)ILI9486, (4)ILI9488, (5)ST7796, (3)ST7796RPI
#define TP_ROTATION             1                               // 1 or 3 (landscape)
#define TP_H_MIRROR             0                               // (0) default, (1) mirror up <-> down
#define TP_V_MIRROR             0                               // (0) default, (1) mittor left <-> right
#define I2S_COMM_FMT            0                               // (0) MAX98357A PCM5102A CS4344, (1) LSBJ (Least Significant Bit Justified format) PT8211
#define SDMMC_FREQUENCY         80000000                        // 80000000 or 40000000 MHz
#define FTP_USERNAME            "esp32"                         // user and pw in FTP Client
#define FTP_PASSWORD            "esp32"
#define CONN_TIMEOUT            2500                            // unencrypted connection timeout in ms (http://...)
#define CONN_TIMEOUT_SSL        3500                            // encrypted connection timeout in ms (https://...)

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#include <Arduino.h>
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
#include "accesspoint.h"
#include "websrv.h"
#include "rtime.h"
#include "IR.h"
#include "tft.h"
#include "SPIFFS.h"
#include "ESP32FtpServer.h"
#include "AC101.h"
#include "ES8388.h"
#include "DLNAClient.h"
#include "KCX_BT_Emitter.h"
#include "BH1750.h"
#include <freertos/task.h>

#ifdef CONFIG_IDF_TARGET_ESP32
    // Digital I/O used
        #define TFT_CS             22
        #define TFT_DC             21
        #define TFT_BL             12  // at -1 the brightness menu is not displayed
        #define TP_IRQ             39  // VN
        #define TP_CS               5
        #define SD_MMC_D0           2  // cannot be changed
        #define SD_MMC_CLK         14  // cannot be changed
        #define SD_MMC_CMD         15  // cannot be changed
        #define IR_PIN             35  // IR Receiver (if available)
        #define TFT_MOSI           23  // TFT and TP (VSPI)
        #define TFT_MISO           19  // TFT and TP (VSPI)
        #define TFT_SCK            18  // TFT and TP (VSPI)

        #define I2S_DOUT           25
        #define I2S_BCLK           27
        #define I2S_LRC            26
        #define I2S_MCLK            0  // mostly not used

        #define I2C_DAC_SDA        -1  // some DACs are controlled via I2C
        #define I2C_DAC_SCL        -1
        #define SD_DETECT          -1  // some pins on special boards: Lyra, Olimex, A1S ...
        #define HP_DETECT          -1
        #define AMP_ENABLED        -1

        #define BT_EMITTER_RX      33  // TX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_TX      36  // RX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_LINK    34  // high if connected                     (-1 if not available)
        #define BT_EMITTER_MODE    13  // high transmit - low receive           (-1 if not available)
        #define BT_EMITTER_CONNECT 32  // -1 if not used

        #define I2C_SDA            -1  // I2C, dala line for additional HW
        #define I2C_SCL            -1  // I2C, clock line for additional HW
#endif

#ifdef CONFIG_IDF_TARGET_ESP32S3
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

        #define I2C_DAC_SDA        -1  // some DACs are controlled via I2C
        #define I2C_DAC_SCL        -1
        #define SD_DETECT          -1  // some pins on special boards: Lyra, Olimex, A1S ...
        #define HP_DETECT          -1
        #define AMP_ENABLED        -1

        #define BT_EMITTER_RX      45  // TX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_TX      38  // RX pin - KCX Bluetooth Transmitter    (-1 if not available)
        #define BT_EMITTER_LINK    19  // high if connected                     (-1 if not available)
        #define BT_EMITTER_MODE    20  // high transmit - low receive           (-1 if not available)
        #define BT_EMITTER_CONNECT 48  // high impulse -> awake after POWER_OFF (-1 if not available)

        #define I2C_SDA            41  // I2C, dala line for additional HW
        #define I2C_SCL            42  // I2C, clock line for additional HW
#endif

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// output on serial terminal
#define ANSI_ESC_BLACK      "\033[30m"
#define ANSI_ESC_RED        "\033[31m"
#define ANSI_ESC_GREEN      "\033[32m"
#define ANSI_ESC_YELLOW     "\033[33m"
#define ANSI_ESC_BLUE       "\033[34m"
#define ANSI_ESC_MAGENTA    "\033[35m"
#define ANSI_ESC_CYAN       "\033[36m"
#define ANSI_ESC_WHITE      "\033[37m"
#define ANSI_ESC_BROWN      "\033[38;5;130m"
#define ANSI_ESC_ORANGE     "\033[38;5;214m"

#define ANSI_ESC_RESET      "\033[0m"
#define ANSI_ESC_BOLD       "\033[1m"
#define ANSI_ESC_FAINT      "\033[2m"
#define ANSI_ESC_ITALIC     "\033[3m"
#define ANSI_ESC_UNDERLINE  "\033[4m"

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
static bool _newLine = false;
extern SemaphoreHandle_t mutex_rtc;
extern RTIME rtc;
#define SerialPrintfln(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
 /* line feed */            if(_newLine){_newLine = false; Serial.println("");} \
                            rtc.hasValidTime()? Serial.printf("%s ", rtc.gettime_s()) : Serial.printf("00:00:00 "); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.printf("\033[0m"); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

#define SerialPrintfcr(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
 /* carriage return */      rtc.hasValidTime()? Serial.printf("%s ", rtc.gettime_s()) : Serial.printf("00:00:00 "); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.printf("\033[0m"); \
                            Serial.print("  \r"); \
                            _newLine = true; \
                            xSemaphoreGive(mutex_rtc);}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
struct dlnaHistory {
    char*  objId = NULL;
    char*   name = NULL;
};
struct releasedArg {
    char*   arg1 = NULL;
    char*   arg2 = NULL;
    int16_t val1 = 0;
    int16_t val2 = 0;
};
struct timecounter {
    uint8_t timer = 0;
    uint8_t factor = 2;
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// clang-format on
//prototypes (main.cpp)
boolean        defaultsettings();
boolean        saveStationsToNVS();
boolean        saveDefaultIRbuttonsToNVS();
void           saveIRbuttonsToNVS();
void           loadIRbuttonsFromNVS();
void           updateSettings();
void           urldecode(char* str);
const char*    SD_stringifyDirContent(String path);
void           setTFTbrightness(uint8_t duty);
void           fall_asleep();
void           wake_up();
void           setRTC(const char* TZString);
boolean        copySDtoFFat(const char* path);
void           display_info(const char* str, int32_t xPos, int32_t yPos, uint16_t color, uint16_t margin_l, uint16_t margin_r, uint16_t winWidth, uint16_t winHeight);
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
void           openAccessPoint();
const char*    scaleImage(const char* path);
void           setVolume(uint8_t vol);
uint8_t        downvolume();
uint8_t        upvolume();
void           setStation(uint16_t sta);
void           nextStation();
void           prevStation();
void           StationsItems();
void           setStationViaURL(const char* url);
void           savefile(const char* fileName, uint32_t contentLength);
String         setI2STone();
void           SD_playFile(const char* path, const char* fileName);
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
void           connecttoFS(const char* filename, uint32_t resumeFilePos = 0);
void           stopSong();
void IRAM_ATTR headphoneDetect();
void           placingGraphicObjects();
void           muteChanged(bool m);
void           BTpowerChanged(int8_t newState); // true -> power on, false -> power off
void           setTimeCounter(uint8_t sec);


//prototypes (audiotask.cpp)
void           audioInit();
void           audioControlTaskDelete();
void           audioSetVolume(uint8_t vol, uint8_t curve);
void           audioSetVolumeSteps(uint8_t steps);
uint8_t        audioGetVolume();
uint32_t       audioGetBitRate();
boolean        audioConnecttohost(const char* host, const char* user = "", const char* pwd = "");
boolean        audioConnecttoFS(const char* filename, uint32_t resumeFilePos = 0);
boolean        audioConnecttospeech(const char* text, const char* lang);
uint32_t       audioStopSong();
void           audioSetTone(int8_t param0, int8_t param1, int8_t param2, int8_t param3);
uint32_t       audioInbuffFilled();
uint32_t       audioInbuffFree();
uint32_t       audioInbuffSize();
boolean        audioIsRunning();
uint32_t       audioGetStackHighWatermark();
uint32_t       audioGetCodec();
boolean        audioPauseResume();
void           audioConnectionTimeout(uint32_t timeout_ms, uint32_t timeout_ms_ssl);
uint32_t       audioGetFileSize();
uint32_t       audioGetFilePosition();
uint16_t       audioGetVUlevel();
uint32_t       audioGetFileDuration();
uint32_t       audioGetCurrentTime();
bool           audioSetTimeOffset(int16_t timeOffset);
void           audioSetCoreID(uint8_t coreId);

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline const char* byte_to_binary(int8_t x) { // e.g. alarmdays
    static char b[9];
    b[0] = '\0';

    int32_t z;
    for(z = 128; z > 0; z >>= 1) { strcat(b, ((x & z) == z) ? "1" : "0"); }
    return b;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline uint32_t simpleHash(const char* str) {
    if(str == NULL) return 0;
    uint32_t hash = 0;
    for(int32_t i = 0; i < strlen(str); i++) {
        if(str[i] < 32) continue; // ignore control sign
        hash += (str[i] - 31) * i * 32;
    }
    return hash;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t str2int(const char* str) {
    int32_t len = strlen(str);
    if(len > 0) {
        for(int32_t i = 0; i < len; i++) {
            if(!isdigit(str[i])) {
                log_e("NaN");
                return 0;
            }
        }
        return stoi(str);
    }
    return 0;
}

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* int2str(int32_t val) {
    static char ret[12];
    itoa(val, ret, 10);
    return ret;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void trim(char* s) {
    // fb   trim in place
    char* pe;
    char* p = s;
    while(isspace(*p)) p++; // left
    pe = p;                 // right
    while(*pe != '\0') pe++;
    do { pe--; } while((pe > p) && isspace(*pe));
    if(p == s) { *++pe = '\0'; }
    else { // move
        while(p <= pe) *s++ = *p++;
        *s = '\0';
    }
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline bool startsWith(const char* base, const char* searchString) {
    if(base == NULL) {log_e("base = NULL"); return false;}                      // guard
    if(searchString == NULL) {log_e("searchString == NULL"); return false;}     // guard
    if(strlen(searchString) > strlen(base)) return false;
    char c;
    while((c = *searchString++) != '\0')
        if(c != *base++) return false;
    return true;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline bool endsWith(const char* base, const char* searchString) {
    if(base == NULL) {log_e("base = NULL"); return false;}                      // guard
    if(searchString == NULL) {log_e("searchString == NULL"); return false;}     // guard
    int32_t slen = strlen(searchString);
    if(slen == 0) return false;
    const char* p = base + strlen(base);
    //  while(p > base && isspace(*p)) p--;  // rtrim
    p -= slen;
    if(p < base) return false;
    return (strncmp(p, searchString, slen) == 0);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t indexOf(const char* haystack, const char* needle, int32_t startIndex) {
    const char* p = haystack;
    for(; startIndex > 0; startIndex--)
        if(*p++ == '\0') return -1;
    char* pos = strstr(p, needle);
    if(pos == nullptr) return -1;
    return pos - haystack;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t lastIndexOf(const char* haystack, const char needle) {
    const char* p = strrchr(haystack, needle);
    return (p ? p - haystack : -1);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline boolean strCompare(char* str1, char* str2) { // returns true if str1 == str2
    if(!str1) return false;
    if(!str2) return false;
    if(strlen(str1) != strlen(str2)) return false;
    boolean  f = true;
    uint16_t i = strlen(str1);
    while(i) {
        i--;
        if(str1[i] != str2[i]) {
            f = false;
            break;
        }
    }
    return f;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline boolean strCompare(const char* str1, char* str2) { // returns true if str1 == str2
    if(!str1) return false;
    if(!str2) return false;
    if(strlen(str1) != strlen(str2)) return false;
    boolean  f = true;
    uint16_t i = strlen(str1);
    while(i) {
        i--;
        if(str1[i] != str2[i]) {
            f = false;
            break;
        }
    }
    return f;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int replacestr(char* line, const char* search, const char* replace) { /* returns number of strings replaced.*/
    int   count;
    char* sp; // start of pattern
    // printf("replacestr(%s, %s, %s)\n", line, search, replace);
    if((sp = strstr(line, search)) == NULL) { return (0); }
    count = 1;
    int sLen = strlen(search);
    int rLen = strlen(replace);
    if(sLen > rLen) {
        // move from right to left
        char* src = sp + sLen;
        char* dst = sp + rLen;
        while((*dst = *src) != '\0') {
            dst++;
            src++;
        }
    }
    else if(sLen < rLen) {
        // move from left to right
        int   tLen = strlen(sp) - sLen;
        char* stop = sp + rLen;
        char* src = sp + sLen + tLen;
        char* dst = sp + rLen + tLen;
        while(dst >= stop) {
            *dst = *src;
            dst--;
            src--;
        }
    }
    memcpy(sp, replace, rLen);
    count += replacestr(sp + rLen, search, replace);
    return (count);
}

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_malloc(uint16_t len) {
    char* ps_str = NULL;
    if(psramFound()){ps_str = (char*) ps_malloc(len);}
    else             {ps_str = (char*)    malloc(len);}
    return ps_str;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_calloc(uint16_t len, uint8_t size) {
    char* ps_str = NULL;
    if(psramFound()){ps_str = (char*) ps_calloc(len, size);}
    else             {ps_str = (char*)    calloc(len, size);}
    return ps_str;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline char* x_ps_strdup(const char* str) {
    char* ps_str = NULL;
    if(psramFound()) { ps_str = (char*)ps_malloc(strlen(str) + 1); }
    else { ps_str = (char*)malloc(strlen(str) + 1); }
    strcpy(ps_str, str);
    return ps_str;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int16_t strlenUTF8(const char* str) { // returns only printable glyphs, all ASCII and UTF-8 until 0xDFBD
    if(str == NULL) return -1;
    uint16_t idx = 0;
    uint16_t cnt = 0;
    while(*(str + idx) != '\0') {
        if((*(str + idx) < 0xC0) && (*(str + idx) > 0x1F)) cnt++;
        if((*(str + idx) == 0xE2) && (*(str + idx + 1) == 0x80)) cnt++; // general punctuation
        idx++;
    }
    return cnt;
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    const int32_t run = in_max - in_min;
    if(run == 0) {
        log_e("map(): Invalid input range, %i == %i (min == max)", in_min, in_max);
        return -1; // AVR returns -1, SAM returns 0
    }
    const int32_t rise = out_max - out_min;
    const int32_t delta = x - in_min;
    return round((delta * rise) / run + out_min);
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void SerialPrintflnCut(const char* item, const char* color, const char* str) {
    uint8_t maxLength = 100;
    if(strlen(str) > maxLength) {
        String f = str;
        SerialPrintfln("%s%s%s ... %s", item, color, f.substring(0, maxLength - 25).c_str(), f.substring(f.length() - 20, f.length()).c_str());
    }
    else { SerialPrintfln("%s%s%s", item, color, str); }
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
extern TFT tft;
inline void hardcopy(){
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
    File hc = SD_MMC.open("/hardcopy.bmp", "w", true);
    if(TFT_CONTROLLER < 2){
        hc.write(bmp320x240, sizeof(bmp320x240));
        uint16_t buff[320];
        for(int i = 240; i > 0; i--){
            tft.readRect(0, i - 1, 320, 1, buff);
            hc.write((uint8_t*)buff, 320 * 2);
        }
        hc.close();
    }
    else{
        hc.write(bmp480x320, sizeof(bmp480x320));
        uint16_t buff[480];
        for(int i = 320; i > 0; i--){
            tft.readRect(0, i - 1, 480, 1, buff);
            hc.write((uint8_t*)buff, 480 * 2);
        }
        hc.close();
    }
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
inline void vector_clear_and_shrink(vector<char*>& vec) {
    uint size = vec.size();
    for(int32_t i = 0; i < size; i++) {
        if(vec[i]) {
            free(vec[i]);
            vec[i] = NULL;
        }
    }
    vec.clear();
    vec.shrink_to_fit();
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class SD_content{
private:
    std::vector<char*>  m_SD_content;
    File m_masterFile;
    File m_slaveFile;
    char* m_buff = NULL;
public:
    SD_content(){
        vector_clear_and_shrink(m_SD_content);
    }
    ~SD_content(){
        vector_clear_and_shrink(m_SD_content);
        if(m_buff){free(m_buff); m_buff = NULL;}
    }
    bool listDir(const char* path, boolean audioFilesOnly, boolean withoutDirs){
        m_buff = x_ps_malloc(1024);
        if(!m_buff) {log_e("oom"); return false;}
        vector_clear_and_shrink(m_SD_content);
        if(m_masterFile) m_masterFile.close();
        if(!SD_MMC.exists(path)) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
            return false;
        }
        m_masterFile = SD_MMC.open(path);
        if(!m_masterFile.isDirectory()) {
            SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
            m_masterFile.close();
            return false;
        }
        while(true) { // get content
            m_slaveFile = m_masterFile.openNextFile();
            if(!m_slaveFile) break;
            if(m_slaveFile.isDirectory()) {
                if(!withoutDirs) {
                    m_buff[0] = 2; // ASCII: start of text, sort set dirs on first position
                    sprintf(m_buff + 1, "%s", m_slaveFile.name());
                    m_SD_content.push_back(x_ps_strdup((const char*)m_buff));
                }
            }
            else {
                if(audioFilesOnly) {
                    if(endsWith(m_slaveFile.name(), ".mp3") || endsWith(m_slaveFile.name(), ".aac") || endsWith(m_slaveFile.name(), ".m4a") || endsWith(m_slaveFile.name(), ".wav") || endsWith(m_slaveFile.name(), ".flac") ||
                       endsWith(m_slaveFile.name(), ".m3u") || endsWith(m_slaveFile.name(), ".opus") || endsWith(m_slaveFile.name(), ".ogg")) {
                        sprintf(m_buff, "%s" ANSI_ESC_YELLOW " %d", m_slaveFile.name(), m_slaveFile.size());
                        m_SD_content.push_back(x_ps_strdup((const char*)m_buff));
                    }
                }
                else {
                    sprintf(m_buff, "%s" ANSI_ESC_YELLOW " %d", m_slaveFile.name(), m_slaveFile.size());
                    m_SD_content.push_back(x_ps_strdup((const char*)m_buff));
                }
            }
        }
        for(int i = 0; i < m_SD_content.size(); i++) { // easy bubble sort
            for(int j = 1; j < m_SD_content.size(); j++) {
                if(strcmp(m_SD_content[j - 1], m_SD_content[i]) > 0) { swap(m_SD_content[i], m_SD_content[j - 1]); }
            }
        }
        for(int i = 0; i < m_SD_content.size(); i++) {
            if(m_SD_content[i][0] == 2) { // remove ASCII 2
                memcpy(m_SD_content[i], m_SD_content[i] + 1, strlen(m_SD_content[i]));
            }
        }
        m_masterFile.close();
        if(m_buff){free(m_buff); m_buff = NULL;}
        return true;
    }
    size_t getSize(){
        return m_SD_content.size();
    }
    const char* getIndex(uint16_t idx){
        return m_SD_content[idx];
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class stationManagement{
private:
    std::vector<char*>  m_stations;
    File m_stationsFile;
    char* m_buff = NULL;
    size_t m_buffSize = 2048;
    bool m_hide = false;
    char* m_staName = NULL;
    char* m_staCy = NULL;
    char* m_staURL = NULL;
    const char m_path[16] = "/stations.html";
    const char m_td[21] = "const tableData = [\n";
    const char m_dummy[33] = "[\"\",\" \",\"dummy\",\"dummy\"]";
    uint16_t m_dataBegin = 0;
    uint16_t m_dataEnd = 0;

public:
    struct stationItems{
        bool hide;
        const char* staCY;
        const char* staName;
        const char* staURL;
    } staItems;
    stationManagement(){
        vector_clear_and_shrink(m_stations);
        m_buff = (char*)x_ps_malloc(m_buffSize);
        m_dataBegin = indexOf(stations_html, "<script>", 0) + 9;
        m_dataEnd = indexOf(stations_html, "</script>", m_dataBegin);
    }
    ~stationManagement(){
        vector_clear_and_shrink(m_stations);
        if(m_buff){free(m_buff); m_buff = NULL;}
        if(m_staName){free(m_buff); m_buff = NULL;}
        if(m_staCy){free(m_staCy); m_staCy = NULL;}
        if(m_staURL){free(m_buff); m_staURL = NULL;}
    }
    void clearStationsList(){
        vector_clear_and_shrink(m_stations);
    }
//----------------------------------------------------------------------------------------------------------
    void insertStation(stationItems items, uint16_t pos){
        if(pos == 0){log_e("pos 0 is reserved");  return;} // guard, 0 is'nt a valid station number
        if(pos > m_stations.size()) {log_e("pos %i is too high", pos); return;} // guard, is too high

        packStations(items); // packed str is located in m_buff
        m_stations.insert(m_stations.begin() + pos, strdup(m_buff));

        createNewStationsFile();
        // m_stationsFile = SD_MMC.open("/stations.html","w", true);
        // m_stationsFile.write((uint8_t*)stations_html, m_dataBegin);
        // m_stationsFile.write((uint8_t*)m_td, strlen(m_td));

        // for(int i = 1; i < m_stations.size(); i++){
        //     m_stationsFile.printf("%s\n", m_stations[i]);
        // }
        // m_stationsFile.write((uint8_t*)"];\n", 3);
        // m_stationsFile.printf("%s\n", stations_html + m_dataEnd);

        // m_stationsFile.close();
    }
//----------------------------------------------------------------------------------------------------------
    void eraseStation(uint16_t pos){
        if(pos == 0){log_e("pos 0 is reserved");  return;} // guard, 0 is'nt a valid station number
        if(pos > m_stations.size()) {log_e("pos %i is too high", pos); return;} // guard, is too high
        if(m_stations[pos]){free(m_stations[pos]), m_stations[pos] = NULL;}
        m_stations.erase(m_stations.begin() + pos);

        createNewStationsFile();
        // m_stationsFile = SD_MMC.open("/stations.html","w", true);
        // m_stationsFile.write((uint8_t*)stations_html, m_dataBegin);
        // m_stationsFile.write((uint8_t*)m_td, strlen(m_td));

        // for(int i = 1; i < m_stations.size(); i++){
        //     m_stationsFile.printf("%s\n", m_stations[i]);
        // }
        // m_stationsFile.write((uint8_t*)"];\n", 3);
        // m_stationsFile.printf("%s\n", stations_html + m_dataEnd);

        // m_stationsFile.close();
    }
//----------------------------------------------------------------------------------------------------------
    uint16_t getNrOfStations(){
        return m_stations.size() - 1; // 0 is dummy
    }
private:
    const char* getStationVector(uint16_t staNr){
        if(staNr >= m_stations.size()) return "station number too high";
        return m_stations[staNr];
    }
public:
    stationItems getStationItems(uint16_t staNr){
        if(staNr == 0) {log_e("station number 0 can*t be used, is reserved"); return staItems;}
        if(staNr > m_stations.size()){log_e("given station number is too high"); return staItems;}
        unpackStation(staNr - 1);
        staItems.hide = m_hide;
        staItems.staCY = m_staCy;
        staItems.staName = m_staName;
        staItems.staURL = m_staURL;
        return staItems;
    }

    const char* getStationName(uint16_t staNr){
        unpackStation(staNr);
        return m_staName;
    }
    const char* getStationUrl(uint16_t staNr){
        unpackStation(staNr);
        return m_staURL;
    }
    const char* getStatonCountry(uint16_t staNr){
        unpackStation(staNr);
        return m_staCy;
    }
    bool isHide(uint16_t staNr){
        unpackStation(staNr);
        return m_hide;
    }
    bool putAllStationsIntoRAM(){ // read file, put entries in vector
        if(!SD_MMC.exists(m_path)){
            log_e("file %s not found", m_path);
            return false;
        }
        m_stationsFile = SD_MMC.open("/stations.html");
        m_stationsFile.readBytes(m_buff, m_dataBegin);
        m_stationsFile.readBytesUntil('\n', m_buff, m_buffSize);
        m_stations.push_back(strdup(m_dummy));
        while(true){
            int len = m_stationsFile.readBytesUntil('\n', (uint8_t*)m_buff, m_buffSize);
            m_buff[len] = '\0';
            if(len < 5) break;
            m_stations.push_back(strdup(m_buff));
        }
        m_stationsFile.close();

//      for(int i = 1; i < m_stations.size(); i++){
//          log_e("%s", m_stations[i]);
//      }
        return true;
    }
private:
    void unpackStation(uint16_t staNr){
        int b1, e1, b2, e2, b3, e3, b4, e4;
        if(!m_stations.size()) {log_e("no stations found!"); return;}
        strcpy(m_buff, m_stations[staNr]);
        b1 = indexOf(m_buff, "[\"",  0) + 2; e1 = indexOf(m_buff, "\",", b1);
        b2 = indexOf(m_buff, ",\"", b1) + 2; e2 = indexOf(m_buff, "\",", b2);
        b3 = indexOf(m_buff, ",\"", b2) + 2; e3 = indexOf(m_buff, "\",", b3);
        b4 = indexOf(m_buff, ",\"", b3) + 2; e4 = indexOf(m_buff, "\"]", b4);
        m_buff[e1] = '\0'; m_buff[e2] = '\0'; m_buff[e3] = '\0'; m_buff[e4] = '\0';
    //    log_w("H: %s, Cy %s, Sn %s, URL %s", m_buff + b1, m_buff + b2, m_buff + b3 , m_buff + b4);
        if(!strlen(m_buff + b1)) m_hide = false; else m_hide = true;
        if(m_staCy){free(m_staCy); m_staCy = NULL;} m_staCy = strdup(m_buff + b2);
        if(m_staName){free(m_buff); m_buff = NULL;} m_staName = strdup(m_buff + 32);
        if(m_staURL){free(m_buff); m_staURL = NULL;} m_staURL = strdup(m_buff + b4);
    }
    void packStations(stationItems items){
        memset(m_buff, 0, m_buffSize);
        strcat(m_buff, "[\"");
        if(items.hide) strcat(m_buff, "*");
        strcat(m_buff, "\",\"");
        strcat(m_buff, items.staCY);
        strcat(m_buff, "\",\"");
        strcat(m_buff, items.staName);
        strcat(m_buff, "\",\"");
        strcat(m_buff, items.staURL);
        strcat(m_buff, "\"],");
    }
    void createNewStationsFile(){ // create or overwrite stations.html
        m_stationsFile = SD_MMC.open("/stations.html","w", true);
        m_stationsFile.write((uint8_t*)stations_html, m_dataBegin);
        m_stationsFile.write((uint8_t*)m_td, strlen(m_td));

        for(int i = 1; i < m_stations.size(); i++){
            m_stationsFile.printf("%s\n", m_stations[i]);
        }
        m_stationsFile.write((uint8_t*)"];\n", 3);
        m_stationsFile.printf("%s\n", stations_html + m_dataEnd);

        m_stationsFile.close();
    }

};

//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     G R A P H I C   O B J E C T S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

extern __attribute__((weak)) void graphicObjects_OnChange(const char* name, int32_t arg1);
extern __attribute__((weak)) void graphicObjects_OnClick(const char* name, uint8_t val);
extern __attribute__((weak)) void graphicObjects_OnRelease(const char* name, releasedArg ra);

extern SemaphoreHandle_t mutex_display;
extern SD_content _SD_content;
class slider{
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
    uint8_t     m_railHigh = 0;
    uint16_t    m_middle_h = 0;
    uint16_t    m_spotPos = 0;
    uint8_t     m_spotRadius = 0;
    char*       m_name = NULL;
    releasedArg m_ra;
public:
    slider(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("slider");
        m_railHigh = 6;
        m_spotRadius = 12;
        m_bgColor = TFT_BLACK;
        m_railColor = TFT_BEIGE;
        m_spotColor = TFT_RED;
    }
    ~slider(){
        m_objectInit = false;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, int16_t minVal, int16_t maxVal){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_minVal = minVal;
        m_maxVal = maxVal;
        m_leftStop = m_x + m_spotRadius + 10; // x pos left stop
        m_rightStop = m_x + m_w - m_spotRadius - 10; // x pos right stop
        m_enabled = false;
        m_middle_h = m_y + (m_h / 2);
        m_spotPos = (m_leftStop + m_rightStop) / 2; // in the middle
        m_objectInit = true;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;

        if(m_enabled){
            if(x < m_leftStop) x = m_leftStop;// (x, y) is in range
            if(x > m_rightStop) x = m_rightStop;
            m_clicked = true;
            drawNewSpot(x);
        }
        if(!m_clicked){ if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);}
        if(!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val){
        if(!m_objectInit) return;
        if(val < m_minVal) val = m_minVal;
        if(val > m_maxVal) val = m_maxVal;
        m_val = val;
        if(m_clicked) return;
        uint16_t spotPos = map_l(val, m_minVal, m_maxVal, m_leftStop, m_rightStop); // val -> x
        if(m_enabled) drawNewSpot(spotPos);
        else m_spotPos = spotPos;
    }
    int16_t getValue(){
        return m_val;
    }
    void setNewMinMaxVal(int16_t minVal, int16_t maxVal){
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    void show(){
        m_enabled = true;
        tft.fillRoundRect(m_x, m_middle_h - (m_railHigh / 2), m_w, m_railHigh, 2, m_railColor);
        drawNewSpot(m_spotPos);
    }
    void disable(){
        m_enabled = false;
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if(run == 0) {
            log_e("map(): Invalid input range, %i == %i (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewSpot(uint16_t xPos){
        if(m_enabled){
            tft.fillRect(m_spotPos - m_spotRadius, m_middle_h - m_spotRadius,     2 * m_spotRadius, 2 * m_spotRadius, m_bgColor);
            tft.fillRect(m_spotPos - m_spotRadius, m_middle_h - (m_railHigh / 2), 2 * m_spotRadius + 1, m_railHigh,   m_railColor);
            tft.fillCircle(xPos, m_middle_h, m_spotRadius, m_spotColor);
        }
        m_spotPos = xPos;
        int32_t val = map_l(m_spotPos, m_leftStop, m_rightStop, m_minVal, m_maxVal); // xPos -> val
        m_ra.val1 = val;
        if(graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, val);
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class progressbar{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_val = 0;
    int16_t     m_minVal = 0;
    int16_t     m_maxVal = 0;
    int16_t     m_oldPos = 0;
    uint32_t    m_bgColor = 0;
    uint32_t    m_frameColor = 0;
    uint32_t    m_railColorLeft = 0;
    uint32_t    m_railColorRight = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_objectInit = false;
    uint8_t     m_railHigh = 0;
    char*       m_name = NULL;
    releasedArg m_ra;
public:
    progressbar(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("progressbar");
        m_railHigh = 6;
        m_bgColor = TFT_BLACK;
        m_frameColor = TFT_WHITE;
        m_railColorLeft = TFT_RED;
        m_railColorRight = TFT_GREEN;
    }
    ~progressbar(){
        m_objectInit = false;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, int16_t minVal, int16_t maxVal){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_minVal = minVal;
        m_maxVal = maxVal;
        m_enabled = false;
        m_objectInit = true;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val){
        if(!m_objectInit) return;
        if(val < m_minVal) val = m_minVal;
        if(val > m_maxVal) val = m_maxVal;
        m_val = val;
        if(m_clicked) return;
        if(m_enabled) drawChanges();
    }
    int16_t getValue(){
        return m_val;
    }
    void setNewMinMaxVal(int16_t minVal, int16_t maxVal){
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    void show(){
        tft.drawRect(m_x, m_y, m_w, m_h, m_frameColor);
        drawNewValue();
        m_enabled = true;
    }
    void disable(){
        m_enabled = false;
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if(run == 0) {
            log_e("map(): Invalid input range, %i == %i (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewValue(){
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, m_x + 1, m_x + m_w - 2);
        tft.fillRect(m_x + 1, m_y + 1,  pos, m_h - 2, m_railColorLeft);
        tft.fillRect(pos, m_y + 1,  m_w  -  pos - 1, m_h - 2, m_railColorRight);
        m_oldPos = pos;
        if(graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, m_val);
    }
    void drawChanges(){
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, m_x + 1, m_x + m_w - 2);
        if(pos > m_oldPos){
            tft.fillRect(m_oldPos, m_y + 1, pos, m_h - 2, m_railColorLeft);
        }
        if(pos < m_oldPos){
            tft.fillRect(pos, m_y + 1,  m_oldPos - pos, m_h - 2, m_railColorRight);
        }
        if(graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, m_val);
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbox{
private:
    int16_t         m_x = 0;
    int16_t         m_y = 0;
    int16_t         m_w = 0;
    int16_t         m_h = 0;
    uint8_t         m_fontSize = 0;
    uint8_t         m_h_align = TFT_ALIGN_RIGHT;
    uint8_t         m_v_align = TFT_ALIGN_TOP;
    uint8_t         m_l_margin = 0; // left margin
    uint8_t         m_r_margin = 0; // right margin
    uint8_t         m_t_margin = 0; // top margin
    uint8_t         m_b_margin = 0; // bottom margin
    uint8_t         m_d_margin = 0;
    uint32_t        m_bgColor = 0;
    uint32_t        m_fgColor = 0;
    char*           m_text = NULL;
    char*           m_name = NULL;
    bool            m_enabled = false;
    bool            m_clicked = false;
    bool            m_autoSize = false;
    releasedArg     m_ra;
public:
    textbox(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("textbox");
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_fontSize = 1;
    }
    ~textbox(){
        if(m_text){free(m_text); m_text = NULL;}
        if(m_name){free(m_name); m_name = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_l_margin = 0;
        m_r_margin = w / 100;
        m_t_margin = 0;
        m_b_margin = h / 50;
    }
    void show(){
        m_enabled = true;
        m_clicked = false;
        if(!m_text){m_text = strdup("");}
        writeText(m_text, m_h_align, m_v_align);
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void enable(){
        m_enabled = true;
    }
    void setFont(uint8_t size){ // size 0 -> auto, choose besr font size
        if(size != 0) {m_fontSize = size; tft.setFont(m_fontSize);}
        else{m_autoSize = true;}
    }
    void setTextColor(uint32_t color){
        m_fgColor = color;
    }
    void setBGcolor(uint32_t color){
        m_bgColor = color;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
    void setText(const char* txt, uint8_t h_align = TFT_ALIGN_RIGHT,  uint8_t v_align = TFT_ALIGN_TOP){ // prepare a text, wait of show() to write it
        if(!txt){txt = strdup("");}
        if(m_text){free(m_text); m_text = NULL;}
        m_text = x_ps_strdup(txt);
        m_h_align = h_align;
        m_v_align = v_align;
    }
    void writeText(const char* txt, uint8_t h_align = TFT_ALIGN_RIGHT, uint8_t v_align = TFT_ALIGN_TOP){
        if(!txt){txt = strdup("");}
        if(txt != m_text){ // no self copy
            if(m_text){free(m_text); m_text = NULL;}
            m_text = x_ps_strdup(txt);
        }
        m_h_align = h_align;
        m_v_align = v_align;
        if(m_enabled){
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            tft.setTextColor(m_fgColor);
            tft.setBackGoundColor(m_bgColor);
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            if(m_fontSize != 0){ tft.setFont(m_fontSize);}
            tft.writeText(m_text, m_x + m_l_margin, m_y + m_t_margin, m_w - m_r_margin, m_h - m_d_margin, m_h_align, m_v_align, false, false, m_autoSize);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button1state{ // click button
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = 0;
    char*       m_defaultPicturePath = NULL;
    char*       m_clickedPicturePath = NULL;
    char*       m_inactivePicturePath = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    char*       m_name = NULL;
    releasedArg m_ra;
public:
    button1state(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("button1state");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        setDefaultPicturePath(NULL);
        setClickedPicturePath(NULL);
        setInactivePicturePath(NULL);
    }
    ~button1state(){
        if(m_defaultPicturePath) {free(m_defaultPicturePath);  m_defaultPicturePath = NULL;}
        if(m_clickedPicturePath) {free(m_clickedPicturePath);  m_clickedPicturePath = NULL;}
        if(m_inactivePicturePath){free(m_inactivePicturePath); m_inactivePicturePath = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
    }
    void show(bool inactive = false){
        m_clicked = false;
        if(inactive){
            setInactive();
            return;
        }
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_enabled = true;
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setInactive(){
        drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        m_enabled = false;
    }
    void setDefaultPicturePath(const char* path){
        if(m_defaultPicturePath){free(m_defaultPicturePath); m_defaultPicturePath = NULL;}
        if(path) m_defaultPicturePath = x_ps_strdup(path);
        else m_defaultPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedPicturePath(const char* path){
        if(m_clickedPicturePath){free(m_clickedPicturePath); m_clickedPicturePath = NULL;}
        if(path) m_clickedPicturePath = x_ps_strdup(path);
        else m_clickedPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path){
        if(m_inactivePicturePath){free(m_inactivePicturePath); m_inactivePicturePath = NULL;}
        if(path) m_inactivePicturePath = x_ps_strdup(path);
        else m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) {
            drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
        }
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
    //    if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button2state{ // on off switch
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
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    char*       m_name = NULL;
    releasedArg m_ra;
public:
    button2state(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("button2state");
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
    ~button2state(){
        if(m_offPicturePath) {free(m_offPicturePath);  m_offPicturePath = NULL;}
        if(m_onPicturePath) {free(m_onPicturePath);  m_onPicturePath = NULL;}
        if(m_clickedOffPicturePath){free(m_clickedOffPicturePath); m_clickedOffPicturePath = NULL;}
        if(m_clickedOnPicturePath){free(m_clickedOnPicturePath); m_clickedOnPicturePath = NULL;}
        if(m_inactivePicturePath){free(m_inactivePicturePath); m_inactivePicturePath = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
    }
    void show(bool inactive = false){
        m_clicked = false;
        if(inactive){
            setInactive();
            return;
        }
        if(m_state) drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
        else drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        m_enabled = true;
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setValue(bool val){
        m_state = val;
        if(m_enabled) {
            if(m_state) drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
            else drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    bool getValue(){
        return m_state;
    }
    void setOn(){
        m_state = true;
    }
    void setOff(){
        m_state = false;
    }
    void setInactive(){
        drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        m_enabled = false;
    }
    void setOffPicturePath(const char* path){
        if(m_offPicturePath){free(m_offPicturePath); m_offPicturePath = NULL;}
        if(path) m_offPicturePath = x_ps_strdup(path);
        else m_offPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedOffPicturePath(const char* path){
        if(m_clickedOffPicturePath){free(m_clickedOffPicturePath); m_clickedOffPicturePath = NULL;}
        if(path) m_clickedOffPicturePath = x_ps_strdup(path);
        else m_clickedOffPicturePath = x_ps_strdup("clickedOffPicturePath is not set");
    }
    void setClickedOnPicturePath(const char* path){
        if(m_clickedOnPicturePath){free(m_clickedOnPicturePath); m_clickedOnPicturePath = NULL;}
        if(path) m_clickedOnPicturePath = x_ps_strdup(path);
        else m_clickedOnPicturePath = x_ps_strdup("clickedOnPicturePath is not set");
    }
    void setOnPicturePath(const char* path){
        if(m_onPicturePath){free(m_onPicturePath); m_onPicturePath = NULL;}
        if(path) m_onPicturePath = x_ps_strdup(path);
        else m_onPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path){
        if(m_inactivePicturePath){free(m_inactivePicturePath); m_inactivePicturePath = NULL;}
        if(path) m_inactivePicturePath = x_ps_strdup(path);
        else m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled){
            if(m_state) drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
            else        drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
            m_state = !m_state;
        }
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
    //    if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        if(m_state) drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
        else drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class pictureBox{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = 0;
    char*       m_PicturePath = NULL;
    char*       m_altPicturePath = NULL;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    releasedArg m_ra;
public:
    pictureBox(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("pictureBox");
        setPicturePath(NULL);
        setAlternativPicturePath(NULL);
    }
    ~pictureBox(){
        if(m_PicturePath) {free(m_PicturePath);  m_PicturePath = NULL;}
        if(m_altPicturePath) {free(m_altPicturePath);  m_altPicturePath = NULL;}
    }
    void begin(uint16_t x, uint16_t y){
        m_x = x; // x pos
        m_y = y; // y pos
        m_enabled = false;
    }
    bool show(){
        if(!GetImageSize(m_PicturePath)){
            GetImageSize(m_altPicturePath);
             m_enabled = drawImage(m_altPicturePath, m_x, m_y);
            return m_enabled;
        }
        else{
            m_enabled = drawImage(m_PicturePath, m_x, m_y);
            return m_enabled;
        }
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void enable(){
        m_enabled = true;
    }
    void setPicturePath(const char* path){
        if(m_PicturePath){free(m_PicturePath); m_PicturePath = NULL;}
        if(path) m_PicturePath = x_ps_strdup(path);
        else m_PicturePath = x_ps_strdup("picturePath is not set");
        if(path){GetImageSize(path);}
    }
    void setAlternativPicturePath(const char* path){
        if(m_altPicturePath){free(m_altPicturePath); m_altPicturePath = NULL;}
        if(path) m_altPicturePath = x_ps_strdup(path);
        else m_altPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
    //    if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
    bool GetImageSize(const char* picturePath){
        const char* scaledPicPath = scaleImage(picturePath);
        if(!SD_MMC.exists(scaledPicPath)) {log_i("file %s not exists, objName: %s", scaledPicPath, m_name); return false;}
        File file = SD_MMC.open(scaledPicPath,"r", false);
        if(file.size() < 24) {log_i("file %s is too small", scaledPicPath); file.close(); return false;}
        char buf[8];
        file.readBytes(buf,3);
        if ((buf[0] == 0xFF) && (buf[1] == 0xD8) && (buf[2] == 0xFF)) { // format jpeg
            int16_t c1, c2;
            while(true){
                c1 = file.read();
                if(c1 == -1) {log_w("sof marker in %s not found", scaledPicPath); file.close(); return false;} //end of file reached
                if(c1 == 0xFF){c2 = file.read(); if(c2 == 0xC0) break;} // 0xFFC0 Marker found
            }
            file.readBytes(buf,7);
            m_h = buf[3] * 256 + buf[4];
            m_w = buf[5] * 256 + buf[6];
        //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        if ((buf[0] == 'B') && (buf[1] == 'M') && (buf[2] == '6')) { // format bmp
            for(int i= 0; i < 15; i++) file.read(); // read 15 dummys
            m_w  = file.read(); // pos 18
            m_w += (file.read() << 8);
            m_w += (file.read() << 16);
            m_w += (file.read() << 24);
            m_h  = file.read(); // pos 22
            m_h += (file.read() << 8);
            m_h += (file.read() << 16);
            m_h += (file.read() << 24);
        //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        if ((buf[0] == 'G') && (buf[1] == 'I') && (buf[2] == 'F')) { // format gif
            for(int i= 0; i < 3; i++) file.read(); // read 3 dummys
            m_w  = file.read(); // pos 6
            m_w += (file.read() << 8);
            m_h  = file.read(); // pos 8
            m_h += (file.read() << 8);
        //    log_i("w %i, h %i", m_w, m_h);
            return true;
        }
        log_e("unknown picture format");
        return false;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock{ // draw a clock in 12 or 24h format
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
#if TFT_CONTROLLER < 2
    uint16_t    m_timeXPos7S[5] = {2, 75, 173, 246, 148}; // seven segment digits "hhmm:"
    uint16_t    m_timeXPosFN[6] = {0, 56, 152, 208, 264, 112}; // folded numbers
#else
    uint16_t m_timeXPos7S[5] = {12, 118, 266, 372, 224}; // seven segment digits "hhmm:""
    uint16_t m_timeXPosFN[6] = {16, 96,  224, 304, 384, 176}; // folded numbers
#endif
    uint16_t    m_minuteOfTheDay = 0;
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    uint8_t     m_timeFormat = 24;
    bool        m_showAll = false;
    char*       m_name = NULL;
    char*       m_pathBuff = NULL;
    uint8_t     m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg m_ra;
public:
    imgClock(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("imgClock");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock(){
        if(m_pathBuff){free(m_pathBuff); m_pathBuff = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
    }
    void show(bool inactive = false){
        m_clicked = false;
        if(inactive){
        //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide(){
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable(){
        m_enabled = false;
    }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday){
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_minuteOfTheDay = minuteOfTheDay;
        m_hour = minuteOfTheDay / 60;
        m_min  = minuteOfTheDay % 60;
        m_weekday  = weekday;
        if(m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t  m_min){
        static uint8_t oldTime[4];
        static bool k = false;
        uint8_t time[5];
        time[0] = m_hour / 10; time[1] = m_hour % 10;
        time[2] = m_min / 10;  time[3] = m_min % 10;

        if(m_timeFormat == 24){
            for(uint8_t i = 0; i < 4; i++){
                if((time[i] != oldTime[i]) || m_showAll){
                    sprintf(m_pathBuff, "/digits/sevenSegment/%igreen.jpg", time[i]);
                    drawImage(m_pathBuff, m_timeXPos7S[i], m_y);
                }
                oldTime[i] = time[i];
            }
        }
        else { // 12h format
             bool isPM = true;
             static bool isOldPM = false;
            for(uint8_t i = 0; i < 4; i++){
                uint8_t hour = m_hour;
                if(hour > 0 && hour < 13) isPM = false;
                else(hour -= 12);
                time[0] = hour / 10; time[1] = hour % 10;
                if((time[i] != oldTime[i]) || m_showAll){
                    sprintf(m_pathBuff, "/digits/foldedNumbers/%iwhite.jpg", time[i]);
                    drawImage(m_pathBuff, m_timeXPosFN[i], m_y);
                }
                oldTime[i] = time[i];
            }
            if((isPM != isOldPM) || m_showAll){
                if(isPM) drawImage("/digits/foldedNumbers/pmwhite.jpg", m_timeXPosFN[4], m_y);
                else     drawImage("/digits/foldedNumbers/amwhite.jpg", m_timeXPosFN[4], m_y);
                isOldPM = isPM;
            }
        }
        k = !k;
        if(m_timeFormat == 24){
            if(k) drawImage("/digits/sevenSegment/dgreen.jpg", m_timeXPos7S[4], m_y);
            else  drawImage("/digits/sevenSegment/egreen.jpg", m_timeXPos7S[4], m_y);
        }
        else{
            if(k) drawImage("/digits/foldedNumbers/dwhite.jpg", m_timeXPosFN[5], m_y);
            else  drawImage("/digits/foldedNumbers/ewhite.jpg", m_timeXPosFN[5], m_y);
        }
        m_showAll = false;
    }
    void setTimeFormat(uint8_t timeFormat){
        m_timeFormat = timeFormat;
        m_showAll = true;
    }
    bool isAlarm(uint8_t alarmdays, int16_t* alarmtime){
        uint8_t mask = 0b00000001 << m_weekday;
        if(alarmdays & mask){ // yes, is alarmday
            if(alarmtime[m_weekday] == m_minuteOfTheDay){ // yes, is alarmtime
                return true;
            }
        }
        return false;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
    //    if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_clicked = false;
        return true;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class alarmClock{ // draw a clock in 12 or 24h format
private:
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    uint16_t m_alarmdaysYPos    = 0;
    uint16_t m_alarmtimeYPos    = 0;
    uint16_t m_digitsYPos       = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_alarmdaysXPos[7] = {2, 47, 92, 137, 182, 227, 272}; // same as altarmTimeXPos
    uint16_t m_digitsPos[5]     = {2, 75, 173, 246, 148}; // seven segment digits "hhmm:"
    uint8_t  m_alarmdaysW = 44;
    uint8_t  m_alarmdaysH = 25;
    uint8_t  m_fontSize = 16;
#else
    uint16_t m_alarmdaysXPos[7] = {9, 75, 141, 207, 273, 339, 405};
    uint16_t m_digitsPos[5] = {23, 123, 258, 358, 223}; // seven segment digits "hhmm:""
    uint8_t  m_alarmdaysW = 65;
    uint8_t  m_alarmdaysH = 25;
    uint8_t  m_fontSize = 21;

#endif
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    bool        m_showAll = false;
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
    alarmClock(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("alarmClock");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        strcpy(m_pathBuff, m_p1);
    }
    ~alarmClock(){
        if(m_name){free(m_name); m_name = NULL;}
        if(m_pathBuff){free(m_pathBuff); m_pathBuff = NULL;}

    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_alarmdaysYPos    = m_y; // m_y;
        m_alarmtimeYPos    = m_alarmdaysYPos + 25 + 1;
        m_digitsYPos       = m_alarmtimeYPos + 25 + 1;
    }
    void show(bool inactive = false){
        m_clicked = false;
        if(inactive){
        //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        updateDigit();
        updateAlarmDaysAndTime();
    }
    void hide(){
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable(){
        m_enabled = false;
    }
    void shiftRight(){
        m_idx++;
        if(m_idx == 4) m_idx = 0;
        m_showAll = true;
        updateDigit();
    }
    void shiftLeft(){
        m_idx--;
        if(m_idx == -1) m_idx = 0;
        m_showAll = true;
        updateDigit();
    }
    void digitUp(){
        if(m_idx == 0){ // 10h
            if(m_alarmDigits[0] == 2) return;
            if(m_alarmDigits[0] == 1 && m_alarmDigits[1] > 3) return;
            m_alarmDigits[0]++;
        }
        if(m_idx == 1){ // 1h
            if(m_alarmDigits[0] == 2 && m_alarmDigits[1] == 3) return;
            if(m_alarmDigits[1] == 9) return;
            m_alarmDigits[1]++;
        }
        if(m_idx == 2){ // 10m
            if(m_alarmDigits[2] == 5) return;
            m_alarmDigits[2]++;
        }
        if(m_idx == 3){ // 1m
            if(m_alarmDigits[3] == 9) return;
            m_alarmDigits[3]++;
        }
        m_showAll = true;
        updateDigit();
    }
    void digitDown(){
        if(m_idx == 0){ // 10h
            if(m_alarmDigits[0] == 0) return;
            m_alarmDigits[0]--;
        }
        if(m_idx == 1){ // 1h
            if(m_alarmDigits[1] == 0) return;
            m_alarmDigits[1]--;
        }
        if(m_idx == 2){ // 10m
            if(m_alarmDigits[2] == 0) return;
            m_alarmDigits[2]--;
        }
        if(m_idx == 3){ // 1m
            if(m_alarmDigits[3] == 0) return;
            m_alarmDigits[3]--;
        }
        m_showAll = true;
        updateDigit();
    }
    void setAlarmTimeAndDays(uint8_t* alarmDays, int16_t alarmTime[7]){
        m_alarmTime = alarmTime;
        m_alarmDays = alarmDays;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        if(y <= m_alarmtimeYPos){
            m_btnAlarmDay = -1;
            if     (x >= m_alarmdaysXPos[6]) {m_btnAlarmDay = 6;}
            else if(x >= m_alarmdaysXPos[5]) {m_btnAlarmDay = 5;}
            else if(x >= m_alarmdaysXPos[4]) {m_btnAlarmDay = 4;}
            else if(x >= m_alarmdaysXPos[3]) {m_btnAlarmDay = 3;}
            else if(x >= m_alarmdaysXPos[2]) {m_btnAlarmDay = 2;}
            else if(x >= m_alarmdaysXPos[1]) {m_btnAlarmDay = 1;}
            else if(x >= m_alarmdaysXPos[0]) {m_btnAlarmDay = 0;}
            if(m_btnAlarmDay >= 0) alarmDaysPressed(m_btnAlarmDay);
        }
        else if(y <= m_digitsYPos){
            if     (x >= m_alarmdaysXPos[6]) {m_btnAlarmTime = 6;}
            else if(x >= m_alarmdaysXPos[5]) {m_btnAlarmTime = 5;}
            else if(x >= m_alarmdaysXPos[4]) {m_btnAlarmTime = 4;}
            else if(x >= m_alarmdaysXPos[3]) {m_btnAlarmTime = 3;}
            else if(x >= m_alarmdaysXPos[2]) {m_btnAlarmTime = 2;}
            else if(x >= m_alarmdaysXPos[1]) {m_btnAlarmTime = 1;}
            else if(x >= m_alarmdaysXPos[0]) {m_btnAlarmTime = 0;}
            if(m_btnAlarmTime >= 0) alarmTimePressed(m_btnAlarmTime);
        }
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        char hhmm[10] = "00:00";
        if(m_btnAlarmDay >= 0){
            sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmDay] / 60, m_alarmTime[m_btnAlarmDay] % 60);
        }
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        if(m_btnAlarmDay >= 0){
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmDay;
        //    log_w("mask %i, m_alarmDays %i", mask, *m_alarmDays);
            *m_alarmDays ^= mask;    // toggle the bit
        //    log_w("m_alarmDays %i", *m_alarmDays);
            if(*m_alarmDays & mask){ // is set
                tft.setFont(m_fontSize);
                tft.drawRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_RED);
                tft.setTextColor(TFT_RED);
                tft.writeText(m_WD[m_btnAlarmDay], m_alarmdaysXPos[m_btnAlarmDay], m_alarmdaysYPos,  m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);

                tft.fillRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
                tft.drawRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_GREEN);
                tft.setTextColor(TFT_GREEN);
                tft.writeText(hhmm, m_alarmdaysXPos[m_btnAlarmDay], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
            }
            else{                    // bit is not set
                tft.setFont(m_fontSize);
                tft.drawRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_DARKGREY);
                tft.setTextColor(TFT_DARKGREY);
                tft.writeText(m_WD[m_btnAlarmDay], m_alarmdaysXPos[m_btnAlarmDay], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);

                tft.fillRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
                tft.drawRect(m_alarmdaysXPos[m_btnAlarmDay], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_DARKGREY);
            }
            m_btnAlarmDay = -1;
        }
        if(m_btnAlarmTime >= 0){
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmTime;
            if(mask & *m_alarmDays){ // bit is set -> alarm is active for that day
                tft.setFont(m_fontSize);
            //    tft.fillRect(m_alarmdaysXPos[m_btnAlarmTime], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
                tft.drawRect(m_alarmdaysXPos[m_btnAlarmTime], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_GREEN);
                tft.setTextColor(TFT_GREEN);
                m_alarmTime[m_btnAlarmTime] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60  + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
                char hhmm[10] = "00:00";
                sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmTime] / 60, m_alarmTime[m_btnAlarmTime] % 60);
                tft.writeText(hhmm, m_alarmdaysXPos[m_btnAlarmTime], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
            }
            m_btnAlarmTime = -1;
        }
        m_clicked = false;
        return true;
    }
private:
    void updateDigit(){
        static uint8_t m_oldAlarmDigits[4] = {0};
        if(m_showAll) drawImage("/digits/sevenSegment/dred.jpg", m_digitsPos[4], m_digitsYPos); // colon
        for(uint8_t i = 0; i < 4; i++){
            if(m_oldAlarmDigits[i] != m_alarmDigits[i] || m_showAll){
                if(i == m_idx){
                    m_pathBuff[m_p1Len + 0] = m_alarmDigits[i] + 48;
                    m_pathBuff[m_p1Len + 1] = '\0';
                    strcat(m_pathBuff, "orange.jpg");
                }
                else{
                    m_pathBuff[m_p1Len + 0] =  m_alarmDigits[i] + 48;
                    m_pathBuff[m_p1Len + 1] = '\0';
                    strcat(m_pathBuff, "red.jpg");
                }
                drawImage(m_pathBuff, m_digitsPos[i], m_digitsYPos);

            }
            m_oldAlarmDigits[i] = m_alarmDigits[i];
        }
    }
    void updateAlarmDaysAndTime(){
        tft.setFont(m_fontSize);
        uint8_t mask = 0b00000001;
        uint16_t color = TFT_BLACK;
        for(int i = 0; i < 7; i++){
            // alarmDays
            if(*m_alarmDays & mask) color = TFT_RED;
            else color = TFT_DARKGREY;
            tft.drawRect(m_alarmdaysXPos[i], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, color);
            tft.setTextColor(color);
            tft.writeText(m_WD[i], m_alarmdaysXPos[i], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
            // alarmTime
            if(*m_alarmDays & mask){
            //    tft.fillRect(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
                tft.drawRect(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_GREEN);
                tft.setTextColor(TFT_GREEN);
                char hhmm[10] = "00:00";
                sprintf(hhmm, "%02d:%02d", m_alarmTime[i] / 60, m_alarmTime[i] % 60);
                tft.writeText(hhmm, m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
            }
            else{
                tft.fillRect(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
                tft.drawRect(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_DARKGREY);
            }
            mask <<= 1;
        }
    }
    void alarmDaysPressed(uint8_t idx){
        tft.setFont(m_fontSize);
        tft.drawRect(m_alarmdaysXPos[idx], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_YELLOW);
        tft.setTextColor(TFT_YELLOW);
        tft.writeText(m_WD[idx], m_alarmdaysXPos[idx], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
    }
    void alarmTimePressed(uint8_t idx){
        uint8_t mask = 0b00000001;
        mask <<= idx;
        if(mask & *m_alarmDays){ // bit is set -> active
            tft.setFont(m_fontSize);
            tft.fillRect(m_alarmdaysXPos[idx], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_BLACK);
            tft.drawRect(m_alarmdaysXPos[idx], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_YELLOW);
            tft.setTextColor(TFT_YELLOW);
            m_alarmTime[idx] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60  + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
            char hhmm[10] = "00:00";
            sprintf(hhmm, "%02d:%02d", m_alarmTime[idx] / 60, m_alarmTime[idx] % 60);
            tft.writeText(hhmm, m_alarmdaysXPos[idx], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER, true);
        }
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class dlnaList{
private:
    int16_t                   m_x = 0;
    int16_t                   m_y = 0;
    int16_t                   m_w = 0;
    int16_t                   m_h = 0;
    int16_t                   m_oldX = 0;
    int16_t                   m_oldY = 0;
    uint8_t*                  m_dlnaLevel;
    uint8_t                   m_fontSize = 0;
    uint8_t                   m_lineHight = 0;
    uint8_t                   m_browseOnRelease = 0;
    uint8_t                   m_itemListPos = 0;
    int8_t                    m_currDLNAsrvNr = -1;
    int16_t                   m_currItemNr = -1;
    uint16_t                  m_dlnaMaxItems = 0;
    uint32_t                  m_bgColor = 0;
    bool                      m_enabled = false;
    bool                      m_clicked = false;
    bool                      m_state = false;
    char*                     m_name = NULL;
    char*                     m_pathBuff = NULL;
    char*                     m_chptr = NULL;
    char*                     m_buff = NULL;
    char*                     m_dlnaItemsPos = NULL;
    DLNA_Client::dlnaServer_t m_dlnaServer;
    DLNA_Client::srvContent_t m_srvContent;
    DLNA_Client*              m_dlna;
    dlnaHistory*              m_dlnaHistory = NULL;
    releasedArg               m_ra;

public:
    dlnaList(const char* name, DLNA_Client *dlna, dlnaHistory* dh, uint8_t dhSize){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("dlnaList");
        m_dlna    = dlna;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        m_dlnaItemsPos = x_ps_malloc(30);
        m_ra.arg1 = NULL;
        m_ra.arg2 = NULL;
        m_ra.val1 = 0;
        m_ra.val2 = 0;
        m_dlnaHistory = dh;
    }
    ~dlnaList(){
        if(m_name){free(m_name); m_name = NULL;}
        if(m_buff){free(m_buff); m_buff = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fontSize){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_fontSize = fontSize;
        m_enabled = false;
        m_lineHight = m_h / 10;
    }
    void show(int8_t number, DLNA_Client::dlnaServer_t dlnaServer, DLNA_Client::srvContent_t srvContent, uint8_t* dlnaLevel,  uint16_t maxItems){
        m_browseOnRelease = 0;
        m_dlnaServer = dlnaServer;
        m_srvContent = srvContent;
        m_dlnaLevel  = dlnaLevel;
        if(m_dlnaLevel == 0) m_currDLNAsrvNr = number;
        m_clicked = false;
        m_enabled = true;
        m_dlnaMaxItems = maxItems;
        dlnaItemsList();
    }
    void hide(){
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable(){
        m_enabled = false;
    }
    bool positionXY(uint16_t x, uint16_t y){ // called every tine if x or y has changed
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_clicked == false){m_oldX = x; m_oldY = y;}
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y){
        if(!m_enabled) return false;
        if(!m_clicked) return false;

        hasReleased(x - m_x, y - m_y);
        m_clicked = false;

        if(m_chptr) {
            tft.setTextColor(TFT_CYAN);
            tft.setFont(m_fontSize);
            tft.writeText(m_chptr, 20, m_y + (m_itemListPos) * m_lineHight, m_w - 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
            m_chptr = NULL;
            vTaskDelay(300);
        }
        if(m_buff){free(m_buff); m_buff = NULL;}

        if(m_browseOnRelease == 1){ m_dlna->browseServer(m_currDLNAsrvNr, "0", 0 , 9);}                                          // get serverlist
        if(m_browseOnRelease == 2){ m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, 0 , 9);}            // content list
        if(m_browseOnRelease == 3){ m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, 0 , 9);}            // folder
        if(m_browseOnRelease == 4){ m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId, m_currItemNr + 1 , 9);} // scroll up / down

        m_browseOnRelease = 0;
        m_oldX = 0; m_oldY = 0;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_ra.val1 = 0;
        return true;
    }

    void longPressed(uint16_t x, uint16_t y){
        bool maybe_a_server = false;
        bool maybe_a_file = false;
        bool maybe_a_folder = false;
        m_clicked = false;                    // ignore tp released event, wait of next clicked
        m_itemListPos = (y / (m_h / 10));
        if(m_itemListPos == 0) goto exit;    // is header
        if(m_itemListPos == 1) { /* log_i("long pressed at return item %s", m_dlnaHistory[*m_dlnaLevel].name); */ goto exit;}
        if(m_itemListPos >= 11) goto exit;   // is footer

        m_itemListPos -= 2;

        if(*m_dlnaLevel == 0){
            if( m_dlnaServer.friendlyName.size() > m_itemListPos) maybe_a_server = true;
        }
        else{
            if(m_srvContent.itemURL.size() > m_itemListPos) maybe_a_file   = true;
            if(m_srvContent.title.size()   > m_itemListPos) maybe_a_folder = true;
        }

        if(maybe_a_server){
            /* log_i("long pressed at server %s", m_dlnaServer.friendlyName[m_itemListPos]); */ goto exit;
        }

        if(maybe_a_file){
            if(startsWith(m_srvContent.itemURL[m_itemListPos], "http")){
                /* log_i("long pressed at file %s", m_srvContent.itemURL[m_itemListPos]); */ goto exit;
            }
        }

        if(maybe_a_folder){
            // log_i("long pressed at folder x %s",m_srvContent.title[m_itemListPos]);
            tft.setTextColor(TFT_MAGENTA);
            tft.setFont(m_fontSize);
            tft.writeText(m_srvContent.title[m_itemListPos], 20, m_y + (m_itemListPos + 1) * m_lineHight, m_w - 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);

            m_ra.arg1 = m_srvContent.objectId[m_itemListPos];
            m_ra.val2 = m_currDLNAsrvNr;
            m_ra.val1 = 2;
            goto exit;
        }

        log_i("something went wrong or out of range");
    exit:
        return;
    }

    void longReleased(){
        if(!m_enabled) return;
        if(!m_clicked) return;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_ra.val1 = 0;
        m_clicked = false;
    }

private:
    void dlnaItemsList(){
        if(!m_buff) m_buff = x_ps_malloc(512);
        uint16_t itemsSize = 0;
        uint8_t pos = 0;
        if(*m_dlnaLevel == 0) {
            itemsSize = m_dlnaServer.size;
        }
        else { itemsSize = m_srvContent.size; }

        auto triangleUp = [&](int16_t x, int16_t y, uint8_t s) {  tft.fillTriangle(x + s, y + 0, x + 0, y + 2  *  s, x + 2  *  s, y + 2  *  s, TFT_RED); };
        auto triangleDown = [&](int16_t x, int16_t y, uint8_t s) {  tft.fillTriangle(x + 0, y + 0, x + 2  *  s, y + 0, x + s, y + 2  *  s, TFT_RED); };

        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        tft.setFont(m_fontSize);
        tft.setTextColor(TFT_ORANGE);
        tft.writeText(m_dlnaHistory[*m_dlnaLevel].name, 10, m_y, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
        tft.setTextColor(TFT_WHITE);
        for(pos = 1; pos < 10; pos++) {
            if(pos == 1 && m_currItemNr > 0) { triangleUp(0, m_y + (pos * m_lineHight), m_lineHight / 3.5); }
            if(pos == 9 && m_currItemNr + 9 < m_dlnaMaxItems - 1) { triangleDown(0, m_y + (pos * m_lineHight), m_lineHight / 3.5); }
            if(pos > 9) break;
            if(pos > itemsSize) break;
            if(*m_dlnaLevel == 0) { tft.writeText(m_dlnaServer.friendlyName[pos - 1], 20, m_y + (pos) * m_lineHight, m_w- 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true); }
            else {
                if(startsWith(m_srvContent.itemURL[pos - 1], "http")) {
                    if(m_srvContent.isAudio[pos - 1] == true) {
                        if(m_srvContent.duration[pos - 1][0] != '?') { sprintf(m_buff, ANSI_ESC_YELLOW "%s" ANSI_ESC_CYAN " (%s)", m_srvContent.title[pos - 1], m_srvContent.duration[pos - 1]); }
                        else { sprintf(m_buff, ANSI_ESC_YELLOW "%s" ANSI_ESC_CYAN " (%li)", m_srvContent.title[pos - 1], (long int)m_srvContent.itemSize[pos - 1]); }
                    }
                    else { sprintf(m_buff, ANSI_ESC_WHITE "%s" ANSI_ESC_CYAN " (%li)", m_srvContent.title[pos - 1], (long int)m_srvContent.itemSize[pos - 1]); }
                }
                else {
                    if(m_srvContent.childCount[pos - 1] == 0) { sprintf(m_buff, ANSI_ESC_WHITE "%s", m_srvContent.title[pos - 1]); }
                    else { sprintf(m_buff, ANSI_ESC_WHITE "%s" ANSI_ESC_CYAN " (%i)", m_srvContent.title[pos - 1], m_srvContent.childCount[pos - 1]); }
                }
                tft.writeText(m_buff, 20, m_y + (pos) * m_lineHight, m_w - 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
            }
        }
        sprintf(m_dlnaItemsPos, "%i-%i/%i", m_currItemNr + 2, m_currItemNr + pos , m_dlnaMaxItems); // shows the current items pos e.g. "30-39/210"
        tft.setTextColor(TFT_ORANGE);
        tft.writeText(m_dlnaItemsPos, 10, m_y, m_w - 10, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true);

        if(m_buff){free(m_buff); m_buff = NULL;}
        return;
    }

    void hasReleased(uint16_t x, uint16_t y){

        bool guard1 = false; if(m_dlnaServer.friendlyName.size() > (m_itemListPos -1)) guard1 = true;
        bool guard2 = false; if(m_srvContent.itemURL.size() >  (m_itemListPos - 1)) guard2 = true;
        bool guard3 = false; if(m_srvContent.title.size() > (m_itemListPos - 1)) guard3 = true;

        if(!m_buff) m_buff = x_ps_malloc(512);
        m_itemListPos = y / (m_h / 10);

        if(m_oldY && (m_oldY + 6 * m_lineHight < y)) {     // fast wipe down
            m_ra.val1 = 0;
            if(m_currItemNr == -1) goto exit;
            if     (m_currItemNr > 36) m_currItemNr -= 36;
            else    m_currItemNr = -1;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if(m_oldY && (m_oldY + 2 * m_lineHight < y)) {     // normal wipe down
            m_ra.val1 = 0;
            if(m_currItemNr == -1) goto exit;
            if(m_currItemNr >  9) m_currItemNr -= 9;
            else m_currItemNr = -1;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if(m_oldY && (m_oldY - 6 * m_lineHight > y)) {     // fast wipe up
            m_ra.val1 = 0;
            if(m_currItemNr + 9 >= m_dlnaMaxItems - 1) goto exit;
            int16_t diff = (m_dlnaMaxItems - 1) - (m_currItemNr + 9);
            if(diff >= 36) m_currItemNr += 36;
            else m_currItemNr += diff;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if(m_oldY && (m_oldY - 2 * m_lineHight > y)) {      // normal wipe up
            m_ra.val1 = 0;
            if(m_currItemNr + 9 >= m_dlnaMaxItems - 1) goto exit;
            m_currItemNr += 9;
            m_browseOnRelease = 4;
            m_chptr = NULL;
            goto exit;
        }

        if(m_itemListPos == 0){ // content list
            m_currItemNr = -1;
            (*m_dlnaLevel) --;
            m_browseOnRelease = 2;
            goto exit;
        }

        if(guard1){ // server list
            if(*m_dlnaLevel == 0){
                m_chptr = m_dlnaServer.friendlyName[m_itemListPos - 1];
                m_currDLNAsrvNr = m_itemListPos - 1;
                (*m_dlnaLevel) ++;
                if(m_dlnaHistory[*m_dlnaLevel].name){free(m_dlnaHistory[*m_dlnaLevel].name); m_dlnaHistory[*m_dlnaLevel].name = NULL;}
                if(m_dlnaServer.friendlyName[m_itemListPos - 1] == NULL){
                    log_e("invalid pointer in dlna history");
                    m_dlnaHistory[*m_dlnaLevel].name = strdup((char*)"dummy");
                    goto exit;
                }
                m_dlnaHistory[*m_dlnaLevel].name = strdup(m_dlnaServer.friendlyName[m_itemListPos - 1]);
                m_browseOnRelease = 1;
                goto exit;
            }
        }

        if(guard2){ // is file
            if(startsWith(m_srvContent.itemURL[m_itemListPos - 1], "http")){
                if(m_srvContent.isAudio[m_itemListPos - 1]){
                    sprintf(m_buff, "%s",m_srvContent.title[m_itemListPos - 1]);
                    m_chptr = m_buff;
                    m_ra.arg1 = m_srvContent.itemURL[m_itemListPos - 1]; // url --> connecttohost()
                    m_ra.arg2 = m_srvContent.title[m_itemListPos - 1];   // filename --> showFileName()
                    if(m_ra.arg1 && m_ra.arg2) m_ra.val1 = 1;
                    m_browseOnRelease = 0;
                    goto exit;
                }
            }
        }

        if(guard3){ // is folder
            m_currItemNr = -1;
            sprintf(m_buff, "%s (%d)",m_srvContent.title[m_itemListPos - 1], m_srvContent.childCount[m_itemListPos - 1]);
            (*m_dlnaLevel) ++;
            m_chptr = m_buff;
            if(m_dlnaHistory[*m_dlnaLevel].objId){free(m_dlnaHistory[*m_dlnaLevel].objId); m_dlnaHistory[*m_dlnaLevel].objId = NULL;}
            m_dlnaHistory[*m_dlnaLevel].objId = strdup(m_srvContent.objectId[m_itemListPos -1]);
            if(m_dlnaHistory[*m_dlnaLevel].name){free(m_dlnaHistory[*m_dlnaLevel].name); m_dlnaHistory[*m_dlnaLevel].name = NULL;}
            m_dlnaHistory[*m_dlnaLevel].name = strdup(m_srvContent.title[m_itemListPos - 1]);
            m_browseOnRelease = 3;
            goto exit;
        }
        // log_i("at this position is nothing to do");
exit:
        return;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class fileList{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int16_t     m_oldX = 0;
    int16_t     m_oldY = 0;
    uint8_t     m_lineHight = 0;
    uint8_t     m_fileListPos = 0;
    uint8_t     m_browseOnRelease = 0;
    uint16_t*   m_curAudioFileNr = NULL;
    uint8_t     m_fontSize = 0;
    uint32_t    m_bgColor = 0;
    bool        m_enabled = false;
    bool        m_clicked = false;
    bool        m_state = false;
    char*       m_name = NULL;
    char*       m_curAudioFolder = NULL;
    char*       m_buff = NULL;
    char*       m_fileItemsPos = NULL;
    releasedArg m_ra;
public:
    fileList(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("fileList");
        m_fileItemsPos = x_ps_malloc(30);
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_ra.arg1 = NULL;
        m_ra.arg2 = NULL;
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~fileList(){
        if(m_name){free(m_name); m_name = NULL;}
        if(m_buff){free(m_buff); m_buff = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fontSize, char* curAudioFolder, uint16_t* curAudioFileNr){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_curAudioFolder = curAudioFolder;
        m_curAudioFileNr = curAudioFileNr;
        m_enabled = false;
    }
    void show(){
        m_browseOnRelease = 0;
        m_clicked = false;
        m_enabled = true;
        audioFileslist();
        ;
    }
    void hide(){
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable(){
        m_enabled = false;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_clicked == false){m_oldX = x; m_oldY = y;}
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y){
        if(!m_enabled) return false;
        if(!m_clicked) return false;

        hasReleased(x - m_x, y - m_y);
        m_clicked = false;

        char* fileName = NULL;

        if(m_browseOnRelease == 1)  {   if(*m_curAudioFileNr + 9 >= _SD_content.getSize()) { goto exit; }       // wipe up
                                        else                       {*m_curAudioFileNr += 9; m_ra.val1 = 1;}
                                    }
        if(m_browseOnRelease == 2)  {   if(*m_curAudioFileNr == 0) { goto exit; }                               // wipe down
                                        if(*m_curAudioFileNr >  9) {*m_curAudioFileNr -= 9; m_ra.val1 = 1;}
                                        else                       {*m_curAudioFileNr  = 0; m_ra.val1 = 1;}
                                    }
        if(m_browseOnRelease == 3)  {   m_curAudioFolder[lastIndexOf(m_curAudioFolder, '/')] = '\0';            // previous folder
                                        *m_curAudioFileNr = 0;
                                        _SD_content.listDir(m_curAudioFolder, true, false);
                                        m_ra.val1 = 1;
                                    }
        if(m_browseOnRelease == 4)  {   *m_curAudioFileNr += m_fileListPos;                                     // next folder
                                        tft.setTextColor(TFT_CYAN);
                                        tft.setFont(m_fontSize);
                                        tft.writeText(_SD_content.getIndex(*m_curAudioFileNr -1), 20, m_y + (m_fileListPos) * m_lineHight, m_w - 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
                                        sprintf(m_buff, "%s",_SD_content.getIndex(*m_curAudioFileNr -1));
                                        strcat(m_curAudioFolder, "/");
                                        strcat(m_curAudioFolder, _SD_content.getIndex(*m_curAudioFileNr - 1));
                                        *m_curAudioFileNr = 0;
                                        _SD_content.listDir(m_curAudioFolder, true, false);
                                        m_ra.val1 = 1;
                                    }
        if(m_browseOnRelease == 5)  {   *m_curAudioFileNr += m_fileListPos;                                     // play file
                                        tft.setTextColor(TFT_CYAN);
                                        tft.setFont(m_fontSize);
                                        tft.writeText(m_buff, 20, m_y + (m_fileListPos) * m_lineHight, m_w - 20, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
                                        fileName = x_ps_strdup(m_buff);
                                        strcpy(m_buff, m_curAudioFolder);
                                        strcat(m_buff, "/");
                                        strcat(m_buff, fileName);
                                        m_ra.arg1 = m_buff;      // filePath
                                        m_ra.arg2 = fileName;    // fileName
                                        m_ra.val1 = 2;
                                    }
    exit:
        m_browseOnRelease = 0;
        m_oldX = 0; m_oldY = 0;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        if(m_buff){free(m_buff); m_buff = NULL;}
        if(fileName){free(fileName); fileName = NULL;}
        m_ra.val1 = 0;
        m_ra.arg1 = NULL;
        return true;
    }
private:
    void audioFileslist(){
        if(!m_curAudioFolder){ log_e("m_curAudioFolder is NULL"); return;}
        if(*m_curAudioFileNr > _SD_content.getSize()) { log_e("m_curAudioFileNr too high"); return;}
        auto triangleUp = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + s, y + 0, x + 0, y + 2 * s, x + 2 * s, y + 2 * s, TFT_RED); };
        auto triangleDown = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + 0, y + 0, x + 2 * s, y + 0, x + s, y + 2 * s, TFT_RED); };

        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        if(_SD_content.getSize() < 10) *m_curAudioFileNr = 0;
        tft.setFont(m_fontSize);
        uint8_t lineHight = m_h / 10;
        tft.setTextColor(TFT_MAGENTA);
        tft.writeText(m_curAudioFolder, 10, m_y, m_w - 10, lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
        tft.setTextColor(TFT_WHITE);
        for(uint8_t pos = 1; pos < 10; pos++) {
            if(pos == 1 && *m_curAudioFileNr > 0) {
                tft.setTextColor(TFT_AQUAMARINE);
                triangleUp(0, m_y + (pos * lineHight), lineHight / 3.5);
            }
            if(pos == 9 && *m_curAudioFileNr + 9 < _SD_content.getSize()) {
                tft.setTextColor(TFT_AQUAMARINE);
                triangleDown(0, m_y + (pos * lineHight), lineHight / 3.5);
            }
            if(*m_curAudioFileNr + pos > _SD_content.getSize()) break;
            if(indexOf(_SD_content.getIndex(pos + *m_curAudioFileNr - 1), "\033[", 0) == -1) tft.setTextColor(TFT_ORANGE); // is folder
            else tft.setTextColor(TFT_WHITE);                                                                       // is file
            tft.writeText(_SD_content.getIndex(pos + *m_curAudioFileNr - 1), 20, m_y + (pos)*lineHight, m_w - 20, lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
        }
        uint16_t firstVal = *m_curAudioFileNr + 1;
        uint16_t secondVal = firstVal + 8;
        if(secondVal > _SD_content.getSize()) secondVal = _SD_content.getSize();
        sprintf(m_fileItemsPos, "%i-%i/%i", firstVal, secondVal , _SD_content.getSize()); // shows the current items pos e.g. "30-39/210"
        tft.setTextColor(TFT_ORANGE);
        tft.writeText(m_fileItemsPos, 10, m_y, m_w - 10, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true);
    }

    void hasReleased(uint16_t x, uint16_t y){
        if(!m_buff) m_buff = x_ps_malloc(1024);
        m_fileListPos = y / (m_h / 10);

        if(m_oldY && (m_oldY - 2 * m_lineHight > y)) {      // normal wipe up
            m_ra.val1 = 0;
            m_browseOnRelease = 1;
            goto exit;
        }

        if(m_oldY && (m_oldY + 2 * m_lineHight < y)) {     // normal wipe down
            m_ra.val1 = 0;
            m_browseOnRelease = 2;
            goto exit;
        }

        if(m_fileListPos == 0) {
            if(lastIndexOf(m_curAudioFolder, '/') > 1){  // not the first '/'
                m_browseOnRelease = 3;
            }
        }
        else {
            if(m_fileListPos + *m_curAudioFileNr > _SD_content.getSize()) goto exit;
            sprintf(m_buff, "%s",_SD_content.getIndex(*m_curAudioFileNr + m_fileListPos -1));
            int32_t idx = indexOf(m_buff, "\033[", 1);
            if(idx == -1){ // is folder
                m_browseOnRelease = 4;
            }
            else {
                m_buff[idx] = '\0';  // remove color and filesize
                m_browseOnRelease = 5;
            }
        }
exit:
        return;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
extern Preferences stations;
class stationsList{
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
    uint16_t    m_maxStations = 0;
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
public:
    stationsList(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("stationsList");
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
    ~stationsList(){
        if(m_name){free(m_name); m_name = NULL;}
        if(m_buff){free(m_buff); m_buff = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fontSize, uint16_t* curStationNr, uint16_t maxStations){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_curSstationNr = curStationNr;
        m_maxStations = maxStations;
        m_enabled = false;
    }
    void show(){
        m_clicked = false;
        m_enabled = true;
        m_browseOnRelease = 0;
        stationslist(true);
    }
    void hide(){
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable(){
        m_enabled = false;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        hasClicked(x - m_x, y - m_y);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;

        if(m_browseOnRelease == 1)  {   stationslist(false);                                             // wipe up
                                    }
        if(m_browseOnRelease == 2)  {   stationslist(false);                                             // wipe down
                                    }
        if(m_browseOnRelease == 3)  {   if(!m_buff) log_e("m_buff is empty");
                                        tft.writeText(m_buff, 10, m_y + (m_stationListPos)*m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
                                        m_ra.val1 = m_firstStationsLineNr + m_stationListPos + 1;   // station number
                                    }
        m_browseOnRelease = 0;
        m_oldX = 0; m_oldY = 0;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        if(m_buff){free(m_buff); m_buff = NULL;}
        m_ra.val1 = 0;
        m_ra.arg1 = NULL;
        return true;
    }
private:
    void stationslist(bool first){
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if(first){
            if(m_maxStations <= 10) m_firstStationsLineNr = 0;
            else  if(*m_curSstationNr < 5) m_firstStationsLineNr = 0;
            else if(*m_curSstationNr +5 <= m_maxStations) m_firstStationsLineNr = *m_curSstationNr - 5;
            else m_firstStationsLineNr = m_maxStations - 10;
        }
        else{
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        char* stationStr = x_ps_malloc(1024);
        tft.setFont(m_fontSize);
        for(uint8_t pos = 0; pos < 10; pos++) {
            if(pos + m_firstStationsLineNr + 1 > m_maxStations) break;
            char staKey[15];
            sprintf(staKey, "station_%03d", pos + m_firstStationsLineNr + 1);
            if((pos + m_firstStationsLineNr + 1) == *m_curSstationNr){
                sprintf(stationStr, ANSI_ESC_YELLOW "%03d " ANSI_ESC_MAGENTA, pos + m_firstStationsLineNr + 1); // is currStationNr
            }
            else{
                sprintf(stationStr, ANSI_ESC_YELLOW "%03d " ANSI_ESC_WHITE, pos + m_firstStationsLineNr + 1);
            }
            stations.getString(staKey, stationStr + strlen(stationStr), 950);
            for(int i = 0; i < strlen(stationStr); i++) {if(stationStr[i] == '#') stationStr[i] = '\0';}
            tft.writeText(stationStr, 10, m_y + (pos)*m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
        }
        xSemaphoreGive(mutex_display);
    }
    void hasClicked(uint16_t x, uint16_t y){
        char staKey[15];
        m_stationListPos = y / (m_h / 10);

        if(m_oldY && (m_oldY + 2 * m_lineHight < y)) { // wipe up
            if(m_browseOnRelease != 1){
                m_browseOnRelease = 1;
            //    log_w("wipe down");
                if(m_firstStationsLineNr == 0) {m_browseOnRelease = 0; return;} // nothing to do
                else if(m_firstStationsLineNr < 10) m_firstStationsLineNr = 0;
                else m_firstStationsLineNr -= 9;
            }
            return;
        }

        if(m_oldY && (m_oldY - 2 * m_lineHight > y)) { // wipe down
            if(m_browseOnRelease != 2){
                m_browseOnRelease = 2;
            //    log_w("wipe up");
                if(m_firstStationsLineNr + 10 >= m_maxStations) {m_browseOnRelease = 0; return;} // nothing to do
                 else m_firstStationsLineNr += 9;
            }
            return;
        }
        if(m_oldX || m_oldY) return;
        m_oldX = x; m_oldY = y;
        if(!m_buff) m_buff = x_ps_malloc(1024);

        tft.setFont(m_fontSize);
        sprintf(staKey, "station_%03d", m_firstStationsLineNr + m_stationListPos + 1);
        sprintf(m_buff, ANSI_ESC_YELLOW "%03d " ANSI_ESC_CYAN, m_firstStationsLineNr + m_stationListPos + 1);
        stations.getString(staKey, m_buff + strlen(m_buff), 950);
        for(int i = 0; i < strlen(m_buff); i++) {if(m_buff[i] == '#') m_buff[i] = '\0';}
        m_browseOnRelease = 3;
        return;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class vuMeter{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint32_t    m_bgColor = TFT_BLACK;
    uint32_t    m_frameColor = TFT_DARKGREY;
    char*       m_name = NULL;
    bool        m_enabled = false;
    bool        m_clicked = false;
    uint8_t     m_VUleftCh = 0;   // VU meter left channel
    uint8_t     m_VUrightCh = 0;  // VU meter right channel
    releasedArg m_ra;
    uint8_t     m_segm_w = 0;
    uint8_t     m_segm_h = 0;
    uint8_t     m_frameSize = 1;
    uint16_t    m_real_w = 0;
    uint16_t    m_real_h = 0;
public:
    vuMeter(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("vuMeter");
        m_bgColor = TFT_BLACK;
    }
    ~vuMeter(){
        if(m_name){free(m_name); m_name = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t real_w, uint16_t real_h){
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
        m_w = 2 *  m_segm_w  +  3 * m_frameSize;
        m_h = 12 * m_segm_h + 13 * m_frameSize;
    }
    void show(){
        m_enabled = true;
        m_clicked = false;
        tft.fillRect(m_x, m_y, m_real_w, m_real_h, m_bgColor);
        tft.drawRect(m_x, m_y, m_w, m_h, m_frameColor);
        for(uint8_t i = 0; i < 12; i++){
            drawRect(i, 0, 0);
            drawRect(i, 1, 0);
        }
        m_VUleftCh = 0;
        m_VUrightCh = 0;
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setBGcolor(uint32_t color){
        m_bgColor = color;
    }
    void update(uint16_t vum){
        if(!m_enabled) return;
        uint8_t left = map_l(vum >> 8, 0, 127, 0, 11);
        uint8_t right = map_l(vum & 0x00FF, 0, 127, 0, 11);

        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if(left > m_VUleftCh) {
            for(int32_t i = m_VUleftCh; i < left; i++) { drawRect(i, 1, 1); }
        }
        if(left < m_VUleftCh) {
            for(int32_t i = left; i < m_VUleftCh; i++) { drawRect(i, 1, 0); }
        }
        m_VUleftCh = left;

        if(right > m_VUrightCh) {
            for(int32_t i = m_VUrightCh; i < right; i++) { drawRect(i, 0, 1); }
        }
        if(right < m_VUrightCh) {
            for(int32_t i = right; i < m_VUrightCh; i++) { drawRect(i, 0, 0); }
        }
        m_VUrightCh = right;
        xSemaphoreGive(mutex_display);
    }

    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
    void drawRect(uint8_t pos, uint8_t ch, bool br) {
        uint16_t color = 0;
        uint16_t y_end = m_y + m_h -m_frameSize - m_segm_h;
        uint16_t xPos = m_x + m_frameSize + ch * (m_segm_w + m_frameSize);
        uint16_t yPos = y_end - pos * (m_frameSize + m_segm_h);
        if(pos > 11) return;
        switch(pos) {
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
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayHeader{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    uint8_t     m_fontSize = 0;
    uint8_t     m_volume = 0;
    uint32_t    m_bgColor = TFT_BLACK;
    char*       m_name = NULL;
    char*       m_item = NULL;
    char        m_time[10] = {0};
    bool        m_enabled = false;
    bool        m_clicked = false;
    releasedArg m_ra;
    uint16_t    m_itemColor = TFT_GREENYELLOW;
    uint16_t    m_volumeColor = TFT_DEEPSKYBLUE;
    uint16_t    m_timeColor = TFT_GREENYELLOW;
#if TFT_CONTROLLER < 2 // 320 x 240px
    uint16_t    m_item_x = 6;
    uint16_t    m_item_w = 174;
    uint16_t    m_volume_x = 180;
    uint16_t    m_volume_w = 50;
    uint16_t    m_time_x = 260;
    uint16_t    m_time_w = 60;
    uint8_t     m_time_pos[8] = {0, 9, 18, 21, 30, 39, 42, 51};  // display 320x240
    uint8_t     m_time_ch_w = 9;
#else // 480 x 320px
    uint16_t    m_item_x = 6;
    uint16_t    m_item_w = 274;
    uint16_t    m_volume_x = 280;
    uint16_t    m_volume_w = 100;
    uint16_t    m_time_x = 380;
    uint16_t    m_time_w = 100;
    uint8_t     m_time_pos[8] = {7, 20, 33, 40, 53, 66, 73, 86}; // display 480x320
    uint8_t     m_time_ch_w = 13;
#endif
public:
    displayHeader(const char* name, uint8_t fontSize){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("displayHeader");
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
    }
    ~displayHeader(){
        if(m_name){free(m_name); m_name = NULL;}
        if(m_item){free(m_item); m_item = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
    }
    void show(){
        m_enabled = true;
        m_clicked = false;
        if(m_item) updateItem(m_item);
        else       updateItem("");
        updateVolume(m_volume);
        updateTime("        ", true);
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setBGcolor(uint32_t color){
        m_bgColor = color;
    }
    void updateItem(const char* hl_item){// radio, clock, audioplayer...
        if(!m_enabled) return;
        if(!hl_item) log_e("hl_item is NULL");
        char* tmp = strdup(hl_item);
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.setFont(m_fontSize);
        tft.setTextColor(m_itemColor);
        tft.fillRect(m_item_x, m_y, m_item_w, m_h, m_bgColor);
        tft.writeText(hl_item, m_item_x, m_y, m_item_w, m_h);
        if(m_item){free(m_item); m_item = NULL;}
        m_item = strdup(tmp);
        if(tmp){free(tmp); tmp = NULL;}
        xSemaphoreGive(mutex_display);
    }
    void setItemColor(uint16_t itemColor){
        m_itemColor = itemColor;
    }
    void updateVolume(uint8_t vol){
        m_volume = vol;
        if(!m_enabled) return;
        char buff[15];
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.setFont(m_fontSize);
        tft.setTextColor(m_volumeColor);
        tft.fillRect(m_volume_x, m_y, m_volume_w, m_h, m_bgColor);
        sprintf(buff, "Vol %d", m_volume);
        tft.writeText(buff, m_volume_x, m_y, m_volume_w, m_h);
        xSemaphoreGive(mutex_display);
    }
    void setVolumeColor(uint16_t volColor){
        m_volumeColor = volColor;
        updateVolume(m_volume);
    }
    void updateTime(const char* hl_time, bool complete = true){
        if(!m_enabled) return;
        memcpy(m_time, hl_time, 8); // hhmmss
        static char oldtime[8] = {255}; // hhmmss
        uint8_t*    pos = NULL;
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.setFont(m_fontSize);
        tft.setTextColor(m_timeColor);
        if(complete == true) {
            tft.fillRect(m_time_x, m_y, m_time_w, m_h, m_bgColor);
            for(uint8_t i = 0; i < 8; i++) { oldtime[i] = 255; }
        }
        for(uint8_t i = 0; i < 8; i++) {
            if(oldtime[i] != m_time[i]) {
                char ch[2] = {0, 0};
                ch[0] = m_time[i];
                pos = m_time_pos;
                tft.fillRect(m_time_x + pos[i], m_y, m_time_ch_w, m_h, TFT_BLACK);
                tft.writeText(ch, m_time_x + pos[i], m_y, m_time_ch_w, m_h, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true);
                oldtime[i] = m_time[i];
            }
        }
        xSemaphoreGive(mutex_display);
    }
    void setTimeColor(uint16_t timeColor){
        m_timeColor = timeColor;
        updateTime(m_time, true);
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayFooter{
private:
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    int8_t      m_rssi = 0;
    int8_t      m_old_rssi = -1;
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
    releasedArg m_ra;
    const char  m_rssiSymbol[5][18]     = {"/common/RSSI0.bmp", "/common/RSSI1.bmp", "/common/RSSI2.bmp", "/common/RSSI3.bmp", "/common/RSSI4.bmp"};
    const char  m_stationSymbol[16]     = "/common/STA.bmp";
    const char  m_hourGlassymbol[2][27] = {"/common/Hourglass_blue.bmp", "/common/Hourglass_red.bmp"};
#if TFT_CONTROLLER < 2 // 320 x 240px
    uint16_t    m_staSymbol_x = 0;
    uint16_t    m_staNr_x = 25, m_staNr_w = 35;
    uint16_t    m_offTimerSymbol_x = 60;
    uint16_t    m_offTimerNr_x = 88, m_offTimerNr_w = 37;
    uint16_t    m_rssiSymbol_x = 125;
    uint16_t    m_rssiSymbol_w = 24;
    uint16_t    m_bitRate_x = 150, m_bitRate_w = 40;
    uint16_t    m_ipAddr_x = 190, m_ipAddr_w = 130;
#else // 480 x 320px
    uint16_t    m_staSymbol_x = 0;
    uint16_t    m_staNr_x = 33, m_staNr_w = 52;
    uint16_t    m_offTimerSymbol_x = 85;
    uint16_t    m_offTimerNr_x = 118, m_offTimerNr_w = 54;
    uint16_t    m_rssiSymbol_x = 172;
    uint16_t    m_rssiSymbol_w = 32;
    uint16_t    m_bitRate_x = 204, m_bitRate_w = 66;
    uint16_t    m_ipAddr_x = 270, m_ipAddr_w = 210;
#endif
public:
    displayFooter(const char* name, uint8_t fontSize){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("displayFooter");
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
    }
    ~displayFooter(){
        if(m_name)  {free(m_name);   m_name = NULL;}
        if(m_ipAddr){free(m_ipAddr); m_ipAddr = NULL;}
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
    }
    void show(){
        m_enabled = true;
        m_clicked = false;
        m_old_rssi = -1;
        drawImage(m_stationSymbol, m_staSymbol_x, m_y);
        updateStation(m_staNr);
        updateOffTime(m_offTime);
        updateRSSI(m_rssi);
        updateBitRate(m_bitRate);
        if(m_ipAddr) setIpAddr(m_ipAddr);
        else         setIpAddr("");
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setBGcolor(uint32_t color){
        m_bgColor = color;
    }
    void updateStation(uint16_t staNr){// radio, clock, audioplayer...
        m_staNr = staNr;
        if(!m_enabled) return;
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.fillRect(m_staNr_x, m_y, m_staNr_w, m_h, m_bgColor);
        tft.setFont(m_fontSize);
        tft.setTextColor(m_stationColor);
        char buff[10];
        sprintf(buff, "%03d", staNr);
        tft.writeText(buff, m_staNr_x, m_y, m_staNr_w, m_h);
        xSemaphoreGive(mutex_display);
    }
    void setStationNrColor(uint16_t stationColor){
        m_stationColor = stationColor;
    }
    void updateOffTime(uint16_t offTime){
        m_offTime = offTime;
        if(!m_enabled) return;
        char buff[15];
        sprintf(buff, "%d:%02d", m_offTime / 60, m_offTime % 60);
        tft.setFont(m_fontSize);
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if(m_offTime){
            tft.setTextColor(TFT_RED);
            drawImage(m_hourGlassymbol[1], m_offTimerSymbol_x, m_y);
        }
        else{
            tft.setTextColor(TFT_DEEPSKYBLUE);
            drawImage(m_hourGlassymbol[0], m_offTimerSymbol_x, m_y);
        }
        tft.fillRect(m_offTimerNr_x, m_y, m_offTimerNr_w, m_h, m_bgColor);
        tft.writeText(buff, m_offTimerNr_x, m_y, m_offTimerNr_w, m_h);
        xSemaphoreGive(mutex_display);
    }
    void updateRSSI(int8_t rssi, bool show = false){
        static int32_t old_rssi = -1;
        int8_t new_rssi = -1;
        if(rssi >= 0) return;
        m_rssi = rssi;
        if(m_rssi < -1)  new_rssi = 4;
        if(m_rssi < -50) new_rssi = 3;
        if(m_rssi < -65) new_rssi = 2;
        if(m_rssi < -75) new_rssi = 1;
        if(m_rssi < -85) new_rssi = 0;

        if(new_rssi != old_rssi) {
            old_rssi = new_rssi; // no need to draw a rssi icon if rssiRange has not changed
            if(ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO) {
                static int32_t tmp_rssi = 0;
                if((abs(rssi - tmp_rssi) > 4)) { SerialPrintfln("WiFI_info:   RSSI is " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " dB", rssi); }
                tmp_rssi = rssi;
            }
            if(m_enabled) show = true;
        }
        if(show) {
            xSemaphoreTake(mutex_display, portMAX_DELAY);
            drawImage(m_rssiSymbol[new_rssi], m_rssiSymbol_x, m_y + 2);
            xSemaphoreGive(mutex_display);
        }
    }
    void updateTC(uint8_t timeCounter){
        m_timeCounter = timeCounter;
        if(!m_enabled) return;
        if(!m_timeCounter) {
            tft.fillRect(m_rssiSymbol_x, m_y, m_rssiSymbol_w, m_h, m_bgColor);
            updateRSSI(m_rssi, true);
        }
        else{
            uint16_t x0   = m_rssiSymbol_x;
            uint16_t x1x2 = round(m_rssiSymbol_x + ((float)((m_rssiSymbol_w) / 10) * timeCounter));
            uint16_t y0y1 = m_y + m_h - 5;
            uint16_t y2   = round((m_y  + m_h - 5) - ((float)(m_h - 6) / 10) * timeCounter);
            tft.fillRect(m_rssiSymbol_x, m_y, m_rssiSymbol_w, m_h, m_bgColor);
            tft.fillTriangle(x0, y0y1, x1x2, y0y1, x1x2, y2, TFT_RED);
        }
    }

    void updateBitRate(uint32_t bitRate){
        m_bitRate = bitRate;
        if(!m_enabled) return;
        char sbr[10];
        itoa(bitRate, sbr, 10);
        if(m_bitRate < 1000) { strcat(sbr, "K"); }
        else {
            sbr[2] = sbr[1];
            sbr[1] = '.';
            sbr[3] = 'M';
            sbr[4] = '\0';
        }
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.setFont(m_fontSize);
        tft.setTextColor(m_bitRateColor);
        tft.fillRect(m_bitRate_x, m_y, m_bitRate_w, m_h, m_bgColor);
        uint8_t space = 2;
        if(strlen(sbr) < 4) space += 5;
        tft.writeText(sbr, m_bitRate_x + space, m_y, m_bitRate_w, m_h, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        xSemaphoreGive(mutex_display);
    }
    void setBitRateColor(uint16_t bitRateColor){
        m_bitRateColor = bitRateColor;
    }
    void setIpAddr(const char* ipAddr){
        if(m_ipAddr){free(m_ipAddr); m_ipAddr = NULL;}

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wuse-after-free"
        m_ipAddr = strdup(ipAddr);
        char myIP[30] = "IP:";
        strcpy(myIP + 3, ipAddr);
        #pragma GCC diagnostic pop

        tft.setFont(m_fontSize);
        tft.setTextColor(m_ipAddrColor);
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        tft.fillRect(m_ipAddr_x, m_y, m_ipAddr_w, m_h, m_bgColor);
        tft.writeText(myIP, m_ipAddr_x, m_y, m_ipAddr_w, m_h, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true);
        xSemaphoreGive(mutex_display);
    }
    void setIpAddrColor(uint16_t ipAddrColor){
        m_ipAddrColor = ipAddrColor;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_enabled) m_clicked = true;
        uint8_t pos = 0;
        if(x >= m_rssiSymbol_x && x <= m_rssiSymbol_x + m_rssiSymbol_w) pos = 3; // RSSI or TC
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, pos);
        if(!m_enabled) return false;
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        return true;
    }
private:
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void GetRunTimeStats( char *pcWriteBuffer ){
    // TaskStatus_t *pxTaskStatusArray;
    // volatile UBaseType_t uxArraySize, x;
    // uint32_t ulTotalRunTime, ulStatsAsPercentage;
    // char leftSpace[] = "             ";

    // // Make sure the write buffer does not contain a string.
    // *pcWriteBuffer = 0x00;

    // // Take a snapshot of the number of tasks in case it changes while this function is executing.
    // uxArraySize = uxTaskGetNumberOfTasks();

    // // Allocate a TaskStatus_t structure for each task.  An array could be allocated statically at compile time.
    // pxTaskStatusArray = (TaskStatus_t*)pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

    // if( pxTaskStatusArray != NULL ) {
    // // Generate raw status information about each task.
    //     uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, (UBaseType_t)uxArraySize, &ulTotalRunTime );

    //     // For percentage calculations.
    //     ulTotalRunTime /= 100UL;

    //     char* tmpBuff = (char*) malloc(100);
    //     strcpy(pcWriteBuffer, leftSpace);
    //     strcat(pcWriteBuffer, ANSI_ESC_YELLOW "TASKNAME            | RUNTIMECOUNTER | TOTALRUNTIME[%] | CORE | PRIO  |\n");
    //     strcat(pcWriteBuffer, leftSpace);
    //     strcat(pcWriteBuffer,                 "--------------------+----------------+-----------------+------+-------|\n");

    //     // Avoid divide by zero errors.
    //     if(ulTotalRunTime > 0){
    //         // For each populated position in the pxTaskStatusArray array, format the raw data as human readable ASCII data
    //         for( x = 0; x < uxArraySize; x++ ) {
    //             // What percentage of the total run time has the task used? This will always be rounded down to the nearest integer.
    //             // ulTotalRunTimeDiv100 has already been divided by 100.
    //             ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime;
    //             memset(tmpBuff, 0x20, 100);
    //             memcpy(tmpBuff, pxTaskStatusArray[x].pcTaskName, strlen(pxTaskStatusArray[x].pcTaskName));
    //             tmpBuff[20] = '|';
    //             uint8_t core = (pxTaskStatusArray[x].xCoreID);
    //             uint8_t prio = (pxTaskStatusArray[x].uxBasePriority);
    //             if(ulStatsAsPercentage){
    //                 sprintf(tmpBuff + 23, "%12lu  |       %02lu%%       |%4d  |%5d  |", (long unsigned int)pxTaskStatusArray[ x ].ulRunTimeCounter, (long unsigned int)ulStatsAsPercentage, core, prio);
    //             }
    //             else{
    //                 sprintf(tmpBuff + 23, "%12lu  |       <1%%       |%4d  |%5d  |", (long unsigned int)pxTaskStatusArray[ x ].ulRunTimeCounter, core, prio);
    //             }
    //             uint8_t i = 23; while(tmpBuff[i] == '0') {tmpBuff[i] = ' '; i++;}
    //             if(tmpBuff[45] == '0') tmpBuff[45] = ' ';
    //             strcat(pcWriteBuffer, leftSpace);
    //             strcat(pcWriteBuffer, tmpBuff);
    //             strcat(pcWriteBuffer, "\n");
    //         }
    //         free(tmpBuff);
    //        }
    //     // The array is no longer needed, free the memory it consumes.
    //     vPortFree( pxTaskStatusArray );
    // }
}