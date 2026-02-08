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
    Version 4.1.0c2 - Feb 08, 2026                                                                                                               ";

/*  display (320x240px) with controller ILI9341 or
    display (480x320px) with controller ILI9486, ILI9488 or ST7796 (SPI) or
    display (800x480px) (RGB-HMI) with TP controller GT911 (I2C)
    display (1024x600px) (DSI) with TP controller GT911 (I2C)

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

SET_LOOP_TASK_STACK_SIZE(14 * 1024);

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

constexpr uint16_t MAX_STATIONS = 1000;

settings_s    s_settings;
volume_s      s_volume;
dlnaHistory_s s_dlnaHistory[10];
timecounter_s s_timeCounter;
SD_content    s_SD_content;
bt_emitter_s  s_bt_emitter;
tone_s        s_tone;
Playlist      playlist;

ps_ptr<char> s_time_s = "";
ps_ptr<char> s_myIP;
ps_ptr<char> s_icyDescription = "";
ps_ptr<char> s_streamTitle = "";
ps_ptr<char> s_cur_AudioFolder = "/audiofiles/";
ps_ptr<char> s_cur_AudioFileName = NULL;
ps_ptr<char> s_stationURL;
ps_ptr<char> s_playlistPath;
ps_ptr<char> s_stationName_air;
ps_ptr<char> s_homepage = "";
ps_ptr<char> s_TZName = "Europe/Berlin";
ps_ptr<char> s_TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
ps_ptr<char> s_timeSpeechLang = "en";
ps_ptr<char> s_lyrics = "";

int8_t   s_state = UNDEFINED; // statemaschine
int8_t   s_subState_radio = UNDEFINED;
int8_t   s_subState_player = UNDEFINED;
int8_t   s_subState_clock = UNDEFINED;
int8_t   s_ir_btn_select = UNDEFINED; // IR menue item
int8_t   s_currDLNAsrvNr = -1;
uint8_t  s_alarmdays = 0;
uint8_t  s_cur_Codec = 0;
uint8_t  s_numServers = 0; //
uint8_t  s_level = 0;
uint8_t  s_sleepMode = 1; // 0 display off, 1 show the clock
uint8_t  s_staListPos = 0;
uint8_t  s_cthFailCounter = 0; // connecttohost fail
uint8_t  s_itemListPos = 0;    // DLNA items
uint8_t  s_fileListPos = 0;
int8_t   s_alarmSubMenue = -1;
int8_t   s_sleepTimerSubMenue = -1;
uint8_t  s_ambientValue = 50;
uint8_t  s_dlnaLevel = 0;
uint8_t  s_resetReason = (esp_reset_reason_t)ESP_RST_UNKNOWN;
int16_t  s_totalNumberReturned = -1;
int16_t  s_dlnaMaxItems = -1;
int16_t  s_dlnaMaXServers = -1;
int16_t  s_alarmtime[7] = {0};  // in minutes (23:59 = 23 *60 + 59) [0] Sun, [1] Mon
int16_t  s_cur_AudioFileNr = 0; // this is the position of the file within the (alpha ordered) folder starting with 0
int16_t  s_brightness = UINT8_MAX;
uint16_t s_staListNr = 0;
uint16_t s_fileListNr = 0;
uint16_t s_cur_station = 0; // current station(nr), will be set later
uint16_t s_sleeptime = 0;   // time in min until MiniWebRadio goes to sleep
uint16_t s_plsCurPos = 0;
uint16_t s_dlnaItemNr = 0;
uint16_t s_bh1750Value = 50;
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

bool s_f_rtc = false; // true if time from ntp is received
bool s_f_100ms = false;
bool s_f_1sec = false;
bool s_f_10sec = false;
bool s_f_1min = false;
bool s_f_mute = false;
bool s_f_muteIsPressed = false;
bool s_f_sleeping = false;
bool s_f_isWebConnected = false;
bool s_f_WiFi_lost = false;
bool s_f_isFSConnected = false;
bool s_f_eof = false;
bool s_f_reconnect = false;
bool s_f_eof_alarm = false;
bool s_f_alarm = false;
bool s_f_newIcyDescription = false;
bool s_f_newStreamTitle = false;
bool s_f_webFailed = false;
bool s_f_newBitRate = false;
bool s_f_newStationName = false;
bool s_f_newLyrics = false;
bool s_f_volBarVisible = false;
bool s_f_switchToClock = false;   // jump into CLOCK mode at the next opportunity
bool s_f_timeAnnouncement = true; // time announcement every full hour
bool s_f_playlistEnabled = false;
bool s_f_playlistNextFile = false;
bool s_f_logoUnknown = false;
bool s_f_pauseResume = false;
bool s_f_FFatFound = false;
bool s_f_BH1750_found = false;
bool s_f_clearLogo = false;
bool s_f_clearStationName = false;
bool s_f_dlnaBrowseServer = false;
bool s_f_dlnaWaitForResponse = false;
bool s_f_dlnaSeekServer = false;
bool s_f_dlnaMakePlaylistOTF = false; // notify callback that this browsing was to build a On-The_fly playlist
bool s_f_dlna_browseReady = false;
bool s_f_brightnessIsChangeable = false;
bool s_f_connectToLastStation = false;
bool s_f_msg_box = false;
bool s_f_esp_restart = false;
bool s_f_timeSpeech = false;
bool s_f_stationsChanged = false;
bool s_f_sd_card_found = false;
bool s_f_isWiFiConnected = false;
bool s_f_ok_from_ir = false;

std::deque<ps_ptr<char>> s_PLS_content;
std::deque<ps_ptr<char>> s_logBuffer;

const char* codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "AACP", "OPUS", "OGG", "VORBIS"};

Audio          audio;
Preferences    pref;
WebSrv         webSrv;
WiFiMulti      wifiMulti;
RTIME          rtc;
Ticker         ticker100ms;
IR_buttons     irb(&s_settings);
IR             ir(IR_PIN); // do not change the objectname, it must be "ir"
File           audioFile;
FtpServer      ftpSrv;
DLNA_Client    dlna;
KCX_BT_Emitter bt_emitter(BT_EMITTER_RX, BT_EMITTER_TX, BT_EMITTER_CONNECT, BT_EMITTER_MODE);
TwoWire        i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire        i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
hp_BH1750      BH1750;                 // create the sensor
SPIClass       spiBus(FSPI);

#if TFT_CONTROLLER < 7 // ⏹⏹⏹⏹
TFT_SPI tft(spiBus, TFT_CS);
#elif TFT_CONTROLLER == 7
TFT_RGB tft;
#elif TFT_CONTROLLER == 8
TFT_DSI tft;
#else
    #error "wrong TFT_CONTROLLER"
#endif

#if TP_CONTROLLER < 7 // ⏹⏹⏹⏹
TP_XPT2046 tp(spiBus, TP_CS);
#elif TP_CONTROLLER == 7
TP_GT911 tp;
#elif TP_CONTROLLER == 8
FT6x36 tp;
#else
    #error "wrong TP_CONTROLLER"
#endif

stationManagement staMgnt(&s_cur_station);

SemaphoreHandle_t mutex_rtc;
SemaphoreHandle_t mutex_display;

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     D E F A U L T S E T T I N G S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

bool SD_MMC_exists(const char* path) {
    return SD_MMC.exists(path);
}

// clang-format off
/*🟢🟡🔴*/
boolean defaultsettings() {

    if (!SD_MMC.exists("/ir_buttons.json")) { // if not found ir_buttons.json create a default file
        File file = SD_MMC.open("/ir_buttons.json", "w", true);
        file.write((uint8_t*)ir_buttons_json, sizeof(ir_buttons_json));
        file.close();
    }
    irb.loadButtonsFromJSON("/ir_buttons.json");
    for (uint i = 0; i < s_settings.numOfIrButtons; i++) {
        MWR_LOG_DEBUG("0x%04X,  %s", s_settings.irbuttons[i].val, s_settings.irbuttons[i].label);
        ir.set_irButtons(i, s_settings.irbuttons[i].val);
    }
    ir.set_irAddress(s_settings.irbuttons[42].val);
    MWR_LOG_DEBUG("0x%04X,  %s", s_settings.irbuttons[42].val, s_settings.irbuttons[42].label);

    if (!SD_MMC.exists("/settings.json")) { // if not found create one
        updateSettings();
    }

    File         file2 = SD_MMC.open("/settings.json", "r", false);
    ps_ptr<char> jO;
    jO.calloc(2048);
    ps_ptr<char> tmp;
    tmp.calloc(1024);
    file2.readBytes(jO.get(), 2048);
    s_settingsHash = simpleHash(jO.get());
    file2.close();

    auto parseJson = [&](const char* s) { // lambda, inner function
        int16_t pos1 = 0, pos2 = 0, pos3 = 0;
        pos1 = jO.index_of(s, 0);
        if (pos1 < 0) {
            MWR_LOG_ERROR("index %s not found", s);
            return "0";
        }
        pos2 = jO.index_of(":", pos1) + 1;
        if (jO[pos2] == '\"')
            pos3 = jO.index_of("\"", pos2 + 1) + 1;
        else
            pos3 = jO.index_of(",", pos2);
        if (pos3 < 0) pos3 = find_first_of(jO.get(), "}\n", pos2);
        if (jO[pos2] == '\"') {
            pos2++;
            pos3--;
        } // remove \" embraced strings
        tmp = jO.substr(pos2, pos3 - pos2);
        tmp[pos3 - pos2] = '\0';
        return (const char*)tmp.c_get();
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
    s_bt_emitter.enabled  = (strcmp(parseJson("\"BTpower\":"), "true") == 0) ? 1 : 0;
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
    s_timeSpeechLang = parseJson("\"timeSpeechLang\":");
    s_f_mute = (strcmp(parseJson("\"mute\":"), "true") == 0) ? 1 : 0;
    s_brightness = max(5, atoi(parseJson("\"brightness\":")));
    s_sleeptime = atoi(parseJson("\"sleeptime\":"));
    s_cur_station = atoi(parseJson("\"station\":"));
    s_tone.LP = atoi(parseJson("\"toneLP\":"));
    s_tone.BP = atoi(parseJson("\"toneBP\":"));
    s_tone.HP = atoi(parseJson("\"toneHP\":"));
    s_tone.BAL = atoi(parseJson("\"balance\":"));
    s_TZName = parseJson("\"Timezone_Name\":");
    s_TZString = parseJson("\"Timezone_String\":");
    s_settings.lastconnectedhost.copy_from(parseJson("\"lastconnectedhost\":"));
    s_settings.lastconnectedfile.copy_from(parseJson("\"lastconnectedfile\":"));
    s_sleepMode = atoi(parseJson("\"sleepMode\":"));
    s_state = atoi(parseJson("\"state\":"));

    // set some items ---------------------------------------------------------------------------------------------
    if (!s_settings.lastconnectedfile.starts_with("/")) { s_settings.lastconnectedfile.assign("/audiofiles/"); } // guard
    s_SD_content.setLastConnectedFile(s_settings.lastconnectedfile.get());
    s_cur_AudioFolder = s_SD_content.getLastConnectedFolder();
    s_cur_AudioFileName = s_SD_content.getLastConnectedFileName();
    s_cur_AudioFileNr = s_SD_content.getPosByFileName(s_cur_AudioFileName.c_get());
    if (s_cur_AudioFileNr == -1) s_cur_AudioFileNr = 0; // not found
    // ------------------------------------------------------------------------------------------------------------

    if (!SD_MMC.exists("/stations.json")) { // if not found create one
        File file1 = SD_MMC.open("/stations.json", "w", true);
        file1.write((uint8_t*)stations_json, sizeof(stations_json) - 1); // without termination
        file1.close();
    }
    staMgnt.updateStationsList();
    return true;
}
// clang-format on
/*🟢🟡🔴*/

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
    s_bt_emitter.enabled == true ? jO.appendf("\"true\"") : jO.appendf("\"false\"");
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
    jO.appendf(",\n  \"timeSpeechLang\":\"%s\"", s_timeSpeechLang.c_get());
    jO.appendf(",\n  \"mute\":");
    (s_f_mute == true) ? jO.appendf("\"true\"") : jO.appendf("\"false\"");
    jO.appendf(",\n  \"brightness\":%i", s_brightness);
    jO.appendf(",\n  \"sleeptime\":%i", s_sleeptime);
    jO.appendf(",\n  \"lastconnectedhost\":\"%s\"", s_settings.lastconnectedhost.c_get());
    jO.appendf(",\n  \"lastconnectedfile\":\"%s\"", s_settings.lastconnectedfile.c_get());
    jO.appendf(",\n  \"station\":%i", s_cur_station);
    jO.appendf(",\n  \"Timezone_Name\":\"%s\"", s_TZName.c_get());
    jO.appendf(",\n  \"Timezone_String\":\"%s\"", s_TZString.c_get());
    jO.appendf(",\n  \"toneLP\":%i", s_tone.LP);
    jO.appendf(",\n  \"toneBP\":%i", s_tone.BP);
    jO.appendf(",\n  \"toneHP\":%i", s_tone.HP);
    jO.appendf(",\n  \"balance\":%i", s_tone.BAL);
    jO.appendf(",\n  \"state\":%i", s_state);
    jO.appendf(",\n  \"sleepMode\":%i\n}", s_sleepMode);

    if (s_settingsHash != simpleHash(jO.get())) {
        File file = SD_MMC.open("/settings.json", "w", false);
        if (!file) {
            MWR_LOG_ERROR("file \"settings.json\" not found");
            return;
        }
        file.print(jO.get());
        s_settingsHash = simpleHash(jO.c_get());

        MWR_LOG_DEBUG("%s", jO.c_get());
    }
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

void timer100ms() {
    static uint16_t ms100 = 0;
    s_f_100ms = true;
    ms100++;
    if (!(ms100 % 10)) {
        s_f_1sec = true;
        s_time_s = rtc.gettime_s();
        if (s_time_s.ends_with("59:53")) s_f_timeSpeech = true;
    }
    if (!(ms100 % 100)) s_f_10sec = true;
    if (!(ms100 % 600)) {
        s_f_1min = true;
        ms100 = 0;
    }
}

/*****************************************************************************************************************************************************
 *                                                               D I S P L A Y                                                                       *
 *****************************************************************************************************************************************************/

inline void bgColorWithOutHeaderFooter() {
    tft.fillRect(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, TFT_BLACK);
}
inline void clearHeader() {
    tft.copyFramebuffer(1, 0, layout.winHeader.x, layout.winHeader.y, layout.winHeader.w, layout.winHeader.h);
}
inline void clearLogo() {
    tft.copyFramebuffer(1, 0, layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h);
}
inline void clearStationName() {
    tft.copyFramebuffer(1, 0, layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
}
inline void clearStreamTitle() {
    tft.copyFramebuffer(1, 0, layout.winSTitle.x, layout.winSTitle.y, layout.winSTitle.w, layout.winSTitle.h);
} // without VUmeter
inline void clearWithOutHeaderFooter() {
    tft.copyFramebuffer(1, 0, layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h);
}
inline void clearAll() {
    tft.copyFramebuffer(1, 0, 0, 0, displayConfig.dispWidth, displayConfig.dispHeight);
}

inline uint16_t txtlen(String str) {
    uint16_t len = 0;
    for (int32_t i = 0; i < str.length(); i++)
        if (str[i] <= 0xC2) len++;
    return len;
}

void showStreamTitle(ps_ptr<char> streamtitle) {
    if (s_f_sleeping) return;

    streamtitle.trim();
    // replacestr(st, " | ", "\n"); // some stations use pipe as \n or
    // replacestr(st, "| ", "\n");
    // replacestr(st, "|", "\n");

    txt_RA_sTitle.setTextColor(TFT_CORNSILK);
    txt_RA_sTitle.writeText(streamtitle.c_get());
}

void showLogoAndStationName(bool force) {
    ps_ptr<char>        SN_utf8;
    ps_ptr<char>        path;
    ps_ptr<char>        staNr;
    static ps_ptr<char> old_SN_utf8;
    if (force) { old_SN_utf8.reset(); }

    if (s_cur_station) {
        MWR_LOG_DEBUG("showLogoAndStationName: %s", staMgnt.getStationName(s_cur_station));
        SN_utf8 = staMgnt.getStationName(s_cur_station);
        SerialPrintfln("Country: ..  " ANSI_ESC_GREEN "%s" ANSI_ESC_RESET "  ", staMgnt.getStationCountry(s_cur_station));
    } else {
        SN_utf8 = s_stationName_air;
    }
    SN_utf8.trim();

    path = "/logo/" + SN_utf8 + ".jpg";
    if (!SD_MMC.exists(scaleImage(path).c_get())) path = "/common/unknown.png";

    if (old_SN_utf8 != SN_utf8) {
        old_SN_utf8 = SN_utf8;
        txt_RA_staName.setTextColor(TFT_CYAN);
        txt_RA_staName.setText(SN_utf8.c_get());
        txt_RA_staName.show(true, false);
        pic_RA_logo.setPicturePath(path.c_get());
        pic_RA_logo.show(true, false);
        staNr.assignf("%i", s_cur_station);
    }
    webSrv.send("stationLogo=", path.c_get());
    webSrv.send("stationNr=", staNr);
    webSrv.send("stationURL=", s_settings.lastconnectedhost.get());

    return;
}

void showFileLogo(int8_t state, int8_t subState) {
    String logo;
    if (state == RADIO) {
        if (s_stationURL.ends_with("m3u8"))
            logo = "/common/" + (String) "M3U8" + ".png";
        else
            logo = "/common/" + (String)codecname[s_cur_Codec] + ".png";
        pic_RA_logo.setPicturePath(logo.c_str());
        pic_RA_logo.setAlternativPicturePath("/common/unknown.png");
        pic_RA_logo.show(true, false);
        webSrv.send("stationLogo=", logo.c_str());
        return;
    } else if (state == DLNA) {
        logo = "/common/DLNA.jpg";
        pic_DL_logo.setPicturePath(logo.c_str());
        pic_DL_logo.setAlternativPicturePath("/common/unknown.png");
        pic_DL_logo.show(true, false);
        webSrv.send("stationLogo=", logo.c_str());
        return;
    }
    if (state == PLAYER) { // s_state PLAYER
        if (s_cur_Codec == 0)
            logo = "/common/AudioPlayer.png";
        else if (s_subState_player == 0)
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
    if (ud == 1) {                  // up
        switch (s_sleeptime) {
            case 0 ... 14: s_sleeptime = (s_sleeptime / 5) * 5 + 5; break;
            case 15 ... 59: s_sleeptime = (s_sleeptime / 15) * 15 + 15; break;
            case 60 ... 359: s_sleeptime = (s_sleeptime / 60) * 60 + 60; break;
            default: s_sleeptime = 360; break; // max 6 hours
        }
    }
    if (ud == -1) { // down
        switch (s_sleeptime) {
            case 1 ... 15: s_sleeptime = ((s_sleeptime - 1) / 5) * 5; break;
            case 16 ... 60: s_sleeptime = ((s_sleeptime - 1) / 15) * 15; break;
            case 61 ... 360: s_sleeptime = ((s_sleeptime - 1) / 60) * 60; break;
            default: s_sleeptime = 0; break; // min
        }
    }
    otb_SL_stime.show(s_sleeptime);
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth, uint16_t maxHeigth) {
    ps_ptr<char> p = path;
    auto         scImg = scaleImage(p);
    if (!SD_MMC.exists(scImg.c_get())) {
        if (scImg.index_of("/.", 0) > 0) return false; // empty filename
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "file \"%s\" not found" ANSI_ESC_RESET "  ", scImg.c_get());
        return false;
    }
    if (scImg.ends_with("bmp")) { return tft.drawBmpFile(SD_MMC, scImg.c_get(), posX, posY, maxWidth, maxHeigth, 1.0); }
    if (scImg.ends_with("jpg")) { return tft.drawJpgFile(SD_MMC, scImg.c_get(), posX, posY, maxWidth, maxHeigth); }
    if (scImg.ends_with("gif")) { return tft.drawGifFile(SD_MMC, scImg.c_get(), posX, posY, 0); }
    if (scImg.ends_with("png")) { return tft.drawPngFile(SD_MMC, scImg.c_get(), posX, posY); }

    SerialPrintfln(ANSI_ESC_RED "the file \"%s\" contains neither a bmp, a gif, a png nor a jpg graphic" ANSI_ESC_RESET "  ", scImg.c_get());
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

void processPlaylist() {
    bool f_isURL, f_isFile;
start:
    f_isURL = false;
    f_isFile = false;
    if (playlist.get_size() == 0) { // guard
        MWR_LOG_ERROR("playlist is empty");
        s_f_playlistEnabled = false;
        return;
    }

    int idx = playlist.next_index();
    if (idx == -1) {
        SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "end of playlist" ANSI_ESC_RESET "  ");
        webSrv.send("SD_playFile=", "end of playlist");
        s_f_playlistEnabled = false;
        changeState(PLAYER, 0);
        return;
    }

    if (idx == 0) { // first
        changeState(PLAYER, 1);
        txt_PL_fName.writeText("");
    }

    SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "next playlist file" ANSI_ESC_RESET "  ");
    s_f_playlistEnabled = true;

    ps_ptr<char> path = playlist.get_file(); // path or url

    if (path.starts_with_icase("http;//") or path.starts_with_icase("https://")) {
        f_isURL = true; // is web file
    }

    if (path.starts_with("/") && SD_MMC.exists(path.c_get())) { f_isFile = true; }

    if (f_isFile == false && f_isURL == false) goto start;

    if (f_isURL) { connecttohost(path); }                  // is web file
    if (f_isFile) { connecttoFS("SD_MMC", path.c_get()); } // is file

    if (s_f_isFSConnected || s_f_isWebConnected) {
        SerialPrintflnCut("Playlist:    ", ANSI_ESC_YELLOW, path.c_get());
        webSrv.send("SD_playFile=", path);
        txt_PL_fNumber.writeText(playlist.get_coloured_index().c_get());
        txt_PL_fName.writeText(playlist.get_items().c_get());
    } else {
        SerialPrintfln("Playlist:    " ANSI_ESC_YELLOW "can't connect to %s" ANSI_ESC_RESET "  ", path.c_get());
        goto start;
    }

    MWR_LOG_WARN("path %s, items %s", playlist.get_file().c_get(), playlist.get_items().c_get());

    return;
}

/*****************************************************************************************************************************************************
 *                                                        C O N N E C T   TO   W I F I                                                               *
 *****************************************************************************************************************************************************/
bool connectToWiFi() {

    MWR_LOG_DEBUG("Connecting to WiFi...");
    ps_ptr<char> line(512);

    // create nvs entries if they do not exist
    if (!pref.isKey("wifiStr0")) pref.putString("wifiStr0", ""); // SSID + \t + PW
    if (!pref.isKey("wifiStr1")) pref.putString("wifiStr1", "");
    if (!pref.isKey("wifiStr2")) pref.putString("wifiStr2", "");
    if (!pref.isKey("wifiStr3")) pref.putString("wifiStr3", "");
    if (!pref.isKey("wifiStr4")) pref.putString("wifiStr4", "");
    if (!pref.isKey("wifiStr5")) pref.putString("wifiStr5", "");

    const char* SSID = _SSID;
    const char* PW = _PW;
    line = SSID;
    line += "\t";
    line += PW;
    pref.putString("wifiStr0", line.c_get());
    WiFi.mode(WIFI_STA);

    for (int i = 0; i < 6; i++) {
        line.clear(); // Move this line outside the switch statement
        switch (i) {
            case 0: line = pref.getString("wifiStr0").c_str(); break;
            case 1: line = pref.getString("wifiStr1").c_str(); break;
            case 2: line = pref.getString("wifiStr2").c_str(); break;
            case 3: line = pref.getString("wifiStr3").c_str(); break;
            case 4: line = pref.getString("wifiStr4").c_str(); break;
            case 5: line = pref.getString("wifiStr5").c_str(); break;
        }
        if (line.strlen() < 5) continue;  // line is empty
        int pos = line.index_of("\t", 0); // find first tab
        if (pos < 0) continue;            // no tab found
        line[pos] = '\0';                 // terminate ssid
        char* ssid = line.get();          // ssid is the first part
        char* pw = line.get() + pos + 1;  // password is the second part
        MWR_LOG_DEBUG("ssid %s", ssid);
        MWR_LOG_DEBUG("pw %s", pw);
        wifiMulti.addAP(ssid, pw); // SSID and PW in code"
        size_t offset = 0;
        size_t pwlen = strlen(pw);
        size_t dot_len = strlen(emoji.blueCircle); // = 4
        size_t buf_size = pwlen * dot_len + 1;     // +1 für '\0'
        if (buf_size > 512) {
            MWR_LOG_ERROR("Password display buffer too large: %zu bytes", buf_size);
            continue;
        }
        char pass[buf_size];
        for (size_t j = 0; j < pwlen; j++) {
            if (offset + dot_len > buf_size - 1) {
                MWR_LOG_ERROR("Buffer overflow in password masking");
                break;
            }
            memcpy(pass + offset, emoji.blueCircle, dot_len);
            offset += dot_len;
        }
        pass[offset] = '\0'; // Zero-terminate the string
        SerialPrintfln("WiFI_info:   add credentials: " ANSI_ESC_CYAN "%s - %s" ANSI_ESC_YELLOW " [%s:%d]" ANSI_ESC_RESET "  ", ssid, pass, __FILENAME__, __LINE__);
    }

    // These options can help when you need ANY kind of wifi connection to get a config file, report errors, etc.
    wifiMulti.setStrictMode(true); // Default is true.  Library will disconnect and forget currently connected AP if it's not in the AP list.
    SerialPrintfln("WiFI_info:   Connecting WiFi...");

    WiFi.mode(WIFI_MODE_STA);

    int i = 0;
    while (!WiFi.isConnected()) {
        wifiMulti.run();
        vTaskDelay(100);
        i++;
        if (i > 50) break;
    }
    if (!WiFi.isConnected()) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "WiFi credentials are not correct" ANSI_ESC_RESET "  ");
        return false;
    }
    WiFi.setAutoReconnect(true);
    if (WIFI_TX_POWER >= 2 && WIFI_TX_POWER <= 21) WiFi.setTxPower((wifi_power_t)(WIFI_TX_POWER * 4));
    SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "WiFi connected" ANSI_ESC_RESET "  ");
    vTaskDelay(1000);
    s_myIP = WiFi.localIP().toString().c_str();
    SerialPrintfln("WiFI_info:   connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", IP address is " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", Received Signal Strength " ANSI_ESC_CYAN "%i" ANSI_ESC_WHITE
                   " dB" ANSI_ESC_RESET "   ",
                   WiFi.SSID().c_str(), s_myIP.c_get(), WiFi.RSSI());
    return true; // can't connect to any network
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void setWiFiCredentials(ps_ptr<char> ssid, ps_ptr<char> password) {
    if (ssid.strlen() < 5) return; // min length

    MWR_LOG_ERROR("ssid %s pw %s", ssid.c_get(), password.c_get());

    ps_ptr<char> line = "";
    ps_ptr<char> credentials;
    int          i = 0, state = 0;

    for (i = 0; i < 6; i++) {
        switch (i) {
            case 0: line = pref.getString("wifiStr0").c_str(); break;
            case 1: line = pref.getString("wifiStr1").c_str(); break;
            case 2: line = pref.getString("wifiStr2").c_str(); break;
            case 3: line = pref.getString("wifiStr3").c_str(); break;
            case 4: line = pref.getString("wifiStr4").c_str(); break;
            case 5: line = pref.getString("wifiStr5").c_str(); break;
        }
        if (line.starts_with(ssid.c_get()) && line[ssid.strlen()] == '\t') { // ssid found
            if (password.strlen() == 0) {
                credentials = "\t"; // delete ssid and password
            } else {                // update password
                credentials = ssid;
                credentials += "\t";
                credentials += password;
            }
            if (i == 0) {
                MWR_LOG_ERROR("password can't changed, is hard coded");
                state = 0;
                goto exit;
            }
            if (i == 1) {
                pref.putString("wifiStr1", credentials.get());
                state = 1;
                goto exit;
            }
            if (i == 2) {
                pref.putString("wifiStr2", credentials.get());
                state = 1;
                goto exit;
            }
            if (i == 3) {
                pref.putString("wifiStr3", credentials.get());
                state = 1;
                goto exit;
            }
            if (i == 4) {
                pref.putString("wifiStr4", credentials.get());
                state = 1;
                goto exit;
            }
            if (i == 5) {
                pref.putString("wifiStr5", credentials.get());
                state = 1;
                goto exit;
            }
        }
    }
    for (i = 1; i < 6; i++) {
        line.clear();
        switch (i) {
            case 1: line = pref.getString("wifiStr1").c_str(); break;
            case 2: line = pref.getString("wifiStr2").c_str(); break;
            case 3: line = pref.getString("wifiStr3").c_str(); break;
            case 4: line = pref.getString("wifiStr4").c_str(); break;
            case 5: line = pref.getString("wifiStr5").c_str(); break;
        }
        if (line.strlen() < 5) { // line is empty
            credentials = ssid;
            credentials += "\t";
            credentials += password;
            if (i == 1) {
                pref.putString("wifiStr1", credentials.get());
                state = 2;
                goto exit;
            }
            if (i == 2) {
                pref.putString("wifiStr2", credentials.get());
                state = 2;
                goto exit;
            }
            if (i == 3) {
                pref.putString("wifiStr3", credentials.get());
                state = 2;
                goto exit;
            }
            if (i == 4) {
                pref.putString("wifiStr4", credentials.get());
                state = 2;
                goto exit;
            }
            if (i == 5) {
                pref.putString("wifiStr5", credentials.get());
                state = 2;
                goto exit;
            }
        }
    }
    state = 3;

exit:
    if (state == 0) { SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "SSID: %s password can't changed, it is hard coded" ANSI_ESC_RESET "  ", ssid.c_get()); }
    if (state == 1) { SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The passord \"%s\" for the SSID: %s has been changed" ANSI_ESC_RESET "  ", password.c_get(), ssid.c_get()); }
    if (state == 2) { SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The SSID: %s has been added" ANSI_ESC_RESET "  ", ssid.c_get()); }
    if (state == 3) { SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "No more memory to save the credentials for: %s" ANSI_ESC_RESET "  ", ssid.c_get()); }
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*****************************************************************************************************************************************************
 *                                                                     A U D I O                                                                     *
 *****************************************************************************************************************************************************/
void connecttohost(ps_ptr<char> host) {
    int32_t      idx1, idx2;
    ps_ptr<char> url;
    ps_ptr<char> user;
    ps_ptr<char> pwd;

    dispFooter.updateBitRate(0);
    s_cur_Codec = 0;
    //    if(s_state == RADIO) clearStreamTitle();
    s_icyBitRate = 0;
    s_decoderBitRate = 0;
    s_f_webFailed = false;

    idx1 = host.index_of("|", 0);
    if (idx1 == -1) { // no pipe found
        s_f_isWebConnected = audio.connecttohost(host.c_get());

        if (!s_f_isWebConnected) {
            s_cthFailCounter++;
        } else
            (s_cthFailCounter = 0);

        s_f_isFSConnected = false;
        return;
    } else { // pipe found     e.g. http://xxx.com/ext|user|pw
        idx2 = host.index_of("|", idx1 + 1);
        // MWR_LOG_INFO("idx2 = %i", idx2);
        if (idx2 == -1) { // second pipe not found
            s_f_isWebConnected = audio.connecttohost(host.c_get());

            if (!s_f_isWebConnected) {
                s_cthFailCounter++;
            } else
                (s_cthFailCounter = 0);

            s_f_isFSConnected = false;
            return;
        } else {                     // extract url, user, pwd
            url = host.substr(idx1); // extract url
            user = host.substr(idx1 + 1, idx2 - idx1 - 1);
            pwd = host.substr(idx2 + 1);
            SerialPrintfln("new host: .  %s user %s, pwd %s", url.c_get(), user.c_get(), pwd.c_get());
            s_f_isWebConnected = audio.connecttohost(url.c_get(), user.c_get(), pwd.c_get());
            s_f_isFSConnected = false;
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
        s_cur_AudioFolder = s_SD_content.getLastConnectedFolder();
        s_cur_AudioFileName = s_SD_content.getLastConnectedFileName();
        s_cur_AudioFileNr = s_SD_content.getPosByFileName(s_cur_AudioFileName.c_get());
        if (s_cur_AudioFileNr == -1) s_cur_AudioFileNr = 0;
    }
    MWR_LOG_DEBUG("Filesize %d", audio.getFileSize());
    MWR_LOG_DEBUG("FilePos %d", audio.getAudioFilePosition());
}
void stopSong() {
    audio.stopSong();
    s_f_isFSConnected = false;
    s_f_isWebConnected = false;
    if (s_f_playlistEnabled) {
        s_PLS_content.clear();
        s_f_playlistEnabled = false;
        SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "playlist stopped" ANSI_ESC_RESET "  ");
        webSrv.send("SD_playFile=", "playlist stopped");
    }
    s_f_pauseResume = false;
    s_f_playlistNextFile = false;
    s_playlistPath.reset();
}

/*****************************************************************************************************************************************************
 *                                                                    S E T U P                                                                      *
 *****************************************************************************************************************************************************/

void setup() {
    Audio::audio_info_callback = my_audio_info; // audio callback
    dlna.dlna_client_callbak(on_dlna_client);   // dlna callback
    bt_emitter.kcx_bt_emitter_callback(on_kcx_bt_emitter);
    webSrv.websrv_callbak(on_websrv);
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_set_vprintf(log_redirect_handler);
    Serial.begin(MONITOR_SPEED);
    vTaskDelay(1500); // wait for Serial to be ready
    mutex_rtc = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    Serial.print("\n\n");
    trim(Version);

    if (I2C_SDA >= 0) {
        i2cBusOne.end();
        i2cBusOne.flush();
        i2cBusOne.begin(I2C_SDA, I2C_SCL, 100000);
    }

    Serial.println("");
    Serial.println("             " ANSI_ESC_BG_MAGENTA " ***************************************************** " ANSI_ESC_RESET "   ");
    Serial.printf("             " ANSI_ESC_BG_MAGENTA " *     MiniWebRadio % 29s    * " ANSI_ESC_RESET "    \n", Version);
    Serial.println("             " ANSI_ESC_BG_MAGENTA " ***************************************************** " ANSI_ESC_RESET "    ");
    Serial.println(ANSI_ESC_RESET "   ");

    if (!get_esp_items(&s_resetReason, &s_f_FFatFound)) return;
    pref.begin("Pref", false); // instance of preferences from AccessPoint (SSID, PW ...)

#if TFT_CONTROLLER < 7
    if (TFT_CONTROLLER == 0) {
        s_h_resolution = 320;
        s_v_resolution = 240;
    } else {
        s_h_resolution = 480;
        s_v_resolution = 320;
    }
    spiBus.begin(TFT_SCK, TFT_MISO, TFT_MOSI, -1); // SPI1 for TFT
    tft.setTFTcontroller(TFT_CONTROLLER);
    tft.setDiaplayInversion(DISPLAY_INVERSION);
    tft.begin(TFT_DC); // Init TFT interface
    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tft.setBackGoundColor(TFT_BLACK);
    if (TFT_BL >= 0) {
        s_f_brightnessIsChangeable = true;
        setupBacklight(TFT_BL, 512);
    }
#elif TFT_CONTROLLER == 7
    s_h_resolution = 800;
    s_v_resolution = 480;
    tft.begin(RGB_PINS, RGB_TIMING);
    tft.setDisplayInversion(false);
    vTaskDelay(100 / portTICK_PERIOD_MS); // wait for TFT to be ready
    tft.reset();
    if (TFT_BL >= 0) {
        s_f_brightnessIsChangeable = true;
        setupBacklight(TFT_BL, 512);
    }
#elif TFT_CONTROLLER == 8
    s_h_resolution = 1024;
    s_v_resolution = 600;
    tft.begin(DSI_TIMING);
    vTaskDelay(100 / portTICK_PERIOD_MS); // wait for TFT to be ready
    if (TFT_BL >= 0) {
        s_f_brightnessIsChangeable = true;
        setupBacklight(TFT_BL, 512);
        setTFTbrightness(5);
    }
#else
    #error "wrong TFT_CONTROLLER"
#endif

#if TP_CONTROLLER < 7 // XPT2046
    tp.begin(TP_IRQ, s_h_resolution, s_v_resolution);
    tp.setVersion(TP_CONTROLLER);
    tp.setRotation(TP_ROTATION);
    tp.setMirror(TP_H_MIRROR, TP_V_MIRROR);
#elif TP_CONTROLLER == 7 // GT911
    tp.begin(&i2cBusOne, GT911_I2C_ADDRESS, s_h_resolution, s_v_resolution);
    tp.getProductID();
    tp.setVersion(TP_GT911::GT911);
    tp.setRotation(TP_ROTATION);
    tp.setMirror(TP_H_MIRROR, TP_V_MIRROR);
#elif TP_CONTROLLER == 8 // FT6x36
    tp.begin(&i2cBusOne, 0x38, s_h_resolution, s_v_resolution);
    tp.get_FT6x36_items();
    tp.setRotation(TP_ROTATION);
    tp.setMirror(TP_H_MIRROR, TP_V_MIRROR);
#else
    #error "wrong TP_CONTROLLER"
#endif

    if (IR_PIN >= 0) pinMode(IR_PIN, INPUT_PULLUP); // if ir_pin is read only, have a external resistor (~10...40KOhm)
    SerialPrintfln("setup: ...   Init SD card");
    pinMode(SD_MMC_D0, INPUT_PULLUP);
    int32_t sdmmc_frequency = SDMMC_FREQUENCY / 1000; // MHz -> KHz, default is 40MHz

#if CONFIG_IDF_TARGET_ESP32S3
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    s_f_sd_card_found = SD_MMC.begin("/sdcard", true, false, sdmmc_frequency);
#endif

#if CONFIG_IDF_TARGET_ESP32P4
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0, SD_MMC_D1, SD_MMC_D2, SD_MMC_D3);
    s_f_sd_card_found = SD_MMC.begin("/sdcard", false, false, sdmmc_frequency);
#endif

    if (!s_f_sd_card_found) {
        clearAll();
        tft.setFont(displayConfig.fonts[6]);
        tft.setTextColor(TFT_YELLOW);
        tft.writeText("SD Card Mount Failed", 0, 50, displayConfig.dispWidth, displayConfig.dispHeight, TFT_ALIGN_CENTER, TFT_ALIGN_TOP, false, false);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed" ANSI_ESC_RESET "  ");
        return;
    }
    float cardSize = ((float)SD_MMC.cardSize()) / (1024 * 1024);
    float freeSize = ((float)SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024);
    SerialPrintfln(ANSI_ESC_WHITE "setup: ....  SD card found, %.1f MB by %.1f MB free" ANSI_ESC_RESET "   ", freeSize, cardSize);
    defaultsettings();
    if (ESP.getFlashChipSize() > 80000000) { FFat.begin(); }

    if (TFT_CONTROLLER > 8) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid" ANSI_ESC_RESET "   ");

    drawImage("/common/MiniWebRadioV4.jpg", 0, 0); // Welcomescreen
    updateSettings();

    if (s_volume.volumeSteps < 21) s_volume.volumeSteps = 21;

    s_f_isWiFiConnected = connectToWiFi();

    placingGraphicObjects();
    sdr_BR_value.setValue(s_brightness);
    sdr_EQ_lowPass.setValue(s_tone.LP);
    sdr_EQ_bandPass.setValue(s_tone.BP);
    sdr_EQ_highPass.setValue(s_tone.HP);
    sdr_EQ_balance.setValue(s_tone.BAL);
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
    btn_BT_power.setValue(s_bt_emitter.enabled);
    lst_DLNA.client_and_history(&dlna, &s_dlnaHistory[0], 10);
    lst_RADIO.currentStationNr(&s_cur_station);
    clk_AC_red.alarm_time_and_days(&s_alarmdays, s_alarmtime);

    audio.setAudioTaskCore(AUDIOTASK_CORE);
    audio.setConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);
    audio.setVolumeSteps(s_volume.volumeSteps);
    audio.setVolume(0);

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setI2SCommFMT_LSB(I2S_COMM_FMT);

    SerialPrintfln("setup: ....  number of saved stations: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", staMgnt.getSumStations());
    SerialPrintfln("setup: ....  number of saved favourites: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", staMgnt.getSumFavStations());
    SerialPrintfln("setup: ....  current station number: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", s_cur_station);
    SerialPrintfln("setup: ....  current volume: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", s_volume.cur_volume);
    SerialPrintfln("setup: ....  volume steps: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", s_volume.volumeSteps);
    SerialPrintfln("setup: ....  volume after alarm: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "   ", s_volume.volumeAfterAlarm);
    SerialPrintfln("setup: ....  last connected host: " ANSI_ESC_CYAN "%s" ANSI_ESC_RESET "   ", s_settings.lastconnectedhost.c_get());
    SerialPrintfln("setup: ....  connection timeout: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms" ANSI_ESC_RESET "   ", CONN_TIMEOUT);
    SerialPrintfln("setup: ....  connection timeout SSL: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms" ANSI_ESC_RESET "   ", CONN_TIMEOUT_SSL);

    ir.begin(); // Init InfraredDecoder

    if (AMP_ENABLED >= 0) { // enable onboard amplifier
        pinMode(AMP_ENABLED, OUTPUT);
        digitalWrite(AMP_ENABLED, HIGH);
        SerialPrintfln("setup: ....  On Board Amplifier pin is: " ANSI_ESC_CYAN "%i" ANSI_ESC_RESET "   ", AMP_ENABLED);
    }

    if (s_f_isWiFiConnected) webSrv.begin(80, 81); // HTTP port, WebSocket port

    if (s_f_mute) { SerialPrintfln("setup: ....  volume is muted: (from " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET ")", s_volume.cur_volume); }
    setI2STone();

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
    dispHeader.updateVolume(s_volume.cur_volume);
    dispHeader.speakerOnOff(!s_f_mute);

    if (s_f_isWiFiConnected) {
        if (s_resetReason == ESP_RST_POWERON ||   // Simply switch on the operating voltage
            s_resetReason == ESP_RST_SW ||        // ESP.restart()
            s_resetReason == ESP_RST_SDIO ||      // The boot button was pressed
            s_resetReason == ESP_RST_DEEPSLEEP) { // Wake up
            if (s_cur_station > 0) {
                s_state = UNDEFINED;
                setStation(s_cur_station);
            } else {
                s_state = UNDEFINED;
                setStationViaURL(s_settings.lastconnectedhost.c_get(), "");
            }
        }
        if (!MDNS.begin("MiniWebRadio")) {
            SerialPrintfln("%s", "WiFI_info:   " ANSI_ESC_YELLOW "Error starting mDNS", ANSI_ESC_RESET);
        } else {
            SerialPrintfln("%s", "WiFI_info:   mDNS started");
            MDNS.addService("esp32", "tcp", 80);
            SerialPrintfln("WiFI_info:   mDNS name: " ANSI_ESC_CYAN "MiniWebRadio" ANSI_ESC_RESET);
        }
        ArduinoOTA.setHostname("MiniWebRadio");
        ArduinoOTA.begin();
        ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); // username, password for ftp.
        setRTC(s_TZString);
        s_f_dlnaSeekServer = true;
    } else {
        s_state = UNDEFINED;
        changeState(WIFI_SETTINGS, 0);
    }

    if (LIGHT_SENSOR >= 0) {
        s_f_BH1750_found = BH1750.begin(&i2cBusOne, BH1750.ADDR_TO_GROUND); // init the sensor with address pin connetcted to ground
        if (s_f_BH1750_found) {                                             // result (bool) wil be be "false" if no sensor found
            SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "Ambient Light Sensor BH1750 found at " ANSI_ESC_CYAN "0x%X" ANSI_ESC_RESET, BH1750.ADDR_TO_GROUND);
            BH1750.setResolutionMode(BH1750.ONE_TIME_H_RESOLUTION_MODE);
            BH1750.setSensitivity(BH1750.SENSITIVITY_ADJ_MAX);
        } else {
            SerialPrintfln("setup: ....  " ANSI_ESC_RED "Ambient Light Sensor BH1750 not found" ANSI_ESC_RESET);
        }
    }
    if (BT_EMITTER_RX >= 0) bt_emitter.begin();
}
/*****************************************************************************************************************************************************
 *                                                                   C O M M O N                                                                     *
 *****************************************************************************************************************************************************/

ps_ptr<char> scaleImage(ps_ptr<char> path) {
    MWR_LOG_DEBUG("path %s", path.c_get());
    bool ok = false;
    if (path.ends_with("bmp")) ok = true;
    if (path.ends_with("jpg")) ok = true;
    if (path.ends_with("gif")) ok = true;
    if (path.ends_with("png")) ok = true;
    if (path.starts_with("/png")) ok = false; // is web button
    if (!ok) return path;

    int idx = path.index_of('/', 1);
    if (idx < 0) return path; // invalid path
    ps_ptr<char> tfts = displayConfig.tftSize;
    tfts += "/";
    path.insert(tfts.c_get(), idx + 1); // "/logo/0N 90s.jpg" --> "/logo/s/0N 90s.jpg"
    MWR_LOG_DEBUG("path %s", path.c_get());
    return path;
}

void setVolume(uint8_t vol) {
    static int16_t oldVol = -1;
    if (vol == oldVol) return;
    MWR_LOG_DEBUG("volume old: %i. new: %i", oldVol, vol);
    s_volume.cur_volume = vol;
    oldVol = vol;
    dispHeader.updateVolume(s_volume.cur_volume);
    sdr_CL_volume.setValue(s_volume.cur_volume);
    sdr_DL_volume.setValue(s_volume.cur_volume);
    sdr_PL_volume.setValue(s_volume.cur_volume);
    sdr_RA_volume.setValue(s_volume.cur_volume);
    SerialPrintfln("action: ...  current volume is " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "  ", s_volume.cur_volume);
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
    if (sta == 0) { setStationViaURL(s_settings.lastconnectedhost.c_get(), ""); }
    if (sta > staMgnt.getSumStations()) sta = s_cur_station;
    s_stationURL = staMgnt.getStationUrl(sta);
    SerialPrintfln("action: ...  switch to station " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "  ", sta);
    s_homepage = "";
    s_streamTitle = "";
    s_icyDescription = "";
    s_f_newStreamTitle = true;
    s_f_newIcyDescription = true;
    if (s_f_isWebConnected && sta == old_cur_station && s_state == RADIO) { // Station is already selected
        //    s_f_newStreamTitle = true;
    } else {
        connecttohost(s_stationURL.c_get());
    }
    changeState(RADIO, 0);
    old_cur_station = sta;
    showLogoAndStationName(true);
    if (s_cur_station == 0) {
        dispFooter.updateFlag(NULL);
    } else {
        dispFooter.updateFlag(getFlagPath(s_cur_station));
    }
    dispFooter.updateStation(s_cur_station);
}

void setStationViaURL(const char* url, const char* extension) {
    // e.g.  http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one&bitrate=96000
    // url is http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one, extension is bitrate=96000
    s_stationName_air.reset();

    s_cur_station = 0;
    ps_ptr<char> origin_url = url;
    if (strlen(extension) > 0) origin_url.appendf("&%s", extension);
    s_stationURL = origin_url;
    connecttohost(origin_url);
    changeState(RADIO, 0);
    clearStreamTitle();
    showLogoAndStationName(true);
    dispFooter.updateFlag(NULL);
    dispFooter.updateStation(0); // set 000
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

void savefile(ps_ptr<char> fileName, uint32_t contentLength, ps_ptr<char> contentType) { // save the uploadfile on SD_MMC

    if (!fileName.starts_with("/")) { fileName = "/" + fileName; }
    if (webSrv.uploadfile(SD_MMC, fileName, contentLength, contentType)) {
        SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " in progress" ANSI_ESC_RESET "  ", fileName.c_get());
        webSrv.sendStatus(200);
    } else {
        SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD failed" ANSI_ESC_RESET "  ", fileName.c_get());
        webSrv.sendStatus(400);
    }
}

void saveImage(const char* fileName, uint32_t contentLength) { // save the jpg image on SD_MMC
    ps_ptr<char> fn;

    if (endsWith(fileName, "jpg")) {
        fn.assign("/logo/");
        fn.append(displayConfig.tftSize);
        if (!startsWith(fileName, "/")) fn.append("/");
        fn.append(fileName);
        if (webSrv.uploadB64image(SD_MMC, fn.c_get(), contentLength)) {
            SerialPrintfln("save image (jpg) " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully" ANSI_ESC_RESET "  ", fn.c_get());
            webSrv.sendStatus(200);
        } else
            webSrv.sendStatus(400);
    }
}

void setI2STone() {
    audio.setTone(s_tone.LP, s_tone.BP, s_tone.HP);
    audio.setBalance(s_tone.BAL);
    return;
}

ps_ptr<char> getI2STone() {
    ps_ptr<char> tone;
    tone.assignf("LowPass=%i\nBandPass=%i\nHighPass=%i\nBalance=%i\n", s_tone.LP, s_tone.BP, s_tone.HP, s_tone.BAL);
    return tone;
}

void SD_playFile(ps_ptr<char> pathWoFileName, const char* fileName) { // pathWithoutFileName e.g. /audiofiles/playlist/
    pathWoFileName += fileName;
    int32_t idx = pathWoFileName.index_of("\033[", 1);
    if (idx == -1) { // do nothing
        SD_playFile(pathWoFileName, 0, true);
        return;
    }
    SD_playFile(pathWoFileName.substr(0, idx), 0, true); // remove color and filesize
    return;
}

void SD_playFile(ps_ptr<char> path, uint32_t fileStartTime, bool showFN) {
    if (!path.valid()) return; // avoid a possible crash

    if (path.ends_with("m3u")) {
        if (playlist.create_playlist_from_file(path)) s_f_playlistEnabled = true;
        return;
    }

    ps_ptr<char> file_name;

    if (s_subState_player != 1) { changeState(PLAYER, 1); }
    int32_t idx = path.last_index_of('/');
    if (idx < 0) return;
    s_cur_AudioFolder = path.substr(0, idx);
    file_name = path.substr(idx + 1); // without '/'

    if (showFN) {
        clearLogo();
        showFileName(path.get() + idx + 1);
    }

    SerialPrintfln("AUDIO_FILE:  " ANSI_ESC_MAGENTA "%s" ANSI_ESC_RESET, file_name.c_get());
    connecttoFS("SD_MMC", (const char*)path.c_get(), fileStartTime);
    if (s_f_playlistEnabled) showPlsFileNumber();
    if (s_f_isFSConnected) { s_settings.lastconnectedfile = path; }
}

bool SD_rename(const char* src, const char* dest) {
    bool success = false;
    if (SD_MMC.exists(src)) { success = SD_MMC.rename(src, dest); }
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
    muteChanged(true);
    s_f_playlistEnabled = false;
    s_f_isFSConnected = false;
    s_f_isWebConnected = false;
    audio.stopSong();
    if (s_sleepMode == 0) {
        clearAll();
        setTFTbrightness(0);
    } else {
        changeState(CLOCK, 0);
    }
    if (s_bt_emitter.found) bt_emitter.power_off();
    SerialPrintfln("falling asleep");
    dispHeader.hide();
    dispFooter.hide();
}

void wake_up() {
    s_f_sleeping = false;
    muteChanged(false);
    SerialPrintfln("awake");
    clearAll();
    setTFTbrightness(s_brightness);
    if (s_cur_station) {
        setStation(s_cur_station);
    } else {
        connecttohost(s_settings.lastconnectedhost.get());
    }
    changeState(RADIO, 0);
    showLogoAndStationName(true);
    dispHeader.show(true);
    dispHeader.speakerOnOff(!s_f_mute);
    dispHeader.updateRSSI(WiFi.RSSI(), true);
    dispFooter.show(true);
    if (s_bt_emitter.found && s_bt_emitter.enabled) bt_emitter.power_on();
}

void setRTC(ps_ptr<char> TZString) {
    rtc.stop();
    rtc.begin(TZString.c_get());
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
    MWR_LOG_DEBUG("file length %i, written %i", file1.size(), len);
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
        if (AMP_ENABLED != -1) {
            digitalWrite(AMP_ENABLED, LOW);
            SerialPrintfln("mute:  ....  On Board Amplifier is off");
        }
        webSrv.send("mute=", "1");
    } else {
        s_f_mute = false;
        if (AMP_ENABLED != -1) {
            digitalWrite(AMP_ENABLED, HIGH);
            SerialPrintfln("mute:  ....  On Board Amplifier is on");
        }
        webSrv.send("mute=", "0");
    }
    dispHeader.speakerOnOff(!s_f_mute);
    dispHeader.updateVolume(s_volume.cur_volume);
    updateSettings();
};

void logAlarmItems() {
    const char wd[7][11] = {"Sunday:   ", "Monday:   ", "Tuesday:  ", "Wednesday:", "Thursday: ", "Friday:   ", "Saturday: "};
    uint8_t    mask = 0b00000001;
    for (uint8_t i = 0; i < 7; i++) {
        if (s_alarmdays & mask) {
            SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s " ANSI_ESC_CYAN "%02i:%02i" ANSI_ESC_RESET "  ", wd[i], s_alarmtime[i] / 60, s_alarmtime[i] % 60);
        } else {
            SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s No alarm is set" ANSI_ESC_RESET "  ", wd[i]);
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

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                              C H A N G E    S T A T E                                                                       ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */
// clang-format off
/*🟢🟡🔴*/
void changeState(int8_t state, int8_t subState) {
    MWR_LOG_DEBUG("state %i, s_state %i, subState %i, s_subState_radio %i, s_subState_player %i", state, s_state, subState, s_subState_radio, s_subState_player);
    bool newState = false;
    bool newSubState = false;
    disableAllObjects();
    setTimeCounter(0);
    if (state == RADIO          && s_state != RADIO)              { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == STATIONSLIST   && s_state != STATIONSLIST)       { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == PLAYER         && s_state != PLAYER)             { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == AUDIOFILESLIST && s_state != AUDIOFILESLIST)     { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == DLNA           && s_state != DLNA)               { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == DLNAITEMSLIST  && s_state != DLNAITEMSLIST)      { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == CLOCK          && s_state != CLOCK)              { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == ALARMCLOCK     && s_state != ALARMCLOCK)         { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == SLEEPTIMER     && s_state != SLEEPTIMER)         { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == SETTINGS       && s_state != SETTINGS)           { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == BRIGHTNESS     && s_state != BRIGHTNESS)         { dispHeader.show(false); dispFooter.show(false); newState = true;}
    if (state == EQUALIZER      && s_state != EQUALIZER)          { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == BLUETOOTH      && s_state != BLUETOOTH)          { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == IR_SETTINGS    && s_state != IR_SETTINGS)        { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == RINGING        && s_state != RINGING)            { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == WIFI_SETTINGS  && s_state != WIFI_SETTINGS)      { dispHeader.show(true);  dispFooter.show(true);  newState = true;}
    if (state == RADIO          && s_subState_radio  != subState) { newSubState = true;  }
    if (state == PLAYER         && s_subState_player != subState) { newSubState = true;  }
    if (state == CLOCK          && s_subState_clock  != subState) { newSubState = true;  }

    s_subState_radio  = UNDEFINED;
    s_subState_player = UNDEFINED;
    s_subState_clock  = UNDEFINED;

    s_f_volBarVisible = false;
    dispHeader.updateItem(_hl_item[state]);
    if (state != RADIO) { dispFooter.updateFlag(NULL); }

    switch (state) {
        case RADIO: {
            if (newState) {
                clearWithOutHeaderFooter();
                dispFooter.updateFlag(getFlagPath(s_cur_station));
                webSrv.send("changeState=", "RADIO");
                if(!s_f_isWebConnected){
                    if (s_cur_station) { setStation(s_cur_station); }
                    else               { connecttohost(s_settings.lastconnectedhost.get()); }
                }
                if(s_f_isWebConnected) showLogoAndStationName(true);
            }
            dispHeader.enable();
            dispFooter.enable();
            txt_RA_staName.enable();
            pic_RA_logo.enable();
            if (subState == 0) {
                if(newSubState) {
                    VUmeter_RA.show(true);
                    txt_RA_sTitle.setText("");
                    txt_RA_sTitle.show(true, false);
                    s_f_newIcyDescription = true;
                    s_f_newStreamTitle = true;
                }
                else {
                    VUmeter_RA.enable();
                    txt_RA_sTitle.enable();
                    txt_RA_sTitle.enable();
                }
                setTimeCounter(0);
            }
            if (subState == 1) {  // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                if(newSubState) {
                    txt_RA_sTitle.hide();
                    VUmeter_RA.hide();
                    sdr_RA_volume.show(true, false);
                    btn_RA_mute.show(); btn_RA_prevSta.show(); btn_RA_nextSta.show();
                    setTimeCounter(2);
                }
                else{
                    sdr_RA_volume.enable();
                    btn_RA_mute.enable(); btn_RA_prevSta.enable(); btn_RA_nextSta.enable();
                }
            }
            if (subState == 2){ // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                if(newSubState) {
                    txt_RA_sTitle.hide();
                    VUmeter_RA.hide();
                    sdr_RA_volume.hide();
                    btn_RA_staList.show();
                    btn_RA_player.show(); btn_RA_dlna.show(); btn_RA_clock.show(); btn_RA_sleep.show(); btn_RA_settings.show();
                    btn_RA_bt.show(!s_bt_emitter.found);
                    btn_RA_off.show();
                    setTimeCounter(2);
                }
                else {
                    btn_RA_staList.enable();
                    btn_RA_player.enable(); btn_RA_dlna.enable(); btn_RA_clock.enable(); btn_RA_sleep.enable(); btn_RA_settings.enable();
                    btn_RA_bt.enable();
                    btn_RA_off.enable();
                }
            }
            s_subState_radio = subState;
            break;
        }

        case STATIONSLIST: {
            clearWithOutHeaderFooter();
            lst_RADIO.show();
            setTimeCounter(LIST_TIMER);
            break;
        }
        case PLAYER: {
            if (newState) {
                stopSong();
                clearWithOutHeaderFooter();
                webSrv.send("changeState=", "PLAYER");
            }
            dispHeader.enable();
            dispFooter.enable();
            pic_PL_logo.enable();
            if (subState == 0){
                s_SD_content.listFilesInDir(s_cur_AudioFolder.c_get(), true, false);
                s_cur_Codec = 0;
                showFileLogo(PLAYER, subState);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                txt_PL_fName.show(true, false);
                pgb_PL_progress.hide();
                sdr_PL_volume.hide();
                txt_PL_fNumber.show(true, false);
                showAudioFileNumber();
                btn_PL_prevFile.show(); btn_PL_nextFile.show(); btn_PL_ready.show(); btn_PL_playAll.show();
                btn_PL_shuffle.show();  btn_PL_fileList.show(); btn_PL_radio.show(); btn_PL_off.show();
            }
            if (subState == 1){
                pgb_PL_progress.setValue(0);
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                if(newSubState){
                    btn_PL_fileList.hide(); btn_PL_radio.hide(); btn_PL_off.hide();
                    pgb_PL_progress.show(true, false);
                    sdr_PL_volume.show(true, true);
                    txt_PL_fName.show(true, false);
                    txt_PL_fNumber.show(true, false);
                    btn_PL_mute.show(); btn_PL_pause.setOff(); btn_PL_pause.show(); btn_PL_cancel.show(); btn_PL_playPrev.show(); btn_PL_playNext.show();
                }
                else{
                    pgb_PL_progress.enable();
                    sdr_PL_volume.enable();
                    txt_PL_fName.enable();
                    txt_PL_fNumber.enable();
                    btn_PL_mute.enable(); btn_PL_pause.enable(); btn_PL_pause.enable(); btn_PL_cancel.enable();  btn_PL_playPrev.enable(); btn_PL_playNext.enable();
                }
            }
            s_subState_player = subState;
            break;
        }
        case AUDIOFILESLIST: {
            clearWithOutHeaderFooter();
            lst_PLAYER.show(s_cur_AudioFolder, s_cur_AudioFileNr);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case DLNA: {
            if (newState && s_state != DLNAITEMSLIST) audio.stopSong();
            clearWithOutHeaderFooter();
            pic_DL_logo.enable();
            dispHeader.enable();
            dispFooter.enable();
            pgb_DL_progress.setValue(0);
            pgb_DL_progress.show(true, false);
            txt_DL_fName.show(true, false);
            showFileLogo(DLNA, subState);
            webSrv.send("changeState=", "DLNA");
            if (audio.isRunning()) btn_DL_pause.setActive(true);
            else                   btn_DL_pause.setActive(false);
            sdr_DL_volume.show(true, false);
            btn_DL_pause.show(); btn_DL_mute.show(); btn_DL_cancel.show(); btn_DL_fileList.show(); btn_DL_radio.show();
            break;
        }
        case DLNAITEMSLIST: {
            lst_DLNA.show(s_currDLNAsrvNr, dlna.getServer(), dlna.getBrowseResult(), &s_dlnaLevel, s_dlnaMaxItems, s_dlnaMaXServers);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case CLOCK: {
            if (newState) { bgColorWithOutHeaderFooter(); }
            dispHeader.enable();
            dispFooter.enable();
            clk_CL_24.show();
            if (subState == 0) {
                btn_CL_mute.hide(); btn_CL_alarm.hide(); btn_CL_radio.hide(); sdr_CL_volume.hide(); btn_CL_off.hide();
            }
            if (subState == 1) {
                setTimeCounter(2);
                sdr_CL_volume.show(false, false);
                btn_CL_mute.show(); btn_CL_alarm.show(); btn_CL_radio.show(); btn_CL_off.show();
            }
            s_subState_clock = subState;
            break;
        }
        case ALARMCLOCK: {
            if (newState) bgColorWithOutHeaderFooter();
            dispHeader.enable();
            dispFooter.enable();
            btn_AC_left.show(); btn_AC_right.show(); btn_AC_up.show(); btn_AC_down.show(); btn_AC_ready.show(); clk_AC_red.show();
            break;
        }
        case SLEEPTIMER: {
            dispHeader.enable(); dispFooter.enable();
            if (newState) {
                clearWithOutHeaderFooter();
                otb_SL_stime.show(s_sleeptime);
                pic_SL_logo.setPicturePath("/common/Night_Gown.jpg");
                pic_SL_logo.align(true, true);
                pic_SL_logo.show(false, false);
            }
            btn_SL_up.show(); btn_SL_up.show(); btn_SL_down.show(); btn_SL_ready.show(); btn_SL_cancel.show();
            break;
        }
        case SETTINGS: {
            dispHeader.enable(); dispFooter.enable();
            if (newState) {
                clearWithOutHeaderFooter();
                showFileLogo(SETTINGS, subState);
            }
            btn_SE_bright.show(!s_f_brightnessIsChangeable);
            btn_SE_bright.show(); btn_SE_equal.show(); btn_SE_wifi.show(); btn_SE_radio.show();
            break;
        }
        case BRIGHTNESS: {
            dispHeader.enable(); dispFooter.enable();
            if (newState) {
                clearWithOutHeaderFooter();
                pic_BR_logo.show(false, false);
                sdr_BR_value.setValue(s_brightness);
                sdr_BR_value.show(true, true);
                txt_BR_value.setText(int2str(s_brightness));
                txt_BR_value.show(true, true);
            } else {
                sdr_BR_value.enable();
                txt_BR_value.enable();
            }
            btn_BR_ready.show();
            break;
        }
        case EQUALIZER:
            dispHeader.enable(); dispFooter.enable();
            if (newState) clearWithOutHeaderFooter();
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
            btn_EQ_Radio.show();
            break;

        case BLUETOOTH: {
            dispHeader.enable(); dispFooter.enable();
            clearWithOutHeaderFooter();
            btn_BT_volUp.show(); btn_BT_volDown.show(); btn_BT_pause.show(); btn_BT_mode.show();
            btn_BT_radio.show(); btn_BT_power.show();
            pic_BT_mode.show(true, false);
            if (s_bt_emitter.mode.equals("RX")) { txt_BT_mode.writeText("RECEIVER");}
            else                                { txt_BT_mode.writeText("EMITTER"); }
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
            dispHeader.enable(); dispFooter.enable();
            clearWithOutHeaderFooter();
            btn_IR_radio.show();
            break;
        case RINGING:
            dispHeader.enable(); dispFooter.enable();
            clearWithOutHeaderFooter();
            if (s_volume.ringVolume > 0) { // alarm with bell
                pic_RI_logo.enable();
                showFileLogo(RINGING, subState);
                setTFTbrightness(s_brightness);
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm" ANSI_ESC_RESET "  ");
                setVolume(s_volume.ringVolume);
                audio.setVolume(s_volume.ringVolume);
                muteChanged(false);
                connecttoFS("SD_MMC", "/ring/alarm_clock.mp3");
                clk_RI_24small.show();
            } else { // alarm without bell
                s_f_eof_alarm = true;
            }
            break;

        case WIFI_SETTINGS:
            dispHeader.enable(); dispFooter.enable();
            clearWithOutHeaderFooter();
            cls_wifiSettings.clearText();
            cls_wifiSettings.setBorderWidth(1);
            cls_wifiSettings.setFontSize(displayConfig.listFontSize);
            int16_t n = WiFi.scanNetworks();
            SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "%i WiFi networks found" ANSI_ESC_RESET "  ", n);
            for (int i = 0; i < n; i++) {
                SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)" ANSI_ESC_RESET "  ", WiFi.SSID(i).c_str(), (int16_t)WiFi.RSSI(i));
                ps_ptr<char> pw = get_WiFi_PW(WiFi.SSID(i).c_str());
                cls_wifiSettings.add_WiFi_Items(WiFi.SSID(i).c_str(), pw.c_get());
            }
            cls_wifiSettings.show(false, false);
            break;
    }
    s_ir_btn_select = UNDEFINED;
    s_state = state;
}
// clang-format on
/*🟢🟡🔴*/
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ps_ptr<char> get_WiFi_PW(const char* ssid) {
    ps_ptr<char> line;
    ps_ptr<char> password = "";

    for (int j = 0; j < 6; j++) {
        if (j == 0) line = pref.getString("wifiStr0").c_str();
        if (j == 1) line = pref.getString("wifiStr1").c_str();
        if (j == 2) line = pref.getString("wifiStr2").c_str();
        if (j == 3) line = pref.getString("wifiStr3").c_str();
        if (j == 4) line = pref.getString("wifiStr4").c_str();
        if (j == 5) line = pref.getString("wifiStr5").c_str();
        if (line.starts_with(ssid) && line[strlen(ssid)] == '\t') {
            int idx = line.index_of("\t", 0);
            password = line.substr(idx + 1);
        }
    }
    return password;
}

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                    L O O P                                                                                  ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

void loop() {
    vTaskDelay(1);
    dlna.loop();
    audio.loop();
    webSrv.loop();
    ftpSrv.handleFTP();
    ir.loop();
    tp.loop();
    ArduinoOTA.handle();
    bt_emitter.loop();
    tft.loop();
    BH1750.loop();

    while (s_logBuffer.size() > 0) {
        size_t i = s_logBuffer.size();
        webSrv.send("serTerminal=", s_logBuffer[i - 1].c_get());
        s_logBuffer.pop_back();
        if (s_logBuffer.size() == 0) s_logBuffer.clear(); // Löscht alle Elemente und gibt den Speicher frei
    }

    if (s_f_dlnaBrowseServer) {
        s_f_dlnaBrowseServer = false;
        dlna.browseServer(s_currDLNAsrvNr, s_dlnaHistory[s_dlnaLevel].objId.c_get(), s_totalNumberReturned);
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
        if (!audio.isRunning() && !s_f_pauseResume) { processPlaylist(); }
    }
    //-----------------------------------------------------0.1 SEC------------------------------------------------------------------------------------
    if (s_f_100ms) { // calls every 0.1 second
        s_f_100ms = false;

        if (s_state == RADIO && s_subState_radio == 0) VUmeter_RA.update(audio.getVUlevel());

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
                        if (s_subState_radio == 1) { changeState(RADIO, 0); }                  // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                        if (s_subState_radio == 2) { changeState(RADIO, 0); }                  // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                    } else if (s_state == STATIONSLIST) {
                        changeState(RADIO, 0);
                    } else if (s_state == PLAYER) {
                        if (!txt_PL_fName.isEnabled()) { txt_PL_fName.show(true, false); } // assume volBox is shown
                    } else if (s_state == AUDIOFILESLIST) {
                        changeState(PLAYER, 0);
                    } else if (s_state == DLNA) {
                        if (!txt_DL_fName.isEnabled()) { txt_DL_fName.show(true, false); } // assume volBox is shown
                    } else if (s_state == DLNAITEMSLIST) {
                        changeState(DLNA, 0);
                    } else if (s_state == CLOCK) {
                        changeState(CLOCK, 0);
                    } else {
                        ;
                    } // all other, do nothing
                }
            }
        }

        if (!s_f_rtc) { s_f_rtc = rtc.hasValidTime(); }
        // ------------------------------------------- volume / mute --------------------------------------------------------------------------------
        if (!s_f_mute) {
            if (audio.getVolume() != s_volume.cur_volume) { audio.setVolume(s_volume.cur_volume); }
        } else {
            if (audio.getVolume() != 0) { audio.setVolume(0); }
        }

        // ------------------------------------------- message box ----------------------------------------------------------------------------------
        if (s_f_msg_box) {                // messagebox is visible?
            if (s_timestamp < millis()) { // time to hide
                s_f_msg_box = false;
                msg_box.hide();
                if (s_f_esp_restart) { // restart after time
                    s_f_esp_restart = false;
                    ESP.restart();
                }
            }
        }
    }
    //----------------------------------------------------- 1 SEC ------------------------------------------------------------------------------------

    if (s_f_1sec) { // calls every second
        s_f_1sec = false;
        s_totalRuntime++;
        // for(int i = 0; i< 3; i++){
        //     uint8_t* sa = audio.getSpectrum();
        //     MWR_LOG_INFO("%i, %i, %i", sa[0], sa[1], sa[2]);
        // }
        uint16_t minuteOfTheDay = rtc.getMinuteOfTheDay();
        uint8_t  weekDay = rtc.getweekday();
        clk_CL_24.updateTime(minuteOfTheDay, weekDay);
        if (s_state == RINGING) clk_RI_24small.updateTime(minuteOfTheDay, weekDay);
        static uint8_t semaphore = 0;
        if (!semaphore) { s_f_alarm = isAlarm(weekDay, s_alarmdays, minuteOfTheDay, s_alarmtime) && s_f_rtc; } // alarm if rtc and CL green
        if (s_f_alarm) { semaphore++; }
        if (semaphore) { semaphore++; }
        if (semaphore >= 65) { semaphore = 0; }

        //------------------------------------------ALARM MANAGEMENT----------------------------------------------------------------------------------
        if (s_f_alarm) {
            s_f_alarm = false;
            changeState(RINGING, 0);
        }
        if (s_f_eof_alarm) { // AFTER RINGING
            s_f_eof_alarm = false;
            if (!s_f_rtc) return;
            s_volume.cur_volume = s_volume.volumeAfterAlarm;
            setVolume(s_volume.cur_volume);
            audio.setVolume(s_volume.cur_volume);
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
            dispHeader.updateTime(s_time_s.c_get(), false);
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
            ps_ptr<char> hh = s_time_s.substr(0, 2);
            int          hour = hh.to_uint32();
            hour++;
            if (hour == 24) hour = 0; //  extract the hour
            if (s_f_mute) return;
            if (s_f_sleeping) return;
            if (s_state != RADIO) return;
            if (s_f_timeAnnouncement) {
                f_resume = true;
                s_f_eof = false;
                ps_ptr<char> p;
                p.assignf("/voice_time/%s/%d_00.mp3", s_timeSpeechLang.c_get(), hour);
                SerialPrintfln("Time: ...... play Audiofile %s", p.c_get());
                connecttoFS("SD_MMC", p.c_get());
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
                setStationViaURL(s_settings.lastconnectedhost.get(), "");
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
                if (s_streamTitle.strlen())
                    showStreamTitle(s_streamTitle);
                else if (s_icyDescription.strlen()) {
                    showStreamTitle(s_icyDescription);
                    s_f_newIcyDescription = false;
                    webSrv.send("icy_description=", s_icyDescription.c_get());
                } else
                    txt_RA_sTitle.writeText("");
            }
            webSrv.send("streamtitle=", s_streamTitle.c_get());
        }
        if (s_f_newLyrics) {
            s_f_newLyrics = false;
            if (s_state == RADIO) showStreamTitle(s_lyrics);
            if (s_state == PLAYER) showFileName(s_lyrics.c_get());
        }
        //------------------------------------------NEW ICY-DESCRIPTION-------------------------------------------------------------------------------
        if (s_f_newIcyDescription && !s_timeCounter.timer) {
            if (s_state == RADIO) {
                if (!s_streamTitle.strlen()) showStreamTitle(s_icyDescription);
            }
            webSrv.send("icy_description=", s_icyDescription.c_get());
            s_f_newIcyDescription = false;
        }
        //------------------------------------------DETERMINE AUDIOCODEC------------------------------------------------------------------------------
        if (s_cur_Codec == 0) {
            uint8_t c = audio.getCodec();
            if (c != 0 && c != 8 && c < 10) { // unknown or OGG, guard: c {1 ... 7, 9}
                s_cur_Codec = c;
                SerialPrintfln("Audiocodec:  " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", codecname[c]);
                if (s_state == PLAYER) showFileLogo(PLAYER, s_subState_player);
                // if (s_state == RADIO && s_f_logoUnknown == true) {
                //     s_f_logoUnknown = false;
                //     showFileLogo(s_state, s_subState_radio);
                // }
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
        if (s_f_reconnect && !s_f_isWiFiConnected) { // not used yet
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
            //    if( playlist.create_playlist_from_DLNA_folder()) s_f_playlistEnabled = true;
        }
        //------------------------------------------DLNA ITEMS RECEIVED-------------------------------------------------------------------------------
        if (s_f_dlna_browseReady) { // unused
            s_f_dlna_browseReady = false;
        }
        //-------------------------------------------WIFI DISCONNECTED?-------------------------------------------------------------------------------
        if (WiFi.isConnected() == false) {
            SerialPrintfln("WiFi      :  " ANSI_ESC_YELLOW "Reconnecting to WiFi...");
            dispHeader.updateRSSI(-86);
            s_f_WiFi_lost = true;
        } else {
            if (s_f_WiFi_lost) {
                s_f_WiFi_lost = false;
                if (s_state == RADIO) audio.connecttohost(s_settings.lastconnectedhost.get());
            }
        }
        //------------------------------------------GET AUDIO FILE ITEMS------------------------------------------------------------------------------
        if (s_f_isFSConnected) {
            //    uint32_t t = 0;
            //    uint32_t fs = audioGetFileSize();
            //    uint32_t br = audioGetBitRate();
            //    if(br) t = (fs * 8)/ br;
            //    MWR_LOG_DEBUG("Br %d, Dur %ds", br, t);
        }
        //--------------------------------------------- BT EMITTER ----------------------------------------------------------------------------------
        if (s_bt_emitter.found) {
            if (s_bt_emitter.enabled) {
                if (!s_f_sleeping) {
                    if (!bt_emitter.get_power_state()) bt_emitter.power_on();
                } else {
                    if (bt_emitter.get_power_state()) bt_emitter.power_off();
                }
            } else {
                if (bt_emitter.get_power_state()) { bt_emitter.power_off(); }
            }
        }
    } //  END s_f_1sec
    //------------------------------------------------------------------------------------------------------------------------------------------------
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
        SerialPrintfln("Terminal  :  " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", r.c_str());
        if (r.startsWith("pr")) {
            s_f_pauseResume = audio.pauseResume();
            if (s_f_pauseResume) {
                SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume" ANSI_ESC_RESET "  ");
            } else {
                SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume not possible" ANSI_ESC_RESET "  ");
            }
        }
        if (r.startsWith("hc")) { // A make_hardcopy_on_sd of the display is created and written to the SD card
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "create hardcopy" ANSI_ESC_RESET "  "); }
            make_hardcopy_on_sd();
        }
        if (r.startsWith("rts")) { // run time stats
            char* timeStatsBuffer = x_ps_calloc(2000, sizeof(char));
            GetRunTimeStats(timeStatsBuffer);
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "task statistics\n\n%s" ANSI_ESC_RESET "  ", timeStatsBuffer); }
            x_ps_free(&timeStatsBuffer);
        }
        if (r.startsWith("cts")) { // connect to speech
            audio.connecttospeech("Hallo, wie geht es dir? Morgen scheint die Sonne und übermorgen regnet es.Aber wir nehmen den Regenschirm mit. Und auch den Rucksack. Dann lesen wir aus dem Buch "
                                  "Hier gibt es nur gutes Wetter.",
                                  "de");
            //    audio.connecttospeech("Hallo", "de");
        }

        if (r.startsWith("bfi")) { // buffer filled
            SerialPrintfln("inBuffer  :  filled %lu bytes" ANSI_ESC_RESET "  ", (long unsigned)audio.inBufferFilled());
            SerialPrintfln("inBuffer  :  free   %lu bytes" ANSI_ESC_RESET "  ", (long unsigned)audio.inBufferFree());
        }
        if (r.startsWith("st")) { // testtext for streamtitle
            if (r[2] == '0') s_streamTitle = "A Ё Ю";
            if (r[2] == '1') s_streamTitle = "A B C D E F G";
            if (r[2] == '2') s_streamTitle = "A B C D E F G H I";
            if (r[2] == '3') s_streamTitle = "A B C D E F G H I J K L";
            if (r[2] == '4') s_streamTitle = "A B C D E F G H I J K J M Q O";
            if (r[2] == '5') s_streamTitle = "A B C D E F G H I K L J M y O P Q R";
            if (r[2] == '6')
                s_streamTitle = "A B C D E F G H I K L J M g O P Q R S T V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T "
                                "U V W K J Q p O P Q R S T U V W K J Q V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U "
                                "V W K J Q p O P Q R S T U V W K J Q";
            if (r[2] == '7')
                s_streamTitle = "A B C D E F G H I K L J M j O P Q R S T U V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S "
                                "T U V W K J Q p O P Q R S T U V W K J Q";
            if (r[2] == '8') s_streamTitle = "A B C D E F G H I K L J M p O P Q R S T U V W A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q";
            if (r[2] == '9') s_streamTitle = "A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q";
            MWR_LOG_INFO("st: %s", s_streamTitle.c_get());
            s_f_newStreamTitle = true;
        }
        if (r.startsWith("ais")) { // openAIspeech
            MWR_LOG_INFO("openAI speech");
            //    audio.openai_speech("openAI-key", "tts-1", "Today is a wonderful day to build something people love!", "", "shimer", "mp3", "1");
        }
        if (r.startsWith("ctfs")) { // connecttoFS
                                    //     MWR_LOG_INFO("SPIFFS");
            connecttoFS("SD", "/Collide.ogg");
        }
        if (r.startsWith("stoff")) { // setTimeOffset
            int32_t t = r.substring(3, r.length() - 1).toInt();
            MWR_LOG_INFO("setTimeOffset %li", t);
            audio.setTimeOffset(t);
        }

        if (r.startsWith("sapt")) { // setAudioPlayTime
            uint32_t t = r.substring(4, r.length() - 1).toInt();
            MWR_LOG_INFO("setAudioPlayTime %lu", t);
            audio.setAudioPlayTime(t);
        }

        if (r.startsWith("gafp")) { // getAudioFilePosition
            MWR_LOG_INFO("getAudioFilePosition %lu", audio.getAudioFilePosition());
        }

        if (r.startsWith("safp")) { // setAudioFilePosition
            uint32_t t = r.substring(4, r.length() - 1).toInt();
            MWR_LOG_INFO("setAudioFilePosition %lu", t);
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
                MWR_LOG_INFO("mono");
            else
                MWR_LOG_INFO("stereo");
        }
        if (r.startsWith("btp")) { // bluetooth RX/TX protocol
            bt_emitter.list_protokol();
        }
        if (r.startsWith("btstr")) { // bluetooth string, send to bt emitter e.g. btstr:AT+
            bt_emitter.userCommand(r.substring(6, r.length() - 1).c_str());
            MWR_LOG_INFO("btstr: %s", r.substring(6, r.length() - 1).c_str());
        }
        if (r.startsWith("tsp")) { s_f_timeSpeech = true; }
        if (r.startsWith("pwd")) { // set password for WiFi
            changeState(WIFI_SETTINGS, 0);
        }
        if (r.startsWith("gif")) { // draw gif image
            MWR_LOG_INFO("gif");
            drawImage("/common/Tom_Jerry.gif", 100, 100);
        }
        static uint32_t time = 0;
        if (r.startsWith("stops")) { // stop song
            time = audio.stopSong();
            MWR_LOG_INFO("file %s stopped at time %lu", s_cur_AudioFileName.c_get(), time);
        }
        if (r.startsWith("starts")) { // start song
            ps_ptr<char> path = "/audiofiles/" + s_cur_AudioFileName;
            bool         ret = audio.connecttoFS(SD_MMC, path.c_get(), time);
            MWR_LOG_INFO("file %s started at time %lu, ret %i", s_cur_AudioFileName.c_get(), time, ret);
        }

        if (r.startsWith("gbr")) { // get bitrate
            uint32_t br = audio.getBitRate();
            MWR_LOG_INFO("bitrate: %lu", br);
        }
        if (r.startsWith("ibs")) { // inbuff status
            audio.inBufferStatus();
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
                s_streamTitle.assignf(ANSI_ESC_ORANGE "%s", m.msg);
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
                SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s" ANSI_ESC_RESET "  ", m.msg);
                return;
            } // all other
            break;

        case Audio::evt_name:
            s_stationName_air = m.msg; // set max length
            SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s" ANSI_ESC_RESET "  ", m.msg);
            s_f_newStationName = true;
            break;

        case Audio::evt_streamtitle:
            s_streamTitle = m.msg;
            SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", m.msg);
            s_f_newStreamTitle = true;
            break;

        case Audio::evt_eof:
            s_f_isWebConnected = false;
            s_f_eof = true;
            s_f_isFSConnected = false;
            SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, m.msg);
            if (s_state == PLAYER) {
                webSrv.send("SD_playFile=", "end of audiofile");
                if (!s_f_playlistEnabled) {
                    //    s_f_clearLogo = true;
                    //    s_f_clearStationName = true;
                    changeState(PLAYER, 0);
                }
            }
            if (s_state == RADIO) {}
            if (s_state == DLNA) {
                txt_DL_fName.setText("");
                txt_DL_fName.show(true, false);
                btn_DL_pause.setActive(false);
                btn_DL_pause.show();
            }
            if (s_state == RINGING) {
                if (startsWith(m.msg, "alarm")) s_f_eof_alarm = true;
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
                s_homepage = m.msg;
                if (!s_homepage.starts_with("http")) s_homepage = "http://" + s_homepage;
            }
            break;

        case Audio::evt_icylogo:
            if (strlen(m.msg) > 5) { SerialPrintflnCut("icy-logo:    ", ANSI_ESC_WHITE, m.msg); }
            break;

        case Audio::evt_id3data: SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s" ANSI_ESC_RESET "  ", m.msg); break;

        case Audio::evt_image:
            for (int i = 0; i < m.vec.size(); i += 2) { SerialPrintfln("CoverImage:  " ANSI_ESC_GREEN "segment %02i, pos %08i, len %08i" ANSI_ESC_RESET "  ", i / 2, m.vec[i], m.vec[i + 1]); }
            break;

        case Audio::evt_icydescription:
            s_icyDescription = m.msg;
            s_f_newIcyDescription = true;
            if (strlen(m.msg)) SerialPrintfln("icy-descr:   %s", m.msg);
            break;

        case Audio::evt_bitrate:
            if (!strlen(m.msg)) return; // guard
            s_icyBitRate = str2int(m.msg);
            s_f_newBitRate = true;
            SerialPrintfln("bitRate:     " ANSI_ESC_GREEN "%i" ANSI_ESC_RESET "  ", s_icyBitRate);
            break;

        case Audio::evt_lyrics:
            SerialPrintfln("sync lyrics: " ANSI_ESC_CYAN "%s" ANSI_ESC_RESET "  ", m.msg);
            s_lyrics = m.msg;
            s_f_newLyrics = true;
            break;

        case Audio::evt_log: SerialPrintfln("%s: .....  %s", m.s, m.msg); break;

        default: SerialPrintfln("message:...  %s", m.msg); break;
    }
}

// ————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_process_i2s(int16_t* outBuff, int32_t validSamples, bool* continueI2S) {

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
void on_BH1750(int32_t ambVal) { //--AMBIENT LIGHT SENSOR BH1750--
    int16_t bh1750Value = 0;
    bh1750Value = map_l(ambVal, 0, 1600, displayConfig.brightnessMin, displayConfig.brightnessMax);
    MWR_LOG_DEBUG("ambVal %i, bh1750Value %i, s_brightness %i", ambVal, bh1750Value, s_brightness);
    if (TFT_CONTROLLER == 8) bh1750Value = 255 - bh1750Value; // invert brightness
    setTFTbrightness(max(bh1750Value, s_brightness));
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
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "0x%02x, " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02x" ANSI_ESC_RESET "  ", addr, cmd);
    char buf[20];
    sprintf(buf, "0x%02x", addr);
    webSrv.send("IR_address=", buf);
    sprintf(buf, "0x%02x", cmd);
    webSrv.send("IR_command=", buf);
}

void ir_res(uint32_t res) {
    if (s_state != RADIO) return;
    if (s_f_sleeping == true) return;
    SerialPrintfln("ir_result:   " ANSI_ESC_YELLOW "Stationnumber " ANSI_ESC_BLUE "%lu" ANSI_ESC_RESET "  ", (long unsigned)res);
    nbr_RA_staBox.hide();
    setStationByNumber(res);
    return;
}
void ir_number(uint16_t num) {
    if (s_state != RADIO) return;
    if (s_f_sleeping) return;
    txt_RA_staName.hide();
    nbr_RA_staBox.enable();
    nbr_RA_staBox.setNumbers(num);
    nbr_RA_staBox.show(TFT_ORANGE);
}

void ir_released(int8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "released ir key nr: " ANSI_ESC_BLUE "%02i, <%s>" ANSI_ESC_RESET "  ", key, ir_symbols[key]);
    // tp_released(0, 0);
    return;
}
// ————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ir_long_key(int8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "long pressed ir key nr: " ANSI_ESC_BLUE "%02i, <%s>" ANSI_ESC_RESET "  ", key, ir_symbols[key]);
    if (key == 16) {
        if (!s_f_sleeping)
            fall_asleep(); // long OK
        else
            wake_up();
    }
}
// ————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// clang-format off
void ir_short_key(int8_t key) {
    s_f_ok_from_ir = false;
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "short pressed ir key nr: " ANSI_ESC_BLUE "%02i, <%s>" ANSI_ESC_RESET "  ", key, ir_symbols[key]);
    if (s_f_sleeping == true && key != 20) return;
    if (s_state == IR_SETTINGS) return; // nothing todo

    switch (key) {
        case 10: // MUTE  ----------------------------------------------------------------------------------------------------------------------------
            muteChanged(!s_f_mute);
            return;
        case 11: // ARROW RIGHT  ---------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                if (s_subState_radio == 0) { nextFavStation(); } // NEXT STATION
                if (s_subState_radio == 2) { set_ir_pos_RA(IR_RIGHT); setTimeCounter(2); } // scroll right
                return;
            }
            if (s_state == STATIONSLIST) { // next page
                lst_RADIO.nextPage(); setTimeCounter(LIST_TIMER);
                break;
            }
            if (s_state == PLAYER) {
                set_ir_pos_PL(IR_RIGHT); // scroll right
                return;
            }
            if (s_state == AUDIOFILESLIST) {
                lst_PLAYER.nextPage();
                setTimeCounter(LIST_TIMER);
                return; // next page
            }
            if (s_state == DLNA) {
                set_ir_pos_DL(IR_RIGHT); // scroll forward (mute, pause, cancel, prev, next)
                return;
            }
            if (s_state == DLNAITEMSLIST) {
                lst_DLNA.nextPage(); setTimeCounter(LIST_TIMER); return; // nextpage
            }
            if (s_state == CLOCK) {
                if (s_subState_clock == 1) { // scroll backward (alarm, radio, mute, off)
                    set_ir_pos_CL(IR_RIGHT); // scroll right
                    setTimeCounter(2);
                }
                return;
            }
            if (s_state == ALARMCLOCK) {
                set_ir_pos_AC(IR_RIGHT); // scroll forward (left, right, up, down, ready)
                return;
            }
            if (s_state == SLEEPTIMER) {
                set_ir_pos_SL(IR_RIGHT); // scroll forward (up, down, ready, cancel)
                return;
            }
            if (s_state == SETTINGS) {
                set_ir_pos_SE(IR_RIGHT); // scroll forward (bright, equal, wifi, radio)
                return;
            }
            if (s_state == BRIGHTNESS) {
                s_brightness += 5;
                s_brightness = clamp_min_max(s_brightness, displayConfig.brightnessMin, displayConfig.brightnessMax);
                sdr_BR_value.setValue(s_brightness);
            }
            if (s_state == EQUALIZER) { // scroll forward (radio, player, mute)
                if(s_ir_btn_select < 3) set_ir_pos_EQ(IR_RIGHT);
                else {
                    if (s_ir_btn_select == 3) { sdr_EQ_balance.setValue(sdr_EQ_balance.getValue() + 1);   setI2STone();} // balance
                    if (s_ir_btn_select == 4) { sdr_EQ_lowPass.setValue(sdr_EQ_lowPass.getValue() + 1);   setI2STone();} // lowpass
                    if (s_ir_btn_select == 5) { sdr_EQ_bandPass.setValue(sdr_EQ_bandPass.getValue() + 1); setI2STone();} // bandpass
                    if (s_ir_btn_select == 6) { sdr_EQ_highPass.setValue(sdr_EQ_highPass.getValue() + 1); setI2STone();} // highpass
                }
            }
            if (s_state == BLUETOOTH) { // scroll forward (bright, equal, radio)
                set_ir_pos_BT(IR_RIGHT);
            }
            break;
        case 12: // ARROW LEFT  ----------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                if (s_subState_radio == 0) { prevFavStation(); return; } // PREV STATION
                if (s_subState_radio == 2) { set_ir_pos_RA(IR_LEFT);  setTimeCounter(2);  return; } // scroll left
            }
            if (s_state == STATIONSLIST) {
                lst_RADIO.prevPage();
                setTimeCounter(LIST_TIMER);
                return;
            } // prev page
            if (s_state == PLAYER) {
                if(s_subState_player == 0){ // scroll backward (prev file, next file, ready, play all, shuffle, file list, radio, off)
                    set_ir_pos_PL(IR_LEFT); setTimeCounter(2);
                    return;
                }
                if(s_subState_player == 1){ // scroll backward (mute, pause, cancel, prev, next)
                    set_ir_pos_PL(IR_LEFT); setTimeCounter(2);
                    return;
                }
            }
            if (s_state == AUDIOFILESLIST) { // prev page
                lst_PLAYER.prevPage();
                setTimeCounter(LIST_TIMER);
                break;
            }
            if (s_state == DLNA) { // scroll backward (mute, pause, cancel, prev, next)
                set_ir_pos_DL(IR_LEFT);
                return;
            }
            if (s_state == DLNAITEMSLIST) { // prev page
                lst_DLNA.prevPage(); setTimeCounter(LIST_TIMER); return;
            }
            if (s_state == CLOCK) {
                if (s_subState_clock == 1) { // scroll backward (alarm, radio, mute, off)
                    set_ir_pos_CL(IR_LEFT);
                    setTimeCounter(2);
                    return;
                }
            }
            if (s_state == ALARMCLOCK) { // scroll backward (left, right, up, down, ready)
                    set_ir_pos_AC(IR_LEFT);
                    return;
            }
            if (s_state == SLEEPTIMER) { // scroll backward (up, down, ready, cancel)
                    set_ir_pos_SL(IR_LEFT);
                    return;
            }
            if (s_state == SETTINGS) { // scroll forward (bright, equal, radio)
                    set_ir_pos_SE(IR_LEFT);
                    return;
            }
            if (s_state == BRIGHTNESS) {
                s_brightness -= 5;
                s_brightness = clamp_min_max(s_brightness, displayConfig.brightnessMin, displayConfig.brightnessMax);
                sdr_BR_value.setValue(s_brightness);
                setTimeCounter(2);
                return;
            }
            if (s_state == EQUALIZER) { // scroll backward (radio, player, mute)
                if(s_ir_btn_select < 3) set_ir_pos_EQ(IR_LEFT);
                else {
                    if (s_ir_btn_select == 3) { sdr_EQ_balance.setValue(sdr_EQ_balance.getValue() - 1);   setI2STone();} // balance
                    if (s_ir_btn_select == 4) { sdr_EQ_lowPass.setValue(sdr_EQ_lowPass.getValue() - 1);   setI2STone();} // lowpass
                    if (s_ir_btn_select == 5) { sdr_EQ_bandPass.setValue(sdr_EQ_bandPass.getValue() - 1); setI2STone();} // bandpass
                    if (s_ir_btn_select == 6) { sdr_EQ_highPass.setValue(sdr_EQ_highPass.getValue() - 1); setI2STone();} // highpass
                }
            }
            if (s_state == BLUETOOTH) { // scroll forward (bright, equal, radio)
                set_ir_pos_BT(IR_LEFT);
            }
            break;
        case 13: // ARROW DOWN  ----------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
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
                volBox.show(TFT_BLUE);
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
                downvolume(); // VOLUME--
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                setTimeCounter(2);
                break;
            }
                if(s_state == DLNAITEMSLIST){lst_DLNA.nextItem(); setTimeCounter(LIST_TIMER); return;} // item++
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
            if (s_state == EQUALIZER) {
                set_ir_pos_EQ(IR_DOWN);
                break;
            }
            break;
        case 14: // ARROW UP  ------------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
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
                volBox.show(TFT_BLUE);
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
                upvolume(); // VOLUME++
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                break;
            }
            if(s_state == DLNAITEMSLIST) { lst_DLNA.prevItem(); setTimeCounter(LIST_TIMER); return; } // item++
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
            if (s_state == EQUALIZER) {
                set_ir_pos_EQ(IR_UP);
                return;
            }
            break;
        case 15: // MODE  ----------------------------------------------------------------------------------------------------------------------------
            if (s_state == SLEEPTIMER) {
                setStation(s_cur_station);
                changeState(RADIO, 0);
                break;
            }
            if (s_state == RADIO) {
                changeState(STATIONSLIST, 0);
                setTimeCounter(40);
                break;
            }
            if (s_state == STATIONSLIST) {
                changeState(PLAYER, 0);
                break;
            }
            if (s_state == PLAYER) {
                changeState(AUDIOFILESLIST, 0);
                break;
            }
            if (s_state == AUDIOFILESLIST) {
                changeState(DLNA, 0);
                break;
            }
            if (s_state == DLNA) {
                changeState(CLOCK, 0);
                break;
            }
            if (s_state == CLOCK) {
                s_sleepTimerSubMenue = 0;
                changeState(SLEEPTIMER, 0);
                break;
            }
            break;
        case 16: // OK -------------------------------------------------------------------------------------------------------------------------------
            s_f_ok_from_ir = true;
            if (s_state == RADIO) {
                if (s_subState_radio == 2) {
                    if (s_ir_btn_select == 0) { btn_RA_staList.click(); }
                    if (s_ir_btn_select == 1) { btn_RA_player.click(); }
                    if (s_ir_btn_select == 2) { btn_RA_dlna.click(); }
                    if (s_ir_btn_select == 3) { btn_RA_clock.click(); }
                    if (s_ir_btn_select == 4) { btn_RA_sleep.click(); }
                    if (s_ir_btn_select == 5) { btn_RA_settings.click(); }
                    if (s_ir_btn_select == 6) { btn_RA_bt.click(); }
                    if (s_ir_btn_select == 7) { btn_RA_off.click(); }
                } else {
                    changeState(RADIO, 2);
                    s_ir_btn_select = 0;
                    set_ir_pos_RA(0);
                }
                break;
            }
            if (s_state == STATIONSLIST) {
                setStationByNumber(lst_RADIO.getSelectedStation());
                changeState(RADIO, 0);
                break;
            }
            if (s_state == PLAYER) {
                if (s_subState_player == 0) {
                    if (s_ir_btn_select == 0) { btn_PL_prevFile.click(); }
                    if (s_ir_btn_select == 1) { btn_PL_nextFile.click(); }
                    if (s_ir_btn_select == 2) { btn_PL_ready.click(); }
                    if (s_ir_btn_select == 3) { btn_PL_playAll.click(); }
                    if (s_ir_btn_select == 4) { btn_PL_shuffle.click(); }
                    if (s_ir_btn_select == 5) { btn_PL_fileList.click(); }
                    if (s_ir_btn_select == 6) { btn_PL_radio.click(); }
                    if (s_ir_btn_select == 7) { btn_PL_off.click(); }
                }
                else if(s_subState_player == 1) {
                    if (s_ir_btn_select == 0) { btn_PL_mute.click(); }
                    if (s_ir_btn_select == 1) { btn_PL_pause.click(); }
                    if (s_ir_btn_select == 2) { btn_PL_cancel.click(); }
                    if (s_ir_btn_select == 3) { btn_PL_playPrev.click(); }
                    if (s_ir_btn_select == 4) { btn_PL_playNext.click(); }
                }
                if(s_ir_btn_select == -1){ s_ir_btn_select = 0; set_ir_pos_PL(0); }
                break;
            }
            if (s_state == AUDIOFILESLIST) {
                const char* r = lst_PLAYER.getSelectedFile();
                if (r) {
                    stopSong();
                    SD_playFile(lst_PLAYER.getSelectedFilePath(), 0, true);
                    s_cur_AudioFileNr = lst_PLAYER.getSelectedFileNr();
                }
                break;
            }
            if (s_state == DLNA) {
                if (s_ir_btn_select == 0) { btn_DL_mute.click(); }
                if (s_ir_btn_select == 1) { btn_DL_pause.click(); }
                if (s_ir_btn_select == 2) { btn_DL_cancel.click(); }
                if (s_ir_btn_select == 3) { btn_DL_fileList.click(); }
                if (s_ir_btn_select == 4) { btn_DL_radio.click(); }
                if (s_ir_btn_select == -1){ s_ir_btn_select = 0; set_ir_pos_DL(0); }
                break;
            }
            if(s_state == DLNAITEMSLIST) {
                const char* r = lst_DLNA.getSelectedURL();
                if(r) { txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(lst_DLNA.getSelectedTitle()); changeState(DLNA, 0); connecttohost(r); }
                else setTimeCounter(2);
                break;
            }
            if (s_state == CLOCK) {
                if (s_subState_clock == 0) {
                    changeState(CLOCK, 1);
                    setTimeCounter(2);
                    if(s_ir_btn_select == -1){ s_ir_btn_select = 0; set_ir_pos_CL(0); }
                    break;
                }
                if (s_subState_clock == 1) {
                    if (s_ir_btn_select == 0) { btn_CL_alarm.click(); }
                    if (s_ir_btn_select == 1) { btn_CL_radio.click(); }
                    if (s_ir_btn_select == 2) { btn_CL_mute.click(); }
                    if (s_ir_btn_select == 3) { btn_CL_off.click(); }
                }
                break;
            }
            if (s_state == ALARMCLOCK) {
                    if (s_ir_btn_select == 0) { btn_AC_left.click(); }
                    if (s_ir_btn_select == 1) { btn_AC_right.click(); }
                    if (s_ir_btn_select == 2) { btn_AC_up.click(); }
                    if (s_ir_btn_select == 3) { btn_AC_down.click(); }
                    if (s_ir_btn_select == 4) { btn_AC_ready.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_AC(0); }
                    break;
            }
            if (s_state == SLEEPTIMER) {
                    if (s_ir_btn_select == 0) { btn_SL_up.click(); }
                    if (s_ir_btn_select == 1) { btn_SL_down.click(); }
                    if (s_ir_btn_select == 2) { btn_SL_ready.click(); }
                    if (s_ir_btn_select == 3) { btn_SL_cancel.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_SL(0); }
                    break;
            }
            if (s_state == SETTINGS) {
                    if (s_ir_btn_select == 0) { btn_SE_bright.click(); }
                    if (s_ir_btn_select == 1) { btn_SE_equal.click(); }
                    if (s_ir_btn_select == 2) { btn_SE_wifi.click(); }
                    if (s_ir_btn_select == 3) { btn_SE_radio.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_SE(0); }
                    break;
            }
            if (s_state == BRIGHTNESS) {
                    if (s_ir_btn_select == 0) { btn_BR_ready.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_BR(0); }
                    break;
            }
            if (s_state == EQUALIZER) {
                    if (s_ir_btn_select == 0) { btn_EQ_Radio.click(); }
                    if (s_ir_btn_select == 1) { btn_EQ_Player.click(); }
                    if (s_ir_btn_select == 2) { btn_EQ_mute.click(); }
                    if (s_ir_btn_select == 3) { btn_EQ_balance.click(); }
                    if (s_ir_btn_select == 4) { btn_EQ_lowPass.click(); }
                    if (s_ir_btn_select == 5) { btn_EQ_bandPass.click(); }
                    if (s_ir_btn_select == 6) { btn_EQ_highPass.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_EQ(0); }
                    break;
            }
            if (s_state == BLUETOOTH) {
                    if (s_ir_btn_select == 0) { btn_BT_volDown.click(); }
                    if (s_ir_btn_select == 1) { btn_BT_volUp.click(); }
                    if (s_ir_btn_select == 2) { btn_BT_pause.click(); }
                    if (s_ir_btn_select == 3) { btn_BT_mode.click(); }
                    if (s_ir_btn_select == 4) { btn_BT_radio.click(); }
                    if (s_ir_btn_select == 5) { btn_BT_power.click(); }
                    if(s_ir_btn_select == -1) { s_ir_btn_select = 0; set_ir_pos_BT(0); }
                    break;
            }
            break;
        case 18: // PAUSE/RESUME  --------------------------------------------------------------------------------------------------------------------
            if (s_state == PLAYER) {
                if (s_f_isFSConnected) s_f_pauseResume = audio.pauseResume();
            }
            break;
        case 19: // STOP  ----------------------------------------------------------------------------------------------------------------------------
            if (s_state == PLAYER) {
                if (s_f_isFSConnected) audio.stopSong();
                changeState(PLAYER, 0);
            }
            break;
        case 20: // ON/OFF  --------------------------------------------------------------------------------------------------------------------------
            if (!s_f_sleeping){
                fall_asleep();
            } else {
                wake_up();
            }
            break;
        case 21: // RADIO  ---------------------------------------------------------------------------------------------------------------------------
            if (s_state != RADIO) {
                setStation(s_cur_station);
                changeState(RADIO, 0);
            }
            break;
        case 22: // PLAYER  --------------------------------------------------------------------------------------------------------------------------
            if (s_state != PLAYER) { changeState(PLAYER, 0); }
            break;
        case 23: // DLNA  ----------------------------------------------------------------------------------------------------------------------------
            if (s_state != DLNA) {
                changeState(DLNA, 0);
            }
            break;
        case 24: // CLOCK  ---------------------------------------------------------------------------------------------------------------------------
            if (s_state != CLOCK) {
                changeState(CLOCK, 1);
            }
            break;
        case 25: // OFF TIIMER  ----------------------------------------------------------------------------------------------------------------------
            if (s_state != SLEEPTIMER) {
                s_sleepTimerSubMenue = 0;
                changeState(SLEEPTIMER, 0);
            }
            break;
        case 26: // VOLUME+  -------------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                setTimeCounter(2);
                break;
            }
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                setTimeCounter(2);
                break;
            } // VOLUME++
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                upvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
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
        case 27: // VOLUME-  -------------------------------------------------------------------------------------------------------------------------
            if (s_state == RADIO) {
                txt_RA_staName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == PLAYER) {
                txt_PL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
                setTimeCounter(2);
                break;
            } // VOLUME--
            if (s_state == DLNA) {
                txt_DL_fName.hide();
                volBox.enable();
                downvolume();
                volBox.setNumbers(s_volume.cur_volume);
                volBox.show(TFT_BLUE);
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
        case 28: // -30sec  --------------------------------------------------------------------------------------------------------------------------
            if (s_state == PLAYER) {
                if (audio.isRunning()) audio.setTimeOffset(-30);
            }
            break;
        case 29: // +30sec  --------------------------------------------------------------------------------------------------------------------------
            if (s_state == PLAYER) {
                if (audio.isRunning()) audio.setTimeOffset(+30);
            }
            break;
        case 30: // NEXT STATION  --------------------------------------------------------------------------------------------------------------------
            nextStation();
            break;
        case 31: // PREV STATION  --------------------------------------------------------------------------------------------------------------------
            prevStation();
            break;
        default: //  ---------------------------------------------------------------------------------------------------------------------------------
            MWR_LOG_WARN("unknown IR code: %i", key);
            break;
    }
}
// clang-format on
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Events from websrv /*🟢🟡🔴*/
// clang-format off

void WEBSRV_onCommand(ps_ptr<char> cmd, ps_ptr<char> param, ps_ptr<char> arg){  // called via html

    if(CORE_DEBUG_LEVEL == ARDUHAL_LOG_LEVEL_DEBUG){
        SerialPrintfln("WS_onCmd:    " ANSI_ESC_YELLOW "cmd=\"%s\", params=\"%s\", arg=\"%s\"" ANSI_ESC_RESET "  ", cmd.c_get(), param.c_get(), arg.c_get());
    }
    #define CMD_EQUALS(x) if(cmd.equals(x) == true)

    CMD_EQUALS("ping"){                 webSrv.send("pong"); return;}                                                                                     // via websocket

    CMD_EQUALS("index.html"){           SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "index.html" ANSI_ESC_RESET "  ");                                      // via XMLHttpRequest
                                        webSrv.show(index_html, webSrv.TEXT);
                                        return;}

    CMD_EQUALS("index.js"){             SerialPrintfln("Script:      " ANSI_ESC_ORANGE "index.js" ANSI_ESC_RESET "  ");                                        // via XMLHttpRequest
                                        webSrv.show(index_js, webSrv.JS); return;}

    CMD_EQUALS("favicon.ico"){          webSrv.streamfile(SD_MMC, "/favicon.ico"); return;}                                                               // via XMLHttpRequest

    CMD_EQUALS("test"){                 ps_ptr<char>p; p.assignf("free heap: %lu, Inbuff filled: %lu, Inbuff free: %lu, PSRAM filled %lu, PSRAM free %lu,",
                                        (long unsigned)ESP.getFreeHeap(), (long unsigned)audio.inBufferFilled(), (long unsigned)audio.inBufferFree(),
                                        (long unsigned) (ESP.getPsramSize() - ESP.getFreePsram()), (long unsigned)ESP.getFreePsram());
                                        webSrv.send("test=", p.c_get());
                                        SerialPrintfln("audiotask .. stackHighWaterMark: %lu bytes", (long unsigned)audio.getHighWatermark() * 4);
                                        SerialPrintfln("looptask ... stackHighWaterMark: %lu bytes", (long unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
                                        return;}

    CMD_EQUALS("get_mute"){             s_f_mute == true ? webSrv.send("mute=", "1") : webSrv.send("mute=", "0"); return;}
    CMD_EQUALS("set_mute"){             muteChanged(!s_f_mute); return;}
    CMD_EQUALS("upvolume"){             webSrv.send("volume=", int2str(upvolume()));  return;}                                                            // via websocket
    CMD_EQUALS("downvolume"){           webSrv.send("volume=", int2str(downvolume())); return;}                                                           // via websocket
    CMD_EQUALS("get_volumeSteps"){      webSrv.send("volumeSteps=", int2str(s_volume.volumeSteps)); return;}

    CMD_EQUALS("set_volumeSteps"){      s_volume.cur_volume = map_l(s_volume.cur_volume, 0, s_volume.volumeSteps, 0, param.to_uint32());
                                        s_volume.ringVolume = map_l(s_volume.ringVolume, 0, s_volume.volumeSteps, 0, param.to_uint32()); webSrv.send("ringVolume=", int2str(s_volume.ringVolume));
                                        s_volume.volumeAfterAlarm = map_l(s_volume.volumeAfterAlarm, 0, s_volume.volumeSteps, 0, param.to_uint32()); webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm));
                                        s_volume.volumeSteps = param.to_uint32(); webSrv.send("volumeSteps=", param.c_get()); audio.setVolumeSteps(s_volume.volumeSteps);
                                        MWR_LOG_DEBUG("s_volumeSteps  %i", s_volume.volumeSteps);
                                        sdr_CL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_DL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_PL_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        sdr_RA_volume.setMinMaxVal(0, s_volume.volumeSteps);
                                        setVolume(s_volume.cur_volume);
                                        SerialPrintfln("action: ...  new volume steps: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "  ", s_volume.volumeSteps);
                                        return;}

    CMD_EQUALS("get_ringVolume"){       webSrv.send("ringVolume=", int2str(s_volume.ringVolume)); return;}
    CMD_EQUALS("set_ringVolume"){       s_volume.ringVolume = param.to_int32(); webSrv.send("ringVolume=", int2str(s_volume.ringVolume));
                                        SerialPrintfln("action: ...  new ring volume: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "  ", s_volume.ringVolume); return;}

    CMD_EQUALS("get_volAfterAlarm"){    webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm)); return;}
    CMD_EQUALS("set_volAfterAlarm"){    s_volume.volumeAfterAlarm = param.to_int32(); webSrv.send("volAfterAlarm=", int2str(s_volume.volumeAfterAlarm));
                                        SerialPrintfln("action: ...  new volume after alarm: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET "  ", s_volume.volumeAfterAlarm); return;}
    CMD_EQUALS("homepage"){             webSrv.send("homepage=", s_homepage.c_get()); return;}

    CMD_EQUALS("to_listen"){            showLogoAndStationName(false); return;}   // via websocket, return the name and number of the current station
    CMD_EQUALS("get_tone"){             webSrv.send("settone=", getI2STone().c_get()); return;}

    CMD_EQUALS("get_streamtitle"){      webSrv.reply(s_streamTitle.c_get(), webSrv.TEXT); return;}

    CMD_EQUALS("LowPass"){              s_tone.LP = param.to_int32(); sdr_EQ_lowPass.setValue(s_tone.LP); // audioI2S tone
                                        ps_ptr<char>lp; lp = "Lowpass set to " + param  + "dB";
                                        setI2STone(); return;}

    CMD_EQUALS("BandPass"){             s_tone.BP = param.to_int32(); sdr_EQ_bandPass.setValue(s_tone.BP); // audioI2S tone
                                        ps_ptr<char>bp; bp = "Bandpass set to " + param + "dB";
                                        setI2STone(); return;}

    CMD_EQUALS("HighPass"){             s_tone.HP = param.to_int32(); sdr_EQ_highPass.setValue(s_tone.HP); // audioI2S tone
                                        ps_ptr<char> hp; hp = "Highpass set to " + param + "dB";
                                        setI2STone(); return;}

    CMD_EQUALS("Balance"){              s_tone.BAL = param.to_int32(); sdr_EQ_balance.setValue(s_tone.BAL); // audioI2S tone
                                        ps_ptr<char> bal = "Balance set to " + param;
                                        setI2STone(); return;}

    CMD_EQUALS("prev_station"){         prevFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("next_station"){         nextFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("set_station"){          setStationByNumber(param.to_uint32()); return;}                                                                      // via websocket

    CMD_EQUALS("stationURL"){           setStationViaURL(param.c_get(), arg.c_get());                                                                        // via websocket
                                        s_stationName_air = param;
                                        SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s" ANSI_ESC_RESET "  ", param.c_get());
                                        s_f_newStationName = true; return;}

    CMD_EQUALS("webFileURL"){           audio.connecttohost(param.c_get())? changeState(PLAYER, 1) : changeState(PLAYER, 0); return;}                        // via websocket

    CMD_EQUALS("get_networks"){         webSrv.send("networks=", WiFi.SSID().c_str()); return;}                                                              // via websocket

    CMD_EQUALS("get_tftSize"){          webSrv.send("tftSize=",displayConfig.tftSize); return;};

    CMD_EQUALS("get_timeZones"){        webSrv.send("timezones=", timezones_json); return;}

    CMD_EQUALS("set_timeZone"){         s_TZName = param;  s_TZString = arg.c_get();
                                        SerialPrintfln("Timezone: .. " ANSI_ESC_BLUE "%s, %s" ANSI_ESC_RESET "  ", param.c_get(), arg.c_get());
                                        setRTC(s_TZString);
                                        updateSettings(); // write new TZ items to settings.json
                                        return;}

    CMD_EQUALS("get_timeZoneName"){     webSrv.reply(s_TZName, webSrv.TEXT); return;}

    CMD_EQUALS("change_state"){         if     (!strcmp(param.c_get(), "RADIO")       && s_state != RADIO)       { changeState(RADIO, 0); return; }
                                        else if(!strcmp(param.c_get(), "PLAYER")      && s_state != PLAYER)      { stopSong(); changeState(PLAYER, 0); return; }
                                        else if(!strcmp(param.c_get(), "DLNA")        && s_state != DLNA)        { stopSong(); changeState(DLNA, 0);   return; }
                                        else if(!strcmp(param.c_get(), "BLUETOOTH")   && s_state != BLUETOOTH)   { changeState(BLUETOOTH, 0); return; }
                                        else if(!strcmp(param.c_get(), "IR_SETTINGS") && s_state != IR_SETTINGS) { changeState(IR_SETTINGS, 0); return; }
                                        else return; }

    CMD_EQUALS("stopfile"){             if(!s_f_isFSConnected && !s_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        stopSong(); changeState(PLAYER, 0); webSrv.send("stopfile=", "audiofile stopped");
                                        return;}

    CMD_EQUALS("pause_resume"){         if(!s_f_isFSConnected && !s_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        s_f_pauseResume = audio.pauseResume();
                                        if(audio.isRunning()){webSrv.send("resumefile=", "audiofile resumed"); btn_PL_pause.setOff(); btn_PL_pause.show();}
                                        else {                webSrv.send("resumefile=", "audiofile paused");  btn_PL_pause.setOn(); btn_PL_pause.show();}
                                        return;}

    CMD_EQUALS("get_alarmdays"){        webSrv.send("alarmdays=", s_alarmdays); return;}

    CMD_EQUALS("set_alarmdays"){        s_alarmdays = param.to_uint32(); updateSettings(); return;}

    CMD_EQUALS("get_alarmtime"){        return;} // not used yet

    CMD_EQUALS("set_alarmtime"){        return;}

    CMD_EQUALS("get_timeAnnouncement"){ if(s_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "1");
                                        if(  !s_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "0");
                                        return;}

    CMD_EQUALS("set_timeAnnouncement"){ if(param == "true" ) {s_f_timeAnnouncement = true;}
                                        if(   param == "false") {s_f_timeAnnouncement = false;}
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "hourly time announcement " ANSI_ESC_BLUE "%s" ANSI_ESC_RESET "  ", (s_f_timeAnnouncement == 1) ? "on" : "off");
                                        return;}

    CMD_EQUALS("get_timeSpeechLang"){   webSrv.send("get_timeSpeechLang=", s_timeSpeechLang); SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "language is " ANSI_ESC_BLUE "%s" ANSI_ESC_RESET "  ", s_timeSpeechLang.c_get()); return;}

    CMD_EQUALS("set_timeSpeechLang"){   if(param.strlen() > 2){MWR_LOG_ERROR("set_timeSpeechLang too long %s", param.c_get()); return;}
                                        s_timeSpeechLang = param;
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "language is " ANSI_ESC_BLUE "%s" ANSI_ESC_RESET "  ", param.c_get());
                                        return;}

    CMD_EQUALS("DLNA_getServer")  {     webSrv.send("DLNA_Names=", dlna.stringifyServer()); s_currDLNAsrvNr = -1; return;}

    CMD_EQUALS("DLNA_getRoot")    {     s_currDLNAsrvNr = param.to_uint32(); dlna.browseServer(s_currDLNAsrvNr, "0"); return;}

    CMD_EQUALS("DLNA_getContent") {     if(param.starts_with("http")) {connecttohost(param.c_get()); showFileName(arg.c_get()); return;}
                                        s_dlnaHistory[s_dlnaLevel].objId = param;
                                        s_totalNumberReturned = 0;
                                        dlna.browseServer(s_currDLNAsrvNr, s_dlnaHistory[s_dlnaLevel].objId.c_get());
                                        return;}

    CMD_EQUALS("SD/"){                  param = scaleImage(param); if(!SD_MMC.exists(param.c_get())) param = scaleImage("/common/unknown.png");
                                        if(!webSrv.streamfile(SD_MMC, param)){ SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "The file could not be transferred " ANSI_ESC_RED "\"%s\"" ANSI_ESC_RESET "  ", param.get()); } // via XMLHttpRequest
                                        return;}

    CMD_EQUALS("SD_Download"){          webSrv.streamfile(SD_MMC, param.c_get());                                                                         // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Load from SD  " ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param.c_get());
                                        return;}

    CMD_EQUALS("SD_GetFolder"){         webSrv.reply(s_SD_content.stringifyDirContent(param), webSrv.JS);                                                           // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "GetFolder " ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param.c_get());
                                        return;}

    CMD_EQUALS("SD_newFolder"){         bool res = SD_newFolder(param.c_get());                                                                           // via XMLHttpRequest
                                        if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "NewFolder " ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param.c_get());
                                        return;}

    CMD_EQUALS("SD_playFile"){          stopSong();
                                        webSrv.reply("SD_playFile=" + param, webSrv.TEXT);                                                                // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play " ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param.c_get());
                                        SD_playFile(param.c_get());
                                        return;}

    CMD_EQUALS("SD_playAllFiles"){      stopSong();
                                        webSrv.send("SD_playFolder=", param);                                                                                      // via websocket
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play Folder" ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param.c_get());
                                        if(playlist.create_playlist_from_SD_folder(param)){
                                            s_f_playlistEnabled = true;
                                            s_subState_player = 1;
                                        }
                                        return;}

    CMD_EQUALS("SD_rename"){            ps_ptr<char> _arg = arg.substr(0, arg.index_of("&")); // only the first argument is used                              // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Rename " ANSI_ESC_ORANGE "old \"%s\" new \"%s\"" ANSI_ESC_RESET "  ",
                                        param.c_get(), _arg.c_get());
                                        bool res = SD_rename(param.c_get(), _arg.c_get());
                                        if(res) webSrv.reply("refresh", webSrv.TEXT);
                                        else webSrv.sendStatus(400);
                                        return;}

    CMD_EQUALS("set_IRcmd"){            int32_t command = param.to_int32(16);
                                        int32_t btnNr = arg.to_int32(10);
                                        SerialPrintfln("set_IR_cmd:  " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02lx, "
                                        ANSI_ESC_YELLOW "IR Button Number " ANSI_ESC_BLUE "%02li" ANSI_ESC_RESET "  ", (long signed)command, (long signed)btnNr);
                                        ir.set_irButtons(btnNr,  command);
                                        s_settings.irbuttons[btnNr].val = command;
                                        return;}

    CMD_EQUALS("set_IRaddr"){           SerialPrintfln("set_IR_addr: " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "%s" ANSI_ESC_RESET "  ", param.c_get());
                                        int32_t address = (int32_t)strtol(param.c_get(), NULL, 16);
                                        ir.set_irAddress(address);
                                        s_settings.irbuttons[42].val = address;
                                        return;}

    CMD_EQUALS("get_sleepMode"){        webSrv.send("sleepMode=", s_sleepMode); return;}

    CMD_EQUALS("set_sleepMode"){        s_sleepMode = param.to_uint32();
                                        if(s_sleepMode == 0) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Display off" ANSI_ESC_RESET "  ");
                                        if(s_sleepMode == 1) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Show the time" ANSI_ESC_RESET "  ");
                                        return;}

    CMD_EQUALS("DLNA_GetFolder"){       webSrv.sendStatus(306); return;}  // todo
    CMD_EQUALS("KCX_BT_connected") {    if(!bt_emitter.get_power_state()) webSrv.send("KCX_BT_connected=", "-1");
                                        else if(bt_emitter.isConnected()) webSrv.send("KCX_BT_connected=",  "1");
                                        else                              webSrv.send("KCX_BT_connected=",  "0");
                                        return;}
    CMD_EQUALS("KCX_BT_clearItems"){    bt_emitter.deleteVMlinks(); return;}
    CMD_EQUALS("KCX_BT_addName"){       bt_emitter.addLinkName(param.c_get()); return;}
    CMD_EQUALS("KCX_BT_addAddr"){       bt_emitter.addLinkAddr(param.c_get()); return;}
    CMD_EQUALS("KCX_BT_mem"){           bt_emitter.getVMlinks(); return;}
    CMD_EQUALS("KCX_BT_scanned"){       webSrv.send("KCX_BT_SCANNED=", bt_emitter.stringifyScannedItems()); return;}
    CMD_EQUALS("KCX_BT_getMode"){       webSrv.send("KCX_BT_MODE=", bt_emitter.getMode().c_get()); return;}
    CMD_EQUALS("KCX_BT_changeMode"){    bt_emitter.changeMode(); return;}
    CMD_EQUALS("KCX_BT_pause"){         bt_emitter.pauseResume(); return;}
    CMD_EQUALS("KCX_BT_downvolume"){    bt_emitter.downvolume(); return;}
    CMD_EQUALS("KCX_BT_upvolume"){      bt_emitter.upvolume();   return;}
    CMD_EQUALS("KCX_BT_getPower"){      bt_emitter.get_power_state() ? webSrv.send("KCX_BT_power=", "1") : webSrv.send("KCX_BT_power=", "0"); return;}
    CMD_EQUALS("KCX_BT_power"){         s_bt_emitter.enabled = !s_bt_emitter.enabled ; return;}

    CMD_EQUALS("hardcopy"){             SerialPrintfln("Webpage: ... " ANSI_ESC_YELLOW "create a display hardcopy" ANSI_ESC_RESET "  "); make_hardcopy_on_sd(); webSrv.send("hardcopy=", "/hardcopy.bmp"); return;}

    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param.c_get());
    webSrv.sendStatus(400);
}
// clang-format on
/*🟢🟡🔴*/
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// POST Events from websrv /*🟢🟡🔴*/
// clang-format off

void WEBSRV_onRequest(const char* cmd,  const char* param, const char* arg, const char* contentType, uint32_t contentLength){
    MWR_LOG_DEBUG("cmd %s, param %s, arg %s, ct %s, cl %i", cmd, param, arg, contentType, contentLength);
    if(strcmp(cmd, "SD_Upload") == 0) {savefile(param, contentLength, contentType); // PC --> SD
                                       if(strcmp(param, "/stations.json") == 0) staMgnt.updateStationsList();
                                       return;}

    if(strcmp(cmd, "upload_player2sd") == 0) {savefile(param, contentLength, contentType); return; }
    if(strcmp(cmd, "upload_text_file") == 0) {savefile(param, contentLength, contentType); return; }
    if(strcmp(cmd, "uploadfile") == 0){saveImage(param, contentLength); return;}
    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param);
    webSrv.sendStatus(400);
}

void WEBSRV_onDelete(const char* cmd,  const char* param, const char* arg){  // via XMLHttpRequest
    if(startsWith(cmd, "SD")){      bool res = SD_delete(param);
                                    if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Delete " ANSI_ESC_ORANGE "\"%s\"" ANSI_ESC_RESET "  ", param);
                                    return;}
    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd, param);
    webSrv.sendStatus(400);
}
// clang-format on
/*🟢🟡🔴*/
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
                        SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW " %s, itemSize %ld" ANSI_ESC_RESET "  ", item.title.c_get(), (long unsigned int)item.itemSize);
                    } else {
                        SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", item.title.c_get());
                    }
                } else {
                    SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, duration %s" ANSI_ESC_RESET "  ", item.title.c_get(), item.duration.c_get());
                }
            }
            if (item.childCount) {
                SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, childCount %i" ANSI_ESC_RESET "  ", item.title.c_get(), item.childCount);
            } else {
                SerialPrintfln("DLNA_server: " ANSI_ESC_CYAN "title " ANSI_ESC_YELLOW "%s, childCount 0" ANSI_ESC_RESET "  ", item.title.c_get());
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
            lst_DLNA.show(s_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &s_dlnaLevel, s_dlnaMaxItems, s_dlnaMaXServers);
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
            SerialPrintfln("DLNA_server: [%d] " ANSI_ESC_CYAN "%s:%d " ANSI_ESC_YELLOW " %s" ANSI_ESC_RESET "  ", i, server.ip.c_get(), server.port, server.friendlyName.c_get());
        }
        s_dlnaMaXServers = msg.server->size();
        SerialPrintfln("DLNA_server: %i media server found", msg.server->size());
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void on_kcx_bt_emitter(const KCX_BT_Emitter::msg_s& msg) {
    if (msg.e == KCX_BT_Emitter::evt_info) { SerialPrintfln("BT-Emitter:  " ANSI_ESC_GREEN "%s" ANSI_ESC_RESET "  ", msg.arg); }
    if (msg.e == KCX_BT_Emitter::evt_found) {
        s_bt_emitter.found = true;
        SerialPrintfln("BT-Emitter:  %s" ANSI_ESC_RESET "  ", "found");
        bt_emitter.userCommand("AT+GMR?");                 // get version
        bt_emitter.userCommand("AT+PAUSE?");               // pause or play?
        bt_emitter.userCommand("AT+NAME+BT-MiniWebRadio"); // set BT receiver name
        bt_emitter.setVolume(s_bt_emitter.volume);
        if (!bt_emitter.getMode().equals(s_bt_emitter.mode)) { bt_emitter.setMode(s_bt_emitter.mode); }
    }
    if (msg.e == KCX_BT_Emitter::evt_connect) {
        s_bt_emitter.connect = true;
        if (s_bt_emitter.mode.equals("TX")) {
            if (s_state == BLUETOOTH) {
                pic_BT_mode.setPicturePath("/common/BT_TX.png");
                pic_BT_mode.show(true, false);
            }
            webSrv.send("KCX_BT_MODE=", "TX");
        } else {
            if (s_state == BLUETOOTH) {
                pic_BT_mode.setPicturePath("/common/BT_RX.png");
                pic_BT_mode.show(true, false);
            }
            webSrv.send("KCX_BT_MODE=", "RX");
        }
        webSrv.send("KCX_BT_connected=", "1");
        SerialPrintfln("BT-Emitter:  %s ", "connected");
    }
    if (msg.e == KCX_BT_Emitter::evt_disconnect) {
        SerialPrintfln("BT-Emitter:  %s ", "disconnected");
        pic_BT_mode.setPicturePath("/common/BTnc.png"); // not connected
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        webSrv.send("KCX_BT_connected=", "0");
    }
    if (msg.e == KCX_BT_Emitter::evt_reset) {
        SerialPrintfln("BT-Emitter:  %s ", "reset");
        s_bt_emitter.connect = false;
    }
    if (msg.e == KCX_BT_Emitter::evt_power_on) {
        webSrv.send("KCX_BT_power=", "1");
        webSrv.send("KCX_BT_connected=", "0");
        btn_BT_power.setValue(true);
        pic_BT_mode.setPicturePath("/common/BTnc.png");
        if (s_state == BLUETOOTH) pic_BT_mode.show(true, false);
        SerialPrintfln("BT-Emitter:  %s ", "power on");
        bt_emitter.userCommand("AT+BT_MODE?");
    }
    if (msg.e == KCX_BT_Emitter::evt_power_off) {
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
        ps_ptr<char> v;
        v.assignf("%i", s_bt_emitter.volume);
        txt_BT_volume.writeText(v);
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%i" ANSI_ESC_RESET "  ", "volume", msg.val);
    }
    if (msg.e == KCX_BT_Emitter::evt_version) {
        s_bt_emitter.version = msg.arg;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", "version", msg.arg);
    }
    if (msg.e == KCX_BT_Emitter::evt_mode) {
        webSrv.send("KCX_BT_MODE=", msg.arg);
        if (s_state == BLUETOOTH) txt_BT_mode.writeText(msg.arg[0] == 'R' ? "RECEIVER" : "EMITTER");
        s_bt_emitter.mode = msg.arg;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s" ANSI_ESC_RESET "  ", "RX_TX_mode", msg.arg);
    }
    if (msg.e == KCX_BT_Emitter::evt_pause) {
        s_bt_emitter.play = false;
        SerialPrintfln("BT-Emitter:  %s ", "Pause");
    }
    if (msg.e == KCX_BT_Emitter::evt_play) {
        s_bt_emitter.play = true;
        SerialPrintfln("BT-Emitter:  %s ", "Play");
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void on_websrv(const WebSrv::msg_s& msg) {
    if (msg.e == WebSrv::evt_info) {
        if (msg.arg.starts_with("WebSocket")) return;      // suppress WebSocket client available
        if (msg.arg.starts_with("ping")) return;           // suppress ping
        if (msg.arg.starts_with("to_listen")) return;      // suppress to_isten
        if (msg.arg.starts_with("Command client")) return; // suppress Command client available
        if (msg.arg.starts_with("test=")) return;          // suppress stackHighWaterMark
        if (msg.arg.starts_with("get_")) return;           // suppress all getters
        if (msg.arg.starts_with("set_")) return;           // suppress all setters
        if (msg.arg.starts_with("SD")) return;             // suppress all SD commands
        if (msg.arg.starts_with("Length")) return;         // suppress all file length infos
        SerialPrintfln("WebSrv Info: " ANSI_ESC_GREEN "%s " ANSI_ESC_RESET "  ", msg.arg.c_get());
    }
    if (msg.e == WebSrv::evt_error) { SerialPrintfln("WebSrv Err:  " ANSI_ESC_RED "%s" ANSI_ESC_RESET "  ", msg.arg.c_get()); }
    if (msg.e == WebSrv::evt_command) { WEBSRV_onCommand(msg.cmd, msg.param1, msg.arg1); }
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
//  📌📌📌         T O U C H            📌📌📌
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// clang-format off
/*🟢🟡🔴*/
void tp_pressed(uint16_t x, uint16_t y) {
    //  SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint  x=%d, y=%d", x, y);
    if (s_f_sleeping) return; // awake in tp_released()
    const char* objName = NULL;
    if(y > layout.winHeader.y + layout.winHeader.h && y < layout.winProgbar.y) {
        objName = "backpane";
        if (s_state == RADIO){
            changeState(RADIO, s_subState_radio + 1 == 3 ? 0 : s_subState_radio + 1);
            goto exit;
        }
        if (s_state == CLOCK){
            changeState(CLOCK, s_subState_clock + 1 == 2 ? 0 : s_subState_clock + 1);
            goto exit;
        }
    }
    objName = isObjectClicked(x, y);
exit:
    if (objName) { SerialPrintfln("click on ..  %s", objName); }
    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_long_pressed(uint16_t x, uint16_t y) {

    if (s_state == DLNAITEMSLIST) {
        //    lst_DLNA.longPressed(x, y);
    }
    MWR_LOG_INFO("tp long pressed");
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_moved(uint16_t x, uint16_t y) {
    if (s_state == RADIO)          { if (sdr_RA_volume.positionXY(x, y))     return; }
    if (s_state == STATIONSLIST)   { if (lst_RADIO.positionXY(x, y))         return; }
    if (s_state == PLAYER)         { if (sdr_PL_volume.positionXY(x, y))     return; }
    if (s_state == AUDIOFILESLIST) { if (lst_PLAYER.positionXY(x, y))        return; }
    if (s_state == DLNA)           { if (sdr_DL_volume.positionXY(x, y))     return; }
    if (s_state == CLOCK)          { if (sdr_CL_volume.positionXY(x, y))     return; }
    if (s_state == DLNAITEMSLIST)  { if (lst_DLNA.positionXY(x, y))          return; }
    if (s_state == BRIGHTNESS)     { if (sdr_BR_value.positionXY(x, y))      return; }
    if (s_state == EQUALIZER)      { if (sdr_EQ_lowPass.positionXY(x, y))  { return; }
                                     if (sdr_EQ_bandPass.positionXY(x, y)) { return; }
                                     if (sdr_EQ_highPass.positionXY(x, y)) { return; }
                                     if (sdr_EQ_balance.positionXY(x, y))  { return; }}
    if (s_state == IR_SETTINGS)    { if (btn_IR_radio.positionXY(x, y))      return; }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_released(uint16_t x, uint16_t y){

    if(s_f_sleeping){ wake_up(); return;}   // if sleeping

    // all state
    dispHeader.released();
    dispFooter.released();

    switch(s_state){
        case RADIO:
            VUmeter_RA.released();    sdr_RA_volume.released(); btn_RA_mute.released();  btn_RA_prevSta.released(); btn_RA_nextSta.released();
            btn_RA_player.released(); btn_RA_dlna.released();   btn_RA_clock.released(); btn_RA_sleep.released();   btn_RA_settings.released();
            btn_RA_bt.released();     btn_RA_off.released();    btn_RA_staList.released();
            break;
        case STATIONSLIST:
            lst_RADIO.released();
            break;
        case PLAYER:
            btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released();
            btn_PL_fileList.released(); btn_PL_radio.released();    btn_PL_off.released();
            btn_PL_mute.released();     btn_PL_pause.released();    btn_PL_cancel.released(); sdr_PL_volume.released(); btn_PL_playNext.released();
            btn_PL_playPrev.released(); pgb_PL_progress.released();
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
            btn_CL_mute.released(); btn_CL_alarm.released(); btn_CL_radio.released(); clk_CL_24.released(); sdr_CL_volume.released(); btn_CL_off.released();
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
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_long_released(uint16_t x, uint16_t y){
    MWR_LOG_INFO("tp long released");
//    if(s_state == DLNAITEMSLIST) {lst_DLNA.longReleased();}
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnChange(ps_ptr<char> name, int32_t val) {
    ps_ptr<char> c;
    if (name.equals("sdr_RA_volume"))   { setTimeCounter(2); setVolume(val); goto exit; }
    if (name.equals("sdr_PL_volume"))   { setVolume(val); goto exit; }
    if (name.equals("sdr_DL_volume"))   { setVolume(val); goto exit; }
    if (name.equals("sdr_CL_volume"))   { setVolume(val); goto exit; }
    if (name.equals("sdr_BR_value"))    { s_brightness = val; setTFTbrightness(val); txt_BR_value.writeText(int2str(val)); goto exit; }
    if (name.equals("sdr_EQ_LP"))       { c.assignf("%i dB", val); txt_EQ_lowPass.writeText(c.c_get());  s_tone.LP  = val; webSrv.send("settone=", getI2STone().c_get()); setI2STone(); goto exit; }
    if (name.equals("sdr_EQ_BP"))       { c.assignf("%i dB", val); txt_EQ_bandPass.writeText(c.c_get()); s_tone.BP  = val; webSrv.send("settone=", getI2STone().c_get()); setI2STone(); goto exit; }
    if (name.equals("sdr_EQ_HP"))       { c.assignf("%i dB", val); txt_EQ_highPass.writeText(c.c_get()); s_tone.HP  = val; webSrv.send("settone=", getI2STone().c_get()); setI2STone(); goto exit; }
    if (name.equals("sdr_EQ_BAL"))      { c.assignf("%i dB", val); txt_EQ_balance.writeText(c.c_get());  s_tone.BAL = val; webSrv.send("settone=", getI2STone().c_get()); setI2STone(); goto exit; }
    if (name.equals("pgb_PL_progress")) { goto exit; }
    if (name.equals("pgb_DL_progress")) { goto exit; }

    MWR_LOG_WARN("unused event: graphicObject %s was changed, val %li", name.c_get(), val);
exit:
    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnClick(ps_ptr<char> name, uint8_t val) { // val = 0 --> is inactive

    // all state
    if (name.equals("dispHeader"))                 { goto exit; }
    if (name.equals("header_Item"))                { goto exit; }
    if (name.equals("timeString"))                 { goto exit; }
    if (name.equals("dispFooter"))                 { goto exit; }
    if (name.equals("footer_StaNr"))               { goto exit; }
    if (name.equals("footer_Antenna"))             { goto exit; }
    if (name.equals("footer_Flag"))                { goto exit; }
    if (name.equals("footer_OffTimer"))            { goto exit; }
    if (name.equals("footer_Hourglass"))           { goto exit; }
    if (name.equals("footer_BitRate"))             { goto exit; }
    if (name.equals("footer_IPaddr"))              { goto exit; }

    if (s_state == RADIO) {
        if (val && name.equals("btn_RA_mute"))     { setTimeCounter(2); if (!s_f_mute) s_f_muteIsPressed = true; goto exit; }
        if (val && name.equals("btn_RA_prevSta"))  { setTimeCounter(2); goto exit; }
        if (val && name.equals("btn_RA_nextSta"))  { setTimeCounter(2); goto exit; }
        if (val && name.equals("btn_RA_staList"))  { goto exit; }
        if (val && name.equals("btn_RA_player"))   { goto exit; }
        if (val && name.equals("btn_RA_dlna"))     { goto exit; }
        if (val && name.equals("btn_RA_clock"))    { goto exit; }
        if (val && name.equals("btn_RA_sleep"))    { goto exit; }
        if (val && name.equals("btn_RA_bright"))   { goto exit; }
        if (!val && name.equals("btn_RA_bright"))  { setTimeCounter(2); goto exit; }
        if (val && name.equals("btn_RA_equal"))    { goto exit; }
        if (val && name.equals("btn_RA_bt"))       { goto exit; }
        if (!val && name.equals("btn_RA_bt"))      { setTimeCounter(2); goto exit; }
        if (val && name.equals("btn_RA_off"))      { goto exit; }
        if (val && name.equals("btn_RA_settings")) { goto exit; }
        if (val && name.equals("VUmeter_RA"))      { goto exit; }
        if (val && name.equals("txt_RA_sTitle"))   { goto exit; }
    }
    if (s_state == STATIONSLIST) {
        if (val && name.equals("lst_RADIO"))       { setTimeCounter(LIST_TIMER); goto exit; }
    }
    if (s_state == PLAYER) {
        if (val && name.equals("btn_PL_mute"))     { if (!s_f_mute) s_f_muteIsPressed = true; goto exit; }
        if (val && name.equals("btn_PL_pause"))    { goto exit; }
        if (val && name.equals("btn_PL_cancel"))   { goto exit; }
        if (val && name.equals("btn_PL_prevFile")) {
            if (s_cur_AudioFileNr > 0) {
                s_cur_AudioFileNr--;
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                showAudioFileNumber();
            }
            goto exit;
        }
        if (val && name.equals("btn_PL_nextFile")) {
            if (s_cur_AudioFileNr + 1 < s_SD_content.getSize()) {
                s_cur_AudioFileNr++;
                showFileName(s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                showAudioFileNumber();
            }
            goto exit;
        }
        if (val && name.equals("btn_PL_ready"))    { goto exit; }
        if (val && name.equals("btn_PL_playAll"))  { goto exit; }
        if (val && name.equals("btn_PL_shuffle"))  { goto exit; }
        if (val && name.equals("btn_PL_fileList")) { goto exit; }
        if (val && name.equals("btn_PL_radio"))    { goto exit; }
        if (val && name.equals("btn_PL_off"))      { goto exit; }
        if (val && name.equals("btn_PL_playPrev")) { s_cur_AudioFileNr = s_SD_content.getPrevAudioFile(s_cur_AudioFileNr); goto exit; }
        if (val && name.equals("btn_PL_playNext")) { s_cur_AudioFileNr = s_SD_content.getNextAudioFile(s_cur_AudioFileNr); goto exit; }
        if (val && name.equals("pgb_PL_progress")) { goto exit; }
        if (val && name.equals("txt_PL_fName"))    { goto exit; }
        if (val && name.equals("sdr_PL_volume"))   { goto exit; }
    }
    if (s_state == AUDIOFILESLIST) {
        if (val && name.equals("lst_PLAYER")) { setTimeCounter(LIST_TIMER); goto exit; }
    }
    if (s_state == DLNA) {
        if (val && name.equals("btn_DL_mute"))     { if (!s_f_mute) s_f_muteIsPressed = true; goto exit; }
        if (val && name.equals("btn_DL_pause"))    { goto exit; }
        if (val && name.equals("btn_DL_radio"))    { goto exit; }
        if (val && name.equals("btn_DL_fileList")) { goto exit; }
        if (val && name.equals("btn_DL_cancel"))   { clearStationName(); btn_DL_pause.setActive(false); goto exit; }
        if (val && name.equals("pgb_DL_progress")) { goto exit; }
    }
    if (s_state == DLNAITEMSLIST) {
        if (val && name.equals("lst_DLNA")) {
            setTimeCounter(LIST_TIMER * 3);
            s_f_dlnaWaitForResponse = true;
            goto exit;
        }
    }
    if (s_state == CLOCK) {
        if (val && name.equals("btn_CL_mute"))         { if (!s_f_mute) { s_f_muteIsPressed = true; } goto exit; }
        if (val && name.equals("btn_CL_alarm"))        { goto exit; }
        if (val && name.equals("btn_CL_radio"))        { goto exit; }
        if (val && name.equals("clk_CL_24"))           { goto exit; }
        if (val && name.equals("btn_CL_off"))          { goto exit; }
    }
    if (s_state == ALARMCLOCK) {
        if (val && name.equals("clk_AC_red"))          { goto exit; }
        if (val && name.equals("btn_AC_left"))         { goto exit; }
        if (val && name.equals("btn_AC_right"))        { goto exit; }
        if (val && name.equals("btn_AC_up"))           { goto exit; }
        if (val && name.equals("btn_AC_down"))         { goto exit; }
        if (val && name.equals("btn_AC_ready"))        { goto exit; }
        if (val && name.starts_with("txt_alarm_days")) { goto exit; }
        if (val && name.starts_with("txt_alarm_time")) { goto exit; }
    }
    if (s_state == SLEEPTIMER) {
        if (val && name.equals("btn_SL_up"))      { goto exit; }
        if (val && name.equals("btn_SL_down"))    { goto exit; }
        if (val && name.equals("btn_SL_ready"))   { goto exit; }
        if (val && name.equals("btn_SL_cancel"))  { goto exit; }
    }
    if (s_state == SETTINGS) {
        if (val && name.equals("btn_SE_bright"))  { goto exit; }
        if (val && name.equals("btn_SE_equal"))   { goto exit; }
        if (val && name.equals("btn_SE_wifi"))    { goto exit; }
        if (val && name.equals("btn_SE_radio"))   { goto exit; }
    }
    if (s_state == BRIGHTNESS) {
        if (val && name.equals("btn_BR_ready"))   { goto exit; }
        if (val && name.equals("pic_BR_logo"))    { goto exit; }
    }
    if (s_state == EQUALIZER) {
        if (val && name.equals("btn_EQ_LP"))      { sdr_EQ_lowPass.setValue(0);  goto exit; }
        if (val && name.equals("btn_EQ_BP"))      { sdr_EQ_bandPass.setValue(0); goto exit; }
        if (val && name.equals("btn_EQ_HP"))      { sdr_EQ_highPass.setValue(0); goto exit; }
        if (val && name.equals("btn_EQ_BAL"))     { sdr_EQ_balance.setValue(0);  goto exit; }
        if (val && name.equals("btn_EQ_Radio"))   { goto exit; }
        if (val && name.equals("btn_EQ_Player"))  { goto exit; }
        if (val && name.equals("btn_EQ_mute"))    { if (!s_f_mute) s_f_muteIsPressed = true; goto exit; }
    }
    if (s_state == BLUETOOTH) {
        if (val && name.equals("btn_BT_pause"))   { bt_emitter.pauseResume(); goto exit; }
        if (val && name.equals("btn_BT_radio"))   { goto exit; }
        if (val && name.equals("btn_BT_volDown")) { bt_emitter.downvolume(); goto exit; }
        if (val && name.equals("btn_BT_volUp"))   { bt_emitter.upvolume();   goto exit; }
        if (val && name.equals("btn_BT_mode"))    { bt_emitter.changeMode(); goto exit; }
        if (val && name.equals("btn_BT_power"))   { goto exit; }
        if (val && name.equals("txt_BT_mode"))    { goto exit; }
    }
    if (s_state == IR_SETTINGS) {
        if (val && name.equals("btn_IR_radio"))   { goto exit; }
    }
    if (s_state == WIFI_SETTINGS) {
        if (val && name.equals("key_WI_input")) {
            MWR_LOG_DEBUG("val %i", val);
            if (val == 13) {
                changeState(RADIO, 0);
                goto exit;
            }
        }
        if(name.starts_with("txt_btn"))                        { goto exit; }
        if (val && name.equals("btn_SE_wifi"))                 { goto exit; }
        if (val && name.equals("select_txtbtn_down"))          { goto exit; }
        if (val && name.equals("wifiSettings_selectbox_ssid")) { goto exit; }
        if (val && name.equals("wifiSettings_selectbox_ssid")) { goto exit; }
        if (val && name.equals("wifiSettings_keyBoard"))       { goto exit; }
    }
    MWR_LOG_WARN("unused event: graphicObject %s was clicked", name.c_get());
exit:
    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnRelease(ps_ptr<char> name, releasedArg ra) {

    // all state
    if (name.equals("dispHeader")) { goto exit; }
    if (name.equals("dispFooter")) { goto exit; }

    if (s_state == RADIO) {
        if (name.equals("btn_RA_mute"))     { muteChanged(btn_RA_mute.getValue()); goto exit; }
        if (name.equals("btn_RA_prevSta"))  { prevFavStation(); dispFooter.updateStation(s_cur_station); goto exit; }
        if (name.equals("btn_RA_nextSta"))  { nextFavStation(); dispFooter.updateStation(s_cur_station); goto exit; }
        if (name.equals("btn_RA_staList"))  { changeState(STATIONSLIST, 0); goto exit; }
        if (name.equals("btn_RA_player"))   { changeState(PLAYER, 0);     if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_PL(0); } goto exit; }
        if (name.equals("btn_RA_dlna"))     { changeState(DLNA, 0);       if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_DL(0); } goto exit; }
        if (name.equals("btn_RA_clock"))    { changeState(CLOCK, 0);                                                                    goto exit; }
        if (name.equals("btn_RA_sleep"))    { changeState(SLEEPTIMER, 0); if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_SL(0); } goto exit; }
        if (name.equals("btn_RA_settings")) { changeState(SETTINGS, 0);   if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_SE(0); } goto exit; }
        if (name.equals("btn_RA_equal"))    { changeState(EQUALIZER, 0);  if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_EQ(0); } goto exit; }
        if (name.equals("btn_RA_bt"))       { changeState(BLUETOOTH, 0);  if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_BT(0); } goto exit; }
        if (name.equals("btn_RA_off"))      { fall_asleep(); goto exit; }
        if (name.equals("VUmeter_RA"))      { goto exit; }
        if (name.equals("sdr_RA_volume"))   { goto exit; }
    }
    if (s_state == STATIONSLIST) {
        if (name.equals("lst_RADIO"))       { if (ra.val1) { setStationByNumber(ra.val1); changeState(RADIO, 0); } goto exit; }
    }
    if (s_state == PLAYER) {
        if (name.equals("btn_PL_mute"))     { muteChanged(btn_PL_mute.getValue()); goto exit; }
        if (name.equals("btn_PL_pause"))    { if (s_f_isFSConnected) { s_f_pauseResume = audio.pauseResume(); } goto exit; }
        if (name.equals("btn_PL_cancel"))   { changeState(PLAYER, 0); if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_PL(0); } goto exit; }
        if (name.equals("btn_PL_prevFile")) { if(s_ir_btn_select == 0) set_ir_pos_PL(0); goto exit; }
        if (name.equals("btn_PL_nextFile")) { if(s_ir_btn_select == 1) set_ir_pos_PL(0); goto exit; }
        if (name.equals("btn_PL_ready"))    { SD_playFile(s_cur_AudioFolder.c_get(), s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr));
                                              changeState(PLAYER, 1); if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_PL(0); } showAudioFileNumber(); goto exit; }
        if (name.equals("btn_PL_playAll"))  { if(playlist.create_playlist_from_SD_folder(s_cur_AudioFolder)){
                                                  playlist.sort_alphabetical(); s_subState_player = 1; s_f_playlistEnabled = true; }
                                              goto exit; }
        if (name.equals("btn_PL_shuffle"))  { if(playlist.create_playlist_from_SD_folder(s_cur_AudioFolder)){
                                                  playlist.sort_random(); s_subState_player = 1; s_f_playlistEnabled = true; }
                                              goto exit; }
        if (name.equals("btn_PL_fileList")) { s_SD_content.listFilesInDir(s_cur_AudioFolder.c_get(), true, false); changeState(AUDIOFILESLIST, 0); goto exit; }
        if (name.equals("btn_PL_radio"))    { setStation(s_cur_station); goto exit; }
        if (name.equals("btn_PL_off"))      { fall_asleep(); goto exit; }
        if (name.equals("sdr_PL_volume"))   { goto exit; }
        if (name.equals("btn_PL_playNext")) { SD_playFile(s_cur_AudioFolder.c_get(), s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber(); goto exit; }
        if (name.equals("btn_PL_playPrev")) { SD_playFile(s_cur_AudioFolder.c_get(), s_SD_content.getColouredSStringByIndex(s_cur_AudioFileNr)); showAudioFileNumber(); goto exit; }
        if (name.equals("pgb_PL_progress")) { audio.setTimeOffset(ra.val2); goto exit; }
    }
    if (s_state == AUDIOFILESLIST) {
        if (name.equals("lst_PLAYER"))      { if (ra.val1 == 1) { ; } // wipe up/down
                                              if (ra.val1 == 2) {     // next prev folder
                                                  s_cur_AudioFolder = ra.arg1;
                                                  s_cur_AudioFileNr = ra.val2;
                                                  lst_PLAYER.show(s_cur_AudioFolder, s_cur_AudioFileNr);
                                              }
                                              if (ra.val1 == 3) {     // audio file
                                                  s_cur_AudioFolder = ra.arg1;
                                                  s_cur_AudioFileNr = ra.val2;
                                                  stopSong();
                                                  SD_playFile(ra.arg3.c_get());
                                              }
                                              goto exit; }
    }
    if (s_state == DLNA) {
        if (name.equals("btn_DL_mute"))     { muteChanged(btn_DL_mute.getValue());   if(s_ir_btn_select == 0) set_ir_pos_DL(0); goto exit; }
        if (name.equals("btn_DL_pause"))    { s_f_pauseResume = audio.pauseResume(); if(s_ir_btn_select == 1) set_ir_pos_DL(0); goto exit; }
        if (name.equals("btn_DL_cancel"))   { stopSong();
                                              txt_DL_fName.setText("");
                                              txt_DL_fName.show(true, false);
                                              pgb_DL_progress.reset();
                                              btn_DL_pause.setActive(false);
                                              btn_DL_pause.show();
                                              if(s_ir_btn_select == 3) set_ir_pos_DL(0);
                                              goto exit; }
        if (name.equals("btn_DL_fileList")) { changeState(DLNAITEMSLIST, 0); txt_DL_fName.setText(""); goto exit; }
        if (name.equals("btn_DL_radio"))    { setStation(s_cur_station); goto exit; }
        if (name.equals("sdr_DL_volume"))   { goto exit; }
        if (name.equals("pgb_DL_progress")) { audio.setTimeOffset(ra.val2); goto exit; }
    }
    if (s_state == DLNAITEMSLIST) {
        if (name.equals("lst_DLNA"))        {   if (ra.val1 == 0) { // wipe up/down
                                                    goto exit;
                                                }
                                                if (ra.val1 == 1) { // play a file
                                                    txt_DL_fName.setTextColor(TFT_CYAN);
                                                    txt_DL_fName.setText(ra.arg2.c_get());
                                                    connecttohost(ra.arg1);
                                                    changeState(DLNA, 0);
                                                    goto exit;
                                                }
                                                if (ra.val1 == 2) {// browse dlna object, waiting for content and create a playlist
                                                    dlna.browseServer(ra.val2, ra.arg1.c_get(), 0, 50);
                                                    s_f_dlnaMakePlaylistOTF = true;
                                                    goto exit;
                                                }
                                                else {
                                                    MWR_LOG_WARN("unknown val: %i", ra.val1);
                                                }
                                            }
    }
    if (s_state == CLOCK) {
        if (name.equals("btn_CL_mute"))     { muteChanged(btn_CL_mute.getValue()); if(s_ir_btn_select == 2) set_ir_pos_CL(2); goto exit; }
        if (name.equals("btn_CL_alarm"))    { changeState(ALARMCLOCK, 0); if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_AC(0); } goto exit; }
        if (name.equals("btn_CL_radio"))    { changeState(RADIO, 0); goto exit; }
        if (name.equals("clk_CL_24"))       { changeState(CLOCK, 0); goto exit; }
        if (name.equals("btn_CL_off"))      { fall_asleep(); goto exit; }
        if (name.equals("sdr_CL_volume"))   { goto exit; }
    }
    if (s_state == ALARMCLOCK) {
        if (name.equals("clk_AC_red"))      { goto exit; }
        if (name.equals("btn_AC_left"))     { clk_AC_red.shiftLeft();  if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_AC(0); } goto exit; }
        if (name.equals("btn_AC_right"))    { clk_AC_red.shiftRight(); if(s_f_ok_from_ir) { s_ir_btn_select = 1; set_ir_pos_AC(1); } goto exit; }
        if (name.equals("btn_AC_up"))       { clk_AC_red.digitUp();    if(s_f_ok_from_ir) { s_ir_btn_select = 2; set_ir_pos_AC(2); } goto exit; }
        if (name.equals("btn_AC_down"))     { clk_AC_red.digitDown();  if(s_f_ok_from_ir) { s_ir_btn_select = 3; set_ir_pos_AC(3); } goto exit; }
        if (name.equals("btn_AC_ready"))    { updateSettings(); changeState(CLOCK, 0); logAlarmItems(); goto exit; }
    }
    if (s_state == SLEEPTIMER) {
        if (name.equals("btn_SL_up"))       { display_sleeptime(1);  if(s_ir_btn_select == 0) set_ir_pos_SL(0); goto exit; }
        if (name.equals("btn_SL_down"))     { display_sleeptime(-1); if(s_ir_btn_select == 1) set_ir_pos_SL(1); goto exit; }
        if (name.equals("btn_SL_ready"))    { dispFooter.updateOffTime(s_sleeptime); changeState(RADIO, 0); goto exit; }
        if (name.equals("btn_SL_cancel"))   { changeState(RADIO, 0); goto exit; }
    }
    if (s_state == BRIGHTNESS) {
        if (name.equals("btn_BR_ready"))    { changeState(RADIO, 0); goto exit;}
        if (name.equals("pic_BR_logo"))     { goto exit; }
        if (name.equals("sdr_BR_value"))    { goto exit; }
    }
    if (s_state == EQUALIZER) {
        if (name.equals("btn_EQ_Radio"))    { changeState(RADIO, 0); goto exit; }
        if (name.equals("btn_EQ_Player"))   { changeState(PLAYER, 0); goto exit; }
        if (name.equals("btn_EQ_mute"))     { muteChanged(btn_EQ_mute.getValue()); if(s_ir_btn_select == 2) set_ir_pos_EQ(2); goto exit; }
        if (name.equals("btn_EQ_BAL"))      {                                      if(s_ir_btn_select == 3) set_ir_pos_EQ(3); goto exit; }
        if (name.equals("btn_EQ_LP"))       {                                      if(s_ir_btn_select == 4) set_ir_pos_EQ(4); goto exit; }
        if (name.equals("btn_EQ_BP"))       {                                      if(s_ir_btn_select == 5) set_ir_pos_EQ(5); goto exit; }
        if (name.equals("btn_EQ_HP"))       {                                      if(s_ir_btn_select == 6) set_ir_pos_EQ(6); goto exit; }
        if (name.equals("sdr_EQ_HP"))       { goto exit; }
        if (name.equals("sdr_EQ_BP"))       { goto exit; }
        if (name.equals("sdr_EQ_LP"))       { goto exit; }
        if (name.equals("sdr_EQ_BAL"))      { goto exit; }
    }
    if (s_state == SETTINGS) {
        if (name.equals("btn_SE_bright"))   { changeState(BRIGHTNESS, 0);    if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_BR(0); } goto exit; }
        if (name.equals("btn_SE_equal"))    { changeState(EQUALIZER, 0);     if(s_f_ok_from_ir) { s_ir_btn_select = 0; set_ir_pos_EQ(0); } goto exit; }
        if (name.equals("btn_SE_wifi"))     { changeState(WIFI_SETTINGS, 0); if(s_f_ok_from_ir) goto exit; }
        if (name.equals("btn_SE_radio"))    { changeState(RADIO, 0); goto exit; }
    }
    if (s_state == BLUETOOTH) {
        if (name.equals("btn_BT_volDown"))  { if(s_ir_btn_select == 0) set_ir_pos_BT(0); goto exit; }
        if (name.equals("btn_BT_volUp"))    { if(s_ir_btn_select == 1) set_ir_pos_BT(1); goto exit; }
        if (name.equals("btn_BT_pause"))    { if(s_ir_btn_select == 2) set_ir_pos_BT(2); goto exit; }
        if (name.equals("btn_BT_mode"))     { if(s_ir_btn_select == 3) set_ir_pos_BT(3); goto exit; }
        if (name.equals("btn_BT_radio"))    { changeState(RADIO, 0); goto exit; }
        if (name.equals("btn_BT_power"))    { if(s_ir_btn_select == 5 && s_bt_emitter.found) set_ir_pos_BT(5); s_bt_emitter.enabled = !s_bt_emitter.enabled; goto exit; }
    }
    if (s_state == IR_SETTINGS) {
        if (name.equals("btn_IR_radio"))    { changeState(RADIO, 0); goto exit; }
    }
    if (s_state == WIFI_SETTINGS) {
        if (name.equals("wifiSettings"))    { setWiFiCredentials(ra.arg1.c_get(), ra.arg2.c_get());
                                              msg_box.setText("ESP restart", false, false);
                                              msg_box.show();
                                              vTaskDelay(4000);
                                              s_f_msg_box = true;
                                              s_timestamp = millis() + 4000;
                                              s_f_esp_restart = true;
                                              goto exit; }
    }
    MWR_LOG_WARN("unused event: graphicObject %s was released", name.c_get());
exit:
    s_f_ok_from_ir = false;
    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// clang-format on
/*🟢🟡🔴*/