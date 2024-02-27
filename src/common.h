// created: 10.Feb.2022
// updated: 08.Feb 2024

#pragma once
#pragma GCC optimize("Os") // optimize for code size

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
#define BT_SINK_NAME        "MiniWebRadio"                  // ESP32 only

/**********************************************************************************************************************/

#include <Arduino.h>
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

#if CONFIG_IDF_TARGET_ESP32 == 1
#include "a2dp_sink.h"
#endif


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
        #define IR_PIN           -1  // IR Receiver (if available)
        #define TFT_MOSI         23  // TFT and TP (VSPI)
        #define TFT_MISO         35  // TFT and TP (VSPI)
        #define TFT_SCK          12  // TFT and TP (VSPI)

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

/**********************************************************************************************************************/
// output on serial terminal
#define ANSI_ESC_BLACK      "\033[30m"
#define ANSI_ESC_RED        "\033[31m"
#define ANSI_ESC_GREEN      "\033[32m"
#define ANSI_ESC_YELLOW     "\033[33m"
#define ANSI_ESC_BLUE       "\033[34m"
#define ANSI_ESC_MAGENTA    "\033[35m"
#define ANSI_ESC_CYAN       "\033[36m"
#define ANSI_ESC_WHITE      "\033[37m"
#define ANSI_ESC_RESET      "\033[0m"
#define ANSI_ESC_BROWN      "\033[38;5;130m"
#define ANSI_ESC_ORANGE     "\033[38;5;214m"

#define SerialPrintfln(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
                            Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.printf("\033[0m"); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

/**********************************************************************************************************************/

// //prototypes (main.cpp)
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
void           updateVUmeter();
void           showLogoAndStationName();
void           showStationName(String sn);
void           showStationLogo(String ln);
void           showFileLogo(uint8_t state);
void           showFileName(const char* fname);
void           showFileNumber();
void           showStationsList(uint16_t staListNr);
void           display_time(boolean showall = false);
void           display_alarmDays(uint8_t ad, boolean showall = false);
void           display_alarmtime(int8_t xy = 0, int8_t ud = 0, boolean showall = false);
void           display_sleeptime(int8_t ud = 0);
boolean        drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0, uint16_t maxHeigth = 0);
bool           SD_listDir(const char* path, boolean audioFilesOnly, boolean withoutDirs);
boolean        isAudio(File file);
boolean        isAudio(const char* path);
boolean        isPlaylist(File file);
bool           connectToWiFi();
void           openAccessPoint();
const char*    byte_to_binary(int8_t x);
uint32_t       simpleHash(const char* str);
void           trim(char* s);
bool           startsWith(const char* base, const char* str);
bool           endsWith(const char* base, const char* str);
int32_t        indexOf(const char* base, const char* str, int32_t startIndex);
int32_t        lastIndexOf(const char* haystack, const char needle);
boolean        strCompare(char* str1, char* str2);
boolean        strCompare(const char* str1, char* str2);
char*          x_ps_malloc(uint16_t len);
char*          x_ps_calloc(uint16_t len, uint8_t size);
char*          x_ps_strdup(const char* str);
int16_t        strlenUTF8(const char* str);
int32_t        map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
void           SerialPrintflnCut(const char* item, const char* color, const char* str);
const char*    scaleImage(const char* path);
void           xchgShuffle(uint16_t n);
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
void           SD_playFile(const char* path, uint32_t resumeFilePos = 0, bool showFN = true);
void           SD_playFolder(const char* folderPath, bool showFN);
bool           SD_rename(const char* src, const char* dest);
bool           SD_newFolder(const char* folderPathName);
bool           SD_delete(const char* itemPath);
void           processPlaylist(boolean first = false);
void           changeState(int32_t state);
void           connecttohost(const char* host);
void           connecttoFS(const char* filename, uint32_t resumeFilePos = 0);
void           stopSong();
void IRAM_ATTR headphoneDetect();
void           showDlnaItemsList(uint16_t itemListNr, const char* parentName);

//prototypes (audiotask.cpp)
void     audioInit();
void     audioTaskDelete();
void     audioSetVolume(uint8_t vol);
uint8_t  audioGetVolume();
uint32_t audioGetBitRate();
boolean  audioConnecttohost(const char* host, const char* user = "", const char* pwd = "");
boolean  audioConnecttoFS(const char* filename, uint32_t resumeFilePos = 0);
uint32_t audioStopSong();
void     audioSetTone(int8_t param0, int8_t param1, int8_t param2, int8_t param3 = 0);
uint32_t audioInbuffFilled();
uint32_t audioInbuffFree();
uint32_t audioInbuffSize();
boolean  audioIsRunning();
uint32_t audioGetStackHighWatermark();
uint32_t audioGetCodec();
boolean  audioPauseResume();
void     audioConnectionTimeout(uint32_t timeout_ms, uint32_t timeout_ms_ssl);
uint32_t audioGetFileSize();
uint32_t audioGetFilePosition();
uint16_t audioGetVUlevel();

#ifdef CONFIG_IDF_TARGET_ESP32S3

// dummy functions (simulate ESP32 bluetooth)

static inline void bt_set_volume(int v){;}
static inline void a2dp_sink_init(const char* s, int a, int b, int c){;}
static inline void a2dp_sink_deinit(){;}
static inline void bt_av_get_last_RSSI_delta(){;}
static inline void bt_av_pause_track(){;}
static inline void bt_av_resume_track(){;}
static inline void bt_av_previous_track(){;}
static inline void bt_av_next_track(){;}

#endif