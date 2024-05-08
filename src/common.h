// created: 10.Feb.2022
// updated: 24.Apr 2024

#pragma once
#pragma GCC optimize("Os") // optimize for code size
// clang-format off
#define _SSID               "mySSID"                        // Your WiFi credentials here
#define _PW                 "myWiFiPassword"                // Or in textfile on SD-card
#define DECODER             1                               // (1)MAX98357A PCM5102A CS4344... (2)AC101, (3)ES8388, (4)WM8978
#define TFT_CONTROLLER      4                               // (0)ILI9341, (1)HX8347D, (2)ILI9486a, (3)ILI9486b, (4)ILI9488, (5)ST7796, (6)ST7796RPI
#define DISPLAY_INVERSION   0                               // (0) off (1) on
#define TFT_ROTATION        1                               // 1 or 3 (landscape)
#define TFT_FREQUENCY       40000000                        // 80000000, 40000000, 27000000, 20000000, 10000000
#define TP_VERSION          4                               // (0)ILI9341, (1)ILI9341RPI, (2)HX8347D, (3)ILI9486, (4)ILI9488, (5)ST7796, (3)ST7796RPI
#define TP_ROTATION         1                               // 1 or 3 (landscape)
#define TP_H_MIRROR         0                               // (0) default, (1) mirror up <-> down
#define TP_V_MIRROR         0                               // (0) default, (1) mittor left <-> right
#define AUDIOTASK_CORE      0                               // 0 or 1
#define AUDIOTASK_PRIO      2                               // 0 ... 24  Priority of the Task (0...configMAX_PRIORITIES -1)
#define I2S_COMM_FMT        0                               // (0) MAX98357A PCM5102A CS4344, (1) LSBJ (Least Significant Bit Justified format) PT8211
#define SDMMC_FREQUENCY     80000000                        // 80000000, 40000000, 27000000, 20000000, 10000000 not every SD Card will run at 80MHz
#define FTP_USERNAME        "esp32"                         // user and pw in FTP Client
#define FTP_PASSWORD        "esp32"
#define CONN_TIMEOUT        500                             // unencrypted connection timeout in ms (http://...)
#define CONN_TIMEOUT_SSL    2000                            // encrypted connection timeout in ms (https://...)

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
#include <WiFiMulti.h>
#include <vector>
#include "index.h"
#include "index.js.h"
#include "accesspoint.h"
#include "websrv.h"
#include "rtime.h"
#include "IR.h"
#include "tft.h"
#include "ESP32FtpServer.h"
#include "AC101.h"
#include "ES8388.h"
#include "WM8978.h"
#include "DLNAClient.h"
#include "KCX_BT_Emitter.h"

extern TFT tft;

#ifdef CONFIG_IDF_TARGET_ESP32
    // Digital I/O used
        #define TFT_CS           22
        #define TFT_DC           21
        #define TFT_BL           32  // at -1 the brightness menu is not displayed
        #define TP_IRQ           39  // VN
        #define TP_CS             5
        #define SD_MMC_D0         2  // cannot be changed
        #define SD_MMC_CLK       14  // cannot be changed
        #define SD_MMC_CMD       15  // cannot be changed
        #define IR_PIN           35  // IR Receiver (if available)
        #define TFT_MOSI         23  // TFT and TP (VSPI)
        #define TFT_MISO         19  // TFT and TP (VSPI)
        #define TFT_SCK          18  // TFT and TP (VSPI)

        #define I2S_DOUT         25
        #define I2S_BCLK         27
        #define I2S_LRC          26
        #define I2S_MCLK          0  // mostly not used

        #define I2C_DATA         -1  // some DACs are controlled via I2C
        #define I2C_CLK          -1
        #define SD_DETECT        -1  // some pins on special boards: Lyra, Olimex, A1S ...
        #define HP_DETECT        -1
        #define AMP_ENABLED      -1

        #define BT_EMITTER_RX    33  // TX pin - KCX Bluetooth Transmitter (-1 if not available)
        #define BT_EMITTER_TX    36  // RX pin - KCX Bluetooth Transmitter (-1 if not available)
        #define BT_EMITTER_LINK  34  // high if connected                  (-1 if not available)
        #define BT_EMITTER_MODE  13  // high transmit - low receive        (-1 if not available)
#endif

#ifdef CONFIG_IDF_TARGET_ESP32S3
    // Digital I/O used
        #define TFT_CS            8
        #define TFT_DC           12
        #define TFT_BL           10 // at -1 the brightness menu is not displayed
        #define TP_IRQ           39
        #define TP_CS            15
        #define SD_MMC_D0        11
        #define SD_MMC_CLK       13
        #define SD_MMC_CMD       14
        #define IR_PIN            4  // IR Receiver (if available)
        #define TFT_MOSI         18  // TFT and TP (FSPI)
        #define TFT_MISO          2  // TFT and TP (FSPI)
        #define TFT_SCK          17  // TFT and TP (FSPI)

        #define I2S_DOUT          9
        #define I2S_BCLK          3
        #define I2S_LRC           1
        #define I2S_MCLK          0

        #define I2C_DATA         -1  // some DACs are controlled via I2C
        #define I2C_CLK          -1
        #define SD_DETECT        -1  // some pins on special boards: Lyra, Olimex, A1S ...
        #define HP_DETECT        -1
        #define AMP_ENABLED      -1

        #define BT_EMITTER_RX    45  // TX pin - KCX Bluetooth Transmitter (-1 if not available)
        #define BT_EMITTER_TX    38  // RX pin - KCX Bluetooth Transmitter (-1 if not available)
        #define BT_EMITTER_LINK  19  // high if connected                  (-1 if not available)
        #define BT_EMITTER_MODE  20  // high transmit - low receive        (-1 if not available)

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
                            Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.printf("\033[0m"); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

#define SerialPrintfcr(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
 /* carriage return */      Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.printf("\033[0m"); \
                            Serial.print("  \r"); \
                            _newLine = true; \
                            xSemaphoreGive(mutex_rtc);}
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
void           showHeadlineVolume();
void           showHeadlineTime(bool complete = true);
void           showHeadlineItem(uint8_t idx);
void           showFooterIPaddr();
void           showFooterStaNr();
void           showFooterRSSI(boolean show = false);
void           fall_asleep();
void           wake_up();
void           setRTC(const char* TZString);
void           vector_clear_and_shrink(vector<char*>& vec);
boolean        copySDtoFFat(const char* path);
void           updateSleepTime(boolean noDecrement = false);
void           showVolumeBar();
void           showBrightnessBar();
void           showFooter();
void           display_info(const char* str, int32_t xPos, int32_t yPos, uint16_t color, uint16_t margin_l, uint16_t margin_r, uint16_t winWidth, uint16_t winHeight);
void           showStreamTitle(const char* streamTitle);
void           showVUmeter();
void           hideVUmeter();
void           updateVUmeter();
void           showLogoAndStationName();
void           showStationName(String sn);
void           showStationLogo(String ln);
void           showFileLogo(uint8_t state);
void           showFileName(const char* fname);
void           showPlsFileNumber();
void           showAudioFileNumber();
void           showStationsList(uint16_t staListNr);
void           display_sleeptime(int8_t ud = 0);
boolean        drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
bool           SD_listDir(const char* path, boolean audioFilesOnly, boolean withoutDirs);
boolean        isAudio(File file);
boolean        isAudio(const char* path);
boolean        isPlaylist(File file);
bool           connectToWiFi();
void           openAccessPoint();
const char*    scaleImage(const char* path);
void           setVolume(uint8_t vol);
inline uint8_t getvolume();
uint8_t        downvolume();
uint8_t        upvolume();
void           setStation(uint16_t sta);
void           nextStation();
void           prevStation();
void           StationsItems();
void           setStationViaURL(const char* url);
void           changeBtn_pressed(uint8_t btnNr);
void           changeBtn_released(uint8_t btnNr);
void           savefile(const char* fileName, uint32_t contentLength);
String         setI2STone();
void           SD_playFile(const char* path, const char* fileName);
void           SD_playFile(const char* path, uint32_t resumeFilePos = 0, bool showFN = true);
bool           SD_rename(const char* src, const char* dest);
bool           SD_newFolder(const char* folderPathName);
bool           SD_delete(const char* itemPath);
bool           preparePlaylistFromFile(const char* path);
bool           preparePlaylistFromFolder(const char* path);
void           sortPlayListAlphabetical();
void           sortPlayListRandom();
void           processPlaylist(boolean first = false);
void           changeState(int32_t state);
void           connecttohost(const char* host);
void           connecttoFS(const char* filename, uint32_t resumeFilePos = 0);
void           stopSong();
void IRAM_ATTR headphoneDetect();
void           showDlnaItemsList(uint16_t itemListNr, const char* parentName);
void           placingGraphicObjects();
void           muteChanged(bool m);

//prototypes (audiotask.cpp)
void           audioInit();
void           audioTaskDelete();
void           audioSetVolume(uint8_t vol);
uint8_t        audioGetVolume();
uint32_t       audioGetBitRate();
boolean        audioConnecttohost(const char* host, const char* user = "", const char* pwd = "");
boolean        audioConnecttoFS(const char* filename, uint32_t resumeFilePos = 0);
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
void           audioMute(uint8_t vol);

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
        log_e("map(): Invalid input range, min == max");
        return -1; // AVR returns -1, SAM returns 0
    }
    const int32_t rise = out_max - out_min;
    const int32_t delta = x - in_min;
    return (delta * rise) / run + out_min;
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

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     G R A P H I C   O B J E C T S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

extern __attribute__((weak)) void graphicObjects_OnChange(const char* name, int32_t arg1);
extern __attribute__((weak)) void graphicObjects_OnClick(const char* name);
extern __attribute__((weak)) void graphicObjects_OnRelease(const char* name);
class slider{
private:
    int16_t m_x = 0;
    int16_t m_y = 0;
    int16_t m_w = 0;
    int16_t m_h = 0;
    int16_t m_val = 0;
    int16_t m_minVal = 0;
    int16_t m_maxVal = 0;
    uint16_t m_leftStop = 0;
    uint16_t m_rightStop = 0;
    uint32_t m_bgColor = 0;
    uint32_t m_railColor = 0;
    uint32_t m_spotColor = 0;
    bool     m_enabled = false;
    bool     m_clicked = false;
    uint8_t  m_railHigh = 0;
    uint16_t m_middle_h = 0;
    uint16_t m_spotPos = 0;
    uint8_t  m_spotRadius = 0;
    char*    m_name = NULL;
public:
    slider(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("textbox");
        m_railHigh = 6;
        m_spotRadius = 12;
        m_bgColor = TFT_BLACK;
        m_railColor = TFT_BEIGE;
        m_spotColor = TFT_RED;
    }
    ~slider(){
        ;
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
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(!m_enabled)  return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;

        // (x, y) is in range
        if(x < m_leftStop) x = m_leftStop;
        if(x > m_rightStop) x = m_rightStop;
        if(!m_clicked){ if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);}
        m_clicked = true;
        drawNewSpot(x);
        return true;
    }
    void setValue(int16_t val){
        if(val < m_minVal) val = m_minVal;
        if(val > m_maxVal) val = m_maxVal;
        m_val = map_l(val, m_minVal, m_maxVal, m_leftStop, m_rightStop); // val -> x
        drawNewSpot(m_val);
    }
    int16_t getValue(){
        return map_l(m_spotPos, m_leftStop, m_rightStop, m_minVal, m_maxVal); // xPos -> val
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
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        return true;
    }
private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if(run == 0) {
            log_e("map(): Invalid input range, min == max");
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
        if(graphicObjects_OnChange) graphicObjects_OnChange((const char*)m_name, val);
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbox{
private:
    int16_t m_x = 0;
    int16_t m_y = 0;
    int16_t m_w = 0;
    int16_t m_h = 0;
    uint8_t m_fontSize = 0;
    uint32_t m_bgColor = 0;
    uint32_t m_fgColor = 0;
    char* m_text = NULL;
    char* m_name = NULL;
    bool  m_enabled = false;
    bool  m_clicked = false;
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
    }
    void show(){
        m_enabled = true;
        m_clicked = false;
        if(!m_text){char c[] = " "; m_text = c;}
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        tft.setTextColor(m_fgColor);
        tft.setFont(m_fontSize);
        uint8_t offset_v = 0;
        if(m_fontSize < m_h) offset_v = (m_h - m_fontSize) / 2;
        tft.writeText(m_text, m_x, m_y + offset_v, m_w, m_h, TFT_ALIGN_RIGHT);
    }
    void hide(){
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable(){
        m_enabled = false;
    }
    void setFont(uint8_t size){
        m_fontSize = size;
        tft.setFont(m_fontSize);
    }
    void setBGcolor(uint32_t color){
        m_bgColor = color;
    }
    bool positionXY(uint16_t x, uint16_t y){
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        return true;
    }
    void writeText(const char* txt){
        if(m_text){free(m_text); m_text = NULL;}
        m_text = x_ps_strdup(txt);
        if(m_enabled){
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            tft.setTextColor(m_fgColor);
            tft.setFont(m_fontSize);
            uint8_t offset_v = 0;
            if(m_fontSize < m_h) offset_v = (m_h - m_fontSize) / 2;
            tft.writeText(m_text, m_x, m_y + offset_v, m_w, m_h, TFT_ALIGN_RIGHT);
        }
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button1state{ // click button
private:
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    uint32_t m_bgColor = 0;
    char*    m_defaultPicturePath = NULL;
    char*    m_clickedPicturePath = NULL;
    char*    m_inactivePicturePath = NULL;
    bool     m_enabled = false;
    bool     m_clicked = false;
    char*    m_name = NULL;
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
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        return true;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button2state{ // on off switch
private:
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    uint32_t m_bgColor = 0;
    char*    m_offPicturePath = NULL;
    char*    m_onPicturePath = NULL;
    char*    m_clickedOffPicturePath = NULL;
    char*    m_clickedOnPicturePath = NULL;
    bool     m_enabled = false;
    bool     m_clicked = false;
    bool     m_state = false;
    char*    m_name = NULL;
public:
    button2state(const char* name){
        if(name) m_name = x_ps_strdup(name);
        else     m_name = x_ps_strdup("button2state");
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        setOffPicturePath(NULL);
        setClickedOffPicturePath(NULL);
        setClickedOnPicturePath(NULL);
        setOnPicturePath(NULL);
    }
    ~button2state(){
        if(m_offPicturePath) {free(m_offPicturePath);  m_offPicturePath = NULL;}
        if(m_onPicturePath) {free(m_onPicturePath);  m_onPicturePath = NULL;}
        if(m_clickedOffPicturePath){free(m_clickedOffPicturePath); m_clickedOffPicturePath = NULL;}
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
    // void setInactive(){
    //     drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
    //     m_enabled = false;
    // }
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
    // void setInactivePicturePath(const char* path){
    //     if(m_inactivePicturePath){free(m_clickedPicturePath); m_clickedPicturePath = NULL;}
    //     if(path) m_inactivePicturePath = x_ps_strdup(path);
    //     else m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    // }
    bool positionXY(uint16_t x, uint16_t y){
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        if(m_state) drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
        else        drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = true;
        m_state = !m_state;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        if(m_state) drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
        else drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        return true;
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class pictureBox{
private:
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    uint32_t m_bgColor = 0;
    char*    m_PicturePath = NULL;
    char*    m_altPicturePath = NULL;
    char*    m_name = NULL;
    bool     m_enabled = false;
    bool     m_clicked = false;
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
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        m_clicked = false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        return true;
    }
private:
    bool GetImageSize(const char* picturePath){
        const char* scaledPicPath = scaleImage(picturePath);
        if(!SD_MMC.exists(scaledPicPath)) {log_w("file %s not exists", scaledPicPath); return false;}
        File file = SD_MMC.open(scaledPicPath,"r", false);
        if(file.size() < 24) {log_w("file %s is too small", scaledPicPath); file.close(); return false;}
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
    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
#if TFT_CONTROLLER < 2
    uint16_t m_timeXPos7S[5] = {2, 75, 173, 246, 148}; // seven segment digits "hhmm:"
    uint16_t m_timeXPosFN[6] = {0, 56, 152, 208, 264, 112}; // folded numbers
#else
    uint16_t m_timeXPos7S[5] = {12, 118, 266, 372, 224}; // seven segment digits "hhmm:""
    uint16_t m_timeXPosFN[6] = {16, 96,  224, 304, 384, 176}; // folded numbers
#endif
    uint16_t m_minuteOfTheDay = 0;
    uint32_t m_bgColor = 0;
    bool     m_enabled = false;
    bool     m_clicked = false;
    bool     m_state = false;
    uint8_t  m_timeFormat = 24;
    bool     m_showAll = false;
    char*    m_name = NULL;
    char*    m_pathBuff = NULL;
    uint8_t  m_min = 0, m_hour = 0, m_weekday = 0;
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
        m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);
        return true;
    }
    bool released(){
        if(!m_enabled) return false;
        if(!m_clicked) return false;
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
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
    uint8_t  m_fontSize = 16
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
        if(!m_enabled) return false;
        if(x < m_x) return false;
        if(y < m_y) return false;
        if(x > m_x + m_w) return false;
        if(y > m_y + m_h) return false;
        m_clicked = true;
        if(graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name);

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
        if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name);
        if(m_btnAlarmDay >= 0){
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmDay;
            log_w("mask %i, m_alarmDays %i", mask, *m_alarmDays);
            *m_alarmDays ^= mask;    // toggle the bit
            log_w("m_alarmDays %i", *m_alarmDays);
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
            tft.writeText(hhmm, m_alarmdaysXPos[idx], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, TFT_ALIGN_CENTER, true);
        }
    }
};
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
