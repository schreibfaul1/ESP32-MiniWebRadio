// created: 10.02.2022
// updated: 28.06.2026

#include "settings.h"
#pragma once

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
#include <deque>
#include <mbedtls/base64.h>
#include <vector>

Audio       audio;
Preferences pref;
WebSrv      webSrv;
WiFiMulti   wifiMulti;
RTIME       rtc;
Ticker      ticker100ms;
TwoWire     i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire     i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
SPIClass    spiBus(FSPI);

#include "mwr_src/function.h"
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

SET_LOOP_TASK_STACK_SIZE(14 * 1024);
constexpr uint16_t MAX_STATIONS = 1000;

bool s_f_pauseResume = false;
bool s_f_newStreamTitle = false;
bool s_f_rtc = false; // true if time from ntp is received
bool s_f_100ms = false;
bool s_f_1sec = false;
bool s_f_10sec = false;
bool s_f_1min = false;
bool s_f_1h = false;
bool s_f_mute = false;
bool s_f_muteIsPressed = false;
bool s_f_recording = false;
bool s_f_sleeping = false;
bool s_f_isWebConnected = false;
bool s_f_WiFi_lost = false;
bool s_f_isFSConnected = false;
bool s_f_eof = false;
bool s_f_reconnect = false;
bool s_f_eof_alarm = false;
bool s_f_alarm = false;
bool s_f_newIcyDescription = false;
bool s_f_webFailed = false;
bool s_f_newBitRate = false;
bool s_f_newStationName = false;
bool s_f_newLyrics = false;
bool s_f_volBarVisible = false;
bool s_f_switchToClock = false;   // jump into CLOCK mode at the next opportunity
bool s_f_timeAnnouncement = true; // time announcement every full hour
bool s_f_vu_meter_enabled = false;
bool s_f_spectrum_enabled = false;
bool s_f_playlistEnabled = false;
bool s_f_playlistNextFile = false;
bool s_f_logoUnknown = false;
bool s_f_FFatFound = false;
bool s_f_clearLogo = false;
bool s_f_clearStationName = false;
bool s_f_dlnaBrowseServer = false;
bool s_f_dlnaWaitForResponse = false;
bool s_f_dlnaMakePlaylistOTF = false; // notify callback that this browsing was to build a On-The_fly playlist
bool s_f_dlna_browseReady = false;
bool s_f_brightnessIsChangeable = false;
bool s_f_connectToLastStation = false;
bool s_f_msg_box = false;
bool s_f_esp_restart = false;
bool s_f_timeSpeech = false;
bool s_f_update_meteo = false;
bool s_f_stationsChanged = false;
bool s_f_sd_card_found = false;
bool s_f_isWiFiConnected = false;
bool s_f_ok_from_ir = false;

int8_t   s_state = NONE; // statemaschine
int8_t   s_lastState = NONE;
int8_t   s_subState = UNDEFINED;
int8_t   s_subState_radio = UNDEFINED;
int8_t   s_subState_player = UNDEFINED;
int8_t   s_subState_clock = UNDEFINED;
int8_t   s_subState_weather = UNDEFINED;
int8_t   s_ir_btn_select = UNDEFINED; // IR menue item
int8_t   s_currDLNAsrvNr = -1;
int8_t   s_alarmSubMenue = -1;
int8_t   s_sleepTimerSubMenue = -1;
uint8_t  s_alarmdays = 0;
uint8_t  s_cur_Codec = 0;
uint8_t  s_numServers = 0; //
uint8_t  s_level = 0;
uint8_t  s_sleepMode = 1; // 0 display off, 1 show the clock
uint8_t  s_staListPos = 0;
uint8_t  s_cthFailCounter = 0; // connecttohost fail
uint8_t  s_itemListPos = 0;    // DLNA items
uint8_t  s_fileListPos = 0;
uint8_t  s_ambientValue = 50;
uint8_t  s_dlnaLevel = 0;
uint8_t  s_resetReason = (esp_reset_reason_t)ESP_RST_UNKNOWN;
uint8_t  s_brightness = UINT8_MAX / 2;
uint8_t  s_start_counter = 0;
int16_t  s_totalNumberReturned = -1;
int16_t  s_dlnaMaxItems = -1;
int16_t  s_dlnaMaXServers = -1;
int16_t  s_alarmtime[7] = {0};  // in minutes (23:59 = 23 *60 + 59) [0] Sun, [1] Mon
int16_t  s_cur_AudioFileNr = 0; // this is the position of the file within the (alpha ordered) folder starting with 0
uint16_t s_staListNr = 0;
uint16_t s_fileListNr = 0;
uint16_t s_cur_station = 0; // current station(nr), will be set later
uint16_t s_sleeptime = 0;   // time in min until MiniWebRadio goes to sleep
uint16_t s_plsCurPos = 0;
uint16_t s_dlnaItemNr = 0;
uint16_t s_h_resolution = 320;
uint16_t s_v_resolution = 240;
uint32_t s_icyBitRate = 0;     // from http response header via event
uint32_t s_decoderBitRate = 0; // from decoder via getBitRate(false)
uint32_t s_playlistTime = 0;   // playlist start time millis() for timeout
uint32_t s_settingsHash = 0;
uint32_t s_audioFileSize = 0;
uint32_t s_media_downloadPort = 0;
uint32_t s_audioCurrentTime = 0;
uint32_t s_timestamp = 0;
uint32_t s_audioFileDuration = 0;
uint64_t s_totalRuntime = 0; // total runtime in seconds since start

ps_ptr<char> s_streamTitle;
ps_ptr<char> s_myIP = "000.000.000.000";
ps_ptr<char> s_cur_AudioFolder = "/audiofiles/";
ps_ptr<char> s_icyDescription;
ps_ptr<char> s_cur_AudioFileName;
ps_ptr<char> s_stationURL;
ps_ptr<char> s_stationName_air;
ps_ptr<char> s_homepage;
ps_ptr<char> s_TZName = "Europe/Berlin";
ps_ptr<char> s_TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
ps_ptr<char> s_timeSpeechLang = "en";
ps_ptr<char> s_lyrics = "";
ps_ptr<char> s_location = "Europe/Berlin";
ps_ptr<char> s_latiitude = "52.52";
ps_ptr<char> s_longitude = "13.41";
ps_ptr<char> s_temperature_unit = "C";   // *C or °F
ps_ptr<char> s_pressure_unit = "hPa";    // hPa or mmHg
ps_ptr<char> s_wind_speed_unit = "km/h"; // km/h, m/s, bft
ps_ptr<char> s_version;

#include "mwr_src/classes.hpp"

dlnaHistory_s  s_dlnaHistory[10];
timecounter_s  s_timeCounter;
SD_content     s_SD_content;
Playlist       playlist;
IR_buttons     irb(&s_settings);
IR             ir(IR_PIN); // do not change the objectname, it must be "ir"
File           audioFile;
FtpServer      ftpSrv;
DLNA_Client    dlna;
KCX_BT_Emitter bt_emitter(BT_EMITTER_RX, BT_EMITTER_TX, BT_EMITTER_CONNECT, BT_EMITTER_MODE);
hp_BH1750      BH1750; // create the sensor
ES8311         es8311;
METEO          meteo;
RTIME::rtime   s_time;
TCA9554        tca9554;

std::deque<ps_ptr<char>> s_PLS_content;
ps_ptr<char>             codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "OPUS", "VORBIS", "OGG"};
stationManagement        staMgnt(&s_cur_station);

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
void         show_DLNA_FileName(ps_ptr<char> fname);
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
inline void  get_registered_names();
inline void  clearLogo();
inline void  clearStationName();
void         timer100ms();

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
// 📌📌📌   U S E R   I N P U T    📌📌📌
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ui_pause_resume() {
    bool res = audio.pauseResume();
    if (!res) { printfln(s_tag.terminal, ANSI_ESC_YELLOW "Pause-Resume not possible"); }
    s_f_pauseResume = audio.isRunning();
    if (s_f_pauseResume) {
        printfln(s_tag.terminal, ANSI_ESC_YELLOW "Pause-Resume --> resume");
    } else {
        printfln(s_tag.terminal, ANSI_ESC_YELLOW "Pause-Resume --> pause");
    }
}
void ui_runtimeStats() {
    ps_ptr<char> timeStatsBuffer;
    timeStatsBuffer.set_name("timeStatsBuffer");
    GetRunTimeStats(timeStatsBuffer);
    { printfln(s_tag.terminal, ANSI_ESC_YELLOW "task statistics\n\n{}", timeStatsBuffer); }
}
void ui_localFile(uint16_t s) {
    const char* path = "/audiofiles/sample.mp3";
    uint16_t    fileStart = s;
    printfln(s_tag.terminal, ANSI_ESC_YELLOW "path: {}, fileStart {}s", path, fileStart);
    connecttoFS("SD_MMC", path, fileStart);
}
void ui_connecttospeech() {
    audio.connecttospeech("Hallo, wie geht es dir? Morgen scheint die Sonne und übermorgen regnet es.Aber wir nehmen den Regenschirm mit. Und auch den Rucksack. Dann lesen wir aus dem Buch "
                          "Hier gibt es nur gutes Wetter.",
                          "de");
}
void ui_bufferFilled() {
    printfln(s_tag.terminal, "inBuffer filled {} bytes", audio.inBufferFilled());
    printfln(s_tag.terminal, "inBuffer free   {} bytes", audio.inBufferFree());
}
void ui_fontTest(uint16_t s) {
    if (s == 0) s_streamTitle = "We’re Going To Ibiza";
    if (s == 1) s_streamTitle = "Á á É é Í í Ó ó Ő ő Ú ú Ű ű";
    if (s == 2) s_streamTitle = "Č č Ć ć Š š Ž ž Đ đ Ł ł Ń ń Ś ś Ź ź Ż ż";
    if (s == 3) s_streamTitle = "Ő ő Ű ű € – ← ’ “ ” …🔵🟥";
    if (s == 4) s_streamTitle = "Ă ă Â â Î î Ș ș Ț ț Ş ş Ţ ţ Ş ş Ţ ţ";
    if (s == 5) s_streamTitle = "A B C D E F G H I K L J M y O P Q R";
    printfln(s_tag.terminal, "st: {}", s_streamTitle);
    s_f_newStreamTitle = true;
}
void ui_setTimeOffset(int16_t s) {
    printfln(s_tag.terminal, "setTimeOffset {}", s);
    audio.setTimeOffset(s);
}
void ui_setPlayTime(int16_t s) {
    printfln(s_tag.terminal, "setAudioPlayTime {}", s);
    audio.setAudioPlayTime(s);
}
void ui_getAudioFilePosition() {
    printfln(s_tag.terminal, "getAudioFilePosition {}", audio.getAudioFilePosition());
}
void ui_setAudioFilePos(uint32_t s) {
    printfln(s_tag.terminal, "setAudioFilePosition {}", s);
    audio.setAudioFilePosition(s);
}
void ui_getRegistredNames() {
    get_registered_names();
}
void ui_forceMono() {
    static bool f_mono = false;
    f_mono = !f_mono;
    audio.forceMono(f_mono);
    if (f_mono)
        printfln(s_tag.terminal, "mono");
    else
        printfln(s_tag.terminal, "stereo");
}
void ui_setMute() {
    static bool f_mute = false;
    f_mute = !f_mute;
    audio.setMute(f_mute);
    if (f_mute)
        printfln(s_tag.terminal, "mute on");
    else
        printfln(s_tag.terminal, "mute off");
}
void ui_output48KHz() {
    static bool f_o48 = false;
    f_o48 = !f_o48;
    if (f_o48) {
        audio.setOutputSampleRate(Audio::SR_48000);
        printfln(s_tag.terminal, "output 48KHz");
    } else {
        audio.setOutputSampleRate(Audio::SR_ORIGIN);
        printfln(s_tag.terminal, "normal output {} Hz", audio.getSampleRate());
    }
}
void ui_output44KHz() {
    static bool f_o44 = false;
    f_o44 = !f_o44;
    if (f_o44) {
        audio.setOutputSampleRate(Audio::SR_44100);
        printfln(s_tag.terminal, "output 44.1KHz");
    } else {
        audio.setOutputSampleRate(Audio::SR_ORIGIN);
        printfln(s_tag.terminal, "normal output {} Hz", audio.getSampleRate());
    }
}
void ui_btProtocol() {
    bt_emitter.list_protokol();
}
void ui_btCommand(ps_ptr<char> cmd) {
    bt_emitter.userCommand(cmd);
    printfln(s_tag.terminal, "btstr: {}", cmd);
}
void ui_meteoRequest() {
    meteo.send_request();
}
void ui_meteoProtocol() {
    meteo.protocol();
}
void ui_timeSpeech() {
    s_f_timeSpeech = !s_f_timeSpeech;
    if (s_f_timeSpeech) {
        printfln(s_tag.terminal, "time speech is on");
    } else {
        printfln(s_tag.terminal, "time speech is off");
    }
}
void ui_setWiFiPWD() {
    changeState(WIFI_SETTINGS, 0);
}
void ui_openAIspeech() {
    printfln(s_tag.terminal, "openAI speech");
    audio.openai_speech("openAI-key", "tts-1", "Today is a wonderful day to build something people love!", "", "shimer", "mp3", "1");
}
uint32_t song_time = 0;
void     ui_stopSong() {
    song_time = audio.stopSong();
    printfln(s_tag.terminal, "file {} stopped at time {}", s_cur_AudioFileName, song_time);
}
void ui_start_song() {
    ps_ptr<char> path = "/audiofiles/" + s_cur_AudioFileName;
    bool         ret = audio.connecttoFS(SD_MMC, path.c_get(), song_time);
    printfln(s_tag.terminal, "file {} started at time {}, ret {}", s_cur_AudioFileName, song_time, ret);
}
void ui_get_bitRate() {
    uint32_t br = audio.getBitRate();
    printfln(s_tag.terminal, "bitrate: {}", br);
}
void ui_get_inbuffStatus() {
    audio.inBufferStatus();
}
void ui_isRunning() {
    printfln(s_tag.terminal, "is running: {}", audio.isRunning());
}
void ui_volFadingSpeed(float s) {
    printfln(s_tag.terminal, "set volume fading speed {}, current: {}", s, audio.settings.VOL_FADING_SPEED);
    audio.settings.VOL_FADING_SPEED = s;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void user_input(ps_ptr<char> input) {

    printfln(s_tag.terminal, ANSI_ESC_YELLOW "{}", input);

    if (input == "pr") ui_pause_resume();                                          // pause resume
    if (input == "rts") ui_runtimeStats();                                         // get runtime stats
    if (input.starts_with("lf")) ui_localFile(input.substr(2).to_uint16());        // file starts at 30s; lf30
    if (input == "cts") ui_connecttospeech();                                      // connect to speech
    if (input == "bfi") ui_bufferFilled();                                         // buffer filled
    if (input.starts_with("ft")) ui_fontTest(input.substr(2).to_uint16());         // test some characters shown as streamTitle
    if (input.starts_with("stoff")) ui_setTimeOffset(input.substr(5).to_int16());  // set offset +- x seconds; stoff-25
    if (input.starts_with("sapt")) ui_setPlayTime(input.substr(4).to_int16());     // set audio playtime at x seconds
    if (input == "gafp") ui_getAudioFilePosition();                                // getAudioFilePosition
    if (input.starts_with("safp")) ui_setAudioFilePos(input.substr(4).to_int32()); // setAudioFilePosition
    if (input == "grn") ui_getRegistredNames();                                    // list of all self registered objects
    if (input == "fomo") ui_forceMono();                                           // force mono
    if (input == "sem") ui_setMute();                                              // set mute
    if (input == "o48") ui_output48KHz();                                          // output48KHz
    if (input == "o44") ui_output44KHz();                                          // output44KHz
    if (input == "btp") ui_btProtocol();                                           // blurtooth kcx protocol
    if (input.starts_with("btcmd")) ui_btCommand(input.substr(6));                 // bluetooth command, send to bt emitter e.g. btstr:AT+
    if (input == "meteor") ui_meteoRequest();                                      // open meteo update request
    if (input == "meteop") ui_meteoProtocol();                                     // open meteo update protocol
    if (input == "tsp") ui_timeSpeech();                                           // toogle time speech
    if (input == "pwd") ui_setWiFiPWD();                                           // set password for WiFi
    if (input == "oais") ui_openAIspeech();                                        // openAIspeech
    if (input == "stops") ui_stopSong();                                           // stop song
    if (input == "starts") ui_start_song();                                        // start song
    if (input == "gbr") ui_get_bitRate();                                          // get bitrate
    if (input == "gibs") ui_get_inbuffStatus();                                    //  // get inbuff status
    if (input == "ir") ui_isRunning();                                             // is running?
    if (input.starts_with("vfs")) ui_volFadingSpeed(input.substr(3).to_float());   // volume fading speed
}