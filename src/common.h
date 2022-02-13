// created: 10.02.2022
// updated: 13.02.2022

#pragma once


#define _SSID           "Wolles-FRITZBOX"//"mySSID"                        // Your WiFi credentials here
#define _PW             "40441061073895958449"//"myWiFiPassword"
#define TZName          "CET-1CEST,M3.5.0,M10.5.0/3"    // Timezone (more TZNames in "rtime.cpp")
#define DECODER         1                               // (0)VS1053 , (1)SW DECODER, DAC via I2S
#define TFT_CONTROLLER  2                               // (0)ILI9341, (1)HX8347D, (2)ILI9486
#define TFT_FREQUENCY   40000000                        // 27000000, 40000000, 80000000
#define TFT_ROTATION    3                               // 0 ... 3
#define TP_ROTATION     1

/**********************************************************************************************************************/

#include <Arduino.h>
#include <Preferences.h>
#include <Ticker.h>
#include <SPI.h>
#include <SD_MMC.h>
#include <FS.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include "index.h"
#include "websrv.h"
#include "rtime.h"
#include "IR.h"

#if TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1 // 320x240px ILI9431-HX8347D
    #include "tft.h"
#endif

#if TFT_CONTROLLER == 2 // 480x320px ILI9486
    #include "ili9486.h"
#endif

// Digital I/O used
#define VS1053_CS     33
#define VS1053_DCS     4
#define VS1053_DREQ   36
#define TFT_CS        22
#define TFT_DC        21
#define TFT_BL        32
#define TP_IRQ        39
#define TP_CS          5
#define SD_MMC_D0      2  // cannot be changed
#define SD_MMC_CLK    14  // cannot be changed
#define SD_MMC_CMD    15  // cannot be changed
#define IR_PIN        34
#define SPI_MOSI      23  // (VSPI)
#define SPI_MISO      19  // (VSPI)
#define SPI_SCK       18  // (VSPI)
#define VS1053_MOSI   13  // VS1053     (HSPI)
#define VS1053_MISO   34  // VS1053     (HSPI)
#define VS1053_SCK    12  // VS1053     (HSPI)
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_MCLK       0

#define SerialPrintfln(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
                            Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

/**********************************************************************************************************************/



// //prototypes (main.cpp)
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
void savefile(const char* fileName, uint32_t contentLength);
String setTone();
void audiotrack(const char* fileName);
void changeState(int state);

// //prototypes (audiotask.cpp)
void audioInit();
void audioSetVolume(uint8_t vol);
uint8_t audioGetVolume();
boolean audioConnecttohost(const char* host);
void audioConnecttoFS(const char* filename);
void audioStopSong();
void audioSetTone(int8_t lowPass, int8_t bandPass, int8_t highPass);

