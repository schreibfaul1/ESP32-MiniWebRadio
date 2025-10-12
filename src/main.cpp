#include "common.h"
#include "mwr_src/function.hpp"
#include "mwr_src/graphical.hpp"
#include "mwr_src/index.h"
#include "mwr_src/index.js.h"
#include "mwr_src/layout.hpp"
// clang-format off
/*****************************************************************************************************************************************************
    MiniWebRadio -- Webradio receiver for ESP32-S3

    first release on 03/2017                                                                                                      */char Version[] ="\
    Version 4.0.4 - 04.10.2025                                                                                                               ";

/*  display (320x240px) with controller ILI9341 or
    display (480x320px) with controller ILI9486 or ILI9488 (SPI) or
    display (800x480px) (RGB-HMI) with TP controller GT911 (I2C)

    SD_MMC is mandatory
    IR remote is optional
    BT Transmitter is optional
    BH1750 (lightsensor) is optional

*****************************************************************************************************************************************************/

// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

// clang-format on

SET_LOOP_TASK_STACK_SIZE(12 * 1024);

// global variables

char _hl_item[18][40]{"",                    // none
                      "Internet Radio",      // "* интернет-радио *"  "ραδιόφωνο Internet"
                      "Audio Player",        // "** цифрово́й плеер **
                      "DLNA",                // Digital Living Network Alliance
                      "Clock",               // Clock "** часы́ **"  "** ρολόι **"
                      "Brightness",          // Brightness яркость λάμψη
                      "Alarm Clock (hh:mm)", // AlarmClock "будильник" "ξύπνημα"
                      "Off Timer (h:mm)",    // "Sleeptimer" "Χρονομετρητής" "Таймер сна"
                      "Stations List",
                      "Audio Files",
                      "DLNA List",
                      "Bluetooth",
                      "Equalizer",
                      "Settings",
                      "IR Settings",
                      "Alarm",
                      "WiFi Settings", //
                      ""};

settings_s     s_settings;
volume_s       s_volume;
dlnaHistory_s  s_dlnaHistory[10];
timecounter_s  s_timeCounter;
SD_content     s_SD_content;
bt_emitter_s   s_bt_emitter;
const uint16_t s_max_stations = 1000;
int8_t         s_currDLNAsrvNr = -1;
uint8_t        s_alarmdays = 0;
uint8_t        s_brightness = 100;   // percent
uint8_t        s_state = UNDEFINED;  // statemaschine
uint8_t        s_commercial_dur = 0; // duration of advertising
uint8_t        s_cur_Codec = 0;
uint8_t        s_numServers = 0; //
uint8_t        s_level = 0;
uint8_t        s_timeFormat = 24; // 24 or 12
uint8_t        s_sleepMode = 1;   // 0 display off, 1 show the clock
uint8_t        s_staListPos = 0;
uint8_t        s_WiFi_disconnectCnt = 0;
uint8_t        s_reconnectCnt = 0;
uint8_t        s_cthFailCounter = 0; // connecttohost fail
uint16_t       s_staListNr = 0;
uint8_t        s_fileListPos = 0;
uint8_t        s_radioSubMenue = 0;
uint8_t        s_playerSubMenue = 0;
uint8_t        s_dlnaSubMenue = 0;
uint8_t        s_clockSubMenue = 0;
uint8_t        s_alarmSubMenue = 0;
uint8_t        s_sleepTimerSubMenue = 0;
uint8_t        s_settingsSubMenue = 0;
uint8_t        s_brightnessSubMenue = 0;
uint8_t        s_equalizerSubMenue = 0;
uint8_t        s_ambientValue = 50;
uint16_t       s_fileListNr = 0;
uint16_t       s_irNumber = 0;
uint16_t       s_irResult = 0;
uint8_t        s_itemListPos = 0; // DLNA items
uint16_t       s_dlnaItemNr = 0;
uint8_t        s_dlnaLevel = 0;
int16_t        s_alarmtime[7] = {0};  // in minutes (23:59 = 23 *60 + 59) [0] Sun, [1] Mon
int16_t        s_toneLP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t        s_toneBP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t        s_toneHP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t        s_toneBAL = 0;         // -16...0....+16         audioI2S
uint32_t       s_icyBitRate = 0;      // from http response header via event
uint32_t       s_decoderBitRate = 0;  // from decoder via getBitRate(false)
uint16_t       s_cur_station = 0;     // current station(nr), will be set later
int16_t        s_cur_AudioFileNr = 0; // this is the position of the file within the (alpha ordered) folder starting with 0
uint16_t       s_sleeptime = 0;       // time in min until MiniWebRadio goes to sleep
uint16_t       s_plsCurPos = 0;
int16_t        s_totalNumberReturned = -1;
int16_t        s_dlnaMaxItems = -1;
uint16_t       s_bh1750Value = 50;
uint32_t       s_playlistTime = 0; // playlist start time millis() for timeout
uint32_t       s_settingsHash = 0;
uint32_t       s_audioFileSize = 0;
uint32_t       s_media_downloadPort = 0;
uint32_t       s_audioCurrentTime = 0;
uint32_t       s_audioFileDuration = 0;
uint64_t       s_totalRuntime = 0; // total runtime in seconds since start
uint8_t        s_resetReason = (esp_reset_reason_t)ESP_RST_UNKNOWN;
const char*    s_pressBtn[8];
const char*    s_releaseBtn[8];
const char*    s_time_s = "";
bool           s_btEmitterMode = KCX_BT_Emitter::BT_MODE_EMITTER;
char           s_chbuf[512];
char           s_fName[256];
char           s_myIP[25] = {0};
char           s_path[128];
char           s_commercial[25];
char           s_icyDescription[512] = {};
char           s_streamTitle[512] = {};
char           s_timeSpeechLang[10] = "en";
char*          s_cur_AudioFolder = NULL;
char*          s_cur_AudioFileName = NULL;
char*          s_stationURL = NULL;
char*          s_BT_metaData = NULL;
char*          s_playlistPath = NULL;
bool           s_f_rtc = false; // true if time from ntp is received
bool           s_f_100ms = false;
bool           s_f_1sec = false;
bool           s_f_10sec = false;
bool           s_f_1min = false;
bool           s_f_mute = false;
bool           s_f_muteIsPressed = false;
bool           s_f_volumeDownIsPressed = false;
bool           s_f_volumeUpIsPressed = false;
bool           s_f_sleeping = false;
bool           s_f_irOnOff = false;
bool           s_f_isWebConnected = false;
bool           s_f_isFSConnected = false;
bool           s_f_eof = false;
bool           s_f_reconnect = false;
bool           s_f_eof_alarm = false;
bool           s_f_alarm = false;
bool           s_f_irNumberSeen = false;
bool           s_f_irResultSeen = false;
bool           s_f_newIcyDescription = false;
bool           s_f_newStreamTitle = false;
bool           s_f_webFailed = false;
bool           s_f_newBitRate = false;
bool           s_f_newStationName = false;
bool           s_f_newCommercial = false;
bool           s_f_volBarVisible = false;
bool           s_f_switchToClock = false;   // jump into CLOCK mode at the next opportunity
bool           s_f_timeAnnouncement = true; // time announcement every full hour
bool           s_f_playlistEnabled = false;
bool           s_f_playlistNextFile = false;
bool           s_f_logoUnknown = false;
bool           s_f_pauseResume = false;
bool           s_f_SD_Upload = false;
bool           s_f_FFatFound = false;
bool           s_f_SD_MMCfound = false;
bool           s_f_ESPfound = false;
bool           s_f_BH1750_found = false;
bool           s_f_clearLogo = false;
bool           s_f_clearStationName = false;
bool           s_f_shuffle = false;
bool           s_f_dlnaBrowseServer = false;
bool           s_f_dlnaWaitForResponse = false;
bool           s_f_dlnaSeekServer = false;
bool           s_f_dlnaMakePlaylistOTF = false; // notify callback that this browsing was to build a On-The_fly playlist
bool           s_f_dlna_browseReady = false;
bool           s_f_BTEmitterConnected = false;
bool           s_f_brightnessIsChangeable = false;
bool           s_f_connectToLastStation = false;

bool   s_f_timeSpeech = false;
bool   s_f_stationsChanged = false;
bool   s_f_WiFiConnected = false;
String s_station = "";
char*  s_stationName_air = NULL;
String s_homepage = "";
String s_filename = "";
String s_TZName = "Europe/Berlin";
String s_TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
String s_media_downloadIP = "";

std::vector<char*>       s_PLS_content;
std::deque<ps_ptr<char>> s_logBuffer;

const char* codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "AACP", "OPUS", "OGG", "VORBIS"};

Audio       audio;
Preferences pref;
WebSrv      webSrv;
WiFiMulti   wifiMulti;
RTIME       rtc;
Ticker      ticker100ms;
IR_buttons  irb(&s_settings);
IR          ir(IR_PIN); // do not change the objectname, it must be "ir"

File           audioFile;
FtpServer      ftpSrv;
DLNA_Client    dlna;
KCX_BT_Emitter bt_emitter(BT_EMITTER_RX, BT_EMITTER_TX, BT_EMITTER_LINK, BT_EMITTER_MODE);
TwoWire        i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire        i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
hp_BH1750      BH1750(&i2cBusOne);     // create the sensor
SPIClass       spiBus(FSPI);

#if TFT_CONTROLLER < 7 // ⏹⏹⏹⏹
TFT_SPI    tft(spiBus, TFT_CS);
TP_XPT2046 tp(spiBus, TP_CS);
#else
TFT_RGB  tft;
TP_GT911 tp(&i2cBusOne);
#endif

stationManagement staMgnt(&s_cur_station);

SemaphoreHandle_t mutex_rtc;
SemaphoreHandle_t mutex_display;

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     D E F A U L T S E T T I N G S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format off  🟢🟡🔴
boolean defaultsettings() {

    if (!SD_MMC.exists("/ir_buttons.json")) { // if not found ir_buttons.json create a default file
        File file = SD_MMC.open("/ir_buttons.json", "w", true);
        file.write((uint8_t*)ir_buttons_json, sizeof(ir_buttons_json));
        file.close();
    }
    irb.loadButtonsFromJSON("/ir_buttons.json");
    for (uint i = 0; i < s_settings.numOfIrButtons; i++) {
        //    log_w("0x%04X,  %s", s_settings.irbuttons[i].val, s_settings.irbuttons[i].label);
        ir.set_irButtons(i, s_settings.irbuttons[i].val);
    }
    ir.set_irAddress(s_settings.irbuttons[42].val);
    //    log_w("0x%04X,  %s", s_settings.irbuttons[42].val, s_settings.irbuttons[42].label);

    if (!SD_MMC.exists("/settings.json")) { // if not found create one
        updateSettings();
    }

    File  file2 = SD_MMC.open("/settings.json", "r", false);
    char* jO = x_ps_calloc(2048, 1);
    char* tmp = x_ps_malloc(1024);
    file2.readBytes(jO, 2048);
    s_settingsHash = simpleHash(jO);
    file2.close();

    auto parseJson = [&](const char* s) { // lambda, inner function
        int16_t pos1 = 0, pos2 = 0, pos3 = 0;
        pos1 = indexOf(jO, s, 0);
        if (pos1 < 0) {
            log_e("index %s not found", s);
            return "0";
        }
        pos2 = indexOf(jO, ":", pos1) + 1;
        if (jO[pos2] == '\"')
            pos3 = indexOf(jO, "\"", pos2 + 1) + 1;
        else
            pos3 = indexOf(jO, ",", pos2);
        if (pos3 < 0) pos3 = find_first_of(jO, "}\n", pos2);
        if (jO[pos2] == '\"') {
            pos2++;
            pos3--;
        } // remove \" embraced strings
        strncpy(tmp, jO + pos2, pos3 - pos2);
        tmp[pos3 - pos2] = '\0';
        return (const char*)tmp;
    };

    auto computeMinuteOfTheDay = [&](const char* s) {
        if (!s) return 0;
        int h = atoi(s);
        int m = atoi(s + 3);
        return h * 60 + m;
    };

    s_settings.lastconnectedhost.reset();
    s_settings.lastconnectedfile.reset();

    s_volume.cur_volume = atoi(parseJson("\"volume\":"));
    s_volume.volumeSteps = atoi(parseJson("\"volumeSteps\":"));
    s_volume.ringVolume = atoi(parseJson("\"ringVolume\":"));
    s_volume.volumeAfterAlarm = atoi(parseJson("\"volumeAfterAlarm\":"));
    s_bt_emitter.volume = atoi(parseJson("\"BTvolume\":"));
    s_bt_emitter.power_state = (strcmp(parseJson("\"BTpower\":"), "true") == 0) ? 1 : 0;
    s_bt_emitter.mode = ((strcmp(parseJson("\"BTmode\":"), "TX") == 0) ? "TX" : "RX");
    s_alarmtime[0] = computeMinuteOfTheDay(parseJson("\"alarmtime_sun\":"));
    s_alarmtime[1] = computeMinuteOfTheDay(parseJson("\"alarmtime_mon\":"));
    s_alarmtime[2] = computeMinuteOfTheDay(parseJson("\"alarmtime_tue\":"));
    s_alarmtime[3] = computeMinuteOfTheDay(parseJson("\"alarmtime_wed\":"));
    s_alarmtime[4] = computeMinuteOfTheDay(parseJson("\"alarmtime_thu\":"));
    s_alarmtime[5] = computeMinuteOfTheDay(parseJson("\"alarmtime_fri\":"));
    s_alarmtime[6] = computeMinuteOfTheDay(parseJson("\"alarmtime_sat\":"));
    s_alarmdays = atoi(parseJson("\"alarm_weekdays\":"));
    s_f_timeAnnouncement = (strcmp(parseJson("\"timeAnnouncing\":"), "true") == 0) ? 1 : 0;
    strcpy(s_timeSpeechLang, parseJson("\"timeSpeechLang\":"));
    s_f_mute = (strcmp(parseJson("\"mute\":"), "true") == 0) ? 1 : 0;
    s_brightness = atoi(parseJson("\"brightness\":"));
    s_sleeptime = atoi(parseJson("\"sleeptime\":"));
    s_cur_station = atoi(parseJson("\"station\":"));
    s_toneLP = atoi(parseJson("\"toneLP\":"));
    s_toneBP = atoi(parseJson("\"toneBP\":"));
    s_toneHP = atoi(parseJson("\"toneHP\":"));
    s_toneBAL = atoi(parseJson("\"balance\":"));
    s_timeFormat = atoi(parseJson("\"timeFormat\":"));
    s_TZName = parseJson("\"Timezone_Name\":");
    s_TZString = parseJson("\"Timezone_String\":");
    s_settings.lastconnectedhost.copy_from(parseJson("\"lastconnectedhost\":"));
    s_settings.lastconnectedfile.copy_from(parseJson("\"lastconnectedfile\":"));
    s_sleepMode = atoi(parseJson("\"sleepMode\":"));
    s_state = atoi(parseJson("\"state\":"));

    // set some items ---------------------------------------------------------------------------------------------
    if (!s_settings.lastconnectedfile.starts_with("/")) { s_settings.lastconnectedfile.assign("/audiofiles/"); } // guard
    s_SD_content.setLastConnectedFile(s_settings.lastconnectedfile.get());
    x_ps_free(&s_cur_AudioFolder);
    s_cur_AudioFolder = x_ps_strdup(s_SD_content.getLastConnectedFolder());
    x_ps_free(&s_cur_AudioFileName);
    s_cur_AudioFileName = x_ps_strdup(s_SD_content.getLastConnectedFileName());
    s_cur_AudioFileNr = s_SD_content.getPosByFileName(s_cur_AudioFileName);
    if (s_cur_AudioFileNr == -1) s_cur_AudioFileNr = 0; // not found
    // ------------------------------------------------------------------------------------------------------------
    x_ps_free(&jO);
    x_ps_free(&tmp);

    if (!SD_MMC.exists("/stations.json")) { // if not found create one
        File file1 = SD_MMC.open("/stations.json", "w", true);
        file1.write((uint8_t*)stations_json, sizeof(stations_json) - 1); // without termination
        file1.close();
    }
    staMgnt.updateStationsList();
    return true;
}
// clang-format on  🟢🟡🔴

void updateSettings() {
    if (!s_settings.lastconnectedhost.valid()) s_settings.lastconnectedhost.assign("");
    if (!s_settings.lastconnectedfile.valid()) s_settings.lastconnectedfile.assign("/audiofiles/");
    ps_ptr<char> jO;
    ; // JSON Object
    jO.assign("{\n");
    jO.appendf("  \"volume\":%i", s_volume.cur_volume);
    jO.appendf(",\n  \"volumeSteps\":%i", s_volume.volumeSteps);
    jO.appendf(",\n  \"ringVolume\":%i", s_volume.ringVolume);
    jO.appendf(",\n  \"volumeAfterAlarm\":%i", s_volume.volumeAfterAlarm);
    jO.appendf(",\n  \"BTvolume\":%i", s_bt_emitter.volume);
    jO.appendf(",\n  \"BTpower\":");
    s_bt_emitter.power_state == true ? jO.appendf("\"true\"") : jO.appendf("\"false\"");
    jO.appendf(",\n  \"BTmode\":\"%s\"", bt_emitter.getMode().c_get());
    jO.appendf(",\n  \"alarmtime_sun\":\"%02d:%02d\"", s_alarmtime[0] / 60, s_alarmtime[0] % 60);
    jO.appendf(",\n  \"alarmtime_mon\":\"%02d:%02d\"", s_alarmtime[1] / 60, s_alarmtime[1] % 60);
    jO.appendf(",\n  \"alarmtime_tue\":\"%02d:%02d\"", s_alarmtime[2] / 60, s_alarmtime[2] % 60);
    jO.appendf(",\n  \"alarmtime_wed\":\"%02d:%02d\"", s_alarmtime[3] / 60, s_alarmtime[3] % 60);
    jO.appendf(",\n  \"alarmtime_thu\":\"%02d:%02d\"", s_alarmtime[4] / 60, s_alarmtime[4] % 60);
    jO.appendf(",\n  \"alarmtime_fri\":\"%02d:%02d\"", s_alarmtime[5] / 60, s_alarmtime[5] % 60);
    jO.appendf(",\n  \"alarmtime_sat\":\"%02d:%02d\"", s_alarmtime[6] / 60, s_alarmtime[6] % 60);
    jO.appendf(",\n  \"alarm_weekdays\":%i", s_alarmdays);
    jO.appendf(",\n  \"timeAnnouncing\":");
    (s_f_timeAnnouncement == true) ? jO.appendf("\"true\"") : jO.appendf("\"false\"");
    jO.appendf(",\n  \"timeSpeechLang\":\"%s\"", s_timeSpeechLang);
    jO.appendf(",\n  \"mute\":");
    (s_f_mute == true) ? jO.appendf("\"true\"") : jO.appendf("\"false\"");
    jO.appendf(",\n  \"brightness\":%i", s_brightness);
    jO.appendf(",\n  \"sleeptime\":%i", s_sleeptime);
    jO.appendf(",\n  \"lastconnectedhost\":\"%s\"", s_settings.lastconnectedhost.c_get());
    jO.appendf(",\n  \"lastconnectedfile\":\"%s\"", s_settings.lastconnectedfile.c_get());
    jO.appendf(",\n  \"station\":%i", s_cur_station);
    jO.appendf(",\n  \"Timezone_Name\":\"%s\"", s_TZName.c_str());
    jO.appendf(",\n  \"Timezone_String\":\"%s\"", s_TZString.c_str());
    jO.appendf(",\n  \"toneLP\":%i", s_toneLP);
    jO.appendf(",\n  \"toneBP\":%i", s_toneBP);
    jO.appendf(",\n  \"toneHP\":%i", s_toneHP);
    jO.appendf(",\n  \"balance\":%i", s_toneBAL);
    jO.appendf(",\n  \"timeFormat\":%i", s_timeFormat);
    jO.appendf(",\n  \"state\":%i", s_state);
    jO.appendf(",\n  \"sleepMode\":%i\n}", s_sleepMode);

    if (s_settingsHash != simpleHash(jO.get())) {
        File file = SD_MMC.open("/settings.json", "w", false);
        if (!file) {
            log_e("file \"settings.json\" not found");
            return;
        }
        file.print(jO.get());
        s_settingsHash = simpleHash(jO.c_get());
    }
}
/*****************************************************************************************************************************************************
 *                                                    T F T   B R I G H T N E S S                                                                    *
 *****************************************************************************************************************************************************/
void setTFTbrightness(uint8_t duty) { // duty 0...100 (min...max)
    if (TFT_BL == -1) return;
    uint8_t d = round((double)duty * 2.55); // #186
    ledcWrite(TFT_BL, d);
}

int16_t getTFTbrightness() { // duty 0...100 (min...max)
    if (TFT_BL == -1) return -1;
    return ledcRead(TFT_BL);
}

/*****************************************************************************************************************************************************
 *                                                      U R L d e c o d e                                                                            *
 *****************************************************************************************************************************************************/
// In m3u playlists, file names can be URL encoded.
// Since UTF-8 is always shorter than URI, the same memory is used for decoding
// e.g. Born%20On%20The%20B.mp3 --> Born On The B.mp3
// e.g. %D0%B8%D1%81%D0%BF%D1%8B%D1%82%D0%B0%D0%BD%D0%B8%D0%B5.mp3 --> испытание.mp3
void urldecode(char* str) {
    uint16_t p1 = 0, p2 = 0;
    char     a, b;
    while (str[p1]) {
        if ((str[p1] == '%') && ((a = str[p1 + 1]) && (b = str[p1 + 2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A')
                a -= ('A' - 10);
            else
                a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A')
                b -= ('A' - 10);
            else
                b -= '0';
            str[p2++] = 16 * a + b;
            p1 += 3;
        } else if (str[p1] == '+') {
            str[p2++] = ' ';
            p1++;
        } else {
            str[p2++] = str[p1++];
        }
    }
    str[p2++] = '\0';
}

/*****************************************************************************************************************************************************
 *                                                               T I M E R                                                                           *
 *****************************************************************************************************************************************************/

// clang-format off
void timer100ms(){
    static uint16_t ms100 = 0;
    s_f_100ms = true;
    ms100 ++;
    if(!(ms100 % 10))   {
        s_f_1sec  = true;
        s_time_s = rtc.gettime_s();
        if(endsWith(s_time_s, "59:53"))  s_f_timeSpeech = true;
    }
    if(!(ms100 % 100))  s_f_10sec = true;
    if(!(ms100 % 600)) {s_f_1min  = true; ms100 = 0;}
}
// clang-format on

/*****************************************************************************************************************************************************
 *                                                               D I S P L A Y                                                                       *
 *****************************************************************************************************************************************************/

inline void bgColorHeader() {
    tft.fillRect(layout.winHeader.x, layout.winHeader.y, layout.winHeader.w, layout.winHeader.h, TFT_BLACK);
}
inline void bgColorLogo() {
    tft.fillRect(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, TFT_BLACK);
}
inline void bgColorStationName() {
    tft.fillRect(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, TFT_BLACK);
}
inline void bgColorLogoAndStationname() {
    tft.fillRect(layout.winFName.x, layout.winFName.y, layout.winFName.w, layout.winFName.h, TFT_BLACK);
}
inline void bgColorTitle() {
    tft.fillRect(layout.winTitle.x, layout.winTitle.y, layout.winTitle.w, layout.winTitle.h, TFT_BLACK);
} // incl. VUmeter
inline void bgColorStreamTitle() {
    tft.fillRect(layout.winSTitle.x, layout.winSTitle.y, layout.winSTitle.w, layout.winSTitle.h, TFT_BLACK);
} // without VUmeter
inline void bgColorWithOutHeaderFooter() {
    tft.fillRect(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, TFT_BLACK);
}
inline void bgColorFooter() {
    tft.fillRect(layout.winFooter.x, layout.winFooter.y, layout.winFooter.w, layout.winFooter.h, TFT_BLACK);
}
inline void bgColorStaNr() {
    tft.fillRect(layout.winStaNr.x, layout.winStaNr.y, layout.winStaNr.w, layout.winStaNr.h, TFT_BLACK);
}
inline void bgColorSleep() {
    tft.fillRect(layout.winSleep.x, layout.winSleep.y, layout.winSleep.w, layout.winSleep.h, TFT_BLACK);
}
inline void bgColorDigits() {
    tft.fillRect(layout.winDigits.x, layout.winDigits.y, layout.winDigits.w, layout.winDigits.h, TFT_BLACK);
}
inline void bgColorButtonBar() {
    tft.fillRect(0, layout.winButton.y, displayConfig.dispWidth, layout.winButton.h, TFT_BLACK);
}
inline void bgColorAll() {
    tft.fillScreen(TFT_BLACK);
} // y   0...239

inline void clearHeader() {
    tft.copyFramebuffer(1, 0, layout.winHeader.x, layout.winHeader.y, layout.winHeader.w, layout.winHeader.h);
}
inline void clearLogo() {
    tft.copyFramebuffer(1, 0, layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h);
}
inline void clearStationName() {
    tft.copyFramebuffer(1, 0, layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
}
inline void clearLogoAndStationname() {
    tft.copyFramebuffer(1, 0, layout.winFName.x, layout.winFName.y, layout.winFName.w, layout.winFName.h);
}
inline void clearTitle() {
    tft.copyFramebuffer(1, 0, layout.winTitle.x, layout.winTitle.y, layout.winTitle.w, layout.winTitle.h);
} // incl. VUmeter
inline void clearStreamTitle() {
    tft.copyFramebuffer(1, 0, layout.winSTitle.x, layout.winSTitle.y, layout.winSTitle.w, layout.winSTitle.h);
} // without VUmeter
inline void clearWithOutHeaderFooter() {
    tft.copyFramebuffer(1, 0, layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h);
}
inline void clearFooter() {
    tft.copyFramebuffer(1, 0, layout.winFooter.x, layout.winFooter.y, layout.winFooter.w, layout.winFooter.h);
}
inline void clearStaNr() {
    tft.copyFramebuffer(1, 0, layout.winStaNr.x, layout.winStaNr.y, layout.winStaNr.w, layout.winStaNr.h);
}
inline void clearSleep() {
    tft.copyFramebuffer(1, 0, layout.winSleep.x, layout.winSleep.y, layout.winSleep.w, layout.winSleep.h);
}
inline void clearDigits() {
    tft.copyFramebuffer(1, 0, layout.winDigits.x, layout.winDigits.y, layout.winDigits.w, layout.winDigits.h);
}
inline void clearButtonBar() {
    tft.copyFramebuffer(1, 0, 0, layout.winButton.y, displayConfig.dispWidth, layout.winButton.h);
}
inline void clearAll() {
    tft.copyFramebuffer(1, 0, 0, 0, displayConfig.dispWidth, displayConfig.dispHeight);
}

// clang-format on

inline uint16_t txtlen(String str) {
    uint16_t len = 0;
    for (int32_t i = 0; i < str.length(); i++)
        if (str[i] <= 0xC2) len++;
    return len;
}

void showStreamTitle(const char* streamtitle) {
    if (s_f_sleeping) return;

    char* st = x_ps_strdup(streamtitle);
    trim(st);
    // replacestr(st, " | ", "\n"); // some stations use pipe as \n or
    // replacestr(st, "| ", "\n");
    // replacestr(st, "|", "\n");

    txt_RA_sTitle.setTextColor(TFT_CORNSILK);
    txt_RA_sTitle.writeText(st);

    x_ps_free(&st);
}

void showLogoAndStationName(bool force) {
    char*        SN_utf8 = NULL;
    static char* old_SN_utf8 = strdup("");
    if (force) {
        if (old_SN_utf8) {
            x_ps_free(&old_SN_utf8);
            old_SN_utf8 = strdup("");
        }
    }

    if (s_cur_station) {
        //    log_w("showLogoAndStationName: %s", staMgnt.getStationName(s_cur_station));
        SN_utf8 = x_ps_calloc(strlen(staMgnt.getStationName(s_cur_station)) + 12, 1);
        memcpy(SN_utf8, staMgnt.getStationName(s_cur_station), strlen(staMgnt.getStationName(s_cur_station)) + 1);
        SerialPrintfln("Country: ..  " ANSI_ESC_GREEN "%s", staMgnt.getStationCountry(s_cur_station));
    } else {
        if (!s_stationName_air) s_stationName_air = strdup("");
        SN_utf8 = x_ps_calloc(strlen(s_stationName_air) + 12, 1);
        memcpy(SN_utf8, s_stationName_air, strlen(s_stationName_air) + 1);
    }
    trim(SN_utf8);
    if (strcmp(old_SN_utf8, SN_utf8) == 0) { goto exit; }
    x_ps_free(&old_SN_utf8);
    old_SN_utf8 = x_ps_strdup(SN_utf8);
    txt_RA_staName.setTextColor(TFT_CYAN);
    txt_RA_staName.setText(SN_utf8);
    txt_RA_staName.show(true, false);

    memmove(SN_utf8 + 6, SN_utf8, strlen(SN_utf8) + 1);
    memmove(SN_utf8, "/logo/", 6);
    strcat(SN_utf8, ".jpg");
    pic_RA_logo.setPicturePath(SN_utf8);
    pic_RA_logo.setAlternativPicturePath("/common/unknown.png");
    pic_RA_logo.show(true, false);
exit:
    x_ps_free(&SN_utf8);
}

void showFileLogo(uint8_t state) {
    String logo;
    if (state == RADIO) {
        if (endsWith(s_stationURL, "m3u8"))
            logo = "/common/" + (String) "M3U8" + ".png";
        else
            logo = "/common/" + (String)codecname[s_cur_Codec] + ".png";
        pic_RA_logo.setPicturePath(logo.c_str());
        pic_RA_logo.setAlternativPicturePath("/common/unknown.png");
        pic_RA_logo.show(true, false);
        webSrv.send("stationLogo=", logo);
        return;
    } else if (state == DLNA) {
        logo = "/common/DLNA.jpg";
        pic_DL_logo.setPicturePath(logo.c_str());
        pic_DL_logo.setAlternativPicturePath("/common/unknown.png");
        pic_DL_logo.show(true, false);
        webSrv.send("stationLogo=", logo);
        return;
    }
    if (state == PLAYER) { // s_state PLAYER
        if (s_cur_Codec == 0)
            logo = "/common/AudioPlayer.png";
        else if (s_playerSubMenue == 0)
            logo = "/common/AudioPlayer.png";
        else
            logo = "/common/" + (String)codecname[s_cur_Codec] + ".png";
        pic_PL_logo.setPicturePath(logo.c_str());
        pic_PL_logo.setAlternativPicturePath("/common/unknown.png");
        pic_PL_logo.show(true, false);
        return;
    }
    if (state == SETTINGS) {
        logo = "/common/Settings.png";
        pic_SE_logo.setPicturePath(logo.c_str());
        pic_SE_logo.setAlternativPicturePath("/common/unknown.png");
        pic_SE_logo.show(true, false);
        return;
    }
    if (state == RINGING) {
        logo = "/common/Alarm.png";
        pic_RI_logo.setPicturePath(logo.c_str());
        pic_RI_logo.setAlternativPicturePath("/common/unknown.png");
        pic_RI_logo.show(true, false);
        return;
    }
}

void showFileName(const char* fname) {
    if (!fname) return;
    txt_PL_fName.setTextColor(TFT_CYAN);
    txt_PL_fName.writeText(fname);
}

void showPlsFileNumber() {
    txt_PL_fNumber.setTextColor(TFT_ORANGE);
    char buf[15];
    sprintf(buf, "%03u/%03u", s_plsCurPos, s_PLS_content.size());
    txt_PL_fNumber.writeText(buf);
}

void showAudioFileNumber() {
    txt_PL_fNumber.setTextColor(TFT_ORANGE);
    char buf[15];
    sprintf(buf, "%03u/%03u", s_cur_AudioFileNr + 1, s_SD_content.getSize());
    txt_PL_fNumber.writeText(buf);
}

void display_sleeptime(int8_t ud) { // set sleeptimer
    if (ud == 1) {
        switch (s_sleeptime) {
            case 0 ... 14: s_sleeptime = (s_sleeptime / 5) * 5 + 5; break;
            case 15 ... 59: s_sleeptime = (s_sleeptime / 15) * 15 + 15; break;
            case 60 ... 359: s_sleeptime = (s_sleeptime / 60) * 60 + 60; break;
            default: s_sleeptime = 360; break; // max 6 hours
        }
    }
    if (ud == -1) {
        switch (s_sleeptime) {
            case 1 ... 15: s_sleeptime = ((s_sleeptime - 1) / 5) * 5; break;
            case 16 ... 60: s_sleeptime = ((s_sleeptime - 1) / 15) * 15; break;
            case 61 ... 360: s_sleeptime = ((s_sleeptime - 1) / 60) * 60; break;
            default: s_sleeptime = 0; break; // min
        }
    }
    char tmp[10];
    sprintf(tmp, "%d%02d", s_sleeptime / 60, s_sleeptime % 60);
    char path[128] = "/digits_small/";

    for (uint8_t i = 0; i < 4; i++) {
        strcpy(path, "/digits_small/");
        if (i == 3) {
            if (!s_sleeptime)
                strcat(path, "dsgn.jpg");
            else
                strcat(path, "dsrt.jpg");
        } else {
            strncat(path, (tmp + i), 1);
            if (!s_sleeptime)
                strcat(path, "sgn.jpg");
            else
                strcat(path, "srt.jpg");
        }
        drawImage(path, displayConfig.sleeptimeXPos[i], displayConfig.sleeptimeYPos);
    }
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth, uint16_t maxHeigth) {
    const char* scImg = scaleImage(path);
    if (!SD_MMC.exists(scImg)) {
        if (indexOf(scImg, "/.", 0) > 0) return false; // empty filename
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "file \"%s\" not found", scImg);
        return false;
    }
    if (endsWith(scImg, "bmp")) { return tft.drawBmpFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth, 1.0); }
    if (endsWith(scImg, "jpg")) { return tft.drawJpgFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth); }
    if (endsWith(scImg, "gif")) { return tft.drawGifFile(SD_MMC, scImg, posX, posY, 0); }
    if (endsWith(scImg, "png")) { return tft.drawPngFile(SD_MMC, scImg, posX, posY); }

    SerialPrintfln(ANSI_ESC_RED "the file \"%s\" contains neither a bmp, a gif nor a jpg graphic", scImg);
    return false; // neither jpg nor bmp
}
/*****************************************************************************************************************************************************
 *                                                   H A N D L E  A U D I O F I L E                                                                  *
 *****************************************************************************************************************************************************/

boolean isAudio(File file) {
    if (endsWith(file.name(), ".mp3") || endsWith(file.name(), ".aac") || endsWith(file.name(), ".m4a") || endsWith(file.name(), ".wav") || endsWith(file.name(), ".flac") ||
        endsWith(file.name(), ".opus") || endsWith(file.name(), ".ogg")) {
        return true;
    }
    return false;
}

boolean isAudio(const char* path) {
    if (endsWith(path, ".mp3") || endsWith(path, ".aac") || endsWith(path, ".m4a") || endsWith(path, ".wav") || endsWith(path, ".flac") || endsWith(path, ".opus") || endsWith(path, ".ogg")) {
        return true;
    }
    return false;
}

boolean isPlaylist(File file) {
    if (endsWith(file.name(), ".m3u")) { return true; }
    return false;
}

/*****************************************************************************************************************************************************
 *                                                                     P L A Y L I S T                                                               *
 *****************************************************************************************************************************************************/

bool preparePlaylistFromFile(const char* path) { // *.m3u
    File playlistFile = SD_MMC.open(path);
    if (!playlistFile) {
        log_e("playlistfile path not found");
        return false;
    }

    if (playlistFile.size() > 1048576) {
        log_e("Playlist too big, size > 1MB");
        playlistFile.close();
        return false;
    }
    x_ps_free(&s_playlistPath);
    vector_clear_and_shrink(s_PLS_content); // clear s_PLS_content first
    char* buff1 = x_ps_malloc(2024);
    char* buff2 = x_ps_malloc(1048);
    if (!buff1 || !buff2) {
        log_e("oom");
        playlistFile.close();
        return false;
    }
    size_t bytesRead = 0;
    bool   f_EXTINF_seen = false;

    while (playlistFile.available() > 0) {
        bytesRead = playlistFile.readBytesUntil('\n', buff1, 1024);
        if (bytesRead < 5) continue; // line is # or space or nothing, smallest filename "1.mp3" < 5
        buff1[bytesRead] = '\0';
        trim(buff1);
        if (startsWith(buff1, "#EXTM3U")) continue;
        if (startsWith(buff1, "#EXTINF:")) { // #EXTINF:8,logo-1.mp3
            strcpy(buff2, buff1 + 8);
            f_EXTINF_seen = true;
            continue;
        }
        if (startsWith(buff1, "#")) continue; // all other lines
        if (f_EXTINF_seen) {
            f_EXTINF_seen = false;
            strcat(buff1, "\n");
            strcat(buff1, buff2);
        }
        s_PLS_content.push_back(x_ps_strdup((const char*)buff1));
    }
    s_playlistPath = x_ps_strdup(playlistFile.path());
    int idx = lastIndexOf((const char*)s_playlistPath, '/');
    if (idx < 0) log_e("wrong playlist path");
    s_playlistPath[idx] = '\0';
    playlistFile.close();
    x_ps_free(&buff1);
    x_ps_free(&buff2);
    return true;
}
//____________________________________________________________________________________________________________________________________________________

bool preparePlaylistFromSDFolder(const char* path) { // all files within a SD folder
    if (!SD_MMC.exists(path)) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
        return false;
    }
    File folder = SD_MMC.open(path);
    if (!folder.isDirectory()) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
        folder.close();
        return false;
    }
    vector_clear_and_shrink(s_PLS_content); // clear _PLS_content first

    while (true) { // get content
        File file = folder.openNextFile();
        if (!file) break;
        if (file.isDirectory()) continue;
        if (isAudio(file)) { s_PLS_content.push_back(x_ps_strdup((const char*)file.path())); }
        file.close();
    }
    folder.close();

    for (int i = 0; i < s_PLS_content.size(); i++) {
        if (s_PLS_content[i][0] == 2) { // remove ASCII 2
            memcpy(s_PLS_content[i], s_PLS_content[i] + 1, strlen(s_PLS_content[i]));
        }
    }

    if (s_f_shuffle)
        sortPlayListRandom();
    else
        sortPlayListAlphabetical();

    return true;
}
//____________________________________________________________________________________________________________________________________________________
bool preparePlaylistFromDLNAFolder() {
    vector_clear_and_shrink(s_PLS_content); // clear _PLS_content first
    const std::deque<DLNA_Client::srvItem>* foldercontent = &dlna.getBrowseResult();
    for (int i = 0; i < foldercontent->size(); i++) {
        // log_i( "%d : (%d) %s %s -- %s",i, foldercontent.isAudio[i], foldercontent.itemURL[i], foldercontent.title[i], foldercontent.duration[i]);
        if (!foldercontent->at(i).isAudio) continue;
        uint16_t len =
            strlen((const char*)foldercontent->at(i).itemURL.c_get()) + strlen((const char*)foldercontent->at(i).title.c_get()) + strlen((const char*)foldercontent->at(i).duration.c_get()) + 3;
        char* itstr = x_ps_malloc(len);
        strcpy(itstr, (const char*)foldercontent->at(i).itemURL.c_get());
        strcat(itstr, "\n");
        strcat(itstr, (const char*)foldercontent->at(i).duration.c_get());
        strcat(itstr, ",");
        strcat(itstr, (const char*)foldercontent->at(i).title.c_get());
        log_i("pushing to playlist : %s", itstr);
        s_PLS_content.push_back(itstr);
    }
    if (!s_PLS_content.size()) return false;
    log_i("pls length %i", s_PLS_content.size());
    return true;
}
//____________________________________________________________________________________________________________________________________________________

void sortPlayListAlphabetical() {
    for (int i = 0; i < s_PLS_content.size(); i++) { // easy bubble sort
        for (int j = 1; j < s_PLS_content.size(); j++) {
            if (strcmp(s_PLS_content[j - 1], s_PLS_content[i]) > 0) { swap(s_PLS_content[i], s_PLS_content[j - 1]); }
        }
    }
}
//____________________________________________________________________________________________________________________________________________________

void sortPlayListRandom() {
    for (int i = 0; i < s_PLS_content.size(); i++) { // easy bubble sort
        uint16_t randIndex = random(0, s_PLS_content.size());
        swap(s_PLS_content[i], s_PLS_content[randIndex]); // swapping the values
    }
}
//____________________________________________________________________________________________________________________________________________________

void processPlaylist(boolean first) {

    char *playFile = NULL, *duration = NULL, *title = NULL, *plsContent = NULL, *plsExtension = NULL;
    bool  f_hasTitle = false, f_isURL = false, f_isRoot = false, f_isConnected = false;

    if (s_PLS_content.size() == 0) {
        log_e("playlist is empty");
        return;
    } // guard
    if (s_plsCurPos == s_PLS_content.size()) goto exit; // end of playlist
    if (first) {
        s_plsCurPos = 0;
        s_f_playlistEnabled = true;
    } // reset before start

    if (indexOf(s_PLS_content[s_plsCurPos], "\n", 0) > 0) f_hasTitle = true; // has additional infos: duration, title
    // now read from vector _PLS_content
    if (startsWith(s_PLS_content[s_plsCurPos], "http")) f_isURL = true;     // is web file
    if (startsWith(s_PLS_content[s_plsCurPos], "file://")) f_isRoot = true; // SD file from root
    if (f_hasTitle) {
        int idx = indexOf(s_PLS_content[s_plsCurPos], "\n", 0);
        plsContent = x_ps_strndup(s_PLS_content[s_plsCurPos], idx);
        plsExtension = x_ps_strdup(s_PLS_content[s_plsCurPos] + idx + 1);
    } else {
        plsContent = x_ps_strdup(s_PLS_content[s_plsCurPos]);
    }

    s_cur_Codec = 0;
    if (f_isURL) { // is web file
        playFile = x_ps_strdup(plsContent);
        connecttohost(playFile);
        f_isConnected = s_f_isWebConnected;
    } else { // is local file
        if (f_isRoot) {
            playFile = x_ps_strdup(plsContent + 7); // skip "file://"
            urldecode(playFile);
            f_isConnected = s_f_isFSConnected;
        } else { // local file, not root
            urldecode(plsContent);
            if (s_playlistPath) { // path of m3u file
                playFile = x_ps_malloc(strlen(s_playlistPath) + strlen(plsContent) + 5);
                strcpy(playFile, s_playlistPath);
                if (plsContent[0] != '/') strcat(playFile, "/");
                strcat(playFile, plsContent);
            } else { // have no playlistpath
                playFile = x_ps_malloc(strlen(plsContent) + 5);
                strcpy(playFile, plsContent);
            }
        }
        connecttoFS("SD_MMC", playFile);
        f_isConnected = s_f_isFSConnected;
    }
    if (f_isConnected) {
        SerialPrintflnCut("Playlist:    ", ANSI_ESC_YELLOW, playFile);
        webSrv.send("SD_playFile=", playFile);
        s_playerSubMenue = 1;
        changeState(PLAYER);
        if (f_hasTitle) {
            int16_t idx1 = indexOf(plsExtension, ",", 0); // "18,logo-1.mp3"
            duration = x_ps_strndup(plsExtension, idx1);
            title = x_ps_strdup(plsExtension + idx1 + 1);
            SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "Title: %s", title);
            showFileName(title);
        }
    }
    x_ps_free(&playFile);
    x_ps_free(&duration);
    x_ps_free(&title);
    x_ps_free(&plsContent);
    x_ps_free(&plsExtension);
    s_plsCurPos++;
    showPlsFileNumber();
    return;

exit:
    SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "end of playlist");
    webSrv.send("SD_playFile=", "end of playlist");
    s_f_playlistEnabled = false;
    s_playerSubMenue = 0;
    changeState(PLAYER);
    s_plsCurPos = 0;
    x_ps_free(&playFile);
    x_ps_free(&duration);
    x_ps_free(&title);
    x_ps_free(&plsContent);
    x_ps_free(&plsExtension);
    x_ps_free(&s_playlistPath);
    return;
}
/*****************************************************************************************************************************************************
 *                                         C O N N E C T   TO   W I F I     /     A C C E S S P O I N T                                              *
 *****************************************************************************************************************************************************/
bool connectToWiFi() {

    char* line = x_ps_malloc(512);
    if (!line) {
        log_e("oom");
        return false;
    }

    // create nvs entries if they do not exist
    if (!pref.isKey("wifiStr0")) pref.putString("wifiStr0", ""); // SSID + \t + PW
    if (!pref.isKey("wifiStr1")) pref.putString("wifiStr1", "");
    if (!pref.isKey("wifiStr2")) pref.putString("wifiStr2", "");
    if (!pref.isKey("wifiStr3")) pref.putString("wifiStr3", "");
    if (!pref.isKey("wifiStr4")) pref.putString("wifiStr4", "");
    if (!pref.isKey("wifiStr5")) pref.putString("wifiStr5", "");

    const char* SSID = _SSID;
    const char* PW = _PW;
    line[0] = '\0';
    strcpy(line, SSID);
    strcat(line, "\t");
    strcat(line, PW);
    pref.putString("wifiStr0", line);

    for (int i = 0; i < 6; i++) {
        line[0] = '\0'; // Move this line outside the switch statement
        switch (i) {
            case 0: strcpy(line, pref.getString("wifiStr0").c_str()); break;
            case 1: strcpy(line, pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line, pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line, pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line, pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line, pref.getString("wifiStr5").c_str()); break;
        }
        if (strlen(line) < 5) continue;   // line is empty
        int pos = indexOf(line, "\t", 0); // find first tab
        if (pos < 0) continue;            // no tab found
        line[pos] = '\0';                 // terminate ssid
        char* ssid = line;                // ssid is the first part
        char* pw = line + pos + 1;        // password is the second part
        WiFi.mode(WIFI_STA);
        wifiMulti.addAP(ssid, pw); // SSID and PW in code"
        size_t offset = 0;
        size_t pwlen = strlen(pw);
        size_t dot_len = strlen(emoji.blueCircle); // = 4
        size_t buf_size = pwlen * dot_len + 1;     // +1 für '\0'
        char   pass[buf_size];
        for (size_t j = 0; j < pwlen; j++) {
            memcpy(pass + offset, emoji.blueCircle, dot_len);
            offset += dot_len;
        }
        pass[offset] = '\0'; // Zero-terminate the string
        SerialPrintfln("WiFI_info:   add credentials: " ANSI_ESC_CYAN "%s - %s" ANSI_ESC_YELLOW " [%s:%d]", ssid, pass, __FILENAME__, __LINE__);
    }

    wifiMulti.setStrictMode(true);
    WiFi.mode(WIFI_STA);
    wifiMulti.run();
    SerialPrintfln("WiFI_info:   Connecting WiFi...");
    if (WiFi.isConnected()) {
        WiFi.setAutoReconnect(true);
        if (WIFI_TX_POWER >= 2 && WIFI_TX_POWER <= 21) WiFi.setTxPower((wifi_power_t)(WIFI_TX_POWER * 4));
        return true;
    } else {
        WiFi.mode(WIFI_MODE_NULL);
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "WiFi credentials are not correct");
        return false; // can't connect to any network
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void setWiFiCredentials(const char* ssid, const char* password) {
    if (!ssid || !password) return;
    if (strlen(ssid) < 5) return; // min length

    log_e("ssid %s pw %s", ssid, password);

    char* line = x_ps_malloc(512);
    int   i = 0, state = 0;
    if (!line) {
        log_e("oom");
        goto exit;
    }

    for (i = 0; i < 6; i++) {
        line[0] = '\0'; // Move this line outside the switch statement
        switch (i) {
            case 0: strcpy(line, pref.getString("wifiStr0").c_str()); break;
            case 1: strcpy(line, pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line, pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line, pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line, pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line, pref.getString("wifiStr5").c_str()); break;
        }
        if (startsWith(line, ssid) && line[strlen(ssid)] == '\t') { // ssid found, update password
            line[0] = '\0';                                         // clear line
            strcat(line, ssid);
            strcat(line, "\t");
            strcat(line, password);
            if (i == 0) {
                log_e("password can't changed, is hard coded");
                state = 0;
                goto exit;
            }
            if (i == 1) {
                pref.putString("wifiStr1", line);
                state = 1;
                goto exit;
            }
            if (i == 2) {
                pref.putString("wifiStr2", line);
                state = 1;
                goto exit;
            }
            if (i == 3) {
                pref.putString("wifiStr3", line);
                state = 1;
                goto exit;
            }
            if (i == 4) {
                pref.putString("wifiStr4", line);
                state = 1;
                goto exit;
            }
            if (i == 5) {
                pref.putString("wifiStr5", line);
                state = 1;
                goto exit;
            }
        }
    }
    for (i = 1; i < 6; i++) {
        line[0] = '\0';
        switch (i) {
            case 1: strcpy(line, pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line, pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line, pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line, pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line, pref.getString("wifiStr5").c_str()); break;
        }
        if (strlen(line) < 5) { // line is empty
            line[0] = '\0';     // clear line
            strcat(line, ssid);
            strcat(line, "\t");
            strcat(line, password);
            if (i == 1) {
                pref.putString("wifiStr1", line);
                state = 2;
                goto exit;
            }
            if (i == 2) {
                pref.putString("wifiStr2", line);
                state = 2;
                goto exit;
            }
            if (i == 3) {
                pref.putString("wifiStr3", line);
                state = 2;
                goto exit;
            }
            if (i == 4) {
                pref.putString("wifiStr4", line);
                state = 2;
                goto exit;
            }
            if (i == 5) {
                pref.putString("wifiStr5", line);
                state = 2;
                goto exit;
            }
        }
    }
    state = 3;

exit:
    x_ps_free(&line); // free memory
    if (state == 0) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "SSID: %s password can't changed, it is hard coded", ssid);
        if (s_f_WiFiConnected) audio.connecttospeech("S S I D and password are hard coded", "en");
    }
    if (state == 1) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The passord \"%s\" for the SSID: %s has been changed", password, ssid);
        if (s_f_WiFiConnected) audio.connecttospeech("The password has been changed", "en");
    }
    if (state == 2) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The SSID: %s has been added", ssid);
        if (s_f_WiFiConnected) audio.connecttospeech("The credentials has been added", "en");
    }
    if (state == 3) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "No more memory to save the credentials for: %s", ssid);
        if (s_f_WiFiConnected) audio.connecttospeech("The S S I D and password can't be stored", "en");
    }
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*****************************************************************************************************************************************************
 *                                                                     A U D I O                                                                     *
 *****************************************************************************************************************************************************/
void connecttohost(const char* host) {
    int32_t idx1, idx2;
    char*   url = nullptr;
    char*   user = nullptr;
    char*   pwd = nullptr;

    dispFooter.updateBitRate(0);
    s_cur_Codec = 0;
    //    if(s_state == RADIO) clearStreamTitle();
    s_icyBitRate = 0;
    s_decoderBitRate = 0;
    s_f_webFailed = false;

    idx1 = indexOf(host, "|", 0);
    if (idx1 == -1) { // no pipe found
        s_f_isWebConnected = audio.connecttohost(host);

        if (!s_f_isWebConnected) {
            s_cthFailCounter++;
        } else
            (s_cthFailCounter = 0);

        s_f_isFSConnected = false;
        return;
    } else { // pipe found     e.g. http://xxx.com/ext|user|pw
        idx2 = indexOf(host, "|", idx1 + 1);
        // log_i("idx2 = %i", idx2);
        if (idx2 == -1) { // second pipe not found
            s_f_isWebConnected = audio.connecttohost(host);

            if (!s_f_isWebConnected) {
                s_cthFailCounter++;
            } else
                (s_cthFailCounter = 0);

            s_f_isFSConnected = false;
            return;
        } else {                       // extract url, user, pwd
            url = strndup(host, idx1); // extract url
            user = strndup(host + idx1 + 1, idx2 - idx1 - 1);
            pwd = strdup(host + idx2 + 1);
            SerialPrintfln("new host: .  %s user %s, pwd %s", url, user, pwd);
            s_f_isWebConnected = audio.connecttohost(url, user, pwd);
            s_f_isFSConnected = false;
            x_ps_free(&url);
            x_ps_free(&user);
            x_ps_free(&pwd);
        }
    }
    if (s_cthFailCounter >= 3) {
        audio.connecttospeech("The last hosts were not connected", "en");
        s_settings.lastconnectedhost.assign("");
    }
}
void connecttoFS(const char* FS, const char* filename, uint32_t fileStartTime) {
    if (!filename) return;
    dispFooter.updateBitRate(0);
    s_icyBitRate = 0;
    s_decoderBitRate = 0;
    s_cur_Codec = 0;
    s_f_webFailed = false;
    s_f_isFSConnected = audio.connecttoFS(SD_MMC, filename, fileStartTime);
    s_f_isWebConnected = false;
    if (!startsWith(filename, "/audiofiles/")) { return; }
    if (s_f_isFSConnected && isAudio(filename)) {
        s_settings.lastconnectedfile.copy_from(filename);
        s_SD_content.setLastConnectedFile(filename);
        x_ps_free(&s_cur_AudioFolder);
        s_cur_AudioFolder = x_ps_strdup(s_SD_content.getLastConnectedFolder());
        x_ps_free(&s_cur_AudioFileName);
        s_cur_AudioFileName = x_ps_strdup(s_SD_content.getLastConnectedFileName());
        s_cur_AudioFileNr = s_SD_content.getPosByFileName(s_cur_AudioFileName);
        if (s_cur_AudioFileNr == -1) s_cur_AudioFileNr = 0;
    }
    //    log_w("Filesize %d", audioGetFileSize());
    //    log_w("FilePos %d", audioGetFilePosition());
}
void stopSong() {
    audio.stopSong();
    s_f_isFSConnected = false;
    s_f_isWebConnected = false;
    if (s_f_playlistEnabled) {
        vector_clear_and_shrink(s_PLS_content);
        s_f_playlistEnabled = false;
        SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "playlist stopped");
        webSrv.send("SD_playFile=", "playlist stopped");
    }
    s_f_pauseResume = false;
    s_f_playlistNextFile = false;
    s_f_shuffle = false;
    x_ps_free(&s_playlistPath);
}

/*****************************************************************************************************************************************************
 *                                                                    S E T U P                                                                      *
 *****************************************************************************************************************************************************/

void setup() {
    Audio::audio_info_callback = my_audio_info; // audio callback
    dlna.dlna_client_callbak(on_dlna_client);   // dlna callback
    bt_emitter.kcx_bt_emitter_callback(on_kcx_bt_emitter);
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_set_vprintf(log_redirect_handler);
    Serial.begin(MONITOR_SPEED);
    vTaskDelay(1000); // wait for serial terminal is ready
    mutex_rtc = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    Serial.print("\n\n");
    trim(Version);

    Serial.println("             " ANSI_ESC_BG_MAGENTA " ***************************************************** " ANSI_ESC_RESET "   ");
    Serial.printf("             " ANSI_ESC_BG_MAGENTA " *     MiniWebRadio % 29s    * " ANSI_ESC_RESET "    \n", Version);
    Serial.println("             " ANSI_ESC_BG_MAGENTA " ***************************************************** " ANSI_ESC_RESET "    ");
    Serial.println(ANSI_ESC_RESET "   ");

    if (!get_esp_items(&s_resetReason, &s_f_FFatFound)) return;
    s_f_ESPfound = true;

    s_cur_AudioFolder = strdup("/audiofiles/");

    pref.begin("Pref", false); // instance of preferences from AccessPoint (SSID, PW ...)

#if TFT_CONTROLLER < 7
    spiBus.begin(TFT_SCK, TFT_MISO, TFT_MOSI, -1); // SPI1 for TFT
    tft.setTFTcontroller(TFT_CONTROLLER);
    tft.setDiaplayInversion(DISPLAY_INVERSION);
    tft.begin(TFT_DC); // Init TFT interface
    setTFTbrightness(100);
    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tft.setBackGoundColor(TFT_BLACK);

    tp.begin(TP_IRQ);
    tp.setVersion(TP_VERSION);
    tp.setRotation(TP_ROTATION);
    tp.setMirror(TP_H_MIRROR, TP_V_MIRROR);
#else
    tp.begin(TP_SDA, TP_SCL, GT911_I2C_ADDRESS, I2C_MASTER_FREQ_HZ, TP_IRQ, -1);
    tp.getProductID();
    tp.setVersion(TP_GT911::GT911);
    tp.setRotation(TP_GT911::Rotate::_0);
    tft.begin(RGB_PINS, RGB_TIMING);
    tft.setDisplayInversion(false);
    vTaskDelay(100 / portTICK_PERIOD_MS); // wait for TFT to be ready
    tft.reset();
#endif

    SerialPrintfln("setup: ...   Init SD card");
    if (IR_PIN >= 0) pinMode(IR_PIN, INPUT_PULLUP); // if ir_pin is read only, have a external resistor (~10...40KOhm)
    pinMode(SD_MMC_D0, INPUT_PULLUP);
#ifdef CONFIG_IDF_TARGET_ESP32S3
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
    int32_t sdmmc_frequency = SDMMC_FREQUENCY / 1000; // MHz -> KHz, default is 40MHz
    if (!SD_MMC.begin("/sdcard", true, false, sdmmc_frequency)) {
        clearAll();
        tft.setFont(displayConfig.fonts[6]);
        tft.setTextColor(TFT_YELLOW);
        tft.writeText("SD Card Mount Failed", 0, 50, displayConfig.dispWidth, displayConfig.dispHeight, TFT_ALIGN_CENTER, TFT_ALIGN_TOP, false, false);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed");
        return;
    }
    float cardSize = ((float)SD_MMC.cardSize()) / (1024 * 1024);
    float freeSize = ((float)SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024);
    SerialPrintfln(ANSI_ESC_WHITE "setup: ....  SD card found, %.1f MB by %.1f MB free", freeSize, cardSize);
    s_f_SD_MMCfound = true;
    defaultsettings();
    if (ESP.getFlashChipSize() > 80000000) { FFat.begin(); }
    if (TFT_BL >= 0) { s_f_brightnessIsChangeable = true; }
#if ESP_IDF_VERSION_MAJOR == 5
    if (TFT_BL >= 0) ledcAttach(TFT_BL, 1200, 8); // 1200 Hz PWM and 8 bit resolution
#endif

    if (TFT_CONTROLLER > 8) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid");

    drawImage("/common/MiniWebRadioV4.jpg", 0, 0); // Welcomescreen
    updateSettings();
    if (s_brightness < 5) s_brightness = 5;
    if (s_volume.volumeSteps < 21) s_volume.volumeSteps = 21;
    setTFTbrightness(s_brightness);

    s_f_WiFiConnected = connectToWiFi();
    if (s_f_WiFiConnected) {
        strcpy(s_myIP, WiFi.localIP().toString().c_str());
        SerialPrintfln("setup: ....  connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", IP address is " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", Received Signal Strength " ANSI_ESC_CYAN
                       "%i" ANSI_ESC_WHITE " dB",
                       WiFi.SSID().c_str(), s_myIP, WiFi.RSSI());

        if (!MDNS.begin("MiniWebRadio")) {
            SerialPrintfln("WiFI_info:   " ANSI_ESC_YELLOW "Error starting mDNS");
        } else
            (SerialPrintfln("WiFI_info:   mDNS started"));
        MDNS.addService("esp32", "tcp", 80);
        SerialPrintfln("WiFI_info:   mDNS name: " ANSI_ESC_CYAN "MiniWebRadio");

        ArduinoOTA.setHostname("MiniWebRadio");
        ArduinoOTA.begin();
        ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); // username, password for ftp.
        setRTC(s_TZString.c_str());
    }

    placingGraphicObjects();
    sdr_BR_value.setValue(s_brightness);
    sdr_EQ_lowPass.setValue(s_toneLP);
    sdr_EQ_bandPass.setValue(s_toneBP);
    sdr_EQ_highPass.setValue(s_toneHP);
    sdr_EQ_balance.setValue(s_toneBAL);
    sdr_DL_volume.setMinMaxVal(0, s_volume.volumeSteps);
    sdr_DL_volume.setValue(s_volume.cur_volume);
    sdr_PL_volume.setMinMaxVal(0, s_volume.volumeSteps);
    sdr_PL_volume.setValue(s_volume.cur_volume);
    sdr_RA_volume.setMinMaxVal(0, s_volume.volumeSteps);
    sdr_RA_volume.setValue(s_volume.cur_volume);
    sdr_CL_volume.setMinMaxVal(0, s_volume.volumeSteps);
    sdr_CL_volume.setValue(s_volume.cur_volume);
    btn_RA_mute.setValue(s_f_mute);
    btn_CL_mute.setValue(s_f_mute);
    btn_EQ_mute.setValue(s_f_mute);
    btn_PL_mute.setValue(s_f_mute);
    btn_DL_mute.setValue(s_f_mute);
    btn_BT_power.setValue(s_bt_emitter.power_state);
    lst_DLNA.client_and_history(&dlna, &s_dlnaHistory[0]);
    lst_RADIO.currentStationNr(&s_cur_station);
    clk_AC_red.alarm_time_and_days(&s_alarmdays, s_alarmtime);

    audio.setAudioTaskCore(AUDIOTASK_CORE);
    audio.setConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);
    audio.setVolumeSteps(s_volume.volumeSteps);
    audio.setVolume(0);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setI2SCommFMT_LSB(I2S_COMM_FMT);

    SerialPrintfln("setup: ....  number of saved stations: " ANSI_ESC_CYAN "%d", staMgnt.getSumStations());
    SerialPrintfln("setup: ....  number of saved favourites: " ANSI_ESC_CYAN "%d", staMgnt.getSumFavStations());
    SerialPrintfln("setup: ....  current station number: " ANSI_ESC_CYAN "%d", s_cur_station);
    SerialPrintfln("setup: ....  current volume: " ANSI_ESC_CYAN "%d", s_volume.cur_volume);
    SerialPrintfln("setup: ....  volume steps: " ANSI_ESC_CYAN "%d", s_volume.volumeSteps);
    SerialPrintfln("setup: ....  volume after alarm: " ANSI_ESC_CYAN "%d", s_volume.volumeAfterAlarm);
    SerialPrintfln("setup: ....  last connected host: " ANSI_ESC_CYAN "%s", s_settings.lastconnectedhost.c_get());
    SerialPrintfln("setup: ....  connection timeout: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT);
    SerialPrintfln("setup: ....  connection timeout SSL: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT_SSL);

    ir.begin(); // Init InfraredDecoder

    if (s_f_WiFiConnected) webSrv.begin(80, 81); // HTTP port, WebSocket port

    if (BT_EMITTER_CONNECT >= 0) {
        pinMode(BT_EMITTER_CONNECT, OUTPUT);
        digitalWrite(BT_EMITTER_CONNECT, HIGH); // POWER_ON
    }

    s_dlnaLevel = 0;
    s_dlnaHistory[0].name = strdup("Media Server");
    s_dlnaHistory[0].objId = strdup("");
    s_dlnaHistory[1].objId = strdup("0");
    s_f_dlnaSeekServer = true;

    if (s_resetReason == ESP_RST_POWERON ||   // Simply switch on the operating voltage
        s_resetReason == ESP_RST_SW ||        // ESP.restart()
        s_resetReason == ESP_RST_SDIO ||      // The boot button was pressed
        s_resetReason == ESP_RST_DEEPSLEEP) { // Wake up
        if (WiFi.isConnected()) {
            if (s_cur_station > 0)
                setStation(s_cur_station);
            else { setStationViaURL(s_settings.lastconnectedhost.c_get(), ""); }
        }
    }

    if (s_f_mute) { SerialPrintfln("setup: ....  volume is muted: (from " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET ")", s_volume.cur_volume); }
    setI2STone();

    if (I2C_SCL != -1) {
        s_f_BH1750_found = BH1750.begin(BH1750.ADDR_TO_GROUND, I2C_SDA, I2C_SCL); // init the sensor with address pin connetcted to ground
    }
    if (s_f_BH1750_found) { // result (bool) wil be be "false" if no sensor found
        SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "Ambient Light Sensor BH1750 found");
        BH1750.setResolutionMode(BH1750.ONE_TIME_H_RESOLUTION_MODE);
        BH1750.setSensitivity(BH1750.SENSITIVITY_ADJ_MAX);
    }
    ticker100ms.attach(0.1, timer100ms);
#if TFT_CONTROLLER == 7
    tft.clearVsyncCounter(); // clear the vsync counter and start them
#endif

    tft.fillScreen(TFT_BLACK);                                                          // Clear screen
    drawImage("/common/Wallpaper.jpg", 0, 0);                                           // Wallpaper
    tft.copyFramebuffer(0, 1, 0, 0, displayConfig.dispWidth, displayConfig.dispHeight); // copy wallpaper to background
    muteChanged(s_f_mute);
    dispFooter.setIpAddr(WiFi.localIP().toString().c_str());
    dispFooter.updateStation(s_cur_station);
    dispFooter.updateOffTime(s_sleeptime);
    dispFooter.show(true);

    dispHeader.updateItem(_hl_item[RADIO]);
    dispHeader.updateVolume(s_volume.cur_volume);
    dispHeader.speakerOnOff(!s_f_mute);
    dispHeader.show(true);
    s_radioSubMenue = 0;
    s_state = NONE;
    if (s_f_WiFiConnected) {
        changeState(RADIO);
    } else {
        changeState(WIFI_SETTINGS);
    }

    bt_emitter.begin();
    bt_emitter.userCommand("AT+GMR?"); // get version
    bt_emitter.userCommand("AT+NAME+BT-MiniWebRadio");
    bt_emitter.setVolume(s_bt_emitter.volume);
    bt_emitter.setMode(s_bt_emitter.mode);
}
/*****************************************************************************************************************************************************
 *                                                                   C O M M O N                                                                     *
 *****************************************************************************************************************************************************/

const char* scaleImage(const char* path) {
    if ((!endsWith(path, "bmp")) && (!endsWith(path, "jpg")) && (!endsWith(path, "gif") && (!endsWith(path, "png")))) { // not a image
        return path;
    }
    if (startsWith(path, "/png")) return path; // nothing to do, icons for website
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    int idx = indexOf(path, "/", 1);
    if (idx < 0)
        return path; // invalid path
    else {
        char prefix[03] = " /";
        prefix[0] = displayConfig.tftSize;
        strncpy(pathBuff, path, idx + 1);
        strcat(pathBuff, prefix);
        strcat(pathBuff, path + idx + 1);
    }
    return pathBuff;
}

void setVolume(uint8_t vol) {
    static int16_t oldVol = -1;
    if (vol == oldVol) return;
    log_i("volume old: %i. new: %i", oldVol, vol);
    s_volume.cur_volume = vol;
    oldVol = vol;
    dispHeader.updateVolume(s_volume.cur_volume);
    sdr_CL_volume.setValue(s_volume.cur_volume);
    sdr_DL_volume.setValue(s_volume.cur_volume);
    sdr_PL_volume.setValue(s_volume.cur_volume);
    sdr_RA_volume.setValue(s_volume.cur_volume);
    SerialPrintfln("action: ...  current volume is " ANSI_ESC_CYAN "%d", s_volume.cur_volume);
}

uint8_t downvolume() {
    uint8_t steps = s_volume.volumeSteps / 20;
    if (s_volume.cur_volume == 0)
        return s_volume.cur_volume;
    else if (steps < s_volume.cur_volume)
        s_volume.cur_volume -= steps;
    else
        s_volume.cur_volume--;
    sdr_CL_volume.setValue(s_volume.cur_volume);
    sdr_DL_volume.setValue(s_volume.cur_volume);
    sdr_PL_volume.setValue(s_volume.cur_volume);
    sdr_RA_volume.setValue(s_volume.cur_volume);
    s_f_mute = false;
    muteChanged(s_f_mute); // set mute off
    return s_volume.cur_volume;
}

uint8_t upvolume() {
    uint8_t steps = s_volume.volumeSteps / 20;
    if (s_volume.cur_volume == s_volume.volumeSteps)
        return s_volume.cur_volume;
    else if (s_volume.volumeSteps > s_volume.cur_volume + steps)
        s_volume.cur_volume += steps;
    else
        s_volume.cur_volume++;
    sdr_CL_volume.setValue(s_volume.cur_volume);
    sdr_DL_volume.setValue(s_volume.cur_volume);
    sdr_PL_volume.setValue(s_volume.cur_volume);
    sdr_RA_volume.setValue(s_volume.cur_volume);
    s_f_mute = false;
    muteChanged(s_f_mute); // set mute off
    return s_volume.cur_volume;
}

void setStation(uint16_t sta) {
    static uint16_t old_cur_station = 0;
    if (sta == 0) { return; }
    if (sta > staMgnt.getSumStations()) sta = s_cur_station;
    x_ps_free(&s_stationURL);
    s_stationURL = x_ps_strdup(staMgnt.getStationUrl(sta));
    s_homepage = "";
    SerialPrintfln("action: ...  switch to station " ANSI_ESC_CYAN "%d", sta);

    if (s_f_isWebConnected && sta == old_cur_station && s_state == RADIO) { // Station is already selected
        s_f_newStreamTitle = true;
    } else {
        if (s_state != RADIO) {
            s_radioSubMenue = 0;
            changeState(RADIO);
        }
        s_streamTitle[0] = '\0';
        s_icyDescription[0] = '\0';
        s_f_newStreamTitle = true;
        s_f_newIcyDescription = true;
        connecttohost(s_stationURL);
        //    if(!s_f_isWebConnected) s_cur_station = old_cur_station; // host is not connected
    }
    old_cur_station = sta;
    StationsItems();
    if (s_state == RADIO) {
        showLogoAndStationName(true);
        if (s_cur_station == 0) {
            dispFooter.updateFlag(NULL);
        } else {
            dispFooter.updateFlag(getFlagPath(s_cur_station));
        }
    }
    dispFooter.updateStation(s_cur_station);
}
const char* getFlagPath(uint16_t station) {
    static char flagPath[40];
    flagPath[0] = '\0';
    strcpy(flagPath, "/flags/");
    strcat(flagPath, staMgnt.getStationCountry(station));
    for (int i = 0; i < strlen(flagPath); i++) flagPath[i] = tolower(flagPath[i]);
    strcat(flagPath, ".jpg");
    return flagPath;
}

void nextStation() {
    setStation(staMgnt.nextStation());
}
void nextFavStation() {
    setStation(staMgnt.nextFavStation());
}
void prevStation() {
    setStation(staMgnt.prevStation());
}
void prevFavStation() {
    setStation(staMgnt.prevFavStation());
}
void setStationByNumber(uint16_t staNr) {
    setStation(staMgnt.setStationByNumber(staNr));
}

void StationsItems() {
    if (!s_stationName_air) s_stationName_air = (char*)calloc(1, 1);
    char* stationLogo_air = x_ps_malloc(strlen(s_stationName_air) + 15);
    strcpy(stationLogo_air, "/logo/");
    strcat(stationLogo_air, s_stationName_air);
    strcat(stationLogo_air, ".jpg");
    char staNr[10];
    itoa(s_cur_station, staNr, 10);

    if (s_cur_station == 0) {
        webSrv.send("stationLogo=", stationLogo_air);
        webSrv.send("stationNr=", staNr);
        webSrv.send("stationURL=", s_settings.lastconnectedhost.get());
    } else {
        webSrv.send("stationLogo=", "/logo/" + String(staMgnt.getStationName(s_cur_station)) + ".jpg");
        webSrv.send("stationNr=", staNr);
        if (s_stationURL) webSrv.send("stationURL=", String(s_stationURL));
    }
    x_ps_free(&stationLogo_air);
}

void setStationViaURL(const char* url, const char* extension) {
    // e.g.  http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one&bitrate=96000
    // url is http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one, extension is bitrate=96000
    x_ps_free(&s_stationName_air);
    s_cur_station = 0;
    x_ps_free(&s_stationURL);
    int   len_url = strlen(url) + strlen(extension) + 3;
    char* origin_url = (char*)calloc(1, len_url);
    if (strlen(extension) > 0) {
        strcpy(origin_url, url);
        strcat(origin_url, "&");
        strcat(origin_url, extension);
    } else {
        strcpy(origin_url, url);
    }
    s_stationURL = x_ps_strdup(origin_url);
    connecttohost(origin_url);
    StationsItems();
    if (s_state == RADIO) {
        clearStreamTitle();
        showLogoAndStationName(true);
        dispFooter.updateFlag(NULL);
    }
    dispFooter.updateStation(0); // set 000
}

void savefile(const char* fileName, uint32_t contentLength) { // save the uploadfile on SD_MMC
    char fn[256];
    s_f_SD_Upload = false;
    if (!startsWith(fileName, "/")) {
        strcpy(fn, "/");
        strcat(fn, fileName);
    } else {
        strcpy(fn, fileName);
    }
    if (webSrv.uploadfile(SD_MMC, fn, contentLength)) {
        SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
        webSrv.sendStatus(200);
    } else {
        SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD failed", fn);
        webSrv.sendStatus(400);
    }
}

void saveImage(const char* fileName, uint32_t contentLength) { // save the jpg image on SD_MMC
    char fn[256];

    if (!s_f_SD_Upload && endsWith(fileName, "jpg")) {
        strcpy(fn, "/logo/");
        char prefix[03] = " /";
        prefix[0] = displayConfig.tftSize;
        strcat(fn, prefix);
        if (!startsWith(fileName, "/")) strcat(fn, "/");
        strcat(fn, fileName);
        if (webSrv.uploadB64image(SD_MMC, fn, contentLength)) {
            SerialPrintfln("save image (jpg) " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.sendStatus(200);
        } else
            webSrv.sendStatus(400);
    }
}

String setI2STone() {
    int8_t LP = s_toneLP;
    int8_t BP = s_toneBP;
    int8_t HP = s_toneHP;
    int8_t BAL = s_toneBAL;
    audio.setTone(LP, BP, HP);
    audio.setBalance(BAL);
    sprintf(s_chbuf, "LowPass=%i\nBandPass=%i\nHighPass=%i\nBalance=%i\n", LP, BP, HP, BAL);
    String tone = String(s_chbuf);
    return tone;
}

void SD_playFile(const char* pathWoFileName, const char* fileName) { // pathWithoutFileName e.g. /audiofiles/playlist/
    sprintf(s_chbuf, "%s%s", pathWoFileName, fileName);
    int32_t idx = indexOf(s_chbuf, "\033[", 1);
    if (idx == -1) {
        ;
    } // do nothing
    else {
        s_chbuf[idx] = '\0';
    } // remove color and filesize
    SD_playFile(s_chbuf, 0, true);
}

void SD_playFile(const char* path, uint32_t fileStartTime, bool showFN) {
    if (!path) return;                            // avoid a possible crash
    if (endsWith(path, "ogg")) fileStartTime = 0; // resume only mp3, m4a, flac and wav
    if (endsWith(path, "m3u")) {
        if (SD_MMC.exists(path)) {
            preparePlaylistFromFile(path);
            processPlaylist(true);
        }
        return;
    }
    if (s_playerSubMenue != 1 && s_playerSubMenue != 3) {
        s_playerSubMenue = 1;
        changeState(PLAYER);
    }
    int32_t idx = lastIndexOf(path, '/');
    if (idx < 0) return;
    x_ps_free(&s_cur_AudioFolder);
    s_cur_AudioFolder = strdup(path);
    s_cur_AudioFolder[idx] = '\0';

    if (showFN) {
        clearLogo();
        showFileName(path + idx + 1);
    }

    SerialPrintfln("AUDIO_FILE:  " ANSI_ESC_MAGENTA "%s", path + idx + 1);
    connecttoFS("SD_MMC", (const char*)path, fileStartTime);
    if (s_f_playlistEnabled) showPlsFileNumber();
    if (s_f_isFSConnected) { s_settings.lastconnectedfile.copy_from(path); }
}

bool SD_rename(const char* src, const char* dest) {
    bool success = false;
    if (SD_MMC.exists(src)) {
        log_i("exists");
        success = SD_MMC.rename(src, dest);
    }
    return success;
}

bool SD_newFolder(const char* folderPathName) {
    bool success = false;
    success = SD_MMC.mkdir(folderPathName);
    return success;
}

bool SD_delete(const char* itemPath) {
    bool success = false;
    if (SD_MMC.exists(itemPath)) {
        File dirTest = SD_MMC.open(itemPath, "r");
        bool isDir = dirTest.isDirectory();
        dirTest.close();
        if (isDir)
            success = SD_MMC.rmdir(itemPath);
        else
            success = SD_MMC.remove(itemPath);
    }
    return success;
}

void fall_asleep() {
    s_f_sleeping = true;
    s_f_mute = true;
    s_f_playlistEnabled = false;
    s_f_isFSConnected = false;
    s_f_isWebConnected = false;
    audio.stopSong();
    if (s_sleepMode == 0) {
        clearAll();
        setTFTbrightness(0);
    } else {
        s_clockSubMenue = 0;
        changeState(CLOCK);
    }
    if (s_bt_emitter.power_state) bt_emitter.power_off();
    SerialPrintfln("falling asleep");
    dispHeader.hide();
    dispFooter.hide();
}

void wake_up() {
    s_f_sleeping = false;
    s_f_mute = false;
    s_f_irOnOff = false;
    SerialPrintfln("awake");
    s_f_mute = true;
    muteChanged(false);
    clearAll();
    setTFTbrightness(s_brightness);
    if (s_cur_station)
        setStation(s_cur_station);
    else
        connecttohost(s_settings.lastconnectedhost.get());
    s_radioSubMenue = 0;
    changeState(RADIO);
    showLogoAndStationName(true);
    dispHeader.show(true);
    dispHeader.speakerOnOff(!s_f_mute);
    dispHeader.updateRSSI(WiFi.RSSI(), true);
    dispFooter.show(true);
    if (!s_bt_emitter.power_state) bt_emitter.power_on();
}

void setRTC(const char* TZString) {
    rtc.stop();
    rtc.begin(s_TZString.c_str());
    // if(!s_f_rtc) {
    //     SerialPrintfln(ANSI_ESC_RED "connection to NTP failed, trying again");
    //     ESP.restart();
    // }
}

boolean isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime) {
    uint8_t mask = 0b00000001 << weekDay;
    if (alarmDays & mask) {                         // yes, is alarmDay
        if (alarmTime[weekDay] == minuteOfTheDay) { // yes, is alarmTime
            return true;
        }
    }
    return false;
}

boolean copySDtoFFat(const char* path) {
    if (!s_f_FFatFound) return false;
    uint8_t buffer[1024];
    size_t  r = 0, w = 0;
    size_t  len = 0;
    File    file1 = SD_MMC.open(path, "r");
    File    file2 = FFat.open(path, "w");
    while (true) {
        r = file1.read(buffer, 1024);
        w = file2.write(buffer, r);
        if (r != w) {
            file1.close();
            file2.close();
            FFat.remove(path);
            return false;
        }
        len += r;
        if (r == 0) break;
    }
    log_i("file length %i, written %i", file1.size(), len);
    if (file1.size() == len) return true;
    return false;
}

void muteChanged(bool m) {
    s_f_muteIsPressed = false;
    btn_CL_mute.setValue(m);
    btn_DL_mute.setValue(m);
    btn_EQ_mute.setValue(m);
    btn_PL_mute.setValue(m);
    btn_RA_mute.setValue(m);
    if (m) {
        s_f_mute = true;
    } else {
        s_f_mute = false;
    }
    if (m)
        webSrv.send("mute=", "1");
    else
        webSrv.send("mute=", "0");
    dispHeader.speakerOnOff(!s_f_mute);
    dispHeader.updateVolume(s_volume.cur_volume);
    updateSettings();
};

void logAlarmItems() {
    const char wd[7][11] = {"Sunday:   ", "Monday:   ", "Tuesday:  ", "Wednesday:", "Thursday: ", "Friday:   ", "Saturday: "};
    uint8_t    mask = 0b00000001;
    for (uint8_t i = 0; i < 7; i++) {
        if (s_alarmdays & mask) {
            SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s " ANSI_ESC_CYAN "%02i:%02i", wd[i], s_alarmtime[i] / 60, s_alarmtime[i] % 60);
        } else {
            SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s No alarm is set", wd[i]);
        }
        mask <<= 1;
    }
}

void setTimeCounter(uint8_t sec) {
    if (sec) {
        s_timeCounter.timer = 10;
        s_timeCounter.factor = sec;
    } else {
        s_timeCounter.timer = 0;
        s_timeCounter.factor = 0;
        dispFooter.updateTC(0);
    }
}

// clang-format off

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                              C H A N G E    S T A T E                                                                       ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format on
void changeState(int32_t state) {
    disableAllObjects();
    s_f_volBarVisible = false;
    if (s_timeCounter.timer) { setTimeCounter(0); }
    dispFooter.enable();
    dispHeader.enable();
    dispHeader.updateItem(_hl_item[state]);
    if (state != RADIO) { dispFooter.updateFlag(NULL); }
    switch (state) {
        case RADIO: {
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != RADIO) {
                clearWithOutHeaderFooter();
                dispFooter.updateFlag(getFlagPath(s_cur_station));
            }
            txt_RA_staName.enable();
            pic_RA_logo.enable();
            if (s_radioSubMenue == 0) {
                if (s_f_irResultSeen) {
                    txt_RA_irNum.hide();
                    setStationByNumber(s_irResult);
                    s_f_irResultSeen = false;
                } // ir_number, valid between 1 ... 999
                if (s_state != RADIO) { showLogoAndStationName(true); }
                setTimeCounter(0);
                VUmeter_RA.show(true);
                txt_RA_sTitle.setText("");
                txt_RA_sTitle.show(true, false);
                s_f_newStreamTitle = true;
            }
            if (s_radioSubMenue == 1) { // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                clearTitle();
                txt_RA_sTitle.disable();
                sdr_RA_volume.show(true, false);
                btn_RA_mute.show();
                btn_RA_prevSta.show();
                btn_RA_nextSta.show();
                //    txt_RA_staName.show(true, false);
                setTimeCounter(2);
            }
            if (s_radioSubMenue == 2) { // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                btn_RA_staList.show();
                btn_RA_player.show();
                btn_RA_dlna.show();
                btn_RA_clock.show();
                btn_RA_sleep.show();
                btn_RA_settings.show();
                btn_RA_bt.show(!s_bt_emitter.found);
                btn_RA_off.show();
                setTimeCounter(2);
            }
            if (s_radioSubMenue == 3) { // show Numbers from IR
                char buf[10];
                itoa(s_irNumber, buf, 10);
                txt_RA_irNum.setText(buf);
                txt_RA_irNum.show(true, false);
                setTimeCounter(1);
            }
            if (s_radioSubMenue == 4) { // IR select mode
                clearTitle();
                txt_RA_sTitle.disable();
                btn_RA_staList.showAlternativePic();
                btn_RA_player.show();
                btn_RA_dlna.show();
                btn_RA_clock.show();
                btn_RA_sleep.show();
                btn_RA_settings.show();
                btn_RA_bt.show(!s_bt_emitter.found);
                btn_RA_off.show();
                setTimeCounter(2);
            }
            if (s_state != RADIO) webSrv.send("changeState=", "RADIO");
            break;
        }

        case STATIONSLIST: {
            dispHeader.show(false);
            dispFooter.show(false);
            clearWithOutHeaderFooter();
            lst_RADIO.show();
            setTimeCounter(LIST_TIMER);
            break;
        }

        case PLAYER: {
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != PLAYER) clearWithOutHeaderFooter();
            pic_PL_logo.enable();
            if (s_playerSubMenue == 0) { // prev, next, ready, play_all, shuffle, list, radio, off
                if (!s_cur_AudioFolder) {
                    x_ps_free(&s_cur_AudioFolder);
                    s_cur_AudioFolder = strdup("/audiofiles/");
                }
                s_SD_content.listFilesInDir(s_cur_AudioFolder, true, false);
                s_cur_Codec = 0;
                showFileLogo(PLAYER);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                txt_PL_fName.show(true, false);
                pgb_PL_progress.hide();
                sdr_PL_volume.disable();
                if (s_state != PLAYER) webSrv.send("changeState=", "PLAYER");
                txt_PL_fNumber.show(true, false);
                showAudioFileNumber();
                btn_PL_prevFile.show();
                btn_PL_nextFile.show();
                btn_PL_ready.show();
                btn_PL_playAll.show();
                btn_PL_shuffle.show();
                btn_PL_fileList.show();
                btn_PL_radio.show();
                btn_PL_off.show();
            }
            if (s_playerSubMenue == 1) {
                btn_PL_fileList.hide();
                btn_PL_radio.hide();
                btn_PL_off.hide();
                pgb_PL_progress.setValue(0);
                pgb_PL_progress.show(true, false);
                sdr_PL_volume.show(true, false);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                txt_PL_fName.show(true, false);
                txt_PL_fNumber.show(true, false);
                btn_PL_mute.show();
                btn_PL_pause.setOff();
                btn_PL_pause.show();
                btn_PL_cancel.show();
                btn_PL_playPrev.show();
                btn_PL_playNext.show();
            }
            if (s_playerSubMenue == 2) { // same as submenue 0 for IR
                if (!s_cur_AudioFolder) {
                    s_cur_AudioFolder = strdup("/audiofiles/");
                    s_cur_AudioFileNr = 0;
                }
                s_SD_content.listFilesInDir(s_cur_AudioFolder, true, false);
                s_cur_Codec = 0;
                showFileLogo(PLAYER);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                txt_PL_fName.show(true, false);
                pgb_PL_progress.hide();
                if (s_state != PLAYER) webSrv.send("changeState=", "PLAYER");
                txt_PL_fNumber.show(true, false);
                showAudioFileNumber();
                btn_PL_prevFile.showAlternativePic();
                btn_PL_nextFile.show();
                btn_PL_ready.show();
                btn_PL_playAll.show();
                btn_PL_shuffle.show();
                btn_PL_fileList.show();
                btn_PL_radio.show();
                btn_PL_off.show();
            }
            if (s_playerSubMenue == 3) { // same as submenue 1 for IR
                btn_PL_fileList.hide();
                btn_PL_radio.hide();
                btn_PL_off.hide();
                pgb_PL_progress.setValue(0);
                pgb_PL_progress.show(true, false);
                sdr_PL_volume.show(true, false);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                txt_PL_fName.show(true, false);
                txt_PL_fNumber.show(true, false);
                btn_PL_mute.showAlternativePic();
                btn_PL_pause.setOff();
                btn_PL_pause.show();
                btn_PL_cancel.show();
                btn_PL_playPrev.show();
                btn_PL_playNext.show();
            }
            break;
        }
        case AUDIOFILESLIST: {
            dispHeader.show(false);
            dispFooter.show(false);
            clearWithOutHeaderFooter();
            lst_PLAYER.show(s_cur_AudioFolder, s_cur_AudioFileNr);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case DLNA: {
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != DLNA) {
                if (s_state != DLNAITEMSLIST) audio.stopSong();
                clearWithOutHeaderFooter();
                pic_DL_logo.enable();
                pgb_DL_progress.setValue(0);
                pgb_DL_progress.show(true, false);
                txt_DL_fName.show(true, false);
                showFileLogo(DLNA);
            }
            if (s_dlnaSubMenue == 0) { // TP submenu
                if (audio.isRunning())
                    btn_DL_pause.setActive(true);
                else
                    btn_DL_pause.setActive(false);
                btn_DL_pause.show();
                sdr_DL_volume.show(true, false);
                btn_DL_mute.show();
                btn_DL_cancel.show();
                btn_DL_fileList.show();
                btn_DL_radio.show();
            }
            if (s_dlnaSubMenue == 1) { // IR submenu
                if (audio.isRunning())
                    btn_DL_pause.setActive(true);
                else
                    btn_DL_pause.setActive(false);
                btn_DL_pause.show();
                sdr_DL_volume.show(true, false);
                btn_DL_mute.showAlternativePic();
                btn_DL_cancel.show();
                btn_DL_fileList.show();
                btn_DL_radio.show();
            }
            if (s_state != DLNA) webSrv.send("changeState=", "DLNA");
            break;
        }
        case DLNAITEMSLIST: {
            dispHeader.show(false);
            dispFooter.show(false);
            lst_DLNA.show(s_currDLNAsrvNr, dlna.getServer(), dlna.getBrowseResult(), &s_dlnaLevel, s_dlnaMaxItems);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case CLOCK: {
            dispHeader.show(false);
            dispFooter.show(false);
            if (s_state != CLOCK) { bgColorWithOutHeaderFooter(); }
            if (s_timeFormat == 24)
                clk_CL_24.show();
            else
                clk_CL_12.show();
            if (s_clockSubMenue == 0) {
                btn_CL_mute.hide();
                btn_CL_alarm.hide();
                btn_CL_radio.hide();
                sdr_CL_volume.disable();
                btn_CL_off.hide();
            }
            if (s_clockSubMenue == 1) {
                setTimeCounter(2);
                btn_CL_mute.show();
                btn_CL_alarm.show();
                btn_CL_radio.show();
                sdr_CL_volume.show(false, false);
                btn_CL_off.show();
            }
            if (s_clockSubMenue == 2) { // same as s_clockSubMenue 1 for IR
                setTimeCounter(2);
                btn_CL_alarm.showAlternativePic();
                btn_CL_radio.show();
                btn_CL_mute.show();
                btn_CL_off.show();
                sdr_CL_volume.show(false, false);
            }
            break;
        }
        case ALARMCLOCK: {
            dispHeader.show(false);
            dispFooter.show(false);
            if (s_state != ALARMCLOCK) bgColorWithOutHeaderFooter();
            if (s_alarmSubMenue == 0) {
                btn_AC_left.show();
                btn_AC_right.show();
                btn_AC_up.show();
                btn_AC_down.show();
                btn_AC_ready.show();
                clk_AC_red.show();
            }
            if (s_alarmSubMenue == 1) { // same as s_alarmSubMenue for IR
                setTimeCounter(2);
                btn_AC_left.showAlternativePic();
                btn_AC_right.show();
                btn_AC_up.show();
                btn_AC_down.show();
                btn_AC_ready.show();
                clk_AC_red.show();
            }
            break;
        }
        case SLEEPTIMER: {
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != SLEEPTIMER) {
                clearWithOutHeaderFooter();
                display_sleeptime();
                if (TFT_CONTROLLER < 2)
                    drawImage("/common/Night_Gown.bmp", 198, 23);
                else if (TFT_CONTROLLER < 7)
                    drawImage("/common/Night_Gown.bmp", 280, 45);
                else
                    drawImage("/common/Night_Gown.jpg", 482, 112);
            }
            if (s_sleepTimerSubMenue == 0) {
                btn_SL_up.show();
                btn_SL_down.show();
                btn_SL_ready.show();
                btn_SL_cancel.show();
            }
            if (s_sleepTimerSubMenue == 1) {
                btn_SL_up.showAlternativePic();
                btn_SL_down.show();
                btn_SL_ready.show();
                btn_SL_cancel.show();
            }
            break;
        }
        case SETTINGS: {
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != SETTINGS) {
                clearWithOutHeaderFooter();
                showFileLogo(SETTINGS);
            }
            if (s_settingsSubMenue == 0) {
                btn_SE_bright.show(!s_f_brightnessIsChangeable);
                btn_SE_equal.show();
                btn_SE_wifi.show();
                btn_SE_radio.show();
            }
            if (s_settingsSubMenue == 1) {
                btn_SE_bright.showAlternativePic(!s_f_brightnessIsChangeable);
                btn_SE_equal.show();
                btn_SE_wifi.show();
                btn_SE_radio.show();
            }
            break;
        }
        case BRIGHTNESS: {
            dispHeader.show(false);
            dispFooter.show(false);
            if (s_state != BRIGHTNESS) {
                clearWithOutHeaderFooter();
                pic_BR_logo.show(true, false);
            }
            sdr_BR_value.show(true, true);
            txt_BR_value.setText(int2str(s_brightness));
            txt_BR_value.show(true, true);
            if (s_brightnessSubMenue == 0) { btn_BR_ready.show(); }
            if (s_brightnessSubMenue == 1) { // same as s_brightnessSubMenue for IR
                btn_BR_ready.showAlternativePic();
            }
            break;
        }
        case EQUALIZER:
            dispHeader.show(true);
            dispFooter.show(true);
            if (s_state != EQUALIZER) clearWithOutHeaderFooter();
            sdr_EQ_lowPass.show(true, false);
            sdr_EQ_bandPass.show(true, false);
            sdr_EQ_highPass.show(true, false);
            sdr_EQ_balance.show(true, false);
            btn_EQ_lowPass.show();
            btn_EQ_bandPass.show();
            btn_EQ_highPass.show();
            btn_EQ_balance.show();
            btn_EQ_Player.show();
            btn_EQ_mute.show();
            txt_EQ_lowPass.show(true, false);
            txt_EQ_bandPass.show(true, false);
            txt_EQ_highPass.show(true, false);
            txt_EQ_balance.show(true, false);
            if (s_equalizerSubMenue == 0) { btn_EQ_Radio.show(); }
            if (s_equalizerSubMenue == 1) { btn_EQ_Radio.showAlternativePic(); }

            break;

        case BLUETOOTH: {
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            btn_BT_volUp.show();
            btn_BT_volDown.show();
            btn_BT_pause.show();
            btn_BT_mode.show();
            btn_BT_radio.show();
            btn_BT_power.show();
            pic_BT_mode.show(true, false);
            if (s_bt_emitter.mode.equals("RX")) {
                txt_BT_mode.writeText("RECEIVER");
            } else {
                txt_BT_mode.writeText("EMITTER");
            }
            txt_BT_mode.setBGcolor(TFT_BROWN);
            txt_BT_mode.show(true, false);
            char c[10];
            sprintf(c, "Vol: %02i", bt_emitter.getVolume());
            txt_BT_volume.writeText(c);
            txt_BT_volume.show(true, false);
            if (s_state != BLUETOOTH) webSrv.send("changeState=", "BLUETOOTH");
            break;
        }
        case IR_SETTINGS:
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            btn_IR_radio.show();
            break;

        case RINGING:
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            if (s_volume.ringVolume > 0) { // alarm with bell
                pic_RI_logo.enable();
                showFileLogo(RINGING);
                setTFTbrightness(s_brightness);
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm");
                setVolume(s_volume.ringVolume);
                audio.setVolume(s_volume.ringVolume, s_volume.volumeCurve);
                muteChanged(false);
                connecttoFS("SD_MMC", "/ring/alarm_clock.mp3");
                clk_RI_24small.show();
            } else { // alarm without bell
                s_f_eof_alarm = true;
            }
            break;

        case WIFI_SETTINGS:
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            cls_wifiSettings.clearText();
            cls_wifiSettings.setBorderWidth(1);
            cls_wifiSettings.setFontSize(displayConfig.listFontSize);
            int16_t n = WiFi.scanNetworks();
            SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "%i WiFi networks found", n);
            for (int i = 0; i < n; i++) {
                SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)", WiFi.SSID(i).c_str(), (int16_t)WiFi.RSSI(i));
                const char* pw = getWiFiPW(WiFi.SSID(i).c_str());
                // if(pw){log_e("found password %s for %s", pw, WiFi.SSID(i).c_str());}
                // else  {log_e("no password found for %s", WiFi.SSID(i).c_str());}
                cls_wifiSettings.addWiFiItems(WiFi.SSID(i).c_str(), pw);
            }
            cls_wifiSettings.show(false, false);
            break;
    }
    s_state = state;
}
// clang-format on

const char* getWiFiPW(const char* ssid) {
    static char* line = NULL;
    if (line) x_ps_free(&line);
    line = (char*)x_ps_malloc(128);

    static char* password = NULL;
    if (line) x_ps_free(&password);
    password = (char*)x_ps_malloc(128);
    if (!password) {
        log_e("oom");
        return NULL;
    }

    for (int j = 0; j < 6; j++) {
        if (j == 0) strcpy(line, pref.getString("wifiStr0").c_str());
        if (j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if (j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if (j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if (j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if (j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if (startsWith(line, ssid) && line[strlen(ssid)] == '\t') {
            if (indexOf(line, "\t", 0) > 0) {
                int idx = indexOf(line, "\t", 0);
                strcpy(password, line + idx + 1);
                // SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "SSID '%s', PW '%s'", ssid, password);
                return password;
            } else {
                // SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "SSID '%s', PW '%s'", ssid, "is not set");
                return NULL;
            }
        }
    }
    return NULL;
}

bool setWiFiPW(const char* ssid, const char* password) {
    if (!ssid || !password) return NULL;
    static char* line = NULL;
    if (line) x_ps_free(&line);
    line = (char*)x_ps_malloc(512);
    if (!line) {
        log_e("oom");
        return NULL;
    }
    strcpy(line, ssid);
    strcat(line, "\t");
    strcat(line, password);

    for (int j = 1; j < 6; j++) {
        if (j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if (j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if (j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if (j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if (j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if (startsWith(line, ssid) && line[strlen(ssid)] == '\t') {
            switch (j) {
                case 1: pref.putString("wifiStr1", line); break;
                case 2: pref.putString("wifiStr2", line); break;
                case 3: pref.putString("wifiStr3", line); break;
                case 4: pref.putString("wifiStr4", line); break;
                case 5: pref.putString("wifiStr5", line); break;
            }
            SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "save new PW for SSID %s,  %s", ssid, line);
            return true;
        }
    }
    for (int j = 0; j < 6; j++) {
        if (j == 0) strcpy(line, pref.getString("wifiStr0").c_str());
        if (j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if (j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if (j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if (j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if (j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if (strcmp(line, "\t") == 0) {
            switch (j) {
                case 0: pref.putString("wifiStr0", line); break;
                case 1: pref.putString("wifiStr1", line); break;
                case 2: pref.putString("wifiStr2", line); break;
                case 3: pref.putString("wifiStr3", line); break;
                case 4: pref.putString("wifiStr4", line); break;
                case 5: pref.putString("wifiStr5", line); break;
            }
            SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "save new SSID and PW %s,  %s", ssid, line);
            return true;
        }
    }
    SerialPrintfln("WiFi: .....  " ANSI_ESC_RED "no free space for new SSID and PW %s,  %s", ssid, line);
    x_ps_free(&line);
    return false;
}

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                    L O O P                                                                                  ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

void loop() {
    if (!s_f_ESPfound) {
        vTaskDelay(10);
        return;
    } // Guard:  wrong chip?
    if (!s_f_SD_MMCfound) {
        vTaskDelay(10);
        return;
    } // Guard:  SD_MMC could not be initialisized

    vTaskDelay(1);
    audio.loop();
    webSrv.loop();
    ir.loop();
    tp.loop();
    ftpSrv.handleFTP();
    ArduinoOTA.handle();
    dlna.loop();
    bt_emitter.loop();
    tft.loop();

    while (s_logBuffer.size() > 0) {
        size_t i = s_logBuffer.size();
        webSrv.send("serTerminal=", s_logBuffer[i - 1].c_get());
        s_logBuffer.pop_back();
        if (s_logBuffer.size() == 0) s_logBuffer.clear(); // Löscht alle Elemente und gibt den Speicher frei
    }

    if (s_f_dlnaBrowseServer) {
        s_f_dlnaBrowseServer = false;
        dlna.browseServer(s_currDLNAsrvNr, s_dlnaHistory[s_dlnaLevel].objId, s_totalNumberReturned);
    }
    if (s_f_clearLogo) {
        s_f_clearLogo = false;
        clearLogo();
    }
    if (s_f_clearStationName) {
        s_f_clearStationName = false;
        clearStationName();
    }

    if (s_f_playlistEnabled) {
        if (!s_f_playlistNextFile) {
            if (!audio.isRunning() && !s_f_pauseResume) {
                SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "next playlist file");
                processPlaylist(false);
                s_playlistTime = millis();
                s_f_playlistNextFile = true;
            }
        } else {
            if (s_playlistTime + 5000 < millis()) s_f_playlistNextFile = false;
        }
    }
    //-----------------------------------------------------0.1 SEC------------------------------------------------------------------------------------
    if (s_f_100ms) { // calls every 0.1 second
        s_f_100ms = false;

        if (s_state == RADIO && s_radioSubMenue == 0) VUmeter_RA.update(audio.getVUlevel());

        static uint8_t factor = 0;
        static bool    f_tc = false;
        if (factor > 0) {
            factor--;
        } else {
            if (s_timeCounter.timer > 0) {
                factor = s_timeCounter.factor;
                dispFooter.updateTC(s_timeCounter.timer);
                s_timeCounter.timer--;
                f_tc = true;
            } else {
                if (f_tc) {
                    f_tc = false;
                    dispFooter.updateTC(0);
                    if (s_f_sleeping) return; // tc is active by pressing a button, but do nothing if "off"

                    if (s_state == RADIO) {
                        if (!txt_RA_staName.isEnabled()) { txt_RA_staName.show(true, false); } // assume volBox is shown
                        if (s_radioSubMenue == 1) {
                            s_radioSubMenue = 0;
                            changeState(RADIO);
                        } // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                        if (s_radioSubMenue == 2) {
                            s_radioSubMenue = 0;
                            changeState(RADIO);
                        } // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                        if (s_radioSubMenue == 3) {
                            s_radioSubMenue = 0;
                            changeState(RADIO);
                        } // show Numbers from IR
                        if (s_radioSubMenue == 4) {
                            s_radioSubMenue = 0;
                            changeState(RADIO);
                        } // IR select mode
                    } else if (s_state == STATIONSLIST) {
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                    } else if (s_state == PLAYER) {
                        if (!txt_PL_fName.isEnabled()) { txt_PL_fName.show(true, false); } // assume volBox is shown
                        if (s_playerSubMenue == 2) {
                            s_playerSubMenue = 0;
                            changeState(PLAYER);
                        }
                        if (s_playerSubMenue == 3) {
                            s_playerSubMenue = 1;
                            changeState(PLAYER);
                        }
                    } else if (s_state == AUDIOFILESLIST) {
                        s_playerSubMenue = 0;
                        changeState(PLAYER);
                    } else if (s_state == DLNA) {
                        if (!txt_DL_fName.isEnabled()) { txt_DL_fName.show(true, false); } // assume volBox is shown
                        if (s_dlnaSubMenue == 1) {
                            s_dlnaSubMenue = 0;
                            changeState(DLNA);
                        }
                    } else if (s_state == DLNAITEMSLIST) {
                        s_dlnaSubMenue = 0;
                        changeState(DLNA);
                    } else if (s_state == CLOCK) {
                        s_clockSubMenue = 0;
                        changeState(CLOCK);
                    } else if (s_state == ALARMCLOCK) {
                        s_alarmSubMenue = 0;
                        changeState(ALARMCLOCK);
                    } else if (s_state == SETTINGS) {
                        s_settingsSubMenue = 0;
                        changeState(SETTINGS);
                    } else if (s_state == BRIGHTNESS) {
                        s_brightnessSubMenue = 0;
                        changeState(BRIGHTNESS);
                    } else if (s_state == EQUALIZER) {
                        s_equalizerSubMenue = 0;
                        changeState(EQUALIZER);
                    } else {
                        ;
                    } // all other, do nothing
                }
            }
        }

        if (!s_f_rtc) { s_f_rtc = rtc.hasValidTime(); }

        int16_t audioVol = audio.getVolume();
        uint8_t currVol = s_volume.cur_volume;
        if (s_f_mute) currVol = 0;
        uint8_t steps = s_volume.volumeSteps / 7;
        if (audioVol > currVol) { // downvolume
            if (audioVol - steps >= currVol) {
                if (audioVol - steps < 0)
                    audio.setVolume(0, s_volume.volumeCurve);
                else
                    audio.setVolume(audioVol - steps, s_volume.volumeCurve);
            } else
                audio.setVolume(audioVol - 1, s_volume.volumeCurve);
        }
        if (audioVol < currVol) { // upvolume
            if (audioVol + steps <= currVol) {
                if (audioVol + steps > 255)
                    audio.setVolume(255, s_volume.volumeCurve);
                else
                    audio.setVolume(audioVol + steps, s_volume.volumeCurve);
            } else {
                audio.setVolume(audioVol + 1, s_volume.volumeCurve);
            }
        }
    }
    //-----------------------------------------------------1 SEC--------------------------------------------------------------------------------------

    if (s_f_1sec) { // calls every second
        s_f_1sec = false;
        s_totalRuntime++;
        uint16_t minuteOfTheDay = rtc.getMinuteOfTheDay();
        uint8_t  weekDay = rtc.getweekday();
        if (s_timeFormat == 24)
            clk_CL_24.updateTime(minuteOfTheDay, weekDay);
        else
            clk_CL_12.updateTime(minuteOfTheDay, weekDay);
        if (s_state == RINGING) clk_RI_24small.updateTime(minuteOfTheDay, weekDay);
        static uint8_t semaphore = 0;
        if (!semaphore) { s_f_alarm = isAlarm(weekDay, s_alarmdays, minuteOfTheDay, s_alarmtime) && s_f_rtc; } // alarm if rtc and CL green
        if (s_f_alarm) { semaphore++; }
        if (semaphore) { semaphore++; }
        if (semaphore >= 65) { semaphore = 0; }

        //------------------------------------------ALARM MANAGEMENT----------------------------------------------------------------------------------
        if (s_f_alarm) {
            s_f_alarm = false;
            changeState(RINGING);
        }
        if (s_f_eof_alarm) { // AFTER RINGING
            s_f_eof_alarm = false;
            if (!s_f_rtc) return;
            s_volume.cur_volume = s_volume.volumeAfterAlarm;
            setVolume(s_volume.cur_volume);
            audio.setVolume(s_volume.cur_volume, s_volume.volumeCurve);
            dispHeader.updateVolume(s_volume.cur_volume);
            wake_up();
        }

        if (s_f_stationsChanged) {
            s_f_stationsChanged = false;
            staMgnt.updateStationsList();
        }
        dispHeader.updateRSSI(WiFi.RSSI());
        //------------------------------------------UPDATE DISPLAY------------------------------------------------------------------------------------
        if (!s_f_sleeping || s_state == RINGING) {
            dispHeader.updateTime(s_time_s, false);
            if (s_f_newBitRate) {
                s_f_newBitRate = false;
                dispFooter.updateBitRate(s_icyBitRate);
            }
            if (s_f_newStationName) {
                s_f_newStationName = false;
                showLogoAndStationName(false);
            }
        }
        //---------------------------------------------TIME SPEECH -----------------------------------------------------------------------------------
        static bool f_resume = false;
        if (s_f_timeSpeech) { // speech the time 7 sec before a new hour is arrived
            s_f_timeSpeech = false;
            int hour = atoi(s_time_s);
            hour++;
            if (hour == 24) hour = 0; //  extract the hour
            if (s_f_mute) return;
            if (s_f_sleeping) return;
            if (s_state != RADIO) return;
            if (s_f_timeAnnouncement) {
                f_resume = true;
                s_f_eof = false;
                if (s_timeFormat == 12) {
                    if (hour > 12) hour -= 12;
                }
                sprintf(s_chbuf, "/voice_time/%s/%d_00.mp3", s_timeSpeechLang, hour);
                SerialPrintfln("Time: ...... play Audiofile %s", s_chbuf);
                connecttoFS("SD_MMC", s_chbuf);
                return;
            } else {
                SerialPrintfln("Time: ...... Announcement at %d o'clock is silent", hour);
            }
        }
        if (f_resume && s_f_eof) {
            f_resume = false;
            s_f_eof = false;
            if (s_cur_station)
                setStation(s_cur_station);
            else
                connecttohost(s_settings.lastconnectedhost.get());
            return;
        }
        //------------------------------------------AUDIO_CURRENT_TIME - DURATION---------------------------------------------------------------------
        if (audio.isRunning()) {
            s_audioFileDuration = audio.getAudioFileDuration();
            if (s_audioFileDuration > 0) {
                s_audioCurrentTime = audio.getAudioCurrentTime();
                if (s_state == PLAYER && s_audioFileDuration) {
                    pgb_PL_progress.setNewMinMaxVal(0, s_audioFileDuration);
                    pgb_PL_progress.setValue(s_audioCurrentTime);
                }
                if (s_state == DLNA && s_audioFileDuration) {
                    pgb_DL_progress.setNewMinMaxVal(0, s_audioFileDuration);
                    pgb_DL_progress.setValue(s_audioCurrentTime);
                }
                if (s_audioFileDuration) {
                    SerialPrintfcr("AUDIO_FILE:  " ANSI_ESC_GREEN "AudioCurrentTime " ANSI_ESC_GREEN "%li:%02lis, " ANSI_ESC_GREEN "AudioFileDuration " ANSI_ESC_GREEN "%li:%02lis      ",
                                   (long int)s_audioCurrentTime / 60, (long int)s_audioCurrentTime % 60, (long int)s_audioFileDuration / 60, (long int)s_audioFileDuration % 60);
                }
            }
        }
        //------------------------------------------NEW STREAMTITLE-----------------------------------------------------------------------------------
        if (s_f_newStreamTitle && !s_timeCounter.timer) {
            s_f_newStreamTitle = false;
            if (s_state == RADIO) {
                if (strlen(s_streamTitle))
                    showStreamTitle(s_streamTitle);
                else if (strlen(s_icyDescription)) {
                    showStreamTitle(s_icyDescription);
                    s_f_newIcyDescription = false;
                    webSrv.send("icy_description=", s_icyDescription);
                } else
                    txt_RA_sTitle.writeText("");
            }
            webSrv.send("streamtitle=", s_streamTitle);
        }
        //------------------------------------------NEW ICY-DESCRIPTION-------------------------------------------------------------------------------
        if (s_f_newIcyDescription && !s_timeCounter.timer) {
            if (s_state == RADIO) {
                if (!strlen(s_streamTitle)) showStreamTitle(s_icyDescription);
            }
            webSrv.send("icy_description=", s_icyDescription);
            s_f_newIcyDescription = false;
        }
        //------------------------------------------NEW COMMERCIALS-----------------------------------------------------------------------------------
        if (s_f_newCommercial && !s_timeCounter.timer) {
            if (s_state == RADIO) { showStreamTitle(s_commercial); }
            webSrv.send("streamtitle=", s_commercial);
            s_f_newCommercial = false;
        }
        //------------------------------------------END OF COMMERCIALS--------------------------------------------------------------------------------
        if (s_commercial_dur > 0) {
            s_commercial_dur--;
            if ((s_commercial_dur == 2) && (s_state == RADIO)) /* showStreamTitle(""); */
                s_f_newStreamTitle = true;                     // end of commercial? clear streamtitle
        }
        //------------------------------------------DETERMINE AUDIOCODEC------------------------------------------------------------------------------
        if (s_cur_Codec == 0) {
            uint8_t c = audio.getCodec();
            if (c != 0 && c != 8 && c < 10) { // unknown or OGG, guard: c {1 ... 7, 9}
                s_cur_Codec = c;
                SerialPrintfln("Audiocodec:  " ANSI_ESC_YELLOW "%s", codecname[c]);
                if (s_state == PLAYER) showFileLogo(PLAYER);
                if (s_state == RADIO && s_f_logoUnknown == true) {
                    s_f_logoUnknown = false;
                    showFileLogo(s_state);
                }
            }
        }
        //------------------------------------------CONNECT TO LASTHOST-------------------------------------------------------------------------------
        if (s_f_connectToLastStation) { // not used yet
            s_f_connectToLastStation = false;
            if (s_cur_station)
                setStation(s_cur_station);
            else
                connecttohost(s_settings.lastconnectedhost.get());
        }
        //------------------------------------------RECONNECT AFTER FAIL------------------------------------------------------------------------------
        if (s_f_reconnect && !s_f_WiFiConnected) {
            s_f_reconnect = false;
            connecttohost(s_settings.lastconnectedhost.get());
        }
        //------------------------------------------SEEK DLNA SERVER----------------------------------------------------------------------------------
        if (s_f_dlnaSeekServer) {
            s_f_dlnaSeekServer = false;
            dlna.seekServer();
        }
        //------------------------------------------CREATE DLNA PLAYLIST------------------------------------------------------------------------------
        if (s_f_dlnaMakePlaylistOTF && s_f_dlna_browseReady) {
            s_f_dlnaMakePlaylistOTF = false;
            s_f_dlna_browseReady = false;
            if (preparePlaylistFromDLNAFolder()) processPlaylist(true);
        }
        //------------------------------------------DLNA ITEMS RECEIVED-------------------------------------------------------------------------------
        if (s_f_dlna_browseReady) { // unused
            s_f_dlna_browseReady = false;
        }
        //-------------------------------------------WIFI DISCONNECTED?-------------------------------------------------------------------------------
        if (s_f_WiFiConnected) {
            if ((WiFi.status() != WL_CONNECTED)) {
                s_WiFi_disconnectCnt++;
                if (s_WiFi_disconnectCnt == 15) {
                    s_WiFi_disconnectCnt = 1;
                    SerialPrintfln("WiFi      :  " ANSI_ESC_YELLOW "Reconnecting to WiFi...");
                    WiFi.disconnect();
                    WiFi.reconnect();
                }
            } else {
                if (s_WiFi_disconnectCnt) {
                    s_WiFi_disconnectCnt = 0;
                    if (s_state == RADIO) audio.connecttohost(s_settings.lastconnectedhost.get());
                }
            }
        }
        //------------------------------------------CONNECTTOHOST FAIL--------------------------------------------------------------------------------
        static uint8_t failCnt = 0;
        if (s_f_webFailed) {
            failCnt++;
            if (failCnt == 30) {
                failCnt = 0;
                if (WiFi.isConnected())
                    connecttohost(s_settings.lastconnectedhost.get());
                else
                    ESP.restart();
            }
        } else
            failCnt = 0;
        //------------------------------------------GET AUDIO FILE ITEMS------------------------------------------------------------------------------
        if (s_f_isFSConnected) {
            //    uint32_t t = 0;
            //    uint32_t fs = audioGetFileSize();
            //    uint32_t br = audioGetBitRate();
            //    if(br) t = (fs * 8)/ br;
            //    log_w("Br %d, Dur %ds", br, t);
        }
        //--------------------------------------AMBIENT LIGHT SENSOR BH1750---------------------------------------------------------------------------
        if (s_f_BH1750_found) {
            int32_t ambVal = BH1750.getBrightness();
            if (ambVal < 0) goto endbrightness;
            if (ambVal > 350) ambVal = 350;
            s_bh1750Value = map_l(ambVal, 0, 350, 5, 100);
            //    log_i("s_bh1750Value %i, s_brightness %i", s_bh1750Value, s_brightness);
            if (!s_f_sleeping) {
                if (s_bh1750Value >= s_brightness)
                    setTFTbrightness(s_bh1750Value);
                else
                    setTFTbrightness(s_brightness);
            }
        endbrightness:
            BH1750.start();
        }
    } //  END s_f_1sec

    if (s_f_10sec == true) { // calls every 10 seconds
        s_f_10sec = false;
        // if(s_state == RADIO && !s_icyBitRate && !s_f_sleeping) {
        //     s_decoderBitRate = audio.getBitRate();
        //     static uint32_t oldBr = 0;
        //     if(s_decoderBitRate != oldBr){
        //         oldBr = s_decoderBitRate;
        //         dispFooter.updateBitRate(s_decoderBitRate);
        //     }
        // }
        updateSettings();
    }

    if (s_f_1min == true) { // calls every minute
        s_f_1min = false;
        if (s_sleeptime) {
            s_sleeptime--;
            if (!s_sleeptime) fall_asleep();
            dispFooter.updateOffTime(s_sleeptime);
        }
        static uint8_t btEmitterCnt = 0;
        if (!s_bt_emitter.found && btEmitterCnt < 1) {
            btEmitterCnt++;
            bt_emitter.begin(); // if the emitter has not yet responded
        }
    }

    //-------------------------------------------------DEBUG / WIFI_SETTINGS ----------------------------------------------------------------------------------
    if (Serial.available()) { // input: serial terminal
        String r = Serial.readString();
        r.replace("\n", "");
        SerialPrintfln("Terminal  :  " ANSI_ESC_YELLOW "%s", r.c_str());
        if (r.startsWith("pr")) {
            s_f_pauseResume = audio.pauseResume();
            if (s_f_pauseResume) {
                SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume");
            } else {
                SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume not possible");
            }
        }
        if (r.startsWith("hc")) { // A make_hardcopy_on_sd of the display is created and written to the SD card
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "create hardcopy"); }
            make_hardcopy_on_sd();
        }
        if (r.startsWith("rts")) { // run time stats
            char* timeStatsBuffer = x_ps_calloc(2000, sizeof(char));
            GetRunTimeStats(timeStatsBuffer);
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "task statistics\n\n%s", timeStatsBuffer); }
            x_ps_free(&timeStatsBuffer);
        }
        if (r.startsWith("cts")) { // connect to speech
            audio.connecttospeech("Hallo, wie geht es dir? Morgen scheint die Sonne und übermorgen regnet es.Aber wir nehmen den Regenschirm mit. Und auch den Rucksack. Dann lesen wir aus dem Buch "
                                  "Hier gibt es nur gutes Wetter.",
                                  "de");
            //    audio.connecttospeech("Hallo", "de");
        }

        if (r.startsWith("bfi")) { // buffer filled
            SerialPrintfln("inBuffer  :  filled %lu bytes", (long unsigned)audio.inBufferFilled());
            SerialPrintfln("inBuffer  :  free   %lu bytes", (long unsigned)audio.inBufferFree());
        }
        if (r.startsWith("st")) { // testtext for streamtitle
            if (r[2] == '0') strcpy(s_streamTitle, "A Ё Ю");
            if (r[2] == '1') strcpy(s_streamTitle, "A B C D E F G");
            if (r[2] == '2') strcpy(s_streamTitle, "A B C D E F G H I");
            if (r[2] == '3') strcpy(s_streamTitle, "A B C D E F G H I J K L");
            if (r[2] == '4') strcpy(s_streamTitle, "A B C D E F G H I J K J M Q O");
            if (r[2] == '5') strcpy(s_streamTitle, "A B C D E F G H I K L J M y O P Q R");
            if (r[2] == '6')
                strcpy(s_streamTitle, "A B C D E F G H I K L J M g O P Q R S T V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T "
                                      "U V W K J Q p O P Q R S T U V W K J Q V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U "
                                      "V W K J Q p O P Q R S T U V W K J Q");
            if (r[2] == '7')
                strcpy(s_streamTitle, "A B C D E F G H I K L J M j O P Q R S T U V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S "
                                      "T U V W K J Q p O P Q R S T U V W K J Q");
            if (r[2] == '8') strcpy(s_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if (r[2] == '9') strcpy(s_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            log_w("st: %s", s_streamTitle);
            s_f_newStreamTitle = true;
        }
        if (r.startsWith("ais")) { // openAIspeech
            log_w("openAI speech");
            //    audio.openai_speech("openAI-key", "tts-1", "Today is a wonderful day to build something people love!", "", "shimer", "mp3", "1");
        }
        if(r.startsWith("ctfs")){ // connecttoFS
        //     log_w("SPIFFS");
            connecttoFS("SD", "/Collide.ogg");
        }
        if (r.startsWith("sto")) { // setTimeOffset
            int32_t t = r.substring(3, r.length() - 1).toInt();
            log_w("setTimeOffset %li", t);
            audio.setTimeOffset(t);
        }

        if (r.startsWith("sapt")) { // setAudioPlayTime
            uint32_t t = r.substring(4, r.length() - 1).toInt();
            log_w("setAudioPlayTime %lu", t);
            audio.setAudioPlayTime(t);
        }

        if (r.startsWith("gafp")) { // getAudioFilePosition
            log_w("getAudioFilePosition %lu", audio.getAudioFilePosition());
        }

        if (r.startsWith("safp")) { // setAudioFilePosition
            uint32_t t = r.substring(4, r.length() - 1).toInt();
            log_w("setAudioFilePosition %lu", t);
            audio.setAudioFilePosition(t);
        }

        if (r.startsWith("grn")) { // list of all self registered objects
            get_registered_names();
        }
        if (r.startsWith("fm")) { // force mono
            static bool f_mono = false;
            f_mono = !f_mono;
            audio.forceMono(f_mono);
            if (f_mono)
                log_w("mono");
            else
                log_w("stereo");
        }
        if (r.startsWith("btp")) { // bluetooth RX/TX protocol
            bt_emitter.list_protokol();
        }
        if (r.startsWith("btstr")) { // bluetooth string, send to bt emitter e.g. btstr:AT+
            bt_emitter.userCommand(r.substring(6, r.length() - 1).c_str());
            log_w("btstr: %s", r.substring(6, r.length() - 1).c_str());
        }
        if (r.startsWith("tsp")) { s_f_timeSpeech = true; }
        if (r.startsWith("pwd")) { // set password for WiFi
            changeState(WIFI_SETTINGS);
        }
        if (r.startsWith("gif")) { // draw gif image
            log_w("gif");
            drawImage("/common/Tom_Jerry.gif", 100, 100);
        }
        static uint32_t time = 0;
        if (r.startsWith("stops")) { // stop song
            time = audio.stopSong();
            log_w("file %s stopped at time %lu", s_cur_AudioFileName, time);
        }
        if (r.startsWith("starts")) { // start song
            String path = "/audiofiles/" + (String)s_cur_AudioFileName;
            audio.connecttoFS(SD_MMC, path.c_str(), time);
            log_w("file %s started at time %lu", s_cur_AudioFileName, time);
        }

        if (r.startsWith("gbr")) { // get bitrate
            uint32_t br = audio.getBitRate();
            log_w("bitrate: %lu", br);
        }
    }
}

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                  E V E N T S                                                                                ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// Events from audioI2S library
void my_audio_info(Audio::msg_t m) {
    switch (m.e) {
        case Audio::evt_info:
            if (endsWith(m.msg, "failed!")) {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_YELLOW, m.msg);
                sprintf(s_streamTitle, "" ANSI_ESC_ORANGE "%s", m.msg);
                s_f_newStreamTitle = true;
                s_f_webFailed = true;
                return;
            }
            if (startsWith(m.msg, "FLAC")) {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN, m.msg);
                return;
            }
            if (endsWith(m.msg, "Stream lost")) {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_YELLOW, m.msg);
                return;
            }
            if (startsWith(m.msg, "authent")) {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN, m.msg);
                return;
            }
            if (startsWith(m.msg, "StreamTitle=")) { return; }
            if (startsWith(m.msg, "HTTP/") && m.msg[9] > '3') {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED, m.msg);
                return;
            }
            if (startsWith(m.msg, "ERROR:")) {
                SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED, m.msg);
                return;
            }
            if (CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN) {
                SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", m.msg);
                return;
            } // all other
            break;

        case Audio::evt_name:
            x_ps_free(&s_stationName_air);
            s_stationName_air = x_ps_strndup(m.msg, 200); // set max length
            SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", m.msg);
            s_f_newStationName = true;
            break;

        case Audio::evt_streamtitle:
            strcpy(s_streamTitle, m.msg);
            if (!s_f_irNumberSeen) s_f_newStreamTitle = true;
            SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", m.msg);
            break;

        case Audio::evt_eof:
            s_f_isWebConnected = false;
            s_f_eof = true;
            s_f_isFSConnected = false;
            SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, m.msg);
            if (s_state == PLAYER) {
                webSrv.send("SD_playFile=", "end of audiofile");
                if (!s_f_playlistEnabled) {
                    s_f_clearLogo = true;
                    s_f_clearStationName = true;
                    s_playerSubMenue = 0;
                    changeState(PLAYER);
                }
            }
            if (s_state == RADIO) { clearWithOutHeaderFooter(); }
            if (s_state == DLNA) {
                txt_DL_fName.setText("");
                txt_DL_fName.show(true, false);
                btn_DL_pause.setActive(false);
                btn_DL_pause.show();
            }
            s_f_eof = true;
            break;

        case Audio::evt_lasthost:
            if (s_f_playlistEnabled) return;
            s_settings.lastconnectedhost.assign(m.msg);
            SerialPrintflnCut("lastURL: ..  ", ANSI_ESC_WHITE, s_settings.lastconnectedhost.get());
            webSrv.send("stationURL=", s_settings.lastconnectedhost.get());
            break;

        case Audio::evt_icyurl:
            if (strlen(m.msg) > 5) {
                SerialPrintflnCut("icy-url: ..  ", ANSI_ESC_WHITE, m.msg);
                s_homepage = String(m.msg);
                if (!s_homepage.startsWith("http")) s_homepage = "http://" + s_homepage;
            }
            break;

        case Audio::evt_icylogo:
            if (strlen(m.msg) > 5) { SerialPrintflnCut("icy-logo:    ", ANSI_ESC_WHITE, m.msg); }
            break;

        case Audio::evt_id3data: SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s", m.msg); break;

        case Audio::evt_image:
            for (int i = 0; i < m.vec.size(); i += 2) { SerialPrintfln("CoverImage:  " ANSI_ESC_GREEN "segment %02i, pos %08i, len %08i", i / 2, m.vec[i], m.vec[i + 1]); }
            break;

        case Audio::evt_icydescription:
            strcpy(s_icyDescription, m.msg);
            s_f_newIcyDescription = true;
            if (strlen(m.msg)) SerialPrintfln("icy-descr:   %s", m.msg);
            break;

        case Audio::evt_bitrate:
            if (!strlen(m.msg)) return; // guard
            s_icyBitRate = str2int(m.msg);
            s_f_newBitRate = true;
            SerialPrintfln("bitRate:     " ANSI_ESC_GREEN "%i", s_icyBitRate);
            break;

        case Audio::evt_lyrics:
            SerialPrintfln("sync lyrics: " ANSI_ESC_CYAN "%s", m.msg);
            if (s_state == RADIO) showStreamTitle(m.msg); // web file
            if (s_state == PLAYER) showFileName(m.msg);
            break;

        case Audio::evt_log: SerialPrintfln("%s: .....  %s", m.s, m.msg); break;

        default: SerialPrintfln("message:...  %s", m.msg); break;
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_process_i2s(int16_t* outBuff, uint16_t validSamples, bool* continueI2S) {

    // int16_t sineWaveTable[44] = {
    //      0,   3743,   7377,  10793,  14082,  17136,  19848,  22113,  23825,  24908,
    //   25311,  24908,  23825,  22113,  19848,  17136,  14082,  10793,   7377,   3743,
    //      0,  -3743,  -7377, -10793, -14082, -17136, -19848, -22113, -23825, -24908,
    //  -25311, -24908, -23825, -22113, -19848, -17136, -14082, -10793,  -7377,  -3743
    // };

    // static uint8_t tabPtr = 0;
    // int16_t* sample[2]; // assume 2 channels, 16bit
    // for(int i= 0; i < validSamples; i++){
    //     *(sample + 0) = outBuff + i * 2;     // channel left
    //     *(sample + 1) = outBuff + i * 2 + 1; // channel right

    //     *(*sample + 0) = (sineWaveTable[tabPtr] /50 + *(*sample + 0));
    //     *(*sample + 1) = (sineWaveTable[tabPtr] /50 + *(*sample + 1));
    //     tabPtr++;
    //     if(tabPtr == 44) tabPtr = 0;
    // }
    *continueI2S = true;
}
// ————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ftp_debug(const char* info) {
    if (startsWith(info, "File Name")) return;
    SerialPrintfln("ftpServer:   %s", info);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Events from rtime library
void RTIME_info(const char* info) {
    SerialPrintfln("rtime_info:  %s", info);
}
// Events from tft library
void tft_info(const char* info) {
    SerialPrintfln("tft_info: .  %s", info);
}
// Events from tp library
void tp_info(const char* info) {
    SerialPrintfln("tp_info: ..  %s", info);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Events from IR Library
void ir_code(uint8_t addr, uint8_t cmd) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "0x%02x, " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02x", addr, cmd);
    char buf[20];
    sprintf(buf, "0x%02x", addr);
    webSrv.send("IR_address=", buf);
    sprintf(buf, "0x%02x", cmd);
    webSrv.send("IR_command=", buf);
}

void ir_res(uint32_t res) {
    if (s_state != RADIO) return;
    if (s_f_sleeping == true) return;
    SerialPrintfln("ir_result:   " ANSI_ESC_YELLOW "Stationnumber " ANSI_ESC_BLUE "%lu", (long unsigned)res);
    s_f_irResultSeen = true;
    s_f_irNumberSeen = false;
    s_irResult = res;
    return;
}
void ir_number(uint16_t num) {
    if (s_state != RADIO) return;
    if (s_f_sleeping) return;
    s_f_irNumberSeen = true;
    s_irNumber = num;
    s_radioSubMenue = 3;
    changeState(RADIO);
}
void ir_short_key(uint8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "short pressed key nr: " ANSI_ESC_BLUE "%02i", key);
    if (s_f_sleeping == true && !s_f_irOnOff) {
        wake_up();
        return;
    }
    if (s_f_irOnOff == true && key != 20) return;
    if (s_state == IR_SETTINGS) return; // nothing todo
    static uint8_t btnNr = 0;
    switch (key) {
        case 10: // MUTE
            muteChanged(!s_f_mute);
            break;
        case 11: // ARROW RIGHT
            if (s_state == RADIO) {
                if (s_radioSubMenue < 4) {
                    nextFavStation();
                    return; // NEXT STATION
                }
                if (s_radioSubMenue == 4) {   // scroll forward
                    if (btnNr > 7) btnNr = 0; // guard
                    btnNr++;
                    if (btnNr == 1) {
                        btn_RA_staList.show();
                        btn_RA_player.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_RA_player.show();
                        btn_RA_dlna.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_RA_dlna.show();
                        btn_RA_clock.showAlternativePic();
                    }
                    if (btnNr == 4) {
                        btn_RA_clock.show();
                        btn_RA_sleep.showAlternativePic();
                    }
                    if (btnNr == 5) {
                        btn_RA_sleep.show();
                        btn_RA_settings.showAlternativePic();
                    }
                    if (btnNr == 6) {
                        btn_RA_settings.show();
                        btn_RA_bt.showAlternativePic();
                    }
                    if (btnNr == 7) {
                        btn_RA_bt.show();
                        btn_RA_off.showAlternativePic();
                    }
                    if (btnNr == 8) {
                        btn_RA_off.show();
                        btn_RA_staList.showAlternativePic();
                        btnNr = 0;
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == STATIONSLIST) { // next page
                lst_RADIO.nextPage();
                setTimeCounter(LIST_TIMER);
                return;
            }
            if (s_state == PLAYER) {
                if (s_playerSubMenue == 2) {  // scroll forward
                    if (btnNr > 7) btnNr = 0; // guard
                    btnNr++;
                    if (btnNr == 1) {
                        btn_PL_prevFile.show();
                        btn_PL_nextFile.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_PL_nextFile.show();
                        btn_PL_ready.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_PL_ready.show();
                        btn_PL_playAll.showAlternativePic();
                    }
                    if (btnNr == 4) {
                        btn_PL_playAll.show();
                        btn_PL_shuffle.showAlternativePic();
                    }
                    if (btnNr == 5) {
                        btn_PL_shuffle.show();
                        btn_PL_fileList.showAlternativePic();
                    }
                    if (btnNr == 6) {
                        btn_PL_fileList.show();
                        btn_PL_radio.showAlternativePic();
                    }
                    if (btnNr == 7) {
                        btn_PL_radio.show();
                        btn_PL_off.showAlternativePic();
                    }
                    if (btnNr == 8) {
                        btn_PL_off.show();
                        btn_PL_prevFile.showAlternativePic();
                        btnNr = 0;
                    }
                    setTimeCounter(2);
                    return;
                }
                if (s_playerSubMenue == 3) { // scroll forward (mute, pause, cancel, prev, next)
                    if (btnNr < 4) btnNr++;
                    if (btnNr == 1) {
                        btn_PL_mute.show();
                        btn_PL_pause.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_PL_pause.show();
                        btn_PL_cancel.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_PL_cancel.show();
                        btn_PL_playPrev.showAlternativePic();
                    }
                    if (btnNr == 4) {
                        btn_PL_playPrev.show();
                        btn_PL_playNext.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == AUDIOFILESLIST) {
                lst_PLAYER.nextPage();
                setTimeCounter(LIST_TIMER);
                return; // next page
            }
            if (s_state == DLNA) {
                if (s_dlnaSubMenue == 1) { // scroll forward (mute, pause, cancel, prev, next)
                    if (btnNr < 4) btnNr++;
                    if (btnNr == 1) {
                        btn_DL_mute.show();
                        btn_DL_pause.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_DL_pause.show();
                        btn_DL_cancel.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_DL_cancel.show();
                        btn_DL_fileList.showAlternativePic();
                    }
                    if (btnNr == 4) {
                        btn_DL_fileList.show();
                        btn_DL_radio.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == DLNAITEMSLIST) {
                //                        lst_DLNA.nextPage(); setTimeCounter(LIST_TIMER); return; // nextpage
            }
            if (s_state == CLOCK) {
                if (s_clockSubMenue == 2) { // scroll forward (alarm, radio, mute, off)
                    if (btnNr < 3) btnNr++;
                    if (btnNr == 1) {
                        btn_CL_alarm.show();
                        btn_CL_radio.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_CL_radio.show();
                        btn_CL_mute.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_CL_mute.show();
                        btn_CL_off.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == ALARMCLOCK) {
                if (s_alarmSubMenue == 1) { // scroll forward (left, right, up, down, ready)
                    if (btnNr < 4) btnNr++;
                    if (btnNr == 1) {
                        btn_AC_left.show();
                        btn_AC_right.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_AC_right.show();
                        btn_AC_up.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_AC_up.show();
                        btn_AC_down.showAlternativePic();
                    }
                    if (btnNr == 4) {
                        btn_AC_down.show();
                        btn_AC_ready.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == SLEEPTIMER) {
                if (s_sleepTimerSubMenue == 1) { // scroll forward (up, down, ready, cancel)
                    if (btnNr < 3) btnNr++;
                    if (btnNr == 1) {
                        btn_SL_up.show();
                        btn_SL_down.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_SL_down.show();
                        btn_SL_ready.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_SL_ready.show();
                        btn_SL_cancel.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == SETTINGS) {
                if (s_settingsSubMenue == 1) { // scroll forward (bright, equal, wifi, radio)
                    if (btnNr < 3) btnNr++;
                    if (btnNr == 1) {
                        btn_SE_bright.show();
                        btn_SE_equal.showAlternativePic();
                    }
                    if (btnNr == 2) {
                        btn_SE_equal.show();
                        btn_SE_wifi.showAlternativePic();
                    }
                    if (btnNr == 3) {
                        btn_SE_wifi.show();
                        btn_SE_radio.showAlternativePic();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == BRIGHTNESS) {
                if (s_brightnessSubMenue == 1) {
                    s_brightness += 5;
                    if (s_brightness > 100) s_brightness = 100;
                    sdr_BR_value.setValue(s_brightness);
                    setTimeCounter(2);
                }
            }
            if (s_state == EQUALIZER) {
                if (s_equalizerSubMenue == 1) {
                    char c[10];
                    if (btnNr == 10) {
                        if (s_toneBAL < 16) {
                            s_toneBAL++;
                            itoa(s_toneBAL, c, 10);
                            txt_EQ_balance.writeText(c);
                            sdr_EQ_balance.setValue(s_toneBAL);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // balance
                    if (btnNr == 20) {
                        if (s_toneLP < 6) {
                            s_toneLP++;
                            itoa(s_toneLP, c, 10);
                            txt_EQ_lowPass.writeText(c);
                            sdr_EQ_lowPass.setValue(s_toneLP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // HP
                    if (btnNr == 30) {
                        if (s_toneBP < 6) {
                            s_toneBP++;
                            itoa(s_toneBP, c, 10);
                            txt_EQ_bandPass.writeText(c);
                            sdr_EQ_bandPass.setValue(s_toneBP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // BP
                    if (btnNr == 40) {
                        if (s_toneHP < 6) {
                            s_toneHP++;
                            itoa(s_toneHP, c, 10);
                            txt_EQ_highPass.writeText(c);
                            sdr_EQ_highPass.setValue(s_toneHP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // LP
                    if (btnNr == 1) {
                        btnNr = 2;
                        btn_EQ_Player.show();
                        btn_EQ_mute.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 0) {
                        btnNr = 1;
                        btn_EQ_Radio.show();
                        btn_EQ_Player.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                }
            }
            break;
        case 12: // ARROW LEFT
            if (s_state == RADIO) {
                if (s_radioSubMenue < 4) {
                    prevFavStation();
                    return; // PREV STATION
                }
                if (s_radioSubMenue == 4) {   // scroll backward
                    if (btnNr > 7) btnNr = 0; // guard
                    if (btnNr == 0) btnNr = 8;
                    btnNr--;
                    if (btnNr == 0) {
                        btn_RA_staList.showAlternativePic();
                        btn_RA_player.show();
                    }
                    if (btnNr == 1) {
                        btn_RA_player.showAlternativePic();
                        btn_RA_dlna.show();
                    }
                    if (btnNr == 2) {
                        btn_RA_dlna.showAlternativePic();
                        btn_RA_clock.show();
                    }
                    if (btnNr == 3) {
                        btn_RA_clock.showAlternativePic();
                        btn_RA_sleep.show();
                    }
                    if (btnNr == 4) {
                        btn_RA_sleep.showAlternativePic();
                        btn_RA_settings.show();
                    }
                    if (btnNr == 5) {
                        btn_RA_settings.showAlternativePic();
                        btn_RA_bt.show();
                    }
                    if (btnNr == 6) {
                        btn_RA_bt.showAlternativePic();
                        btn_RA_off.show();
                    }
                    if (btnNr == 7) {
                        btn_RA_off.showAlternativePic();
                        btn_RA_staList.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == STATIONSLIST) {
                lst_RADIO.prevPage();
                setTimeCounter(LIST_TIMER);
                break;
            } // prev page
            if (s_state == PLAYER) {
                if (s_playerSubMenue == 2) {  // scroll backward
                    if (btnNr > 7) btnNr = 0; // guard
                    if (btnNr == 0) btnNr = 8;
                    btnNr--;
                    if (btnNr == 0) {
                        btn_PL_prevFile.showAlternativePic();
                        btn_PL_nextFile.show();
                    }
                    if (btnNr == 1) {
                        btn_PL_nextFile.showAlternativePic();
                        btn_PL_ready.show();
                    }
                    if (btnNr == 2) {
                        btn_PL_ready.showAlternativePic();
                        btn_PL_playAll.show();
                    }
                    if (btnNr == 3) {
                        btn_PL_playAll.showAlternativePic();
                        btn_PL_shuffle.show();
                    }
                    if (btnNr == 4) {
                        btn_PL_shuffle.showAlternativePic();
                        btn_PL_fileList.show();
                    }
                    if (btnNr == 5) {
                        btn_PL_fileList.showAlternativePic();
                        btn_PL_radio.show();
                    }
                    if (btnNr == 6) {
                        btn_PL_radio.showAlternativePic();
                        btn_PL_off.show();
                    }
                    if (btnNr == 7) {
                        btn_PL_off.showAlternativePic();
                        btn_PL_prevFile.show();
                    }
                    setTimeCounter(2);
                    return;
                }
                if (s_playerSubMenue == 3) { // scroll backward (mute, pause, cancel, prev, next)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_PL_mute.showAlternativePic();
                        btn_PL_pause.show();
                    }
                    if (btnNr == 1) {
                        btn_PL_pause.showAlternativePic();
                        btn_PL_cancel.show();
                    }
                    if (btnNr == 2) {
                        btn_PL_cancel.showAlternativePic();
                        btn_PL_playPrev.show();
                    }
                    if (btnNr == 3) {
                        btn_PL_playPrev.showAlternativePic();
                        btn_PL_playNext.show();
                    }
                    setTimeCounter(2);
                    return;
                }
                if (s_cur_AudioFileNr > 0) {
                    s_cur_AudioFileNr--;
                    showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                    showAudioFileNumber();
                    return;
                }
            }
            if (s_state == AUDIOFILESLIST) {
                lst_PLAYER.prevPage();
                setTimeCounter(LIST_TIMER);
                break; // prev page
            }
            if (s_state == DLNA) {
                if (s_dlnaSubMenue == 1) { // scroll backward (mute, pause, cancel, prev, next)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_DL_mute.showAlternativePic();
                        btn_DL_pause.show();
                    }
                    if (btnNr == 1) {
                        btn_DL_pause.showAlternativePic();
                        btn_DL_cancel.show();
                    }
                    if (btnNr == 2) {
                        btn_DL_cancel.showAlternativePic();
                        btn_DL_fileList.show();
                    }
                    if (btnNr == 3) {
                        btn_DL_fileList.showAlternativePic();
                        btn_DL_radio.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == DLNAITEMSLIST) {
                //                        lst_DLNA.prevPage(); setTimeCounter(LIST_TIMER); return; // prev page
            }

            if (s_state == CLOCK) {
                if (s_clockSubMenue == 2) { // scroll backward (alarm, radio, mute, off)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_CL_alarm.showAlternativePic();
                        btn_CL_radio.show();
                    }
                    if (btnNr == 1) {
                        btn_CL_radio.showAlternativePic();
                        btn_CL_mute.show();
                    }
                    if (btnNr == 2) {
                        btn_CL_mute.showAlternativePic();
                        btn_CL_off.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == ALARMCLOCK) {
                if (s_alarmSubMenue == 1) { // scroll backward (left, right, up, down, ready)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_AC_left.showAlternativePic();
                        btn_AC_right.show();
                    }
                    if (btnNr == 1) {
                        btn_AC_right.showAlternativePic();
                        btn_AC_up.show();
                    }
                    if (btnNr == 2) {
                        btn_AC_up.showAlternativePic();
                        btn_AC_down.show();
                    }
                    if (btnNr == 3) {
                        btn_AC_down.showAlternativePic();
                        btn_AC_ready.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == SLEEPTIMER) {
                if (s_sleepTimerSubMenue == 1) { // scroll backward (up, down, ready, cancel)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_SL_up.showAlternativePic();
                        btn_SL_down.show();
                    }
                    if (btnNr == 1) {
                        btn_SL_down.showAlternativePic();
                        btn_SL_ready.show();
                    }
                    if (btnNr == 2) {
                        btn_SL_ready.showAlternativePic();
                        btn_SL_cancel.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == SETTINGS) {
                if (s_settingsSubMenue == 1) { // scroll forward (bright, equal, radio)
                    if (btnNr > 0) btnNr--;
                    if (btnNr == 0) {
                        btn_SE_bright.showAlternativePic();
                        btn_SE_equal.show();
                    }
                    if (btnNr == 1) {
                        btn_SE_equal.showAlternativePic();
                        btn_SE_wifi.show();
                    }
                    if (btnNr == 2) {
                        btn_SE_wifi.showAlternativePic();
                        btn_SE_radio.show();
                    }
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == BRIGHTNESS) {
                if (s_brightnessSubMenue == 1) {
                    if (s_brightness > 5) s_brightness -= 5;
                    if (s_brightness < 5) s_brightness = 5;
                    sdr_BR_value.setValue(s_brightness);
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == EQUALIZER) {
                if (s_equalizerSubMenue == 1) {
                    char c[10];
                    if (btnNr == 10) {
                        if (s_toneBAL > -16) {
                            s_toneBAL--;
                            itoa(s_toneBAL, c, 10);
                            txt_EQ_balance.writeText(c);
                            sdr_EQ_balance.setValue(s_toneBAL);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // balance
                    if (btnNr == 20) {
                        if (s_toneLP > -40) {
                            s_toneLP--;
                            itoa(s_toneLP, c, 10);
                            txt_EQ_lowPass.writeText(c);
                            sdr_EQ_lowPass.setValue(s_toneLP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // LP
                    if (btnNr == 30) {
                        if (s_toneBP > -40) {
                            s_toneBP--;
                            itoa(s_toneBP, c, 10);
                            txt_EQ_bandPass.writeText(c);
                            sdr_EQ_bandPass.setValue(s_toneBP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // BP
                    if (btnNr == 40) {
                        if (s_toneHP > -40) {
                            s_toneHP--;
                            itoa(s_toneHP, c, 10);
                            txt_EQ_highPass.writeText(c);
                            sdr_EQ_highPass.setValue(s_toneHP);
                        }
                        webSrv.send("settone=", setI2STone());
                        setTimeCounter(2);
                        return;
                    } // HP
                    if (btnNr == 1) {
                        btnNr = 0;
                        btn_EQ_Player.show();
                        btn_EQ_Radio.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 2) {
                        btnNr = 1;
                        btn_EQ_mute.show();
                        btn_EQ_Player.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                }
            }
            break;
        case 13: // ARROW DOWN
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == STATIONSLIST) {
                lst_RADIO.nextStation();
                setTimeCounter(LIST_TIMER);
                break;
            } // station++
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == AUDIOFILESLIST) {
                lst_PLAYER.nextFile();
                setTimeCounter(LIST_TIMER);
                break;
            } // file++
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
              //        if(s_state == DLNAITEMSLIST){lst_DLNA.nextItem(); setTimeCounter(LIST_TIMER); return;} // item++
            if (s_state == CLOCK) {
                downvolume();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == SLEEPTIMER) {
                downvolume();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == EQUALIZER && s_equalizerSubMenue == 1) {
                if (btnNr == 10) {
                    btnNr = 0;
                    btn_EQ_Radio.showAlternativePic();
                    btn_EQ_balance.show();
                } else if (btnNr == 20) {
                    btnNr = 10;
                    btn_EQ_balance.showAlternativePic();
                    btn_EQ_lowPass.show();
                } else if (btnNr == 30) {
                    btnNr = 20;
                    btn_EQ_lowPass.showAlternativePic();
                    btn_EQ_bandPass.show();
                } else if (btnNr == 40) {
                    btnNr = 30;
                    btn_EQ_bandPass.showAlternativePic();
                    btn_EQ_highPass.show();
                }
                setTimeCounter(2);
                return;
            }
            break;
        case 14: // ARROW UP
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == STATIONSLIST) {
                lst_RADIO.prevStation();
                setTimeCounter(LIST_TIMER);
                break;
            } // station--
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == AUDIOFILESLIST) {
                lst_PLAYER.prevFile();
                setTimeCounter(LIST_TIMER);
                break;
            } // file-
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
              //            if(s_state == DLNAITEMSLIST){lst_DLNA.prevItem(); setTimeCounter(LIST_TIMER); return;} // item++
            if (s_state == CLOCK) {
                upvolume();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == SLEEPTIMER) {
                upvolume();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == EQUALIZER && s_equalizerSubMenue == 1) {
                if (btnNr < 10) {
                    btnNr = 10;
                    btn_EQ_balance.showAlternativePic();
                    btn_EQ_Radio.show();
                    btn_EQ_Player.show();
                    btn_EQ_mute.show();
                } else if (btnNr == 10) {
                    btnNr = 20;
                    btn_EQ_lowPass.showAlternativePic();
                    btn_EQ_balance.show();
                } else if (btnNr == 20) {
                    btnNr = 30;
                    btn_EQ_bandPass.showAlternativePic();
                    btn_EQ_lowPass.show();
                } else if (btnNr == 30) {
                    btnNr = 40;
                    btn_EQ_highPass.showAlternativePic();
                    btn_EQ_bandPass.show();
                }
                setTimeCounter(2);
                return;
            }
            break;
        case 15: // MODE
            if (s_state == SLEEPTIMER) {
                setStation(s_cur_station);
                s_radioSubMenue = 0;
                changeState(RADIO);
                break;
            } //  RADIO -> STATIONSLIST -> PLAYER -> DLNA -> CLOCK -> SLEEPTIMER
            if (s_state == RADIO) {
                changeState(STATIONSLIST);
                setTimeCounter(40);
                break;
            }
            if (s_state == STATIONSLIST) {
                s_playerSubMenue = 0;
                changeState(PLAYER);
                break;
            }
            if (s_state == PLAYER) {
                changeState(AUDIOFILESLIST);
                break;
            }
            if (s_state == AUDIOFILESLIST) {
                s_dlnaSubMenue = 0;
                changeState(DLNA);
                break;
            }
            if (s_state == DLNA) {
                s_clockSubMenue = 0;
                changeState(CLOCK);
                break;
            }
            if (s_state == CLOCK) {
                s_sleepTimerSubMenue = 0;
                changeState(SLEEPTIMER);
                break;
            }
            break;
        case 16: // OK
            if (s_state == RADIO) {
                if (s_radioSubMenue == 4) {
                    if (btnNr == 0) {
                        btn_RA_staList.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        changeState(STATIONSLIST);
                        setTimeCounter(LIST_TIMER);
                        break;
                    }
                    if (btnNr == 1) {
                        btn_RA_player.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_playerSubMenue = 2;
                        changeState(PLAYER);
                        setTimeCounter(2);
                        break;
                    }
                    if (btnNr == 2) {
                        btn_RA_dlna.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_dlnaSubMenue = 1;
                        changeState(DLNA);
                        setTimeCounter(2);
                        break;
                    }
                    if (btnNr == 3) {
                        btn_RA_clock.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_clockSubMenue = 2;
                        changeState(CLOCK);
                        setTimeCounter(2);
                        break;
                    }
                    if (btnNr == 4) {
                        btn_RA_sleep.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_sleepTimerSubMenue = 1;
                        changeState(SLEEPTIMER);
                        setTimeCounter(2);
                        break;
                    }
                    if (btnNr == 5) {
                        btn_RA_settings.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_settingsSubMenue = 1;
                        changeState(SETTINGS);
                        setTimeCounter(2);
                        break;
                    }
                    if (btnNr == 6) {
                        btn_RA_bt.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        changeState(BLUETOOTH);
                        break;
                    }
                    if (btnNr == 7) {
                        btn_RA_off.showClickedPic();
                        vTaskDelay(100);
                        fall_asleep();
                        break;
                    }
                } else {
                    s_radioSubMenue = 4;
                    btnNr = 0;
                    changeState(RADIO);
                }
                break;
            }
            if (s_state == STATIONSLIST) {
                setStationByNumber(lst_RADIO.getSelectedStation());
                s_radioSubMenue = 0;
                changeState(RADIO);
                break;
            }
            if (s_state == PLAYER) {
                if (s_playerSubMenue == 0) {
                    s_playerSubMenue = 2;
                    btnNr = 0;
                    changeState(PLAYER);
                    setTimeCounter(2);
                    break;
                }
                if (s_playerSubMenue == 1) {
                    s_playerSubMenue = 3;
                    btnNr = 0;
                    changeState(PLAYER);
                    setTimeCounter(2);
                    break;
                }
                if (s_playerSubMenue == 2) {
                    if (btnNr == 0) { // prev AudioFile
                        btn_PL_prevFile.showClickedPic();
                        vTaskDelay(50);
                        btn_PL_prevFile.showAlternativePic();
                        if (s_cur_AudioFileNr > 0) {
                            s_cur_AudioFileNr--;
                            showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                            showAudioFileNumber();
                            setTimeCounter(2);
                        }
                        return;
                    }
                    if (btnNr == 1) { // next AudioFile
                        btn_PL_nextFile.showClickedPic();
                        vTaskDelay(50);
                        btn_PL_nextFile.showAlternativePic();
                        if (s_cur_AudioFileNr + 1 < s_SD_content.getSize()) {
                            s_cur_AudioFileNr++;
                            showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                            showAudioFileNumber();
                            setTimeCounter(2);
                        }
                        return;
                    }
                    if (btnNr == 2) { // play file
                        btn_PL_ready.showClickedPic();
                        vTaskDelay(100);
                        stopSong();
                        SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                        btnNr = 0;
                        s_playerSubMenue = 3;
                        changeState(PLAYER);
                        showAudioFileNumber();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 3) { // play all files
                        btn_PL_playAll.showClickedPic();
                        s_f_shuffle = false;
                        preparePlaylistFromSDFolder(s_cur_AudioFolder);
                        processPlaylist(true);
                        s_playerSubMenue = 1;
                        changeState(PLAYER);
                        return;
                    }
                    if (btnNr == 4) { // shuffle and play all files
                        btn_PL_shuffle.showClickedPic();
                        vTaskDelay(100);
                        s_f_shuffle = true;
                        preparePlaylistFromSDFolder(s_cur_AudioFolder);
                        processPlaylist(true);
                        s_playerSubMenue = 1;
                        changeState(PLAYER);
                        return;
                    }
                    if (btnNr == 5) { // show file list
                        btn_PL_fileList.showClickedPic();
                        vTaskDelay(100);
                        s_SD_content.listFilesInDir(s_cur_AudioFolder, true, false);
                        changeState(AUDIOFILESLIST);
                        return;
                    }
                    if (btnNr == 6) { // back to radio
                        btn_PL_radio.showClickedPic();
                        vTaskDelay(100);
                        setStation(s_cur_station);
                        s_playerSubMenue = 0;
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                    if (btnNr == 7) { // off
                        btn_PL_off.showClickedPic();
                        vTaskDelay(100);
                        fall_asleep();
                        return;
                    }
                }
                if (s_playerSubMenue == 3) {
                    if (btnNr == 0) { // mute
                        btn_PL_mute.showClickedPic();
                        muteChanged(!s_f_mute);
                        vTaskDelay(100);
                        btn_PL_mute.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 1) { // pause
                        btn_PL_pause.showClickedPic();
                        if (s_f_isFSConnected) {
                            s_f_pauseResume = audio.pauseResume();
                            if (!audio.isRunning())
                                btn_PL_pause.setOn();
                            else
                                btn_PL_pause.setOff();
                        }
                        vTaskDelay(100);
                        btn_PL_pause.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 2) { // cancel
                        btn_PL_cancel.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_playerSubMenue = 2;
                        stopSong();
                        changeState(PLAYER);
                        return;
                    }
                    if (btnNr == 3) { // prev
                        btn_PL_playPrev.showClickedPic();
                        vTaskDelay(100);
                        btn_PL_playPrev.showAlternativePic();
                        s_cur_AudioFileNr = s_SD_content.getPrevAudioFile(s_cur_AudioFileNr);
                        SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                        showAudioFileNumber();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 4) { // next
                        btn_PL_playNext.showClickedPic();
                        vTaskDelay(100);
                        btn_PL_playNext.showAlternativePic();
                        s_cur_AudioFileNr = s_SD_content.getNextAudioFile(s_cur_AudioFileNr);
                        SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                        showAudioFileNumber();
                        setTimeCounter(2);
                        return;
                    }
                }
            }
            if (s_state == AUDIOFILESLIST) {
                const char* r = lst_PLAYER.getSelectedFile();
                if (r) {
                    stopSong();
                    SD_playFile(lst_PLAYER.getSelectedFilePath(), 0, true);
                    s_cur_AudioFileNr = lst_PLAYER.getSelectedFileNr();
                }
            }
            if (s_state == DLNA) {
                if (s_dlnaSubMenue == 0) {
                    s_dlnaSubMenue = 1;
                    btnNr = 0;
                    changeState(DLNA);
                    setTimeCounter(2);
                    break;
                }
                if (s_dlnaSubMenue == 1) {
                    if (btnNr == 0) { // mute
                        btn_DL_mute.showClickedPic();
                        muteChanged(!s_f_mute);
                        vTaskDelay(100);
                        btn_DL_mute.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 1) { // pause
                        if (!btn_DL_pause.getActive()) {
                            setTimeCounter(2);
                            return;
                        } // is inactive
                        btn_DL_pause.showClickedPic();
                        if (s_f_isWebConnected) {
                            s_f_pauseResume = audio.pauseResume();
                            if (!audio.isRunning())
                                btn_DL_pause.setOn();
                            else
                                btn_DL_pause.setOff();
                        }
                        vTaskDelay(100);
                        btn_DL_pause.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 2) { // cancel
                        btn_DL_cancel.showClickedPic();
                        vTaskDelay(100);
                        btn_DL_cancel.show();
                        stopSong();
                        txt_DL_fName.setText("");
                        txt_DL_fName.hide();
                        pgb_DL_progress.reset();
                        btn_DL_pause.setActive(false);
                        btn_DL_pause.show();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 3) { // dlna list
                        changeState(DLNAITEMSLIST);
                        txt_DL_fName.setText("");
                        return;
                    }
                    if (btnNr == 4) { // back to radio
                        btn_DL_radio.showClickedPic();
                        vTaskDelay(100);
                        setStation(s_cur_station);
                        s_playerSubMenue = 0;
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                }
                break;
            }
            // if(s_state == DLNAITEMSLIST) {setTimeCounter(2); const char* r = lst_DLNA.getSelectedURL();
            //                              if(r){txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(lst_DLNA.getSelectedTitle());
            //                                    s_dlnaSubMenue = 0; changeState(DLNA); connecttohost(r);}
            //                              break;
            // }
            if (s_state == CLOCK) {
                if (s_clockSubMenue == 0) {
                    s_clockSubMenue = 2;
                    btnNr = 0;
                    changeState(CLOCK);
                    setTimeCounter(2);
                    break;
                }
                if (s_clockSubMenue == 2) {
                    if (btnNr == 0) { // Alarm
                        s_alarmSubMenue = 1;
                        btnNr = 0;
                        btn_CL_alarm.showClickedPic();
                        vTaskDelay(100);
                        changeState(ALARMCLOCK);
                        return;
                    }
                    if (btnNr == 1) { // Radio
                        btn_CL_radio.showClickedPic();
                        vTaskDelay(100);
                        setStation(s_cur_station);
                        s_playerSubMenue = 0;
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                    if (btnNr == 2) { // Mute
                        btn_CL_mute.showClickedPic();
                        muteChanged(!s_f_mute);
                        vTaskDelay(100);
                        btn_CL_mute.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 3) { // Off
                        btn_CL_off.showClickedPic();
                        vTaskDelay(100);
                        fall_asleep();
                        return;
                    }
                }
            }
            if (s_state == ALARMCLOCK) {
                if (s_alarmSubMenue == 0) {
                    s_alarmSubMenue = 1;
                    btnNr = 0;
                    changeState(ALARMCLOCK);
                    setTimeCounter(2);
                    return;
                }
                if (s_alarmSubMenue == 1) {
                    if (btnNr == 0) { // pos left
                        btn_AC_left.showClickedPic();
                        clk_AC_red.shiftLeft();
                        btn_AC_left.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 1) { // pos right
                        btn_AC_right.showClickedPic();
                        clk_AC_red.shiftRight();
                        btn_AC_right.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 2) { // pos +1
                        btn_AC_up.showClickedPic();
                        clk_AC_red.digitUp();
                        btn_AC_up.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 3) { // pos -1
                        btn_AC_down.showClickedPic();
                        clk_AC_red.digitDown();
                        btn_AC_down.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 4) { // ready
                        btn_AC_ready.showClickedPic();
                        vTaskDelay(100);
                        updateSettings();
                        s_clockSubMenue = 0;
                        changeState(CLOCK);
                        logAlarmItems();
                        return;
                    }
                }
            }
            if (s_state == SLEEPTIMER) {
                if (s_sleepTimerSubMenue == 0) {
                    s_sleepTimerSubMenue = 1;
                    btnNr = 0;
                    changeState(SLEEPTIMER);
                    setTimeCounter(2);
                    return;
                }
                if (s_sleepTimerSubMenue == 1) {
                    if (btnNr == 0) {
                        btn_SL_up.showClickedPic();
                        display_sleeptime(1);
                        btn_SL_up.showAlternativePic();
                        return;
                    }
                    if (btnNr == 1) {
                        btn_SL_down.showClickedPic();
                        display_sleeptime(-1);
                        btn_SL_down.showAlternativePic();
                        return;
                    }
                    if (btnNr == 2) {
                        btn_SL_ready.showClickedPic();
                        vTaskDelay(100);
                        dispFooter.updateOffTime(s_sleeptime);
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                    if (btnNr == 3) {
                        btn_SL_cancel.showClickedPic();
                        vTaskDelay(100);
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                }
            }
            if (s_state == SETTINGS) {
                if (s_settingsSubMenue == 0) {
                    s_settingsSubMenue = 1;
                    btnNr = 0;
                    changeState(SETTINGS);
                    setTimeCounter(2);
                    return;
                }
                if (s_settingsSubMenue == 1) {
                    if (btnNr == 0) {
                        btn_SE_bright.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_brightnessSubMenue = 1;
                        changeState(BRIGHTNESS);
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 1) {
                        btn_SE_equal.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        s_equalizerSubMenue = 1;
                        changeState(EQUALIZER);
                        setTimeCounter(2);
                        return;
                    }
                    if (btnNr == 2) {
                        btn_SE_wifi.showClickedPic();
                        vTaskDelay(100);
                        btnNr = 0;
                        changeState(WIFI_SETTINGS);
                        return;
                    }
                    if (btnNr == 3) {
                        btn_SE_radio.showClickedPic();
                        vTaskDelay(100);
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    }
                }
            }
            if (s_state == BRIGHTNESS) {
                if (s_brightnessSubMenue == 0) {
                    s_brightnessSubMenue = 1;
                    btnNr = 0;
                    changeState(BRIGHTNESS);
                    setTimeCounter(2);
                    return;
                }
                if (s_brightnessSubMenue == 1) {
                    s_radioSubMenue = 0;
                    changeState(RADIO);
                    return;
                }
            }
            if (s_state == EQUALIZER) {
                if (s_equalizerSubMenue == 0) {
                    s_equalizerSubMenue = 1;
                    btnNr = 0;
                    changeState(EQUALIZER);
                    setTimeCounter(2);
                    return;
                }
                if (s_equalizerSubMenue == 1) {
                    if (btnNr == 0) {
                        btn_EQ_Radio.showClickedPic();
                        vTaskDelay(100);
                        setStation(s_cur_station);
                        s_radioSubMenue = 0;
                        changeState(RADIO);
                        return;
                    } // Radio
                    if (btnNr == 1) {
                        btn_EQ_Player.showClickedPic();
                        vTaskDelay(100);
                        s_playerSubMenue = 0;
                        changeState(PLAYER);
                        return;
                    } // Player
                    if (btnNr == 2) {
                        btn_EQ_mute.showClickedPic();
                        vTaskDelay(100);
                        muteChanged(!s_f_mute);
                        btn_EQ_mute.showAlternativePic();
                        setTimeCounter(2);
                        return;
                    } // Mute
                    if (btnNr == 10) {
                        btn_EQ_balance.showClickedPic();
                        s_toneBAL = 0;
                        txt_EQ_balance.writeText("0");
                        sdr_EQ_balance.setValue(s_toneLP);
                        webSrv.send("settone=", setI2STone());
                        btn_EQ_balance.showAlternativePic();
                    }
                    if (btnNr == 20) {
                        btn_EQ_lowPass.showClickedPic();
                        s_toneLP = 0;
                        txt_EQ_lowPass.writeText("0");
                        sdr_EQ_lowPass.setValue(s_toneLP);
                        webSrv.send("settone=", setI2STone());
                        btn_EQ_lowPass.showAlternativePic();
                    }
                    if (btnNr == 30) {
                        btn_EQ_bandPass.showClickedPic();
                        s_toneBP = 0;
                        txt_EQ_bandPass.writeText("0");
                        sdr_EQ_bandPass.setValue(s_toneLP);
                        webSrv.send("settone=", setI2STone());
                        btn_EQ_bandPass.showAlternativePic();
                    }
                    if (btnNr == 40) {
                        btn_EQ_highPass.showClickedPic();
                        s_toneHP = 0;
                        txt_EQ_highPass.writeText("0");
                        sdr_EQ_highPass.setValue(s_toneLP);
                        webSrv.send("settone=", setI2STone());
                        btn_EQ_highPass.showAlternativePic();
                    }
                }
                setTimeCounter(2);
                return;
            }
            break;
        case 18:
            if (s_state == PLAYER) {
                if (s_f_isFSConnected) s_f_pauseResume = audio.pauseResume();
            }
            break;
        case 19:
            if (s_state == PLAYER) {
                if (s_f_isFSConnected) audio.stopSong();
                s_playerSubMenue = 0;
                changeState(PLAYER);
            }
            break;
        case 20:
            s_f_irOnOff = !s_f_irOnOff;
            if (s_f_irOnOff)
                fall_asleep();
            else
                wake_up();
            break;
        case 21:
            if (s_state != RADIO) {
                s_radioSubMenue = 0;
                setStation(s_cur_station);
                changeState(RADIO);
            }
            break;
        case 22:
            if (s_state != PLAYER) {
                s_playerSubMenue = 0;
                changeState(PLAYER);
            }
            break;
        case 23:
            if (s_state != DLNA) {
                s_dlnaSubMenue = 0;
                changeState(DLNA);
            }
            break;
        case 24:
            if (s_state != CLOCK) {
                s_clockSubMenue = 0;
                changeState(CLOCK);
            }
            break;
        case 25:
            if (s_state != SLEEPTIMER) {
                s_sleepTimerSubMenue = 0;
                changeState(SLEEPTIMER);
            }
            break;
        case 26: // VOLUME+
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == CLOCK) {
                upvolume();
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == SLEEPTIMER) {
                upvolume();
                setTimeCounter(2);
                break;
            } // VOLUME++
            upvolume();
            break;
        case 27: // VOLUME-
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == CLOCK) {
                downvolume();
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == SLEEPTIMER) {
                downvolume();
                setTimeCounter(2);
                break;
            } // VOLUME--
            downvolume();
            break;
        case 28:
            if (s_state == PLAYER) {
                if (audio.isRunning()) audio.setTimeOffset(-30);
            }
            break;
        case 29:
            if (s_state == PLAYER) {
                if (audio.isRunning()) audio.setTimeOffset(+30);
            }
            break;
        case 30: nextStation(); break;
        case 31: prevStation(); break;
        default: break;
    }
}
void ir_long_key(int8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "long pressed key nr: " ANSI_ESC_BLUE "%02i", key);
    if (key == 16) fall_asleep(); // long OK
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Event from TouchPad
// clang-format off
void tp_pressed(uint16_t x, uint16_t y) {
    //  SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint  x=%d, y=%d", x, y);
    if(s_f_sleeping) return;  // awake in tp_released()
    const char* objName = NULL;
    if(s_state == RADIO && y > layout.winHeader.y + layout.winHeader.h && y < layout.sdrOvBtns.y){
        objName = "backpane";
        s_radioSubMenue++;
        if(s_radioSubMenue == 3) s_radioSubMenue = 0;
        changeState(RADIO);
        goto exit;
    }
    objName = isObjectClicked(x, y);
exit:
    if(objName){
        SerialPrintfln("click on ..  %s", objName);
    }
    return;
}
void tp_long_pressed(uint16_t x, uint16_t y){

    // if(s_f_muteIsPressed) {
    //     if(!s_f_mute){
    //         fall_asleep();
    //     }
    //     else{
    //         muteChanged(false);
    //     }
    //     return;
    // }

    if(s_state == DLNAITEMSLIST){
    //    lst_DLNA.longPressed(x, y);
    }
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_released(uint16_t x, uint16_t y){

    if(s_f_sleeping){ wake_up(); return;}   // if sleeping

    // all state
    dispHeader.released();
    dispFooter.released();

    switch(s_state){
        case RADIO:
            if(s_radioSubMenue == 0){ VUmeter_RA.released();}
            if(s_radioSubMenue == 1){ sdr_RA_volume.released(); btn_RA_mute.released(); btn_RA_prevSta.released(); btn_RA_nextSta.released();}
            if(s_radioSubMenue == 2){ btn_RA_player.released(); btn_RA_dlna.released(); btn_RA_clock.released(); btn_RA_sleep.released(); btn_RA_settings.released();
                                     btn_RA_bt.released(); btn_RA_off.released(); btn_RA_staList.released();}
            if(s_radioSubMenue == 4){ btn_RA_player.released(); btn_RA_dlna.released(); btn_RA_clock.released(); btn_RA_sleep.released(); btn_RA_settings.released();
                                     btn_RA_bt.released(); btn_RA_off.released(); btn_RA_staList.released();}
            break;
        case STATIONSLIST:
            lst_RADIO.released();
            break;
        case PLAYER:
            if(s_playerSubMenue == 0){btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released(); btn_PL_fileList.released();
                                     btn_PL_radio.released(); btn_PL_off.released();}
            if(s_playerSubMenue == 1){btn_PL_mute.released(); btn_PL_pause.released(); btn_PL_cancel.released(); sdr_PL_volume.released(); btn_PL_playNext.released(); btn_PL_playPrev.released();
                                     pgb_PL_progress.released();}
            if(s_playerSubMenue == 2){btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released(); btn_PL_fileList.released();
                                     btn_PL_radio.released(); btn_PL_off.released();}
            if(s_playerSubMenue == 3){btn_PL_mute.released(); btn_PL_pause.released(); btn_PL_cancel.released(); sdr_PL_volume.released(); btn_PL_playNext.released(); btn_PL_playPrev.released();
                                     pgb_PL_progress.released();}
            break;
        case AUDIOFILESLIST:
            lst_PLAYER.released(x, y);
            break;
        case DLNA:
            sdr_DL_volume.released(); btn_DL_mute.released(); btn_DL_pause.released(); btn_DL_radio.released(); btn_DL_fileList.released(); btn_DL_cancel.released(); pgb_DL_progress.released();
            break;
        case DLNAITEMSLIST:
            lst_DLNA.released(x, y);
            break;
        case CLOCK:
            btn_CL_mute.released(); btn_CL_alarm.released(); btn_CL_radio.released(); clk_CL_12.released();  clk_CL_24.released(); sdr_CL_volume.released(); btn_CL_off.released();
            break;
        case ALARMCLOCK:
            clk_AC_red.released(); btn_AC_left.released(); btn_AC_right.released(); btn_AC_up.released(); btn_AC_down.released(); btn_AC_ready.released();
            break;
        case SLEEPTIMER:
            btn_SL_up.released(); btn_SL_down.released(); btn_SL_ready.released(); btn_SL_cancel.released();
            break;
        case SETTINGS:
            btn_SE_bright.released(); btn_SE_equal.released();  btn_SE_wifi.released(); btn_SE_radio.released();
            break;
        case BRIGHTNESS:
            sdr_BR_value.released();  btn_BR_ready.released(); pic_BR_logo.released();
            break;
        case EQUALIZER:
            sdr_EQ_lowPass.released(); sdr_EQ_bandPass.released(); sdr_EQ_highPass.released(); sdr_EQ_balance.released(); btn_EQ_lowPass.released(); btn_EQ_bandPass.released();
            btn_EQ_highPass.released(); btn_EQ_balance.released(); txt_EQ_lowPass.released(); txt_EQ_bandPass.released(); txt_EQ_highPass.released(); txt_EQ_balance.released();
            btn_EQ_Radio.released(); btn_EQ_Player.released(); btn_EQ_mute.released();
            break;
        case BLUETOOTH:
            btn_BT_pause.released(); btn_BT_radio.released(); btn_BT_volDown.released(); btn_BT_volUp.released(); btn_BT_mode.released(); btn_BT_power.released();
            break;
        case IR_SETTINGS:
            btn_IR_radio.released();
            break;
        case WIFI_SETTINGS:
            cls_wifiSettings.released();
            break;
        default:
            break;
    }
    // SerialPrintfln("tp_released, state is: %i", s_state);
}

void tp_long_released(uint16_t x, uint16_t y){
//    log_w("long released)");
//    if(s_state == DLNAITEMSLIST) {lst_DLNA.longReleased();}
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_moved(uint16_t x, uint16_t y){
    if(s_state == RADIO){
        if(sdr_RA_volume.positionXY(x, y)) return;
    }
    if(s_state == STATIONSLIST){
        if(lst_RADIO.positionXY(x, y)) return;
    }
    if(s_state == PLAYER){
        if(sdr_PL_volume.positionXY(x, y)) return;
    }
    if(s_state == AUDIOFILESLIST){
        if(lst_PLAYER.positionXY(x, y)) return;
    }
    if(s_state == DLNA){
        if(sdr_DL_volume.positionXY(x, y)) return;
    }
    if(s_state == CLOCK){
        if(sdr_CL_volume.positionXY(x, y)) return;
    }
    if(s_state == DLNAITEMSLIST){
        if(lst_DLNA.positionXY(x, y)) return;
    }
    if(s_state == BRIGHTNESS){
        if(sdr_BR_value.positionXY(x, y)) return;
    }
    if(s_state == EQUALIZER){
        if(sdr_EQ_lowPass.positionXY(x, y)) return;
        if(sdr_EQ_bandPass.positionXY(x, y)) return;
        if(sdr_EQ_highPass.positionXY(x, y)) return;
        if(sdr_EQ_balance.positionXY(x, y)) return;
    }
    if(s_state == IR_SETTINGS){
        if(btn_IR_radio.positionXY(x, y)) return;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//Events from websrv
void WEBSRV_onCommand(const char* cmd, const String param, const String arg){  // called from html

    if(CORE_DEBUG_LEVEL == ARDUHAL_LOG_LEVEL_DEBUG){
        SerialPrintfln("WS_onCmd:    " ANSI_ESC_YELLOW "cmd=\"%s\", params=\"%s\", arg=\"%s\"", cmd, param.c_str(), arg.c_str());
    }
    #define CMD_EQUALS(x) if(strcmp(cmd, x) == 0)
    #define CMD_STARTS_WITH(x) if(strncmp(cmd, x, strlen(x)) == 0)

    CMD_EQUALS("ping"){                 webSrv.send("pong"); return;}                                                                                     // via websocket

    CMD_EQUALS("index.html"){           SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "index.html");                                                     // via XMLHttpRequest
                                        webSrv.show(index_html, webSrv.TEXT);
                                        return;}

    CMD_EQUALS("index.js"){             SerialPrintfln("Script:      " ANSI_ESC_ORANGE "index.js");                                                       // via XMLHttpRequest
                                        webSrv.show(index_js, webSrv.JS); return;}

    CMD_EQUALS("favicon.ico"){          webSrv.streamfile(SD_MMC, "/favicon.ico"); return;}                                                               // via XMLHttpRequest

    CMD_EQUALS("test"){                 sprintf(s_chbuf, "free heap: %lu, Inbuff filled: %lu, Inbuff free: %lu, PSRAM filled %lu, PSRAM free %lu",
                                            (long unsigned)ESP.getFreeHeap(), (long unsigned)audio.inBufferFilled(), (long unsigned)audio.inBufferFree(),
                                            (long unsigned) (ESP.getPsramSize() - ESP.getFreePsram()), (long unsigned)ESP.getFreePsram());
                                        webSrv.send("test=", s_chbuf);
                                        SerialPrintfln("audiotask .. stackHighWaterMark: %lu bytes", (long unsigned)audio.getHighWatermark() * 4);
                                        SerialPrintfln("looptask ... stackHighWaterMark: %lu bytes", (long unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
                                        return;}

    CMD_EQUALS("getmute"){              if(s_f_mute) webSrv.send("mute=", "1");
                                        else        webSrv.send("mute=", "0");
                                        return;}

    CMD_EQUALS("setmute"){              muteChanged(!s_f_mute); return;}

    CMD_EQUALS("upvolume"){             webSrv.send("volume=", int2str(upvolume()));  return;}                                                            // via websocket
    CMD_EQUALS("downvolume"){           webSrv.send("volume=", int2str(downvolume())); return;}                                                           // via websocket

    CMD_EQUALS("getVolumeSteps"){       webSrv.send("volumeSteps=", int2str(s_volume.volumeSteps)); return;}

    CMD_EQUALS("setVolumeSteps"){       s_volume.cur_volume = map_l(s_volume.cur_volume, 0, s_volume.volumeSteps, 0, param.toInt());
                                        s_volume.ringVolume = map_l(s_volume.ringVolume, 0, s_volume.volumeSteps, 0, param.toInt()); webSrv.send("ringVolume=", int2str(s_volume.ringVolume));
                                        s_volume.volumeAfterAlarm = map_l(s_volume.volumeAfterAlarm, 0, s_volume.volumeSteps, 0, param.toInt()); webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm));
                                        s_volume.volumeSteps = param.toInt(); webSrv.send("volumeSteps=", param); audio.setVolumeSteps(s_volume.volumeSteps);
                                        // log_w("s_volumeSteps  %i", s_volumeSteps);
                                        sdr_CL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_DL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_PL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_RA_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        setVolume(s_volume.cur_volume);
                                        SerialPrintfln("action: ...  new volume steps: " ANSI_ESC_CYAN "%d", s_volume.volumeSteps);
                                        return;}

    CMD_EQUALS("getRingVolume"){        webSrv.send("ringVolume=", int2str(s_volume.ringVolume)); return;}
    CMD_EQUALS("setRingVolume"){        s_volume.ringVolume = param.toInt(); webSrv.send("ringVolume=", int2str(s_volume.ringVolume));
                                        SerialPrintfln("action: ...  new ring volume: " ANSI_ESC_CYAN "%d", s_volume.ringVolume); return;}

    CMD_EQUALS("getVolAfterAlarm"){     webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm)); return;}
    CMD_EQUALS("setVolAfterAlarm"){     s_volume.volumeAfterAlarm = param.toInt(); webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm));
                                        SerialPrintfln("action: ...  new volume after alarm: " ANSI_ESC_CYAN "%d", s_volume.volumeAfterAlarm); return;}

    CMD_EQUALS("homepage"){             webSrv.send("homepage=", s_homepage);
                                        return;}

    CMD_EQUALS("to_listen"){            StationsItems(); // via websocket, return the name and number of the current station
                                        return;}

    CMD_EQUALS("gettone"){              webSrv.send("settone=", setI2STone());
                                        return;}

    CMD_EQUALS("getstreamtitle"){       webSrv.reply(s_streamTitle, webSrv.TEXT);
                                        return;}

    CMD_EQUALS("LowPass"){              s_toneLP = param.toInt();                           // audioI2S tone
                                        char lp[30] = "Lowpass set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                        webSrv.send("tone=", lp); setI2STone(); return;}

    CMD_EQUALS("BandPass"){             s_toneBP = param.toInt();                           // audioI2S tone
                                        char bp[30] = "Bandpass set to "; strcat(bp, param.c_str()); strcat(bp, "dB");
                                        webSrv.send("tone=", bp); setI2STone(); return;}

    CMD_EQUALS("HighPass"){             s_toneHP = param.toInt();                           // audioI2S tone
                                        char hp[30] = "Highpass set to "; strcat(hp, param.c_str()); strcat(hp, "dB");
                                        webSrv.send("tone=", hp); setI2STone(); return;}

    CMD_EQUALS("Balance"){              s_toneBAL = param.toInt();
                                        char bal[30] = "Balance set to "; strcat(bal, param.c_str());
                                        webSrv.send("tone=", bal); setI2STone(); return;}

    CMD_EQUALS("uploadfile"){           s_filename = param; return;}

    CMD_EQUALS("prev_station"){         prevFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("next_station"){         nextFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("set_station"){          setStationByNumber(param.toInt()); return;}                                                                          // via websocket

    CMD_EQUALS("stationURL"){           setStationViaURL(param.c_str(), arg.c_str()); x_ps_free(&s_stationName_air);                                          // via websocket
                                        s_stationName_air = x_ps_strndup(param.c_str(), 200); // set max length
                                        SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", param.c_str());
                                        s_f_newStationName = true; return;}

    CMD_EQUALS("webFileURL"){           audio.connecttohost(param.c_str())? s_playerSubMenue = 1 : s_playerSubMenue = 0; changeState(PLAYER); return;}          // via websocket

    CMD_EQUALS("getnetworks"){          webSrv.send("networks=", WiFi.SSID()); return;}                                                  // via websocket

    CMD_EQUALS("get_tftSize"){          webSrv.send("tftSize=", displayConfig.tftSize); return;};

    CMD_EQUALS("getTimeZones"){         webSrv.send("timezones=", timezones_json); return;}

    CMD_EQUALS("setTimeZone"){          s_TZName = param;  s_TZString = arg;
                                        SerialPrintfln("Timezone: .. " ANSI_ESC_BLUE "%s, %s", param.c_str(), arg.c_str());
                                        setRTC(s_TZString.c_str());
                                        updateSettings(); // write new TZ items to settings.json
                                        return;}

    CMD_EQUALS("getTimeZoneName"){      webSrv.reply(s_TZName, webSrv.TEXT); return;}

    CMD_EQUALS("change_state"){         if     (!strcmp(param.c_str(), "RADIO")       && s_state != RADIO)       {setStation(s_cur_station); s_radioSubMenue = 0; changeState(RADIO); return;}
                                        else if(!strcmp(param.c_str(), "PLAYER")      && s_state != PLAYER)      {stopSong(); s_playerSubMenue = 0; changeState(PLAYER); return;}
                                        else if(!strcmp(param.c_str(), "DLNA")        && s_state != DLNA)        {stopSong(); s_dlnaSubMenue = 0; changeState(DLNA);   return;}
                                        else if(!strcmp(param.c_str(), "BLUETOOTH")   && s_state != BLUETOOTH)   {changeState(BLUETOOTH); return;}
                                        else if(!strcmp(param.c_str(), "IR_SETTINGS") && s_state != IR_SETTINGS) {changeState(IR_SETTINGS); return;}
                                        else return;}

    CMD_EQUALS("stopfile"){             if(!s_f_isFSConnected && !s_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        s_playerSubMenue = 0; stopSong(); changeState(PLAYER); webSrv.send("stopfile=", "audiofile stopped");
                                        return;}

    CMD_EQUALS("pause_resume"){         if(!s_f_isFSConnected && !s_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        s_f_pauseResume = audio.pauseResume();
                                        if(audio.isRunning()){webSrv.send("resumefile=", "audiofile resumed"); btn_PL_pause.setOff(); btn_PL_pause.show();}
                                        else {                webSrv.send("resumefile=", "audiofile paused");  btn_PL_pause.setOn(); btn_PL_pause.show();}
                                        return;}

    CMD_EQUALS("get_alarmdays"){        webSrv.send("alarmdays=", String(s_alarmdays, 10)); return;}

    CMD_EQUALS("set_alarmdays"){        s_alarmdays = param.toInt(); updateSettings(); return;}

    CMD_EQUALS("get_alarmtime"){        return;} // not used yet

    CMD_EQUALS("set_alarmtime"){        return;}

    CMD_EQUALS("get_timeAnnouncement"){ if(s_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "1");
                                        if(  !s_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "0");
                                        return;}

    CMD_EQUALS("set_timeAnnouncement"){ if(param == "true" ) {s_f_timeAnnouncement = true;}
                                        if(   param == "false") {s_f_timeAnnouncement = false;}
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "hourly time announcement " ANSI_ESC_BLUE "%s", (s_f_timeAnnouncement == 1) ? "on" : "off");
                                        return;}

    CMD_EQUALS("getTimeSpeechLang"){    webSrv.send("getTimeSpeechLang=", String(s_timeSpeechLang, 10)); return;}

    CMD_EQUALS("setTimeSpeechLang"){    if(param.length() > 2){log_e("set_timeSpeechLang too long %s", param.c_str()); return;}
                                        strcpy(s_timeSpeechLang, param.c_str());
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "language is " ANSI_ESC_BLUE "%s", param.c_str());
                                        return;}

    CMD_EQUALS("DLNA_getServer")  {     webSrv.send("DLNA_Names=", dlna.stringifyServer()); s_currDLNAsrvNr = -1; return;}

    CMD_EQUALS("DLNA_getRoot")    {     s_currDLNAsrvNr = param.toInt(); dlna.browseServer(s_currDLNAsrvNr, "0"); return;}

    CMD_EQUALS("DLNA_getContent") {     if(param.startsWith("http")) {connecttohost(param.c_str()); showFileName(arg.c_str()); return;}
                                        x_ps_free(&s_dlnaHistory[s_dlnaLevel].objId); s_dlnaHistory[s_dlnaLevel].objId = x_ps_strdup(param.c_str());
                                        s_totalNumberReturned = 0;
                                        dlna.browseServer(s_currDLNAsrvNr, s_dlnaHistory[s_dlnaLevel].objId);
                                        return;}

    CMD_STARTS_WITH("SD/"){             if(!webSrv.streamfile(SD_MMC, scaleImage(cmd + 2))){ // without "SD"                                               // via XMLHttpRequest
                                            SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "The file could not be transferred " ANSI_ESC_RED "\"%s\"", cmd + 2);
                                            webSrv.sendStatus(404);} // not found
                                        //    webSrv.streamfile(SD_MMC, scaleImage("/common/unknown.jpg"));}
                                        return;}

    CMD_EQUALS("SD_Download"){          webSrv.streamfile(SD_MMC, param.c_str());                                                                         // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Load from SD  " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        return;}

    CMD_EQUALS("SD_GetFolder"){         webSrv.reply(s_SD_content.stringifyDirContent(param), webSrv.JS);                                                           // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "GetFolder " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        return;}

    CMD_EQUALS("SD_newFolder"){         bool res = SD_newFolder(param.c_str());                                                                           // via XMLHttpRequest
                                        if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "NewFolder " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        return;}

    CMD_EQUALS("SD_playFile"){          stopSong();
                                        webSrv.reply("SD_playFile=" + param, webSrv.TEXT);                                                                // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        SD_playFile(param.c_str());
                                        return;}

    CMD_EQUALS("SD_playAllFiles"){      stopSong();
                                        webSrv.send("SD_playFolder=", "" + param);                                                                        // via websocket
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play Folder" ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        preparePlaylistFromSDFolder(param.c_str()); processPlaylist(true);
                                        s_playerSubMenue = 1;
                                        changeState(PLAYER);
                                        return;}

    CMD_EQUALS("SD_rename"){            String arg1 = arg.substring(0, arg.indexOf("&")); // only the first argument is used                              // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Rename " ANSI_ESC_ORANGE "old \"%s\" new \"%s\"",
                                        param.c_str(), arg1.c_str());
                                        bool res = SD_rename(param.c_str(), arg1.c_str());
                                        if(res) webSrv.reply("refresh", webSrv.TEXT);
                                        else webSrv.sendStatus(400);
                                        return;}

    CMD_EQUALS("setIRcmd"){             int32_t command = (int32_t)strtol(param.c_str(), NULL, 16);
                                        int32_t btnNr = (int32_t)strtol(arg.c_str(), NULL, 10);
                                        SerialPrintfln("set_IR_cmd:  " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02lx, "
                                        ANSI_ESC_YELLOW "IR Button Number " ANSI_ESC_BLUE "%02li", (long signed)command, (long signed)btnNr);
                                        ir.set_irButtons(btnNr,  command);
                                        s_settings.irbuttons[btnNr].val = command;
                                        return;}

    CMD_EQUALS("setIRadr"){             SerialPrintfln("set_IR_adr:  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "%s",
                                        param.c_str());
                                        int32_t address = (int32_t)strtol(param.c_str(), NULL, 16);
                                        ir.set_irAddress(address);
                                        s_settings.irbuttons[42].val = address;
                                        return;}

    CMD_EQUALS("getTimeFormat"){        webSrv.send("timeFormat=", String(s_timeFormat, 10));
                                        return;}

    CMD_EQUALS("setTimeFormat"){        s_timeFormat = param.toInt();
                                        if(s_state == CLOCK){clearWithOutHeaderFooter();}
                                        SerialPrintfln("TimeFormat:  " ANSI_ESC_YELLOW "new time format: " ANSI_ESC_BLUE "%sh", param.c_str());
                                        return;}

    CMD_EQUALS("getSleepMode"){         webSrv.send("sleepMode=", String(s_sleepMode, 10)); return;}

    CMD_EQUALS("setSleepMode"){         s_sleepMode = param.toInt();
                                        if(s_sleepMode == 0) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Display off");
                                        if(s_sleepMode == 1) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Show the time");
                                        return;}

    CMD_EQUALS("DLNA_GetFolder"){       webSrv.sendStatus(306); return;}  // todo
    CMD_EQUALS("KCX_BT_connected") {    if(!s_bt_emitter.power_state)     webSrv.send("KCX_BT_connected=", "-1");
                                        else if(bt_emitter.isConnected()) webSrv.send("KCX_BT_connected=",  "1");
                                        else                              webSrv.send("KCX_BT_connected=",  "0");
                                        return;}
    CMD_EQUALS("KCX_BT_clearItems"){    bt_emitter.deleteVMlinks(); return;}
    CMD_EQUALS("KCX_BT_addName"){       bt_emitter.addLinkName(param.c_str()); return;}
    CMD_EQUALS("KCX_BT_addAddr"){       bt_emitter.addLinkAddr(param.c_str()); return;}
    CMD_EQUALS("KCX_BT_mem"){           bt_emitter.getVMlinks(); return;}
    CMD_EQUALS("KCX_BT_scanned"){       webSrv.send("KCX_BT_SCANNED=", bt_emitter.stringifyScannedItems()); return;}
    CMD_EQUALS("KCX_BT_getMode"){       webSrv.send("KCX_BT_MODE=", bt_emitter.getMode().c_get()); return;}
    CMD_EQUALS("KCX_BT_changeMode"){    webSrv.send("KCX_BT_connected=", "0");
                                        bt_emitter.changeMode();
                                        webSrv.send("KCX_BT_MODE=", bt_emitter.getMode().c_get()); return;}
    CMD_EQUALS("KCX_BT_pause"){         bt_emitter.pauseResume(); return;}
    CMD_EQUALS("KCX_BT_downvolume"){    bt_emitter.downvolume(); return;}
    CMD_EQUALS("KCX_BT_upvolume"){      bt_emitter.upvolume();   return;}
    CMD_EQUALS("KCX_BT_getPower"){      if(s_bt_emitter.power_state) webSrv.send("KCX_BT_power=", "1"); else webSrv.send("KCX_BT_power=", "0"); return;}
    CMD_EQUALS("KCX_BT_power"){         s_bt_emitter.power_state == 1 ? bt_emitter.power_off() : bt_emitter.power_on() ; return;}

    CMD_EQUALS("hardcopy"){             SerialPrintfln("Webpage: ... " ANSI_ESC_YELLOW "create a display hardcopy"); make_hardcopy_on_sd(); webSrv.send("hardcopy=", "/hardcopy.bmp"); return;}

    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param.c_str());
    webSrv.sendStatus(400);
}

void WEBSRV_onRequest(const char* cmd,  const char* param, const char* arg, const char* contentType, uint32_t contentLength){
    // log_w("cmd %s, param %s, arg %s, ct %s, cl %i", cmd, param, arg, contentType, contentLength);
    if(strcmp(cmd, "SD_Upload") == 0) {savefile(param, contentLength); // PC --> SD
                                       if(strcmp(param, "/stations.json") == 0) staMgnt.updateStationsList();
                                       return;}
    if(strcmp(cmd, "uploadfile") == 0){saveImage(param, contentLength); return;}
    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param);
    webSrv.sendStatus(400);
}

void WEBSRV_onDelete(const char* cmd,  const char* param, const char* arg){  // via XMLHttpRequest
    if(startsWith(cmd, "SD")){      bool res = SD_delete(param);
                                    if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Delete " ANSI_ESC_ORANGE "\"%s\"", param);
                                    return;}
    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param);
    webSrv.sendStatus(400);
}
// clang-format on

void WEBSRV_onInfo(const char* info) {
    if (startsWith(info, "WebSocket")) return;      // suppress WebSocket client available
    if (!strcmp("ping", info)) return;              // suppress ping
    if (!strcmp("to_listen", info)) return;         // suppress to_isten
    if (startsWith(info, "Command client")) return; // suppress Command client available
    if (startsWith(info, "Content-D")) return;      // Content-Disposition
    if (startsWith(info, "test=")) return;          // stackHighWaterMark
    // SerialPrintfln("WebSrv Info: " ANSI_ESC_YELLOW "%s", info); // infos for debug
}

void WEBSRV_onError(const char* info) {
    SerialPrintfln("WebSrv Err:  " ANSI_ESC_RED "%s", info);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  Events from DLNA
void on_dlna_client(const DLNA_Client::msg_s& msg) {
    if (msg.e == DLNA_Client::evt_content) {
        if (!msg.items) return; // security check
        for (size_t i = 0; i < msg.items->size(); i++) {
            const auto& item = msg.items->at(i);
            if (item.isAudio) {
                if (item.duration.equals("?") != 0) { // no duration given
                    if (item.itemSize) {
                        SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW " %s, itemSize %ld", item.title.c_get(), (long unsigned int)item.itemSize);
                    } else {
                        SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s", item.title.c_get());
                    }
                } else {
                    SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, duration %s", item.title.c_get(), item.duration.c_get());
                }
            }
            if (item.childCount) {
                SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, childCount %i", item.title.c_get(), item.childCount);
            } else {
                SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, childCount 0", item.title.c_get());
            }
        }
        if (msg.totalMatches >= 0) SerialPrintfln("DLNA_server: returned %i from %i", msg.numberReturned, msg.totalMatches);
        s_dlnaMaxItems = msg.totalMatches;
        s_totalNumberReturned += msg.numberReturned;
        if (msg.numberReturned == 50 && !s_f_dlnaMakePlaylistOTF) { // next round
            if (s_totalNumberReturned < msg.totalMatches && s_totalNumberReturned < 500) { s_f_dlnaBrowseServer = true; }
        }
        if (s_f_dlnaWaitForResponse) {
            s_f_dlnaWaitForResponse = false;
            lst_DLNA.show(s_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &s_dlnaLevel, s_dlnaMaxItems);
            setTimeCounter(LIST_TIMER);
        } else {
            webSrv.send("dlnaContent=", dlna.stringifyContent());
        }
        if (s_totalNumberReturned == msg.totalMatches || s_totalNumberReturned == 500 || s_f_dlnaMakePlaylistOTF) {
            s_totalNumberReturned = 0;
            s_f_dlna_browseReady = true; // last item received
        }
    }
    if (msg.e == DLNA_Client::evt_server) {
        for (size_t i = 0; i < msg.server->size(); i++) {
            const auto& server = msg.server->at(i);
            SerialPrintfln("DLNA_server: [%d] " ANSI_ESC_CYAN "%s:%d " ANSI_ESC_YELLOW " %s", i, server.ip.c_get(), server.port, server.friendlyName.c_get());
        }
        SerialPrintfln("DLNA_server: %i media server found", msg.server->size());
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void on_kcx_bt_emitter(const KCX_BT_Emitter::msg_s& msg) {
    if(msg.e == KCX_BT_Emitter::evt_info){
        SerialPrintfln("BT-Emitter:  " ANSI_ESC_GREEN "%s ", msg.arg);
    }
    if (msg.e == KCX_BT_Emitter::evt_found) {
        s_bt_emitter.found = true;
        SerialPrintfln("BT-Emitter:  %s ", "found");
    }
    if (msg.e == KCX_BT_Emitter::evt_connect) {
        s_bt_emitter.connect = true;
        webSrv.send("KCX_BT_connected=", "1");
        if (s_bt_emitter.mode.equals("TX")) {
            pic_BT_mode.setPicturePath("/common/BTgold.png");
        } else {
            pic_BT_mode.setPicturePath("/common/BTblue.png");
        }
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        SerialPrintfln("BT-Emitter:  %s ", "connected");
    }
    if (msg.e == KCX_BT_Emitter::evt_disconnect) {
        webSrv.send("KCX_BT_connected=", "0");
        pic_BT_mode.setPicturePath("/common/BTnc.png"); // not connected
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        SerialPrintfln("BT-Emitter:  %s ", "disconnected");
    }
    if (msg.e == KCX_BT_Emitter::evt_reset) {
        SerialPrintfln("BT-Emitter:  %s ", "reset");
        s_bt_emitter.connect = false;
    }
    if (msg.e == KCX_BT_Emitter::evt_power_on) {
        s_bt_emitter.power_state = true;
        webSrv.send("KCX_BT_power=", "1");
        btn_BT_power.setValue(true);
        pic_BT_mode.setPicturePath("/common/BTnc.png");
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        SerialPrintfln("BT-Emitter:  %s ", "power on");
        bt_emitter.userCommand("AT+BT_MODE?");
    }
    if (msg.e == KCX_BT_Emitter::evt_power_off) {
        s_bt_emitter.power_state = false;
        webSrv.send("KCX_BT_power=", "0");
        webSrv.send("KCX_BT_connected=", "-1");
        btn_BT_power.setValue(false);
        pic_BT_mode.setPicturePath("/common/BToff.png");
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        SerialPrintfln("BT-Emitter:  %s ", "power off");
    }
    if (msg.e == KCX_BT_Emitter::evt_scan) {
        s_bt_emitter.connect = false;
        SerialPrintfln("BT-Emitter:  %s ", "scan ...");
    }
    if (msg.e == KCX_BT_Emitter::evt_volume) {
        s_bt_emitter.volume = msg.val;
        char v[5];
        itoa(s_bt_emitter.volume, v, 10);
        txt_BT_volume.writeText(v);
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%i", "volume", msg.val);
    }
    if (msg.e == KCX_BT_Emitter::evt_version) {
        s_bt_emitter.version = msg.arg;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", "version", msg.arg);
    }
    if (msg.e == KCX_BT_Emitter::evt_mode) {
        webSrv.send("KCX_BT_MODE=", msg.arg);
        if (s_state == BLUETOOTH) txt_BT_mode.writeText(msg.arg[0] == 'R' ? "RECEIVER" : "EMITTER");
        s_bt_emitter.mode = msg.arg;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", "RX_TX_mode", msg.arg);
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void kcx_bt_memItems(const char* jsonItems) { // Every time an item (name or address) was added, a JSON string is passed here
    // SerialPrintfln("bt_memItems %s", jsonItems);
    webSrv.send("KCX_BT_MEM=", jsonItems);
}

void kcx_bt_scanItems(const char* jsonItems) { // Every time an item (name and address) was scanned, a JSON string is passed here
    // SerialPrintfln("bt_scanItems %s", jsonItems);
    webSrv.send("KCX_BT_SCANNED=", jsonItems);
}

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// clang-format off
void graphicObjects_OnChange(const char* name, int32_t arg1) {
    char c[10];
    if(strcmp(name, "sdr_RA_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_PL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_DL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_CL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_BR_value") == 0)  {s_brightness = arg1; setTFTbrightness(arg1); txt_BR_value.writeText(int2str(arg1)); return;}
    if(strcmp(name, "sdr_E_LP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_lowPass.writeText(c);  s_toneLP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_bandPass.writeText(c); s_toneBP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_HP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_highPass.writeText(c); s_toneHP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BAL") == 0) {itoa(arg1, c, 10); strcat(c, " ");   txt_EQ_balance.writeText(c);  s_toneBAL = arg1; webSrv.send("settone=", setI2STone()); return;}

    log_d("unused event: graphicObject %s was changed, val %li", name, arg1);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnClick(const char* name, uint8_t val) { // val = 0 --> is inactive
    // all state
    if(val == 3 && !strcmp(name, "dispFooter"))      {setTimeCounter(0); return;} // pos 3 is RSSI or TC

    if(s_state == RADIO) {
        if( val && !strcmp(name, "btn_RA_mute"))     {setTimeCounter(2); {if(!s_f_mute) s_f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_RA_prevSta"))  {setTimeCounter(2); return;}
        if( val && !strcmp(name, "btn_RA_nextSta"))  {setTimeCounter(2); return;}
        if( val && !strcmp(name, "btn_RA_staList"))  {return;}
        if( val && !strcmp(name, "btn_RA_player"))   {return;}
        if( val && !strcmp(name, "btn_RA_dlna"))     {return;}
        if( val && !strcmp(name, "btn_RA_clock"))    {return;}
        if( val && !strcmp(name, "btn_RA_sleep"))    {return;}
        if( val && !strcmp(name, "btn_RA_bright"))   {return;}
        if(!val && !strcmp(name, "btn_RA_bright"))   {setTimeCounter(2); return;}
        if( val && !strcmp(name, "btn_RA_equal"))    {return;}
        if( val && !strcmp(name, "btn_RA_bt"))       {return;}
        if(!val && !strcmp(name, "btn_RA_bt"))       {setTimeCounter(2); return;}
        if( val && !strcmp(name, "btn_RA_off"))      {return;}
        if( val && !strcmp(name, "VUmeter_RA"))      {return;}
    }
    if(s_state == STATIONSLIST) {
        if( val && !strcmp(name, "lst_RADIO"))       {setTimeCounter(LIST_TIMER); return;}
    }
    if(s_state == PLAYER) {
        if( val && !strcmp(name, "btn_PL_mute"))     {{if(!s_f_mute) s_f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_PL_pause"))    {return;}
        if( val && !strcmp(name, "btn_PL_cancel"))   {return;}
        if( val && !strcmp(name, "btn_PL_prevFile")) {if(s_cur_AudioFileNr > 0) {s_cur_AudioFileNr--; showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && !strcmp(name, "btn_PL_nextFile")) {if(s_cur_AudioFileNr + 1 < s_SD_content.getSize()) {s_cur_AudioFileNr++; showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && !strcmp(name, "btn_PL_ready"))    {return;}
        if( val && !strcmp(name, "btn_PL_playAll"))  {return;}
        if( val && !strcmp(name, "btn_PL_shuffle"))  {return;}
        if( val && !strcmp(name, "btn_PL_fileList")) {return;}
        if( val && !strcmp(name, "btn_PL_radio"))    {return;}
        if( val && !strcmp(name, "btn_PL_off"))      {return;}
        if( val && !strcmp(name, "btn_PL_playPrev")) {s_cur_AudioFileNr = s_SD_content.getPrevAudioFile(s_cur_AudioFileNr); return;}
        if( val && !strcmp(name, "btn_PL_playNext")) {s_cur_AudioFileNr = s_SD_content.getNextAudioFile(s_cur_AudioFileNr); return;}
        if( val && !strcmp(name, "pgb_PL_progress")) {return;}
    }
    if(s_state == AUDIOFILESLIST) {
        if( val && !strcmp(name, "lst_PLAYER"))      {setTimeCounter(LIST_TIMER); return;}
    }
    if(s_state == DLNA) {
        if( val && !strcmp(name, "btn_DL_mute"))     {{if(!s_f_mute) s_f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_DL_pause"))    {return;}
        if( val && !strcmp(name, "btn_DL_radio"))    {return;}
        if( val && !strcmp(name, "btn_DL_fileList")) {return;}
        if( val && !strcmp(name, "btn_DL_cancel"))   {clearStationName(); btn_DL_pause.setActive(false); return;}
        if( val && !strcmp(name, "pgb_DL_progress")) {return;}
    }
    if(s_state == DLNAITEMSLIST) {
        if( val && !strcmp(name, "lst_DLNA"))        {setTimeCounter(15); s_f_dlnaWaitForResponse = true; return;}
    }
    if(s_state == CLOCK) {
        if( val && !strcmp(name, "btn_CL_mute"))     {setTimeCounter(2); if(!s_f_mute){ s_f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_CL_alarm"))    {return;}
        if( val && !strcmp(name, "btn_CL_radio"))    {return;}
        if( val && !strcmp(name, "clk_CL_12"))       {return;}
        if( val && !strcmp(name, "clk_CL_24"))       {return;}
        if( val && !strcmp(name, "btn_CL_off"))      {return;}
    }
    if(s_state == ALARMCLOCK) {
        if( val && !strcmp(name, "clk_AC_red"))      {return;}
        if( val && !strcmp(name, "btn_AC_left"))     {return;}
        if( val && !strcmp(name, "btn_AC_right"))    {return;}
        if( val && !strcmp(name, "btn_AC_up"))       {return;}
        if( val && !strcmp(name, "btn_AC_down"))     {return;}
        if( val && !strcmp(name, "btn_AC_ready"))    {return;}
    }
    if(s_state == SLEEPTIMER) {
        if( val && !strcmp(name, "btn_SL_up"))       {return;}
        if( val && !strcmp(name, "btn_SL_down"))     {return;}
        if( val && !strcmp(name, "btn_SL_ready"))    {return;}
        if( val && !strcmp(name, "btn_SL_cancel"))   {return;}
    }
    if(s_state == SETTINGS) {
        if( val && !strcmp(name, "btn_SE_bright"))   {return;}
        if( val && !strcmp(name, "btn_SE_equal"))    {return;}
        if( val && !strcmp(name, "btn_SE_wifi"))    {return;}
        if( val && !strcmp(name, "btn_SE_radio"))    {return;}
    }
    if(s_state == BRIGHTNESS){
        if( val && !strcmp(name, "btn_BR_ready"))    {return;}
        if( val && !strcmp(name, "pic_BR_logo"))     {return;}
    }
    if(s_state == EQUALIZER) {
        if( val && !strcmp(name, "btn_E_LP"))        {sdr_EQ_lowPass.setValue(0);  return;}
        if( val && !strcmp(name, "btn_E_BP"))        {sdr_EQ_bandPass.setValue(0); return;}
        if( val && !strcmp(name, "btn_E_HP"))        {sdr_EQ_highPass.setValue(0); return;}
        if( val && !strcmp(name, "btn_E_BAL"))       {sdr_EQ_balance.setValue(0);  return;}
        if( val && !strcmp(name, "btn_EQ_Radio"))    {return;}
        if( val && !strcmp(name, "btn_EQ_Player"))   {return;}
        if( val && !strcmp(name, "btn_EQ_mute"))     {{if(!s_f_mute) s_f_muteIsPressed = true;} return;}
    }
    if(s_state == BLUETOOTH) {
        if( val && !strcmp(name, "btn_BT_pause"))    {bt_emitter.pauseResume(); return;}
        if( val && !strcmp(name, "btn_BT_radio"))    {return;}
        if( val && !strcmp(name, "btn_BT_volDown"))  {bt_emitter.downvolume(); return;}
        if( val && !strcmp(name, "btn_BT_volUp"))    {bt_emitter.upvolume();  return;}
        if( val && !strcmp(name, "btn_BT_mode"))     {bt_emitter.changeMode(); return;}
        if( val && !strcmp(name, "btn_BT_power"))    {return;}
    }
    if(s_state == IR_SETTINGS) {
        if( val && !strcmp(name, "btn_IR_radio"))    {return;}
    }
    if(s_state == WIFI_SETTINGS) {
        if( val && !strcmp(name, "key_WI_input"))    {log_e("val %i", val); if(val == 13){ s_radioSubMenue = 0; changeState(RADIO); return;}}
    }
    log_d("unused event: graphicObject %s was clicked", name);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnRelease(const char* name, releasedArg ra) {

    // all state
        if(!strcmp(name, "dispFooter"))      {return;}

    if(s_state == RADIO) {
        if(!strcmp(name, "btn_RA_mute"))     {muteChanged(btn_RA_mute.getValue()); return;}
        if(!strcmp(name, "btn_RA_prevSta"))  {prevFavStation(); dispFooter.updateStation(s_cur_station); return;}
        if(!strcmp(name, "btn_RA_nextSta"))  {nextFavStation(); dispFooter.updateStation(s_cur_station); return;}
        if(!strcmp(name, "btn_RA_staList"))  {s_radioSubMenue = 0; changeState(STATIONSLIST); return;}
        if(!strcmp(name, "btn_RA_player"))   {s_playerSubMenue = 0; stopSong(); s_playerSubMenue = 0; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_RA_dlna"))     {s_dlnaSubMenue = 0; stopSong(); s_dlnaSubMenue = 0; changeState(DLNA); return;}
        if(!strcmp(name, "btn_RA_clock"))    {s_clockSubMenue = 0; s_clockSubMenue = 0; changeState(CLOCK); return;}
        if(!strcmp(name, "btn_RA_sleep"))    {s_radioSubMenue = 0; changeState(SLEEPTIMER); return;}
        if(!strcmp(name, "btn_RA_settings")) {s_settingsSubMenue = 0; changeState(SETTINGS); return;}
        if(!strcmp(name, "btn_RA_equal"))    {s_radioSubMenue = 0; changeState(EQUALIZER); return;}
        if(!strcmp(name, "btn_RA_bt"))       {s_radioSubMenue = 0; changeState(BLUETOOTH); return;}
        if(!strcmp(name, "btn_RA_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "VUmeter_RA"))      {return;}
        if(!strcmp(name, "sdr_RA_volume"))   {return;}
    }
    if(s_state == STATIONSLIST) {
        if(!strcmp(name, "lst_RADIO"))       {if(ra.val1){s_radioSubMenue = 0; setStationByNumber(ra.val1); s_radioSubMenue = 0; changeState(RADIO);} return;}
    }
    if(s_state == PLAYER) {
        if(!strcmp(name, "btn_PL_mute"))     {muteChanged(btn_PL_mute.getValue()); return;}
        if(!strcmp(name, "btn_PL_pause"))    {if(s_f_isFSConnected) {s_f_pauseResume = audio.pauseResume();} return;}
        if(!strcmp(name, "btn_PL_cancel"))   {s_playerSubMenue = 0; stopSong(); changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_prevFile")) {return;}
        if(!strcmp(name, "btn_PL_nextFile")) {return;}
        if(!strcmp(name, "btn_PL_ready"))    {stopSong(); SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); s_playerSubMenue = 1; changeState(PLAYER); showAudioFileNumber(); return;}
        if(!strcmp(name, "btn_PL_playAll"))  { s_f_shuffle = false; preparePlaylistFromSDFolder(s_cur_AudioFolder); processPlaylist(true); s_playerSubMenue = 1; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_shuffle"))  { s_f_shuffle = true; preparePlaylistFromSDFolder(s_cur_AudioFolder); processPlaylist(true); s_playerSubMenue = 1; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_fileList")) {s_SD_content.listFilesInDir(s_cur_AudioFolder, true, false); changeState(AUDIOFILESLIST); return;}
        if(!strcmp(name, "btn_PL_radio"))    {setStation(s_cur_station);s_playerSubMenue = 0; s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_PL_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "sdr_PL_volume"))   {return;}
        if(!strcmp(name, "btn_PL_playNext")) {SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber(); return;}
        if(!strcmp(name, "btn_PL_playPrev")) {SD_playFile(s_cur_AudioFolder, s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber(); return;}
        if(!strcmp(name, "pgb_PL_progress")) {audio.setTimeOffset(ra.val2); return;}
    }
    if(s_state == AUDIOFILESLIST){
        if(!strcmp(name, "lst_PLAYER"))      {if(ra.val1 == 1){;} // wipe up/down
                                              if(ra.val1 == 2){x_ps_free(&s_cur_AudioFolder); s_cur_AudioFolder = strdup(ra.arg1); s_cur_AudioFileNr = ra.val2; lst_PLAYER.show(s_cur_AudioFolder, s_cur_AudioFileNr);   } // next prev folder
                                              if(ra.val1 == 3){x_ps_free(&s_cur_AudioFolder); s_cur_AudioFolder = strdup(ra.arg1); s_cur_AudioFileNr = ra.val2; stopSong(); SD_playFile(ra.arg3);} return;}
    }
    if(s_state == DLNA) {
        if(!strcmp(name, "btn_DL_mute") )    {muteChanged(btn_DL_mute.getValue()); return;}
        if(!strcmp(name, "btn_DL_pause"))    {s_f_pauseResume = audio.pauseResume(); return;}
        if(!strcmp(name, "btn_DL_radio"))    {setStation(s_cur_station); txt_DL_fName.setText(""); s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_DL_fileList")) {changeState(DLNAITEMSLIST); txt_DL_fName.setText(""); return;}
        if(!strcmp(name, "btn_DL_cancel"))   {stopSong(); txt_DL_fName.setText(""); txt_DL_fName.show(true, false); pgb_DL_progress.reset(); btn_DL_pause.setActive(false); btn_DL_pause.show(); return;}
        if(!strcmp(name, "sdr_DL_volume"))   {return;}
        if(!strcmp(name, "pgb_DL_progress")) {audio.setTimeOffset(ra.val2); return;}
    }
    if(s_state == DLNAITEMSLIST) {
        if(!strcmp(name, "lst_DLNA"))        {if(ra.val1 == 1){txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(ra.arg2); connecttohost(ra.arg1); s_dlnaSubMenue = 0; changeState(DLNA);} // play a file
                                              if(ra.val1 == 2){dlna.browseServer(ra.val2, ra.arg1, 0, 50); s_f_dlnaMakePlaylistOTF = true; } // browse dlna object, waiting for content and create a playlist
                                              return;}
    }
    if(s_state == CLOCK) {
        if(!strcmp(name, "btn_CL_mute"))     {muteChanged(btn_CL_mute.getValue()); return;}
        if(!strcmp(name, "btn_CL_alarm"))    {changeState(ALARMCLOCK); return;}
        if(!strcmp(name, "btn_CL_radio"))    {s_clockSubMenue = 0; s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "clk_CL_12"))       {s_clockSubMenue = 1; changeState(CLOCK); return;}
        if(!strcmp(name, "clk_CL_24"))       {s_clockSubMenue = 1; changeState(CLOCK); return;}
        if(!strcmp(name, "btn_CL_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "sdr_CL_volume"))   {return;}
    }
    if(s_state == ALARMCLOCK) {
        if(!strcmp(name, "clk_AC_red"))      {return;}
        if(!strcmp(name, "btn_AC_left"))     {clk_AC_red.shiftLeft(); return;}
        if(!strcmp(name, "btn_AC_right"))    {clk_AC_red.shiftRight(); return;}
        if(!strcmp(name, "btn_AC_up"))       {clk_AC_red.digitUp(); return;}
        if(!strcmp(name, "btn_AC_down"))     {clk_AC_red.digitDown(); return;}
        if(!strcmp(name, "btn_AC_ready"))    {updateSettings(); s_clockSubMenue = 0; changeState(CLOCK); logAlarmItems(); return;}
    }
    if(s_state == SLEEPTIMER) {
        if(!strcmp(name, "btn_SL_up"))       {display_sleeptime(1); return;}
        if(!strcmp(name, "btn_SL_down"))     {display_sleeptime(-1); return;}
        if(!strcmp(name, "btn_SL_ready"))    {dispFooter.updateOffTime(s_sleeptime); s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_SL_cancel"))   {s_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(s_state == BRIGHTNESS){
        if(!strcmp(name, "btn_BR_ready"))    {s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "pic_BR_logo"))     {return;}
    }
    if(s_state == EQUALIZER) {
        if(!strcmp(name, "btn_EQ_Radio"))    {setStation(s_cur_station); s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_EQ_Player"))   {s_playerSubMenue = 0; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_EQ_mute"))     {muteChanged(btn_EQ_mute.getValue()); return;}
    }
    if(s_state == SETTINGS) {
        if(!strcmp(name, "btn_SE_bright"))   { changeState(BRIGHTNESS);    return;}
        if(!strcmp(name, "btn_SE_equal"))    { changeState(EQUALIZER);     return;}
        if(!strcmp(name, "btn_SE_wifi"))     { changeState(WIFI_SETTINGS); return;}
        if(!strcmp(name, "btn_SE_radio"))    {s_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(s_state == BLUETOOTH) {
        if(!strcmp(name, "btn_BT_pause"))    {return;}
        if(!strcmp(name, "btn_BT_radio"))    {s_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_BT_volDown"))  {return;}
        if(!strcmp(name, "btn_BT_volUp"))    {return;}
        if(!strcmp(name, "btn_BT_mode"))     {return;}
        if(!strcmp(name, "btn_BT_power"))    {s_bt_emitter.power_state == 1 ? bt_emitter.power_off() : bt_emitter.power_on() ; return;}
    }
    if(s_state == IR_SETTINGS) {
        if(!strcmp(name, "btn_IR_radio"))    {s_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(s_state == WIFI_SETTINGS) {
        if(!strcmp(name, "wifiSettings"))    {setWiFiCredentials(ra.arg1, ra.arg2); ESP.restart();}
    }
    log_d("unused event: graphicObject %s was released", name);
}
// clang-format on