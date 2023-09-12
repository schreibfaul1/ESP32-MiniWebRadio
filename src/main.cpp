#include "common.h"
// clang-format off
/*****************************************************************************************************************************************************
    MiniWebRadio -- Webradio receiver for ESP32

    first release on 03/2017                                                                                                       */String Version="\
    Version 2.11a Sep 02/2023                                                                                         ";

/*  2.8" color display (320x240px) with controller ILI9341 or HX8347D (SPI) or
    3.5" color display (480x320px) wiht controller ILI9486 or ILI9488 (SPI)

    HW decoder VS1053 or
    SW decoder with external DAC over I2S

    SD_MMC is mandatory
    IR remote is optional

*****************************************************************************************************************************************************/

// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

// clang-format on

// global variables
const uint8_t  _max_volume = 21;
const uint16_t _max_stations = 1000;
int8_t         _releaseNr = -1;
int8_t         _currentServer = -1;
uint8_t        _alarmdays = 0;
uint8_t        _cur_volume = 0;           // will be set from stored preferences
uint8_t        _ringvolume = _max_volume; //
uint8_t        _mute_volume = 0;          // decrement to 0 or increment to _cur_volume
uint8_t        _brightness = 0;
uint8_t        _state = 0;                // statemaschine
uint8_t        _commercial_dur = 0;       // duration of advertising
uint8_t        _cur_Codec = 0;
uint8_t        _VUleftCh = 0;             // VU meter left channel
uint8_t        _VUrightCh = 0;            // VU meter right channel
uint8_t        _numServers = 0;           //
uint8_t        _level = 0;
uint8_t        _timeFormat = 24;          // 24 or 12
uint8_t        _staListPos = 0;
uint16_t       _staListNr = 0;
uint8_t        _fileListPos = 0;
uint16_t       _fileListNr = 0;
uint8_t        _itemListPos = 0;          // DLNA items
uint16_t       _itemListNr = 0;
int16_t        _alarmtime = 0;            // in minutes (23:59 = 23 *60 + 59)
int16_t        _toneha = 0;               // BassFreq 0...15        VS1053
int16_t        _tonehf = 0;               // TrebleGain 0...14      VS1053
int16_t        _tonela = 0;               // BassGain 0...15        VS1053
int16_t        _tonelf = 0;               // BassFreq 0...13        VS1053
int16_t        _toneLP = 0;               // -40 ... +6 (dB)        audioI2S
int16_t        _toneBP = 0;               // -40 ... +6 (dB)        audioI2S
int16_t        _toneHP = 0;               // -40 ... +6 (dB)        audioI2S
uint16_t       _icyBitRate = 0;           // from http response header via event
uint16_t       _avrBitRate = 0;           // from decoder via getBitRate(true)
uint16_t       _cur_station = 0;          // current station(nr), will be set later
uint16_t       _sleeptime = 0;            // time in min until MiniWebRadio goes to sleep
uint16_t       _sum_stations = 0;
uint32_t       _resumeFilePos = 0;        //
uint32_t       _playlistTime = 0;         // playlist start time millis() for timeout
uint32_t       _settingsHash = 0;
uint32_t       _audioFileSize = 0;
uint32_t       _media_downloadPort = 0;
const char*    _pressBtn[8];
const char*    _releaseBtn[8];
char           _chbuf[512];
char           _fName[256];
char           _myIP[25];
char           _path[128];
char           _prefix[5] = "/s";
char           _commercial[25];
char           _icyDescription[512] = {};
char           _streamTitle[512] = {};
char*          _lastconnectedfile = nullptr;
char*          _stationURL = nullptr;
boolean        _f_rtc = false; // true if time from ntp is received
boolean        _f_100ms = false;
boolean        _f_1sec = false;
boolean        _f_10sec = false;
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
boolean        _f_newStreamTitle = false;
boolean        _f_newBitRate = false;
boolean        _f_newLogoAndStation = false;
boolean        _f_newCommercial = false;
boolean        _f_volBarVisible = false;
boolean        _f_switchToClock = false;    // jump into CLOCK mode at the next opportunity
boolean        _f_hpChanged = false;        // true, if HeadPhone is plugged or unplugged
boolean        _f_muteIncrement = false;    // if set increase Volume (from 0 to _cur_volume)
boolean        _f_muteDecrement = false;    // if set decrease Volume (from _cur_volume to 0)
boolean        _f_timeAnnouncement = false; // time announcement every full hour
boolean        _f_playlistEnabled = false;
boolean        _f_playlistNextFile = false;
boolean        _f_logoUnknown = false;
boolean        _f_pauseResume = false;
boolean        _f_accessPoint = false;
boolean        _f_state_isChanging = false;
boolean        _f_SD_Upload = false;
boolean        _f_PSRAMfound = false;
boolean        _f_SD_MMCfound = false;
boolean        _f_ESPfound = false;
String         _station = "";
String         _stationName_nvs = "";
String         _stationName_air = "";
String         _homepage = "";
String         _filename = "";
String         _lastconnectedhost = "";
String         _scannedNetworks = "";
String         _curAudioFolder = "/audiofiles";
String         _TZName = "Europe/Berlin";
String         _TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
String         _media_downloadIP = "";
std::vector<String> _names{};
std::vector<char*>  _SD_content;

struct dlna_items{
    std::vector<char*> name{};
    std::vector<uint8_t> isDir; // contains boolean
    std::vector<char*> id{};
    std::vector<size_t> size;
    std::vector<char*> uri{};
    std::vector<uint8_t> isAudio; // contains boolean
    std::vector<char*> serverFriendlyName{};
    std::vector<uint8_t> serverId;
    boolean isReady = true;
    std::vector<char*> path{};
    uint8_t level = 0;
} _dlna_items;
struct timecounter {
    uint8_t timer = 0;
    float factor = 2.0;
} _timeCounter;

char _hl_item[13][40]{
    "   Internet Radio   ", // "* интернет-радио *"  "ραδιόφωνο Internet"
    "   Internet Radio   ", //
    "   Internet Radio   ", //
    "        Clock       ", // Clock "** часы́ **"  "** ρολόι **"
    "        Clock       ", //
    "     Brightness     ", // Brightness яркость λάμψη
    "    Audio player    ", // "** цифрово́й плеер **"
    "    Audio player    ", //
    "    Alarm (hh:mm)   ", // Alarm
    "  Off Timer (h:mm)  ", // "Sleeptimer" "Χρονομετρητής" "Таймер сна"
    "        DLNA        ", // Digital Living Network Alliance
    "   Stations List    ",
    "   Audio Files      ",
};

enum status {
    RADIO = 0,
    RADIOico = 1,
    RADIOmenue = 2,
    CLOCK = 3,
    CLOCKico = 4,
    BRIGHTNESS = 5,
    PLAYER = 6,
    PLAYERico = 7,
    ALARM = 8,
    SLEEP = 9,
    DLNA = 10,
    STATIONSLIST = 11,
    AUDIOFILESLIST = 12,
    DLNAITEMSLIST = 13
};

const char* codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "AACP", "OPUS", "OGG", "VORBIS"};

Preferences pref;
Preferences stations;
WebSrv      webSrv;
WiFiMulti   wifiMulti;
RTIME       rtc;
Ticker      ticker100ms;
IR          ir(IR_PIN); // do not change the objectname, it must be "ir"
TP          tp(TP_CS, TP_IRQ);
File        audioFile;
File        nextAudioFile;
File        playlistFile;
FtpServer   ftpSrv;
WiFiClient  client;
WiFiUDP     udp;
SoapESP32   soap(&client, &udp);

#if DECODER == 2 // ac101
AC101 dac;
#endif
#if DECODER == 3 // es8388
ES8388 dac;
#endif
#if DECODER == 4 // wm8978
WM8978 dac;
#endif

SemaphoreHandle_t mutex_rtc;
SemaphoreHandle_t mutex_display;

#if TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1
// clang-format off
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
const unsigned short* _fonts[7] = {
    Times_New_Roman15x14, Times_New_Roman21x17, Times_New_Roman27x21,
    Times_New_Roman34x27, Times_New_Roman38x31, Times_New_Roman43x35,
    Big_Numbers133x156  // ASCII 32...64 only
};

struct w_h {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 320; uint16_t h = 20; } const _winHeader;
struct w_l {uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 100; uint16_t h = 100;} const _winLogo;
struct w_n {uint16_t x = 100; uint16_t y = 20;  uint16_t w = 220; uint16_t h = 100;} const _winName;
struct w_e {uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 320; uint16_t h = 100;} const _winFName;
struct w_t {uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;} const _winTitle;
struct w_c {uint16_t x = 0;   uint16_t y = 120; uint16_t w = 296; uint16_t h = 100;} const _winSTitle;
struct w_g {uint16_t x = 296; uint16_t y = 120; uint16_t w =  24; uint16_t h = 100;} const _winVUmeter;
struct w_f {uint16_t x = 0;   uint16_t y = 220; uint16_t w = 320; uint16_t h = 20; } const _winFooter;
struct w_i {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 180; uint16_t h = 20; } const _winItem;
struct w_v {uint16_t x = 180; uint16_t y = 0;   uint16_t w =  50; uint16_t h = 20; } const _winVolume;
struct w_m {uint16_t x = 260; uint16_t y = 0;   uint16_t w =  60; uint16_t h = 20; } const _winTime;
struct w_s {uint16_t x = 0;   uint16_t y = 220; uint16_t w =  60; uint16_t h = 20; } const _winStaNr;
struct w_p {uint16_t x = 60;  uint16_t y = 220; uint16_t w =  65; uint16_t h = 20; } const _winSleep;
struct w_r {uint16_t x = 120; uint16_t y = 220; uint16_t w =  24; uint16_t h = 20; } const _winRSSID;
struct w_u {uint16_t x = 144; uint16_t y = 220; uint16_t w =  36; uint16_t h = 20; } const _winBitRate;
struct w_a {uint16_t x = 180; uint16_t y = 220; uint16_t w = 160; uint16_t h = 20; } const _winIPaddr;
struct w_b {uint16_t x = 0;   uint16_t y = 166; uint16_t w = 320; uint16_t h =  6; } const _winVolBar;
struct w_o {uint16_t x = 0;   uint16_t y = 180; uint16_t w =  40; uint16_t h = 40; } const _winButton;
struct w_d {uint16_t x =   0; uint16_t y =  60; uint16_t w = 320; uint16_t h = 120;} const _winDigits;
struct w_y {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h =  40;} const _winAlarmDays;
struct w_w {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 200;} const _winWoHF; // without Header and Footer
uint16_t _alarmdaysXPos[7] = {3, 48, 93, 138, 183, 228, 273};
uint16_t _alarmtimeXPos7S[5] = {2, 75, 148, 173, 246}; // seven segment digits
uint16_t _alarmtimeXPosFN[6] = {0, 56, 112, 152, 208, 264}; // folded numbers
uint16_t _sleeptimeXPos[5] = {5, 77, 129, 57}; // last is colon
uint8_t  _alarmdays_w = 44 + 4;
uint8_t  _alarmdays_h = 40;
uint16_t _dispWidth   = 320;
uint16_t _dispHeight  = 240;
uint8_t  _tftSize     = 0;
uint8_t  _irNumber_x  = 25;
uint8_t  _irNumber_y  = 40;
//
TFT tft(TFT_CONTROLLER, DISPLAY_INVERSION);
//
// clang-format on
#endif // TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1

#if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5 || TFT_CONTROLLER == 6
// clang-format off
//
//  Display 480x320
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       _winHeader=30px
//  +-------------------------------------------+ _yName=30
//  |                                           |
//  | Logo                   StationName        |       _winFName=130px
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
    Times_New_Roman27x21, Times_New_Roman34x27, Times_New_Roman38x31,
    Times_New_Roman43x35, Times_New_Roman56x46, Times_New_Roman66x53,
    Big_Numbers133x156  // ASCII 32...64 only
};

struct w_h {uint16_t x =   0; uint16_t y =   0; uint16_t w = 480; uint16_t h =  30;} const _winHeader;
struct w_l {uint16_t x =   0; uint16_t y =  30; uint16_t w = 130; uint16_t h = 132;} const _winLogo;
struct w_n {uint16_t x = 130; uint16_t y =  30; uint16_t w = 350; uint16_t h = 132;} const _winName;
struct w_e {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 132;} const _winFName;
struct w_t {uint16_t x =   0; uint16_t y = 162; uint16_t w = 480; uint16_t h = 128;} const _winTitle;
struct w_c {uint16_t x =   0; uint16_t y = 162; uint16_t w = 448; uint16_t h = 128;} const _winSTitle;
struct w_g {uint16_t x = 448; uint16_t y = 162; uint16_t w =  32; uint16_t h = 128;} const _winVUmeter;
struct w_f {uint16_t x =   0; uint16_t y = 290; uint16_t w = 480; uint16_t h =  30;} const _winFooter;
struct w_m {uint16_t x = 390; uint16_t y =   0; uint16_t w =  90; uint16_t h =  30;} const _winTime;
struct w_i {uint16_t x =   0; uint16_t y =   0; uint16_t w = 280; uint16_t h =  30;} const _winItem;
struct w_v {uint16_t x = 280; uint16_t y =   0; uint16_t w = 110; uint16_t h =  30;} const _winVolume;
struct w_s {uint16_t x =   0; uint16_t y = 290; uint16_t w =  85; uint16_t h =  30;} const _winStaNr;
struct w_p {uint16_t x =  85; uint16_t y = 290; uint16_t w =  87; uint16_t h =  30;} const _winSleep;
struct w_r {uint16_t x = 172; uint16_t y = 290; uint16_t w =  30; uint16_t h =  30;} const _winRSSID;
struct w_u {uint16_t x = 202; uint16_t y = 290; uint16_t w =  58; uint16_t h =  30;} const _winBitRate;
struct w_a {uint16_t x = 260; uint16_t y = 290; uint16_t w = 220; uint16_t h =  30;} const _winIPaddr;
struct w_b {uint16_t x =   0; uint16_t y = 222; uint16_t w = 480; uint16_t h =   8;} const _winVolBar;
struct w_o {uint16_t x =   0; uint16_t y = 234; uint16_t w =  56; uint16_t h =  56;} const _winButton;
struct w_d {uint16_t x =   0; uint16_t y =  70; uint16_t w = 480; uint16_t h = 160;} const _winDigits;
struct w_y {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h =  40;} const _winAlarmDays;
struct w_w {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 260;} const _winWoHF; // without Header and Footer
uint16_t _alarmdaysXPos[7] = {2, 70, 138, 206, 274, 342, 410};
uint16_t _alarmtimeXPos7S[5] = {12, 118, 224, 266, 372}; // seven segment digits
uint16_t _alarmtimeXPosFN[6] = {16, 96, 176, 224, 304, 384}; // folded numbers
uint16_t _sleeptimeXPos[5] = {5, 107, 175, 73 };
uint8_t  _alarmdays_w = 64 + 4;
uint8_t  _alarmdays_h = 56;
uint16_t _dispWidth   = 480;
uint16_t _dispHeight  = 320;
uint8_t  _tftSize     = 1;
uint8_t  _irNumber_x  = 100;
uint8_t  _irNumber_y  = 80;
//
TFT tft(TFT_CONTROLLER, DISPLAY_INVERSION);
//
// clang-format on
#endif // #if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5|| TFT_CONTROLLER == 6

/*****************************************************************************************************************************************************
 *                                                    D E F A U L T S E T T I N G S                                                                  *
 *****************************************************************************************************************************************************/
// clang-format off
boolean defaultsettings(){
    if(!SD_MMC.exists("/settings.json")){
        File file = SD_MMC.open("/settings.json","w", true);
        JSONVar jObject;
        jObject["volume"]            = (uint8_t)  12; // 0...21
        jObject["ringvolume"]        = (uint8_t)  21;
        jObject["alarmtime"]         = (uint16_t) 0;
        jObject["alarm_weekdays"]    = (uint8_t)  0;
        jObject["timeAnnouncing"]    = (bool)     true;
        jObject["mute"]              = (bool)     false; // no mute
        jObject["brightness"]        = (uint8_t)  100;  // 0...100
        jObject["sleeptime"]         = (uint16_t) 0;
        jObject["lastconnectedhost"] = (String)   "";
        jObject["station"]           = (uint16_t) 1;
        jObject["sumstations"]       = (uint16_t) 0;
        jObject["Timezone_Name"]     = (String)   "Europe/Berlin";
        jObject["Timezone_String"]   = (String)   "CET-1CEST,M3.5.0,M10.5.0/3";
        jObject["toneha"]            = (int16_t)  0; // BassFreq 0...15        VS1053
        jObject["tonehf"]            = (int16_t)  8; // TrebleGain 0...14      VS1053
        jObject["tonela"]            = (int16_t)  8; // BassGain 0...15        VS1053
        jObject["tonelf"]            = (int16_t)  9; // BassFreq 0...13        VS1053
        jObject["toneLP"]            = (int16_t)  0; // -40 ... +6 (dB)        audioI2S
        jObject["toneBP"]            = (int16_t)  0; // -40 ... +6 (dB)        audioI2S
        jObject["toneHP"]            = (int16_t)  0; // -40 ... +6 (dB)        audioI2S
        jObject["timeFormat"]        = (int8_t)   24;
        String jO = JSON.stringify(jObject);
        file.print(jO);
    }

    File file = SD_MMC.open("/settings.json","r", false);
    String jO = file.readString();
    _settingsHash = simpleHash(jO.c_str());
    JSONVar jV = JSON.parse(jO);
    _cur_volume         = (uint8_t)     jV["volume"];
    _ringvolume         = (uint8_t)     jV["ringvolume"];
    _alarmtime          = (uint16_t)    jV["alarmtime"];
    _alarmdays          = (uint8_t)     jV["alarm_weekdays"];
    _f_timeAnnouncement = (bool)        jV["timeAnnouncing"];
    _f_mute             = (bool)        jV["mute"];
    _brightness         = (uint8_t)     jV["brightness"];
    _sleeptime          = (uint16_t)    jV["sleeptime"];
    _cur_station        = (uint16_t)    jV["station"];
    _sum_stations       = (uint16_t)    jV["sumstations"];
    _toneha             = (int16_t)     jV["toneha"];
    _tonehf             = (int16_t)     jV["tonehf"];
    _tonela             = (int16_t)     jV["tonela"];
    _tonelf             = (int16_t)     jV["tonelf"];
    _toneLP             = (int16_t)     jV["toneLP"];
    _toneBP             = (int16_t)     jV["toneBP"];
    _toneHP             = (int16_t)     jV["toneHP"];
    _timeFormat         = (uint8_t)     jV["timeFormat"];

    bool f_updateSettings = false;
    if(!(const char*)jV["Timezone_Name"]     ) f_updateSettings = true;  else _TZName             = (const char*) jV["Timezone_Name"];
    if(!(const char*)jV["Timezone_String"]   ) f_updateSettings = true;  else _TZString           = (const char*) jV["Timezone_String"];
    if(!(const char*) jV["lastconnectedhost"]) f_updateSettings = true;  else _lastconnectedhost  = (const char*) jV["lastconnectedhost"];
    if(f_updateSettings) updateSettings();

    if(!pref.isKey("stations_filled")|| _sum_stations == 0) saveStationsToNVS();  // first init
    if(pref.getShort("IR_numButtons", 0) == 0) saveDefaultIRbuttonsToNVS();
    loadIRbuttonsFromNVS();
    return true;
}
// clang-format on

boolean saveStationsToNVS() {
    String   Hide = "", Cy = "", StationName = "", StreamURL = "", currentLine = "", tmp = "";
    uint16_t cnt = 0;
    // StationList
    if(!SD_MMC.exists("/stations.csv")) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/stations.csv not found");
        return false;
    }

    File file = SD_MMC.open("/stations.csv");
    if(file) {                                        // try to read from SD_MMC
        stations.clear();
        currentLine = file.readStringUntil('\n');     // read the headline
        while(file.available()) {
            currentLine = file.readStringUntil('\n'); // read the line
            uint p = 0, q = 0;
            Hide = "";
            Cy = "";
            StationName = "";
            StreamURL = "";
            for(int32_t i = 0; i < currentLine.length() + 1; i++) {
                if(currentLine[i] == '\t' || i == currentLine.length()) {
                    if(p == 0) Hide = currentLine.substring(q, i);
                    if(p == 1) Cy = currentLine.substring(q, i);
                    if(p == 2) StationName = currentLine.substring(q, i);
                    if(p == 3) StreamURL = currentLine.substring(q, i);
                    p++;
                    i++;
                    q = i;
                }
            }
            if(Hide == "*") continue;
            if(StationName == "") continue; // is empty
            if(StreamURL == "") continue;   // is empty
            SerialPrintfln("Cy=%s, StationName=%s, StreamURL=%s", Cy.c_str(), StationName.c_str(), StreamURL.c_str());
            cnt++;
            if(cnt == _max_stations) {
                SerialPrintfln(ANSI_ESC_RED "No more than %d entries in stationlist allowed!", _max_stations);
                cnt--; // maxstations 999
                break;
            }
            tmp = StationName + "#" + StreamURL;
            sprintf(_chbuf, "station_%03d", cnt);
            stations.putString(_chbuf, tmp);
        }
        _sum_stations = cnt;
        stations.putLong("stations.size", file.size());
        file.close();
        pref.putBool("stations_filled", true);
        SerialPrintfln("stationlist internally loaded");
        SerialPrintfln("number of stations: " ANSI_ESC_CYAN "%i", cnt);
        return true;
    }
    else
        return false;
}

boolean saveDefaultIRbuttonsToNVS() { // default values, first init
    pref.putShort("irAddress", 0x00);
    pref.putShort("button_0", 0x52);  // '0';
    pref.putShort("button_1", 0x16);  // '1';
    pref.putShort("button_2", 0x19);  // '2';
    pref.putShort("button_3", 0x0D);  // '3';
    pref.putShort("button_4", 0x0C);  // '4';
    pref.putShort("button_5", 0x18);  // '5';
    pref.putShort("button_6", 0x5E);  // '6';
    pref.putShort("button_7", 0x08);  // '7';
    pref.putShort("button_8", 0x1C);  // '8';
    pref.putShort("button_9", 0x5A);  // '9';
    pref.putShort("button_10", 0x40); // 'm';  // MUTE
    pref.putShort("button_11", 0x46); // 'u';  // VOLUME+
    pref.putShort("button_12", 0x15); // 'd';  // VOLUME-
    pref.putShort("button_13", 0x43); // 'p';  // PREVIOUS STATION
    pref.putShort("button_14", 0x44); // 'n';  // NEXT STATION
    pref.putShort("button_15", 0x4A); // 'k';  // CLOCK <--> RADIO
    pref.putShort("button_16", 0x42); // 's';  // OFF TIMER
    pref.putShort("button_17", 0x00); // '0';
    pref.putShort("button_18", 0x00); // '0';
    pref.putShort("button_19", 0x00); // '0';

    pref.putShort("IR_numButtons", 20);
    // log_i("saveDefaultIRbuttonsToNVS");

    loadIRbuttonsFromNVS();

    return true;
}

void saveIRbuttonsToNVS() {
    uint8_t  ir_addr = ir.get_irAddress();
    uint8_t* ir_buttons = ir.get_irButtons();
    char     buf[12];
    pref.putShort("irAddress", ir_addr);
    for(uint8_t i = 0; i < 20; i++) {
        sprintf(buf, "button_%d", i);
        pref.putShort(buf, ir_buttons[i]);
        log_i("i=%i ir_buttons[i] %i", i, ir_buttons[i]);
    }
    pref.putShort("IR_numButtons", 20);
}

void loadIRbuttonsFromNVS() {
    // load IR settings from NVS
    uint numButtons = pref.getShort("IR_numButtons", 0);
    ir.set_irAddress(pref.getShort("irAddress", 0));
    char    buf[12];
    uint8_t cmd = 0;
    for(uint i = 0; i < numButtons; i++) {
        sprintf(buf, "button_%d", i);
        cmd = pref.getShort(buf, 0);
        ir.set_irButtons(i, cmd);
    }
}

// clang-format off
void updateSettings(){
    if(!_lastconnectedhost)_lastconnectedhost = "";
    JSONVar jObject;
    jObject["volume"]            = (uint8_t)  _cur_volume;
    jObject["ringvolume"]        = (uint8_t)  _ringvolume;
    jObject["alarmtime"]         = (uint16_t) _alarmtime;
    jObject["alarm_weekdays"]    = (uint8_t)  _alarmdays;
    jObject["timeAnnouncing"]    = (bool)     _f_timeAnnouncement;
    jObject["mute"]              = (bool)     _f_mute;
    jObject["brightness"]        = (uint8_t)  _brightness;
    jObject["sleeptime"]         = (uint16_t) _sleeptime;
    jObject["lastconnectedhost"] = (String)   _lastconnectedhost;
    jObject["station"]           = (uint16_t) _cur_station;
    jObject["sumstations"]       = (uint16_t) _sum_stations;
    jObject["toneha"]            = (int16_t)  _toneha; // BassFreq 0...15        VS1053
    jObject["tonehf"]            = (int16_t)  _tonehf; // TrebleGain 0...14      VS1053
    jObject["tonela"]            = (int16_t)  _tonela; // BassGain 0...15        VS1053
    jObject["tonelf"]            = (int16_t)  _tonelf; // BassFreq 0...13        VS1053
    jObject["toneLP"]            = (int16_t)  _toneLP; // -40 ... +6 (dB)        audioI2S
    jObject["toneBP"]            = (int16_t)  _toneBP; // -40 ... +6 (dB)        audioI2S
    jObject["toneHP"]            = (int16_t)  _toneHP; // -40 ... +6 (dB)        audioI2S
    jObject["Timezone_Name"]     = (String)   _TZName;
    jObject["Timezone_String"]   = (String)   _TZString;
    jObject["timeFormat"]        = (uint8_t)  _timeFormat;

    String jO = JSON.stringify(jObject);
    if(_settingsHash != simpleHash(jO.c_str())) {
        File file = SD_MMC.open("/settings.json", "w", false);
        if(!file) {
            log_e("file \"settings.json\" not found");
            return;
        }
        file.print(jO);
        _settingsHash = simpleHash(jO.c_str());
    }
}
// clang-format on

/*****************************************************************************************************************************************************
 *                                                    F I L E   E X P L O R E R                                                                      *
 *****************************************************************************************************************************************************/
// Sends a list of the content of a directory as JSON file
String SD_dirContent(String path) {
    File    root, file;
    JSONVar jObject, jArr;
    int32_t     i = 0;
    if(path == "") path = "/";
    root = SD_MMC.open(path.c_str());

    if(!root.isDirectory()) {
        SerialPrintfln("FileExplorer:" ANSI_ESC_RED "%s is not a directory", path.c_str());
        return "";
    }
    while(true) {
        file = root.openNextFile();
        if(!file) break;
        if(startsWith(file.name(), "/.")) continue; // ignore hidden folders
        jArr["name"] = (String)file.name();
        jArr["dir"] = (boolean)file.isDirectory();
        jObject[i] = jArr;
        i++;
    }
    file.close();
    root.close();
    if(i) {
        String jO = JSON.stringify(jObject);
        // log_i("%s", jO.c_str());
        return jO;
    }
    return "";
}

// Sends a list of the content of a directory as JSON file
String DLNA_dirContent(String path) {
    // File root, file;
    // JSONVar jObject, jArr;
    // int32_t i = 0;
    // if(path =="") path = "/";
    // root = SD_MMC.open(path.c_str());

    // if (!root.isDirectory()) {
    //     SerialPrintfln("FileExplorer:" ANSI_ESC_RED "%s is not a directory", path.c_str());
    //     return "";
    // }
    // while (true) {
    //     file = root.openNextFile();
    //     if(!file) break;
    //     if (startsWith(file.name() , "/.")) continue;  // ignore hidden folders
    //     jArr["name"] = (String)file.name();
    //     jArr["dir"]  = (boolean)file.isDirectory();
    //     jObject[i]   = jArr;
    //     i++;
    // }
    // file.close();
    // root.close();
    // if(i){
    //     String jO = JSON.stringify(jObject);
    //     // log_i("%s", jO.c_str());
    //     return jO;
    // }
    return "";
}

/*****************************************************************************************************************************************************
 *                                                    T F T   B R I G H T N E S S                                                                    *
 *****************************************************************************************************************************************************/
void setTFTbrightness(uint8_t duty) {       // duty 0...100 (min...max)
    if(TFT_BL == -1) return;
    ledcSetup(0, 1200, 8);                  // 1200 Hz PWM and 8 bit resolution
    ledcAttachPin(TFT_BL, 0);               // Configure variable led, TFT_BL pin to channel 1
    uint8_t d = round((double)duty * 2.55); // #186
    ledcWrite(0, d);
}

inline uint8_t downBrightness() {
    if(_brightness > 5) {
        _brightness -= 5;
        setTFTbrightness(_brightness);
        showBrightnessBar();
        log_i("br %i", _brightness);
    }
    return _brightness;
}

inline uint8_t upBrightness() {
    if(_brightness < 100) {
        _brightness += 5;
        setTFTbrightness(_brightness);
        showBrightnessBar();
        log_i("br %i", _brightness);
    }
    return _brightness;
}
inline uint8_t getBrightness() { return _brightness; }

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
    while(str[p1]) {
        if((str[p1] == '%') && ((a = str[p1 + 1]) && (b = str[p1 + 2])) && (isxdigit(a) && isxdigit(b))) {
            if(a >= 'a') a -= 'a' - 'A';
            if(a >= 'A') a -= ('A' - 10);
            else
                a -= '0';
            if(b >= 'a') b -= 'a' - 'A';
            if(b >= 'A') b -= ('A' - 10);
            else
                b -= '0';
            str[p2++] = 16 * a + b;
            p1 += 3;
        }
        else if(str[p1] == '+') {
            str[p2++] = ' ';
            p1++;
        }
        else { str[p2++] = str[p1++]; }
    }
    str[p2++] = '\0';
}

/*****************************************************************************************************************************************************
 *                                                               T I M E R                                                                           *
 *****************************************************************************************************************************************************/

// clang-format off
void timer100ms(){
    static volatile uint8_t ms100 = 0;
    _f_100ms = true;
    ms100 ++;
    if(!(ms100 % 10))   _f_1sec  = true;
    if(!(ms100 % 100))  _f_10sec = true;
    if(!(ms100 % 600)) {_f_1min  = true; ms100 = 0;}

}
// clang-format on

/*****************************************************************************************************************************************************
 *                                                               D I S P L A Y                                                                       *
 *****************************************************************************************************************************************************/

// clang-format off
inline void clearHeader()             {tft.fillRect(_winHeader.x,    _winHeader.y,    _winHeader.w,    _winHeader.h,   TFT_BLACK);}
inline void clearLogo()               {tft.fillRect(_winLogo.x,      _winLogo.y,      _winLogo.w,      _winLogo.h,     TFT_BLACK);}
inline void clearStationName()        {tft.fillRect(_winName.x,      _winName.y,      _winName.w,      _winName.h,     TFT_BLACK);}
inline void clearLogoAndStationname() {tft.fillRect(_winFName.x,     _winFName.y,     _winFName.w,     _winFName.h,    TFT_BLACK);}
inline void clearTitle()              {tft.fillRect(_winTitle.x,     _winTitle.y,     _winTitle.w,     _winTitle.h,    TFT_BLACK);} // incl. VUmeter
inline void clearStreamTitle()        {tft.fillRect(_winSTitle.x,    _winSTitle.y,    _winSTitle.w,    _winSTitle.h,   TFT_BLACK);} // without VUmeter
inline void clearWithOutHeaderFooter(){tft.fillRect(_winWoHF.x,      _winWoHF.y,      _winWoHF.w,      _winWoHF.h,     TFT_BLACK);}
inline void clearFooter()             {tft.fillRect(_winFooter.x,    _winFooter.y,    _winFooter.w,    _winFooter.h,   TFT_BLACK);}
inline void clearTime()               {tft.fillRect(_winTime.x,      _winTime.y,      _winTime.w,      _winTime.h,     TFT_BLACK);}
inline void clearItem()               {tft.fillRect(_winItem.x,      _winItem.y,      _winItem.w,      _winTime.h,     TFT_BLACK);}
inline void clearVolume()             {tft.fillRect(_winVolume.x,    _winVolume.y,    _winVolume.w,    _winVolume.h,   TFT_BLACK);}
inline void clearIPaddr()             {tft.fillRect(_winIPaddr.x,    _winIPaddr.y,    _winIPaddr.w,    _winIPaddr.h,   TFT_BLACK);}
inline void clearBitRate()            {tft.fillRect(_winBitRate.x,   _winBitRate.y,   _winBitRate.w,   _winBitRate.h,  TFT_BLACK);}
inline void clearStaNr()              {tft.fillRect(_winStaNr.x,     _winStaNr.y,     _winStaNr.w,     _winStaNr.h,    TFT_BLACK);}
inline void clearSleep()              {tft.fillRect(_winSleep.x,     _winSleep.y,     _winSleep.w,     _winSleep.h,    TFT_BLACK);}
inline void clearVolBar()             {tft.fillRect(_winVolBar.x,    _winVolBar.y,    _winVolBar.w,    _winVolBar.h,   TFT_BLACK);}
inline void clearDigits()             {tft.fillRect(_winDigits.x,    _winDigits.y,    _winDigits.w,    _winDigits.h,   TFT_BLACK);}
inline void clearAlarmDaysBar()       {tft.fillRect( 0,              _winAlarmDays.y, _dispWidth,      _winAlarmDays.h,TFT_BLACK);}
inline void clearButtonBar()          {tft.fillRect( 0,              _winButton.y,    _dispWidth,      _winButton.h,   TFT_BLACK);}
inline void clearAll()                {tft.fillScreen(TFT_BLACK);}                      // y   0...239
// clang-format on

inline uint16_t txtlen(String str) {
    uint16_t len = 0;
    for(int32_t i = 0; i < str.length(); i++)
        if(str[i] <= 0xC2) len++;
    return len;
}

void showHeadlineVolume() {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_DEEPSKYBLUE);
    clearVolume();

    int32_t vol = 0;
    if(_f_mute || _f_muteDecrement || _f_muteIncrement) { vol = _mute_volume; }
    else { vol = _cur_volume; }

    sprintf(_chbuf, "Vol %02d", vol);
    tft.setCursor(_winVolume.x + 6, _winVolume.y + 2);
    tft.print(_chbuf);
    xSemaphoreGive(mutex_display);
}
void showHeadlineTime(bool complete) {
    static char oldtime[8];                                 // hhmmss
    char        newtime[8] = {255, 255, 255, 255, 255, 255, 255, 255};
    uint8_t     pos_s[8] = {0, 9, 17, 21, 30, 38, 41, 50};  // display 320x240
    uint8_t     pos_m[8] = {0, 13, 26, 32, 45, 58, 64, 77}; // display 480x320
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    if(!_f_rtc) {
        xSemaphoreGive(mutex_display);
        clearTime();
        return;
    } // has rtc the correct time? no -> return
    memcpy(newtime, rtc.gettime_s(), 8);
    if(complete == true) {
        clearTime();
        for(uint8_t i = 0; i < 8; i++) { oldtime[i] = 255; }
    }
    for(uint8_t i = 0; i < 8; i++) {
        if(oldtime[i] != newtime[i]) {
            char ch[2] = {0, 0};
            ch[0] = newtime[i];
            if(TFT_CONTROLLER < 2) { // 320x240
                tft.fillRect(_winTime.x + pos_s[i], _winTime.y, 9, _winTime.h, TFT_BLACK);
                tft.setCursor(_winTime.x + pos_s[i], _winTime.y + 2);
            }
            else { // 480x320
                tft.fillRect(_winTime.x + pos_m[i], _winTime.y, 13, _winTime.h, TFT_BLACK);
                tft.setCursor(_winTime.x + pos_m[i], _winTime.y + 2);
            }
            tft.print(ch);
            oldtime[i] = newtime[i];
        }
    }

    xSemaphoreGive(mutex_display);
}
void showHeadlineItem(uint8_t idx) { // radio, clock, audioplayer...
    if(_f_sleeping) return;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    clearItem();
    tft.setCursor(_winItem.x + 6, _winItem.y + 2);
    tft.print(_hl_item[idx]);
    xSemaphoreGive(mutex_display);
}
void showFooterIPaddr() {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    char myIP[30] = "myIP:";
    strcpy(myIP + 5, _myIP);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    clearIPaddr();
    tft.setCursor(_winIPaddr.x + 6, _winIPaddr.y + 2);
    tft.print(myIP);
    xSemaphoreGive(mutex_display);
}
void showFooterStaNr() {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    uint8_t offset = 0;
    if(TFT_CONTROLLER < 2) offset = 25;
    else
        offset = 33;
    clearStaNr();
    drawImage("/common/STA.bmp", _winStaNr.x, _winStaNr.y);
    tft.setFont(_fonts[1]);
    tft.setCursor(_winStaNr.x + offset, _winStaNr.y + 2);
    tft.setTextColor(TFT_LAVENDER);
    tft.printf("%03d", _cur_station);
    xSemaphoreGive(mutex_display);
}
void showFooterRSSI(boolean show) {
    static int32_t old_rssi = -1;
    int32_t        new_rssi = -1;
    int32_t        rssi = WiFi.RSSI(); // Received Signal Strength Indicator
    if(rssi < -1) new_rssi = 4;
    if(rssi < -50) new_rssi = 3;
    if(rssi < -65) new_rssi = 2;
    if(rssi < -75) new_rssi = 1;
    if(rssi < -85) new_rssi = 0;

    if(new_rssi != old_rssi) {
        old_rssi = new_rssi; // no need to draw a rssi icon if rssiRange has not changed
        if(ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO) {
            static int32_t tmp_rssi = 0;
            if((abs(rssi - tmp_rssi) > 3)) { SerialPrintfln("WiFI_info:   RSSI is " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " dB", rssi); }
            tmp_rssi = rssi;
        }
        show = true;
    }
    if(show && !_timeCounter.timer) {
        switch(new_rssi) {
        case 4: {
            drawImage("/common/RSSI4.bmp", _winRSSID.x, _winRSSID.y);
            break;
        }
        case 3: {
            drawImage("/common/RSSI3.bmp", _winRSSID.x, _winRSSID.y);
            break;
        }
        case 2: {
            drawImage("/common/RSSI2.bmp", _winRSSID.x, _winRSSID.y);
            break;
        }
        case 1: {
            drawImage("/common/RSSI1.bmp", _winRSSID.x, _winRSSID.y);
            break;
        }
        case 0: {
            drawImage("/common/RSSI0.bmp", _winRSSID.x, _winRSSID.y);
            break;
        }
        }
    }
}
void showFooterBitRate(uint16_t br) {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    clearBitRate();
    char sbr[10];
    itoa(br, sbr, 10);
    if(br < 1000) { strcat(sbr, "K"); }
    else {
        sbr[2] = sbr[1];
        sbr[1] = '.';
        sbr[3] = 'M';
        sbr[4] = '\0';
    }
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_LAVENDER);
    uint8_t space = 6;
    if(strlen(sbr) < 4) space += 5;
    tft.setCursor(_winBitRate.x + space, _winBitRate.y + 2);
    tft.print(sbr);
    xSemaphoreGive(mutex_display);
}

void updateSleepTime(boolean noDecrement) { // decrement and show new value in footer
    if(_f_sleeping) return;
    boolean sleep = false;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    clearSleep();
    drawImage("/common/Hourglass_blue.bmp", _winSleep.x, _winSleep.y);
    uint8_t offset = 0;
    if(TFT_CONTROLLER < 2) offset = 28;
    else
        offset = 33;
    if(_sleeptime == 1) sleep = true;
    if(_sleeptime > 0 && !noDecrement) _sleeptime--;

    char Slt[15];
    sprintf(Slt, "%d:%02d", _sleeptime / 60, _sleeptime % 60);
    tft.setFont(_fonts[1]);
    if(!_sleeptime) {
        drawImage("/common/Hourglass_blue.bmp", _winSleep.x, _winSleep.y);
        tft.setTextColor(TFT_DEEPSKYBLUE);
    }
    else {
        drawImage("/common/Hourglass_red.bmp", _winSleep.x, _winSleep.y);
        tft.setTextColor(TFT_RED);
    }
    tft.setCursor(_winSleep.x + offset, _winSleep.y + 2);
    tft.print(Slt);

    xSemaphoreGive(mutex_display);
    if(sleep) { // fall asleep
        fall_asleep();
        _sleeptime = 0;
    }
}
void showVolumeBar() {
    uint16_t val = tft.width() * getvolume() / 21;
    clearVolBar();
    tft.fillRect(_winVolBar.x, _winVolBar.y + 1, val, _winVolBar.h - 2, TFT_RED);
    tft.fillRect(val + 1, _winVolBar.y + 1, tft.width() - val + 1, _winVolBar.h - 2, TFT_GREEN);
    _f_volBarVisible = true;
}

void updateVUmeter() {
    if(_state != RADIO) return;
    if(_f_state_isChanging) return;
    if(_f_sleeping) return;
    if(_f_irNumberSeen) return;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    uint8_t width = 0, height = 0, xOffs = 0, yOffs = 0, xStart = 0, yStart = 0;
#if TFT_CONTROLLER < 2 // 320 x 240px
    width = 9;
    height = 7;
    xOffs = 11;
    yOffs = 8;
    xStart = 2;
    yStart = 90;
#else // 480 x 320px
    width = 12;
    height = 8;
    xOffs = 16;
    yOffs = 10;
    xStart = 2;
    yStart = 115;
#endif

    // c99 has no inner functions, lambdas are only allowed from c11, please don't use ancient compiler
    auto drawRect = [&](uint8_t pos, uint8_t ch, bool br) { // lambda, inner function
        uint16_t color = 0, xPos = _winVUmeter.x + xStart + ch * xOffs, yPos = _winVUmeter.y + yStart - pos * yOffs;
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
        tft.fillRect(xPos, yPos, width, height, color);
    };

    uint16_t vum = audioGetVUlevel();

    uint8_t left = map_l(vum >> 8, 0, 127, 0, 11);
    uint8_t right = map_l(vum & 0x00FF, 0, 127, 0, 11);

    if(left > _VUleftCh) {
        for(int32_t i = _VUleftCh; i < left; i++) { drawRect(i, 1, 1); }
    }
    if(left < _VUleftCh) {
        for(int32_t i = left; i < _VUleftCh; i++) { drawRect(i, 1, 0); }
    }
    _VUleftCh = left;

    if(right > _VUrightCh) {
        for(int32_t i = _VUrightCh; i < right; i++) { drawRect(i, 0, 1); }
    }
    if(right < _VUrightCh) {
        for(int32_t i = right; i < _VUrightCh; i++) { drawRect(i, 0, 0); }
    }
    _VUrightCh = right;
    xSemaphoreGive(mutex_display);
}

void showBrightnessBar() {
    uint16_t val = tft.width() * getBrightness() / 100;
    clearVolBar();
    tft.fillRect(_winVolBar.x, _winVolBar.y + 1, val, _winVolBar.h - 2, TFT_RED);
    tft.fillRect(val + 1, _winVolBar.y + 1, tft.width() - val + 1, _winVolBar.h - 2, TFT_GREEN);
    _f_volBarVisible = true;
}
void showFooter() { // stationnumber, sleeptime, IPaddress
    showFooterStaNr();
    updateSleepTime(true);
    showFooterIPaddr();
    showFooterRSSI(true);
    showFooterBitRate(_icyBitRate);
}
void display_info(const char* str, int32_t xPos, int32_t yPos, uint16_t color, uint16_t margin_l, uint16_t margin_r, uint16_t winWidth, uint16_t winHeight) {
    tft.fillRect(xPos, yPos, winWidth, winHeight, TFT_BLACK); // Clear the space for new info
    tft.setTextColor(color);                                  // Set the requested color
    tft.setCursor(xPos + margin_l, yPos);                     // Prepare to show the info
    // SerialPrintfln("cursor x=%d, y=%d, winHeight=%d", xPos+indent, yPos, winHeight);
    uint16_t ch_written = tft.writeText((const uint8_t*)str, winWidth - margin_r, winHeight);
    if(ch_written < strlenUTF8(str)) {
        // If this message appears, there is not enough space on the display to write the entire text,
        // a part of the text has been cut off
        SerialPrintfln("txt overflow, winHeight=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", strlen=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE
                       ", written=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", str=" ANSI_ESC_CYAN "%s",
                       winHeight, strlenUTF8(str), ch_written, str);
    }
}
void showStreamTitle(const char* streamtitle) {
    if(_f_sleeping) return;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    String ST = streamtitle;

    ST.trim();               // remove all leading or trailing whitespaces
    ST.replace(" | ", "\n"); // some stations use pipe as \n or
    ST.replace("| ", "\n");  // or
    ST.replace("|", "\n");
    switch(strlenUTF8(ST.c_str())) {
    case 0 ... 30: tft.setFont(_fonts[5]); break;
    case 31 ... 43: tft.setFont(_fonts[4]); break;
    case 44 ... 65: tft.setFont(_fonts[3]); break;
    case 66 ... 130: tft.setFont(_fonts[2]); break;
    case 131 ... 200: tft.setFont(_fonts[1]); break;
    default: tft.setFont(_fonts[0]); break;
    }
    display_info(ST.c_str(), _winSTitle.x, _winSTitle.y, TFT_CORNSILK, 5, 5, _winSTitle.w, _winSTitle.h);
    xSemaphoreGive(mutex_display);
}
void showVUmeter() {
    if(_f_sleeping) return;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    drawImage("/common/level_bar.jpg", _winVUmeter.x, _winVUmeter.y);
    _VUrightCh = 0;
    _VUleftCh = 0;
    xSemaphoreGive(mutex_display);
}

void showLogoAndStationName() {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    clearLogoAndStationname();
    xSemaphoreGive(mutex_display);
    String SN_utf8 = "";
    if(_cur_station) { SN_utf8 = _stationName_nvs; }
    else { SN_utf8 = _stationName_air; }
    SN_utf8.trim();

    showStationName(SN_utf8);

    showStationLogo(SN_utf8);
}

void showStationName(String sn) {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    switch(strlenUTF8(sn.c_str())) {
    case 0 ... 20: tft.setFont(_fonts[5]); break;
    case 21 ... 32: tft.setFont(_fonts[4]); break;
    case 33 ... 45: tft.setFont(_fonts[3]); break;
    case 46 ... 60: tft.setFont(_fonts[2]); break;
    case 61 ... 90: tft.setFont(_fonts[1]); break;
    default: tft.setFont(_fonts[0]); break;
    }
    display_info(sn.c_str(), _winName.x, _winName.y, TFT_CYAN, 10, 0, _winName.w, _winName.h);
    xSemaphoreGive(mutex_display);
}

void showStationLogo(String ln) {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    String logo = "/logo/" + (String)ln.c_str() + ".jpg";
    if(drawImage(logo.c_str(), 0, _winName.y + 2) == false) {
        drawImage("/common/unknown.jpg", 0, _winName.y + 2); // if no draw unknown
        _f_logoUnknown = true;
    }
    xSemaphoreGive(mutex_display);
}

void showFileLogo(uint8_t state) {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    String logo;
    if(state == RADIO) {
        if(endsWith(_stationURL, "m3u8")) logo = "/common/" + (String) "M3U8" + ".jpg";
        else
            logo = "/common/" + (String)codecname[_cur_Codec] + ".jpg";
    }
    else if(state == DLNA) { logo = "/common/DLNA.jpg"; }
    else { // _state PLAYER or PLAYERico
        logo = "/common/" + (String)codecname[_cur_Codec] + ".jpg";
    }
    if(drawImage(logo.c_str(), 0, _winName.y + 2) == true) { webSrv.send("stationLogo=" + logo); }
    else {
        (drawImage("/common/unknown.jpg", 0, _winName.y + 2));
        webSrv.send("stationLogo=");
    }
    xSemaphoreGive(mutex_display);
}

void showFileName(const char* fname) {
    clearLogo();
    if(!fname) return;
    switch(strlenUTF8(fname)) {
    case 0 ... 15: tft.setFont(_fonts[5]); break;
    case 16 ... 30: tft.setFont(_fonts[4]); break;
    case 31 ... 70: tft.setFont(_fonts[3]); break;
    case 71 ... 100: tft.setFont(_fonts[2]); break;
    case 101 ... 150: tft.setFont(_fonts[1]); break;
    default: tft.setFont(_fonts[0]); break;
    }
    display_info(fname, _winName.x, _winName.y, TFT_CYAN, 0, 0, _winName.w, _winName.h);
}

void showStationsList(uint16_t staListNr){
    clearWithOutHeaderFooter();
    if(_sum_stations < 11) staListNr = 0;
    else if(staListNr + 9 > _max_stations) staListNr = _max_stations - 9;
    showHeadlineItem(STATIONSLIST);
    tft.setFont(_fonts[1]);
    uint8_t lineHight = _winWoHF.h / 10;
    for(uint8_t pos = 0; pos < 10; pos++){
        if(pos + staListNr + 1 > _sum_stations) break;
        tft.setCursor(10, _winFooter.h + (pos) * lineHight) ;
        sprintf(_chbuf, "station_%03d", pos + staListNr  + 1);
        String content = stations.getString(_chbuf, " #not_found");
        content.replace('#','\0');
        sprintf(_chbuf, ANSI_ESC_YELLOW"%03d " ANSI_ESC_WHITE "%s\n",pos + staListNr  + 1, content.c_str());
        tft.writeText((uint8_t*)_chbuf, -1, -1, true);
    }
    _timeCounter.timer = 10;
    _timeCounter.factor = 1.0;
}


void display_time(boolean showall) { // show current time on the TFT Display
    static String  t, oldt = "";
    static boolean k = false;
    uint8_t        i = 0;
    if(showall == true) { oldt = "        ";}
    if((_state == CLOCK) || (_state == CLOCKico)) {
        if(_timeFormat == 12) t = rtc.gettime_xs_12h(); // 01:27 PM
        else                  t = rtc.gettime_xs();     // 13:27

        for(i = 0; i < 5; i++) {
            if(t[i] == ':') {
                if(k == false) {
                    k = true;
                    t[i] = 'd';
                }
                else {
                    t[i] = 'e';
                    k = false;
                }
            }
            if(_timeFormat == 12){
                if(t[i] != oldt[i]) {
                    sprintf(_chbuf, "/digits/foldedNumbers/%cwhite.jpg", t[i]);
                    drawImage(_chbuf, _alarmtimeXPosFN[i], _winDigits.y);
                    if(i == 4){
                        if(t.substring(6,8) == "PM"){
                            drawImage("/digits/foldedNumbers/pmwhite.jpg", _alarmtimeXPosFN[5], _winDigits.y);
                        }
                        else{
                            drawImage("/digits/foldedNumbers/amwhite.jpg", _alarmtimeXPosFN[5], _winDigits.y);
                        }
                    }
                }
            }
            else{
                if(t[i] != oldt[i]) { // 24h representation
                    sprintf(_chbuf, "/digits/sevenSegment/%cgreen.jpg", t[i]);
                    drawImage(_chbuf, _alarmtimeXPos7S[i], _winDigits.y);
                }
            }
        }
        oldt = t;
    }
}

void display_alarmDays(uint8_t ad, boolean showall) { // Sun ad=0, Mon ad=1, Tue ad=2 ....
    uint8_t i = 0;
    String  str = "";

    if(!showall) _alarmdays ^= (1 << ad); // toggle bit

    for(i = 0; i < 7; i++) {
        str = "/day/" + String(i);
        if(_alarmdays & (1 << i)) str += "_rt_en.bmp"; // l << i instead pow(2,i)
        else
            str += "_gr_en.bmp";
        drawImage(str.c_str(), _alarmdaysXPos[i], _winAlarmDays.y);
    }
}

void display_alarmtime(int8_t xy, int8_t ud, boolean showall) {
    static int8_t pos, h, m;
    int8_t        updatePos = -1, oldPos = -1;

    if(showall) {
        h = _alarmtime / 60;
        m = _alarmtime % 60;
    }

    if(ud == 1) {
        if(pos == 0)
            if(((h / 10) == 1 && (h % 10) < 4) || ((h / 10) == 0)) {
                h += 10;
                updatePos = 0;
            }
        if(pos == 1)
            if(((h / 10) == 2 && (h % 10) < 3) || ((h / 10) < 2 && (h % 10) < 9)) {
                h++;
                updatePos = 1;
            }
        if(pos == 2)
            if((m / 10) < 5) {
                m += 10;
                updatePos = 2;
            }
        if(pos == 3)
            if((m % 10) < 9) {
                m++;
                updatePos = 3;
            }
        _alarmtime = h * 60 + m;
    }
    if(ud == -1) {
        if(pos == 0)
            if((h / 10) > 0) {
                h -= 10;
                updatePos = 0;
            }
        if(pos == 1)
            if((h % 10) > 0) {
                h--;
                updatePos = 1;
            }
        if(pos == 2)
            if((m / 10) > 0) {
                m -= 10;
                updatePos = 2;
            }
        if(pos == 3)
            if((m % 10) > 0) {
                m--;
                updatePos = 3;
            }
        _alarmtime = h * 60 + m;
    }

    if(xy == 1) {
        oldPos = pos++;
        if(pos == 4) pos = 0;
        updatePos = pos; // pos 0...3 only
    }
    if(xy == -1) {
        oldPos = pos--;
        if(pos == -1) pos = 3;
        updatePos = pos;
    }

    char hhmm[15];
    sprintf(hhmm, "%d%d%d%d", h / 10, h % 10, m / 10, m % 10);

    if(showall) {
        drawImage("/digits/sevenSegment/dred.jpg", _alarmtimeXPos7S[2], _winDigits.y); // colon
    }

    for(uint8_t i = 0; i < 4; i++) {
        uint8_t p = i;
        if(i > 1) p++; // skip colon
        strcpy(_path, "/digits/sevenSegment/");
        strncat(_path, (const char*)hhmm + i, 1);
        if(showall) {
            if(i == pos) strcat(_path, "orange.jpg"); // show orange number
            else
                strcat(_path, "red.jpg");          // show red numbers

            drawImage(_path, _alarmtimeXPos7S[p], _winDigits.y);
        }

        else {
            if(i == updatePos) {
                strcat(_path, "orange.jpg");
                drawImage(_path, _alarmtimeXPos7S[p], _winDigits.y);
            }
            if(i == oldPos) {
                strcat(_path, "red.jpg");
                drawImage(_path, _alarmtimeXPos7S[p], _winDigits.y);
            }
        }
    }
}

void display_sleeptime(int8_t ud) { // set sleeptimer
    if(ud == 1) {
        switch(_sleeptime) {
        case 0 ... 14: _sleeptime = (_sleeptime / 5) * 5 + 5; break;
        case 15 ... 59: _sleeptime = (_sleeptime / 15) * 15 + 15; break;
        case 60 ... 359: _sleeptime = (_sleeptime / 60) * 60 + 60; break;
        default: _sleeptime = 360; break; // max 6 hours
        }
    }
    if(ud == -1) {
        switch(_sleeptime) {
        case 1 ... 15: _sleeptime = ((_sleeptime - 1) / 5) * 5; break;
        case 16 ... 60: _sleeptime = ((_sleeptime - 1) / 15) * 15; break;
        case 61 ... 360: _sleeptime = ((_sleeptime - 1) / 60) * 60; break;
        default: _sleeptime = 0; break; // min
        }
    }
    char tmp[10];
    sprintf(tmp, "%d%02d", _sleeptime / 60, _sleeptime % 60);
    char path[128] = "/digits_small/";

    for(uint8_t i = 0; i < 4; i++) {
        strcpy(path, "/digits_small/");
        if(i == 3) {
            if(!_sleeptime) strcat(path, "dsgn.jpg");
            else
                strcat(path, "dsrt.jpg");
        }
        else {
            strncat(path, (tmp + i), 1);
            if(!_sleeptime) strcat(path, "sgn.jpg");
            else
                strcat(path, "srt.jpg");
        }
        drawImage(path, _sleeptimeXPos[i], 48);
    }
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth, uint16_t maxHeigth) {
    const char* scImg = scaleImage(path);
    if(!SD_MMC.exists(scImg)) {
        if(indexOf(scImg, "/.", 0) > 0) return false; // empty filename
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "file \"%s\" not found", scImg);
        return false;
    }
    if(endsWith(scImg, "bmp")) {
        // log_i("drawImage %s, x=%i, y=%i, mayWidth=%i, maxHeight=%i", scImg, posX, posY, maxWidth, maxHeigth);
        return tft.drawBmpFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth);
    }
    if(endsWith(scImg, "jpg")) { return tft.drawJpgFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth); }
    SerialPrintfln(ANSI_ESC_RED "the file \"%s\" contains neither a bmp nor a jpj graphic", scImg);
    return false; // neither jpg nor bmp
}
/*****************************************************************************************************************************************************
 *                                                   H A N D L E  A U D I O F I L E                                                                  *
 *****************************************************************************************************************************************************/
bool SD_listDir(const char* path){
    File file;
    vector_clear_and_shrink(_SD_content);
    if(audioFile) audioFile.close();
    if(!SD_MMC.exists(path)){
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
        return false;
    }
    audioFile = SD_MMC.open(path);
    if(!audioFile.isDirectory()) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
        audioFile.close();
        return false;
    }
    while(true) {    // get content
        file = audioFile.openNextFile();
        if(!file) break;
        if(file.isDirectory()){
            sprintf(_chbuf, "%s", file.name());
            _SD_content.push_back(strdup((const char*)_chbuf));
        }
        else {
            if(endsWith(file.name(), ".mp3") || endsWith(file.name(), ".aac") || endsWith(file.name(), ".m4a") || endsWith(file.name(), ".wav") ||
               endsWith(file.name(), ".flac") || endsWith(file.name(), ".m3u") || endsWith(file.name(), ".opus") || endsWith(file.name(), ".ogg")) {
                    sprintf(_chbuf, "%s" ANSI_ESC_YELLOW " %d", file.name(), file.size());
                    _SD_content.push_back(strdup((const char*) _chbuf));
            }
        }
    }
    audioFile.close();
    return true;
}

bool setAudioFolder(const char* audioDir) {
    SD_listDir(audioDir);
    if(audioFile) audioFile.close(); // same as rewind()
    if(!SD_MMC.exists(audioDir)) {
        SerialPrintfln(ANSI_ESC_RED "%s not exist", audioDir);
        return false;
    }
    audioFile = SD_MMC.open(audioDir);
    if(!audioFile.isDirectory()) {
        SerialPrintfln(ANSI_ESC_RED "%s is not a directory", audioDir);
        return false;
    }
    return true;
}

void showAudioFilesList(uint16_t fileListNr){
    clearWithOutHeaderFooter();
    if(_SD_content.size() < 10) fileListNr = 0;
    showHeadlineItem(AUDIOFILESLIST);
    tft.setCursor(10, _winFooter.h) ;
    tft.setTextColor(TFT_ORANGE);
    tft.writeText((uint8_t*)_curAudioFolder.c_str(), -1,-1, true);
    tft.setTextColor(TFT_WHITE);
    tft.setFont(_fonts[1]);
    uint8_t lineHight = _winWoHF.h / 10;
    for(uint8_t pos = 1; pos < 10; pos++){
        if(pos == 1 && fileListNr > 0){
            tft.setCursor(0, _winFooter.h + (pos) * lineHight);
            tft.setTextColor(TFT_AQUAMARINE);
            tft.writeText((uint8_t*)"˄");
        }
        if(pos == 9 && fileListNr + 9 < _SD_content.size()){
            tft.setCursor(0, _winFooter.h + (pos) * lineHight);
            tft.setTextColor(TFT_AQUAMARINE);
            tft.writeText((uint8_t*)"˅");
        }
        if(fileListNr + pos > _SD_content.size()) break;
        tft.setCursor(20, _winFooter.h + (pos) * lineHight);
        if(indexOf(_SD_content[pos + fileListNr - 1 ], "\033[", 0) == -1) tft.setTextColor(TFT_GRAY); // is folder
        else tft.setTextColor(TFT_WHITE);                                                     // is file
        tft.writeText((uint8_t*)_SD_content[pos + fileListNr - 1 ], -1, -1, true);
    }
    _timeCounter.timer = 10;
    _timeCounter.factor = 1.0;
}

File getNextAudioFile() {
    File file;
    while(true) {
        if(!audioFile) {
            SerialPrintfln(ANSI_ESC_BLUE "no audiofiles found");
            break;
        }
        file = audioFile.openNextFile();
        if(!file) {
            audioFile.close();
            SerialPrintfln(ANSI_ESC_BLUE "no more files found");
            break;
        }
        else {
            if(endsWith(file.name(), ".mp3") || endsWith(file.name(), ".aac") || endsWith(file.name(), ".m4a") || endsWith(file.name(), ".wav") ||
               endsWith(file.name(), ".flac") || endsWith(file.name(), ".m3u") || endsWith(file.name(), ".opus") || endsWith(file.name(), ".ogg")) {
                break;
            }
        }
    }
    return file;
}
void processPlaylist(boolean first) {
    static bool f_has_EXTINF = false;
    while(playlistFile.available() > 0) {
        size_t bytesRead = playlistFile.readBytesUntil('\n', _chbuf, 512);
        _chbuf[bytesRead] = '\0';
        if(bytesRead < 5) continue; // line is # or space or nothing, smallest filename "1.mp3" < 5
        if(startsWith(_chbuf, "#")) SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "%s", _chbuf);
        trim(_chbuf);
        if(first) {
            _playlistTime = millis();
            if(startsWith(_chbuf, "#EXTM3U")) {
                first = false;
                _f_playlistEnabled = true;
                continue;
            }
            else {
                SerialPrintfln("Playlist:    " ANSI_ESC_RED "%s is not a valid, #EXTM3U not found", playlistFile.name());
                playlistFile.close();
                return;
            }
        }
        if(startsWith(_chbuf, "#EXTINF")) {
            int8_t idx1 = indexOf(_chbuf, ":", 0) + 1;
            int8_t idx2 = indexOf(_chbuf, ",", 0);
            SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "Title: %s", _chbuf + idx2 + 1);
            showFileName(_chbuf + idx2 + 1);
            f_has_EXTINF = true;
            int8_t len = idx2 - idx1;
            if(len > 0 && len < 6) { // song playtime
                char tmp[7];
                memcpy(tmp, _chbuf + idx1, len);
                tmp[len] = '\0';
                SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "playtime: %is", atoi(tmp));
            }
            continue;
        }
        _f_playlistNextFile = false;
        if(!startsWith(_chbuf, "#")) {
            if(startsWith(_chbuf, "http")) {
                SerialPrintflnCut("Playlist:    ", ANSI_ESC_YELLOW, _chbuf);
                showVolumeBar();
                if(!f_has_EXTINF) clearLogoAndStationname();
                f_has_EXTINF = false;
                webSrv.send((String) "SD_playFile=" + _chbuf);
                changeState(PLAYERico);
                _cur_Codec = 0;
                _f_isWebConnected = audioConnecttohost(_chbuf);
                _f_isFSConnected = false;
            }
            else {
                const char* path = playlistFile.path();
                int32_t         idx = lastIndexOf(path, '/');
                int32_t         len = strlen(_chbuf);
                for(int32_t i = len; i > -1; i--) { _chbuf[idx + i + 1] = _chbuf[i]; }
                strncpy(_chbuf, path, idx + 1);
                log_w("pls path %s, %i, %i", _chbuf, idx, len);
                urldecode(_chbuf);
                SerialPrintfln("Playlist:    " ANSI_ESC_YELLOW "%s", _chbuf);
                webSrv.send((String) "SD_playFile=" + _chbuf);
                if(!f_has_EXTINF) SD_playFile(_chbuf);
                else {
                    f_has_EXTINF = false;
                    SD_playFile(_chbuf, 0, false);
                }
            }
            return;
        }
    }
    SerialPrintfln("end of playlist");
    webSrv.send("SD_playFile=end of playlist");
    playlistFile.close();
    _f_playlistEnabled = false;
    clearLogoAndStationname();
    showFileName(audioFile.name());
    changeState(PLAYER);
}
/*****************************************************************************************************************************************************
 *                                         C O N N E C T   TO   W I F I     /     A C C E S S P O I N T                                              *
 *****************************************************************************************************************************************************/
bool connectToWiFi() {
    String s_ssid = "", s_password = "", s_info = "";
    wifiMulti.addAP(_SSID, _PW);                        // SSID and PW in code
    if(pref.isKey("ap_ssid") && pref.isKey("ap_pw")) {  // exists?
        String ap_ssid = pref.getString("ap_ssid", ""); // credentials from accesspoint
        String ap_pw = pref.getString("ap_pw", "");
        if(ap_ssid.length() > 0 && ap_pw.length() > 0) wifiMulti.addAP(ap_ssid.c_str(), ap_pw.c_str());
    }
    WiFi.setHostname("MiniWebRadio");
    if(psramFound()) WiFi.useStaticBuffers(true);
    File file = SD_MMC.open("/networks.csv"); // try credentials given in "/networks.txt"
    if(file) {                                // try to read from SD_MMC
        String str = "";
        while(file.available()) {
            str = file.readStringUntil('\n');   // read the line
            if(str[0] == '*') continue;         // ignore this, goto next line
            if(str[0] == '\n') continue;        // empty line
            if(str[0] == ' ') continue;         // space as first char
            if(str.indexOf('\t') < 0) continue; // no tab
            str += "\t";
            uint p = 0, q = 0;
            s_ssid = "", s_password = "", s_info = "";
            for(int32_t i = 0; i < str.length(); i++) {
                if(str[i] == '\t') {
                    if(p == 0) s_ssid = str.substring(q, i);
                    if(p == 1) s_password = str.substring(q, i);
                    if(p == 2) s_info = str.substring(q, i);
                    p++;
                    i++;
                    q = i;
                }
            }
            // log_i("s_ssid=%s  s_password=%s  s_info=%s", s_ssid.c_str(), s_password.c_str(), s_info.c_str());
            if(s_ssid == "") continue;
            if(s_password == "") continue;
            wifiMulti.addAP(s_ssid.c_str(), s_password.c_str());
        }
        file.close();
    }
    wifiMulti.run();

    if(WiFi.isConnected()) {
        SerialPrintfln("WiFI_info:   Connecting WiFi...");
        WiFi.setSleep(false);
        if(!MDNS.begin("MiniWebRadio")) {
            SerialPrintfln("WiFI_info:   " ANSI_ESC_YELLOW "Error starting mDNS");
        }
        else{
            MDNS.addService("esp32", "tcp", 80);
            SerialPrintfln("WiFI_info:   mDNS name: " ANSI_ESC_CYAN "MiniWebRadio");
        }
        return true;
    }
    else {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "WiFi credentials are not correct");
        return false; // can't connect to any network
    }
}

void openAccessPoint() { // if credentials are not correct open AP at 192.168.4.1
    clearAll();
    tft.setFont(_fonts[4]);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(25, 80);
    setTFTbrightness(80);
    _f_accessPoint = true;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.softAP("MiniWebRadio");
    IPAddress myIP = WiFi.softAPIP();
    String    AccesspointIP = myIP.toString();
    tft.printf("WiFi credentials are not correct\nAccesspoint IP: %s", AccesspointIP.c_str());
    SerialPrintfln("Accesspoint: " ANSI_ESC_RED "IP: %s", AccesspointIP.c_str());
    int32_t n = WiFi.scanNetworks();
    if(n == 0) {
        SerialPrintfln("setup: ....  no WiFi networks found");
        while(true) { ; }
    }
    else {
        SerialPrintfln("setup: ....  %d WiFi networks found", n);
        for(int32_t i = 0; i < n; ++i) {
            SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            _scannedNetworks += WiFi.SSID(i) + '\n';
        }
    }
    webSrv.begin(80, 81); // HTTP port, WebSocket port
    return;
}

/*****************************************************************************************************************************************************
 *                                                                     A U D I O                                                                     *
 *****************************************************************************************************************************************************/
void connecttohost(const char* host) {
    int32_t   idx1, idx2;
    char* url = nullptr;
    char* user = nullptr;
    char* pwd = nullptr;

    clearBitRate();
    _cur_Codec = 0;
    //    if(_state == RADIO) clearStreamTitle();
    _icyBitRate = 0;
    _avrBitRate = 0;

    idx1 = indexOf(host, "|", 0);
    // log_i("idx1 = %i", idx1);
    if(idx1 == -1) { // no pipe found
        _f_isWebConnected = audioConnecttohost(host);
        _f_isFSConnected = false;
        return;
    }
    else { // pipe found
        idx2 = indexOf(host, "|", idx1 + 1);
        // log_i("idx2 = %i", idx2);
        if(idx2 == -1) { // second pipe not found
            _f_isWebConnected = audioConnecttohost(host);
            _f_isFSConnected = false;
            return;
        }
        else {                         // extract url, user, pwd
            url = strndup(host, idx1); // extract url
            user = strndup(host + idx1 + 1, idx2 - idx1 - 1);
            pwd = strdup(host + idx2 + 1);
            SerialPrintfln("new host: .  %s user %s, pwd %s", url, user, pwd) _f_isWebConnected = audioConnecttohost(url, user, pwd);
            _f_isFSConnected = false;
            if(url) free(url);
            if(user) free(user);
            if(pwd) free(pwd);
        }
    }
}
void connecttoFS(const char* filename, uint32_t resumeFilePos) {
    clearBitRate();
    _icyBitRate = 0;
    _avrBitRate = 0;
    _cur_Codec = 0;
    _f_isFSConnected = audioConnecttoFS(filename, resumeFilePos);
    _f_isWebConnected = false;
    //    log_w("Filesize %d", audioGetFileSize());
    //    log_w("FilePos %d", audioGetFilePosition());
}
void stopSong() {
    audioStopSong();
    _f_isFSConnected = false;
    _f_isWebConnected = false;
    _f_playlistEnabled = false;
    _f_pauseResume = false;
}

/*****************************************************************************************************************************************************
 *                                                                    S E T U P                                                                      *
 *****************************************************************************************************************************************************/
void setup() {
    Serial.begin(115200);
    Serial.print("\n\n");
    const char* chipModel = ESP.getChipModel();
    uint8_t     avMajor = ESP_ARDUINO_VERSION_MAJOR;
    uint8_t     avMinor = ESP_ARDUINO_VERSION_MINOR;
    uint8_t     avPatch = ESP_ARDUINO_VERSION_PATCH;
    Serial.printf("ESP32 Chip: %s\n", chipModel);
    Serial.printf("Arduino Version: %d.%d.%d\n", avMajor, avMinor, avPatch);
    uint8_t idfMajor = ESP_IDF_VERSION_MAJOR;
    uint8_t idfMinor = ESP_IDF_VERSION_MINOR;
    uint8_t idfPatch = ESP_IDF_VERSION_PATCH;
    Serial.printf("ESP-IDF Version: %d.%d.%d\n", idfMajor, idfMinor, idfPatch);
    Version = Version.substring(0, 30);
    Serial.printf("MiniWebRadio %s\n", Version.c_str());
    Serial.printf("ARDUINO_LOOP_STACK_SIZE %d words (32 bit)\n", CONFIG_ARDUINO_LOOP_STACK_SIZE);
    Serial.printf("FLASH size %d bytes, speed %d MHz\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed() / 1000000);
    Serial.printf("CPU speed %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("SDMMC speed %d MHz\n", SDMMC_FREQUENCY / 1000000);
    Serial.printf("TFT speed %d MHz\n", TFT_FREQUENCY / 1000000);
    if(psramInit()) { Serial.printf("PSRAM total size: %d bytes\n", esp_spiram_get_size()); }
    else { Serial.printf(ANSI_ESC_RED "PSRAM not found! MiniWebRadio does not work without PSRAM!" ANSI_ESC_WHITE); return;}
    _f_PSRAMfound = true;
    Serial.print("\n\n");
    mutex_rtc = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    SerialPrintfln("   ");
    SerialPrintfln(ANSI_ESC_YELLOW "       ***************************    ");
    SerialPrintfln(ANSI_ESC_YELLOW "       *     MiniWebRadio V2     *    ");
    SerialPrintfln(ANSI_ESC_YELLOW "       ***************************    ");
    SerialPrintfln("   ");
    if(startsWith(chipModel, "ESP32-D")) { ; } // ESP32-D    ...  okay
    if(startsWith(chipModel, "ESP32-P")) { ; } // ESP32-PICO ...  okay
    if(startsWith(chipModel, "ESP32-S2")) {
        SerialPrintfln(ANSI_ESC_RED "MiniWebRadio does not work with ESP32-S2");
        return;
    }
    if(startsWith(chipModel, "ESP32-C3")) {
        SerialPrintfln(ANSI_ESC_RED "MiniWebRadio does not work with ESP32-C3");
        return;
    }
    if(startsWith(chipModel, "ESP32-S3")) { ; } // ESP32-S3  ...  okay
    _f_ESPfound = true;

    SerialPrintfln("setup: ....  Arduino is pinned to core " ANSI_ESC_CYAN "%d", xPortGetCoreID());
    if(TFT_CONTROLLER < 2) strcpy(_prefix, "/s");
    else
        strcpy(_prefix, "/m");
    stations.begin("Stations", false); // instance of preferences for stations (name, url ...)
    pref.begin("Pref", false);         // instance of preferences from AccessPoint (SSID, PW ...)

#if CONFIG_IDF_TARGET_ESP32
    tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK); // Init TFT interface ESP32
#else
    tft.begin(TFT_CS, TFT_DC, FSPI, TFT_MOSI, TFT_MISO, TFT_SCK); // Init TFT interface ESP32S3
#endif

    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tp.setVersion(TP_VERSION);
    tp.setRotation(TP_ROTATION);
    tp.TP_Send(0xD0);
    tp.TP_Send(0x90); // Remove any blockage

    SerialPrintfln("setup: ....  Init SD card");
    pinMode(IR_PIN, INPUT_PULLUP); // if ir_pin is read only, have a external resistor (~10...40KOhm)
    pinMode(SD_MMC_D0, INPUT_PULLUP);
#ifdef CONFIG_IDF_TARGET_ESP32S3
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
    int32_t sdmmc_frequency = SDMMC_FREQUENCY / 1000; // MHz -> KHz, default is 40MHz
    if(!SD_MMC.begin("/sdcard", true, false, sdmmc_frequency)) {
        clearAll();
        tft.setFont(_fonts[5]);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(50, 100);
        tft.print("SD Card Mount Failed");
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed");
        return;
    }
    float cardSize = ((float)SD_MMC.cardSize()) / (1024 * 1024);
    float freeSize = ((float)SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024);
    SerialPrintfln(ANSI_ESC_WHITE "setup: ....  SD card found, %.1f MB by %.1f MB free", freeSize, cardSize);
    _f_SD_MMCfound = true;
    defaultsettings(); // first init
    if(getBrightness() >= 5) setTFTbrightness(getBrightness());
    else
        setTFTbrightness(5);
    if(TFT_CONTROLLER > 6) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid");
    drawImage("/common/MiniWebRadioV2.jpg", 0, 0); // Welcomescreen
    SerialPrintfln("setup: ....  seek for stations.csv");
    File file = SD_MMC.open("/stations.csv");
    if(!file) {
        clearAll();
        tft.setFont(_fonts[5]);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(50, 100);
        tft.print("stations.csv not found");
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "stations.csv not found");
        while(1) {}; // endless loop, MiniWebRadio does not work without stations.csv
    }
    file.close();
    SerialPrintfln("setup: ....  stations.csv found");
    updateSettings();
    SerialPrintfln("setup: ....  seek for WiFi networks");
    if(!connectToWiFi()) {
        openAccessPoint();
        return;
    }
    strcpy(_myIP, WiFi.localIP().toString().c_str());
    SerialPrintfln("setup: ....  connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", IP address is " ANSI_ESC_CYAN "%s", WiFi.SSID().c_str(), _myIP);

    ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); // username, password for ftp.

    setRTC(_TZString.c_str());

#if DECODER > 1 // DAC controlled by I2C
    if(!dac.begin(I2C_DATA, I2C_CLK)) { SerialPrintfln(ANSI_ESC_RED "The DAC was not be initialized"); }
#endif

    audioInit();

    audioConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);

    SerialPrintfln("setup: ....  Number of saved stations: " ANSI_ESC_CYAN "%d", _sum_stations);
    SerialPrintfln("setup: ....  current station number: " ANSI_ESC_CYAN "%d", _cur_station);
    SerialPrintfln("setup: ....  current volume: " ANSI_ESC_CYAN "%d", _cur_volume);
    SerialPrintfln("setup: ....  last connected host: " ANSI_ESC_CYAN "%s", _lastconnectedhost.c_str());
    SerialPrintfln("setup: ....  connection timeout: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT);
    SerialPrintfln("setup: ....  connection timeout SSL: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT_SSL);

    _state = RADIO;

    ir.begin();           // Init InfraredDecoder

    webSrv.begin(80, 81); // HTTP port, WebSocket port

    if(HP_DETECT != -1) {
        pinMode(HP_DETECT, INPUT);
        attachInterrupt(HP_DETECT, headphoneDetect, CHANGE);
    }
    if(AMP_ENABLED != -1) { // enable onboard amplifier
        pinMode(AMP_ENABLED, OUTPUT);
        digitalWrite(AMP_ENABLED, HIGH);
    }

    tft.fillScreen(TFT_BLACK); // Clear screen
    if(_f_mute) {
        SerialPrintfln("setup: ....  volume is muted: (from " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET ")", _cur_volume);
        audioSetVolume(0);
        showHeadlineVolume();
    }
    else {
        setVolume(_cur_volume);
        _mute_volume = _cur_volume;
    }

    showHeadlineItem(RADIO);
    if(_cur_station > 0) setStation(_cur_station);
    else { setStationViaURL(_lastconnectedhost.c_str()); }

    if(DECODER == 0) setTone(); // HW Decoder
    else
        setI2STone();           // SW Decoder
    showFooter();
    soap.seekServer();
    _numServers = soap.getServerCount();
    showVUmeter();
    ticker100ms.attach(0.1, timer100ms);
}
/*****************************************************************************************************************************************************
 *                                                                   C O M M O N                                                                     *
 *****************************************************************************************************************************************************/
const char* byte_to_binary(int8_t x) {
    static char b[9];
    b[0] = '\0';

    int32_t z;
    for(z = 128; z > 0; z >>= 1) { strcat(b, ((x & z) == z) ? "1" : "0"); }
    return b;
}
uint32_t simpleHash(const char* str) {
    if(str == NULL) return 0;
    uint32_t hash = 0;
    for(int32_t i = 0; i < strlen(str); i++) {
        if(str[i] < 32) continue; // ignore control sign
        hash += (str[i] - 31) * i * 32;
    }
    return hash;
}
int32_t str2int(const char* str) {
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
void trim(char* s) {
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
bool startsWith(const char* base, const char* searchString) {
    char c;
    while((c = *searchString++) != '\0')
        if(c != *base++) return false;
    return true;
}
bool endsWith(const char* base, const char* searchString) {
    int32_t         slen = strlen(searchString) - 1;
    const char* p = base + strlen(base) - 1;
    while(p > base && isspace(*p)) p--; // rtrim
    p -= slen;
    if(p < base) return false;
    return (strncmp(p, searchString, slen) == 0);
}
int32_t indexOf(const char* haystack, const char* needle, int32_t startIndex) {
    const char* p = haystack;
    for(; startIndex > 0; startIndex--)
        if(*p++ == '\0') return -1;
    char* pos = strstr(p, needle);
    if(pos == nullptr) return -1;
    return pos - haystack;
}
int32_t lastIndexOf(const char* haystack, const char needle) {
    const char* p = strrchr(haystack, needle);
    return (p ? p - haystack : -1);
}
boolean strCompare(char* str1, char* str2) { return strCompare((const char*)str1, str2); }
boolean strCompare(const char* str1, char* str2) { // returns true if str1 == str2
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

char* ps_strdup(const char* str){
    char* ps_str = (char*) ps_malloc(strlen(str) + 1);
    strcpy(ps_str, str);
    return ps_str;
}

int16_t strlenUTF8(const char* str) { // returns only printable glyphs, all ASCII and UTF-8 until 0xDFBD
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
int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    const int32_t run = in_max - in_min;
    if(run == 0) {
        log_e("map(): Invalid input range, min == max");
        return -1; // AVR returns -1, SAM returns 0
    }
    const int32_t rise = out_max - out_min;
    const int32_t delta = x - in_min;
    return (delta * rise) / run + out_min;
}
void SerialPrintflnCut(const char* item, const char* color, const char* str) {
    if(strlen(str) > 75) {
        String f = str;
        SerialPrintfln("%s%s%s ... %s", item, color, f.substring(0, 50).c_str(), f.substring(f.length() - 20, f.length()).c_str());
    }
    else { SerialPrintfln("%s%s%s", item, color, str); }
}

const char* scaleImage(const char* path) {
    if((!endsWith(path, "bmp")) && (!endsWith(path, "jpg"))) { // not a image
        return path;
    }
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    char* pch = strstr(path + 1, "/");
    if(pch) {
        strncpy(pathBuff, path, (pch - path));
        if(TFT_CONTROLLER < 2) strcat(pathBuff, _prefix); // small pic,  320x240px
        else
            strcat(pathBuff, _prefix);                    // medium pic, 480x320px
        strcat(pathBuff, pch);
    }
    else {
        strcpy(pathBuff, "/common");
        if(TFT_CONTROLLER < 2) strcat(pathBuff, _prefix); // small pic,  320x240px
        else
            strcat(pathBuff, _prefix);                    // medium pic, 480x320px
        strcat(pathBuff, path);
    }
    return pathBuff;
}

inline uint8_t getvolume() { return _cur_volume; }
void           setVolume(uint8_t vol) {
    if(_f_mute == false) {
        audioSetVolume(vol);
        showHeadlineVolume();
    }
    else { showHeadlineVolume(); }
    _cur_volume = vol;
    SerialPrintfln("action: ...  current volume is " ANSI_ESC_CYAN "%d", _cur_volume);

#if DECODER > 1 // ES8388, AC101 ...
    if(HP_DETECT == -1) {
        dac.SetVolumeSpeaker(_cur_volume * 3);
        dac.SetVolumeHeadphone(_cur_volume * 3);
    }
    else {
        if(digitalRead(HP_DETECT) == HIGH) {
            // SerialPrintfln("HP_Detect = High, volume %i", vol);
            dac.SetVolumeSpeaker(_cur_volume * 3);
            dac.SetVolumeHeadphone(0);
        }
        else {
            // SerialPrintfln("HP_Detect = Low, volume %i", vol);
            dac.SetVolumeSpeaker(1);
            dac.SetVolumeHeadphone(_cur_volume * 3);
        }
    }
#endif
}

uint8_t downvolume() {
    if(_cur_volume == 0) return _cur_volume;
    _cur_volume--;
    setVolume(_cur_volume);
    return _cur_volume;
}
uint8_t upvolume() {
    if(_cur_volume == _max_volume) return _cur_volume;
    _cur_volume++;
    setVolume(_cur_volume);
    return _cur_volume;
}
inline void mute() {
    if(_f_mute == false) {
        _f_muteDecrement = true;
        _f_muteIncrement = false;
    }
    else {
        _f_muteIncrement = true;
        _f_muteDecrement = false;
        if(AMP_ENABLED != -1) digitalWrite(AMP_ENABLED, HIGH);
    }
}

void setStation(uint16_t sta) {
    // SerialPrintfln("sta %d, _cur_station %d", sta, _cur_station );
    if(sta == 0) return;
    if(sta > _sum_stations) sta = _cur_station;
    sprintf(_chbuf, "station_%03d", sta);
    String content = stations.getString(_chbuf, " #not_found");
    // SerialPrintfln("content %s", content.c_str());
    _stationName_nvs = content.substring(0, content.indexOf("#"));           // get stationname
    content = content.substring(content.indexOf("#") + 1, content.length()); // get URL
    content.trim();
    free(_stationURL);
    _stationURL = strdup(content.c_str());
    _homepage = "";
    if(_state == RADIO) clearStreamTitle();

    SerialPrintfln("action: ...  switch to station " ANSI_ESC_CYAN "%d", sta);

    if(_f_isWebConnected && sta == _cur_station && _state == RADIO) { // Station is already selected
        _f_newStreamTitle = true;
    }
    else {
        _streamTitle[0] = '\0';
        _icyDescription[0] = '\0';
        _f_newStreamTitle = true;
        _f_newIcyDescription = true;
        connecttohost(_stationURL);
    }
    _cur_station = sta;
    StationsItems();
    if(_state == RADIO || _state == RADIOico) showLogoAndStationName();
    showFooterStaNr();
}
void nextStation() {
    if(_cur_station >= _sum_stations) setStation(1);
    else
        setStation(_cur_station + 1);
}
void prevStation() {
    if(_cur_station > 1) setStation(_cur_station - 1);
    else
        setStation(_sum_stations);
}

void StationsItems() {
    if(_cur_station > 0) {
        webSrv.send("stationLogo=/logo/" + _stationName_nvs + ".jpg");
        webSrv.send("stationNr=" + String(_cur_station));
        webSrv.send("stationURL=" + String(_stationURL));
    }
    else {
        webSrv.send("stationLogo=/logo/" + _stationName_air + ".jpg");
        webSrv.send("stationNr=" + String(_cur_station));
        //    webSrv.send("stationURL=" + _lastconnectedhost);
    }
}

void setStationViaURL(const char* url) {
    _stationName_air = "";
    _stationName_nvs = "";
    _cur_station = 0;
    free(_stationURL);
    _stationURL = strdup(url);
    connecttohost(url);
    StationsItems();
    if(_state == RADIO || _state == RADIOico){
        clearStreamTitle();
        showLogoAndStationName();
    }
    showFooterStaNr(); // set to '000'
}

void changeBtn_pressed(uint8_t btnNr) { drawImage(_pressBtn[btnNr], btnNr * _winButton.w, _winButton.y); }
void changeBtn_released(uint8_t btnNr) {
    if(_state == RADIOico || _state == PLAYERico) {
        if(_f_mute) _releaseBtn[0] = "/btn/Button_Mute_Red.jpg";
        else
            _releaseBtn[0] = "/btn/Button_Mute_Green.jpg";
    }
    if(_state == CLOCKico) {
        if(_f_mute) _releaseBtn[2] = "/btn/Button_Mute_Red.jpg";
        else
            _releaseBtn[2] = "/btn/Button_Mute_Green.jpg";
    }
    drawImage(_releaseBtn[btnNr], btnNr * _winButton.w, _winButton.y);
}

void savefile(const char* fileName, uint32_t contentLength) { // save the uploadfile on SD_MMC
    char fn[256];

    if(!_f_SD_Upload && endsWith(fileName, "jpg")) {
        strcpy(fn, "/logo");
        strcat(fn, _prefix);
        if(!startsWith(fileName, "/")) strcat(fn, "/");
        strcat(fn, fileName);
        if(webSrv.uploadB64image(SD_MMC, fn, contentLength)) {
            SerialPrintfln("save image " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.sendStatus(200);
        }
        else
            webSrv.sendStatus(400);
    }
    else {
        _f_SD_Upload = false;
        if(!startsWith(fileName, "/")) {
            strcpy(fn, "/");
            strcat(fn, fileName);
        }
        else { strcpy(fn, fileName); }
        if(webSrv.uploadfile(SD_MMC, fn, contentLength)) {
            SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.sendStatus(200);
        }
        else {
            SerialPrintfln("save file:   " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD failed", fn);
            webSrv.sendStatus(400);
        }
        if(strcmp(fn, "/stations.csv") == 0) saveStationsToNVS();
    }
}

String setTone() { // vs1053
    uint8_t ha = _toneha;
    uint8_t hf = _tonehf;
    uint8_t la = _tonela;
    uint8_t lf = _tonelf;
    audioSetTone(ha, hf, la, lf);
    sprintf(_chbuf, "toneha=%i\ntonehf=%i\ntonela=%i\ntonelf=%i\n", ha, hf, la, lf);
    String tone = String(_chbuf);
    return tone;
}

String setI2STone() {
    int8_t LP = _toneLP;
    int8_t BP = _toneBP;
    int8_t HP = _toneHP;
    audioSetTone(LP, BP, HP);
    sprintf(_chbuf, "LowPass=%i\nBandPass=%i\nHighPass=%i\n", LP, BP, HP);
    String tone = String(_chbuf);
    return tone;
}

void SD_playFile(const char* path, uint32_t resumeFilePos, bool showFN) {
    if(endsWith(path, "ogg")) resumeFilePos = 0; // resume only mp3, m4a, flac and wav
    if(endsWith(path, "m3u")) {
        playlistFile.close();                    // as a precaution
        if(SD_MMC.exists(path)) {
            playlistFile = SD_MMC.open(path);
            _f_playlistEnabled = false;
            processPlaylist(true);
        }
        return;
    }
    showVolumeBar();
    int32_t idx = lastIndexOf(path, '/');
    if(idx < 0) return;
    if(showFN) showFileName(path + idx + 1);
    changeState(PLAYERico);
    connecttoFS((const char*)path, resumeFilePos);
    if(_f_isFSConnected) {
        free(_lastconnectedfile);
        _lastconnectedfile = strdup(path);
        _resumeFilePos = 0;
    }
}

bool SD_rename(const char* src, const char* dest) {
    bool success = false;
    if(SD_MMC.exists(src)) {
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
    if(SD_MMC.exists(itemPath)) {
        File dirTest = SD_MMC.open(itemPath, "r");
        bool isDir = dirTest.isDirectory();
        dirTest.close();
        if(isDir) success = SD_MMC.rmdir(itemPath);
        else
            success = SD_MMC.remove(itemPath);
    }
    return success;
}

void IRAM_ATTR headphoneDetect() { // called via interrupt
    _f_hpChanged = true;
}

void fall_asleep() {
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    _f_sleeping = true;
    _f_playlistEnabled = false;
    _f_isFSConnected = false;
    _f_isWebConnected = false;
    playlistFile.close();
    audioStopSong();
    if(_state != CLOCK) {
        clearAll();
        setTFTbrightness(0);
    }
    xSemaphoreGive(mutex_display);
    WiFi.disconnect(true);  // Disconnect from the network
    vTaskDelay(300);
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
    setCpuFrequencyMhz(80); // 10MHz and 40MHz seem to stop TPanel after minute or so.
    if(_state == CLOCKico) _state = CLOCK;
    if(_state == RADIOico) _state = RADIO;
    if(_state == PLAYERico) _state = PLAYER;
    SerialPrintfln("falling asleep");
}

void wake_up() {
    if(_f_sleeping == true || _f_eof_alarm) { // awake
        _f_sleeping = false;
        SerialPrintfln("awake");
        _f_mute = true;
        mute();
        clearAll();
        setTFTbrightness(_brightness);
        setCpuFrequencyMhz(240);
        WiFi.disconnect(false); // Reconnect the network
        wifiMulti.run();
        SerialPrintfln("START WIFI");
        while(WiFi.status() != WL_CONNECTED) {
            delay(100);
            SerialPrintfln(".");
        }
        connecttohost(_lastconnectedhost.c_str());
        showFooter();
        showHeadlineTime();
        showHeadlineVolume();
        if(_state == CLOCK) {
            showHeadlineItem(CLOCK);
            display_time(true);
        }
        else {
            changeState(RADIO);
            showVUmeter();
            showLogoAndStationName();
            showHeadlineItem(RADIO);
        }
    }
}

void setRTC(const char* TZString) {
    rtc.stop();
    _f_rtc = rtc.begin(_TZString.c_str());
    if(!_f_rtc) {
        SerialPrintfln(ANSI_ESC_RED "connection to NTP failed, trying again");
        ESP.restart();
    }
}

void vector_clear_and_shrink(vector<char*>&vec){
    uint size = vec.size();
    for (int32_t i = 0; i < size; i++) {
        if(vec[i]){
            free(vec[i]);
            vec[i] = NULL;
        }
    }
    vec.clear();
    vec.shrink_to_fit();
}

void dlna_items_vector_clear_ans_shrink(){
    vector_clear_and_shrink(_dlna_items.name);
    vector_clear_and_shrink(_dlna_items.id);
    vector_clear_and_shrink(_dlna_items.uri);
    _dlna_items.size.clear();
    _dlna_items.isDir.clear();
    _dlna_items.isAudio.clear();
}

/*****************************************************************************************************************************************************
 *                                                            M E N U E / B U T T O N S                                                              *
 *****************************************************************************************************************************************************/
// clang-format off
void changeState(int32_t state){
    if(state == _state) return;  //nothing todo
    _f_state_isChanging = true;
    _f_volBarVisible = false;
    if(_timeCounter.timer){
        _timeCounter.timer = 0;
        showFooterRSSI(true);
    }
    switch(state) {
        case RADIO:{
            showHeadlineItem(RADIO);
            if(_state == RADIOico || _state == RADIOmenue){
                _f_newStreamTitle = true;
            }
            else if(_state == PLAYER  || _state == PLAYERico || _state == DLNA){
                setStation(_cur_station);
                clearTitle();
                showLogoAndStationName();
                _f_newStreamTitle = true;
            }
            else if(_state == CLOCKico){
                showLogoAndStationName();
                _timeCounter.timer = 0;
                _f_newStreamTitle = true;
            }
            else if(_state == SLEEP){
                clearLogoAndStationname();
                clearTitle();
                connecttohost(_lastconnectedhost.c_str());
                showLogoAndStationName();
                showFooter();
                showHeadlineVolume();
            }
            else if(_state == BRIGHTNESS){
                showLogoAndStationName();
                _f_newStreamTitle = true;
                clearTitle();
            }
            else if(_state == STATIONSLIST){
                clearWithOutHeaderFooter();
                showLogoAndStationName();
                showStreamTitle(_streamTitle);
            }
            else{
                showLogoAndStationName();
                _f_newStreamTitle = true;
            }
            showVUmeter();
            break;
        }
        case RADIOico:{
            showHeadlineItem(RADIOico);
            _pressBtn[0] = "/btn/Button_Mute_Yellow.jpg";        _releaseBtn[0] =  _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[1] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2] = "/btn/Button_Volume_Up_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Previous_Yellow.jpg";    _releaseBtn[3] = "/btn/Button_Previous_Green.jpg";
            _pressBtn[4] = "/btn/Button_Next_Yellow.jpg";        _releaseBtn[4] = "/btn/Button_Next_Green.jpg";
            _pressBtn[5] = "/btn/Button_List_Yellow.jpg";        _releaseBtn[5] = "/btn/Button_List_Green.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            clearTitle();
            showVolumeBar();
            for(int32_t i = 0; i < 8 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            _timeCounter.timer = 5;
            _timeCounter.factor = 2.0;
            break;
        }
        case RADIOmenue:{
            showHeadlineItem(RADIOmenue);
            _pressBtn[0] = "/btn/MP3_Yellow.jpg";                _releaseBtn[0] = "/btn/MP3_Green.jpg";
            _pressBtn[1] = "/btn/Button_DLNA_Yellow.jpg";        _releaseBtn[1] = "/btn/Button_DLNA_Green.jpg";
            _pressBtn[2] = "/btn/Clock_Yellow.jpg";              _releaseBtn[2] = "/btn/Clock_Green.jpg";
            _pressBtn[3] = "/btn/Button_Sleep_Yellow.jpg";       _releaseBtn[3] = "/btn/Button_Sleep_Green.jpg";
            if(TFT_BL != -1){
                _pressBtn[4]="/btn/Bulb_Yellow.jpg";             _releaseBtn[4]="/btn/Bulb_Green.jpg";
            }
            else{
                _pressBtn[4]="/btn/Black.jpg";                   _releaseBtn[4]="/btn/Black.jpg";
            }
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            for(int32_t i = 0; i < 8 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            clearVolBar();
            _timeCounter.timer = 5;
            _timeCounter.factor = 2.0;
            break;
        }
        case STATIONSLIST:{
            showStationsList(_staListNr);
            _timeCounter.timer = 10;
            _timeCounter.factor = 1.0;
            break;
        }
        case CLOCK:{
            if(_state == ALARM){
                updateSettings();
                const char day[7][6] = {"Sun, ", "Mon, ", "Tue, ", "Wed, ", "Thu, ", "Fri, ", "Sat, "};
                char wd[40] = "";
                for(uint8_t i = 0; i < 7; i++){
                    uint8_t j = 1 << i;
                    if(_alarmdays & j) strcat(wd, day[i]);
                }
                uint8_t l = strlen(wd);
                if(l > 1) wd[l - 2] = '\0'; // remove last space and comma
                else strcat(wd, "no day of the week activated");

                SerialPrintfln("Alarm set to " ANSI_ESC_CYAN "%02d:%02d" ANSI_ESC_WHITE " on " ANSI_ESC_CYAN
                               "%s -> %s", _alarmtime / 60, _alarmtime % 60, byte_to_binary(_alarmdays), wd);
                _f_semaphore = false;
                _f_alarm = false;
            }
            if(_state == CLOCKico){
                clearButtonBar();
            }
            else{ // != CLOCKico
                clearLogoAndStationname();
                clearTitle();
                showFooter();
                showHeadlineVolume();
                showHeadlineTime();
            }
            _state = CLOCK;
            showHeadlineItem(CLOCK);
            display_time(true);
            break;
        }
        case CLOCKico:{
            if(_state != CLOCK) clearDigits();
            _state = CLOCKico;
            showHeadlineItem(CLOCKico);
            display_time(true);
            _pressBtn[0] = "/btn/Bell_Yellow.jpg";               _releaseBtn[0] = "/btn/Bell_Green.jpg";
            _pressBtn[1] = "/btn/Radio_Yellow.jpg";              _releaseBtn[1] = "/btn/Radio_Green.jpg";
            _pressBtn[2] = "/btn/Button_Mute_Yellow.jpg";        _releaseBtn[2] = _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[3] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[3] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[4] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[4] = "/btn/Button_Volume_Up_Blue.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            for(int32_t i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            _timeCounter.timer = 5;
            _timeCounter.factor = 2.0;
            break;
        }
        case BRIGHTNESS:{
            showHeadlineItem(BRIGHTNESS);
            _pressBtn[0] = "/btn/Button_Left_Yellow.jpg";        _releaseBtn[0] = "/btn/Button_Left_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
            _pressBtn[4] = "/btn/Black.jpg";                     _releaseBtn[4] = "/btn/Black.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            drawImage("/common/Brightness.jpg", 0, _winName.y);
            showBrightnessBar();
            for(int32_t i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case PLAYER:{
            if(_state == RADIO || _state == DLNA || _state == AUDIOFILESLIST){
                clearWithOutHeaderFooter();
            }
            showHeadlineItem(PLAYER);
            _pressBtn[0] = "/btn/Button_First_Yellow.jpg";       _releaseBtn[0] = "/btn/Button_First_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
            _pressBtn[4] = "/btn/Black.jpg";                     _releaseBtn[4] = "/btn/Black.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Button_List_Yellow.jpg";        _releaseBtn[6] = "/btn/Button_List_Green.jpg";
            _pressBtn[7] = "/btn/Radio_Yellow.jpg";              _releaseBtn[7] = "/btn/Radio_Green.jpg";
            for(int32_t i = 0; i < 8 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case PLAYERico:{
            showHeadlineItem(PLAYERico);
            _pressBtn[0] = "/btn/Button_Mute_Yellow.jpg";        _releaseBtn[0] = _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[1] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2] = "/btn/Button_Volume_Up_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Pause_Yellow.jpg";       _releaseBtn[3] = "/btn/Button_Pause_Blue.jpg";
            _pressBtn[4] = "/btn/Button_Cancel_Yellow.jpg";      _releaseBtn[4] = "/btn/Button_Cancel_Red.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Radio_Yellow.jpg";              _releaseBtn[7] = "/btn/Radio_Green.jpg";
            for(int32_t i = 0; i < 8 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case AUDIOFILESLIST:{
            showAudioFilesList(_fileListNr);
            _timeCounter.timer = 10;
            _timeCounter.factor = 1.0;
            break;
        }
        case ALARM:{
            showHeadlineItem(ALARM);
            _pressBtn[0] = "/btn/Button_Left_Yellow.jpg";        _releaseBtn[0] = "/btn/Button_Left_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Up_Yellow.jpg";          _releaseBtn[2] = "/btn/Button_Up_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Down_Yellow.jpg";        _releaseBtn[3] = "/btn/Button_Down_Blue.jpg";
            _pressBtn[4] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[4] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            clearDigits();
            display_alarmtime(0, 0, true);
            display_alarmDays(0, true);
            for(int32_t i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w,  _winButton.y);}
            break;
        }
        case SLEEP:{
            showHeadlineItem(SLEEP);
            _pressBtn[0] = "/btn/Button_Up_Yellow.jpg";          _releaseBtn[0] = "/btn/Button_Up_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Down_Yellow.jpg";        _releaseBtn[1] = "/btn/Button_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
            _pressBtn[4] = "/btn/Button_Cancel_Yellow.jpg";      _releaseBtn[4] = "/btn/Button_Cancel_Blue.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Black.jpg";                     _releaseBtn[7] = "/btn/Black.jpg";
            clearLogoAndStationname();
            clearTitle();
            display_sleeptime();
            if(TFT_CONTROLLER < 2) drawImage("/common/Night_Gown.bmp", 198, 23);
            else                   drawImage("/common/Night_Gown.bmp", 280, 45);
            for(int32_t i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case DLNA:{
            showHeadlineItem(DLNA);
            _pressBtn[0] = "/btn/Button_Mute_Yellow.jpg";        _releaseBtn[0] =  _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[1] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2] = "/btn/Button_Volume_Up_Blue.jpg";

            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
        // todo   _pressBtn[3] = "/btn/Button_List_Yellow.jpg";        _releaseBtn[3] = "/btn/Button_List_Green.jpg";

            _pressBtn[4] = "/btn/Black.jpg";                     _releaseBtn[4] = "/btn/Black.jpg";
            _pressBtn[5] = "/btn/Black.jpg";                     _releaseBtn[5] = "/btn/Black.jpg";
            _pressBtn[6] = "/btn/Black.jpg";                     _releaseBtn[6] = "/btn/Black.jpg";
            _pressBtn[7] = "/btn/Radio_Yellow.jpg";              _releaseBtn[7] = "/btn/Radio_Green.jpg";
            clearLogoAndStationname();
            clearTitle();
            showFileLogo(state);
            showVolumeBar();
            for(int32_t i = 0; i < 8 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case DLNAITEMSLIST:{
            showDlnaItemsList(_dlna_items.level, 0);
            _timeCounter.timer = 10;
            _timeCounter.factor = 1.0;
            break;
        }
    }
    _state = state;
    _f_state_isChanging = false;
}
// clang-format on
/*****************************************************************************************************************************************************
 *                                                                D L N A                                                                            *
 *****************************************************************************************************************************************************/
int32_t DLNA_setCurrentServer(String serverName) {
    int32_t serverNum = -1;
    for(int32_t i = 0; i < _names.size(); i++) {
        if(_names[i] == serverName) serverNum = i;
    }
    _currentServer = serverNum;
    return serverNum;
}

void DLNA_showServer() {      // Show connection details of all discovered, usable media servers
    webSrv.send("clearDLNA"); // delete server list
    soap.listServer();        // -> event dlna_server()
}

void DLNA_browseServer(String objectId, uint8_t level) {
    // Here the user selects the DLNA server whose content he wants to see, level 0 is root
    if(level == 0) { _currentServer = objectId.toInt(); }
    soap.browseServer(_currentServer, objectId.c_str());
}

void DLNA_getFileItems(String uri) {
    String   _media_downloadIP = soap.getMediaDownloadIP();
    uint16_t _media_downloadPort = soap.getMediaDownloadPort();

    log_v("uri: %s", uri.c_str());
    log_v("downloadIP: %s", _media_downloadIP.c_str());
    log_v("downloadport: %d", _media_downloadPort);
    String URL = "http://" + _media_downloadIP + ":" + _media_downloadPort + "/" + uri;
    // log_i("URL=%s", URL.c_str());
    _cur_Codec = 0;
    _f_isWebConnected = audioConnecttohost(URL.c_str());
    _f_isFSConnected = false;
}
void DLNA_showContent(String objectId, uint8_t level) {
    audioStopSong();
    log_v("obkId=%s", objectId.c_str());
    if(level == 0) { DLNA_browseServer(objectId, level); }
    if(objectId.startsWith("D=")) {
        objectId = objectId.substring(2);
        DLNA_browseServer(objectId, level);
    }
    if(objectId.startsWith("F=")) {
        objectId = objectId.substring(2);
        DLNA_getFileItems(objectId);
    }
}

void showDlnaItemsList(uint8_t level, uint16_t itemNr){
    clearWithOutHeaderFooter();
    uint16_t itemsSize = 0;
    if(level == 0){                                  // show DLNA server names
        itemsSize = _dlna_items.serverFriendlyName.size();
        if(itemsSize < 10) itemNr = 0;
    }
    else{
        itemsSize = _dlna_items.name.size();
        if(itemsSize < 10) itemNr = 0; // show DLNA items
    }
    log_i("level %d, itemsSize %d", level, itemsSize);
    showHeadlineItem(DLNA);
    tft.setCursor(10, _winFooter.h) ;
    tft.setTextColor(TFT_ORANGE);
    if(level > 0)  tft.writeText((uint8_t*)_dlna_items.path[0], -1,-1, true);
    tft.setTextColor(TFT_WHITE);
    tft.setFont(_fonts[1]);
    uint8_t lineHight = _winWoHF.h / 10;
    for(uint8_t pos = 1; pos < 10; pos++){
        if(pos == 1 && itemNr > 0){
            tft.setCursor(0, _winFooter.h + (pos) * lineHight);
            tft.setTextColor(TFT_AQUAMARINE);
            tft.writeText((uint8_t*)"˄");
        }
        if(pos == 9 && itemNr + 9 < itemsSize){
            tft.setCursor(0, _winFooter.h + (pos) * lineHight);
            tft.setTextColor(TFT_AQUAMARINE);
            tft.writeText((uint8_t*)"˅");
        }
        if(itemNr + pos > itemsSize) break;
        tft.setCursor(20, _winFooter.h + (pos) * lineHight);
        if(level == 0){
            tft.setTextColor(TFT_LIME);
            tft.writeText((uint8_t*)_dlna_items.serverFriendlyName[(pos - 1) + itemNr], -1, -1, true);
        }
        else{
            //    if(indexOf(_SD_content[pos + itemNr - 1 ], "\033[", 0) == -1) tft.setTextColor(TFT_GRAY); // is folder
            //    else tft.setTextColor(TFT_WHITE);                                                     // is file
        }
    }
    _timeCounter.timer = 10;
    _timeCounter.factor = 1.0;
}

/*****************************************************************************************************************************************************
 *                                                                 L O O P                                                                           *
 *****************************************************************************************************************************************************/
void loop() {
    if(!_f_PSRAMfound) return;  // Guard:  PSRAM could not be initialized
    if(!_f_ESPfound) return;    // Guard:  wrong chip?
    if(!_f_SD_MMCfound) return; // Guard:  SD_MMC could not be initialisized
    webSrv.loop();
    ir.loop();
    tp.loop();
    ftpSrv.handleFTP();
    soap.loop();
    if(_f_muteDecrement) {
        if(_mute_volume > 0) {
            _mute_volume--;
            audioSetVolume(_mute_volume);
            showHeadlineVolume();
        }
        else {
            if(AMP_ENABLED != -1) digitalWrite(AMP_ENABLED, LOW);
            _f_muteDecrement = false;
            _f_mute = true;
            webSrv.send("mute=1");
            if(_state == RADIOico || _state == PLAYERico || _state == DLNA) { drawImage("/btn/Button_Mute_Red.jpg", 0, _winButton.y); }
            if(_state == CLOCKico) { drawImage("/btn/Button_Mute_Red.jpg", 2 * _winButton.w, _winButton.y); }
        }
    }

    if(_f_muteIncrement) {
        if(_mute_volume < _cur_volume) {
            _mute_volume++;
            audioSetVolume(_mute_volume);
            showHeadlineVolume();
        }
        else {
            _f_muteIncrement = false;
            _f_mute = false;
            webSrv.send("mute=0");
            if(_state == RADIOico || _state == PLAYERico) { drawImage("/btn/Button_Mute_Green.jpg", 0, _winButton.y); }
            if(_state == CLOCKico) { drawImage("/btn/Button_Mute_Green.jpg", 2 * _winButton.w, _winButton.y); }
        }
    }
    if(!_f_sleeping) {
        if(_f_newBitRate) {
            showFooterBitRate(_icyBitRate);
            _f_newBitRate = false;
        }

        if(_f_newLogoAndStation) {
            showLogoAndStationName();
            _f_newLogoAndStation = false;
        }

        if(_f_100ms) {
            _f_100ms = false;
            if(_state != ALARM) updateVUmeter();
        }
    }
    if(_f_1sec) {
        _f_1sec = false;
        if(!_f_sleeping) {
            showHeadlineTime(false);
            showFooterRSSI();
            if(_state == CLOCK || _state == CLOCKico) display_time();
        }

        if(_timeCounter.timer) {
            _timeCounter.timer--;
            if(_timeCounter.timer < 10){
                sprintf(_chbuf, "/common/tc%02d.bmp", uint8_t(_timeCounter.timer * _timeCounter.factor));
                drawImage(_chbuf, _winRSSID.x, _winRSSID.y);
            }
            if(!_timeCounter.timer) {
                showFooterRSSI(true);
                if(_state == RADIOico) changeState(RADIO);
                else if(_state == RADIOmenue)
                    changeState(RADIO);
                else if(_state == CLOCKico)
                    changeState(CLOCK);
                else if(_state == RADIO && _f_switchToClock) {
                    changeState(CLOCK);
                    _f_switchToClock = false;
                }
                else if(_state == STATIONSLIST){
                    changeState(RADIO);
                }
                else if(_state == AUDIOFILESLIST){
                    changeState(PLAYER);
                }
                else if(_state == DLNAITEMSLIST){
                    changeState(DLNA);
                }
                else { ; } // all other, do nothing
            }
        }
        if(_f_rtc == true) { // true -> rtc has the current time
            int8_t h = 0;
            String time_s;
            xSemaphoreTake(mutex_rtc, portMAX_DELAY);
            time_s = rtc.gettime_s();
            xSemaphoreGive(mutex_rtc);
            if(_f_eof && (_state == RADIO || _f_eof_alarm)) {
                _f_eof = false;
                if(_f_eof_alarm) {
                    audioSetVolume(_cur_volume);
                    wake_up();
                    _f_eof_alarm = false;
                }
                else { connecttohost(_lastconnectedhost.c_str()); }
            }
            if((_f_mute == false) && (!_f_sleeping)) {
                if(time_s.endsWith("59:53") && _state == RADIO) { // speech the time 7 sec before a new hour is arrived
                    String hour = time_s.substring(0, 2);         // extract the hour
                    h = hour.toInt();
                    h++;
                    if(h == 24) h = 0;
                    if(_f_timeAnnouncement) {
                        if(_timeFormat == 12) if(h > 12) h -=  12;
                        sprintf(_chbuf, "/voice_time/%d_00.mp3", h);
                        SerialPrintfln("Time: ...... play Audiofile %s", _chbuf) connecttoFS(_chbuf);
                    }
                    else { SerialPrintfln("Time: ...... Announcement at %d o'clock is silent", h); }
                }
            }

            if(_alarmtime == rtc.getMinuteOfTheDay() && _state != ALARM) { // is alarmtime?
                SerialPrintfln("is alarmtime");
                if((_alarmdays >> rtc.getweekday()) & 1) {                 // is alarmday? 0-Sun, 1-Mon, 2 Tue ....
                    if(!_f_semaphore) {
                        _f_alarm = true;
                        _f_semaphore = true;
                    } // set alarmflag
                }
            }
            else
                _f_semaphore = false;

            if(_f_alarm) {
                clearAll();
                showFileName("ALARM");
                drawImage("/common/Alarm.jpg", _winLogo.x, _winLogo.y);
                setTFTbrightness(_brightness);
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm");
                _f_alarm = false;
                connecttoFS("/ring/alarm_clock.mp3");
                audioSetVolume(21);
            }
            if(_f_hpChanged) {
                setVolume(_cur_volume);
                if(!digitalRead(HP_DETECT)) { SerialPrintfln("Headphone plugged in"); }
                else { SerialPrintfln("Headphone unplugged"); }
                _f_hpChanged = false;
            }
        }
        if(_commercial_dur > 0) {
            _commercial_dur--;
            if((_commercial_dur == 2) && (_state == RADIO)) clearStreamTitle(); // end of commercial? clear streamtitle
        }
        if(_f_newStreamTitle && !_timeCounter.timer) {
            _f_newStreamTitle = false;
            if(_state == RADIO) {
                if(strlen(_streamTitle)) showStreamTitle(_streamTitle);
                else if(strlen(_icyDescription)) {
                    showStreamTitle(_icyDescription);
                    _f_newIcyDescription = false;
                    webSrv.send((String) "icy_description=" + _icyDescription);
                }
                else
                    clearStreamTitle();
            }
            webSrv.send((String) "streamtitle=" + _streamTitle);
        }
        if(_f_newIcyDescription && !_timeCounter.timer) {
            if(_state == RADIO) {
                if(!strlen(_streamTitle)) showStreamTitle(_icyDescription);
            }
            webSrv.send((String) "icy_description=" + _icyDescription);
            _f_newIcyDescription = false;
        }

        if(_f_newCommercial && !_timeCounter.timer) {
            if(_state == RADIO) { showStreamTitle(_commercial); }
            webSrv.send((String) "streamtitle=" + _commercial);
            _f_newCommercial = false;
        }

        if(_cur_Codec == 0) {
            uint8_t c = audioGetCodec();
            if(c != 0 && c != 8 && c < 10) { // unknown or OGG, guard: c {1 ... 7, 9}
                _cur_Codec = c;
                SerialPrintfln("Audiocodec:  " ANSI_ESC_YELLOW "%s", codecname[c]);
                if(_state == PLAYER || _state == PLAYERico) showFileLogo(_state);
                if(_state == RADIO && _f_logoUnknown == true) {
                    _f_logoUnknown = false;
                    showFileLogo(_state);
                }
            }
        }

        if(_f_isFSConnected) {
            //    uint32_t t = 0;
            //    uint32_t fs = audioGetFileSize();
            //    uint32_t br = audioGetBitRate();
            //    if(br) t = (fs * 8)/ br;
            //    log_w("Br %d, Dur %ds", br, t);
        }
    }

    if(_f_10sec == true) {
        _f_10sec = false;
        if(_state == RADIO && !_icyBitRate && !_f_sleeping) {
            uint32_t ibr = audioGetBitRate() / 1000;
            if(ibr > 0) {
                if(ibr != _avrBitRate) {
                    _avrBitRate = ibr;
                    showFooterBitRate(_avrBitRate);
                }
            }
        }
        updateSettings();
    }

    if(_f_1min == true) {
        _f_1min = false;
        updateSleepTime();
    }

    if(_f_playlistEnabled) {
        if(!_f_playlistNextFile) {
            if(!audioIsRunning() && !_f_pauseResume) {
                SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "next playlist file");
                processPlaylist(false);
                _playlistTime = millis();
                _f_playlistNextFile = true;
            }
        }
        else {
            if(_playlistTime + 5000 < millis()) _f_playlistNextFile = false;
        }
    }
}
/*****************************************************************************************************************************************************
 *                                                                    E V E N T S                                                                    *
 *****************************************************************************************************************************************************/
// Events from vs1053_ext or audioI2S library
void vs1053_info(const char* info) {
    if(startsWith(info, "Request")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(startsWith(info, "FLAC")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
    if(endsWith(info, "Stream lost")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(startsWith(info, "authent")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
    if(startsWith(info, "StreamTitle=")) { return; }
    if(startsWith(info, "HTTP/") && info[9] > '3') {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN) // all other
    {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
}
void audio_info(const char* info) {
    if(startsWith(info, "Request")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(startsWith(info, "FLAC")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
    if(endsWith(info, "Stream lost")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(startsWith(info, "authent")) {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
    if(startsWith(info, "StreamTitle=")) { return; }
    if(startsWith(info, "HTTP/") && info[9] > '3') {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "%s", info);
        return;
    }
    if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN) // all other
    {
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);
        return;
    }
}
//----------------------------------------------------------------------------------------
void vs1053_showstation(const char* info) {
    _stationName_air = info;
    if(strlen(info)) SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", info);
    if(!_cur_station) _f_newLogoAndStation = true;
}
void audio_showstation(const char* info) {
    _stationName_air = info;
    if(strlen(info)) SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", info);
    if(!_cur_station) _f_newLogoAndStation = true;
}
//----------------------------------------------------------------------------------------
void vs1053_showstreamtitle(const char* info) {
    strcpy(_streamTitle, info);
    if(!_f_irNumberSeen) _f_newStreamTitle = true;
    SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", info);
}
void audio_showstreamtitle(const char* info) {
    strcpy(_streamTitle, info);
    if(!_f_irNumberSeen) _f_newStreamTitle = true;
    SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", info);
}
//----------------------------------------------------------------------------------------
void show_ST_commercial(const char* info) {
    _commercial_dur = atoi(info) / 1000; // info is the duration of advertising in ms
    char cdur[10];
    itoa(_commercial_dur, cdur, 10);
    if(_f_newCommercial) return;
    strcpy(_commercial, "Advertising: ");
    strcat(_commercial, cdur);
    strcat(_commercial, "s");
    _f_newCommercial = true;
    SerialPrintfln("StreamTitle: %s", info);
}
void vs1053_commercial(const char* info) { show_ST_commercial(info); }
void audio_commercial(const char* info) { show_ST_commercial(info); }
//----------------------------------------------------------------------------------------
void vs1053_eof_mp3(const char* info) { // end of mp3 file (filename)
    _f_eof = true;
    _f_isFSConnected = false;
    if(startsWith(info, "alarm")) _f_eof_alarm = true;
    SerialPrintfln("end of file: " ANSI_ESC_YELLOW "%s", info);
    if(_state == PLAYER || _state == PLAYERico) {
        clearLogo();
        clearStationName();
    }
    webSrv.send("SD_playFile=end of audiofile");
}
void audio_eof_mp3(const char* info) { // end of mp3 file (filename)
    _f_eof = true;
    _f_isFSConnected = false;
    if(startsWith(info, "alarm")) _f_eof_alarm = true;
    SerialPrintfln("end of file: " ANSI_ESC_YELLOW "%s", info);
    if(_state == PLAYER || _state == PLAYERico) {
        clearLogo();
        clearStationName();
    }
    webSrv.send("SD_playFile=end of audiofile");
}
//----------------------------------------------------------------------------------------
void vs1053_eof_stream(const char* info) {
    _f_isWebConnected = false;
    SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, info);
    if(_state == PLAYER || _state == PLAYERico) {
        clearLogo();
        clearStationName();
    }
}
void audio_eof_stream(const char* info) {
    _f_isWebConnected = false;
    SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, info);
    if(_state == PLAYER || _state == PLAYERico) {
        clearLogo();
        clearStationName();
    }
}
//----------------------------------------------------------------------------------------
void vs1053_lasthost(const char* info) { // really connected URL
    if(_f_playlistEnabled) return;
    _lastconnectedhost = info;
    SerialPrintfln("lastURL: ..  %s", _lastconnectedhost.c_str());
    webSrv.send("stationURL=" + _lastconnectedhost);
}
void audio_lasthost(const char* info) { // really connected URL
    if(_f_playlistEnabled) return;
    _lastconnectedhost = info;
    SerialPrintfln("lastURL: ..  %s", _lastconnectedhost.c_str());
    webSrv.send("stationURL=" + _lastconnectedhost);
}
//----------------------------------------------------------------------------------------
void vs1053_icyurl(const char* info) { // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5) {
        SerialPrintfln("icy-url: ..  %s", info);
        _homepage = String(info);
        if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
    }
}
void audio_icyurl(const char* info) { // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5) {
        SerialPrintfln("icy-url: ..  %s", info);
        _homepage = String(info);
        if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
    }
}
//----------------------------------------------------------------------------------------
void vs1053_id3data(const char* info) { SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s", info); }
void audio_id3data(const char* info) { SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s", info); }
//----------------------------------------------------------------------------------------
void vs1053_icydescription(const char* info) {
    strcpy(_icyDescription, info);
    _f_newIcyDescription = true;
    if(strlen(info)) SerialPrintfln("icy-descr:   %s", info);
}

void audio_icydescription(const char* info) {
    strcpy(_icyDescription, info);
    _f_newIcyDescription = true;
    if(strlen(info)) SerialPrintfln("icy-descr:   %s", info);
}
//----------------------------------------------------------------------------------------
void audio_bitrate(const char* info) {
    if(!strlen(info)) return; // guard
    _icyBitRate = str2int(info) / 1000;
    _f_newBitRate = true;
    SerialPrintfln("bitRate:     " ANSI_ESC_CYAN "%iKbit/s", _icyBitRate);
}
void vs1053_bitrate(const char* info) {
    if(!strlen(info)) return; // guard
    _icyBitRate = str2int(info) / 1000;
    _f_newBitRate = true;
    SerialPrintfln("bitRate:     " ANSI_ESC_CYAN "%iKbit/s", _icyBitRate);
}
//----------------------------------------------------------------------------------------
void ftp_debug(const char* info) {
    if(startsWith(info, "File Name")) return;
    SerialPrintfln("ftpServer:   %s", info);
}
//----------------------------------------------------------------------------------------
void RTIME_info(const char* info) { SerialPrintfln("rtime_info:  %s", info); }

// Events from tft library
void tft_info(const char* info) { SerialPrintfln("tft_info: .  %s", info); }

void ir_code(uint8_t addr, uint8_t cmd) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "0x%02x, " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02x", addr,
                   cmd);
    char buf[20];
    sprintf(buf, "IR_address=0x%02x", addr);
    webSrv.send(buf);
    sprintf(buf, "IR_command=0x%02x", cmd);
    webSrv.send(buf);
}

// Events from IR Library
void ir_res(uint32_t res) {
    _f_irNumberSeen = false;
    if(_state != RADIO) return;
    if(_f_sleeping == true) return;
    tft.fillRect(_winLogo.x, _winLogo.y, _dispWidth, _winName.h + _winTitle.h, TFT_BLACK);
    SerialPrintfln("ir_result:   " ANSI_ESC_YELLOW "Stationnumber " ANSI_ESC_BLUE "%d", res);
    if(res != 0) {
        setStation(res); // valid between 1 ... 999
        showVUmeter();
    }
    else {
        setStation(_cur_station);
        showVUmeter();
    }
    return;
}
void ir_number(uint16_t num) {
    if(_state != RADIO) return;
    if(_f_sleeping) return;
    if(!_f_irNumberSeen) tft.fillRect(_winLogo.x, _winLogo.y, _dispWidth, _winName.h + _winTitle.h, TFT_BLACK);
    _f_irNumberSeen = true;
    tft.setFont(_fonts[6]);
    tft.setTextColor(TFT_GOLD);
    tft.setCursor(_irNumber_x, _irNumber_y);
    tft.print(num);
}
void ir_key(uint8_t key) {
    if(_f_sleeping == true && key != 10) return;
    if(_f_sleeping == true && key == 10) { // awake
        wake_up();
        return;
    }

    switch(key) {
    case 15:
        if(_state == SLEEP) { // CLOCK <-> RADIO
            updateSleepTime(true);
            changeState(RADIO);
            break;
        }
        if(_state == RADIO) {
            changeState(CLOCK);
            break;
        }
        if(_state == CLOCK) {
            changeState(RADIO);
            break;
        }
        break;
    case 11:
        upvolume(); // VOLUME+
        break;
    case 12:
        downvolume(); // VOLUME-
        break;
    case 14:
        if(_state == RADIO) {
            nextStation();
            break;
        } // NEXT STATION
        if(_state == CLOCK) {
            nextStation();
            changeState(RADIO);
            _f_switchToClock = true;
            break;
        }
        if(_state == SLEEP) {
            display_sleeptime(1);
            break;
        }
        break;
    case 13:
        if(_state == RADIO) {
            prevStation();
            break;
        } // PREV STATION
        if(_state == CLOCK) {
            prevStation();
            changeState(RADIO);
            _f_switchToClock = true;
            break;
        }
        if(_state == SLEEP) display_sleeptime(-1);
        break;
    case 10:
        mute(); // MUTE
        break;
    case 16:
        if(_state == RADIO) {
            changeState(SLEEP);
            break;
        } // OFF TIMER
        if(_state == SLEEP) {
            changeState(RADIO);
            break;
        }
    default: break;
    }
}
void ir_long_key(int8_t key) {
    log_i("long pressed key nr: %i", key);
    if(key == 10) fall_asleep(); // long mute
}
// Event from TouchPad
// clang-format off
void tp_pressed(uint16_t x, uint16_t y) {
    // SerialPrintfln("tp_pressed, state is: %i", _state);
    //  SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint  x=%d, y=%d", x, y);
    enum : int8_t {
        none = -1,
        RADIO_1,
        RADIOico_1,
        RADIOico_2,
        RADIOmenue_1,
        PLAYER_1,
        PLAYERico_1,
        ALARM_1,
        BRIGHTNESS_1,
        CLOCK_1,
        CLOCKico_1,
        ALARM_2,
        SLEEP_1,
        DLNA_1,
        DLNAITEMSLIST_1,
        STATIONSLIST_1,
        AUDIOFILESLIST_1,
    };
    int8_t yPos = none;
    int8_t btnNr = none;     // buttonnumber

    if(_f_sleeping) return;  // awake in tp_released()

    switch(_state) {
        case RADIO:
            if(y <= _winTitle.y) { yPos = RADIO_1; }
            break;
        case RADIOico:
            if(y <= _winTitle.y) { yPos = RADIOico_1; }
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = RADIOico_2;
                btnNr = x / _winButton.w;
            }
            break;
        case RADIOmenue:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = RADIOmenue_1;
                btnNr = x / _winButton.w;
            }
            break;
        case PLAYER:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = PLAYER_1;
                btnNr = x / _winButton.w;
            }
            break;
        case PLAYERico:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = PLAYERico_1;
                btnNr = x / _winButton.w;
            }
            break;
        case CLOCK:
            if(_winDigits.y <= y && y <= _winDigits.y + _winDigits.h) { yPos = CLOCK_1; }
            break;
        case CLOCKico:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = CLOCKico_1;
                btnNr = x / _winButton.w;
            }
            break;
        case ALARM:
            if(_winAlarmDays.y <= y && y <= _winAlarmDays.y + _winAlarmDays.h) {
                yPos = ALARM_1;
                btnNr = (x - 2) / _alarmdays_w;
            }  // weekdays
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = ALARM_2;
                btnNr = x / _winButton.w;
            }
            break;
        case SLEEP:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = SLEEP_1;
                btnNr = x / _winButton.w;
            }
            break;
        case BRIGHTNESS:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = BRIGHTNESS_1;
                btnNr = x / _winButton.w;
            }
            break;
        case DLNA:
            if((y > _winButton.y) && (y < _winButton.y + _winButton.h)) {
                yPos = DLNA_1;
                btnNr = x / _winButton.w;
            }
            break;
        case STATIONSLIST:
            if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                btnNr = (y -_winHeader.h)  / (_winWoHF.h / 10);
                yPos = STATIONSLIST_1;
            }
            break;
        case AUDIOFILESLIST:
            if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                btnNr = (y -_winHeader.h)  / (_winWoHF.h / 10);
                yPos = AUDIOFILESLIST_1;
            }
            break;
        case DLNAITEMSLIST:
            if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                btnNr = (y -_winHeader.h)  / (_winWoHF.h / 10);
                yPos = DLNAITEMSLIST_1;
            }
            break;
        default:
            break;
    }
    if(yPos == none) {
        SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint not valid x=%d, y=%d", x, y);
        return;
    }

    switch(yPos){
        case RADIO_1:       changeState(RADIOico); break;
        case RADIOico_1:    changeState(RADIOmenue); break;
        case CLOCK_1:       changeState(CLOCKico);   break;
        case RADIOico_2:    if     (btnNr == 0){_releaseNr =  0; mute();}
                            else if(btnNr == 1){_releaseNr =  1; _timeCounter.timer = 5;} // Vol-
                            else if(btnNr == 2){_releaseNr =  2; _timeCounter.timer = 5;} // Vol+
                            else if(btnNr == 3){_releaseNr =  3; } // station--
                            else if(btnNr == 4){_releaseNr =  4; } // station++
                            else if(btnNr == 5){_releaseNr =  5; } // list stations
                            else   {SerialPrintfln(ANSI_ESC_YELLOW "invalid button nr: %i", btnNr); break;}
                            changeBtn_pressed(btnNr);
                            break;
        case RADIOmenue_1:  if(btnNr == 0){_releaseNr = 10; audioStopSong();} // AudioPlayer
                            if(btnNr == 1){_releaseNr = 11;} // DLNA
                            if(btnNr == 2){_releaseNr = 12;} // Clock
                            if(btnNr == 3){_releaseNr = 13;} // Sleep
                            if(TFT_BL != -1){
                                if(btnNr == 4){_releaseNr = 14;} // Brightness
                            }
                            changeBtn_pressed(btnNr); break;
        case CLOCKico_1:    if(btnNr == 0){_releaseNr = 20;} // Bell
                            if(btnNr == 1){_releaseNr = 21;} // Radio
                            if(btnNr == 2){_releaseNr = 22; mute();}
                            if(btnNr == 3){_releaseNr = 23; _timeCounter.timer = 5;} // Vol-
                            if(btnNr == 4){_releaseNr = 24; _timeCounter.timer = 5;} // Vol+
                            changeBtn_pressed(btnNr); break;
        case ALARM_2:       if(btnNr == 0){_releaseNr = 30;} // left
                            if(btnNr == 1){_releaseNr = 31;} // right
                            if(btnNr == 2){_releaseNr = 32;} // up
                            if(btnNr == 3){_releaseNr = 33;} // down
                            if(btnNr == 4){_releaseNr = 34;} // ready (return to CLOCK)
                            changeBtn_pressed(btnNr); break;
        case PLAYER_1:      if(btnNr == 0){_releaseNr = 40;} // first audiofile
                            if(btnNr == 1){_releaseNr = 41;} // next audiofile
                            if(btnNr == 2){_releaseNr = 42;} // ready
                            if(btnNr == 3){_releaseNr = 43;} // unused
                            if(btnNr == 4){_releaseNr = 44;} // unused
                            if(btnNr == 5){_releaseNr = 45;} // unused
                            if(btnNr == 6){_releaseNr = 46;} // audiofiles list
                            if(btnNr == 7){_releaseNr = 47;} // RADIO
                            changeBtn_pressed(btnNr); break;
        case PLAYERico_1:   if(btnNr == 0){_releaseNr = 50; mute();}
                            if(btnNr == 1){_releaseNr = 51;} // Vol-
                            if(btnNr == 2){_releaseNr = 52;} // Vol+
                            if(btnNr == 3){_releaseNr = 53;} // pause/resume
                            if(btnNr == 4){_releaseNr = 54;} // cancel
                            if(btnNr == 5){_releaseNr = 55;} // unused
                            if(btnNr == 6){_releaseNr = 56;} // unused
                            if(btnNr == 7){_releaseNr = 57;} // RADIO
                            changeBtn_pressed(btnNr); break;
        case ALARM_1:       if(btnNr == 0){_releaseNr = 60;} // mon
                            if(btnNr == 1){_releaseNr = 61;} // tue
                            if(btnNr == 2){_releaseNr = 62;} // wed
                            if(btnNr == 3){_releaseNr = 63;} // thu
                            if(btnNr == 4){_releaseNr = 64;} // fri
                            if(btnNr == 5){_releaseNr = 65;} // sat
                            if(btnNr == 6){_releaseNr = 66;} // sun
                            break;
        case SLEEP_1:       if(btnNr == 0){_releaseNr = 70;} // sleeptime up
                            if(btnNr == 1){_releaseNr = 71;} // sleeptime down
                            if(btnNr == 2){_releaseNr = 72;} // display_sleeptime(0, true);} // ready, return to RADIO
                            if(btnNr == 3){_releaseNr = 73;} // unused
                            if(btnNr == 4){_releaseNr = 74;} // return to RADIO without saving sleeptime
                            changeBtn_pressed(btnNr); break;
        case BRIGHTNESS_1:  if(btnNr == 0){_releaseNr = 80;} // darker
                            if(btnNr == 1){_releaseNr = 81;} // brighter
                            if(btnNr == 2){_releaseNr = 82;} // okay
                            changeBtn_pressed(btnNr); break;
        case DLNA_1:        if(btnNr == 0){_releaseNr = 90; mute();}
                            if(btnNr == 1){_releaseNr = 91; } // Vol-
                            if(btnNr == 2){_releaseNr = 92; } // Vol+
                            if(btnNr == 3){_releaseNr = 93; } // DLNAITEMSLIST
                            if(btnNr == 7){_releaseNr = 97;}  // RADIO
                            changeBtn_pressed(btnNr); break;
        case STATIONSLIST_1:if(btnNr == none) break;
                            _releaseNr = 100;
                            _staListPos = btnNr;
                            vTaskDelay(100);
                            break;
        case AUDIOFILESLIST_1: if(btnNr == none) break;
                            _releaseNr = 110;
                            _fileListPos = btnNr;
                            vTaskDelay(100);
                            break;
        case DLNAITEMSLIST_1: if(btnNr == none) break;
                            _releaseNr = 120;
                            _itemListPos = btnNr;
                            vTaskDelay(100);
                            break;
        default:            break;
    }
}
void tp_long_pressed(uint16_t x, uint16_t y){
    // log_w("long pressed %i  %i", x, y);
    if((_releaseNr == 0 || _releaseNr == 22 || _releaseNr == 50 || _releaseNr == 90) && _f_mute) {
        fall_asleep();
    }
}
void tp_released(uint16_t x, uint16_t y){
    // SerialPrintfln("tp_released, state is: %i", _state);
    if(_f_sleeping){ wake_up(); return;}   // if sleeping

    switch(_releaseNr){
        /* RADIOico ******************************/
        case  0:    /*changeBtn_released(0);*/ break; // Mute
        case  1:    changeBtn_released(1); downvolume(); showVolumeBar();  break;  // Vol-
        case  2:    changeBtn_released(2); upvolume();   showVolumeBar();  break;  // Vol+
        case  3:    changeBtn_released(3); prevStation(); showFooterStaNr(); break;  // previousstation
        case  4:    changeBtn_released(4); nextStation(); showFooterStaNr(); break;  //  nextstation
        case  5:    changeBtn_released(5); changeState(STATIONSLIST); break;  //  list stations

        /* RADIOmenue ******************************/
        case 10:    changeState(PLAYER);
                    setAudioFolder(_curAudioFolder.c_str());
                    nextAudioFile = getNextAudioFile();
                    showFileName(nextAudioFile.name());
                    break;
        case 11:    changeState(DLNA); break;
        case 12:    changeState(CLOCK); break;
        case 13:    changeState(SLEEP); break;
        case 14:    changeState(BRIGHTNESS); break;

        /* CLOCKico ******************************/
        case 20:    changeState(ALARM); break;
        case 21:    changeState(RADIO); break;
        case 22:    changeBtn_released(2); break; // Mute
        case 23:    changeBtn_released(3); downvolume(); /* showVolumeBar(); */ break;
        case 24:    changeBtn_released(4); upvolume();   /* showVolumeBar();*/ break;

        /* ALARM ******************************/
        case 30:    changeBtn_released(0); display_alarmtime(-1 ,  0); break;
        case 31:    changeBtn_released(1); display_alarmtime( 1 ,  0); break;
        case 32:    changeBtn_released(2); display_alarmtime( 0 ,  1); break;
        case 33:    changeBtn_released(3); display_alarmtime( 0 , -1); break;
        case 34:    changeState(CLOCK); break;

        /* AUDIOPLAYER ******************************/
        case 40:    changeBtn_released(0); // first audiofile
                    if(setAudioFolder(_curAudioFolder.c_str()));
                    nextAudioFile = getNextAudioFile();
                    showFileName(nextAudioFile.name());
                    break;
        case 41:    changeBtn_released(1); // next audiofile
                    nextAudioFile = getNextAudioFile();
                    showFileName(nextAudioFile.name());
                    break;
        case 42:    changeState(PLAYERico); showVolumeBar(); // ready
                    SD_playFile(nextAudioFile.path());
                    if(_f_isFSConnected && nextAudioFile){
                        free(_lastconnectedfile);
                        _lastconnectedfile = strdup(nextAudioFile.path());
                    } break;
        case 43:    SerialPrintfln(ANSI_ESC_YELLOW "Button number: %d is unsed yet", _releaseNr); break;
        case 44:    SerialPrintfln(ANSI_ESC_YELLOW "Button number: %d is unsed yet", _releaseNr); break;
        case 45:    SerialPrintfln(ANSI_ESC_YELLOW "Button number: %d is unsed yet", _releaseNr); break;
        case 46:    changeState(AUDIOFILESLIST); break;
        case 47:    changeBtn_released(0); changeState(RADIO); break; // RADIO

        /* AUDIOPLAYERico ******************************/
        case 50:    changeBtn_released(0); break; // Mute
        case 51:    changeBtn_released(1); downvolume(); showVolumeBar(); break; // Vol-
        case 52:    changeBtn_released(2); upvolume();   showVolumeBar(); break; // Vol+
        case 53:    if(_f_isFSConnected){
                        if(!_f_pauseResume){_f_pauseResume = true; // toggle pause/resume an set the flag
                            _pressBtn[3] = "/btn/Button_Right_Yellow.jpg"; _releaseBtn[3] = "/btn/Button_Right_Blue.jpg";
                            SerialPrintfln("Audioplayer: " ANSI_ESC_GREEN "Audio file is paused");}
                        else {_f_pauseResume = false;
                            _pressBtn[3] = "/btn/Button_Pause_Yellow.jpg"; _releaseBtn[3] = "/btn/Button_Pause_Blue.jpg";
                            SerialPrintfln("Audioplayer: " ANSI_ESC_GREEN "Audio file is resumed");}
                        drawImage(_releaseBtn[3], 3 * _winButton.w,  _winButton.y);
                        audioPauseResume();
                    }
                    else{
                        changeBtn_released(3);
                        SerialPrintfln("Audioplayer: " ANSI_ESC_GREEN "Web files can't be paused");
                    }
                    break;
        case 54:    stopSong(); changeState(PLAYER);  break;
        case 55:    SerialPrintfln(ANSI_ESC_YELLOW "Button number: %d is unsed yet", _releaseNr); break;
        case 56:    SerialPrintfln(ANSI_ESC_YELLOW "Button number: %d is unsed yet", _releaseNr); break;
        case 57:    stopSong(); changeState(RADIO); break;

        /* ALARM (weekdays) ******************************/
        case 60:    display_alarmDays(0); break;  // sun
        case 61:    display_alarmDays(1); break;  // mon
        case 62:    display_alarmDays(2); break;
        case 63:    display_alarmDays(3); break;
        case 64:    display_alarmDays(4); break;
        case 65:    display_alarmDays(5); break;
        case 66:    display_alarmDays(6); break;

        /* SLEEP ******************************************/
        case 70:    display_sleeptime(1);  changeBtn_released(0); break;
        case 71:    display_sleeptime(-1); changeBtn_released(1); break;
        case 72:    updateSleepTime(true);
                    changeBtn_released(2);
                    changeState(RADIO); break;
        case 73:    changeBtn_released(3); break; // unused
        case 74:    _sleeptime = 0;
                    changeBtn_released(4);
                    changeState(RADIO); break;

        /* BRIGHTNESS ************************************/
        case 80:    downBrightness(); changeBtn_released(0); break;
        case 81:    upBrightness();   changeBtn_released(1); break;
        case 82:    changeState(RADIO); break;

        /* DLNA ******************************************/
        case 90:    changeBtn_released(0); break; // Mute
        case 91:    changeBtn_released(1); downvolume(); showVolumeBar();  break;  // Vol-
        case 92:    changeBtn_released(2); upvolume();   showVolumeBar();  break;  // Vol+
        case 93:    changeState(DLNAITEMSLIST); break;
        case 97:    changeState(RADIO); break;

        /* STATIONSLIST *********************************/
        case 100:   if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                        uint8_t staListPos = (y -_winHeader.h)  / (_winWoHF.h / 10);
                        if(_staListPos + 2 < staListPos){               // wipe down
                            if(_staListNr == 0) break;
                            if(_staListNr >  9) _staListNr -= 9;
                            else _staListNr = 0;
                            showStationsList(_staListNr);
                        }
                        else if(staListPos + 2 < _staListPos){          // wipe up
                            if(_staListNr + 9 >= _sum_stations) break;
                            _staListNr += 9;
                            showStationsList(_staListNr);
                        }
                        else if(staListPos == _staListPos){
                            uint16_t staNr = _staListNr + staListPos + 1;
                            if(staNr > _sum_stations){
                                SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint not valid x=%d, y=%d", x, y);
                                break;
                            }
                            uint8_t lineHight = _winWoHF.h / 10;
                            tft.setCursor(10, _winFooter.h + (staListPos) * lineHight);
                            sprintf(_chbuf, "station_%03d", staNr);
                            String content = stations.getString(_chbuf, " #not_found");
                            int32_t idx = content.indexOf("#");
                            sprintf(_chbuf, ANSI_ESC_YELLOW"%03d " ANSI_ESC_CYAN "%s\n",staNr, content.substring(0, idx).c_str());
                            tft.writeText((uint8_t*)_chbuf, -1, -1, true);
                            _timeCounter.timer = 0;
                            showFooterRSSI(true);
                            setStation(staNr);
                            changeState(RADIO);
                        }
                        else log_i("unknown gesture");
                    } break;
        /* AUDIOFILESLIST *******************************/
        case 110:   if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                        uint8_t fileListPos = (y -_winHeader.h)  / (_winWoHF.h / 10);
                        if(_fileListPos + 2 < fileListPos){               // wipe down
                            if(_fileListNr == 0) break;
                            if(_fileListNr >  9) _fileListNr -= 9;
                            else _fileListNr = 0;
                            showAudioFilesList(_fileListNr);
                        }
                        else if(fileListPos + 2 < _fileListPos){          // wipe up
                            if(_fileListNr + 9 >= _SD_content.size()) break;
                            _fileListNr += 9;
                            showAudioFilesList(_fileListNr);
                        }
                        else if(fileListPos == _fileListPos){
                            uint16_t fileNr = _fileListNr + fileListPos;
                            if(fileNr > _SD_content.size()){
                                SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint not valid x=%d, y=%d", x, y);
                                break;
                            }
                            uint8_t lineHight = _winWoHF.h / 10;
                            tft.setCursor(20, _winFooter.h + (fileListPos) * lineHight);
                            if(fileListPos == 0) {
                                int32_t idx = _curAudioFolder.lastIndexOf("/");
                                if(idx > 1){ // not the first '/'
                                    _curAudioFolder = _curAudioFolder.substring(0, idx);
                                    _fileListNr = 0;
                                    SD_listDir(_curAudioFolder.c_str());
                                    showAudioFilesList(_fileListNr);
                                    break;
                                }
                            }
                            else{
                                if(fileNr > _SD_content.size()) break;
                                tft.setTextColor(TFT_CYAN);
                                tft.writeText((uint8_t*)_SD_content[fileNr - 1], -1, -1, true);
                                sprintf(_chbuf, "%s/%s", _curAudioFolder.c_str() ,_SD_content[fileNr - 1]);
                                int32_t idx = indexOf(_chbuf, "\033[", 1);
                                if(idx == -1){ // is folder
                                    _curAudioFolder += "/" + (String)_SD_content[fileNr - 1];
                                    _fileListNr = 0;
                                    SD_listDir(_curAudioFolder.c_str());
                                    showAudioFilesList(_fileListNr);
                                    break;
                                }
                                else {
                                    _chbuf[idx] = '\0';  // remove color and filesize
                                    changeState(PLAYER);
                                    SD_playFile(_chbuf, 0, true);
                                }
                            }
                            _timeCounter.timer = 0;
                            showFooterRSSI(true);
                        }
                        else log_i("unknown gesture");
                    } break;
        /* DLNAITEMSLIST *********************************/
        case 120:   if(y -_winHeader.h >= 0 && y -_winHeader.h <= _winWoHF.h){
                        uint8_t itemListPos = (y -_winHeader.h)  / (_winWoHF.h / 10);
                        if(_itemListPos + 2 < itemListPos){               // wipe down
                            if(_itemListNr == 0) break;
                            if(_itemListNr >  9) _itemListNr -= 9;
                            else _itemListNr = 0;
                            showDlnaItemsList( 0,   _itemListNr);
                        }
                        else if(itemListPos + 2 < _itemListPos){          // wipe up
                            if(_itemListNr + 9 >= _SD_content.size()) break;
                            _itemListNr += 9;
                            showDlnaItemsList( 0,    _itemListNr);
                        }





                    } break;

        default:    break;
    }
    _releaseNr = -1;
}

void tp_long_released(){
    // log_w("long released)");
    if(_releaseNr == 0 || _releaseNr == 22 || _releaseNr == 50) {return;}
    tp_released(0, 0);
}

//Events from websrv
void WEBSRV_onCommand(const String cmd, const String param, const String arg){  // called from html

    if(CORE_DEBUG_LEVEL == ARDUHAL_LOG_LEVEL_WARN){
        SerialPrintfln("WS_onCmd:    " ANSI_ESC_YELLOW "cmd=\"%s\", params=\"%s\", arg=\"%s\"",
                                                        cmd.c_str(),param.c_str(), arg.c_str());
    }

    String  str;

    if(cmd == "homepage"){          webSrv.send("homepage=" + _homepage);
                                    return;}

    if(cmd == "to_listen"){         StationsItems(); // via websocket, return the name and number of the current station
                                    return;}

    if(cmd == "gettone"){           if(DECODER) webSrv.send((String)"settone=" + setI2STone().c_str());
                                    else        webSrv.send((String)"settone=" + setTone().c_str());
                                    return;}

    if(cmd == "getmute"){           if(_f_mute) webSrv.send("mute=1");
                                    else        webSrv.send("mute=0");
                                    return;}

    if(cmd == "setmute"){           mute();
                                    return;}

    if(cmd == "getstreamtitle"){    webSrv.reply(_streamTitle, webSrv.TEXT);
                                    return;}

    if(cmd == "toneha"){            _toneha = param.toInt();                           // vs1053 treble gain
                                    setTone();
                                    char lp[40] = "tone=Treble Gain set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                    webSrv.send(lp); return;}
    if(cmd == "tonehf"){            _tonehf = param.toInt();                           // vs1053 treble frequency
                                    setTone();
                                    char lp[40] = "tone=Treble Frecquency set to "; strcat(lp, param.c_str()); strcat(lp, "KHz");
                                    webSrv.send(lp); return;}
    if(cmd == "tonela"){            _tonela = param.toInt();                           // vs1053 bass gain
                                    setTone();
                                    char lp[40] = "tone=Bass Gain set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                    webSrv.send(lp); return;}
    if(cmd == "tonelf"){            _tonelf = param.toInt();                           // vs1053 bass frequency
                                    setTone();
                                    char lp[40] = "tone=Bass Frecquency set to "; strcat(lp, param.c_str()); strcat(lp, "KHz");
                                    webSrv.send(lp); return;}
    if(cmd == "LowPass"){           _toneLP = param.toInt();                           // audioI2S tone
                                    char lp[30] = "tone=Lowpass set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                    webSrv.send(lp); setI2STone(); return;}

    if(cmd == "BandPass"){          _toneBP = param.toInt();                           // audioI2S tone
                                    char bp[30] = "tone=Bandpass set to "; strcat(bp, param.c_str()); strcat(bp, "dB");
                                    webSrv.send(bp); setI2STone(); return;}

    if(cmd == "HighPass"){          _toneHP = param.toInt();                           // audioI2S tone
                                    char hp[30] = "tone=Highpass set to "; strcat(hp, param.c_str()); strcat(hp, "dB");
                                    webSrv.send(hp); setI2STone(); return;}

    if(cmd == "uploadfile"){        _filename = param;  return;}

    if(cmd == "upvolume"){          webSrv.send("volume=" + (String)upvolume());  return;}                                                            // via websocket

    if(cmd == "downvolume"){        webSrv.send("volume=" + (String)downvolume()); return;}                                                           // via websocket

    if(cmd == "prev_station"){      prevStation(); return;}                                                                                           // via websocket

    if(cmd == "next_station"){      nextStation(); return;}                                                                                           // via websocket

    if(cmd == "set_station"){       setStation(param.toInt()); return;}                                                                               // via websocket

    if(cmd == "stationURL"){        setStationViaURL(param.c_str()); return;}                                                                         // via websocket

    if(cmd == "getnetworks"){       webSrv.send((String)"networks=" + WiFi.SSID().c_str()); return;}                                                  // via websocket

    if(cmd == "ping"){              webSrv.send("pong"); return;}                                                                                     // via websocket

    if(cmd == "index.html"){        if(_f_accessPoint) {SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "accesspoint.html");                           // via XMLHttpRequest
                                                        webSrv.show(accesspoint_html, webSrv.TEXT);}
                                    else               {SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "index.html");
                                                        webSrv.show(index_html, webSrv.TEXT);      }
                                    return;}

    if(cmd == "index.js"){          SerialPrintfln("Script:      " ANSI_ESC_ORANGE "index.js");                                                       // via XMLHttpRequest
                                    webSrv.show(index_js, webSrv.JS); return;}

    if(cmd == "get_tftSize"){       webSrv.send(_tftSize? "tftSize=m": "tftSize=s"); return;}

    if(cmd == "getTimeZones"){      webSrv.streamfile(SD_MMC, "/timezones.csv"); return;}

    if(cmd == "setTimeZone"){       _TZName = param;  _TZString = arg;
                                    SerialPrintfln("Timezone: .. " ANSI_ESC_BLUE "%s, %s", param.c_str(), arg.c_str());
                                    setRTC(_TZString.c_str());
                                    return;}

    if(cmd == "getTimeZoneName"){   webSrv.reply(_TZName, webSrv.TEXT); return;}

    if(cmd == "get_decoder"){       webSrv.send( DECODER? "decoder=s": "decoder=h"); return;}

    if(cmd == "favicon.ico"){       webSrv.streamfile(SD_MMC, "/favicon.ico"); return;}                                                               // via XMLHttpRequest

    if(cmd == "change_state"){      if(_state != CLOCK) changeState(param.toInt()); return;}

    if(cmd == "stopfile"){          _resumeFilePos = audioStopSong(); webSrv.send("stopfile=audiofile stopped");
                                    if(playlistFile) playlistFile.close();
                                    return;}

    if(cmd == "resumefile"){        if(!_lastconnectedfile) webSrv.send("resumefile=nothing to resume");
                                    else {
                                        SD_playFile(_lastconnectedfile, _resumeFilePos);
                                        webSrv.send("resumefile=audiofile resumed");
                                    }
                                    return;}

    if(cmd == "get_alarmdays"){     webSrv.send("alarmdays=" + String(_alarmdays, 10)); return;}

    if(cmd == "set_alarmdays"){     _alarmdays = param.toInt(); updateSettings(); return;}

    if(cmd == "get_alarmtime"){     webSrv.send("alarmtime=" + String(_alarmtime, 10)); return;}

    if(cmd == "set_alarmtime"){     _alarmtime = param.toInt(); updateSettings(); return;}

    if(cmd == "get_timeAnnouncement"){ if(_f_timeAnnouncement) webSrv.send("timeAnnouncement=1");
                                    if(  !_f_timeAnnouncement) webSrv.send("timeAnnouncement=0");
                                    return;}

    if(cmd == "set_timeAnnouncement"){ if(param == "true" ) _f_timeAnnouncement = true;
                                    if(   param == "false") _f_timeAnnouncement = false;
                                    return;}

    if(cmd == "DLNA_getServer")  {  DLNA_showServer(); return;}
    if(cmd == "DLNA_getContent0"){  _level = 0; DLNA_showContent(param, 0); return;}
    if(cmd == "DLNA_getContent1"){  _level = 1; DLNA_showContent(param, 1); return;} // search for level 1 content
    if(cmd == "DLNA_getContent2"){  _level = 2; DLNA_showContent(param, 2); return;} // search for level 2 content
    if(cmd == "DLNA_getContent3"){  _level = 3; DLNA_showContent(param, 3); return;} // search for level 3 content
    if(cmd == "DLNA_getContent4"){  _level = 4; DLNA_showContent(param, 4); return;} // search for level 4 content
    if(cmd == "DLNA_getContent5"){  _level = 5; DLNA_showContent(param, 5); return;} // search for level 5 content

    if(cmd == "test"){              sprintf(_chbuf, "free heap: %u, Inbuff filled: %u, Inbuff free: %u",
                                    ESP.getFreeHeap(), audioInbuffFilled(), audioInbuffFree());
                                    webSrv.send((String)"test=" + _chbuf);
                                    SerialPrintfln("audiotask .. stackHighWaterMark: %u bytes", audioGetStackHighWatermark() * 4);
                                    SerialPrintfln("looptask ... stackHighWaterMark: %u bytes", uxTaskGetStackHighWaterMark(NULL) * 4);
                                    return;}

    if(cmd == "AP_ready"){          webSrv.send("networks=" + String(_scannedNetworks)); return;}                                                     // via websocket

    if(cmd == "credentials"){       String AP_SSID = param.substring(0, param.indexOf("\n"));                                                         // via websocket
                                    String AP_PW =   param.substring(param.indexOf("\n") + 1);
                                    SerialPrintfln("credentials: SSID " ANSI_ESC_BLUE "%s" ANSI_ESC_WHITE ", PW " ANSI_ESC_BLUE "%s",
                                    AP_SSID.c_str(), AP_PW.c_str());
                                    pref.putString("ap_ssid", AP_SSID);
                                    pref.putString("ap_pw", AP_PW);
                                    ESP.restart();}

    if(cmd.startsWith("SD/")){      String str = cmd.substring(2);                                                                                    // via XMLHttpRequest
                                    if(!webSrv.streamfile(SD_MMC, scaleImage(str.c_str()))){
                                        webSrv.streamfile(SD_MMC, scaleImage("/unknown.jpg"));}
                                    return;}

    if(cmd == "SD_Download"){       webSrv.streamfile(SD_MMC, param.c_str());
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Download  " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                    return;}

    if(cmd == "SD_GetFolder"){      webSrv.reply(SD_dirContent(param), webSrv.JS);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "GetFolder " ANSI_ESC_ORANGE "\"%s\"", param.c_str());             // via XMLHttpRequest
                                    return;}

    if(cmd == "SD_newFolder"){      bool res = SD_newFolder(param.c_str());
                                    if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "NewFolder " ANSI_ESC_ORANGE "\"%s\"", param.c_str());             // via XMLHttpRequest
                                    return;}

    if(cmd == "SD_playFile"){       webSrv.reply("SD_playFile=" + param, webSrv.TEXT);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play " ANSI_ESC_ORANGE "\"%s\"", param.c_str());                  // via XMLHttpRequest
                                    SD_playFile(param.c_str());
                                    return;}

    if(cmd == "SD_rename"){         SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Rename " ANSI_ESC_ORANGE "old \"%s\" new \"%s\"",                 // via XMLHttpRequest
                                    param.c_str(), arg.c_str());
                                    bool res = SD_rename(param.c_str(), arg.c_str());
                                    if(res) webSrv.reply("refresh", webSrv.TEXT);
                                    else webSrv.sendStatus(400);
                                    return;}

    if(cmd == "SD_delete"){         bool res = SD_delete(param.c_str());
                                    if(res) webSrv.sendStatus(200); else webSrv.sendStatus(400);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Delete " ANSI_ESC_ORANGE "\"%s\"", param.c_str());                // via XMLHttpRequest
                                    return;}

    if(cmd == "SD_Upload"){        _filename = param;
                                   _f_SD_Upload = true;
                                   SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Upload  " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                   return;}

    if(cmd == "setIRcmd"){         int32_t command = (int32_t)strtol(param.c_str(), NULL, 16);
                                   int32_t btnNr = (int32_t)strtol(arg.c_str(), NULL, 10);
                                   SerialPrintfln("set_IR_cmd:  " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02x, "
                                   ANSI_ESC_YELLOW "IR Button Number " ANSI_ESC_BLUE "0x%02x", command, btnNr);
                                   ir.set_irButtons(btnNr,  command);
                                   return;}
    if(cmd == "setIRadr"){         SerialPrintfln("set_IR_adr:  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "%s",
                                   param.c_str());
                                   int32_t address = (int32_t)strtol(param.c_str(), NULL, 16);
                                   ir.set_irAddress(address);
                                   return;}

    if(cmd == "saveIRbuttons"){    saveIRbuttonsToNVS(); return;}

    if(cmd == "getTimeFormat"){    webSrv.send("timeFormat=" + String(_timeFormat, 10));
                                   return;}

    if(cmd == "setTimeFormat"){    _timeFormat = param.toInt();
                                   if(_state == CLOCK){
                                        clearLogoAndStationname();
                                        clearTitle();
                                        display_time(true);
                                   }
                                   SerialPrintfln("TimeFormat:  " ANSI_ESC_YELLOW "new time format: " ANSI_ESC_BLUE "%sh", param.c_str());
                                   return;}

    if(cmd == "loadIRbuttons"){    loadIRbuttonsFromNVS(); // update IR buttons in ir.cpp
                                   char buf[150];
                                   uint8_t* buttons = ir.get_irButtons();
                                   sprintf(buf,"0x%02x,", ir.get_irAddress());
                                   for(uint8_t i = 0; i< 20; i++){
                                        sprintf(buf + 5 + 5 * i, "0x%02x,", buttons[i]);
                                   }
                                   buf[5 + 5 * 20] = '\0';
                                   webSrv.reply(buf, webSrv.TEXT); return;}

    if(cmd == "DLNA_GetFolder"){   webSrv.sendStatus(306); return;}  // todo

    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd.c_str(), param.c_str());
    webSrv.sendStatus(400);
}
// clang-format on
void WEBSRV_onRequest(const String request, uint32_t contentLength) {
    if(CORE_DEBUG_LEVEL > ARDUHAL_LOG_LEVEL_INFO) {
        SerialPrintfln("WS_onReq:    " ANSI_ESC_YELLOW "%s contentLength %d", request.c_str(), contentLength);
    }

    if(request.startsWith("------")) return;     // uninteresting WebKitFormBoundaryString
    if(request.indexOf("form-data") > 0) return; // uninteresting Info
    if(request == "fileUpload") {
        savefile(_filename.c_str(), contentLength);
        return;
    }
    if(request.startsWith("Content")) return; // suppress Content-Disposition and Content-Type

    SerialPrintfln(ANSI_ESC_RED "unknown request: %s", request.c_str());
}
void WEBSRV_onInfo(const char* info) {
    if(startsWith(info, "WebSocket")) return;                           // suppress WebSocket client available
    if(!strcmp("ping", info)) return;                                   // suppress ping
    if(!strcmp("to_listen", info)) return;                              // suppress to_isten
    if(startsWith(info, "Command client")) return;                      // suppress Command client available
    if(startsWith(info, "Content-D")) return;                           // Content-Disposition
    if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG) {
        SerialPrintfln("HTML_info:   " ANSI_ESC_YELLOW "\"%s\"", info); // infos for debug
    }
}

// Events from DLNA
void dlna_info(const char* info) {
    SerialPrintfln("DLNA_info:   " ANSI_ESC_WHITE "%s", info); // infos
}

void dlna_server(uint8_t serverId, size_t serverSize, String IP_addr, uint16_t port, String friendlyName, String controlURL) {
    SerialPrintfln("DLNA_server: [%d][%d] %s : %d %s -> %s", serverId, serverSize, IP_addr.c_str(), port, friendlyName.c_str(), controlURL.c_str());
    char id[4];
    itoa(serverId, id, 10);
    String msg = "DLNA_Names=" + friendlyName + "," + id;
    webSrv.send(msg);

    _dlna_items.serverFriendlyName.push_back(ps_strdup(friendlyName.c_str()));
    _dlna_items.serverId.push_back(serverId);
}

void dlna_folder(bool lastItem, String name, String id, size_t childCount) { // list folders
    static JSONVar myObject;
    static uint8_t i = 0, j = 0;
    if(lastItem) {
        i = 0;
        if(!j) return;
    }
    if(lastItem || j == 10) {
        j = 0;
        String msg = "Level" + String(_level + 1, 10) + "=" + JSON.stringify(myObject);
        webSrv.send(msg);
        myObject = "";
        return;
    }
    if(lastItem || j == 10) {
        j = 0;
        String msg = "Level" + String(_level + 1, 10) + "=" + JSON.stringify(myObject);
        webSrv.send(msg);
        myObject = "";
        return;
    }
    if(i < 10) { SerialPrintfln("DLNA_folder: " ANSI_ESC_ORANGE "\"%s\", id: \"%s\", childCount: %d", name.c_str(), id.c_str(), childCount); }
    if(i == 10) { SerialPrintfln("DLNA_folder: " ANSI_ESC_ORANGE "more entries follows"); }
    myObject[j]["name"] = name.c_str();
    myObject[j]["isDir"] = 1;
    myObject[j]["id"] = id.c_str();
    myObject[j]["size"] = childCount;
    myObject[j]["isAudio"] = true;
    i++;
    j++;
}

void dlna_file(bool lastItem, String name, String id, size_t size, String uri, bool isAudio) { // list files
    static JSONVar myObject;
    static uint8_t i = 0, j = 0;
    if(lastItem) {
        i = 0;
        if(!j) return;
    }
    if(lastItem || j == 10) {
        j = 0;
        String msg = "Level" + String(_level + 1, 10) + "=" + JSON.stringify(myObject);
        webSrv.send(msg);
        myObject = "";
        return;
    }
    if(i < 10) {
        if(isAudio) { SerialPrintfln("DLNA_file:   " ANSI_ESC_GREEN "\"%s\"", name.c_str()); }
        else { SerialPrintfln("DLNA_file:   " ANSI_ESC_ORANGE "\"%s\"", name.c_str()); }
    }
    if(i == 10) { SerialPrintfln("DLNA_file:   " ANSI_ESC_GREEN "more entries follows"); }

    myObject[j]["name"] = name.c_str();
    myObject[j]["isDir"] = 0;
    myObject[j]["id"] = uri.c_str();
    myObject[j]["size"] = size;
    myObject[j]["isAudio"] = isAudio;
    i++;
    j++;
}

void dlna_item(bool lastItem, String name, String id, size_t size, String uri, bool isDir, bool isAudio){
    //log_w("lastItem %i, name %s, id %s, size %d, uri %s, isDir %i, isAudio %i", lastItem, name.c_str(), id.c_str(), size, uri.c_str(), isDir, isAudio);
    if(lastItem){
        _dlna_items.isReady = true;

        int32_t s = _dlna_items.name.size();

        for(int32_t i = 0; i < s;  i++){
            log_w("name %s, id %s, size %d, uri %s, isDir %i, isAudio %i", _dlna_items.name[i], _dlna_items.id[i], _dlna_items.size[i], _dlna_items.uri[i], _dlna_items.isDir[i], _dlna_items.isAudio[i]);
        }

        dlna_items_vector_clear_ans_shrink();
        return;
    }
    _dlna_items.isReady = false;
    _dlna_items.name.push_back(ps_strdup(name.c_str()));
    _dlna_items.id.push_back(ps_strdup(id.c_str()));
    _dlna_items.size.push_back(size);
    _dlna_items.uri.push_back(ps_strdup(uri.c_str()));
    _dlna_items.isDir.push_back(isDir == true);
    _dlna_items.isAudio.push_back(isAudio == true);



}