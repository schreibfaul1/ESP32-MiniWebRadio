#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <Ticker.h>
#include <SPI.h>
#include <SD.h> //<SD_MMC.h>
#include <FS.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include "index.h"
#include "websrv.h"
#include "rtime.h"
#include "IR.h"             // see my repository at github "ESP32-IR-Remote-Control"
#include "vs1053_ext.h"     // see my repository at github "ESP32-vs1053_ext"
#include "tft.h"            // see my repository at github "ESP32-TFT-Library-ILI9431-HX8347D"





#define SerialPrintfln(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
                            Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

#if TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1
    //
    //  Display 320x240
    //  +-------------------------------------------+ _yHeader=0
    //  | Header                                    |       _hHeader=20px
    //  +-------------------------------------------+ _yName=20
    //  |                                           |
    //  | Logo                   StationName        |       _hName=100px
    //  |                                           |
    //  +-------------------------------------------+ _yTitle=120
    //  |                                           |
    //  |              StreamTitle                  |       _hTitle=100px
    //  |                                           |
    //  +-------------------------------------------+ _yFooter=220
    //  | Footer                                    |       _hFooter=20px
    //  +-------------------------------------------+ 240
    //                                             320
    const unsigned short* _fonts[6] = {
        Times_New_Roman15x14,
        Times_New_Roman21x17,
        Times_New_Roman27x21,
        Times_New_Roman34x27,
        Times_New_Roman38x31,
        Times_New_Roman43x35,
    };

    struct w_h{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 320; uint16_t h = 20; } const _winHeader;
    struct w_n{uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 320; uint16_t h = 100;} const _winName;
    struct w_t{uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;} const _winTitle;
    struct w_f{uint16_t x = 0;   uint16_t y = 220; uint16_t w = 320; uint16_t h = 20; } const _winFooter;
    struct w_i{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 180; uint16_t h = 20; } const _winItem;
    struct w_v{uint16_t x = 180; uint16_t y = 0;   uint16_t w =  50; uint16_t h = 20; } const _winVolume;
    struct w_m{uint16_t x = 260; uint16_t y = 0;   uint16_t w =  60; uint16_t h = 20; } const _winTime;
    struct w_s{uint16_t x = 0;   uint16_t y = 220; uint16_t w =  60; uint16_t h = 20; } const _winStaNr;
    struct w_l{uint16_t x = 60;  uint16_t y = 220; uint16_t w = 120; uint16_t h = 20; } const _winSleep;
    struct w_a{uint16_t x = 180; uint16_t y = 220; uint16_t w = 160; uint16_t h = 20; } const _winIPaddr;
    struct w_b{uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 14; } const _winVolBar;
    struct w_o{uint16_t x = 0;   uint16_t y = 154; uint16_t w =  64; uint16_t h = 64; } const _winButton;
    uint16_t _alarmdaysXPos_s[7] = {3, 48, 93, 138, 183, 228, 273};
    //
    TFT tft(TFT_CONTROLLER);
    //
#endif //TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1


#if TFT_CONTROLLER == 2
    //
    //  Display 480x320
    //  +-------------------------------------------+ _yHeader=0
    //  | Header                                    |       _winHeader=30px
    //  +-------------------------------------------+ _yName=30
    //  |                                           |
    //  | Logo                   StationName        |       _winName=130px
    //  |                                           |
    //  +-------------------------------------------+ _yTitle=160
    //  |                                           |
    //  |              StreamTitle                  |       _winTitle=130px
    //  |                                           |
    //  +-------------------------------------------+ _yFooter=290
    //  | Footer                                    |       _winFooter=30px
    //  +-------------------------------------------+ 320
    //                                             480

    const unsigned short* _fonts[7] = {
        Times_New_Roman21x17,
        Times_New_Roman27x21,
        Times_New_Roman34x27,
        Times_New_Roman38x31,
        Times_New_Roman43x35,
        Times_New_Roman56x46,
        Times_New_Roman66x53,
    };

    struct w_h{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 480; uint16_t h = 30; } const _winHeader;
    struct w_n{uint16_t x = 0;   uint16_t y = 30;  uint16_t w = 480; uint16_t h = 130;} const _winName;
    struct w_t{uint16_t x = 0;   uint16_t y = 160; uint16_t w = 480; uint16_t h = 130;} const _winTitle;
    struct w_f{uint16_t x = 0;   uint16_t y = 290; uint16_t w = 480; uint16_t h = 30; } const _winFooter;
    struct w_m{uint16_t x = 390; uint16_t y = 0;   uint16_t w =  90; uint16_t h = 30; } const _winTime;
    struct w_i{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 280; uint16_t h = 30; } const _winItem;
    struct w_v{uint16_t x = 280; uint16_t y = 0;   uint16_t w = 110; uint16_t h = 30; } const _winVolume;
    struct w_a{uint16_t x = 260; uint16_t y = 290; uint16_t w = 220; uint16_t h = 30; } const _winIPaddr;
    struct w_s{uint16_t x = 0;   uint16_t y = 290; uint16_t w = 100; uint16_t h = 30; } const _winStaNr;
    struct w_l{uint16_t x = 100; uint16_t y = 290; uint16_t w = 160; uint16_t h = 30; } const _winSleep;
    struct w_b{uint16_t x = 0;   uint16_t y = 160; uint16_t w = 480; uint16_t h = 34; } const _winVolBar;
    //
    TFT tft;        // @suppress("Abstract class cannot be instantiated")
    //
#endif  // TFT_CONTROLLER == 2



//global variables
const uint8_t  _max_volume   = 21;
const uint16_t _max_stations = 1000;
const uint16_t _yBtn = _winTitle.y + 34; // yPos Buttons
uint8_t        _alarmdays      = 0;
uint16_t       _cur_station    = 0;      // current station(nr), will be set later
uint16_t       _sleeptime      = 0;      // time in min until MiniWebRadio goes to sleep
uint16_t       _sum_stations   = 0;
uint8_t        _cur_volume     = 0;      // will be set from stored preferences
uint8_t        _state          = 0;      // statemaschine
uint8_t        _touchCnt       = 0;
uint8_t        _commercial_dur = 0;      // duration of advertising
uint16_t       _alarmtime      = 0;      // in minutes (23:59 = 23 *60 + 59)
int8_t         _releaseNr      = -1;
char           _chbuf[512];
char           _myIP[25];
char           _afn[256];                // audioFileName
char           _path[128];
const char*    _pressBtn[5];
const char*    _releaseBtn[5];
boolean        _f_rtc=false;             // true if time from ntp is received
boolean        _f_1sec = false;
boolean        _f_1min = false;
boolean        _f_mute = false;
boolean        _f_sleeping = false;
boolean        _f_isWebConnected = false;
boolean        _f_isFSConnected = false;
boolean        _f_eof = false;
boolean        _f_eof_alarm = false;
boolean        _f_semaphore = false;
boolean        _f_alarm = false;
boolean        _f_irNumberSeen = false;
boolean        _f_newIcyDescription = false;
boolean        _f_volBarVisible = false;
boolean        _f_SD_okay = false;

String         _station = "";
String         _stationName_nvs = "";
String         _stationName_air = "";
String         _stationURL = "";
String         _homepage = "";
String         _streamTitle = "";
String         _lastconnectedhost = "";
String         _filename = "";
String         _icydescription = "";

char _hl_item[10][25]{                          // Title in headline
                "** Internet Radio **",         // "* интернет-радио *"  "ραδιόφωνο Internet"
                "** Internet Radio **",
                "** Internet Radio **",
                "** Uhr **",                    // Clock "** часы́ **"  "** ρολόι **"
                "** Uhr **",
                "** Helligkeit **",             // Brightness яркость λάμψη
                "** Audioplayer **",            // "** цифрово́й плеер **"
                "** Audioplayer **",
                "" ,                            // Alarm should be empty
                "* Einschlafautomatik *",       // "Sleeptimer" "Χρονομετρητής" "Таймер сна"
};

enum status{RADIO = 0, RADIOico = 1, RADIOmenue = 2,
            CLOCK = 3, CLOCKico = 4, BRIGHTNESS = 5,
            PLAYER= 6, PLAYERico= 7,
            ALARM = 8, SLEEP    = 9};


//objects
#if DECODER == 0
    VS1053 vs1053(VS1053_CS, VS1053_DCS, VS1053_DREQ, VSPI, SPI_MOSI, SPI_MISO, SPI_SCK);
#endif

#if DECODER == 1
    Audio audio;
#endif

WebSrv webSrv;
Preferences pref;
Preferences stations;
RTIME rtc;
Ticker ticker;
IR ir(IR_PIN);                  // do not change the objectname, it must be "ir"
TP tp(TP_CS, TP_IRQ);
WiFiMulti wifiMulti;
File audioFile;

SemaphoreHandle_t  mutex_rtc;
SemaphoreHandle_t  mutex_display;

//prototypes (main.cpp)
boolean defaultsettings();
boolean saveStationsToNVS();
void setTFTbrightness(uint8_t duty);
const char* UTF8toASCII(const char* str);
const char* ASCIItoUTF8(const char* str);
void showHeadlineVolume(uint8_t vol);
void showHeadlineTime();
void showHeadlineItem(uint8_t idx);
void showFooterIPaddr();
void showFooterStaNr();
void updateSleepTime(boolean noDecrement = false);
void showVolumeBar();
void showBrightnessBar();
void showFooter();
void display_info(const char *str, int ypos, int height, uint16_t color, uint16_t indent, uint16_t winHight);
void showStreamTitle();
void showLogoAndStationName();
void showFileName(const char* fname);
void display_time(boolean showall = false);
void display_alarmDays(uint8_t ad, boolean showall=false);
void display_alarmtime(int8_t xy = 0, int8_t ud = 0, boolean showall = false);
void display_sleeptime(int8_t ud = 0);
boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth = 0 , uint16_t maxHeigth = 0);
bool setAudioFolder(const char* audioDir);
const char* listAudioFile();
bool sendAudioList2Web(const char* audioDir);
bool connectToWiFi();
const char* byte_to_binary(int8_t x);
bool startsWith (const char* base, const char* str);
bool endsWith (const char* base, const char* str);
int indexOf (const char* base, const char* str, int startIndex);
const char* scaleImage(const char* path);
inline uint8_t getvolume();
uint8_t downvolume();
uint8_t upvolume();
void setStation(uint16_t sta);
void nextStation();
void prevStation();
void StationsItems();
void changeBtn_pressed(uint8_t btnNr);
void changeBtn_released(uint8_t btnNr);
void savefile(String fileName, uint32_t contentLength);
String setTone();
void audiotrack(const char* fileName);
void changeState(int state);