#include "common.h"
// clang-format off
/*****************************************************************************************************************************************************
    MiniWebRadio -- Webradio receiver for ESP32-S3

    first release on 03/2017                                                                                                      */char Version[] ="\
    Version 4.0-rc3   - Aug 23/2025                                                                                                               ";

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

char _hl_item[18][40]{  "",                     // none
                        "Internet Radio",       // "* интернет-радио *"  "ραδιόφωνο Internet"
                        "Audio Player",         // "** цифрово́й плеер **
                        "DLNA",                 // Digital Living Network Alliance
                        "Clock",                // Clock "** часы́ **"  "** ρολόι **"
                        "Brightness",           // Brightness яркость λάμψη
                        "Alarm Clock (hh:mm)",  // AlarmClock "будильник" "ξύπνημα"
                        "Off Timer (h:mm)",     // "Sleeptimer" "Χρονομετρητής" "Таймер сна"
                        "Stations List",
                        "Audio Files",
                        "DLNA List",
                        "Bluetooth",
                        "Equalizer",
                        "Settings",
                        "IR Settings",
                        "Alarm",
                        "WiFi Settings",     //
                        ""};

settings_t                  _settings;
const uint16_t              _max_stations = 1000;
int8_t                      _currDLNAsrvNr = -1;
uint8_t                     _alarmdays = 0;
uint8_t                     _cur_volume = 21;
uint8_t                     _BTvolume = 16;      // KCX-BT_Emitter volume
uint8_t                     _ringVolume = 21;
uint8_t                     _volumeAfterAlarm = 12;
uint8_t                     _volumeSteps = 21;
uint8_t                     _volumeCurve = 1;
uint8_t                     _brightness = 100;   // percent
uint8_t                     _state = UNDEFINED;  // statemaschine
uint8_t                     _commercial_dur = 0; // duration of advertising
uint8_t                     _cur_Codec = 0;
uint8_t                     _numServers = 0;     //
uint8_t                     _level = 0;
uint8_t                     _timeFormat = 24;    // 24 or 12
uint8_t                     _sleepMode = 1;      // 0 display off,     1 show the clock
uint8_t                     _staListPos = 0;
uint8_t                     _WiFi_disconnectCnt = 0;
uint8_t                     _reconnectCnt = 0;
uint8_t                     _cthFailCounter = 0; // connecttohost fail
uint16_t                    _staListNr = 0;
uint8_t                     _fileListPos = 0;
uint8_t                     _radioSubMenue = 0;
uint8_t                     _playerSubMenue = 0;
uint8_t                     _dlnaSubMenue = 0;
uint8_t                     _clockSubMenue = 0;
uint8_t                     _alarmSubMenue = 0;
uint8_t                     _sleepTimerSubMenue = 0;
uint8_t                     _settingsSubMenue = 0;
uint8_t                     _brightnessSubMenue = 0;
uint8_t                     _equalizerSubMenue = 0;
uint8_t                     _ambientValue = 50;
uint16_t                    _fileListNr = 0;
uint16_t                    _irNumber = 0;
uint16_t                    _irResult = 0;
uint8_t                     _itemListPos = 0; // DLNA items
uint16_t                    _dlnaItemNr = 0;
uint8_t                     _dlnaLevel = 0;
int16_t                     _alarmtime[7] = {0};  // in minutes (23:59 = 23 *60 + 59) [0] Sun, [1] Mon
int16_t                     _toneLP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t                     _toneBP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t                     _toneHP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t                     _toneBAL = 0;         // -16...0....+16         audioI2S
uint16_t                    _icyBitRate = 0;      // from http response header via event
uint32_t                    _decoderBitRate = 0;  // from decoder via getBitRate(false)
uint16_t                    _cur_station = 0;     // current station(nr), will be set later
uint16_t                    _cur_AudioFileNr = 0; // this is the position of the file within the (alpha ordered) folder starting with 0
uint16_t                    _sleeptime = 0;       // time in min until MiniWebRadio goes to sleep
uint16_t                    _plsCurPos = 0;
uint16_t                    _totalNumberReturned = 0;
uint16_t                    _dlnaMaxItems = 0;
uint16_t                    _bh1750Value = 50;
uint32_t                    _playlistTime = 0;  // playlist start time millis() for timeout
uint32_t                    _settingsHash = 0;
uint32_t                    _audioFileSize = 0;
uint32_t                    _media_downloadPort = 0;
uint32_t                    _audioCurrentTime = 0;
uint32_t                    _audioFileDuration = 0;
uint64_t                    _totalRuntime = 0; // total runtime in seconds since start
uint8_t                     _resetReason = (esp_reset_reason_t)ESP_RST_UNKNOWN;
const char*                 _pressBtn[8];
const char*                 _releaseBtn[8];
const char*                 _time_s = "";
const char*                 _btEmitterMode = NULL;
char                        _chbuf[512];
char                        _fName[256];
char                        _myIP[25] = {0};
char                        _path[128];
char                        _prefix[5] = "s/";
char                        _commercial[25];
char                        _icyDescription[512] = {};
char                        _streamTitle[512] = {};
char                        _timeSpeechLang[10] = "en";
char*                       _cur_AudioFolder = NULL;
char*                       _cur_AudioFileName = NULL;
char*                       _stationURL = NULL;
char*                       _JSONstr = NULL;
char*                       _BT_metaData = NULL;
char*                       _playlistPath = NULL;
bool                        _f_rtc = false; // true if time from ntp is received
bool                        _f_100ms = false;
bool                        _f_1sec = false;
bool                        _f_10sec = false;
bool                        _f_1min = false;
bool                        _f_mute = false;
bool                        _f_muteIsPressed = false;
bool                        _f_volumeDownIsPressed = false;
bool                        _f_volumeUpIsPressed = false;
bool                        _f_sleeping = false;
bool                        _f_irOnOff = false;
bool                        _f_isWebConnected = false;
bool                        _f_isFSConnected = false;
bool                        _f_eof = false;
bool                        _f_reconnect = false;
bool                        _f_eof_alarm = false;
bool                        _f_alarm = false;
bool                        _f_irNumberSeen = false;
bool                        _f_irResultSeen = false;
bool                        _f_newIcyDescription = false;
bool                        _f_newStreamTitle = false;
bool                        _f_webFailed = false;
bool                        _f_newBitRate = false;
bool                        _f_newStationName = false;
bool                        _f_newCommercial = false;
bool                        _f_volBarVisible = false;
bool                        _f_switchToClock = false;    // jump into CLOCK mode at the next opportunity
bool                        _f_timeAnnouncement = true;  // time announcement every full hour
bool                        _f_playlistEnabled = false;
bool                        _f_playlistNextFile = false;
bool                        _f_logoUnknown = false;
bool                        _f_pauseResume = false;
bool                        _f_SD_Upload = false;
bool                        _f_PSRAMfound = false;
bool                        _f_FFatFound = false;
bool                        _f_SD_MMCfound = false;
bool                        _f_ESPfound = false;
bool                        _f_BH1750_found = false;
bool                        _f_clearLogo = false;
bool                        _f_clearStationName = false;
bool                        _f_shuffle = false;
bool                        _f_dlnaBrowseServer = false;
bool                        _f_dlnaWaitForResponse = false;
bool                        _f_dlnaSeekServer = false;
bool                        _f_dlnaMakePlaylistOTF = false; // notify callback that this browsing was to build a On-The_fly playlist
bool                        _f_dlna_browseReady = false;
bool                        _f_BtEmitterFound = false;
bool                        _f_BTEmitterConnected = false;
bool                        _f_brightnessIsChangeable = false;
bool                        _f_connectToLastStation = false;
bool                        _f_BTpower = false;
bool                        _f_BTcurPowerState = false;
bool                        _f_timeSpeech = false;
bool                        _f_stationsChanged = false;
bool                        _f_WiFiConnected = false;
String                      _station = "";
char*                       _stationName_air = NULL;
String                      _homepage = "";
String                      _filename = "";
String                      _TZName = "Europe/Berlin";
String                      _TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
String                      _media_downloadIP = "";
dlnaHistory                 _dlnaHistory[10];
timecounter                 _timeCounter;
SD_content                  _SD_content;
std::vector<char*>          _PLS_content;
std::vector<ps_ptr<char>>   _logBuffer;

const char* codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "AACP", "OPUS", "OGG", "VORBIS"};

Audio               audio;
Preferences         pref;
WebSrv              webSrv;
WiFiMulti           wifiMulti;
RTIME               rtc;
Ticker              ticker100ms;
IR_buttons          irb(&_settings);
IR                  ir(IR_PIN); // do not change the objectname, it must be "ir"

File                audioFile;
FtpServer           ftpSrv;
DLNA_Client         dlna;
KCX_BT_Emitter      bt_emitter(BT_EMITTER_RX, BT_EMITTER_TX, BT_EMITTER_LINK, BT_EMITTER_MODE);
TwoWire             i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire             i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
hp_BH1750           BH1750(&i2cBusOne);     // create the sensor
SPIClass            spiBus(FSPI);


#if TFT_CONTROLLER < 7 // ⏹⏹⏹⏹
TFT_SPI     tft(spiBus, TFT_CS);
TP_XPT2046  tp(spiBus, TP_CS);
#else
TFT_RGB     tft;
TP_GT911    tp(&i2cBusOne);
#endif

stationManagement   staMgnt(&_cur_station);

SemaphoreHandle_t mutex_rtc;
SemaphoreHandle_t mutex_display;
#if TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1 // ⏹⏹⏹⏹
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

const uint8_t _fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};
const uint8_t _listFontSize = 16;
const uint8_t _headerFontSize = 0; //16; // 0 -> autoSize
const uint8_t _footerFontSize = 0; //16; // 0 -> autoSize
const uint8_t _bigNumbersFontSize = 156;
const uint8_t _fileNumberFontSize = 21;
// ------------------------------------------------------------------------------------------padding-left, padding-right, padding-top, padding-bottom
struct w_h  {uint16_t x =   0; uint16_t y =   0; uint16_t w = 320; uint16_t h =  20;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winHeader;
struct w_l  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 100; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winLogo;
struct w_n  {uint16_t x = 100; uint16_t y =  20; uint16_t w = 220; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 3; uint8_t pt = 0; uint8_t pb = 3;} const _winName;
struct w_e  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 3; uint8_t pt = 0; uint8_t pb = 3;} const _winFName;
struct w_j  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 100; uint16_t h =  40;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winFileNr;
struct w_v  {uint16_t x = 138; uint16_t y =  34; uint16_t w = 144; uint16_t h =  72;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winVolBox;   // volumeBox
struct w_a  {uint16_t x =   0; uint16_t y = 160; uint16_t w = 320; uint16_t h =  11;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winProgbar;
struct w_t  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 3; uint8_t pt = 0; uint8_t pb = 3;} const _winTitle;
struct w_c  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 296; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 3; uint8_t pt = 0; uint8_t pb = 3;} const _winSTitle;
struct w_g  {uint16_t x = 296; uint16_t y = 120; uint16_t w =  24; uint16_t h = 100;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winVUmeter;
struct w_f  {uint16_t x =   0; uint16_t y = 220; uint16_t w = 320; uint16_t h =  20;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winFooter;
struct w_s  {uint16_t x =   0; uint16_t y = 220; uint16_t w =  60; uint16_t h =  20;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winStaNr;
struct w_p  {uint16_t x =  60; uint16_t y = 220; uint16_t w =  65; uint16_t h =  20;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winSleep;
struct w_b  {uint16_t x =   0; uint16_t y = 150; uint16_t w = 320; uint16_t h =  30;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrOvBtns;    // slider over buttons, max width
struct w_o  {uint16_t x =   0; uint16_t y = 180; uint16_t w =  40; uint16_t h =  40;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winButton;
struct w_y  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 160;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winDigits;    // clock and alarm digits
struct w_w  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 200;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _winWoHF;      // without Header and Footer
struct w_s1 {uint16_t x =  80; uint16_t y =  30; uint16_t w = 150; uint16_t h =  34;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrHP;        // slider highpass in equalizer
struct w_s2 {uint16_t x =  80; uint16_t y =  64; uint16_t w = 150; uint16_t h =  34;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrBP;        // slider bandpass in equalizer
struct w_s3 {uint16_t x =  80; uint16_t y =  98; uint16_t w = 150; uint16_t h =  34;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrLP;        // slider lowpass in equalizer
struct w_s4 {uint16_t x =  80; uint16_t y = 132; uint16_t w = 150; uint16_t h =  34;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrBAL;       // slider balance in equalizer

// derived structs
struct w_b1 {uint16_t x = 10;                  uint16_t y = _sdrHP.y;  uint16_t w = _sdrHP.w;  uint16_t h = _sdrHP.h; } const _btnHP;
struct w_b2 {uint16_t x = 10;                  uint16_t y = _sdrBP.y;  uint16_t w = _sdrBP.w;  uint16_t h = _sdrBP.h; } const _btnBP;
struct w_b3 {uint16_t x = 10;                  uint16_t y = _sdrLP.y;  uint16_t w = _sdrLP.w;  uint16_t h = _sdrLP.h; } const _btnLP;
struct w_b4 {uint16_t x = 10;                  uint16_t y = _sdrBAL.y; uint16_t w = _sdrBAL.w; uint16_t h = _sdrBAL.h;} const _btnBAL;
struct w_t1 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrHP.y;  uint16_t w = 90;        uint16_t h = _sdrHP.h; } const _txtHP;
struct w_t2 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBP.y;  uint16_t w = 90;        uint16_t h = _sdrBP.h; } const _txtBP;
struct w_t3 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrLP.y;  uint16_t w = 90;        uint16_t h = _sdrLP.h; } const _txtLP;
struct w_t4 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBAL.y; uint16_t w = 90;        uint16_t h = _sdrBAL.h;} const _txtBAL;

uint16_t _sleeptimeXPos[5] = {5, 77, 129, 57}; // last is colon
uint16_t _sleeptimeYPos = 48;
uint16_t _dispWidth   = 320;
uint16_t _dispHeight  = 240;
uint8_t  _BrightnessMin = 5; // slider Brightness left value
const char _tftSize[2] = "s";
//
// clang-format on
#endif // TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1

#if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5 || TFT_CONTROLLER == 6 // ⏹⏹⏹⏹⏫
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

const uint8_t _fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};
const uint8_t _listFontSize       = 21;
const uint8_t _headerFontSize     = 0; //21; // 0 -> autoSize
const uint8_t _footerFontSize     = 0; //21; // 0 -> autoSize
const uint8_t _bigNumbersFontSize = 156;
const uint8_t _fileNumberFontSize = 27;
// -----------------------------------------------------------------------------------padding-left-right-top-bottom
struct w_h  {uint16_t x =   0; uint16_t y =   0; uint16_t w = 480; uint16_t h =  30;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winHeader;
struct w_l  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 130; uint16_t h = 132;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winLogo;
struct w_n  {uint16_t x = 132; uint16_t y =  30; uint16_t w = 348; uint16_t h = 132;  uint8_t pl = 0; uint8_t pr = 4; uint8_t pt =  0; uint8_t pb =  3;} const _winName;     // station nane
struct w_e  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 132;  uint8_t pl = 0; uint8_t pr = 5; uint8_t pt =  0; uint8_t pb =  3;} const _winFName;
struct w_j  {uint16_t x =   0; uint16_t y = 164; uint16_t w = 130; uint16_t h =  40;  uint8_t pl = 0; uint8_t pr = 1; uint8_t pt =  0; uint8_t pb =  1;} const _winFileNr;
struct w_v  {uint16_t x = 200; uint16_t y =  48; uint16_t w = 256; uint16_t h =  96;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winVolBox;   // volumeBox
struct w_a  {uint16_t x =   0; uint16_t y = 210; uint16_t w = 480; uint16_t h =  14;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winProgbar;  // progressbar
struct w_t  {uint16_t x =   0; uint16_t y = 162; uint16_t w = 480; uint16_t h = 128;  uint8_t pl = 0; uint8_t pr = 5; uint8_t pt =  0; uint8_t pb =  3;} const _winTitle;
struct w_c  {uint16_t x =   0; uint16_t y = 162; uint16_t w = 448; uint16_t h = 128;  uint8_t pl = 0; uint8_t pr = 4; uint8_t pt =  0; uint8_t pb =  3;} const _winSTitle;   // streamTitle, space for VUmeter
struct w_g  {uint16_t x = 448; uint16_t y = 162; uint16_t w =  32; uint16_t h = 128;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winVUmeter;
struct w_f  {uint16_t x =   0; uint16_t y = 290; uint16_t w = 480; uint16_t h =  30;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winFooter;
struct w_s  {uint16_t x =   0; uint16_t y = 290; uint16_t w =  85; uint16_t h =  30;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winStaNr;
struct w_p  {uint16_t x =  85; uint16_t y = 290; uint16_t w =  87; uint16_t h =  30;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winSleep;
struct w_b  {uint16_t x =   0; uint16_t y = 194; uint16_t w = 480; uint16_t h =  40;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _sdrOvBtns;   // slider over buttons, max width
struct w_o  {uint16_t x =   0; uint16_t y = 234; uint16_t w =  56; uint16_t h =  56;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winButton;
struct w_y  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 200;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winDigits;    // clock and alarm digits
struct w_w  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 260;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  0; uint8_t pb =  0;} const _winWoHF;      // without Header and Footer
struct w_s1 {uint16_t x = 140; uint16_t y =  30; uint16_t w = 200; uint16_t h =  50;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  8; uint8_t pb =  8;} const _sdrHP;        // slider highpass in equalizer
struct w_s2 {uint16_t x = 140; uint16_t y =  80; uint16_t w = 200; uint16_t h =  50;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  8; uint8_t pb =  8;} const _sdrBP;        // slider bandpass in equalizer
struct w_s3 {uint16_t x = 140; uint16_t y = 130; uint16_t w = 200; uint16_t h =  50;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  8; uint8_t pb =  8;} const _sdrLP;        // slider lowpass in equalizer
struct w_s4 {uint16_t x = 140; uint16_t y = 180; uint16_t w = 200; uint16_t h =  50;  uint8_t pl = 0; uint8_t pr = 0; uint8_t pt =  8; uint8_t pb =  8;} const _sdrBAL;       // slider balance in equalizer

// derived structs
struct w_b1 {uint16_t x = 50;                  uint16_t y = _sdrHP.y;  uint16_t w = _sdrHP.w;  uint16_t h = _sdrHP.h; } const _btnHP;
struct w_b2 {uint16_t x = 50;                  uint16_t y = _sdrBP.y;  uint16_t w = _sdrBP.w;  uint16_t h = _sdrBP.h; } const _btnBP;
struct w_b3 {uint16_t x = 50;                  uint16_t y = _sdrLP.y;  uint16_t w = _sdrLP.w;  uint16_t h = _sdrLP.h; } const _btnLP;
struct w_b4 {uint16_t x = 50;                  uint16_t y = _sdrBAL.y; uint16_t w = _sdrBAL.w; uint16_t h = _sdrBAL.h;} const _btnBAL;
struct w_t1 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrHP.y;  uint16_t w = 140;       uint16_t h = _sdrHP.h; } const _txtHP;
struct w_t2 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBP.y;  uint16_t w = 140;       uint16_t h = _sdrBP.h; } const _txtBP;
struct w_t3 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrLP.y;  uint16_t w = 140;       uint16_t h = _sdrLP.h; } const _txtLP;
struct w_t4 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBAL.y; uint16_t w = 140;       uint16_t h = _sdrBAL.h;} const _txtBAL;

uint16_t _sleeptimeXPos[5] = {5, 107, 175, 73 };
uint16_t _sleeptimeYPos = 48;
uint16_t _dispWidth   = 480;
uint16_t _dispHeight  = 320;
uint8_t  _BrightnessMin = 5; // slider Brightness left value
const char _tftSize[2] = "m";
//
// clang-format on
#endif // #if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5|| TFT_CONTROLLER == 6

#if TFT_CONTROLLER >= 7 // ⏹⏹⏹⏹⏫
// clang-format off
//
//  Display 800x480
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       _winHeader=50px
//  +-------------------------------------------+ _yName=50
//  |                                           |
//  | Logo                   StationName        |       _winFName=190px
//  |                                           |
//  +-------------------------------------------+ _yTitle=240
//  |                                           |
//  |              StreamTitle                  |       _winTitle=190px
//  |                                           |
//  +-------------------------------------------+ _yFooter=430
//  | Footer                                    |       _winFooter=50px
//  +-------------------------------------------+ 480
//                                             800

const uint8_t _fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};
const uint8_t _listFontSize = 27;
const uint8_t _headerFontSize = 38;
const uint8_t _footerFontSize = 38;
const uint8_t _bigNumbersFontSize = 156;
const uint8_t _fileNumberFontSize = 34;
//----------------------------------------------------------------------------------------padding-left-right-top-bottom
struct w_h  {uint16_t x =   0; uint16_t y =   0; uint16_t w = 800; uint16_t h =  50; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winHeader;
struct w_l  {uint16_t x =   0; uint16_t y =  50; uint16_t w = 190; uint16_t h = 190; uint8_t pl =  4; uint8_t pr =  4; uint8_t pt = 4; uint8_t pb = 4;} const _winLogo;
struct w_n  {uint16_t x = 190; uint16_t y =  50; uint16_t w = 610; uint16_t h = 190; uint8_t pl = 15; uint8_t pr =  5; uint8_t pt = 0; uint8_t pb = 0;} const _winName;      // station name
struct w_e  {uint16_t x =   0; uint16_t y =  50; uint16_t w = 800; uint16_t h = 190; uint8_t pl =  5; uint8_t pr =  5; uint8_t pt = 0; uint8_t pb = 0;} const _winFName;
struct w_j  {uint16_t x =   0; uint16_t y = 240; uint16_t w = 190; uint16_t h =  50; uint8_t pl =  0; uint8_t pr =  4; uint8_t pt = 0; uint8_t pb = 0;} const _winFileNr;    // e.g. 020/066 in the middle of the logo
struct w_v  {uint16_t x = 200; uint16_t y =  73; uint16_t w = 258; uint16_t h = 144; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winVolBox;    // volumeBox
struct w_a  {uint16_t x =   0; uint16_t y = 290; uint16_t w = 800; uint16_t h =  14; uint8_t pl =  5; uint8_t pr =  5; uint8_t pt = 0; uint8_t pb = 0;} const _winProgbar;   // progressbar
struct w_t  {uint16_t x =   0; uint16_t y = 240; uint16_t w = 800; uint16_t h = 190; uint8_t pl = 15; uint8_t pr =  5; uint8_t pt = 2; uint8_t pb = 2;} const _winTitle;
struct w_c  {uint16_t x =   0; uint16_t y = 240; uint16_t w = 768; uint16_t h = 190; uint8_t pl = 10; uint8_t pr =  5; uint8_t pt = 2; uint8_t pb = 2;} const _winSTitle;    // streamTitle, space for VUmeter
struct w_g  {uint16_t x = 768; uint16_t y = 280; uint16_t w =  32; uint16_t h = 150; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winVUmeter;
struct w_f  {uint16_t x =   0; uint16_t y = 430; uint16_t w = 800; uint16_t h =  50; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winFooter;
struct w_s  {uint16_t x =   0; uint16_t y = 290; uint16_t w =  85; uint16_t h =  50; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winStaNr;
struct w_p  {uint16_t x =  85; uint16_t y = 290; uint16_t w =  87; uint16_t h =  50; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winSleep;
struct w_b  {uint16_t x =   0; uint16_t y = 265; uint16_t w = 800; uint16_t h =  60; uint8_t pl =  5; uint8_t pr =  5; uint8_t pt = 0; uint8_t pb = 0;} const _sdrOvBtns;    // slider over buttons, max width
struct w_o  {uint16_t x =   0; uint16_t y = 345; uint16_t w =  80; uint16_t h =  80; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winButton;
struct w_y  {uint16_t x =   0; uint16_t y =  50; uint16_t w = 800; uint16_t h = 295; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winDigits;    // clock and alarm digits
struct w_w  {uint16_t x =   0; uint16_t y =  50; uint16_t w = 800; uint16_t h = 380; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _winWoHF;      // without Header and Footer
struct w_s1 {uint16_t x = 200; uint16_t y =  53; uint16_t w = 300; uint16_t h =  73; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrHP;        // slider highpass in equalizer
struct w_s2 {uint16_t x = 200; uint16_t y = 126; uint16_t w = 300; uint16_t h =  73; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrBP;        // slider bandpass in equalizer
struct w_s3 {uint16_t x = 200; uint16_t y = 199; uint16_t w = 300; uint16_t h =  73; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrLP;        // slider lowpass in equalizer
struct w_s4 {uint16_t x = 200; uint16_t y = 272; uint16_t w = 300; uint16_t h =  73; uint8_t pl =  0; uint8_t pr =  0; uint8_t pt = 0; uint8_t pb = 0;} const _sdrBAL;       // slider balance in equalizer

// derived structs
struct w_b1 {uint16_t x = 100;                 uint16_t y = _sdrHP.y;  uint16_t w = _sdrHP.w;  uint16_t h = _sdrHP.h; } const _btnHP;
struct w_b2 {uint16_t x = 100;                 uint16_t y = _sdrBP.y;  uint16_t w = _sdrBP.w;  uint16_t h = _sdrBP.h; } const _btnBP;
struct w_b3 {uint16_t x = 100;                 uint16_t y = _sdrLP.y;  uint16_t w = _sdrLP.w;  uint16_t h = _sdrLP.h; } const _btnLP;
struct w_b4 {uint16_t x = 100;                 uint16_t y = _sdrBAL.y; uint16_t w = _sdrBAL.w; uint16_t h = _sdrBAL.h;} const _btnBAL;
struct w_t1 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrHP.y;  uint16_t w = 190;       uint16_t h = _sdrHP.h; } const _txtHP;
struct w_t2 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBP.y;  uint16_t w = 190;       uint16_t h = _sdrBP.h; } const _txtBP;
struct w_t3 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrLP.y;  uint16_t w = 190;       uint16_t h = _sdrLP.h; } const _txtLP;
struct w_t4 {uint16_t x = _sdrHP.x + _sdrHP.w; uint16_t y = _sdrBAL.y; uint16_t w = 190;       uint16_t h = _sdrBAL.h;} const _txtBAL;


uint16_t _sleeptimeXPos[5] = {20, 137, 223, 106};
uint16_t _sleeptimeYPos = 112;
uint16_t _dispWidth   = 800;
uint16_t _dispHeight  = 480;
uint8_t  _BrightnessMin = 25; // slider Brightness left value
const char _tftSize[2] = "l";
//
// clang-format on
#endif // #if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3 || TFT_CONTROLLER == 4 || TFT_CONTROLLER == 5|| TFT_CONTROLLER == 6

// ALL STATE
displayHeader   dispHeader("dispHeader", _headerFontSize); // 0 -> autoSize
displayFooter   dispFooter("dispFooter", _footerFontSize); // 0 -> autoSize
numbersBox      volBox("volBox");
uniList         myList("myList");
// RADIO
button2state    btn_RA_mute("btn_RA_mute");
button1state    btn_RA_prevSta("btn_RA_prevSta"), btn_RA_nextSta("btn_RA_nextSta");
button1state    btn_RA_staList("btn_RA_staList"), btn_RA_player("btn_RA_player"), btn_RA_dlna("btn_RA_dlna"), btn_RA_clock("btn_RA_clock");
button1state    btn_RA_sleep("btn_RA_sleep"), btn_RA_bt("btn_RA_bt");
button1state    btn_RA_off("btn_RA_off"), btn_RA_settings("btn_RA_settings");
pictureBox      pic_RA_logo("pic_RA_logo");
textbox         txt_RA_sTitle("txt_RA_sTitle"), txt_RA_staName("txt_RA_staName"), txt_RA_irNum("txt_RA_irNum");
vuMeter         VUmeter_RA("VUmeter_RA");
slider          sdr_RA_volume("sdr_RA_volume");
// STATIONSLIST
stationsList    lst_RADIO("lst_RADIO");
// PLAYER
button2state    btn_PL_mute("btn_PL_mute"), btn_PL_pause("btn_PL_pause");
button1state    btn_PL_ready("btn_PL_ready"), btn_PL_shuffle("btn_PL_shuffle");
button1state    btn_PL_playAll("btn_PL_playAll"), btn_PL_fileList("btn_PL_fileList"), btn_PL_radio("btn_PL_radio"), btn_PL_cancel("btn_PL_cancel");
button1state    btn_PL_prevFile("btn_PL_prevFile"), btn_PL_nextFile("btn_PL_nextFile"), btn_PL_off("btn_PL_off");
button1state    btn_PL_playNext("btn_PL_playNext"), btn_PL_playPrev("btn_PL_playPrev");
textbox         txt_PL_fName("txt_PL_fName");
textbox         txt_PL_fNumber("txt_PL_fNumber");
slider          sdr_PL_volume("sdr_PL_volume");
pictureBox      pic_PL_logo("pic_PL_logo");
progressbar     pgb_PL_progress("pgb_PL_progress");
// AUDIOFILESLIST
fileList        lst_PLAYER("lst_PLAYER");
// DLNA
button2state    btn_DL_mute("btn_DL_mute"), btn_DL_pause("btn_DL_pause");
button1state    btn_DL_radio("btn_DL_radio"), btn_DL_fileList("btn_DL_fileList"), btn_DL_cancel("btn_DL_cancel");
textbox         txt_DL_fName("txt_DL_fName");
slider          sdr_DL_volume("sdr_DL_volume");
pictureBox      pic_DL_logo("pic_DL_logo");
progressbar     pgb_DL_progress("pgb_DL_progress");
// DLNAITEMSLIST
dlnaList        lst_DLNA("lst_DLNA", &dlna, &_dlnaHistory[0], 10);
// CLOCK
imgClock24      clk_CL_24("clk_CL_24");
imgClock12      clk_CL_12("clk_CL_12");
button2state    btn_CL_mute("btn_CL_mute");
button1state    btn_CL_alarm("btn_CL_alarm"), btn_CL_radio("btn_CL_radio"), btn_CL_off("btn_CL_off");
slider          sdr_CL_volume("sdr_CL_volume");
// ALARMCLOCK
alarmClock      clk_AC_red("clk_AC_red");
button1state    btn_AC_left("btn_AC_left"), btn_AC_right("btn_AC_right"), btn_AC_up("btn_AC_up"), btn_AC_down("btn_AC_down");
button1state    btn_AC_ready("btn_AC_ready");
// RINGING
pictureBox      pic_RI_logo("pic_RI_logo");
imgClock24small clk_RI_24small("clk_RI_24small");
// SETTINGS
pictureBox      pic_SE_logo("pic_SE_logo");
button1state    btn_SE_bright("btn_SE_bright"), btn_SE_equal("btn_SE_equal"), btn_SE_wifi("btn_SE_wifi"), btn_SE_radio("btn_SE_radio");
// BRIGHTNESS
button1state    btn_BR_ready("btn_BR_ready");
pictureBox      pic_BR_logo("pic_BR_logo");
slider          sdr_BR_value("sdr_BR_value");
textbox         txt_BR_value("txt_BR_value");
// SLEEPTIMER
button1state    btn_SL_up("btn_SL_up"), btn_SL_down("btn_SL_down"), btn_SL_ready("btn_SL_ready"), btn_SL_cancel("btn_SL_cancel");
// EQUALIZER
slider          sdr_EQ_lowPass("sdr_E_LP"), sdr_EQ_bandPass("sdr_E_BP"), sdr_EQ_highPass("sdr_E_HP"), sdr_EQ_balance("sdr_E_BAL");
textbox         txt_EQ_lowPass("txt_E_LP"), txt_EQ_bandPass("txt_E_BP"), txt_EQ_highPass("txt_E_HP"), txt_EQ_balance("txt_E_BAL");
button1state    btn_EQ_lowPass("btn_E_LP");
button1state    btn_EQ_bandPass("btn_E_BP"), btn_EQ_highPass("btn_E_HP"), btn_EQ_balance("btn_E_BAL");
button1state    btn_EQ_Radio("btn_EQ_Radio"), btn_EQ_Player("btn_EQ_Player");
button2state    btn_EQ_mute("btn_EQ_mute");
// BLUETOOTH
button2state    btn_BT_pause("btn_BT_pause"), btn_BT_power("btn_BT_power");
button1state    btn_BT_volDown("btn_BT_volDown"), btn_BT_volUp("btn_BT_volUp"), btn_BT_radio("btn_BT_radio"), btn_BT_mode("btn_BT_mode");
pictureBox      pic_BT_mode("pic_BT_mode");
textbox         txt_BT_mode("txt_BT_mode");
textbox         txt_BT_volume("txt_BT_volume");
// IR_SETTINGS
button1state    btn_IR_radio("btn_IR_radio");
// WIFI_SETTINGS
wifiSettings    cls_wifiSettings("wifiSettings", 2);

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     D E F A U L T S E T T I N G S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format off  🟢🟡🔴
boolean defaultsettings(){

    if(!SD_MMC.exists("/ir_buttons.json")){  // if not found ir_buttons.json create a default file
        File file = SD_MMC.open("/ir_buttons.json", "w", true);
        file.write((uint8_t*)ir_buttons_json, sizeof(ir_buttons_json));
        file.close();
    }
    irb.loadButtonsFromJSON("/ir_buttons.json");
    for(uint i = 0; i < _settings.numOfIrButtons; i++) {
    //    log_w("0x%04X,  %s", _settings.irbuttons[i].val, _settings.irbuttons[i].label);
        ir.set_irButtons(i, _settings.irbuttons[i].val);
    }
        ir.set_irAddress(_settings.irbuttons[42].val);
    //    log_w("0x%04X,  %s", _settings.irbuttons[42].val, _settings.irbuttons[42].label);

    if(!SD_MMC.exists("/settings.json")){  // if not found create one
        updateSettings();
    }

    File file2 = SD_MMC.open("/settings.json","r", false);
    char*  jO = x_ps_calloc(2048, 1);
    char* tmp = x_ps_malloc(1024);
    file2.readBytes(jO, 2048);
    _settingsHash = simpleHash(jO);
    file2.close();

    auto parseJson = [&](const char* s) { // lambda, inner function
        int16_t pos1 = 0, pos2 = 0, pos3 = 0;
        pos1 = indexOf(jO, s, 0);
        if(pos1 < 0) {log_e("index %s not found", s); return "0";}
        pos2 = indexOf(jO, ":", pos1) + 1;
        if(jO[pos2] == '\"') pos3 = indexOf(jO, "\"", pos2 + 1) + 1;
        else pos3 = indexOf(jO, ",", pos2);
        if(pos3 < 0) pos3 = find_first_of(jO, "}\n", pos2);
        if(jO[pos2] == '\"'){pos2++; pos3--;}  // remove \" embraced strings
        strncpy(tmp, jO + pos2, pos3 - pos2);
        tmp[pos3 - pos2] = '\0';
        return (const char*)tmp;
    };

    auto computeMinuteOfTheDay = [&](const char* s){
        if(!s) return 0;
        int h = atoi(s);
        int m = atoi(s + 3);
        return h * 60 + m;
    };

    x_ps_free(&_settings.lastconnectedhost);
    x_ps_free(&_settings.lastconnectedfile);

    _cur_volume                 = atoi(   parseJson("\"volume\":"));
    _volumeSteps                = atoi(   parseJson("\"volumeSteps\":"));
    _ringVolume                 = atoi(   parseJson("\"ringVolume\":"));
    _volumeAfterAlarm           = atoi(   parseJson("\"volumeAfterAlarm\":"));
    _BTvolume                   = atoi(   parseJson("\"BTvolume\":"));
    _f_BTpower                  = (strcmp(parseJson("\"BTpower\":"), "true") == 0) ? 1 : 0;
    _btEmitterMode              = ((strcmp(parseJson("\"BTmode\":"), "TX") == 0) ? "TX" : "RX");
    _alarmtime[0]               = computeMinuteOfTheDay(parseJson("\"alarmtime_sun\":"));
    _alarmtime[1]               = computeMinuteOfTheDay(parseJson("\"alarmtime_mon\":"));
    _alarmtime[2]               = computeMinuteOfTheDay(parseJson("\"alarmtime_tue\":"));
    _alarmtime[3]               = computeMinuteOfTheDay(parseJson("\"alarmtime_wed\":"));
    _alarmtime[4]               = computeMinuteOfTheDay(parseJson("\"alarmtime_thu\":"));
    _alarmtime[5]               = computeMinuteOfTheDay(parseJson("\"alarmtime_fri\":"));
    _alarmtime[6]               = computeMinuteOfTheDay(parseJson("\"alarmtime_sat\":"));
    _alarmdays                  = atoi(       parseJson("\"alarm_weekdays\":"));
    _f_timeAnnouncement         = (strcmp(    parseJson("\"timeAnnouncing\":"), "true") == 0) ? 1 : 0;
    strcpy(_timeSpeechLang,                   parseJson("\"timeSpeechLang\":"));
    _f_mute                     = (strcmp(    parseJson("\"mute\":"), "true") == 0) ? 1 : 0;
    _brightness                 = atoi(       parseJson("\"brightness\":"));
    _sleeptime                  = atoi(       parseJson("\"sleeptime\":"));
    _cur_station                = atoi(       parseJson("\"station\":"));
    _toneLP                     = atoi(       parseJson("\"toneLP\":"));
    _toneBP                     = atoi(       parseJson("\"toneBP\":"));
    _toneHP                     = atoi(       parseJson("\"toneHP\":"));
    _toneBAL                    = atoi(       parseJson("\"balance\":"));
    _timeFormat                 = atoi(       parseJson("\"timeFormat\":"));
    _TZName                     =             parseJson("\"Timezone_Name\":");
    _TZString                   =             parseJson("\"Timezone_String\":");
    _settings.lastconnectedhost = x_ps_strdup(parseJson("\"lastconnectedhost\":"));
    _settings.lastconnectedfile = x_ps_strdup(parseJson("\"lastconnectedfile\":"));
    _sleepMode                  = atoi(       parseJson("\"sleepMode\":"));
    _state                      = atoi(       parseJson("\"state\":"));

    // set some items ---------------------------------------------------------------------------------------------
    if(!startsWith(_settings.lastconnectedfile, "/")) {x_ps_free(&_settings.lastconnectedfile); _settings.lastconnectedfile = x_ps_strdup("/audiofiles/");} // guard
    _SD_content.setLastConnectedFile(_settings.lastconnectedfile);
    x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = x_ps_strdup(_SD_content.getLastConnectedFolder());
    x_ps_free(&_cur_AudioFileName); _cur_AudioFileName = x_ps_strdup(_SD_content.getLastConnectedFileName());
    _cur_AudioFileNr = _SD_content.getPosByFileName(_cur_AudioFileName);
    // ------------------------------------------------------------------------------------------------------------
    x_ps_free(&jO);
    x_ps_free(&tmp);

    if(!SD_MMC.exists("/stations.json")){  // if not found create one
        File file1 = SD_MMC.open("/stations.json","w", true);
        file1.write((uint8_t*)stations_json, sizeof(stations_json) -1); // without termination
        file1.close();
    }
    staMgnt.updateStationsList();
    return true;
}
// clang-format on  🟢🟡🔴

// clang-format off  🟢🟡🔴
void updateSettings(){
    if(!_settings.lastconnectedhost) _settings.lastconnectedhost= strdup("");
    if(!_settings.lastconnectedfile) _settings.lastconnectedfile= strdup("/audiofiles/");
    char*  jO = x_ps_malloc(2048 + strlen(_settings.lastconnectedhost)); // JSON Object
    char tmp[40 + strlen(_settings.lastconnectedhost)];
    strcpy(jO,   "{\n");
    sprintf(tmp, "  \"volume\":%i", _cur_volume);                                                   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"volumeSteps\":%i", _volumeSteps);                                          strcat(jO, tmp);
    sprintf(tmp, ",\n  \"ringVolume\":%i", _ringVolume);                                            strcat(jO, tmp);
    sprintf(tmp, ",\n  \"volumeAfterAlarm\":%i", _volumeAfterAlarm);                                strcat(jO, tmp);
    sprintf(tmp, ",\n  \"BTvolume\":%i", _BTvolume);                                                strcat(jO, tmp);
    strcat(jO,   ",\n  \"BTpower\":"); (_f_BTpower == true) ?                                       strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    sprintf(tmp, ",\n  \"BTmode\":\"%s\"", bt_emitter.getMode());                                   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_sun\":\"%02d:%02d\"", _alarmtime[0] / 60, _alarmtime[0] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_mon\":\"%02d:%02d\"", _alarmtime[1] / 60, _alarmtime[1] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_tue\":\"%02d:%02d\"", _alarmtime[2] / 60, _alarmtime[2] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_wed\":\"%02d:%02d\"", _alarmtime[3] / 60, _alarmtime[3] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_thu\":\"%02d:%02d\"", _alarmtime[4] / 60, _alarmtime[4] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_fri\":\"%02d:%02d\"", _alarmtime[5] / 60, _alarmtime[5] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarmtime_sat\":\"%02d:%02d\"", _alarmtime[6] / 60, _alarmtime[6] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\n  \"alarm_weekdays\":%i", _alarmdays);                                         strcat(jO, tmp);
    strcat(jO,   ",\n  \"timeAnnouncing\":"); (_f_timeAnnouncement == true) ?                       strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    sprintf(tmp, ",\n  \"timeSpeechLang\":\"%s\"", _timeSpeechLang);                                strcat(jO, tmp);
    strcat(jO,   ",\n  \"mute\":");           (_f_mute == true)             ?                       strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    sprintf(tmp, ",\n  \"brightness\":%i", _brightness);                                            strcat(jO, tmp);
    sprintf(tmp, ",\n  \"sleeptime\":%i", _sleeptime);                                              strcat(jO, tmp);
    sprintf(tmp, ",\n  \"lastconnectedhost\":\"%s\"", _settings.lastconnectedhost);                 strcat(jO, tmp);
    sprintf(tmp, ",\n  \"lastconnectedfile\":\"%s\"", _settings.lastconnectedfile);                 strcat(jO, tmp);
    sprintf(tmp, ",\n  \"station\":%i", _cur_station);                                              strcat(jO, tmp);
    sprintf(tmp, ",\n  \"Timezone_Name\":\"%s\"", _TZName.c_str());                                 strcat(jO, tmp);
    sprintf(tmp, ",\n  \"Timezone_String\":\"%s\"", _TZString.c_str());                             strcat(jO, tmp);
    sprintf(tmp, ",\n  \"toneLP\":%i", _toneLP);                                                    strcat(jO, tmp);
    sprintf(tmp, ",\n  \"toneBP\":%i", _toneBP);                                                    strcat(jO, tmp);
    sprintf(tmp, ",\n  \"toneHP\":%i", _toneHP);                                                    strcat(jO, tmp);
    sprintf(tmp, ",\n  \"balance\":%i", _toneBAL);                                                  strcat(jO, tmp);
    sprintf(tmp, ",\n  \"timeFormat\":%i", _timeFormat);                                            strcat(jO, tmp);
    sprintf(tmp, ",\n  \"state\":%i", _state);                                                      strcat(jO, tmp);
    sprintf(tmp, ",\n  \"sleepMode\":%i\n}", _sleepMode);                                           strcat(jO, tmp);

    if(_settingsHash != simpleHash(jO)) {
        File file = SD_MMC.open("/settings.json", "w", false);
        if(!file) {
            log_e("file \"settings.json\" not found");
            return;
        }
        file.print(jO);
        _settingsHash = simpleHash(jO);
    }
    x_ps_free(&jO);
}
// clang-format on

/*****************************************************************************************************************************************************
 *                                                    F I L E   E X P L O R E R                                                                      *
 *****************************************************************************************************************************************************/
// Sends a list of the content of a directory as JSON file
const char* SD_stringifyDirContent(String path) {
    uint16_t JSONstrLength = 0;
    uint8_t  isDir = 0;
    uint16_t fnLen = 0; // length of file mame
    uint8_t  fsLen = 0; // length of file size
    x_ps_free(&_JSONstr);
    if(!_SD_content.listFilesInDir(path.c_str(), false, false)) return "[]"; // if success: result will be in _SD_content
    if(psramFound()) { _JSONstr = (char*)ps_malloc(2); }
    else { _JSONstr = (char*)malloc(2); }
    JSONstrLength += 2;
    memcpy(_JSONstr, "[\0", 2);
    if(!_SD_content.getSize()) return "[]"; // empty?

    for(int i = 0; i < _SD_content.getSize(); i++) { // build a JSON string in PSRAM, e.g. [{"name":"m","dir":true},{"name":"s","dir":true}]
        const char* fn = _SD_content.getColouredSStringByIndex(i);
        if(startsWith(fn, "/.")) continue;    // ignore hidden folders
        int16_t idx = indexOf(fn, "\033", 1); // idx >0 we have size (after ANSI ESC SEQUENCE)
        if(idx > 0) {
            isDir = 0;
            fnLen = idx;
            fsLen = strlen(fn) - (idx + 6);          // "033[33m"
            JSONstrLength += fnLen + 24 + 8 + fsLen; // {"name":"test.mp3","dir":false,"size":"3421"}
        }
        else {
            isDir = 1;
            fnLen = strlen(fn);
            fsLen = 0;
            JSONstrLength += fnLen + 23 + 11;
        }
        if(psramFound()) { _JSONstr = (char*)ps_realloc(_JSONstr, JSONstrLength); }
        else { _JSONstr = (char*)realloc(_JSONstr, JSONstrLength); }

        strcat(_JSONstr, "{\"name\":\"");
        strncat(_JSONstr, fn, fnLen);
        strcat(_JSONstr, "\",\"dir\":");
        if(isDir) { strcat(_JSONstr, "true"); }
        else { strcat(_JSONstr, "false"); }
        if(!isDir) {
            strcat(_JSONstr, ",\"size\":");
            strncat(_JSONstr, fn + idx + 6, fsLen);
        }
        else { strcat(_JSONstr, ",\"size\": \"\""); }
        strcat(_JSONstr, "},");
    }
    _JSONstr[JSONstrLength - 2] = ']'; // replace comma by square bracket close
    return _JSONstr;
}

/*****************************************************************************************************************************************************
 *                                                    T F T   B R I G H T N E S S                                                                    *
 *****************************************************************************************************************************************************/
void setTFTbrightness(uint8_t duty) { // duty 0...100 (min...max)
    if(TFT_BL == -1) return;
    uint8_t d = round((double)duty * 2.55); // #186
    ledcWrite(TFT_BL, d);
}

int16_t getTFTbrightness() { // duty 0...100 (min...max)
    if(TFT_BL == -1) return -1;
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
    while(str[p1]) {
        if((str[p1] == '%') && ((a = str[p1 + 1]) && (b = str[p1 + 2])) && (isxdigit(a) && isxdigit(b))) {
            if(a >= 'a') a -= 'a' - 'A';
            if(a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if(b >= 'a') b -= 'a' - 'A';
            if(b >= 'A') b -= ('A' - 10);
            else b -= '0';
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
    static uint16_t ms100 = 0;
    _f_100ms = true;
    ms100 ++;
    if(!(ms100 % 10))   {
        _f_1sec  = true;
        _time_s = rtc.gettime_s();
        if(endsWith(_time_s, "59:53"))  _f_timeSpeech = true;
    }
    if(!(ms100 % 100))  _f_10sec = true;
    if(!(ms100 % 600)) {_f_1min  = true; ms100 = 0;}
}
// clang-format on

/*****************************************************************************************************************************************************
 *                                                               D I S P L A Y                                                                       *
 *****************************************************************************************************************************************************/

// clang-format off
inline void bgColorHeader()             {tft.fillRect(_winHeader.x,    _winHeader.y,    _winHeader.w,    _winHeader.h,   TFT_BLACK);}
inline void bgColorLogo()               {tft.fillRect(_winLogo.x,      _winLogo.y,      _winLogo.w,      _winLogo.h,     TFT_BLACK);}
inline void bgColorStationName()        {tft.fillRect(_winName.x,      _winName.y,      _winName.w,      _winName.h,     TFT_BLACK);}
inline void bgColorLogoAndStationname() {tft.fillRect(_winFName.x,     _winFName.y,     _winFName.w,     _winFName.h,    TFT_BLACK);}
inline void bgColorTitle()              {tft.fillRect(_winTitle.x,     _winTitle.y,     _winTitle.w,     _winTitle.h,    TFT_BLACK);} // incl. VUmeter
inline void bgColorStreamTitle()        {tft.fillRect(_winSTitle.x,    _winSTitle.y,    _winSTitle.w,    _winSTitle.h,   TFT_BLACK);} // without VUmeter
inline void bgColorWithOutHeaderFooter(){tft.fillRect(_winWoHF.x,      _winWoHF.y,      _winWoHF.w,      _winWoHF.h,     TFT_BLACK);}
inline void bgColorFooter()             {tft.fillRect(_winFooter.x,    _winFooter.y,    _winFooter.w,    _winFooter.h,   TFT_BLACK);}
inline void bgColorStaNr()              {tft.fillRect(_winStaNr.x,     _winStaNr.y,     _winStaNr.w,     _winStaNr.h,    TFT_BLACK);}
inline void bgColorSleep()              {tft.fillRect(_winSleep.x,     _winSleep.y,     _winSleep.w,     _winSleep.h,    TFT_BLACK);}
inline void bgColorDigits()             {tft.fillRect(_winDigits.x,    _winDigits.y,    _winDigits.w,    _winDigits.h,   TFT_BLACK);}
inline void bgColorButtonBar()          {tft.fillRect( 0,              _winButton.y,    _dispWidth,      _winButton.h,   TFT_BLACK);}
inline void bgColorAll()                {tft.fillScreen(TFT_BLACK);}                      // y   0...239


inline void clearHeader()             {tft.copyFramebuffer(1, 0, _winHeader.x,    _winHeader.y,    _winHeader.w,    _winHeader.h);}
inline void clearLogo()               {tft.copyFramebuffer(1, 0, _winLogo.x,      _winLogo.y,      _winLogo.w,      _winLogo.h);}
inline void clearStationName()        {tft.copyFramebuffer(1, 0, _winName.x,      _winName.y,      _winName.w,      _winName.h);}
inline void clearLogoAndStationname() {tft.copyFramebuffer(1, 0, _winFName.x,     _winFName.y,     _winFName.w,     _winFName.h);}
inline void clearTitle()              {tft.copyFramebuffer(1, 0, _winTitle.x,     _winTitle.y,     _winTitle.w,     _winTitle.h);} // incl. VUmeter
inline void clearStreamTitle()        {tft.copyFramebuffer(1, 0, _winSTitle.x,    _winSTitle.y,    _winSTitle.w,    _winSTitle.h);} // without VUmeter
inline void clearWithOutHeaderFooter(){tft.copyFramebuffer(1, 0, _winWoHF.x,       _winWoHF.y,      _winWoHF.w,     _winWoHF.h);}
inline void clearFooter()             {tft.copyFramebuffer(1, 0, _winFooter.x,    _winFooter.y,    _winFooter.w,    _winFooter.h);}
inline void clearStaNr()              {tft.copyFramebuffer(1, 0, _winStaNr.x,     _winStaNr.y,     _winStaNr.w,     _winStaNr.h);}
inline void clearSleep()              {tft.copyFramebuffer(1, 0, _winSleep.x,     _winSleep.y,     _winSleep.w,     _winSleep.h);}
inline void clearDigits()             {tft.copyFramebuffer(1, 0, _winDigits.x,    _winDigits.y,    _winDigits.w,    _winDigits.h);}
inline void clearButtonBar()          {tft.copyFramebuffer(1, 0,  0,              _winButton.y,    _dispWidth,      _winButton.h);}
inline void clearAll()                {tft.copyFramebuffer(1, 0,  0,               0,              _dispWidth,      _dispHeight);}



// clang-format on

inline uint16_t txtlen(String str) {
    uint16_t len = 0;
    for(int32_t i = 0; i < str.length(); i++)
        if(str[i] <= 0xC2) len++;
    return len;
}

void showStreamTitle(const char* streamtitle) {
    if(_f_sleeping) return;

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
    char* SN_utf8 = NULL;
    static char* old_SN_utf8 = strdup("");
    if(force){
        if(old_SN_utf8){x_ps_free(&old_SN_utf8); old_SN_utf8 = strdup("");}
    }

    if(_cur_station) {
    //    log_w("showLogoAndStationName: %s", staMgnt.getStationName(_cur_station));
        SN_utf8 = x_ps_calloc(strlen(staMgnt.getStationName(_cur_station)) + 12, 1);
        memcpy(SN_utf8, staMgnt.getStationName(_cur_station), strlen(staMgnt.getStationName(_cur_station)) + 1);
        SerialPrintfln("Country: ... " ANSI_ESC_GREEN "%s", staMgnt.getStationCountry(_cur_station));
    }
    else {
        if(!_stationName_air) _stationName_air = strdup("");
        SN_utf8 = x_ps_calloc(strlen(_stationName_air) + 12, 1);
        memcpy(SN_utf8, _stationName_air,  strlen(_stationName_air) + 1);
    }
    trim(SN_utf8);
    if(strcmp(old_SN_utf8, SN_utf8) == 0) {goto exit;}
    x_ps_free(&old_SN_utf8);
    old_SN_utf8 = x_ps_strdup(SN_utf8);
    txt_RA_staName.setTextColor(TFT_CYAN);
    txt_RA_staName.setText(SN_utf8);
    txt_RA_staName.show(true, false);

    memmove(SN_utf8  + 6, SN_utf8, strlen(SN_utf8) + 1);
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
    if(state == RADIO) {
        if(endsWith(_stationURL, "m3u8")) logo = "/common/" + (String) "M3U8" + ".png";
        else logo = "/common/" + (String)codecname[_cur_Codec] + ".png";
        pic_RA_logo.setPicturePath(logo.c_str());
        pic_RA_logo.setAlternativPicturePath("/common/unknown.png");
        pic_RA_logo.show(true, false);
        webSrv.send("stationLogo=", logo);
        return;
    }
    else if(state == DLNA) {
        logo = "/common/DLNA.jpg";
        pic_DL_logo.setPicturePath(logo.c_str());
        pic_DL_logo.setAlternativPicturePath("/common/unknown.png");
        pic_DL_logo.show(true, false);
        webSrv.send("stationLogo=", logo);
        return;
    }
    if(state == PLAYER) { // _state PLAYER
        if(_cur_Codec == 0) logo = "/common/AudioPlayer.png";
        else if(_playerSubMenue == 0) logo = "/common/AudioPlayer.png";
        else logo = "/common/" + (String)codecname[_cur_Codec] + ".png";
        pic_PL_logo.setPicturePath(logo.c_str());
        pic_PL_logo.setAlternativPicturePath("/common/unknown.png");
        pic_PL_logo.show(true, false);
        return;
    }
    if(state == SETTINGS) {
        logo = "/common/Settings.png";
        pic_SE_logo.setPicturePath(logo.c_str());
        pic_SE_logo.setAlternativPicturePath("/common/unknown.png");
        pic_SE_logo.show(true, false);
        return;
    }
    if(state == RINGING) {
        logo = "/common/Alarm.png";
        pic_RI_logo.setPicturePath(logo.c_str());
        pic_RI_logo.setAlternativPicturePath("/common/unknown.png");
        pic_RI_logo.show(true, false);
        return;
    }
}

void showFileName(const char* fname) {
    if(!fname) return;
    txt_PL_fName.setTextColor(TFT_CYAN);
    txt_PL_fName.writeText(fname);
}

void showPlsFileNumber() {
    txt_PL_fNumber.setTextColor(TFT_ORANGE);
    char buf[15];
    sprintf(buf, "%03u/%03u", _plsCurPos, _PLS_content.size());
    txt_PL_fNumber.writeText(buf);
}

void showAudioFileNumber() {
    txt_PL_fNumber.setTextColor(TFT_ORANGE);
    char buf[15];
    sprintf(buf, "%03u/%03u", _cur_AudioFileNr + 1, _SD_content.getSize());
    txt_PL_fNumber.writeText(buf);
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
            else strcat(path, "dsrt.jpg");
        }
        else {
            strncat(path, (tmp + i), 1);
            if(!_sleeptime) strcat(path, "sgn.jpg");
            else strcat(path, "srt.jpg");
        }
        drawImage(path, _sleeptimeXPos[i], _sleeptimeYPos);
    }
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth, uint16_t maxHeigth) {
    const char* scImg = scaleImage(path);
    if(!SD_MMC.exists(scImg)) {
        if(indexOf(scImg, "/.", 0) > 0) return false; // empty filename
        SerialPrintfln("AUDIO_info:  " ANSI_ESC_RED "file \"%s\" not found", scImg);
        return false;
    }
    if(endsWith(scImg, "bmp")) { return tft.drawBmpFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth, 1.0); }
    if(endsWith(scImg, "jpg")) { return tft.drawJpgFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth); }
    if(endsWith(scImg, "gif")) { return tft.drawGifFile(SD_MMC, scImg, posX, posY, 0); }
    if(endsWith(scImg, "png")) { return tft.drawPngFile(SD_MMC, scImg, posX, posY); }

    SerialPrintfln(ANSI_ESC_RED "the file \"%s\" contains neither a bmp, a gif nor a jpg graphic", scImg);
    return false; // neither jpg nor bmp
}
/*****************************************************************************************************************************************************
 *                                                   H A N D L E  A U D I O F I L E                                                                  *
 *****************************************************************************************************************************************************/

boolean isAudio(File file) {
    if(endsWith(file.name(), ".mp3") || endsWith(file.name(), ".aac") || endsWith(file.name(), ".m4a") || endsWith(file.name(), ".wav") || endsWith(file.name(), ".flac") ||
       endsWith(file.name(), ".opus") || endsWith(file.name(), ".ogg")) {
        return true;
    }
    return false;
}

boolean isAudio(const char* path) {
    if(endsWith(path, ".mp3") || endsWith(path, ".aac") || endsWith(path, ".m4a") || endsWith(path, ".wav") || endsWith(path, ".flac") || endsWith(path, ".opus") || endsWith(path, ".ogg")) {
        return true;
    }
    return false;
}

boolean isPlaylist(File file) {
    if(endsWith(file.name(), ".m3u")) { return true; }
    return false;
}

/*****************************************************************************************************************************************************
 *                                                                     P L A Y L I S T                                                               *
 *****************************************************************************************************************************************************/

bool preparePlaylistFromFile(const char* path) {  // *.m3u
    File playlistFile = SD_MMC.open(path);
    if(!playlistFile) {
        log_e("playlistfile path not found");
        return false;
    }

    if(playlistFile.size() > 1048576) {
        log_e("Playlist too big, size > 1MB");
        playlistFile.close();
        return false;
    }
    x_ps_free(&_playlistPath);
    vector_clear_and_shrink(_PLS_content); // clear _PLS_content first
    char* buff1 = x_ps_malloc(2024);
    char* buff2 = x_ps_malloc(1048);
    if(!buff1 || !buff2) {
        log_e("oom");
        playlistFile.close();
        return false;
    }
    size_t bytesRead = 0;
    bool   f_EXTINF_seen = false;

    while(playlistFile.available() > 0) {
        bytesRead = playlistFile.readBytesUntil('\n', buff1, 1024);
        if(bytesRead < 5) continue; // line is # or space or nothing, smallest filename "1.mp3" < 5
        buff1[bytesRead] = '\0';
        trim(buff1);
        if(startsWith(buff1, "#EXTM3U")) continue;
        if(startsWith(buff1, "#EXTINF:")) { // #EXTINF:8,logo-1.mp3
            strcpy(buff2, buff1 + 8);
            f_EXTINF_seen = true;
            continue;
        }
        if(startsWith(buff1, "#")) continue; // all other lines
        if(f_EXTINF_seen) {
            f_EXTINF_seen = false;
            strcat(buff1, "\n");
            strcat(buff1, buff2);
        }
        _PLS_content.push_back(x_ps_strdup((const char*)buff1));
    }
    _playlistPath = x_ps_strdup(playlistFile.path());
    int idx = lastIndexOf((const char*)_playlistPath, '/');
    if(idx < 0) log_e("wrong playlist path");
    _playlistPath[idx] = '\0';
    playlistFile.close();
    x_ps_free(&buff1);
    x_ps_free(&buff2);
    return true;
}
//____________________________________________________________________________________________________________________________________________________

bool preparePlaylistFromSDFolder(const char* path) { // all files within a SD folder
    if(!SD_MMC.exists(path)) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s not exist", path);
        return false;
    }
    File folder = SD_MMC.open(path);
    if(!folder.isDirectory()) {
        SerialPrintfln(ANSI_ESC_RED "SD_MMC/%s is not a directory", path);
        folder.close();
        return false;
    }
    vector_clear_and_shrink(_PLS_content); // clear _PLS_content first

    while(true) { // get content
        File file = folder.openNextFile();
        if(!file) break;
        if(file.isDirectory()) continue;
        if(isAudio(file)) { _PLS_content.push_back(x_ps_strdup((const char*)file.path())); }
        file.close();
    }
    folder.close();

    for(int i = 0; i < _PLS_content.size(); i++) {
        if(_PLS_content[i][0] == 2) { // remove ASCII 2
            memcpy(_PLS_content[i], _PLS_content[i] + 1, strlen(_PLS_content[i]));
        }
    }

    if(_f_shuffle) sortPlayListRandom();
    else sortPlayListAlphabetical();

    return true;
}
//____________________________________________________________________________________________________________________________________________________
bool preparePlaylistFromDLNAFolder(){
    vector_clear_and_shrink(_PLS_content); // clear _PLS_content first
    DLNA_Client::srvContent_t foldercontent = dlna.getBrowseResult();
    for ( int i=0; i<foldercontent.size; i++) {
        // log_i( "%d : (%d) %s %s -- %s",i, foldercontent.isAudio[i], foldercontent.itemURL[i], foldercontent.title[i], foldercontent.duration[i]);
        if(!foldercontent.isAudio[i]) continue;
        uint16_t len = strlen((const char*)foldercontent.itemURL[i]) + strlen((const char*)foldercontent.title[i]) + strlen((const char*)foldercontent.duration[i]) + 3;
        // log_i("malloc with size %d %d %d %d",len, strlen((const char*)foldercontent.itemURL[i]), strlen((const char*)foldercontent.title[i]), strlen((const char*)foldercontent.duration[i]));
        char* itstr = x_ps_malloc( len );
        strcpy( itstr, (const char*)foldercontent.itemURL[i]);
        strcat( itstr, "\n");
        strcat( itstr, (const char*)foldercontent.duration[i]);
        strcat( itstr, ",");
        strcat( itstr, (const char*)foldercontent.title[i]);
        log_i("pushing to playlist : %s",itstr);
        _PLS_content.push_back(itstr);
        //_PLS_content.push_back(x_ps_strdup((const char*)foldercontent.itemURL[i]));
    }
    if(!_PLS_content.size()) return false;
    log_i("pls length %i", _PLS_content.size());
    return true;
}
//____________________________________________________________________________________________________________________________________________________

void sortPlayListAlphabetical() {
    for(int i = 0; i < _PLS_content.size(); i++) { // easy bubble sort
        for(int j = 1; j < _PLS_content.size(); j++) {
            if(strcmp(_PLS_content[j - 1], _PLS_content[i]) > 0) { swap(_PLS_content[i], _PLS_content[j - 1]); }
        }
    }
}
//____________________________________________________________________________________________________________________________________________________

void sortPlayListRandom() {
    for(int i = 0; i < _PLS_content.size(); i++) { // easy bubble sort
        uint16_t randIndex = random(0, _PLS_content.size());
        swap(_PLS_content[i], _PLS_content[randIndex]); // swapping the values
    }
}
//____________________________________________________________________________________________________________________________________________________

void processPlaylist(boolean first) {

    char *playFile = NULL, *duration = NULL, *title = NULL, *plsContent = NULL, *plsExtension = NULL;
    bool f_hasTitle= false, f_isURL = false, f_isRoot = false, f_isConnected = false;

    if(_PLS_content.size() == 0) {log_e("playlist is empty"); return;}      // guard
    if(_plsCurPos == _PLS_content.size()) goto exit;                        // end of playlist
    if(first) {_plsCurPos = 0; _f_playlistEnabled = true;}                  // reset before start

    if(indexOf(_PLS_content[_plsCurPos], "\n", 0) > 0) f_hasTitle = true;   // has additional infos: duration, title
    // now read from vector _PLS_content
    if(startsWith(_PLS_content[_plsCurPos], "http")) f_isURL = true;        // is web file
    if(startsWith(_PLS_content[_plsCurPos], "file://")) f_isRoot = true;    // SD file from root
    if(f_hasTitle){
        int idx = indexOf(_PLS_content[_plsCurPos], "\n", 0);
        plsContent = x_ps_strndup(_PLS_content[_plsCurPos], idx);
        plsExtension = x_ps_strdup(_PLS_content[_plsCurPos] + idx + 1);
    }
    else{
        plsContent = x_ps_strdup(_PLS_content[_plsCurPos]);
    }

    _cur_Codec = 0;
    if(f_isURL){ // is web file
        playFile = x_ps_strdup(plsContent);
        connecttohost(playFile);
        f_isConnected = _f_isWebConnected;
    }
    else{ // is local file
        if(f_isRoot){
            playFile = x_ps_strdup(plsContent + 7); // skip "file://"
            urldecode(playFile);
            f_isConnected =_f_isFSConnected;
        }
        else{ // local file, not root
            urldecode(plsContent);
            if(_playlistPath) { // path of m3u file
                playFile = x_ps_malloc(strlen(_playlistPath) + strlen(plsContent) + 5);
                strcpy(playFile, _playlistPath);
                if(plsContent[0] != '/') strcat(playFile, "/");
                strcat(playFile, plsContent);
            }
            else { // have no playlistpath
                playFile = x_ps_malloc(strlen(plsContent) + 5);
                strcpy(playFile, plsContent);
            }
        }
        connecttoFS("SD_MMC", playFile);
        f_isConnected =_f_isFSConnected;
    }
    if(f_isConnected){
        SerialPrintflnCut("Playlist:    ", ANSI_ESC_YELLOW, playFile);
        webSrv.send("SD_playFile=", playFile);
        _playerSubMenue = 1;
        changeState(PLAYER);
        if(f_hasTitle){
            int16_t idx1 = indexOf(plsExtension, ",", 0); // "18,logo-1.mp3"
            duration = x_ps_strndup(plsExtension, idx1);
            title = x_ps_strdup(plsExtension + idx1 +1);
            SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "Title: %s", title);
            showFileName(title);
        }
    }
    x_ps_free(&playFile);
    x_ps_free(&duration);
    x_ps_free(&title);
    x_ps_free(&plsContent);
    x_ps_free(&plsExtension);
    _plsCurPos++;
    showPlsFileNumber();
    return;

exit:
    SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "end of playlist");
    webSrv.send("SD_playFile=", "end of playlist");
    _f_playlistEnabled = false;
    _playerSubMenue = 0;
    changeState(PLAYER);
    _plsCurPos = 0;
    x_ps_free(&playFile);
    x_ps_free(&duration);
    x_ps_free(&title);
    x_ps_free(&plsContent);
    x_ps_free(&plsExtension);
    x_ps_free(&_playlistPath);
    return;
}
/*****************************************************************************************************************************************************
 *                                         C O N N E C T   TO   W I F I     /     A C C E S S P O I N T                                              *
 *****************************************************************************************************************************************************/
bool connectToWiFi() {

    char* line = x_ps_malloc(512);
    if(!line) { log_e("oom"); return false;}

    // create nvs entries if they do not exist
    if(!pref.isKey("wifiStr0")) pref.putString("wifiStr0", ""); // SSID + \t + PW
    if(!pref.isKey("wifiStr1")) pref.putString("wifiStr1", "");
    if(!pref.isKey("wifiStr2")) pref.putString("wifiStr2", "");
    if(!pref.isKey("wifiStr3")) pref.putString("wifiStr3", "");
    if(!pref.isKey("wifiStr4")) pref.putString("wifiStr4", "");
    if(!pref.isKey("wifiStr5")) pref.putString("wifiStr5", "");

    const char* SSID = _SSID;
    const char* PW = _PW;
    line[0] = '\0';
    strcpy(line, SSID); strcat(line, "\t"); strcat(line, PW);
    pref.putString("wifiStr0", line);

    for(int i = 0; i < 6; i++) {
        line[0] = '\0'; // Move this line outside the switch statement
        switch (i) {
            case 0: strcpy(line,pref.getString("wifiStr0").c_str()); break;
            case 1: strcpy(line,pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line,pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line,pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line,pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line,pref.getString("wifiStr5").c_str()); break;
        }
        if(strlen(line) < 5) continue;    // line is empty
        int pos = indexOf(line, "\t", 0); // find first tab
        if(pos < 0) continue;             // no tab found
        line[pos] = '\0';                 // terminate ssid
        char* ssid = line;                // ssid is the first part
        char* pw = line + pos + 1;        // password is the second part
        WiFi.mode(WIFI_STA);
        wifiMulti.addAP(ssid, pw); // SSID and PW in code"
        size_t offset = 0;
        size_t pwlen = strlen(pw);
        size_t dot_len = strlen(emoji.blueCircle); // = 4
        size_t buf_size = pwlen * dot_len + 1; // +1 für '\0'
        char pass[buf_size]; for(size_t j = 0; j < pwlen; j++) {memcpy(pass + offset, emoji.blueCircle, dot_len); offset += dot_len;}
        pass[offset] = '\0'; // Zero-terminate the string
        SerialPrintfln("WiFI_info:   add credentials: " ANSI_ESC_CYAN "%s - %s" ANSI_ESC_YELLOW " [%s:%d]", ssid, pass, __FILENAME__, __LINE__);
    }

    wifiMulti.setStrictMode(true);
    WiFi.mode(WIFI_STA);
    wifiMulti.run();
    SerialPrintfln("WiFI_info:   Connecting WiFi...");
    if(WiFi.isConnected()) {
        WiFi.setAutoReconnect(true);
        if(WIFI_TX_POWER >= 2 && WIFI_TX_POWER <= 21) WiFi.setTxPower((wifi_power_t)(WIFI_TX_POWER * 4));
        return true;
    }
    else {
        WiFi.mode(WIFI_MODE_NULL);
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "WiFi credentials are not correct");
        return false; // can't connect to any network
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void setWiFiCredentials(const char* ssid, const char* password) {
    if(!ssid || !password) return;
    if(strlen(ssid) < 5) return; // min length

    log_e("ssid %s pw %s", ssid, password);

    char* line = x_ps_malloc(512);
    int i = 0, state = 0;
    if(!line) { log_e("oom"); goto exit; }

    for(i = 0; i < 6; i++) {
        line[0] = '\0'; // Move this line outside the switch statement
        switch (i) {
            case 0: strcpy(line,pref.getString("wifiStr0").c_str()); break;
            case 1: strcpy(line,pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line,pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line,pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line,pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line,pref.getString("wifiStr5").c_str()); break;
        }
        if(startsWith(line, ssid) && line[strlen(ssid)] == '\t') { // ssid found, update password
            line[0] = '\0'; // clear line
            strcat(line, ssid);
            strcat(line, "\t");
            strcat(line, password);
            if(i == 0) {log_e("password can't changed, is hard coded"); state = 0;goto exit;}
            if(i == 1) {pref.putString("wifiStr1", line); state = 1; goto exit;}
            if(i == 2) {pref.putString("wifiStr2", line); state = 1; goto exit;}
            if(i == 3) {pref.putString("wifiStr3", line); state = 1; goto exit;}
            if(i == 4) {pref.putString("wifiStr4", line); state = 1; goto exit;}
            if(i == 5) {pref.putString("wifiStr5", line); state = 1; goto exit;}
        }
    }
    for(i = 1; i < 6; i++) {
        line[0] = '\0';
        switch (i) {
            case 1: strcpy(line,pref.getString("wifiStr1").c_str()); break;
            case 2: strcpy(line,pref.getString("wifiStr2").c_str()); break;
            case 3: strcpy(line,pref.getString("wifiStr3").c_str()); break;
            case 4: strcpy(line,pref.getString("wifiStr4").c_str()); break;
            case 5: strcpy(line,pref.getString("wifiStr5").c_str()); break;
        }
        if(strlen(line) < 5) { // line is empty
            line[0] = '\0'; // clear line
            strcat(line, ssid);
            strcat(line, "\t");
            strcat(line, password);
            if(i == 1) {pref.putString("wifiStr1", line); state = 2; goto exit;}
            if(i == 2) {pref.putString("wifiStr2", line); state = 2; goto exit;}
            if(i == 3) {pref.putString("wifiStr3", line); state = 2; goto exit;}
            if(i == 4) {pref.putString("wifiStr4", line); state = 2; goto exit;}
            if(i == 5) {pref.putString("wifiStr5", line); state = 2; goto exit;}
        }
    }
    state = 3;

exit:
    x_ps_free(&line); // free memory
    if(state == 0) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "SSID: %s password can't changed, it is hard coded", ssid);
        if(_f_WiFiConnected) audio.connecttospeech("S S I D and password are hard coded", "en");
    }
    if(state == 1) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The passord \"%s\" for the SSID: %s has been changed", password, ssid);
        if(_f_WiFiConnected) audio.connecttospeech("The password has been changed", "en");
    }
    if(state == 2) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_GREEN "The SSID: %s has been added", ssid);
        if(_f_WiFiConnected) audio.connecttospeech("The credentials has been added", "en");
    }
    if(state == 3) {
        SerialPrintfln("WiFI_info:   " ANSI_ESC_RED "No more memory to save the credentials for: %s", ssid);
        if(_f_WiFiConnected) audio.connecttospeech("The S S I D and password can't be stored", "en");
    }
    return;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*****************************************************************************************************************************************************
 *                                                                     A U D I O                                                                     *
 *****************************************************************************************************************************************************/
void connecttohost(const char* host) {
    int32_t idx1, idx2;
    char*   url = nullptr;
    char*   user = nullptr;
    char*   pwd = nullptr;

    dispFooter.updateBitRate(0);
    _cur_Codec = 0;
    //    if(_state == RADIO) clearStreamTitle();
    _icyBitRate = 0;
    _decoderBitRate = 0;
    _f_webFailed = false;

    idx1 = indexOf(host, "|", 0);
    if(idx1 == -1) { // no pipe found
        _f_isWebConnected = audio.connecttohost(host);

        if(!_f_isWebConnected){_cthFailCounter++;}
        else(_cthFailCounter = 0);

        _f_isFSConnected = false;
        return;
    }
    else { // pipe found     e.g. http://xxx.com/ext|user|pw
        idx2 = indexOf(host, "|", idx1 + 1);
        // log_i("idx2 = %i", idx2);
        if(idx2 == -1) { // second pipe not found
            _f_isWebConnected = audio.connecttohost(host);

            if(!_f_isWebConnected) {_cthFailCounter++;}
            else(_cthFailCounter = 0);

            _f_isFSConnected = false;
            return;
        }
        else {                         // extract url, user, pwd
            url = strndup(host, idx1); // extract url
            user = strndup(host + idx1 + 1, idx2 - idx1 - 1);
            pwd = strdup(host + idx2 + 1);
            SerialPrintfln("new host: .  %s user %s, pwd %s", url, user, pwd); _f_isWebConnected = audio.connecttohost(url, user, pwd);
            _f_isFSConnected = false;
            x_ps_free(&url);
            x_ps_free(&user);
            x_ps_free(&pwd);
        }
    }
    if(_cthFailCounter >= 3){
        audio.connecttospeech("The last hosts were not connected", "en");
        x_ps_free(&_settings.lastconnectedhost);
        _settings.lastconnectedhost = strdup("");
    }
}
void connecttoFS(const char* FS, const char* filename, uint32_t resumeFilePos) {
    if(!filename) return;
    dispFooter.updateBitRate(0);
    _icyBitRate = 0;
    _decoderBitRate = 0;
    _cur_Codec = 0;
    _f_webFailed = false;
    _f_isFSConnected = audio.connecttoFS(SD_MMC, filename, resumeFilePos);
    _f_isWebConnected = false;
    if(!startsWith(filename, "/audiofiles/")) {return;}
    if(_f_isFSConnected && isAudio(filename)) {
        x_ps_free(&_settings.lastconnectedfile);
        _settings.lastconnectedfile = x_ps_strdup(filename);
        _SD_content.setLastConnectedFile(filename);
        x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = x_ps_strdup(_SD_content.getLastConnectedFolder());
        x_ps_free(&_cur_AudioFileName); _cur_AudioFileName = x_ps_strdup(_SD_content.getLastConnectedFileName());
        _cur_AudioFileNr = _SD_content.getPosByFileName(_cur_AudioFileName);
    }
    //    log_w("Filesize %d", audioGetFileSize());
    //    log_w("FilePos %d", audioGetFilePosition());
}
void stopSong() {
    audio.stopSong();
    _f_isFSConnected = false;
    _f_isWebConnected = false;
    if(_f_playlistEnabled){
        vector_clear_and_shrink(_PLS_content);
        _f_playlistEnabled = false;
        SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "playlist stopped");
        webSrv.send("SD_playFile=", "playlist stopped");
    }
    _f_pauseResume = false;
    _f_playlistNextFile = false;
    _f_shuffle = false;
    x_ps_free(&_playlistPath);
}

/*****************************************************************************************************************************************************
 *                                                                    S E T U P                                                                      *
 *****************************************************************************************************************************************************/

void setup() {
    Audio::audio_info_callback = my_audio_info;
    esp_log_level_set("*", ESP_LOG_DEBUG);
    esp_log_set_vprintf(log_redirect_handler);
    Serial.begin(MONITOR_SPEED);
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
    trim(Version); Serial.printf("MiniWebRadio %s\n", Version);
    Serial.printf("ARDUINO_LOOP_STACK_SIZE %d words (32 bit)\n", CONFIG_ARDUINO_LOOP_STACK_SIZE);
    Serial.printf("FLASH size %lu bytes, speed %lu MHz\n", (long unsigned)ESP.getFlashChipSize(), (long unsigned)ESP.getFlashChipSpeed() / 1000000);
    Serial.printf("CPU speed %lu MHz\n", (long unsigned)ESP.getCpuFreqMHz());
    Serial.printf("SDMMC speed %d MHz\n", SDMMC_FREQUENCY / 1000000);
    Serial.printf("TFT speed %d MHz\n", TFT_FREQUENCY / 1000000);


    if(!psramInit()) { Serial.printf(ANSI_ESC_RED "PSRAM not found! MiniWebRadio doesn't work properly without PSRAM!" ANSI_ESC_WHITE); }
    else {
        _f_PSRAMfound = true;
        Serial.printf("PSRAM total size: %lu bytes\n", (long unsigned)ESP.getPsramSize());
    }
    if(ESP.getFlashChipSize() > 80000000) {
        if(!FFat.begin()) {
            if(!FFat.format()) Serial.printf("FFat Mount Failed\n");
        }
        else {
            Serial.printf("FFat total space: %d bytes, free space: %d bytes", FFat.totalBytes(), FFat.freeBytes());
            _f_FFatFound = true;
        }
    }
    const char* rr = NULL;
    _resetReason = esp_reset_reason();
    switch(_resetReason) {
        case ESP_RST_UNKNOWN: rr = "Reset reason can not be determined"; break;
        case ESP_RST_POWERON: rr = "Reset due to power-on event"; break;
        case ESP_RST_EXT: rr = "Reset by external pin (not applicable for ESP32)"; break;
        case ESP_RST_SW: rr = "Software reset via esp_restart"; break;
        case ESP_RST_PANIC: rr = "Software reset due to exception/panic"; break;
        case ESP_RST_INT_WDT: rr = "Reset (software or hardware) due to interrupt watchdog"; break;
        case ESP_RST_TASK_WDT: rr = "Reset due to task watchdog"; break;
        case ESP_RST_WDT:
            rr = "Reset due to other watchdogs";
            _resetReason = 1;
            break;
        case ESP_RST_DEEPSLEEP: rr = "Reset after exiting deep sleep mode"; break;
        case ESP_RST_BROWNOUT: rr = "Brownout reset (software or hardware)"; break;
        case ESP_RST_SDIO: rr = "Reset over SDIO"; break;
    }
    Serial.printf("RESET_REASON: %s", rr);
    Serial.print("\n\n");
    mutex_rtc = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    SerialPrintfln("   ");
    SerialPrintfln(ANSI_ESC_YELLOW "       ***************************    ");
    SerialPrintfln(ANSI_ESC_YELLOW "       *     MiniWebRadio V4     *    ");
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

    _cur_AudioFolder = strdup("/audiofiles/");

    if(TFT_CONTROLLER < 2)      strcpy(_prefix, "s/");
    else if(TFT_CONTROLLER < 7) strcpy(_prefix, "m/");
    else                        strcpy(_prefix, "l/");

    pref.begin("Pref", false);         // instance of preferences from AccessPoint (SSID, PW ...)

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

    SerialPrintfln("setup: ....  Init SD card");
    if(IR_PIN >= 0) pinMode(IR_PIN, INPUT_PULLUP); // if ir_pin is read only, have a external resistor (~10...40KOhm)
    pinMode(SD_MMC_D0, INPUT_PULLUP);
#ifdef CONFIG_IDF_TARGET_ESP32S3
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif
    int32_t sdmmc_frequency = SDMMC_FREQUENCY / 1000; // MHz -> KHz, default is 40MHz
    if(!SD_MMC.begin("/sdcard", true, false, sdmmc_frequency)) {
        clearAll();
        tft.setFont(_fonts[6]);
        tft.setTextColor(TFT_YELLOW);
        tft.writeText("SD Card Mount Failed", 0, 50, _dispWidth, _dispHeight, TFT_ALIGN_CENTER, TFT_ALIGN_TOP, false, false);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed");
        return;
    }
    float cardSize = ((float)SD_MMC.cardSize()) / (1024 * 1024);
    float freeSize = ((float)SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024);
    SerialPrintfln(ANSI_ESC_WHITE "setup: ....  SD card found, %.1f MB by %.1f MB free", freeSize, cardSize);
    _f_SD_MMCfound = true;
    defaultsettings();
    if(ESP.getFlashChipSize() > 80000000) { FFat.begin(); }
    if(TFT_BL >= 0){_f_brightnessIsChangeable = true;}
#if ESP_IDF_VERSION_MAJOR == 5
    if(TFT_BL >= 0) ledcAttach(TFT_BL, 1200, 8); // 1200 Hz PWM and 8 bit resolution
#endif

    if(TFT_CONTROLLER > 8) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid");

    drawImage("/common/MiniWebRadioV4.jpg", 0, 0); // Welcomescreen
    updateSettings();
    if(_brightness < 5) _brightness = 5;
    if(_volumeSteps < 21) _volumeSteps = 21;
    setTFTbrightness(_brightness);

    _f_WiFiConnected = connectToWiFi();
    if(_f_WiFiConnected){
        strcpy(_myIP, WiFi.localIP().toString().c_str());
        SerialPrintfln("setup: ....  connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", IP address is " ANSI_ESC_CYAN "%s"
                                                    ANSI_ESC_WHITE ", Received Signal Strength " ANSI_ESC_CYAN "%i"
                                                    ANSI_ESC_WHITE " dB", WiFi.SSID().c_str(), _myIP, WiFi.RSSI());

        if(!MDNS.begin("MiniWebRadio")) { SerialPrintfln("WiFI_info:   " ANSI_ESC_YELLOW "Error starting mDNS"); }
        else(SerialPrintfln("WiFI_info:   mDNS started"));
        MDNS.addService("esp32", "tcp", 80);
        SerialPrintfln("WiFI_info:   mDNS name: " ANSI_ESC_CYAN "MiniWebRadio");

        ArduinoOTA.setHostname("MiniWebRadio");
        ArduinoOTA.begin();
        ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); // username, password for ftp.
        setRTC(_TZString.c_str());
    }
    placingGraphicObjects();

    audio.setAudioTaskCore(AUDIOTASK_CORE);
    audio.setConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);
    audio.setVolumeSteps(_volumeSteps);
    audio.setVolume(0);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    audio.setI2SCommFMT_LSB(I2S_COMM_FMT);

    if(AMP_ENABLED >= 0) { // enable onboard amplifier
        pinMode(AMP_ENABLED, OUTPUT);
        digitalWrite(AMP_ENABLED, LOW);
    }

    sdr_CL_volume.setNewMinMaxVal(0, _volumeSteps);
    sdr_DL_volume.setNewMinMaxVal(0, _volumeSteps);
    sdr_PL_volume.setNewMinMaxVal(0, _volumeSteps);
    sdr_RA_volume.setNewMinMaxVal(0, _volumeSteps);

    SerialPrintfln("setup: ....  number of saved stations: " ANSI_ESC_CYAN "%d", staMgnt.getSumStations());
    SerialPrintfln("setup: ....  number of saved favourites: " ANSI_ESC_CYAN "%d", staMgnt.getSumFavStations());
    SerialPrintfln("setup: ....  current station number: " ANSI_ESC_CYAN "%d", _cur_station);
    SerialPrintfln("setup: ....  current volume: " ANSI_ESC_CYAN "%d", _cur_volume);
    SerialPrintfln("setup: ....  volume steps: " ANSI_ESC_CYAN "%d", _volumeSteps);
    SerialPrintfln("setup: ....  volume after alarm: " ANSI_ESC_CYAN "%d", _volumeAfterAlarm);
    SerialPrintfln("setup: ....  last connected host: " ANSI_ESC_CYAN "%s", _settings.lastconnectedhost);
    SerialPrintfln("setup: ....  connection timeout: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT);
    SerialPrintfln("setup: ....  connection timeout SSL: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT_SSL);

    ir.begin(); // Init InfraredDecoder

    if(_f_WiFiConnected) webSrv.begin(80, 81); // HTTP port, WebSocket port

    if(BT_EMITTER_CONNECT >= 0){
        pinMode(BT_EMITTER_CONNECT, OUTPUT);
       /*digitalWrite(BT_EMITTER_CONNECT, LOW); vTaskDelay(100);*/ digitalWrite(BT_EMITTER_CONNECT, HIGH); // POWER_ON
        _f_BTcurPowerState = true;
    }

    _dlnaLevel = 0;
    _dlnaHistory[0].name = strdup("Media Server");
    _dlnaHistory[0].objId = strdup("");
    _dlnaHistory[1].objId = strdup("0");
    _f_dlnaSeekServer = true;

    if( _resetReason == ESP_RST_POWERON ||   // Simply switch on the operating voltage
        _resetReason == ESP_RST_SW ||        // ESP.restart()
        _resetReason == ESP_RST_SDIO ||      // The boot button was pressed
        _resetReason == ESP_RST_DEEPSLEEP) { // Wake up
        if(WiFi.isConnected()){
            if(_cur_station > 0) setStation(_cur_station);
            else { setStationViaURL(_settings.lastconnectedhost, ""); }
        }
    }
    else { SerialPrintfln("RESET_REASON:" ANSI_ESC_RED "%s", rr); }

    if(_f_mute) { SerialPrintfln("setup: ....  volume is muted: (from " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET ")", _cur_volume); }
    setI2STone();

    if(I2C_SCL != -1){
        _f_BH1750_found = BH1750.begin(BH1750.ADDR_TO_GROUND , I2C_SDA, I2C_SCL);// init the sensor with address pin connetcted to ground
    }
    if (_f_BH1750_found) {                                                  // result (bool) wil be be "false" if no sensor found
        SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "Ambient Light Sensor BH1750 found");
        BH1750.setResolutionMode(BH1750.ONE_TIME_H_RESOLUTION_MODE);
        BH1750.setSensitivity(BH1750.SENSITIVITY_ADJ_MAX);
    }
    ticker100ms.attach(0.1, timer100ms);
    #if TFT_CONTROLLER == 7
        tft.clearVsyncCounter(); // clear the vsync counter and start them
    #endif

    tft.fillScreen(TFT_BLACK); // Clear screen
    drawImage("/common/Wallpaper.jpg", 0, 0); // Wallpaper
    tft.copyFramebuffer( 0, 1, 0, 0, _dispWidth, _dispHeight); // copy wallpaper to background
    muteChanged(_f_mute);
    dispFooter.setIpAddr(WiFi.localIP().toString().c_str());
    dispFooter.updateStation(_cur_station);
    dispFooter.updateOffTime(_sleeptime);
    dispFooter.show(true);

    dispHeader.updateItem(_hl_item[RADIO]);
    dispHeader.updateVolume(_cur_volume);
    dispHeader.speakerOnOff(!_f_mute);
    dispHeader.show(true);
    _radioSubMenue = 0;
    _state = NONE;
    if(_f_WiFiConnected) changeState(RADIO);
    else changeState(WIFI_SETTINGS);
    bt_emitter.begin();
}
/*****************************************************************************************************************************************************
 *                                                                   C O M M O N                                                                     *
 *****************************************************************************************************************************************************/

const char* scaleImage(const char* path) {
    if((!endsWith(path, "bmp")) && (!endsWith(path, "jpg")) && (!endsWith(path, "gif") &&(!endsWith(path, "png")))) { // not a image
        return path;
    }
    if(startsWith(path, "/png")) return path; // nothing to do, icons for website
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    int idx = indexOf(path, "/", 1);
    if(idx < 0) return path; // invalid path
    else {
        strncpy(pathBuff, path, idx + 1);
        strcat(pathBuff, _prefix);
        strcat(pathBuff, path + idx + 1);
    }
    return pathBuff;
}

void setVolume(uint8_t vol) {
    static int16_t oldVol = -1;
    log_i("volume old: %i. new: %i", oldVol, vol);
    if(vol == oldVol) return;
    _cur_volume = vol;
    oldVol = vol;
    dispHeader.updateVolume(_cur_volume);
    sdr_CL_volume.setValue(_cur_volume);
    sdr_DL_volume.setValue(_cur_volume);
    sdr_PL_volume.setValue(_cur_volume);
    sdr_RA_volume.setValue(_cur_volume);
    SerialPrintfln("action: ...  current volume is " ANSI_ESC_CYAN "%d", _cur_volume);
}

uint8_t downvolume() {
    uint8_t steps = _volumeSteps / 20;
    if(_cur_volume == 0) return _cur_volume;
    else if (steps < _cur_volume) _cur_volume -= steps;
    else _cur_volume --;
    sdr_CL_volume.setValue(_cur_volume);
    sdr_DL_volume.setValue(_cur_volume);
    sdr_PL_volume.setValue(_cur_volume);
    sdr_RA_volume.setValue(_cur_volume);
    _f_mute = false;
    muteChanged(_f_mute); // set mute off
    return _cur_volume;
}

uint8_t upvolume() {
    uint8_t steps = _volumeSteps / 20;
    if(_cur_volume == _volumeSteps) return _cur_volume;
    else if (_volumeSteps > _cur_volume + steps) _cur_volume += steps;
    else  _cur_volume ++;
    sdr_CL_volume.setValue(_cur_volume);
    sdr_DL_volume.setValue(_cur_volume);
    sdr_PL_volume.setValue(_cur_volume);
    sdr_RA_volume.setValue(_cur_volume);
    _f_mute = false;
    muteChanged(_f_mute); // set mute off
    return _cur_volume;
}

void setStation(uint16_t sta) {
    static uint16_t old_cur_station = 0;
    if(sta == 0) {return;}
    if(sta > staMgnt.getSumStations()) sta = _cur_station;
    x_ps_free(&_stationURL);
    _stationURL = x_ps_strdup(staMgnt.getStationUrl(sta));
    _homepage = "";
    SerialPrintfln("action: ...  switch to station " ANSI_ESC_CYAN "%d", sta);

    if(_f_isWebConnected && sta == old_cur_station && _state == RADIO) { // Station is already selected
        _f_newStreamTitle = true;
    }
    else {
        if(_state != RADIO) {_radioSubMenue = 0; changeState(RADIO);}
        _streamTitle[0] = '\0';
        _icyDescription[0] = '\0';
        _f_newStreamTitle = true;
        _f_newIcyDescription = true;
        connecttohost(_stationURL);
    //    if(!_f_isWebConnected) _cur_station = old_cur_station; // host is not connected
    }
    old_cur_station = sta;
    StationsItems();
    if(_state == RADIO) {
        showLogoAndStationName(true);
        if(_cur_station == 0){
            dispFooter.updateFlag(NULL);
        }
        else{
            dispFooter.updateFlag(getFlagPath(_cur_station));
        }
    }
    dispFooter.updateStation(_cur_station);
}
const char* getFlagPath(uint16_t station) {
    static char flagPath[40];
    flagPath[0] = '\0';
    strcpy(flagPath, "/flags/");
    strcat(flagPath, staMgnt.getStationCountry(station));
    for(int i = 0; i< strlen(flagPath); i++) flagPath[i] = tolower(flagPath[i]);
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
void setStationByNumber(uint16_t staNr){
    setStation(staMgnt.setStationByNumber(staNr));
}

void StationsItems() {
    if(!_stationName_air)  _stationName_air = (char*)calloc(1, 1);
    char* stationLogo_air = x_ps_malloc(strlen(_stationName_air) + 15);
    strcpy(stationLogo_air, "/logo/");
    strcat(stationLogo_air, _stationName_air);
    strcat(stationLogo_air, ".jpg");
    char staNr[10]; itoa(_cur_station, staNr, 10);

    if(_cur_station == 0){
        webSrv.send("stationLogo=", stationLogo_air);
        webSrv.send("stationNr=", staNr);
        webSrv.send("stationURL=", _settings.lastconnectedhost);
    }
    else{
        webSrv.send("stationLogo=", "/logo/" + String(staMgnt.getStationName(_cur_station)) + ".jpg");
        webSrv.send("stationNr=", staNr);
        if(_stationURL) webSrv.send("stationURL=", String(_stationURL));
    }
    x_ps_free(&stationLogo_air);
}

void setStationViaURL(const char* url, const char* extension) {
    // e.g.  http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one&bitrate=96000
    // url is http://lstn.lv/bbcradio.m3u8?station=bbc_radio_one, extension is bitrate=96000
    x_ps_free(&_stationName_air);
    _cur_station = 0;
    x_ps_free(&_stationURL);
    int len_url = strlen(url) + strlen(extension) + 3;
    char* origin_url = (char*)calloc(1, len_url);
    if(strlen(extension) > 0) {
        strcpy(origin_url, url);
        strcat(origin_url, "&");
        strcat(origin_url, extension);
    }
    else { strcpy(origin_url, url); }
    _stationURL = x_ps_strdup(origin_url);
    connecttohost(origin_url);
    StationsItems();
    if(_state == RADIO) {
        clearStreamTitle();
        showLogoAndStationName(true);
        dispFooter.updateFlag(NULL);
    }
    dispFooter.updateStation(0); // set 000
}

void savefile(const char* fileName, uint32_t contentLength) { // save the uploadfile on SD_MMC
    char fn[256];
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
}

void saveImage(const char* fileName, uint32_t contentLength) { // save the jpg image on SD_MMC
    char fn[256];

    if(!_f_SD_Upload && endsWith(fileName, "jpg")) {
        strcpy(fn, "/logo/");
        strcat(fn, _prefix);
        if(!startsWith(fileName, "/")) strcat(fn, "/");
        strcat(fn, fileName);
        if(webSrv.uploadB64image(SD_MMC, fn, contentLength)) {
            SerialPrintfln("save image (jpg) " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.sendStatus(200);
        }
        else webSrv.sendStatus(400);
    }
}

String setI2STone() {
    int8_t LP = _toneLP;
    int8_t BP = _toneBP;
    int8_t HP = _toneHP;
    int8_t BAL = _toneBAL;
    audio.setTone(LP, BP, HP);
    audio.setBalance(BAL);
    sprintf(_chbuf, "LowPass=%i\nBandPass=%i\nHighPass=%i\nBalance=%i\n", LP, BP, HP, BAL);
    String tone = String(_chbuf);
    return tone;
}

void SD_playFile(const char* pathWoFileName, const char* fileName) { // pathWithoutFileName e.g. /audiofiles/playlist/
    sprintf(_chbuf, "%s%s", pathWoFileName, fileName);
    int32_t idx = indexOf(_chbuf, "\033[", 1);
    if(idx == -1) { ; }          // do nothing
    else { _chbuf[idx] = '\0'; } // remove color and filesize
    SD_playFile(_chbuf, 0, true);
}

void SD_playFile(const char* path, uint32_t resumeFilePos, bool showFN) {
    if(!path) return;                            // avoid a possible crash
    if(endsWith(path, "ogg")) resumeFilePos = 0; // resume only mp3, m4a, flac and wav
    if(endsWith(path, "m3u")) {
        if(SD_MMC.exists(path)) {
            preparePlaylistFromFile(path);
            processPlaylist(true);
        }
        return;
    }
    if(_playerSubMenue != 1 && _playerSubMenue != 3){
        _playerSubMenue = 1; changeState(PLAYER);
    }
    int32_t idx = lastIndexOf(path, '/');
    if(idx < 0) return;
    x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = strdup(path); _cur_AudioFolder[idx] = '\0';

    if(showFN) {
        clearLogo();
        showFileName(path + idx + 1);
    }

    SerialPrintfln("AUDIO_FILE:  " ANSI_ESC_MAGENTA "%s", path + idx + 1);
    connecttoFS("SD_MMC", (const char*)path, resumeFilePos);
    if(_f_playlistEnabled) showPlsFileNumber();
    if(_f_isFSConnected) {
        x_ps_free(&_settings.lastconnectedfile);
        _settings.lastconnectedfile = x_ps_strdup(path);
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
        else success = SD_MMC.remove(itemPath);
    }
    return success;
}

void fall_asleep() {
    _f_sleeping = true;
    _f_mute = true;
    _f_playlistEnabled = false;
    _f_isFSConnected = false;
    _f_isWebConnected = false;
    audio.stopSong();
    if(_sleepMode == 0){
        clearAll();
        setTFTbrightness(0);
    }
    else{
        _clockSubMenue = 0;
        changeState(CLOCK);
    }
    if(_f_BTpower) BTpowerChanged(false);
    SerialPrintfln("falling asleep");
    dispHeader.hide();
    dispFooter.hide();
}

void wake_up() {
    _f_sleeping = false;
    _f_mute = false;
    _f_irOnOff = false;
    SerialPrintfln("awake");
    _f_mute = true;
    muteChanged(false);
    clearAll();
    setTFTbrightness(_brightness);
    if(_cur_station) setStation(_cur_station);
    else connecttohost(_settings.lastconnectedhost);
    _radioSubMenue = 0;
    changeState(RADIO);
    showLogoAndStationName(true);
    dispHeader.show(true);
    dispHeader.speakerOnOff(!_f_mute);
    dispHeader.updateRSSI(WiFi.RSSI(), true);
    dispFooter.show(true);
    if(_f_BTpower) BTpowerChanged(true);
}

void setRTC(const char* TZString) {
    rtc.stop();
    rtc.begin(_TZString.c_str());
    // if(!_f_rtc) {
    //     SerialPrintfln(ANSI_ESC_RED "connection to NTP failed, trying again");
    //     ESP.restart();
    // }
}

boolean isAlarm(uint8_t weekDay, uint8_t alarmDays, uint16_t minuteOfTheDay, int16_t* alarmTime){
    uint8_t mask = 0b00000001 << weekDay;
    if(alarmDays & mask){ // yes, is alarmDay
        if(alarmTime[weekDay] == minuteOfTheDay){ // yes, is alarmTime
            return true;
        }
    }
    return false;
}

boolean copySDtoFFat(const char* path) {
    if(!_f_FFatFound) return false;
    uint8_t buffer[1024];
    size_t  r = 0, w = 0;
    size_t  len = 0;
    File    file1 = SD_MMC.open(path, "r");
    File    file2 = FFat.open(path, "w");
    while(true) {
        r = file1.read(buffer, 1024);
        w = file2.write(buffer, r);
        if(r != w) {
            file1.close();
            file2.close();
            FFat.remove(path);
            return false;
        }
        len += r;
        if(r == 0) break;
    }
    log_i("file length %i, written %i", file1.size(), len);
    if(file1.size() == len) return true;
    return false;
}

void muteChanged(bool m) {
    _f_muteIsPressed = false;
    btn_CL_mute.setValue(m);
    btn_DL_mute.setValue(m);
    btn_EQ_mute.setValue(m);
    btn_PL_mute.setValue(m);
    btn_RA_mute.setValue(m);
    if(m) {_f_mute = true;}
    else  {_f_mute = false;}
    if(m) webSrv.send("mute=", "1");
    else webSrv.send("mute=", "0");
    dispHeader.speakerOnOff(!_f_mute);
    dispHeader.updateVolume(_cur_volume);
    updateSettings();
};

void BTpowerChanged(int8_t newState){
    if(BT_EMITTER_CONNECT == -1) return;    // guard
    if(_f_BtEmitterFound == false) return;  // guard

    if(newState){                           // POWER_ON
        digitalWrite(BT_EMITTER_CONNECT, LOW);
        vTaskDelay(100);
        digitalWrite(BT_EMITTER_CONNECT, HIGH);
    }
    else{
        bt_emitter.cmd_PowerOff();          // POWER OFF
    }
    return;
}

void logAlarmItems() {
    const char wd[7][11] = {"Sunday:   ", "Monday:   ", "Tuesday:  ", "Wednesday:", "Thursday: ", "Friday:   ", "Saturday: "};
    uint8_t    mask = 0b00000001;
    for(uint8_t i = 0; i < 7; i++) {
        if(_alarmdays & mask) { SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s " ANSI_ESC_CYAN "%02i:%02i", wd[i], _alarmtime[i] / 60, _alarmtime[i] % 60); }
        else { SerialPrintfln("AlarmTime:   " ANSI_ESC_YELLOW "%s No alarm is set", wd[i]); }
        mask <<= 1;
    }
}

void setTimeCounter(uint8_t sec){
    if(sec){
        _timeCounter.timer = 10;
        _timeCounter.factor = sec;
    }
    else{
        _timeCounter.timer = 0;
        _timeCounter.factor = 0;
        dispFooter.updateTC(0);
    }
}


/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                  M E N U E / B U T T O N S                                                                  ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format off
void placingGraphicObjects() { // and initialize them
    // ALL STATE
    dispHeader.begin(         _winHeader.x, _winHeader.y, _winHeader.w, _winHeader.h);   dispHeader.setTimeColor(TFT_LIGHTGREEN);
    dispFooter.begin(         _winFooter.x, _winFooter.y, _winFooter.w, _winFooter.h);
    volBox.begin(             _winVolBox.x, _winVolBox.y, _winVolBox.w, _winVolBox.h);
    myList.begin(             _winWoHF.x,   _winWoHF.y,   _winWoHF.w,   _winWoHF.h, _fonts[0]);
    // RADIO -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_RA_volume.begin(      _sdrOvBtns.x, _sdrOvBtns.y, _sdrOvBtns.w, _sdrOvBtns.h, _sdrOvBtns.pl, _sdrOvBtns.pr, _sdrOvBtns.pt, _sdrOvBtns.pb, 0, _volumeSteps);               sdr_RA_volume.setValue(_cur_volume);
    btn_RA_mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
                                                                                         btn_RA_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
                                                                                         btn_RA_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
                                                                                         btn_RA_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
                                                                                         btn_RA_mute.setValue(_f_mute);
    btn_RA_prevSta.begin( 1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_prevSta.setDefaultPicturePath("/btn/Button_Previous_Green.png");
                                                                                         btn_RA_prevSta.setClickedPicturePath("/btn/Button_Previous_Yellow.png");
    btn_RA_nextSta.begin( 2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_nextSta.setDefaultPicturePath("/btn/Button_Next_Green.png");
                                                                                         btn_RA_nextSta.setClickedPicturePath("/btn/Button_Next_Yellow.png");
    btn_RA_staList.begin( 0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_staList.setDefaultPicturePath("/btn/Button_List_Green.png");
                                                                                         btn_RA_staList.setClickedPicturePath("/btn/Button_List_Yellow.png");
                                                                                         btn_RA_staList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_RA_player.begin(  1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_player.setDefaultPicturePath("/btn/Button_Player_Green.png");
                                                                                         btn_RA_player.setClickedPicturePath("/btn/Button_Player_Yellow.png");
                                                                                         btn_RA_player.setAlternativePicturePath("/btn/Button_Player_Magenta.png");
    btn_RA_dlna.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_dlna.setDefaultPicturePath("/btn/Button_DLNA_Green.png");
                                                                                         btn_RA_dlna.setClickedPicturePath("/btn/Button_DLNA_Yellow.png");
                                                                                         btn_RA_dlna.setAlternativePicturePath("/btn/Button_DLNA_Magenta.png");
    btn_RA_clock.begin(   3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_clock.setDefaultPicturePath("/btn/Button_Clock_Green.png");
                                                                                         btn_RA_clock.setClickedPicturePath("/btn/Button_Clock_Yellow.png");
                                                                                         btn_RA_clock.setAlternativePicturePath("/btn/Button_Clock_Magenta.png");
    btn_RA_sleep.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_sleep.setDefaultPicturePath("/btn/Button_OffTimer_Green.png");
                                                                                         btn_RA_sleep.setClickedPicturePath("/btn/Button_OffTimer_Yellow.png");
                                                                                         btn_RA_sleep.setAlternativePicturePath("/btn/Button_OffTimer_Magenta.png");
    btn_RA_settings.begin(5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_settings.setDefaultPicturePath("/btn/Button_Settings_Green.png");
                                                                                         btn_RA_settings.setClickedPicturePath("/btn/Button_Settings_Yellow.png");
                                                                                         btn_RA_settings.setAlternativePicturePath("/btn/Button_Settings_Magenta.png");
    btn_RA_bt.begin(      6 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_bt.setDefaultPicturePath("/btn/Button_Bluetooth_Green.png");
                                                                                         btn_RA_bt.setClickedPicturePath("/btn/Button_Bluetooth_Yellow.png");
                                                                                         btn_RA_bt.setAlternativePicturePath("/btn/Button_Bluetooth_Magenta.png");
                                                                                         btn_RA_bt.setInactivePicturePath("/btn/Button_Bluetooth_Grey.png");
    btn_RA_off.begin(     7 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
                                                                                         btn_RA_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
                                                                                         btn_RA_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    txt_RA_sTitle.begin( _winSTitle.x, _winSTitle.y, _winSTitle.w, _winSTitle.h, _winSTitle.pl, _winSTitle.pr, _winSTitle.pt, _winSTitle.pb); txt_RA_sTitle.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);  txt_RA_sTitle.setFont(0); // 0 -> auto
    txt_RA_staName.begin(       _winName.x,   _winName.y,   _winName.w,   _winName.h, _winName.pl, _winName.pr, _winName.pt, _winName.pl);    txt_RA_staName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_TOP);    txt_RA_staName.setFont(0); // 0 -> auto
    txt_RA_irNum.begin(         _winWoHF.x,   _winWoHF.y,   _winWoHF.w,   _winWoHF.h, _winWoHF.pl, _winWoHF.pr, _winWoHF.pt, _winWoHF.pb);    txt_RA_irNum.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_RA_irNum.setTextColor(TFT_GOLD); txt_RA_irNum.setFont(_bigNumbersFontSize);
    pic_RA_logo.begin(          _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h, _winLogo.pl, _winLogo.pr, _winLogo.pt, _winLogo.pb);
    VUmeter_RA.begin(        _winVUmeter.x,_winVUmeter.y,_winVUmeter.w,_winVUmeter.h);
    // STATIONSLIST ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_RADIO.begin(          _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _tftSize, _listFontSize, &_cur_station);
    // PLAYER-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_PL_mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
                                                                                             btn_PL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
                                                                                             btn_PL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
                                                                                             btn_PL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
                                                                                             btn_PL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
                                                                                             btn_PL_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
                                                                                             btn_PL_mute.setValue(_f_mute);
    btn_PL_pause.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
                                                                                             btn_PL_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
                                                                                             btn_PL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
                                                                                             btn_PL_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
                                                                                             btn_PL_pause.setAlternativeOffPicturePath("/btn/Button_Pause_Magenta.png");
                                                                                             btn_PL_pause.setAlternativeOnPicturePath("/btn/Button_Play_Magenta.png");
                                                                                             btn_PL_pause.setValue(false);
    btn_PL_cancel.begin(  2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.png");
                                                                                             btn_PL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
                                                                                             btn_PL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    sdr_PL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _dispWidth - (5 * _winButton.w + 20), _winButton.h, _winButton.pl, _winButton.pr, _winButton.pt, _winButton.pb, 0, _volumeSteps); sdr_PL_volume.setValue(_cur_volume);
    btn_PL_prevFile.begin(0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_prevFile.setDefaultPicturePath("/btn/Button_Left_Blue.png");
                                                                                             btn_PL_prevFile.setClickedPicturePath("/btn/Button_Left_Yellow.png");
                                                                                             btn_PL_prevFile.setAlternativePicturePath("/btn/Button_Left_Magenta.png");
    btn_PL_nextFile.begin(1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_nextFile.setDefaultPicturePath("/btn/Button_Right_Blue.png");
                                                                                             btn_PL_nextFile.setClickedPicturePath("/btn/Button_Right_Yellow.png");
                                                                                             btn_PL_nextFile.setAlternativePicturePath("/btn/Button_Right_Magenta.png");
    btn_PL_ready.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
                                                                                             btn_PL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
                                                                                             btn_PL_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    btn_PL_playAll.begin( 3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_playAll.setDefaultPicturePath("/btn/Button_PlayAll_Blue.png");
                                                                                             btn_PL_playAll.setClickedPicturePath("/btn/Button_PlayAll_Yellow.png");
                                                                                             btn_PL_playAll.setAlternativePicturePath("/btn/Button_PlayAll_Magenta.png");
    btn_PL_shuffle.begin( 4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_shuffle.setDefaultPicturePath("/btn/Button_Shuffle_Blue.png");
                                                                                             btn_PL_shuffle.setClickedPicturePath("/btn/Button_Shuffle_Yellow.png");
                                                                                             btn_PL_shuffle.setAlternativePicturePath("/btn/Button_Shuffle_Magenta.png");
    btn_PL_fileList.begin(5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h, true); btn_PL_fileList.setDefaultPicturePath("/btn/Button_List_Green.png");
                                                                                             btn_PL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.png");
                                                                                             btn_PL_fileList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_PL_radio.begin(   6 * _winButton.w, _winButton.y, _winButton.w, _winButton.h, true); btn_PL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                             btn_PL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
                                                                                             btn_PL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_PL_off.begin(     7 * _winButton.w, _winButton.y, _winButton.w, _winButton.h, true); btn_PL_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
                                                                                             btn_PL_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
                                                                                             btn_PL_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    btn_PL_playPrev.begin(3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_playPrev.setDefaultPicturePath("/btn/Button_Previous_Blue.png");
                                                                                             btn_PL_playPrev.setClickedPicturePath("/btn/Button_Previous_Yellow.png");
                                                                                             btn_PL_playPrev.setAlternativePicturePath("/btn/Button_Previous_Magenta.png");
    btn_PL_playNext.begin(4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);       btn_PL_playNext.setDefaultPicturePath("/btn/Button_Next_Blue.png");
                                                                                             btn_PL_playNext.setClickedPicturePath("/btn/Button_Next_Yellow.png");
                                                                                             btn_PL_playNext.setAlternativePicturePath("/btn/Button_Next_Magenta.png");

    txt_PL_fName.begin(         _winName.x,   _winName.y,   _winName.w,   _winName.h,    _winName.pl,    _winName.pr,    _winName.pt,    _winName.pb);  txt_PL_fName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);  txt_PL_fName.setFont(0); // 0 -> auto
    txt_PL_fNumber.begin(       _winFileNr.x, _winFileNr.y, _winFileNr.w, _winFileNr.h,  _winFileNr.pl,  _winFileNr.pr,  _winFileNr.pt,  _winFileNr.pb);txt_PL_fNumber.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_PL_fNumber.setFont(_fileNumberFontSize);
    pic_PL_logo.begin(          _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h,    _winLogo.pl,    _winLogo.pr,    _winLogo.pt,    _winLogo.pb);
    pgb_PL_progress.begin(      _winProgbar.x,_winProgbar.y,_winProgbar.w,_winProgbar.h, _winProgbar.pl, _winProgbar.pr, _winProgbar.pt, _winProgbar.pb, 0, 30); pgb_PL_progress.setValue(0);
    // AUDIOFILESLIST-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_PLAYER.begin(         _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _tftSize, _listFontSize);
    // DLNA --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_DL_mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
                                                                                         btn_DL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
                                                                                         btn_DL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
                                                                                         btn_DL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
                                                                                         btn_DL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
                                                                                         btn_DL_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
                                                                                         btn_DL_mute.setValue(_f_mute);
    btn_DL_pause.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
                                                                                         btn_DL_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
                                                                                         btn_DL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
                                                                                         btn_DL_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
                                                                                         btn_DL_pause.setInactivePicturePath("/btn/Button_Pause_Grey.png");
                                                                                         btn_DL_pause.setAlternativeOffPicturePath("/btn/Button_Pause_Magenta.png");
                                                                                         btn_DL_pause.setAlternativeOnPicturePath("/btn/Button_Play_Magenta.png");
                                                                                         btn_DL_pause.setValue(false);
    btn_DL_cancel.begin(  2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.png");
                                                                                         btn_DL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
                                                                                         btn_DL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    btn_DL_fileList.begin(3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_fileList.setDefaultPicturePath("/btn/Button_List_Green.png");
                                                                                         btn_DL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.png");
                                                                                         btn_DL_fileList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_DL_radio.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_DL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
                                                                                         btn_DL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    sdr_DL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _winButton.w * 3 - 10, _winButton.h, _winButton.pl, _winButton.pr, _winButton.pt, _winButton.pb, 0, _volumeSteps); sdr_DL_volume.setValue(_cur_volume);
    txt_DL_fName.begin(       _winName.x,   _winName.y,   _winName.w,   _winName.h, _winName.pl, _winName.pr, _winName.pt, _winName.pb); txt_DL_fName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER); txt_DL_fName.setFont(0); // 0 -> auto)
    pic_DL_logo.begin(        _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h, _winLogo.pl,   _winLogo.pr, _winLogo.pt, _winLogo.pb);
    pgb_DL_progress.begin(    _winProgbar.x,_winProgbar.y,_winProgbar.w,_winProgbar.h, _winProgbar.pl, _winProgbar.pr, _winProgbar.pt, _winProgbar.pb, 0, 30); pgb_DL_progress.setValue(0);
    // DLNAITEMSLIST -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_DLNA.begin(           _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _tftSize, _listFontSize);
    // CLOCK -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_CL_24.begin(          _winDigits.x, _winDigits.y, _winDigits.w, _winDigits.h);
    clk_CL_12.begin(          _winDigits.x, _winDigits.y, _winDigits.w, _winDigits.h);
    btn_CL_alarm.begin(   0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_alarm.setDefaultPicturePath("/btn/Button_Bell_Green.png");
                                                                                         btn_CL_alarm.setClickedPicturePath("/btn/Button_Bell_Yellow.png");
                                                                                         btn_CL_alarm.setAlternativePicturePath("/btn/Button_Bell_Magenta.png");
    btn_CL_radio.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_CL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
                                                                                         btn_CL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_CL_mute.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
                                                                                         btn_CL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
                                                                                         btn_CL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
                                                                                         btn_CL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
                                                                                         btn_CL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
                                                                                         btn_CL_mute.setAlternativeOnPicturePath("/btn/Button_MuteOn_Magenta.png");
                                                                                         btn_CL_mute.setValue(_f_mute);
    btn_CL_off.begin(     3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
                                                                                         btn_CL_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
                                                                                         btn_CL_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    sdr_CL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _winButton.w * 3 - 10, _winButton.h, _winButton.pl, _winButton.pr, _winButton.pt, _winButton.pb, 0, _volumeSteps); sdr_CL_volume.setValue(_cur_volume);
    // ALARM -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_AC_red.begin(         _winDigits.x, _winDigits.y, _winDigits.w, _winDigits.h);      clk_AC_red.setAlarmTimeAndDays(&_alarmdays, _alarmtime);
    btn_AC_left.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AC_left.setDefaultPicturePath("/btn/Button_Left_Blue.png");
                                                                                         btn_AC_left.setClickedPicturePath("/btn/Button_Left_Yellow.png");
                                                                                         btn_AC_left.setAlternativePicturePath("/btn/Button_Left_Magenta.png");
    btn_AC_right.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AC_right.setDefaultPicturePath("/btn/Button_Right_Blue.png");
                                                                                         btn_AC_right.setClickedPicturePath("/btn/Button_Right_Yellow.png");
                                                                                         btn_AC_right.setAlternativePicturePath("/btn/Button_Right_Magenta.png");
    btn_AC_up.begin(      2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AC_up.setDefaultPicturePath("/btn/Button_Up_Blue.png");
                                                                                         btn_AC_up.setClickedPicturePath("/btn/Button_Up_Yellow.png");
                                                                                         btn_AC_up.setAlternativePicturePath("/btn/Button_Up_Magenta.png");
    btn_AC_down.begin(    3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AC_down.setDefaultPicturePath("/btn/Button_Down_Blue.png");
                                                                                         btn_AC_down.setClickedPicturePath("/btn/Button_Down_Yellow.png");
                                                                                         btn_AC_down.setAlternativePicturePath("/btn/Button_Down_Magenta.png");
    btn_AC_ready.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AC_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
                                                                                         btn_AC_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
                                                                                         btn_AC_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    // RINGING -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    pic_RI_logo.begin(        _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h, _winLogo.pl, _winLogo.pr, _winLogo.pt, _winLogo.pb);
    clk_RI_24small.begin(     _winName.x,   _winName.y,   _winName.w,   _winName.h);
    // SLEEPTIMER --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SL_up.begin(      0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_up.setDefaultPicturePath("/btn/Button_Up_Blue.png");
                                                                                         btn_SL_up.setClickedPicturePath("/btn/Button_Up_Yellow.png");
                                                                                         btn_SL_up.setAlternativePicturePath("/btn/Button_Up_Magenta.png");
    btn_SL_down.begin(    1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_down.setDefaultPicturePath("/btn/Button_Down_Blue.png");
                                                                                         btn_SL_down.setClickedPicturePath("/btn/Button_Down_Yellow.png");
                                                                                         btn_SL_down.setAlternativePicturePath("/btn/Button_Down_Magenta.png");
    btn_SL_ready.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
                                                                                         btn_SL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
                                                                                         btn_SL_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    btn_SL_cancel.begin(  4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Blue.png");
                                                                                         btn_SL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
                                                                                         btn_SL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    // SETTINGS ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SE_bright.begin(  0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SE_bright.setDefaultPicturePath("/btn/Button_Brightness_Green.png");
                                                                                         btn_SE_bright.setClickedPicturePath("/btn/Button_Brightness_Yellow.png");
                                                                                         btn_SE_bright.setAlternativePicturePath("/btn/Button_Brightness_Magenta.png");
                                                                                         btn_SE_bright.setInactivePicturePath("/btn/Button_Brightness_Grey.png");
    btn_SE_equal.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SE_equal.setDefaultPicturePath("/btn/Button_Equalizer_Green.png");
                                                                                         btn_SE_equal.setClickedPicturePath("/btn/Button_Equalizer_Yellow.png");
                                                                                         btn_SE_equal.setAlternativePicturePath("/btn/Button_Equalizer_Magenta.png");
    btn_SE_wifi.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SE_wifi.setDefaultPicturePath("/btn/Button_WiFi_Green.png");
                                                                                         btn_SE_wifi.setClickedPicturePath("/btn/Button_WiFi_Yellow.png");
                                                                                         btn_SE_wifi.setAlternativePicturePath("/btn/Button_WiFi_Magenta.png");
    btn_SE_radio.begin(   3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SE_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_SE_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
                                                                                         btn_SE_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    pic_SE_logo.begin(        _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h, _winLogo.pl, _winLogo.pr, _winLogo.pt, _winLogo.pb);
    // BRIGHTNESS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_BR_value.begin(   2 * _winButton.w, _winButton.y + 5,  4 * _winButton.w, _sdrOvBtns.h, _winButton.pl, _winButton.pr, _winButton.pt, _winButton.pb, _BrightnessMin, 100); sdr_BR_value.setValue(_brightness);
    btn_BR_ready.begin(   7 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BR_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
                                                                                         btn_BR_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
                                                                                         btn_BR_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    pic_BR_logo.begin(    0, _winWoHF.y, _winWoHF.w, _winWoHF.h, _winWoHF.pl, _winWoHF.pr, _winWoHF.pt, _winWoHF.pb); pic_BR_logo.setPicturePath("/common/Brightness.jpg");
    txt_BR_value.begin(   0, _winButton.y, _winButton.w * 2, _winButton.h, _winButton.pl, _winButton.pr, _winButton.pt, _winButton.pb); txt_BR_value.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_BR_value.setFont(_fonts[4]);
    // EQUALIZER ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_EQ_lowPass.begin(  _sdrLP.x,  _sdrLP.y,  _sdrLP.w,  _sdrLP.h,  _sdrLP.pl,  _sdrLP.pr,  _sdrLP.pt,  _sdrLP.pb, -40,  6); sdr_EQ_lowPass.setValue(_toneLP);
    sdr_EQ_bandPass.begin( _sdrBP.x,  _sdrBP.y,  _sdrBP.w,  _sdrBP.h,  _sdrBP.pl,  _sdrBP.pr,  _sdrBP.pt,  _sdrBP.pb, -40,  6); sdr_EQ_bandPass.setValue(_toneBP);
    sdr_EQ_highPass.begin( _sdrHP.x,  _sdrHP.y,  _sdrHP.w,  _sdrHP.h,  _sdrHP.pl,  _sdrHP.pr,  _sdrHP.pt,  _sdrHP.pb, -40,  6); sdr_EQ_highPass.setValue(_toneHP);
    sdr_EQ_balance.begin( _sdrBAL.x, _sdrBAL.y, _sdrBAL.w, _sdrBAL.h, _sdrBAL.pl, _sdrBAL.pr, _sdrBAL.pt, _sdrBAL.pb, -16, 16); sdr_EQ_balance.setValue(_toneBAL);
    txt_EQ_lowPass.begin(  _txtLP.x,  _txtLP.y,  _txtLP.w,  _txtLP.h, 0, 0, 0, 0); txt_EQ_lowPass.setAlign( TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER); txt_EQ_lowPass.setFont(0); // 0 -> auto
    txt_EQ_bandPass.begin( _txtBP.x,  _txtBP.y,  _txtBP.w,  _txtBP.h, 0, 0, 0, 0); txt_EQ_bandPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER); txt_EQ_bandPass.setFont(0); // 0 -> auto
    txt_EQ_highPass.begin( _txtHP.x,  _txtHP.y,  _txtHP.w,  _txtHP.h, 0, 0, 0, 0); txt_EQ_highPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER); txt_EQ_highPass.setFont(0); // 0 -> auto
    txt_EQ_balance.begin( _txtBAL.x, _txtBAL.y, _txtBAL.w, _txtBAL.h, 0, 0, 0, 0); txt_EQ_balance.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);  txt_EQ_balance.setFont(0); // 0 -> auto
    btn_EQ_lowPass.begin( _btnLP.x, _btnLP.y, _btnLP.w, _btnLP.h);                       btn_EQ_lowPass.setDefaultPicturePath("/btn/Button_LP_Green.png");
                                                                                         btn_EQ_lowPass.setClickedPicturePath("/btn/Button_LP_Yellow.png");
                                                                                         btn_EQ_lowPass.setAlternativePicturePath("/btn/Button_LP_Magenta.png");
    btn_EQ_bandPass.begin(_btnBP.x, _btnBP.y, _btnBP.w, _btnBP.h);                       btn_EQ_bandPass.setDefaultPicturePath("/btn/Button_BP_Green.png");
                                                                                         btn_EQ_bandPass.setClickedPicturePath("/btn/Button_BP_Yellow.png");
                                                                                         btn_EQ_bandPass.setAlternativePicturePath("/btn/Button_BP_Magenta.png");
    btn_EQ_highPass.begin(_btnHP.x, _btnHP.y, _btnHP.w, _btnHP.h);                       btn_EQ_highPass.setDefaultPicturePath("/btn/Button_HP_Green.png");
                                                                                         btn_EQ_highPass.setClickedPicturePath("/btn/Button_HP_Yellow.png");
                                                                                         btn_EQ_highPass.setAlternativePicturePath("/btn/Button_HP_Magenta.png");
    btn_EQ_balance.begin(_btnBAL.x, _btnBAL.y, _btnBAL.w, _btnBAL.h);                    btn_EQ_balance.setDefaultPicturePath("/btn/Button_BAL_Green.png");
                                                                                         btn_EQ_balance.setClickedPicturePath("/btn/Button_BAL_Yellow.png");
                                                                                         btn_EQ_balance.setAlternativePicturePath("/btn/Button_BAL_Magenta.png");
    btn_EQ_Radio.begin(   0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_Radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_EQ_Radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
                                                                                         btn_EQ_Radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_EQ_Player.begin(  1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_Player.setDefaultPicturePath("/btn/Button_Player_Green.png");
                                                                                         btn_EQ_Player.setClickedPicturePath("/btn/Button_Player_Yellow.png");
                                                                                         btn_EQ_Player.setAlternativePicturePath("/btn/Button_Player_Magenta.png");
    btn_EQ_mute.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
                                                                                         btn_EQ_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
                                                                                         btn_EQ_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
                                                                                         btn_EQ_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
                                                                                         btn_EQ_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
                                                                                         btn_EQ_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
                                                                                         btn_EQ_mute.setValue(_f_mute);
    // BLUETOOTH ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_BT_volDown.begin( 0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_volDown.setDefaultPicturePath("/btn/Button_Volume_Down_Blue.png");
                                                                                         btn_BT_volDown.setClickedPicturePath("/btn/Button_Volume_Down_Yellow.png");
    btn_BT_volUp.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_volUp.setDefaultPicturePath("/btn/Button_Volume_Up_Blue.png");
                                                                                         btn_BT_volUp.setClickedPicturePath("/btn/Button_Volume_Up_Yellow.png");
    btn_BT_pause.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
                                                                                         btn_BT_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
                                                                                         btn_BT_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
                                                                                         btn_BT_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
    btn_BT_mode.begin(    3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_mode.setDefaultPicturePath("/btn/Button_RxTx_Blue.png");
                                                                                         btn_BT_mode.setClickedPicturePath("/btn/Button_RxTx_Yellow.png");
    btn_BT_radio.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_BT_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_BT_power.begin(   5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_power.setOffPicturePath("/btn/Button_Bluetooth_Red.png");
                                                                                         btn_BT_power.setOnPicturePath("/btn/Button_Bluetooth_Blue.png");
                                                                                         btn_BT_power.setClickedOffPicturePath("/btn/Button_Bluetooth_Yellow.png");
                                                                                         btn_BT_power.setClickedOnPicturePath("/btn/Button_Bluetooth_Yellow.png");
                                                                                         btn_BT_power.setValue(_f_BTpower);
                                                                                         pic_BT_mode.setPicturePath("/common/BTnc.png");
    pic_BT_mode.begin(        _winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h,   _winLogo.pl,   _winLogo.pr,   _winLogo.pt,   _winLogo.pb); pic_BT_mode.setAlternativPicturePath("/common/BTnc.png");
    txt_BT_volume.begin(      _winFileNr.x, _winFileNr.y, _winFileNr.w, _winFileNr.h, _winFileNr.pl, _winFileNr.pr, _winFileNr.pt, _winFileNr.pb); txt_BT_volume.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_BT_volume.setFont(_fonts[2]);
    txt_BT_mode.begin(        _winName.x,   _winName.y,   _winName.w,   _winName.h,   _winName.pl,   _winName.pr,   _winName.pt,   _winName.pb); txt_BT_mode.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_BT_mode.setFont(_fonts[5]);
    // IR_SETTINGS -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_IR_radio.begin( 0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);     btn_IR_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
                                                                                         btn_IR_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    // WIFI_SETTINGS -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    cls_wifiSettings.begin(_winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _winWoHF.pl, _winWoHF.pr, _winWoHF.pt, _winWoHF.pb);
}
// clang-format off

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                              C H A N G E    S T A T E                                                                       ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format on
void changeState(int32_t state){
    disableAllObjects();
    _f_volBarVisible = false;
    if(_timeCounter.timer){setTimeCounter(0);}
    dispFooter.enable();
    dispHeader.enable();
    dispHeader.updateItem(_hl_item[state]);
    if(state != RADIO) {dispFooter.updateFlag(NULL);}
    switch(state) {
        case RADIO:{
            dispHeader.show(true);
            dispFooter.show(true);
            if(_state != RADIO) {clearWithOutHeaderFooter(); dispFooter.updateFlag(getFlagPath(_cur_station));}
            txt_RA_staName.enable();
            pic_RA_logo.enable();
            if(_radioSubMenue == 0){
                if(_f_irResultSeen){txt_RA_irNum.hide(); setStationByNumber(_irResult); _f_irResultSeen = false;} // ir_number, valid between 1 ... 999
                if(_state != RADIO) {showLogoAndStationName(true);}
                setTimeCounter(0);
                VUmeter_RA.show(true);
                txt_RA_sTitle.setText("");
                txt_RA_sTitle.show(true, false);
                _f_newStreamTitle = true;
            }
            if(_radioSubMenue == 1){ // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                clearTitle();
                txt_RA_sTitle.disable();
                sdr_RA_volume.show(true, false);
                btn_RA_mute.show();
                btn_RA_prevSta.show();   btn_RA_nextSta.show();
            //    txt_RA_staName.show(true, false);
                setTimeCounter(2);
            }
            if(_radioSubMenue == 2){ // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                btn_RA_staList.show();
                btn_RA_player.show();    btn_RA_dlna.show();  btn_RA_clock.show();
                btn_RA_sleep.show(); btn_RA_settings.show();
                btn_RA_bt.show(!_f_BtEmitterFound); btn_RA_off.show();
                setTimeCounter(2);
            }
            if(_radioSubMenue == 3){ // show Numbers from IR
                char buf[10];
                itoa(_irNumber, buf, 10);
                txt_RA_irNum.setText(buf);
                txt_RA_irNum.show(true, false);
                setTimeCounter(1);
            }
            if(_radioSubMenue == 4){ // IR select mode
                clearTitle();
                txt_RA_sTitle.disable();
                btn_RA_staList.showAlternativePic();
                btn_RA_player.show();
                btn_RA_dlna.show();
                btn_RA_clock.show();
                btn_RA_sleep.show();
                btn_RA_settings.show();
                btn_RA_bt.show(!_f_BtEmitterFound);
                btn_RA_off.show();
                setTimeCounter(2);
            }
            if(_state != RADIO) webSrv.send("changeState=", "RADIO");
            break;
        }

        case STATIONSLIST:{
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
            if(_state != PLAYER) clearWithOutHeaderFooter();
            pic_PL_logo.enable();
            if(_playerSubMenue == 0){ // prev, next, ready, play_all, shuffle, list, radio, off
                if(!_cur_AudioFolder) {x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = strdup("/audiofiles/");}
                _SD_content.listFilesInDir(_cur_AudioFolder, true, false);
                _cur_Codec = 0;
                showFileLogo(PLAYER);
                showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); txt_PL_fName.show(true, false);
                pgb_PL_progress.hide();
                sdr_PL_volume.disable();
                if(_state != PLAYER) webSrv.send("changeState=", "PLAYER");
                txt_PL_fNumber.show(true, false);
                showAudioFileNumber();
                btn_PL_prevFile.show(); btn_PL_nextFile.show(); btn_PL_ready.show(); btn_PL_playAll.show(); btn_PL_shuffle.show(); btn_PL_fileList.show(); btn_PL_radio.show(); btn_PL_off.show();
            }
            if(_playerSubMenue == 1){
                btn_PL_fileList.hide();
                btn_PL_radio.hide();
                btn_PL_off.hide();
                pgb_PL_progress.setValue(0); pgb_PL_progress.show(true, false);
                sdr_PL_volume.show(true, false);
                showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); txt_PL_fName.show(true, false);
                txt_PL_fNumber.show(true, false);
                btn_PL_mute.show();
                btn_PL_pause.setOff(); btn_PL_pause.show();
                btn_PL_cancel.show(); btn_PL_playPrev.show(); btn_PL_playNext.show();
            }
            if(_playerSubMenue == 2){ // same as submenue 0 for IR
                if(!_cur_AudioFolder) {_cur_AudioFolder = strdup("/audiofiles/"); _cur_AudioFileNr = 0;}
                _SD_content.listFilesInDir(_cur_AudioFolder, true, false);
                _cur_Codec = 0;
                showFileLogo(PLAYER);
                showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); txt_PL_fName.show(true, false);
                pgb_PL_progress.hide();
                if(_state != PLAYER) webSrv.send("changeState=", "PLAYER");
                txt_PL_fNumber.show(true, false);
                showAudioFileNumber();
                btn_PL_prevFile.showAlternativePic(); btn_PL_nextFile.show(); btn_PL_ready.show(); btn_PL_playAll.show(); btn_PL_shuffle.show(); btn_PL_fileList.show(); btn_PL_radio.show(); btn_PL_off.show();
            }
            if(_playerSubMenue == 3){ // same as submenue 1 for IR
                btn_PL_fileList.hide();
                btn_PL_radio.hide();
                btn_PL_off.hide();
                pgb_PL_progress.setValue(0); pgb_PL_progress.show(true, false);
                sdr_PL_volume.show(true, false);
                showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); txt_PL_fName.show(true, false);
                txt_PL_fNumber.show(true, false);
                btn_PL_mute.showAlternativePic();
                btn_PL_pause.setOff(); btn_PL_pause.show();
                btn_PL_cancel.show(); btn_PL_playPrev.show(); btn_PL_playNext.show();
            }
            break;
        }
        case AUDIOFILESLIST: {
            dispHeader.show(false);
            dispFooter.show(false);
            clearWithOutHeaderFooter();
            lst_PLAYER.show(_cur_AudioFolder, _cur_AudioFileNr);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case DLNA:{
            dispHeader.show(true);
            dispFooter.show(true);
            if(_state != DLNA){
                if(_state != DLNAITEMSLIST) audio.stopSong();
                clearWithOutHeaderFooter();
                pic_DL_logo.enable();
                pgb_DL_progress.setValue(0);
                pgb_DL_progress.show(true, false);
                txt_DL_fName.show(true, false);
                showFileLogo(DLNA);
            }
            if(_dlnaSubMenue == 0){ // TP submenu
                if(audio.isRunning()) btn_DL_pause.setActive(true);
                else                  btn_DL_pause.setActive(false);
                btn_DL_pause.show(); sdr_DL_volume.show(true, false); btn_DL_mute.show(); btn_DL_cancel.show(); btn_DL_fileList.show(); btn_DL_radio.show();
            }
            if(_dlnaSubMenue == 1){ // IR submenu
                if(audio.isRunning()) btn_DL_pause.setActive(true);
                else                  btn_DL_pause.setActive(false);
                btn_DL_pause.show(); sdr_DL_volume.show(true, false); btn_DL_mute.showAlternativePic(); btn_DL_cancel.show(); btn_DL_fileList.show(); btn_DL_radio.show();
            }
            if(_state != DLNA) webSrv.send("changeState=", "DLNA");
            break;
        }
        case DLNAITEMSLIST:{
            dispHeader.show(false);
            dispFooter.show(false);
            lst_DLNA.show(_currDLNAsrvNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
            setTimeCounter(LIST_TIMER);
            break;
        }
        case CLOCK:{
            dispHeader.show(false);
            dispFooter.show(false);
            if(_state != CLOCK) {bgColorWithOutHeaderFooter();}
            if(_timeFormat == 24) clk_CL_24.show();
            else                  clk_CL_12.show();
            if(_clockSubMenue == 0){
                btn_CL_mute.hide(); btn_CL_alarm.hide(); btn_CL_radio.hide(); sdr_CL_volume.disable(); btn_CL_off.hide();
            }
            if(_clockSubMenue == 1){
                setTimeCounter(2);
                btn_CL_mute.show();     btn_CL_alarm.show();    btn_CL_radio.show(); sdr_CL_volume.show(false, false); btn_CL_off.show();
            }
            if(_clockSubMenue == 2){ // same as _clockSubMenue 1 for IR
                setTimeCounter(2);
                btn_CL_alarm.showAlternativePic(); btn_CL_radio.show(); btn_CL_mute.show(); btn_CL_off.show(); sdr_CL_volume.show(false, false);
            }
            break;
        }
        case ALARMCLOCK:{
            dispHeader.show(false);
            dispFooter.show(false);
            if(_state != ALARMCLOCK) bgColorWithOutHeaderFooter();
            if(_alarmSubMenue == 0){
                btn_AC_left.show(); btn_AC_right.show(); btn_AC_up.show(); btn_AC_down.show(); btn_AC_ready.show();
                clk_AC_red.show();
            }
            if(_alarmSubMenue == 1){ // same as _alarmSubMenue for IR
                setTimeCounter(2);
                btn_AC_left.showAlternativePic(); btn_AC_right.show(); btn_AC_up.show(); btn_AC_down.show(); btn_AC_ready.show();
                clk_AC_red.show();
            }
            break;
        }
        case SLEEPTIMER:{
            dispHeader.show(true);
            dispFooter.show(true);
            if(_state != SLEEPTIMER) {
                clearWithOutHeaderFooter(); display_sleeptime();
                if(     TFT_CONTROLLER < 2) drawImage("/common/Night_Gown.bmp", 198, 23);
                else if(TFT_CONTROLLER < 7) drawImage("/common/Night_Gown.bmp", 280, 45);
                else                        drawImage("/common/Night_Gown.jpg", 482, 112);
            }
            if(_sleepTimerSubMenue == 0){
                btn_SL_up.show(); btn_SL_down.show(); btn_SL_ready.show(); btn_SL_cancel.show();
            }
            if(_sleepTimerSubMenue == 1){
                btn_SL_up.showAlternativePic(); btn_SL_down.show(); btn_SL_ready.show(); btn_SL_cancel.show();
            }
            break;
        }
        case SETTINGS:{
            dispHeader.show(true);
            dispFooter.show(true);
            if(_state != SETTINGS){
                clearWithOutHeaderFooter();
                showFileLogo(SETTINGS);
            }
            if(_settingsSubMenue == 0){
                btn_SE_bright.show(!_f_brightnessIsChangeable); btn_SE_equal.show(); btn_SE_wifi.show(); btn_SE_radio.show();
            }
            if(_settingsSubMenue == 1){
                btn_SE_bright.showAlternativePic(!_f_brightnessIsChangeable); btn_SE_equal.show(); btn_SE_wifi.show(); btn_SE_radio.show();
            }
            break;
        }
        case BRIGHTNESS:{
            dispHeader.show(false);
            dispFooter.show(false);
            if(_state != BRIGHTNESS) {clearWithOutHeaderFooter(); pic_BR_logo.show(true, false);}
            sdr_BR_value.show(true, true);
            txt_BR_value.setText(int2str(_brightness));
            txt_BR_value.show(true, true);
            if(_brightnessSubMenue == 0){
                btn_BR_ready.show();
            }
            if(_brightnessSubMenue == 1){ // same as _brightnessSubMenue for IR
                btn_BR_ready.showAlternativePic();
            }
            break;
        }
        case EQUALIZER:
        dispHeader.show(true);
        dispFooter.show(true);
            if(_state != EQUALIZER) clearWithOutHeaderFooter();
            sdr_EQ_lowPass.show(true, false); sdr_EQ_bandPass.show(true, false); sdr_EQ_highPass.show(true, false); sdr_EQ_balance.show(true, false);
            btn_EQ_lowPass.show(); btn_EQ_bandPass.show(); btn_EQ_highPass.show(); btn_EQ_balance.show(); btn_EQ_Player.show(); btn_EQ_mute.show();
            txt_EQ_lowPass.show(true, false); txt_EQ_bandPass.show(true, false); txt_EQ_highPass.show(true,  false); txt_EQ_balance.show(true, false);
            if(_equalizerSubMenue == 0){
                btn_EQ_Radio.show();
            }
            if(_equalizerSubMenue == 1){
                btn_EQ_Radio.showAlternativePic();
            }

            break;

        case BLUETOOTH: {
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            btn_BT_volUp.show(); btn_BT_volDown.show(); btn_BT_pause.show(); btn_BT_mode.show(); btn_BT_radio.show(); btn_BT_power.show(); pic_BT_mode.show(true, false);
            char* mode = strdup(bt_emitter.getMode());
            if(strcmp(mode, "RX") == 0){
                txt_BT_mode.writeText("RECEIVER");
                if(bt_emitter.isConnected()) muteChanged(true);
            }
            else {
                txt_BT_mode.writeText("EMITTER");
            }
            txt_BT_mode.setBGcolor(TFT_BROWN); txt_BT_mode.show(true, false);
            char c[10]; sprintf(c, "Vol: %02i", bt_emitter.getVolume()); txt_BT_volume.writeText(c); txt_BT_volume.show(true, false);
            if(_state != BLUETOOTH) webSrv.send("changeState=", "BLUETOOTH");
            x_ps_free(&mode);
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
            if(_ringVolume > 0){ // alarm with bell
                pic_RI_logo.enable();
                showFileLogo(RINGING);
                setTFTbrightness(_brightness);
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm");
                setVolume(_ringVolume);
                audio.setVolume(_ringVolume, _volumeCurve);
                muteChanged(false);
                connecttoFS("SD_MMC", "/ring/alarm_clock.mp3");
                clk_RI_24small.show();
            }
            else{ // alarm without bell
                _f_eof_alarm = true;
            }
            break;

        case WIFI_SETTINGS:
            dispHeader.show(true);
            dispFooter.show(true);
            clearWithOutHeaderFooter();
            cls_wifiSettings.clearText();
            cls_wifiSettings.setBorderWidth(1);
            cls_wifiSettings.setFontSize(_listFontSize);
            int16_t n = WiFi.scanNetworks();
            SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "%i WiFi networks found", n);
            for(int i = 0; i < n; i++){
                SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)", WiFi.SSID(i).c_str(), (int16_t)WiFi.RSSI(i));
                const char* pw = getWiFiPW(WiFi.SSID(i).c_str());
                // if(pw){log_e("found password %s for %s", pw, WiFi.SSID(i).c_str());}
                // else  {log_e("no password found for %s", WiFi.SSID(i).c_str());}
                cls_wifiSettings.addWiFiItems(WiFi.SSID(i).c_str(), pw);
            }
            cls_wifiSettings.show(false, false);
            break;
    }
    _state = state;
}
// clang-format on

const char* getWiFiPW(const char* ssid){
    static char* line = NULL;
    if(line) x_ps_free(&line);
    line = (char*)x_ps_malloc(128);

    static char* password = NULL;
    if(line) x_ps_free(&password);
    password = (char*)x_ps_malloc(128);
    if(!password) { log_e("oom"); return NULL;}

    for(int j = 0; j < 6; j++){
        if(j == 0) strcpy(line, pref.getString("wifiStr0").c_str());
        if(j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if(j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if(j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if(j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if(j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if(startsWith(line, ssid) && line[strlen(ssid)] == '\t') {
            if(indexOf(line, "\t", 0) > 0) {
                int idx = indexOf(line, "\t", 0);
                strcpy(password, line + idx + 1);
                // SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "SSID '%s', PW '%s'", ssid, password);
                return password;
            }
            else {
                // SerialPrintfln("WiFi: .....  " ANSI_ESC_GREEN "SSID '%s', PW '%s'", ssid, "is not set");
                return NULL;
            }
        }
    }
    return NULL;
}

bool setWiFiPW(const char* ssid, const char* password){
    if(!ssid || !password) return NULL;
    static char* line = NULL;
    if(line) x_ps_free(&line);
    line = (char*)x_ps_malloc(512);
    if(!line) { log_e("oom"); return NULL;}
    strcpy(line, ssid);
    strcat(line, "\t");
    strcat(line, password);

    for(int j = 1; j < 6; j++){
        if(j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if(j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if(j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if(j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if(j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if(startsWith(line, ssid) && line[strlen(ssid)] == '\t') {
            switch(j){
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
    for(int j = 0; j < 6; j++){
        if(j == 0) strcpy(line, pref.getString("wifiStr0").c_str());
        if(j == 1) strcpy(line, pref.getString("wifiStr1").c_str());
        if(j == 2) strcpy(line, pref.getString("wifiStr2").c_str());
        if(j == 3) strcpy(line, pref.getString("wifiStr3").c_str());
        if(j == 4) strcpy(line, pref.getString("wifiStr4").c_str());
        if(j == 5) strcpy(line, pref.getString("wifiStr5").c_str());
        if(strcmp(line, "\t") == 0) {
            switch(j){
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
    if(!_f_ESPfound)    {vTaskDelay(10);return;}    // Guard:  wrong chip?
    if(!_f_SD_MMCfound) {vTaskDelay(10); return;}   // Guard:  SD_MMC could not be initialisized

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

    if(_logBuffer.size() > 0){
        size_t i = _logBuffer.size();
        webSrv.send("serTerminal=", _logBuffer[i - 1].c_get());
        _logBuffer.pop_back();
        if(_logBuffer.size() == 0) _logBuffer.clear(); // Löscht alle Elemente und gibt den Speicher frei
        return;
    }


    if(_f_dlnaBrowseServer) {_f_dlnaBrowseServer = false; dlna.browseServer(_currDLNAsrvNr, _dlnaHistory[_dlnaLevel].objId, _totalNumberReturned);}
    if(_f_clearLogo)        {_f_clearLogo = false; clearLogo();}
    if(_f_clearStationName) { _f_clearStationName = false; clearStationName();}

    if(_f_playlistEnabled) {
        if(!_f_playlistNextFile) {
            if(!audio.isRunning() && !_f_pauseResume) {
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
    //-----------------------------------------------------0.1 SEC------------------------------------------------------------------------------------
    if(_f_100ms) { // calls every 0.1 second
        _f_100ms = false;

        if(_state == RADIO && _radioSubMenue == 0) VUmeter_RA.update(audio.getVUlevel());

        static uint8_t factor = 0;
        static bool f_tc = false;
        if(factor > 0){
            factor --;
        }
        else{
            if(_timeCounter.timer > 0){
                factor = _timeCounter.factor;
                dispFooter.updateTC(_timeCounter.timer);
                _timeCounter.timer --;
                f_tc = true;
            }
            else{
                if(f_tc){
                    f_tc = false;
                    dispFooter.updateTC(0);
                    if(_f_sleeping) return; // tc is active by pressing a button, but do nothing if "off"

                    if(_state == RADIO) {
                                            if(!txt_RA_staName.isEnabled()){  txt_RA_staName.show(true, false); }// assume volBox is shown
                                            if(_radioSubMenue == 1) { _radioSubMenue = 0; changeState(RADIO);} // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                                            if(_radioSubMenue == 2) { _radioSubMenue = 0; changeState(RADIO);} // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                                            if(_radioSubMenue == 3) { _radioSubMenue = 0; changeState(RADIO);} // show Numbers from IR
                                            if(_radioSubMenue == 4) { _radioSubMenue = 0; changeState(RADIO);} // IR select mode
                    }
                    else if(_state == STATIONSLIST) {
                                            _radioSubMenue = 0; changeState(RADIO);
                    }
                    else if(_state == PLAYER){
                                            if(!txt_PL_fName.isEnabled()){txt_PL_fName.show(true, false);} // assume volBox is shown
                                            if(_playerSubMenue == 2) {_playerSubMenue = 0; changeState(PLAYER);}
                                            if(_playerSubMenue == 3){ _playerSubMenue = 1; changeState(PLAYER);}
                    }
                    else if(_state == AUDIOFILESLIST) {
                                            _playerSubMenue = 0; changeState(PLAYER);
                    }
                    else if(_state == DLNA) {
                                            if(!txt_DL_fName.isEnabled()){ txt_DL_fName.show(true, false);} // assume volBox is shown
                                            if(_dlnaSubMenue == 1) {_dlnaSubMenue = 0; changeState(DLNA);}
                    }
                    else if(_state == DLNAITEMSLIST) {
                                            _dlnaSubMenue = 0;       changeState(DLNA);
                    }
                    else if(_state == CLOCK){
                                            _clockSubMenue = 0;      changeState(CLOCK);
                    }
                    else if(_state == ALARMCLOCK){
                                            _alarmSubMenue = 0;      changeState(ALARMCLOCK);
                    }
                    else if(_state == SETTINGS){
                                            _settingsSubMenue = 0;   changeState(SETTINGS);
                    }
                    else if(_state == BRIGHTNESS){
                                            _brightnessSubMenue = 0; changeState(BRIGHTNESS);
                    }
                    else if(_state == EQUALIZER){
                                            _equalizerSubMenue = 0;  changeState(EQUALIZER);
                    }
                    else { ; } // all other, do nothing
                }
            }
        }

        if(!_f_rtc) {
            _f_rtc = rtc.hasValidTime();
        }

        int16_t audioVol = audio.getVolume();
        //------------------------------------------------------------------------
        if(AMP_ENABLED >= 0){                // An external amplifier can be switched, if a GPIO has been assigned.
            static bool av = false;          // The amplifier switches off when the real volume (not _cur_volume) reaches the value 0.
            if((audioVol > 0) != av){        // This is also the case in sleep mode.
                av = (audioVol > 0);
                digitalWrite(AMP_ENABLED, av);
                if(av)  {SerialPrintfln("Amplifier:   " ANSI_ESC_YELLOW "The external amplifier was activated");}
                else    {SerialPrintfln("Amplifier:   " ANSI_ESC_YELLOW "The external amplifier was deactivated");}
            }
        }
        //------------------------------------------------------------------------
        uint8_t currVol = _cur_volume;
        if(_f_mute) currVol = 0;
        uint8_t steps = _volumeSteps / 7;
        if (audioVol > currVol){ // downvolume
            if(audioVol - steps >= currVol ) {if(audioVol - steps <   0) audio.setVolume(0, _volumeCurve);   else  audio.setVolume(audioVol- steps, _volumeCurve);}
            else audio.setVolume(audioVol - 1, _volumeCurve);
        }
        if (audioVol < currVol){ // upvolume
            if(audioVol + steps <= currVol) {if(audioVol + steps > 255) audio.setVolume(255, _volumeCurve); else audio.setVolume(audioVol + steps, _volumeCurve);}
            else {audio.setVolume(audioVol + 1, _volumeCurve);}
        }

    }
    //-----------------------------------------------------1 SEC--------------------------------------------------------------------------------------

    if(_f_1sec) { // calls every second
        _f_1sec = false;
        _totalRuntime++;
        uint16_t minuteOfTheDay = rtc.getMinuteOfTheDay();
        uint8_t  weekDay = rtc.getweekday();
        if(_timeFormat == 24) clk_CL_24.updateTime(minuteOfTheDay, weekDay);
        else                  clk_CL_12.updateTime(minuteOfTheDay, weekDay);
        if(_state == RINGING) clk_RI_24small.updateTime(minuteOfTheDay, weekDay);
        static uint8_t semaphore = 0;
        if(!semaphore) {_f_alarm = isAlarm(weekDay, _alarmdays, minuteOfTheDay, _alarmtime) && _f_rtc;} // alarm if rtc and CL green
        if(_f_alarm)        {semaphore++;}
        if(semaphore)       {semaphore++;}
        if(semaphore >= 65) {semaphore = 0;}

        //------------------------------------------ALARM MANAGEMENT----------------------------------------------------------------------------------
        if(_f_alarm) {
            _f_alarm = false;
            changeState(RINGING);
        }
        if(_f_eof_alarm) { // AFTER RINGING
            _f_eof_alarm = false;
            if(!_f_rtc) return;
            _cur_volume = _volumeAfterAlarm;
            setVolume(_cur_volume);
            audio.setVolume(_cur_volume, _volumeCurve);
            dispHeader.updateVolume(_cur_volume);
            wake_up();
        }

        if(_f_stationsChanged) {
            _f_stationsChanged = false;
            staMgnt.updateStationsList();
        }

        //------------------------------------------UPDATE DISPLAY------------------------------------------------------------------------------------
        if(!_f_sleeping || _state == RINGING) {
            dispHeader.updateTime(_time_s, false);
            dispHeader.updateRSSI(WiFi.RSSI());

            if(_f_newBitRate) {
               _f_newBitRate = false;
               dispFooter.updateBitRate(_icyBitRate);
            }
            if(_f_newStationName) {
                _f_newStationName = false;
                showLogoAndStationName(false);
            }
        }
        //---------------------------------------------TIME SPEECH -----------------------------------------------------------------------------------
        static bool f_resume = false;
        if(_f_timeSpeech){ // speech the time 7 sec before a new hour is arrived
            _f_timeSpeech =  false;
            int hour = atoi(_time_s); hour++; if(hour == 24) hour = 0; //  extract the hour
            if(_f_mute) return;
            if(_f_sleeping) return;
            if(_state != RADIO) return;
            if(_f_timeAnnouncement) {
                f_resume = true;
                _f_eof = false;
                if(_timeFormat == 12) {if(hour > 12) hour -= 12;}
                sprintf(_chbuf, "/voice_time/%s/%d_00.mp3", _timeSpeechLang, hour);
                SerialPrintfln("Time: ...... play Audiofile %s", _chbuf); connecttoFS("SD_MMC", _chbuf);
                return;
            }
            else { SerialPrintfln("Time: ...... Announcement at %d o'clock is silent", hour); }
        }
        if(f_resume && _f_eof){
            f_resume = false;
            _f_eof = false;
            if(_cur_station) setStation(_cur_station);
            else connecttohost(_settings.lastconnectedhost);
            return;
        }
        //------------------------------------------AUDIO_CURRENT_TIME - DURATION---------------------------------------------------------------------
        if(audio.isRunning()){
            _audioFileDuration = audio.getAudioFileDuration();
            if(_audioFileDuration > 0){
                _audioCurrentTime = audio.getAudioCurrentTime();
                if(_state == PLAYER && _audioFileDuration){pgb_PL_progress.setNewMinMaxVal(0, _audioFileDuration); pgb_PL_progress.setValue(_audioCurrentTime);}
                if(_state == DLNA &&   _audioFileDuration){pgb_DL_progress.setNewMinMaxVal(0, _audioFileDuration); pgb_DL_progress.setValue(_audioCurrentTime);}
                if(_audioFileDuration) {
                    SerialPrintfcr("AUDIO_FILE:  " ANSI_ESC_GREEN "AudioCurrentTime " ANSI_ESC_GREEN "%li:%02lis, " ANSI_ESC_GREEN "AudioFileDuration " ANSI_ESC_GREEN "%li:%02lis",
                                   (long int)_audioCurrentTime / 60, (long int)_audioCurrentTime % 60, (long int)_audioFileDuration / 60, (long int)_audioFileDuration % 60);
                }
            }
        }
        //------------------------------------------NEW STREAMTITLE-----------------------------------------------------------------------------------
        if(_f_newStreamTitle && !_timeCounter.timer) {
            _f_newStreamTitle = false;
            if(_state == RADIO) {
                if(strlen(_streamTitle)) showStreamTitle(_streamTitle);
                else if(strlen(_icyDescription)) {
                    showStreamTitle(_icyDescription);
                    _f_newIcyDescription = false;
                    webSrv.send("icy_description=", _icyDescription);
                }
                else txt_RA_sTitle.writeText("");
            }
            webSrv.send("streamtitle=", _streamTitle);
        }
        //------------------------------------------NEW ICY-DESCRIPTION-------------------------------------------------------------------------------
        if(_f_newIcyDescription && !_timeCounter.timer) {
            if(_state == RADIO) {
                if(!strlen(_streamTitle)) showStreamTitle(_icyDescription);
            }
            webSrv.send("icy_description=", _icyDescription);
            _f_newIcyDescription = false;
        }
        //------------------------------------------NEW COMMERCIALS-----------------------------------------------------------------------------------
        if(_f_newCommercial && !_timeCounter.timer) {
            if(_state == RADIO) { showStreamTitle(_commercial); }
            webSrv.send("streamtitle=", _commercial);
            _f_newCommercial = false;
        }
        //------------------------------------------END OF COMMERCIALS--------------------------------------------------------------------------------
        if(_commercial_dur > 0) {
            _commercial_dur--;
            if((_commercial_dur == 2) && (_state == RADIO)) /* showStreamTitle(""); */ _f_newStreamTitle = true; // end of commercial? clear streamtitle
        }
        //------------------------------------------DETERMINE AUDIOCODEC------------------------------------------------------------------------------
        if(_cur_Codec == 0) {
            uint8_t c = audio.getCodec();
            if(c != 0 && c != 8 && c < 10) { // unknown or OGG, guard: c {1 ... 7, 9}
                _cur_Codec = c;
                SerialPrintfln("Audiocodec:  " ANSI_ESC_YELLOW "%s", codecname[c]);
                if(_state == PLAYER) showFileLogo(PLAYER);
                if(_state == RADIO && _f_logoUnknown == true) {
                    _f_logoUnknown = false;
                    showFileLogo(_state);
                }
            }
        }
        //------------------------------------------CONNECT TO LASTHOST-------------------------------------------------------------------------------
        if(_f_connectToLastStation){ // not used yet
            _f_connectToLastStation = false;
            if(_cur_station) setStation(_cur_station);
            else connecttohost(_settings.lastconnectedhost);
        }
        //------------------------------------------RECONNECT AFTER FAIL------------------------------------------------------------------------------
        if(_f_reconnect && !_f_WiFiConnected){
            _f_reconnect = false;
            connecttohost(_settings.lastconnectedhost);
        }
        //------------------------------------------SEEK DLNA SERVER----------------------------------------------------------------------------------
        if(_f_dlnaSeekServer) {
            _f_dlnaSeekServer = false;
            dlna.seekServer();
        }
        //------------------------------------------CREATE DLNA PLAYLIST------------------------------------------------------------------------------
        if(_f_dlnaMakePlaylistOTF && _f_dlna_browseReady){
            _f_dlnaMakePlaylistOTF = false;
            _f_dlna_browseReady = false;
            if(preparePlaylistFromDLNAFolder()) processPlaylist(true);
        }
        //------------------------------------------DLNA ITEMS RECEIVED-------------------------------------------------------------------------------
        if(_f_dlna_browseReady){ // unused
            _f_dlna_browseReady = false;
        }
        //-------------------------------------------WIFI DISCONNECTED?-------------------------------------------------------------------------------
        if(_f_WiFiConnected){
            if((WiFi.status() != WL_CONNECTED)) {
                _WiFi_disconnectCnt ++;
                if(_WiFi_disconnectCnt == 15){
                    _WiFi_disconnectCnt = 1;
                    SerialPrintfln("WiFi      :  " ANSI_ESC_YELLOW "Reconnecting to WiFi...");
                    WiFi.disconnect();
                    WiFi.reconnect();
                }
            }
            else{
                if(_WiFi_disconnectCnt){
                    _WiFi_disconnectCnt = 0;
                    if(_state == RADIO) audio.connecttohost(_settings.lastconnectedhost);
                }
            }
        }
        //------------------------------------------CONNECTTOHOST FAIL--------------------------------------------------------------------------------
        static uint8_t failCnt = 0;
        if(_f_webFailed){
            failCnt++;
            if(failCnt == 30){
                failCnt = 0;
                if(WiFi.isConnected()) connecttohost(_settings.lastconnectedhost);
                else ESP.restart();
            }
        }
        else failCnt = 0;
        //------------------------------------------GET AUDIO FILE ITEMS------------------------------------------------------------------------------
        if(_f_isFSConnected) {
            //    uint32_t t = 0;
            //    uint32_t fs = audioGetFileSize();
            //    uint32_t br = audioGetBitRate();
            //    if(br) t = (fs * 8)/ br;
            //    log_w("Br %d, Dur %ds", br, t);
        }
        //--------------------------------------AMBIENT LIGHT SENSOR BH1750---------------------------------------------------------------------------
        if(_f_BH1750_found){
            int32_t ambVal = BH1750.getBrightness();
            if(ambVal < 0) goto endbrightness;
            if(ambVal > 350) ambVal = 350;
            _bh1750Value = map_l(ambVal, 0, 350, 5, 100);
        //    log_i("_bh1750Value %i, _brightness %i", _bh1750Value, _brightness);
            if(!_f_sleeping){
                if(_bh1750Value >= _brightness) setTFTbrightness(_bh1750Value);
                else setTFTbrightness(_brightness);
            }
endbrightness:
            BH1750.start();
        }
    } //  END _f_1sec


    if(_f_10sec == true) { // calls every 10 seconds
        _f_10sec = false;
        if(_state == RADIO && !_icyBitRate && !_f_sleeping) {
            _decoderBitRate = audio.getBitRate();
            static uint32_t oldBr = 0;
            if(_decoderBitRate != oldBr){
                oldBr = _decoderBitRate;
                dispFooter.updateBitRate(_decoderBitRate / 1000);
            }
        }
        updateSettings();
    }

    if(_f_1min == true) {  // calls every minute
        _f_1min = false;
        if(_sleeptime){
            _sleeptime--;
            if(!_sleeptime) fall_asleep();
            dispFooter.updateOffTime(_sleeptime);
        }
        static uint8_t btEmitterCnt = 0;
        if(!_f_BtEmitterFound && btEmitterCnt < 1){
            btEmitterCnt++;
            bt_emitter.begin(); // if the emitter has not yet responded
        }
    }

    //-------------------------------------------------DEBUG / WIFI_SETTINGS ----------------------------------------------------------------------------------
    if(Serial.available()) { // input: serial terminal
        String r = Serial.readString();
        r.replace("\n", "");
        SerialPrintfln("Terminal  :  " ANSI_ESC_YELLOW "%s", r.c_str());
        if(r.startsWith("pr")) {
            _f_pauseResume = audio.pauseResume();
            if(_f_pauseResume) { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume"); }
            else { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume not possible"); }
        }
        if(r.startsWith("hc")) { // A hardcopy of the display is created and written to the SD card
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "create hardcopy"); }
            hardcopy();
        }
        if(r.startsWith("rts")) {
            char* timeStatsBuffer = x_ps_calloc(2000, sizeof(char));
            GetRunTimeStats(timeStatsBuffer);
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "task statistics\n\n%s", timeStatsBuffer); }
            x_ps_free(&timeStatsBuffer);
        }
        if(r.startsWith("cts")) {
           audio.connecttospeech("Hallo, wie geht es dir? Morgen scheint die Sonne und übermorgen regnet es.Aber wir nehmen den Regenschirm mit. Und auch den Rucksack. Dann lesen wir aus dem Buch Hier gibt es nur gutes Wetter.", "de");
        //    audio.connecttospeech("Hallo", "de");
        }

        if(r.toInt() != 0) { // is integer?
            if(audio.setTimeOffset(r.toInt())) { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "TimeOffset %li", r.toInt()); }
            else { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "TimeOffset not possible"); }
        }
        if(r.startsWith("bfi")){ // buffer filled
            SerialPrintfln("inBuffer  :  filled %lu bytes", (long unsigned)audio.inBufferFilled());
            SerialPrintfln("inBuffer  :  free   %lu bytes", (long unsigned)audio.inBufferFree());
        }
        if(r.startsWith("st")){ // testtext for streamtitle
            if(r[2] == '0') strcpy(_streamTitle, "A Ё Ю");
            if(r[2] == '1') strcpy(_streamTitle, "A B C D E F G");
            if(r[2] == '2') strcpy(_streamTitle, "A B C D E F G H I");
            if(r[2] == '3') strcpy(_streamTitle, "A B C D E F G H I J K L");
            if(r[2] == '4') strcpy(_streamTitle, "A B C D E F G H I J K J M Q O");
            if(r[2] == '5') strcpy(_streamTitle, "A B C D E F G H I K L J M y O P Q R");
            if(r[2] == '6') strcpy(_streamTitle, "A B C D E F G H I K L J M g O P Q R S T V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '7') strcpy(_streamTitle, "A B C D E F G H I K L J M j O P Q R S T U V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '8') strcpy(_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '9') strcpy(_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            log_w("st: %s", _streamTitle);
            _f_newStreamTitle = true;
        }
        if(r.startsWith("ais")){ // openAIspeech
            log_w("openAI speech");
        //    audio.openai_speech("openAI-key", "tts-1", "Today is a wonderful day to build something people love!", "", "shimer", "mp3", "1");
        }
        // if(r.startsWith("ctfs")){ // connecttoFS
        //     log_w("SPIFFS");
        //     connecttoFS("SPIFFS", "/Collide.ogg");
        // }

        if(r.startsWith("sapp")){ // setAudioPlayPosition
            uint32_t pos = r.substring(4, r.length() -1).toInt();
            log_w("setAudioPlayPosition %lu", pos);
            audio.setAudioPlayPosition(pos);
        }

        if(r.startsWith("grn")){ // list of all self registered objects
            get_registered_names();
        }
        if(r.startsWith("fm")){ // force mono
            static bool f_mono = false;
            f_mono = !f_mono;
            audio.forceMono(f_mono);
            if(f_mono) log_w("mono"); else log_w("stereo");
        }
        if(r.startsWith("btp")){ // bluetooth RX/TX protocol
            uint16_t i = 0;
            while(bt_emitter.list_protokol(i)){
                log_e("%s", bt_emitter.list_protokol(i));
                i++;
            }
        }
        if(r.startsWith("btstr")){ // bluetooth string, send to bt emitter e.g. btstr:AT+
            bt_emitter.userCommand(r.substring(6, r.length() -1).c_str());
            log_w("btstr: %s", r.substring(6, r.length() -1).c_str());
        }
        if(r.startsWith("tsp")){
            _f_timeSpeech = true;
        }
        if(r.startsWith("pwd")){ // set password for WiFi
            changeState(WIFI_SETTINGS);
        }
        if(r.startsWith("gif")){ // draw gif image
            log_w("gif");
            drawImage("/common/Tom_Jerry.gif", 100, 100);
        }
        if(r.startsWith("bfi")){ // buffer filled
            log_w("inBuffer  :  filled %lu bytes", (long unsigned)audio.inBufferFilled());
            log_w("inBuffer  :  free   %lu bytes", (long unsigned)audio.inBufferFree());
        }
    }
}

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                  E V E N T S                                                                                ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// Events from audioI2S library
void my_audio_info(Audio::msg_t m) {
    switch(m.e){
        case Audio::evt_info:
            if(endsWith(m.msg, "failed!"))                   {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_YELLOW, m.msg); sprintf(_streamTitle, "" ANSI_ESC_ORANGE "%s", m.msg);
                                                                                                                     _f_newStreamTitle = true; _f_webFailed = true; return;}
            if(startsWith(m.msg, "FLAC"))                    {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN,  m.msg); return;}
            if(endsWith(  m.msg, "Stream lost"))             {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_YELLOW, m.msg); return;}
            if(startsWith(m.msg, "authent"))                 {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN,  m.msg); return;}
            if(startsWith(m.msg, "StreamTitle="))            {return;}
            if(startsWith(m.msg, "HTTP/") && m.msg[9] > '3') {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED,    m.msg); return;}
            if(startsWith(m.msg, "ERROR:"))                  {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED,    m.msg); return;}
            if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN)   {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN,  m.msg); return;} // all other
            break;

        case Audio::evt_name:
            x_ps_free(&_stationName_air);
            _stationName_air = x_ps_strndup(m.msg, 200); // set max length
            SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", m.msg);
            _f_newStationName = true;
            break;

        case Audio::evt_streamtitle:
            strcpy(_streamTitle, m.msg);
            if(!_f_irNumberSeen) _f_newStreamTitle = true;
            SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", m.msg);
            break;

        case Audio::evt_eof:
            _f_isWebConnected = false;
            _f_eof = true;
            _f_isFSConnected = false;
            SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, m.msg);
            if(_state == PLAYER) {
                webSrv.send("SD_playFile=", "end of audiofile");
                if(!_f_playlistEnabled) {
                    _f_clearLogo = true;
                    _f_clearStationName = true;
                    _playerSubMenue = 0;
                    changeState(PLAYER);
                }
            }
            if(_state == RADIO)  { clearWithOutHeaderFooter(); }
            if(_state == DLNA)   { txt_DL_fName.setText(""); txt_DL_fName.show(true, false); btn_DL_pause.setActive(false); btn_DL_pause.show();}
            _f_eof = true;
            break;

        case Audio::evt_lasthost:
            if(_f_playlistEnabled) return;
            x_ps_free(&_settings.lastconnectedhost);
            _settings.lastconnectedhost = x_ps_strdup(m.msg);
            SerialPrintflnCut("lastURL: ..  ", ANSI_ESC_WHITE, _settings.lastconnectedhost);
            webSrv.send("stationURL=", _settings.lastconnectedhost);
            break;

        case Audio::evt_icyurl:
            if(strlen(m.msg) > 5) {
                SerialPrintflnCut("icy-url: ..  ", ANSI_ESC_WHITE, m.msg);
                _homepage = String(m.msg);
                if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
            }
            break;

        case Audio::evt_icylogo:
            if(strlen(m.msg) > 5) { SerialPrintflnCut("icy-logo:    ", ANSI_ESC_WHITE, m.msg); }
            break;

        case Audio::evt_id3data:
            SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s", m.msg);
            break;

        case Audio::evt_image:
            for(int i = 0; i < m.vec.size(); i += 2){
                SerialPrintfln("CoverImage:  " ANSI_ESC_GREEN "segment %02i, pos %08i, len %08i", i / 2, m.vec[i], m.vec[i + 1]);}
             break;

        case Audio::evt_icydescription:
            strcpy(_icyDescription, m.msg);
            _f_newIcyDescription = true;
            if(strlen(m.msg)) SerialPrintfln("icy-descr:   %s", m.msg);
            break;

        case Audio::evt_bitrate:
            if(!strlen(m.msg)) return; // guard
            _icyBitRate = str2int(m.msg);
            _f_newBitRate = true;
            SerialPrintfln("bitRate:     " ANSI_ESC_GREEN "%i", _icyBitRate);
            break;

        case Audio::evt_lyrics:
            SerialPrintfln("sync lyrics: " ANSI_ESC_CYAN "%s", m.msg);
            if(_state == RADIO) showStreamTitle(m.msg); // web file
            if(_state == PLAYER) showFileName(m.msg);
            break;

        case Audio::evt_log:
            SerialPrintfln("%s      :   %s", m.s, m.msg);
            break;

        default: SerialPrintfln("message:...  %s", m.msg); break;
    }
}


//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_process_i2s(int16_t* outBuff, uint16_t validSamples, bool *continueI2S){

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
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ftp_debug(const char* info) {
    if(startsWith(info, "File Name")) return;
    SerialPrintfln("ftpServer:   %s", info);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Events from rtime library
void RTIME_info(const char* info) { SerialPrintfln("rtime_info:  %s", info); }
// Events from tft library
void tft_info(const char* info) { SerialPrintfln("tft_info: .  %s", info); }
// Events from tp library
void tp_info(const char* info) { SerialPrintfln("tp_info: ..  %s", info); }
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
    if(_state != RADIO) return;
    if(_f_sleeping == true) return;
    SerialPrintfln("ir_result:   " ANSI_ESC_YELLOW "Stationnumber " ANSI_ESC_BLUE "%lu", (long unsigned)res);
    _f_irResultSeen = true;
    _f_irNumberSeen = false;
    _irResult = res;
    return;
}
void ir_number(uint16_t num) {
    if(_state != RADIO) return;
    if(_f_sleeping) return;
    _f_irNumberSeen = true;
    _irNumber = num;
    _radioSubMenue = 3;
    changeState(RADIO);
}
void ir_short_key(uint8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "short pressed key nr: " ANSI_ESC_BLUE "%02i", key);
    if(_f_sleeping == true && ! _f_irOnOff){
        wake_up();
        return;
    }
    if(_f_irOnOff == true && key != 20) return;
    if(_state == IR_SETTINGS) return;  // nothing todo
    static uint8_t btnNr = 0;
    switch(key) {
        case 10:     // MUTE
                    muteChanged(!_f_mute);
                    break;
        case 11:    // ARROW RIGHT
                    if(_state == RADIO) {
                        if(_radioSubMenue < 4){
                            nextFavStation(); return;                                  // NEXT STATION
                        }
                        if(_radioSubMenue == 4){ // scroll forward
                            if(btnNr > 7) btnNr = 0; // guard
                            btnNr++;
                            if(btnNr == 1){btn_RA_staList.show(); btn_RA_player.showAlternativePic();}
                            if(btnNr == 2){btn_RA_player.show(); btn_RA_dlna.showAlternativePic();}
                            if(btnNr == 3){btn_RA_dlna.show(); btn_RA_clock.showAlternativePic();}
                            if(btnNr == 4){btn_RA_clock.show(); btn_RA_sleep.showAlternativePic();}
                            if(btnNr == 5){btn_RA_sleep.show(); btn_RA_settings.showAlternativePic();}
                            if(btnNr == 6){btn_RA_settings.show(); btn_RA_bt.showAlternativePic();}
                            if(btnNr == 7){btn_RA_bt.show(); btn_RA_off.showAlternativePic();}
                            if(btnNr == 8){btn_RA_off.show(); btn_RA_staList.showAlternativePic(); btnNr = 0;}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == STATIONSLIST) {    // next page
                        lst_RADIO.nextPage(); setTimeCounter(LIST_TIMER);
                        return;
                    }
                    if(_state == PLAYER){
                        if(_playerSubMenue == 2){ // scroll forward
                            if(btnNr > 7) btnNr = 0; // guard
                            btnNr++;
                            if(btnNr == 1){btn_PL_prevFile.show(); btn_PL_nextFile.showAlternativePic();}
                            if(btnNr == 2){btn_PL_nextFile.show(); btn_PL_ready.showAlternativePic();}
                            if(btnNr == 3){btn_PL_ready.show(); btn_PL_playAll.showAlternativePic();}
                            if(btnNr == 4){btn_PL_playAll.show(); btn_PL_shuffle.showAlternativePic();}
                            if(btnNr == 5){btn_PL_shuffle.show(); btn_PL_fileList.showAlternativePic();}
                            if(btnNr == 6){btn_PL_fileList.show(); btn_PL_radio.showAlternativePic();}
                            if(btnNr == 7){btn_PL_radio.show(); btn_PL_off.showAlternativePic();}
                            if(btnNr == 8){btn_PL_off.show(); btn_PL_prevFile.showAlternativePic(); btnNr = 0;}
                            setTimeCounter(2);
                            return;
                        }
                        if(_playerSubMenue == 3){ // scroll forward (mute, pause, cancel, prev, next)
                            if(btnNr < 4) btnNr++;
                            if(btnNr == 1){btn_PL_mute.show(); btn_PL_pause.showAlternativePic();}
                            if(btnNr == 2){btn_PL_pause.show(); btn_PL_cancel.showAlternativePic();}
                            if(btnNr == 3){btn_PL_cancel.show(); btn_PL_playPrev.showAlternativePic();}
                            if(btnNr == 4){btn_PL_playPrev.show(); btn_PL_playNext.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == AUDIOFILESLIST) {
                            lst_PLAYER.nextPage(); setTimeCounter(LIST_TIMER); return; // next page
                    }
                    if(_state == DLNA){
                        if(_dlnaSubMenue == 1){ // scroll forward (mute, pause, cancel, prev, next)
                            if(btnNr < 4) btnNr++;
                            if(btnNr == 1){btn_DL_mute.show(); btn_DL_pause.showAlternativePic();}
                            if(btnNr == 2){btn_DL_pause.show(); btn_DL_cancel.showAlternativePic();}
                            if(btnNr == 3){btn_DL_cancel.show(); btn_DL_fileList.showAlternativePic();}
                            if(btnNr == 4){btn_DL_fileList.show(); btn_DL_radio.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == DLNAITEMSLIST){
                        lst_DLNA.nextPage(); setTimeCounter(LIST_TIMER); return; // nextpage
                    }
                    if(_state == CLOCK){
                        if(_clockSubMenue == 2){ // scroll forward (alarm, radio, mute, off)
                            if(btnNr < 3) btnNr++;
                            if(btnNr == 1){btn_CL_alarm.show(); btn_CL_radio.showAlternativePic();}
                            if(btnNr == 2){btn_CL_radio.show(); btn_CL_mute.showAlternativePic();}
                            if(btnNr == 3){btn_CL_mute.show(); btn_CL_off.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == ALARMCLOCK){
                        if(_alarmSubMenue == 1){ // scroll forward (left, right, up, down, ready)
                            if(btnNr < 4) btnNr++;
                            if(btnNr == 1){btn_AC_left.show(); btn_AC_right.showAlternativePic();}
                            if(btnNr == 2){btn_AC_right.show(); btn_AC_up.showAlternativePic();}
                            if(btnNr == 3){btn_AC_up.show(); btn_AC_down.showAlternativePic();}
                            if(btnNr == 4){btn_AC_down.show(); btn_AC_ready.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == SLEEPTIMER){
                        if(_sleepTimerSubMenue == 1){ // scroll forward (up, down, ready, cancel)
                            if(btnNr < 3) btnNr++;
                            if(btnNr == 1){btn_SL_up.show(); btn_SL_down.showAlternativePic();}
                            if(btnNr == 2){btn_SL_down.show(); btn_SL_ready.showAlternativePic();}
                            if(btnNr == 3){btn_SL_ready.show(); btn_SL_cancel.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == SETTINGS){
                        if(_settingsSubMenue == 1){ // scroll forward (bright, equal, wifi, radio)
                            if(btnNr < 3) btnNr++;
                            if(btnNr == 1){btn_SE_bright.show(); btn_SE_equal.showAlternativePic();}
                            if(btnNr == 2){btn_SE_equal.show(); btn_SE_wifi.showAlternativePic();}
                            if(btnNr == 3){btn_SE_wifi.show(); btn_SE_radio.showAlternativePic();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == BRIGHTNESS){
                        if(_brightnessSubMenue == 1){
                            _brightness += 5;
                            if(_brightness > 100) _brightness = 100;
                            sdr_BR_value.setValue(_brightness); setTimeCounter(2);
                        }
                    }
                    if(_state == EQUALIZER){
                        if(_equalizerSubMenue == 1){
                            char c[10];
                            if(btnNr == 10){if(_toneBAL < 16){_toneBAL++; itoa(_toneBAL, c, 10); txt_EQ_balance.writeText(c); sdr_EQ_balance.setValue(_toneBAL);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // balance
                            if(btnNr == 20){if(_toneLP < 6){_toneLP++; itoa(_toneLP, c, 10); txt_EQ_lowPass.writeText( c); sdr_EQ_lowPass.setValue(_toneLP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // HP
                            if(btnNr == 30){if(_toneBP < 6){_toneBP++; itoa(_toneBP, c, 10); txt_EQ_bandPass.writeText(c); sdr_EQ_bandPass.setValue(_toneBP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // BP
                            if(btnNr == 40){if(_toneHP < 6){_toneHP++; itoa(_toneHP, c, 10); txt_EQ_highPass.writeText(c); sdr_EQ_highPass.setValue(_toneHP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // LP
                            if(btnNr == 1) {btnNr = 2; btn_EQ_Player.show(); btn_EQ_mute.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 0) {btnNr = 1; btn_EQ_Radio.show(); btn_EQ_Player.showAlternativePic(); setTimeCounter(2); return;}
                        }
                    }
                    break;
        case 12:    // ARROW LEFT
                    if(_state == RADIO) {
                        if(_radioSubMenue < 4){
                            prevFavStation(); return;               // PREV STATION
                        }
                        if(_radioSubMenue == 4){ // scroll backward
                            if(btnNr > 7) btnNr = 0; // guard
                            if(btnNr == 0) btnNr = 8;
                            btnNr--;
                            if(btnNr == 0){btn_RA_staList.showAlternativePic(); btn_RA_player.show();}
                            if(btnNr == 1){btn_RA_player.showAlternativePic(); btn_RA_dlna.show();}
                            if(btnNr == 2){btn_RA_dlna.showAlternativePic(); btn_RA_clock.show();}
                            if(btnNr == 3){btn_RA_clock.showAlternativePic(); btn_RA_sleep.show();}
                            if(btnNr == 4){btn_RA_sleep.showAlternativePic(); btn_RA_settings.show();}
                            if(btnNr == 5){btn_RA_settings.showAlternativePic(); btn_RA_bt.show();}
                            if(btnNr == 6){btn_RA_bt.showAlternativePic(); btn_RA_off.show();}
                            if(btnNr == 7){btn_RA_off.showAlternativePic(); btn_RA_staList.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == STATIONSLIST) {lst_RADIO.prevPage(); setTimeCounter(LIST_TIMER); break;}    // prev page
                    if(_state == PLAYER){
                        if(_playerSubMenue == 2){ // scroll backward
                            if(btnNr > 7) btnNr = 0; // guard
                            if(btnNr == 0) btnNr = 8;
                            btnNr--;
                            if(btnNr == 0){btn_PL_prevFile.showAlternativePic(); btn_PL_nextFile.show();}
                            if(btnNr == 1){btn_PL_nextFile.showAlternativePic(); btn_PL_ready.show();}
                            if(btnNr == 2){btn_PL_ready.showAlternativePic(); btn_PL_playAll.show();}
                            if(btnNr == 3){btn_PL_playAll.showAlternativePic(); btn_PL_shuffle.show();}
                            if(btnNr == 4){btn_PL_shuffle.showAlternativePic(); btn_PL_fileList.show();}
                            if(btnNr == 5){btn_PL_fileList.showAlternativePic(); btn_PL_radio.show();}
                            if(btnNr == 6){btn_PL_radio.showAlternativePic(); btn_PL_off.show();}
                            if(btnNr == 7){btn_PL_off.showAlternativePic(); btn_PL_prevFile.show();}
                            setTimeCounter(2);
                            return;
                        }
                        if(_playerSubMenue == 3){ // scroll backward (mute, pause, cancel, prev, next)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_PL_mute.showAlternativePic(); btn_PL_pause.show();}
                            if(btnNr == 1){btn_PL_pause.showAlternativePic(); btn_PL_cancel.show();}
                            if(btnNr == 2){btn_PL_cancel.showAlternativePic(); btn_PL_playPrev.show();}
                            if(btnNr == 3){btn_PL_playPrev.showAlternativePic(); btn_PL_playNext.show();}
                            setTimeCounter(2);
                            return;
                        }
                        if(_cur_AudioFileNr > 0) {
                            _cur_AudioFileNr--;
                            showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr));
                            showAudioFileNumber();
                            return;
                        }
                    }
                    if(_state == AUDIOFILESLIST) {
                            lst_PLAYER.prevPage(); setTimeCounter(LIST_TIMER); break; // prev page
                    }
                    if(_state == DLNA){
                        if(_dlnaSubMenue == 1){ // scroll backward (mute, pause, cancel, prev, next)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_DL_mute.showAlternativePic(); btn_DL_pause.show();}
                            if(btnNr == 1){btn_DL_pause.showAlternativePic(); btn_DL_cancel.show();}
                            if(btnNr == 2){btn_DL_cancel.showAlternativePic(); btn_DL_fileList.show();}
                            if(btnNr == 3){btn_DL_fileList.showAlternativePic(); btn_DL_radio.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == DLNAITEMSLIST){
                        lst_DLNA.prevPage(); setTimeCounter(LIST_TIMER); return; // prev page
                    }

                    if(_state == CLOCK){
                        if(_clockSubMenue == 2){ // scroll backward (alarm, radio, mute, off)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_CL_alarm.showAlternativePic(); btn_CL_radio.show();}
                            if(btnNr == 1){btn_CL_radio.showAlternativePic(); btn_CL_mute.show();}
                            if(btnNr == 2){btn_CL_mute.showAlternativePic(); btn_CL_off.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == ALARMCLOCK){
                        if(_alarmSubMenue == 1){ // scroll backward (left, right, up, down, ready)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_AC_left.showAlternativePic(); btn_AC_right.show();}
                            if(btnNr == 1){btn_AC_right.showAlternativePic(); btn_AC_up.show();}
                            if(btnNr == 2){btn_AC_up.showAlternativePic(); btn_AC_down.show();}
                            if(btnNr == 3){btn_AC_down.showAlternativePic(); btn_AC_ready.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == SLEEPTIMER){
                        if(_sleepTimerSubMenue == 1){ // scroll backward (up, down, ready, cancel)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_SL_up.showAlternativePic(); btn_SL_down.show();}
                            if(btnNr == 1){btn_SL_down.showAlternativePic(); btn_SL_ready.show();}
                            if(btnNr == 2){btn_SL_ready.showAlternativePic(); btn_SL_cancel.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == SETTINGS){
                        if(_settingsSubMenue == 1){ // scroll forward (bright, equal, radio)
                            if(btnNr > 0) btnNr--;
                            if(btnNr == 0){btn_SE_bright.showAlternativePic(); btn_SE_equal.show();}
                            if(btnNr == 1){btn_SE_equal.showAlternativePic(); btn_SE_wifi.show();}
                            if(btnNr == 2){btn_SE_wifi.showAlternativePic(); btn_SE_radio.show();}
                            setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == BRIGHTNESS){
                        if(_brightnessSubMenue == 1){
                            if(_brightness > 5) _brightness -= 5;
                            if(_brightness < 5) _brightness = 5;
                            sdr_BR_value.setValue(_brightness); setTimeCounter(2);
                            return;
                        }
                    }
                    if(_state == EQUALIZER){
                        if(_equalizerSubMenue == 1){
                            char c[10];
                            if(btnNr == 10){if(_toneBAL > -16){_toneBAL--; itoa(_toneBAL, c, 10); txt_EQ_balance.writeText(c); sdr_EQ_balance.setValue(_toneBAL);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // balance
                            if(btnNr == 20){if(_toneLP > -40){_toneLP--; itoa(_toneLP, c, 10); txt_EQ_lowPass.writeText(c); sdr_EQ_lowPass.setValue(_toneLP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // LP
                            if(btnNr == 30){if(_toneBP > -40){_toneBP--; itoa(_toneBP, c, 10); txt_EQ_bandPass.writeText(c); sdr_EQ_bandPass.setValue(_toneBP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // BP
                            if(btnNr == 40){if(_toneHP > -40){_toneHP--; itoa(_toneHP, c, 10); txt_EQ_highPass.writeText(c); sdr_EQ_highPass.setValue(_toneHP);} webSrv.send("settone=", setI2STone()); setTimeCounter(2); return;} // HP
                            if(btnNr == 1) {btnNr = 0; btn_EQ_Player.show(); btn_EQ_Radio.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 2) {btnNr = 1; btn_EQ_mute.show(); btn_EQ_Player.showAlternativePic(); setTimeCounter(2); return;}
                        }
                    }
                    break;
        case 13:    // ARROW DOWN
                    if(_state == RADIO)  {txt_RA_staName.hide(); volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == STATIONSLIST) {lst_RADIO.nextStation(); setTimeCounter(LIST_TIMER); break;} // station++
                    if(_state == PLAYER) {txt_PL_fName.hide();   volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == AUDIOFILESLIST){lst_PLAYER.nextFile(); setTimeCounter(LIST_TIMER); break;} // file++
                    if(_state == DLNA)  {txt_DL_fName.hide(); volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == DLNAITEMSLIST){lst_DLNA.nextItem(); setTimeCounter(LIST_TIMER); return;} // item++
                    if(_state == CLOCK) {downvolume(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == SLEEPTIMER) {downvolume(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == EQUALIZER && _equalizerSubMenue == 1){
                        if     (btnNr == 10){btnNr = 0;  btn_EQ_Radio.showAlternativePic();    btn_EQ_balance.show();}
                        else if(btnNr == 20){btnNr = 10; btn_EQ_balance.showAlternativePic();  btn_EQ_lowPass.show();}
                        else if(btnNr == 30){btnNr = 20; btn_EQ_lowPass.showAlternativePic();  btn_EQ_bandPass.show();}
                        else if(btnNr == 40){btnNr = 30; btn_EQ_bandPass.showAlternativePic(); btn_EQ_highPass.show();}
                        setTimeCounter(2); return;
                    }
                    break;
        case 14:    // ARROW UP
                    if(_state == RADIO)  {txt_RA_staName.hide(); volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == STATIONSLIST) {lst_RADIO.prevStation(); setTimeCounter(LIST_TIMER); break;} // station--
                    if(_state == PLAYER) {txt_PL_fName.hide();   volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == AUDIOFILESLIST){lst_PLAYER.prevFile(); setTimeCounter(LIST_TIMER); break;} // file-
                    if(_state == DLNA)  {txt_DL_fName.hide(); volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == DLNAITEMSLIST){lst_DLNA.prevItem(); setTimeCounter(LIST_TIMER); return;} // item++
                    if(_state == CLOCK) {upvolume(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == SLEEPTIMER) {upvolume(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == EQUALIZER && _equalizerSubMenue == 1){
                        if     (btnNr <  10){btnNr = 10; btn_EQ_balance.showAlternativePic();  btn_EQ_Radio.show(); btn_EQ_Player.show(); btn_EQ_mute.show();}
                        else if(btnNr == 10){btnNr = 20; btn_EQ_lowPass.showAlternativePic();  btn_EQ_balance.show();}
                        else if(btnNr == 20){btnNr = 30; btn_EQ_bandPass.showAlternativePic(); btn_EQ_lowPass.show();}
                        else if(btnNr == 30){btnNr = 40; btn_EQ_highPass.showAlternativePic(); btn_EQ_bandPass.show();}
                        setTimeCounter(2); return;
                    }
                    break;
        case 15:    // MODE
                    if(_state == SLEEPTIMER) {setStation(_cur_station); _radioSubMenue = 0; changeState(RADIO); break;} //  RADIO -> STATIONSLIST -> PLAYER -> DLNA -> CLOCK -> SLEEPTIMER
                    if(_state == RADIO) {changeState(STATIONSLIST); setTimeCounter(40); break;}
                    if(_state == STATIONSLIST) { _playerSubMenue = 0; changeState(PLAYER); break;}
                    if(_state == PLAYER) {changeState(AUDIOFILESLIST); break;}
                    if(_state == AUDIOFILESLIST) {_dlnaSubMenue = 0; changeState(DLNA); break;}
                    if(_state == DLNA) {_clockSubMenue = 0; changeState(CLOCK); break;}
                    if(_state == CLOCK) {_sleepTimerSubMenue =0; changeState(SLEEPTIMER); break;}
                    break;
        case 16:    // OK
                    if(_state == RADIO) {
                        if(_radioSubMenue == 4){
                            if(btnNr == 0){btn_RA_staList.showClickedPic(); vTaskDelay(100); btnNr = 0;                            changeState(STATIONSLIST); setTimeCounter(LIST_TIMER); break;}
                            if(btnNr == 1){btn_RA_player.showClickedPic(); vTaskDelay(100); btnNr = 0;    _playerSubMenue = 2;     changeState(PLAYER);       setTimeCounter(2); break;}
                            if(btnNr == 2){btn_RA_dlna.showClickedPic();   vTaskDelay(100); btnNr = 0;    _dlnaSubMenue = 1;       changeState(DLNA);         setTimeCounter(2); break;}
                            if(btnNr == 3){btn_RA_clock.showClickedPic();  vTaskDelay(100); btnNr = 0;    _clockSubMenue = 2;      changeState(CLOCK);        setTimeCounter(2); break;}
                            if(btnNr == 4){btn_RA_sleep.showClickedPic();  vTaskDelay(100); btnNr = 0;    _sleepTimerSubMenue = 1; changeState(SLEEPTIMER);   setTimeCounter(2); break;}
                            if(btnNr == 5){btn_RA_settings.showClickedPic(); vTaskDelay(100); btnNr = 0;  _settingsSubMenue = 1;   changeState(SETTINGS);     setTimeCounter(2); break;}
                            if(btnNr == 6){btn_RA_bt.showClickedPic();     vTaskDelay(100); btnNr = 0; changeState(BLUETOOTH); break;}
                            if(btnNr == 7){btn_RA_off.showClickedPic();    vTaskDelay(100); fall_asleep(); break;}
                        }
                        else{
                            _radioSubMenue = 4; btnNr = 0; changeState(RADIO);
                        }
                        break;
                    }
                    if(_state == STATIONSLIST)   {setStationByNumber(lst_RADIO.getSelectedStation()); _radioSubMenue = 0; changeState(RADIO); break;}
                    if(_state == PLAYER){
                        if(_playerSubMenue == 0){_playerSubMenue = 2; btnNr = 0; changeState(PLAYER); setTimeCounter(2); break;}
                        if(_playerSubMenue == 1){_playerSubMenue = 3; btnNr = 0; changeState(PLAYER); setTimeCounter(2); break;}
                        if(_playerSubMenue == 2){
                            if(btnNr == 0){ // prev AudioFile
                                            btn_PL_prevFile.showClickedPic(); vTaskDelay(50); btn_PL_prevFile.showAlternativePic(); if(_cur_AudioFileNr > 0) {_cur_AudioFileNr--;
                                            showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); setTimeCounter(2);} return;}
                            if(btnNr == 1){ // next AudioFile
                                            btn_PL_nextFile.showClickedPic(); vTaskDelay(50); btn_PL_nextFile.showAlternativePic(); if(_cur_AudioFileNr + 1 < _SD_content.getSize()) {_cur_AudioFileNr++;
                                            showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); setTimeCounter(2);} return;}
                            if(btnNr == 2){ // play file
                                            btn_PL_ready.showClickedPic(); vTaskDelay(100); stopSong(); SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr));
                                            btnNr = 0; _playerSubMenue = 3; changeState(PLAYER); showAudioFileNumber(); setTimeCounter(2); return;}
                            if(btnNr == 3){ // play all files
                                            btn_PL_playAll.showClickedPic(); _f_shuffle = false; preparePlaylistFromSDFolder(_cur_AudioFolder); processPlaylist(true);
                                            _playerSubMenue = 1; changeState(PLAYER); return;}
                            if(btnNr == 4){ // shuffle and play all files
                                            btn_PL_shuffle.showClickedPic(); vTaskDelay(100);  _f_shuffle = true; preparePlaylistFromSDFolder(_cur_AudioFolder); processPlaylist(true);
                                            _playerSubMenue = 1; changeState(PLAYER); return;}
                            if(btnNr == 5){ // show file list
                                            btn_PL_fileList.showClickedPic(); vTaskDelay(100); _SD_content.listFilesInDir(_cur_AudioFolder, true, false);
                                            changeState(AUDIOFILESLIST); return;}
                            if(btnNr == 6){ // back to radio
                                            btn_PL_radio.showClickedPic(); vTaskDelay(100); setStation(_cur_station);_playerSubMenue = 0; _radioSubMenue = 0; changeState(RADIO); return;}
                            if(btnNr == 7){ // off
                                            btn_PL_off.showClickedPic(); vTaskDelay(100); fall_asleep(); return;}
                        }
                        if(_playerSubMenue == 3){
                            if(btnNr == 0){ // mute
                                            btn_PL_mute.showClickedPic(); muteChanged(!_f_mute); vTaskDelay(100); btn_PL_mute.showAlternativePic(); setTimeCounter(2); return;
                            }
                            if(btnNr == 1){ // pause
                                            btn_PL_pause.showClickedPic(); if(_f_isFSConnected) {_f_pauseResume = audio.pauseResume(); if(!audio.isRunning()) btn_PL_pause.setOn(); else btn_PL_pause.setOff();}
                                            vTaskDelay(100); btn_PL_pause.showAlternativePic(); setTimeCounter(2); return;
                            }
                            if(btnNr == 2){ // cancel
                                            btn_PL_cancel.showClickedPic(); vTaskDelay(100); btnNr = 0; _playerSubMenue = 2; stopSong(); changeState(PLAYER); return;
                            }
                            if(btnNr == 3){ // prev
                                            btn_PL_playPrev.showClickedPic(); vTaskDelay(100); btn_PL_playPrev.showAlternativePic(); _cur_AudioFileNr = _SD_content.getPrevAudioFile(_cur_AudioFileNr);
                                            SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); setTimeCounter(2); return;
                            }
                            if(btnNr == 4){ // next
                                            btn_PL_playNext.showClickedPic(); vTaskDelay(100); btn_PL_playNext.showAlternativePic(); _cur_AudioFileNr = _SD_content.getNextAudioFile(_cur_AudioFileNr);
                                            SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); setTimeCounter(2); return;
                            }
                        }

                    }
                    if(_state == AUDIOFILESLIST){   const char* r = lst_PLAYER.getSelectedFile();
                                            if(r){stopSong(); SD_playFile(lst_PLAYER.getSelectedFilePath(), 0, true); _cur_AudioFileNr = lst_PLAYER.getSelectedFileNr();}
                    }
                    if(_state == DLNA) {
                        if(_dlnaSubMenue == 0) { _dlnaSubMenue = 1; btnNr = 0; changeState(DLNA); setTimeCounter(2); break;}
                        if(_dlnaSubMenue == 1) {
                            if(btnNr == 0){ // mute
                                            btn_DL_mute.showClickedPic(); muteChanged(!_f_mute); vTaskDelay(100); btn_DL_mute.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 1){ // pause
                                            if(!btn_DL_pause.getActive()) {setTimeCounter(2); return;}// is inactive
                                            btn_DL_pause.showClickedPic(); if(_f_isWebConnected) {_f_pauseResume = audio.pauseResume(); if(!audio.isRunning()) btn_DL_pause.setOn(); else btn_DL_pause.setOff();}
                                            vTaskDelay(100); btn_DL_pause.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 2){ // cancel
                                            btn_DL_cancel.showClickedPic(); vTaskDelay(100); btn_DL_cancel.show(); stopSong(); txt_DL_fName.setText(""); txt_DL_fName.hide(); pgb_DL_progress.reset();
                                            btn_DL_pause.setActive(false); btn_DL_pause.show(); setTimeCounter(2); return;}
                            if(btnNr == 3){ // dlna list
                                            changeState(DLNAITEMSLIST); txt_DL_fName.setText(""); return;}
                            if(btnNr == 4){ // back to radio
                                            btn_DL_radio.showClickedPic(); vTaskDelay(100); setStation(_cur_station); _playerSubMenue = 0; _radioSubMenue = 0; changeState(RADIO); return;}
                        }
                        break;
                    }
                    if(_state == DLNAITEMSLIST) {setTimeCounter(2); const char* r = lst_DLNA.getSelectedURL();
                                                 if(r){txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(lst_DLNA.getSelectedTitle());
                                                       _dlnaSubMenue = 0; changeState(DLNA); connecttohost(r);}
                                                 break;
                    }
                    if(_state == CLOCK){
                        if(_clockSubMenue == 0) { _clockSubMenue = 2; btnNr = 0; changeState(CLOCK); setTimeCounter(2); break;}
                        if(_clockSubMenue == 2){
                            if(btnNr == 0){ // Alarm
                                            _alarmSubMenue = 1; btnNr = 0; btn_CL_alarm.showClickedPic(); vTaskDelay(100); changeState(ALARMCLOCK); return;}
                            if(btnNr == 1){ // Radio
                                            btn_CL_radio.showClickedPic(); vTaskDelay(100); setStation(_cur_station);_playerSubMenue = 0; _radioSubMenue = 0; changeState(RADIO); return;}
                            if(btnNr == 2){ // Mute
                                            btn_CL_mute.showClickedPic(); muteChanged(!_f_mute); vTaskDelay(100); btn_CL_mute.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 3){ // Off
                                            btn_CL_off.showClickedPic(); vTaskDelay(100); fall_asleep(); return;}
                        }
                    }
                    if(_state == ALARMCLOCK){
                        if(_alarmSubMenue == 0) { _alarmSubMenue = 1; btnNr = 0; changeState(ALARMCLOCK); setTimeCounter(2); return;}
                        if(_alarmSubMenue == 1){
                            if(btnNr == 0){ // pos left
                                btn_AC_left.showClickedPic(); clk_AC_red.shiftLeft(); btn_AC_left.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 1){ // pos right
                                btn_AC_right.showClickedPic(); clk_AC_red.shiftRight(); btn_AC_right.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 2){ // pos +1
                                btn_AC_up.showClickedPic(); clk_AC_red.digitUp(); btn_AC_up.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 3){ // pos -1
                                btn_AC_down.showClickedPic(); clk_AC_red.digitDown(); btn_AC_down.showAlternativePic(); setTimeCounter(2); return;}
                            if(btnNr == 4){ // ready
                                btn_AC_ready.showClickedPic(); vTaskDelay(100); updateSettings(); _clockSubMenue = 0; changeState(CLOCK); logAlarmItems(); return;}
                        }
                    }
                    if(_state == SLEEPTIMER) {
                        if(_sleepTimerSubMenue == 0){
                            _sleepTimerSubMenue = 1; btnNr = 0; changeState(SLEEPTIMER); setTimeCounter(2); return;}
                        if(_sleepTimerSubMenue == 1){
                            if(btnNr == 0){btn_SL_up.showClickedPic(); display_sleeptime(1); btn_SL_up.showAlternativePic(); return;}
                            if(btnNr == 1){btn_SL_down.showClickedPic(); display_sleeptime(-1); btn_SL_down.showAlternativePic(); return;}
                            if(btnNr == 2){btn_SL_ready.showClickedPic(); vTaskDelay(100); dispFooter.updateOffTime(_sleeptime); _radioSubMenue = 0; changeState(RADIO); return;}
                            if(btnNr == 3){ btn_SL_cancel.showClickedPic(); vTaskDelay(100); _radioSubMenue = 0; changeState(RADIO); return;}
                        }
                    }
                    if(_state == SETTINGS){
                        if(_settingsSubMenue == 0){_settingsSubMenue = 1; btnNr = 0; changeState(SETTINGS); setTimeCounter(2); return;}
                        if(_settingsSubMenue == 1){
                            if(btnNr == 0){btn_SE_bright.showClickedPic(); vTaskDelay(100); btnNr = 0; _brightnessSubMenue = 1;  changeState(BRIGHTNESS); setTimeCounter(2); return;}
                            if(btnNr == 1){btn_SE_equal.showClickedPic(); vTaskDelay(100); btnNr = 0; _equalizerSubMenue = 1; changeState(EQUALIZER); setTimeCounter(2); return;}
                            if(btnNr == 2){btn_SE_wifi.showClickedPic(); vTaskDelay(100); btnNr = 0; changeState(WIFI_SETTINGS); return;}
                            if(btnNr == 3){btn_SE_radio.showClickedPic(); vTaskDelay(100); _radioSubMenue = 0; changeState(RADIO); return;}
                        }
                    }
                    if(_state == BRIGHTNESS){
                        if(_brightnessSubMenue == 0){_brightnessSubMenue = 1; btnNr = 0; changeState(BRIGHTNESS); setTimeCounter(2); return;}
                        if(_brightnessSubMenue == 1){_radioSubMenue = 0; changeState(RADIO); return;}
                    }
                    if(_state == EQUALIZER){
                        if(_equalizerSubMenue == 0){_equalizerSubMenue = 1; btnNr = 0; changeState(EQUALIZER); setTimeCounter(2); return;}
                        if(_equalizerSubMenue == 1){
                            if(btnNr == 0){btn_EQ_Radio.showClickedPic(); vTaskDelay(100);  setStation(_cur_station); _radioSubMenue = 0; changeState(RADIO); return;} // Radio
                            if(btnNr == 1){btn_EQ_Player.showClickedPic(); vTaskDelay(100); _playerSubMenue = 0; changeState(PLAYER); return;} // Player
                            if(btnNr == 2){btn_EQ_mute.showClickedPic(); vTaskDelay(100);  muteChanged(!_f_mute); btn_EQ_mute.showAlternativePic(); setTimeCounter(2); return;} // Mute
                            if(btnNr == 10){btn_EQ_balance.showClickedPic(); _toneBAL = 0; txt_EQ_balance.writeText( "0");  sdr_EQ_balance.setValue(_toneLP);  webSrv.send("settone=", setI2STone()); btn_EQ_balance.showAlternativePic();}
                            if(btnNr == 20){btn_EQ_lowPass.showClickedPic();  _toneLP = 0; txt_EQ_lowPass.writeText( "0");  sdr_EQ_lowPass.setValue(_toneLP);  webSrv.send("settone=", setI2STone()); btn_EQ_lowPass.showAlternativePic();}
                            if(btnNr == 30){btn_EQ_bandPass.showClickedPic(); _toneBP = 0; txt_EQ_bandPass.writeText("0"); sdr_EQ_bandPass.setValue(_toneLP); webSrv.send("settone=", setI2STone()); btn_EQ_bandPass.showAlternativePic();}
                            if(btnNr == 40){btn_EQ_highPass.showClickedPic(); _toneHP = 0; txt_EQ_highPass.writeText("0"); sdr_EQ_highPass.setValue(_toneLP); webSrv.send("settone=", setI2STone()); btn_EQ_highPass.showAlternativePic();}
                        }
                        setTimeCounter(2); return;
                    }
                    break;
        case 18:    if(_state == PLAYER){if(_f_isFSConnected) _f_pauseResume = audio.pauseResume();} break;
        case 19:    if(_state == PLAYER){if(_f_isFSConnected) audio.stopSong(); _playerSubMenue = 0; changeState(PLAYER);} break;
        case 20:    _f_irOnOff = ! _f_irOnOff;
                    if(_f_irOnOff) fall_asleep();
                    else           wake_up();
                    break;
        case 21:    if(_state != RADIO)      {_radioSubMenue      = 0; setStation(_cur_station); changeState(RADIO);}      break;
        case 22:    if(_state != PLAYER)     {_playerSubMenue     = 0; changeState(PLAYER);}     break;
        case 23:    if(_state != DLNA)       {_dlnaSubMenue       = 0; changeState(DLNA);}       break;
        case 24:    if(_state != CLOCK)      {_clockSubMenue      = 0; changeState(CLOCK);}      break;
        case 25:    if(_state != SLEEPTIMER) {_sleepTimerSubMenue = 0; changeState(SLEEPTIMER);} break;
        case 26:    // VOLUME+
                    if(_state == RADIO)  {txt_RA_staName.hide(); volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == PLAYER) {txt_PL_fName.hide();   volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == DLNA)  {txt_DL_fName.hide(); volBox.enable(); upvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == CLOCK) {upvolume(); setTimeCounter(2); break;} // VOLUME++
                    if(_state == SLEEPTIMER) {upvolume(); setTimeCounter(2); break;} // VOLUME++
                    upvolume(); break;
        case 27:    // VOLUME-
                    if(_state == RADIO)  {txt_RA_staName.hide(); volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == PLAYER) {txt_PL_fName.hide();   volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == DLNA)  {txt_DL_fName.hide(); volBox.enable(); downvolume(); volBox.setNumbers(_cur_volume); volBox.show(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == CLOCK) {downvolume(); setTimeCounter(2); break;} // VOLUME--
                    if(_state == SLEEPTIMER) {downvolume(); setTimeCounter(2); break;} // VOLUME--
                    downvolume(); break;
        case 28:    if(_state == PLAYER) {if(audio.isRunning()) audio.setTimeOffset(-30);}       break;
        case 29:    if(_state == PLAYER) {if(audio.isRunning()) audio.setTimeOffset(+30);}       break;
        case 30:    nextStation(); break;
        case 31:    prevStation(); break;
        default:    break;
    }
}
void ir_long_key(int8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "long pressed key nr: " ANSI_ESC_BLUE "%02i", key);
    if(key == 16) fall_asleep(); // long OK
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Event from TouchPad
// clang-format off
void tp_pressed(uint16_t x, uint16_t y) {
    //  SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint  x=%d, y=%d", x, y);
    if(_f_sleeping) return;  // awake in tp_released()
    const char* objName = NULL;
    if(_state == RADIO && y > _winHeader.y + _winHeader.h && y < _sdrOvBtns.y){
        objName = "backpane";
        _radioSubMenue++;
        if(_radioSubMenue == 3) _radioSubMenue = 0;
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

    // if(_f_muteIsPressed) {
    //     if(!_f_mute){
    //         fall_asleep();
    //     }
    //     else{
    //         muteChanged(false);
    //     }
    //     return;
    // }

    if(_state == DLNAITEMSLIST){
    //    lst_DLNA.longPressed(x, y);
    }
}
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_released(uint16_t x, uint16_t y){

    if(_f_sleeping){ wake_up(); return;}   // if sleeping

    // all state
    dispHeader.released();
    dispFooter.released();

    switch(_state){
        case RADIO:
            if(_radioSubMenue == 0){ VUmeter_RA.released();}
            if(_radioSubMenue == 1){ sdr_RA_volume.released(); btn_RA_mute.released(); btn_RA_prevSta.released(); btn_RA_nextSta.released();}
            if(_radioSubMenue == 2){ btn_RA_player.released(); btn_RA_dlna.released(); btn_RA_clock.released(); btn_RA_sleep.released(); btn_RA_settings.released();
                                     btn_RA_bt.released(); btn_RA_off.released(); btn_RA_staList.released();}
            if(_radioSubMenue == 4){ btn_RA_player.released(); btn_RA_dlna.released(); btn_RA_clock.released(); btn_RA_sleep.released(); btn_RA_settings.released();
                                     btn_RA_bt.released(); btn_RA_off.released(); btn_RA_staList.released();}
            break;
        case STATIONSLIST:
            lst_RADIO.released();
            break;
        case PLAYER:
            if(_playerSubMenue == 0){btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released(); btn_PL_fileList.released();
                                     btn_PL_radio.released(); btn_PL_off.released();}
            if(_playerSubMenue == 1){btn_PL_mute.released(); btn_PL_pause.released(); btn_PL_cancel.released(); sdr_PL_volume.released(); btn_PL_playNext.released(); btn_PL_playPrev.released();
                                     pgb_PL_progress.released();}
            if(_playerSubMenue == 2){btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released(); btn_PL_fileList.released();
                                     btn_PL_radio.released(); btn_PL_off.released();}
            if(_playerSubMenue == 3){btn_PL_mute.released(); btn_PL_pause.released(); btn_PL_cancel.released(); sdr_PL_volume.released(); btn_PL_playNext.released(); btn_PL_playPrev.released();
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
    // SerialPrintfln("tp_released, state is: %i", _state);
}

void tp_long_released(uint16_t x, uint16_t y){
//    log_w("long released)");
//    if(_state == DLNAITEMSLIST) {lst_DLNA.longReleased();}
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_moved(uint16_t x, uint16_t y){
    if(_state == RADIO){
        if(sdr_RA_volume.positionXY(x, y)) return;
    }
    if(_state == STATIONSLIST){
        if(lst_RADIO.positionXY(x, y)) return;
    }
    if(_state == PLAYER){
        if(sdr_PL_volume.positionXY(x, y)) return;
    }
    if(_state == AUDIOFILESLIST){
        if(lst_PLAYER.positionXY(x, y)) return;
    }
    if(_state == DLNA){
        if(sdr_DL_volume.positionXY(x, y)) return;
    }
    if(_state == CLOCK){
        if(sdr_CL_volume.positionXY(x, y)) return;
    }
    if(_state == DLNAITEMSLIST){
        if(lst_DLNA.positionXY(x, y)) return;
    }
    if(_state == BRIGHTNESS){
        if(sdr_BR_value.positionXY(x, y)) return;
    }
    if(_state == EQUALIZER){
        if(sdr_EQ_lowPass.positionXY(x, y)) return;
        if(sdr_EQ_bandPass.positionXY(x, y)) return;
        if(sdr_EQ_highPass.positionXY(x, y)) return;
        if(sdr_EQ_balance.positionXY(x, y)) return;
    }
    if(_state == IR_SETTINGS){
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

    CMD_EQUALS("test"){                 sprintf(_chbuf, "free heap: %lu, Inbuff filled: %lu, Inbuff free: %lu, PSRAM filled %lu, PSRAM free %lu",
                                            (long unsigned)ESP.getFreeHeap(), (long unsigned)audio.inBufferFilled(), (long unsigned)audio.inBufferFree(),
                                            (long unsigned) (ESP.getPsramSize() - ESP.getFreePsram()), (long unsigned)ESP.getFreePsram());
                                        webSrv.send("test=", _chbuf);
                                        SerialPrintfln("audiotask .. stackHighWaterMark: %lu bytes", (long unsigned)audio.getHighWatermark() * 4);
                                        SerialPrintfln("looptask ... stackHighWaterMark: %lu bytes", (long unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
                                        return;}

    CMD_EQUALS("getmute"){              if(_f_mute) webSrv.send("mute=", "1");
                                        else        webSrv.send("mute=", "0");
                                        return;}

    CMD_EQUALS("setmute"){              muteChanged(!_f_mute); return;}

    CMD_EQUALS("upvolume"){             webSrv.send("volume=", int2str(upvolume()));  return;}                                                            // via websocket
    CMD_EQUALS("downvolume"){           webSrv.send("volume=", int2str(downvolume())); return;}                                                           // via websocket

    CMD_EQUALS("getVolumeSteps"){       webSrv.send("volumeSteps=", int2str(_volumeSteps)); return;}

    CMD_EQUALS("setVolumeSteps"){       _cur_volume = map_l(_cur_volume, 0, _volumeSteps, 0, param.toInt());
                                        _ringVolume = map_l(_ringVolume, 0, _volumeSteps, 0, param.toInt()); webSrv.send("ringVolume=", int2str(_ringVolume));
                                        _volumeAfterAlarm = map_l(_volumeAfterAlarm, 0, _volumeSteps, 0, param.toInt()); webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm));
                                        _volumeSteps = param.toInt(); webSrv.send("volumeSteps=", param); audio.setVolumeSteps(_volumeSteps);
                                        // log_w("_volumeSteps  %i", _volumeSteps);
                                        sdr_CL_volume.setNewMinMaxVal(0, _volumeSteps);
                                        sdr_DL_volume.setNewMinMaxVal(0, _volumeSteps);
                                        sdr_PL_volume.setNewMinMaxVal(0, _volumeSteps);
                                        sdr_RA_volume.setNewMinMaxVal(0, _volumeSteps);
                                        setVolume(_cur_volume);
                                        SerialPrintfln("action: ...  new volume steps: " ANSI_ESC_CYAN "%d", _volumeSteps);
                                        return;}

    CMD_EQUALS("getRingVolume"){        webSrv.send("ringVolume=", int2str(_ringVolume)); return;}
    CMD_EQUALS("setRingVolume"){        _ringVolume = param.toInt(); webSrv.send("ringVolume=", int2str(_ringVolume));
                                        SerialPrintfln("action: ...  new ring volume: " ANSI_ESC_CYAN "%d", _ringVolume); return;}

    CMD_EQUALS("getVolAfterAlarm"){     webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm)); return;}
    CMD_EQUALS("setVolAfterAlarm"){     _volumeAfterAlarm = param.toInt(); webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm));
                                        SerialPrintfln("action: ...  new volume after alarm: " ANSI_ESC_CYAN "%d", _volumeAfterAlarm); return;}

    CMD_EQUALS("homepage"){             webSrv.send("homepage=", _homepage);
                                        return;}

    CMD_EQUALS("to_listen"){            StationsItems(); // via websocket, return the name and number of the current station
                                        return;}

    CMD_EQUALS("gettone"){              webSrv.send("settone=", setI2STone());
                                        return;}

    CMD_EQUALS("getstreamtitle"){       webSrv.reply(_streamTitle, webSrv.TEXT);
                                        return;}

    CMD_EQUALS("LowPass"){              _toneLP = param.toInt();                           // audioI2S tone
                                        char lp[30] = "Lowpass set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                        webSrv.send("tone=", lp); setI2STone(); return;}

    CMD_EQUALS("BandPass"){             _toneBP = param.toInt();                           // audioI2S tone
                                        char bp[30] = "Bandpass set to "; strcat(bp, param.c_str()); strcat(bp, "dB");
                                        webSrv.send("tone=", bp); setI2STone(); return;}

    CMD_EQUALS("HighPass"){             _toneHP = param.toInt();                           // audioI2S tone
                                        char hp[30] = "Highpass set to "; strcat(hp, param.c_str()); strcat(hp, "dB");
                                        webSrv.send("tone=", hp); setI2STone(); return;}

    CMD_EQUALS("Balance"){              _toneBAL = param.toInt();
                                        char bal[30] = "Balance set to "; strcat(bal, param.c_str());
                                        webSrv.send("tone=", bal); setI2STone(); return;}

    CMD_EQUALS("uploadfile"){           _filename = param; return;}

    CMD_EQUALS("prev_station"){         prevFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("next_station"){         nextFavStation(); return;}                                                                                           // via websocket

    CMD_EQUALS("set_station"){          setStationByNumber(param.toInt()); return;}                                                                          // via websocket

    CMD_EQUALS("stationURL"){           setStationViaURL(param.c_str(), arg.c_str()); x_ps_free(&_stationName_air);                                          // via websocket
                                        _stationName_air = x_ps_strndup(param.c_str(), 200); // set max length
                                        SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", param.c_str());
                                        _f_newStationName = true; return;}

    CMD_EQUALS("webFileURL"){           audio.connecttohost(param.c_str())? _playerSubMenue = 1 : _playerSubMenue = 0; changeState(PLAYER); return;}          // via websocket

    CMD_EQUALS("getnetworks"){          webSrv.send("networks=", WiFi.SSID()); return;}                                                  // via websocket

    CMD_EQUALS("get_tftSize"){          webSrv.send("tftSize=", _tftSize); return;};

    CMD_EQUALS("getTimeZones"){         webSrv.send("timezones=", timezones_json); return;}

    CMD_EQUALS("setTimeZone"){          _TZName = param;  _TZString = arg;
                                        SerialPrintfln("Timezone: .. " ANSI_ESC_BLUE "%s, %s", param.c_str(), arg.c_str());
                                        setRTC(_TZString.c_str());
                                        updateSettings(); // write new TZ items to settings.json
                                        return;}

    CMD_EQUALS("getTimeZoneName"){      webSrv.reply(_TZName, webSrv.TEXT); return;}

    CMD_EQUALS("change_state"){         if     (!strcmp(param.c_str(), "RADIO")       && _state != RADIO)       {setStation(_cur_station); _radioSubMenue = 0; changeState(RADIO); return;}
                                        else if(!strcmp(param.c_str(), "PLAYER")      && _state != PLAYER)      {stopSong(); _playerSubMenue = 0; changeState(PLAYER); return;}
                                        else if(!strcmp(param.c_str(), "DLNA")        && _state != DLNA)        {stopSong(); _dlnaSubMenue = 0; changeState(DLNA);   return;}
                                        else if(!strcmp(param.c_str(), "BLUETOOTH")   && _state != BLUETOOTH)   {changeState(BLUETOOTH); return;}
                                        else if(!strcmp(param.c_str(), "IR_SETTINGS") && _state != IR_SETTINGS) {changeState(IR_SETTINGS); return;}
                                        else return;}

    CMD_EQUALS("stopfile"){             if(!_f_isFSConnected && !_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        _playerSubMenue = 0; stopSong(); changeState(PLAYER); webSrv.send("stopfile=", "audiofile stopped");
                                        return;}

    CMD_EQUALS("pause_resume"){         if(!_f_isFSConnected && !_f_isWebConnected) {webSrv.send("resumefile=", "There is no audio file active"); return;}
                                        _f_pauseResume = audio.pauseResume();
                                        if(audio.isRunning()){webSrv.send("resumefile=", "audiofile resumed"); btn_PL_pause.setOff(); btn_PL_pause.show();}
                                        else {                webSrv.send("resumefile=", "audiofile paused");  btn_PL_pause.setOn(); btn_PL_pause.show();}
                                        return;}

    CMD_EQUALS("get_alarmdays"){        webSrv.send("alarmdays=", String(_alarmdays, 10)); return;}

    CMD_EQUALS("set_alarmdays"){        _alarmdays = param.toInt(); updateSettings(); return;}

    CMD_EQUALS("get_alarmtime"){        return;} // not used yet

    CMD_EQUALS("set_alarmtime"){        return;}

    CMD_EQUALS("get_timeAnnouncement"){ if(_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "1");
                                        if(  !_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "0");
                                        return;}

    CMD_EQUALS("set_timeAnnouncement"){ if(param == "true" ) {_f_timeAnnouncement = true;}
                                        if(   param == "false") {_f_timeAnnouncement = false;}
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "hourly time announcement " ANSI_ESC_BLUE "%s", (_f_timeAnnouncement == 1) ? "on" : "off");
                                        return;}

    CMD_EQUALS("getTimeSpeechLang"){    webSrv.send("getTimeSpeechLang=", String(_timeSpeechLang, 10)); return;}

    CMD_EQUALS("setTimeSpeechLang"){    if(param.length() > 2){log_e("set_timeSpeechLang too long %s", param.c_str()); return;}
                                        strcpy(_timeSpeechLang, param.c_str());
                                        SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "language is " ANSI_ESC_BLUE "%s", param.c_str());
                                        return;}

    CMD_EQUALS("DLNA_getServer")  {     webSrv.send("DLNA_Names=", dlna.stringifyServer()); _currDLNAsrvNr = -1; return;}

    CMD_EQUALS("DLNA_getRoot")    {     _currDLNAsrvNr = param.toInt(); dlna.browseServer(_currDLNAsrvNr, "0"); return;}

    CMD_EQUALS("DLNA_getContent") {     if(param.startsWith("http")) {connecttohost(param.c_str()); showFileName(arg.c_str()); return;}
                                        x_ps_free(&_dlnaHistory[_dlnaLevel].objId); _dlnaHistory[_dlnaLevel].objId = x_ps_strdup(param.c_str());
                                        _totalNumberReturned = 0;
                                        dlna.browseServer(_currDLNAsrvNr, _dlnaHistory[_dlnaLevel].objId);
                                        return;}

    CMD_STARTS_WITH("SD/"){             if(!webSrv.streamfile(SD_MMC, scaleImage(cmd + 2))){ // without "SD"                                               // via XMLHttpRequest
                                            SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "The file could not be transferred " ANSI_ESC_RED "\"%s\"", cmd + 2);
                                            webSrv.sendStatus(404);} // not found
                                        //    webSrv.streamfile(SD_MMC, scaleImage("/common/unknown.jpg"));}
                                        return;}

    CMD_EQUALS("SD_Download"){          webSrv.streamfile(SD_MMC, param.c_str());                                                                         // via XMLHttpRequest
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Load from SD  " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                        return;}

    CMD_EQUALS("SD_GetFolder"){         webSrv.reply(SD_stringifyDirContent(param), webSrv.JS);                                                           // via XMLHttpRequest
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
                                        _playerSubMenue = 1;
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
                                        _settings.irbuttons[btnNr].val = command;
                                        return;}

    CMD_EQUALS("setIRadr"){             SerialPrintfln("set_IR_adr:  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "%s",
                                        param.c_str());
                                        int32_t address = (int32_t)strtol(param.c_str(), NULL, 16);
                                        ir.set_irAddress(address);
                                        _settings.irbuttons[42].val = address;
                                        return;}

    CMD_EQUALS("getTimeFormat"){        webSrv.send("timeFormat=", String(_timeFormat, 10));
                                        return;}

    CMD_EQUALS("setTimeFormat"){        _timeFormat = param.toInt();
                                        if(_state == CLOCK){clearWithOutHeaderFooter();}
                                        SerialPrintfln("TimeFormat:  " ANSI_ESC_YELLOW "new time format: " ANSI_ESC_BLUE "%sh", param.c_str());
                                        return;}

    CMD_EQUALS("getSleepMode"){         webSrv.send("sleepMode=", String(_sleepMode, 10)); return;}

    CMD_EQUALS("setSleepMode"){         _sleepMode = param.toInt();
                                        if(_sleepMode == 0) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Display off");
                                        if(_sleepMode == 1) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Show the time");
                                        return;}

    CMD_EQUALS("DLNA_GetFolder"){       webSrv.sendStatus(306); return;}  // todo
    CMD_EQUALS("KCX_BT_connected") {    if     (!_f_BTpower)              webSrv.send("KCX_BT_connected=", "-1");
                                        else if(bt_emitter.isConnected()) webSrv.send("KCX_BT_connected=",  "1");
                                        else                              webSrv.send("KCX_BT_connected=",  "0");
                                        return;}
    CMD_EQUALS("KCX_BT_clearItems"){    bt_emitter.deleteVMlinks(); return;}
    CMD_EQUALS("KCX_BT_addName"){       bt_emitter.addLinkName(param.c_str()); return;}
    CMD_EQUALS("KCX_BT_addAddr"){       bt_emitter.addLinkAddr(param.c_str()); return;}
    CMD_EQUALS("KCX_BT_mem"){           bt_emitter.getVMlinks(); return;}
    CMD_EQUALS("KCX_BT_scanned"){       webSrv.send("KCX_BT_SCANNED=", bt_emitter.stringifyScannedItems()); return;}
    CMD_EQUALS("KCX_BT_getMode"){       webSrv.send("KCX_BT_MODE=", bt_emitter.getMode()); return;}
    CMD_EQUALS("KCX_BT_changeMode"){    bt_emitter.changeMode(); return;}
    CMD_EQUALS("KCX_BT_pause"){         bt_emitter.pauseResume(); return;}
    CMD_EQUALS("KCX_BT_downvolume"){    if(_BTvolume > 0)  {_BTvolume--; bt_emitter.downvolume();} return;}
    CMD_EQUALS("KCX_BT_upvolume"){      if(_BTvolume < 31) {_BTvolume++; bt_emitter.upvolume();}   return;}
    CMD_EQUALS("KCX_BT_getPower"){      if(_f_BTpower) webSrv.send("KCX_BT_power=", "1"); else webSrv.send("KCX_BT_power=", "0"); return;}
    CMD_EQUALS("KCX_BT_power"){         _f_BTpower = !_f_BTcurPowerState; BTpowerChanged(!_f_BTcurPowerState); return;}

    CMD_EQUALS("hardcopy"){             SerialPrintfln("Webpage: ... " ANSI_ESC_YELLOW "create a display hardcopy"); hardcopy(); webSrv.send("hardcopy=", "/hardcopy.bmp"); return;}

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
    if(startsWith(info, "WebSocket")) return;      // suppress WebSocket client available
    if(!strcmp("ping", info)) return;              // suppress ping
    if(!strcmp("to_listen", info)) return;         // suppress to_isten
    if(startsWith(info, "Command client")) return; // suppress Command client available
    if(startsWith(info, "Content-D")) return;      // Content-Disposition
    if(startsWith(info, "test=")) return;          // stackHighWaterMark
    // SerialPrintfln("WebSrv Info: " ANSI_ESC_YELLOW "%s", info); // infos for debug
}

void WEBSRV_onError(const char* info) {
    SerialPrintfln("WebSrv Err:  " ANSI_ESC_RED "%s", info);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  Events from DLNA
void dlna_info(const char* info) {
    if(endsWith(info, "is not responding after request")) { // timeout
        _f_dlnaBrowseServer = false;
        if(_dlnaLevel > 0) _dlnaLevel--;
        lst_DLNA.show(_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
        setTimeCounter(LIST_TIMER);
    }
    SerialPrintfln("DLNA_info:   %s", info);
}

void dlna_server(uint8_t serverId, const char* IP_addr, uint16_t port, const char* friendlyName, const char* controlURL) {
    SerialPrintfln("DLNA_server: [%d] " ANSI_ESC_CYAN "%s:%d " ANSI_ESC_YELLOW " %s", serverId, IP_addr, port, friendlyName);
}

void dlna_seekReady(uint8_t numberOfServer) { SerialPrintfln("DLNA_server: %i media server found", numberOfServer); }

void dlna_browseResult(const char* objectId, const char* parentId, uint16_t childCount, const char* title, bool isAudio, uint32_t itemSize, const char* duration, const char* itemURL) {
    SerialPrintfln("DLNA_server: " ANSI_ESC_YELLOW "title %s, childCount %d, itemSize %ld, duration %s", title, childCount, (long unsigned int)itemSize, duration);
}

void dlna_browseReady(uint16_t numberReturned, uint16_t totalMatches) {
    SerialPrintfln("DLNA_server: returned %i from %i", numberReturned /*+ _totalNumberReturned*/, totalMatches);
    _dlnaMaxItems = totalMatches;
    _totalNumberReturned += numberReturned;
    if(numberReturned == 50 && !_f_dlnaMakePlaylistOTF) { // next round
        if(_totalNumberReturned < totalMatches && _totalNumberReturned < 500) { _f_dlnaBrowseServer = true; }
    }
    if(_f_dlnaWaitForResponse) {
        _f_dlnaWaitForResponse = false;
        lst_DLNA.show(_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
        setTimeCounter(LIST_TIMER);
    }
    else { webSrv.send("dlnaContent=", dlna.stringifyContent()); }
    if(_totalNumberReturned == totalMatches || _totalNumberReturned == 500 || _f_dlnaMakePlaylistOTF){
        _totalNumberReturned = 0;
        _f_dlna_browseReady = true; // last item received
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void kcx_bt_info(const char* info, const char* val) {
    if(endsWith(info, "Emitter found")){
        _f_BtEmitterFound = true;
        if(_btEmitterMode) bt_emitter.setMode(_btEmitterMode); else bt_emitter.setMode("TX");
        bt_emitter.userCommand("AT+GMR?");                      // get version
        bt_emitter.userCommand("AT+VOL?");                      // get volume (in receiver mode 0 ... 31)
        bt_emitter.userCommand("AT+BT_MODE?");                  // transmitter or receiver
        if(!_f_BTpower) bt_emitter.userCommand("AT+POWER_OFF"); // forced by user
    }

    if(startsWith(info, "Volume")){
        char c[10]; sprintf(c, "Vol: %s", val); txt_BT_volume.writeText(c);
        if(_BTvolume != atoi(val)) bt_emitter.setVolume(_BTvolume);
    }
    if(startsWith(info, "Mode")){
        txt_BT_mode.writeText(val);
    }
    if(startsWith(info, "POWER OFF")){
        _f_BTcurPowerState = false;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
        webSrv.send("KCX_BT_power=", "0");
        if(_state == BLUETOOTH) {
            btn_BT_power.setValue(false);
            pic_BT_mode.setPicturePath("/common/BToff.png");
            pic_BT_mode.show(true, false);
        }
        webSrv.send("KCX_BT_connected=", "-1");
    }
    if(startsWith(info, "POWER ON")) {
        _f_BTcurPowerState = true;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
        webSrv.send("KCX_BT_power=", "1");
        if(_state == BLUETOOTH) {
            btn_BT_power.setValue(true);
            pic_BT_mode.setPicturePath("/common/BTnc.png");
            pic_BT_mode.show(true, false);
        }
    }
    // log_w("BT-Emitter:  " ANSI_ESC_YELLOW "%s" ANSI_ESC_CYAN " %s", info, val);
    SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
}

void kcx_bt_status(bool status) { // is always called when the status changes from disconnected to connected and vice versa

    if(status) {
        if(!_f_BTcurPowerState) return;
        const char* mode = bt_emitter.getMode();
        webSrv.send("KCX_BT_connected=", "1");
        if(strcmp(mode, "TX") == 0) pic_BT_mode.setPicturePath("/common/BTgold.png");
        else                      { pic_BT_mode.setPicturePath("/common/BTblue.png"); muteChanged(true);}
    }
    else {
        webSrv.send("KCX_BT_connected=", "0");
        pic_BT_mode.setPicturePath("/common/BTnc.png"); // not connected
    }
    if(_state == BLUETOOTH) pic_BT_mode.show(true, false);
}

void kcx_bt_memItems(const char* jsonItems) { // Every time an item (name or address) was added, a JSON string is passed here
    // SerialPrintfln("bt_memItems %s", jsonItems);
    webSrv.send("KCX_BT_MEM=", jsonItems);
}

void kcx_bt_scanItems(const char* jsonItems) { // Every time an item (name and address) was scanned, a JSON string is passed here
    // SerialPrintfln("bt_scanItems %s", jsonItems);
    webSrv.send("KCX_BT_SCANNED=", jsonItems);
}

void kcx_bt_modeChanged(const char* m) { // Every time the mode has changed
    if(strcmp("RX", m) == 0) {
        webSrv.send("KCX_BT_MODE=RX");
    }
    if(strcmp("TX", m) == 0) {
        webSrv.send("KCX_BT_MODE=TX");
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// clang-format off
void graphicObjects_OnChange(const char* name, int32_t arg1) {
    char c[10];
    if(strcmp(name, "sdr_RA_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_PL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_DL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_CL_volume") == 0) {setTimeCounter(2); setVolume(arg1); return;}
    if(strcmp(name, "sdr_BR_value") == 0)  {_brightness = arg1; setTFTbrightness(arg1); txt_BR_value.writeText(int2str(arg1)); return;}
    if(strcmp(name, "sdr_E_LP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_lowPass.writeText(c);  _toneLP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_bandPass.writeText(c); _toneBP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_HP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_highPass.writeText(c); _toneHP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BAL") == 0) {itoa(arg1, c, 10); strcat(c, " ");   txt_EQ_balance.writeText(c);  _toneBAL = arg1; webSrv.send("settone=", setI2STone()); return;}

    log_d("unused event: graphicObject %s was changed, val %li", name, arg1);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnClick(const char* name, uint8_t val) { // val = 0 --> is inactive
    // all state
    if(val == 3 && !strcmp(name, "dispFooter"))      {setTimeCounter(0); return;} // pos 3 is RSSI or TC

    if(_state == RADIO) {
        if( val && !strcmp(name, "btn_RA_mute"))     {setTimeCounter(2); {if(!_f_mute) _f_muteIsPressed = true;} return;}
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
    if(_state == STATIONSLIST) {
        if( val && !strcmp(name, "lst_RADIO"))       {setTimeCounter(LIST_TIMER); return;}
    }
    if(_state == PLAYER) {
        if( val && !strcmp(name, "btn_PL_mute"))     {{if(!_f_mute) _f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_PL_pause"))    {return;}
        if( val && !strcmp(name, "btn_PL_cancel"))   {return;}
        if( val && !strcmp(name, "btn_PL_prevFile")) {if(_cur_AudioFileNr > 0) {_cur_AudioFileNr--; showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && !strcmp(name, "btn_PL_nextFile")) {if(_cur_AudioFileNr + 1 < _SD_content.getSize()) {_cur_AudioFileNr++; showFileName(_SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && !strcmp(name, "btn_PL_ready"))    {return;}
        if( val && !strcmp(name, "btn_PL_playAll"))  {return;}
        if( val && !strcmp(name, "btn_PL_shuffle"))  {return;}
        if( val && !strcmp(name, "btn_PL_fileList")) {return;}
        if( val && !strcmp(name, "btn_PL_radio"))    {return;}
        if( val && !strcmp(name, "btn_PL_off"))      {return;}
        if( val && !strcmp(name, "btn_PL_playPrev")) {_cur_AudioFileNr = _SD_content.getPrevAudioFile(_cur_AudioFileNr); return;}
        if( val && !strcmp(name, "btn_PL_playNext")) {_cur_AudioFileNr = _SD_content.getNextAudioFile(_cur_AudioFileNr); return;}
        if( val && !strcmp(name, "pgb_PL_progress")) {return;}
    }
    if(_state == AUDIOFILESLIST) {
        if( val && !strcmp(name, "lst_PLAYER"))      {setTimeCounter(LIST_TIMER); return;}
    }
    if(_state == DLNA) {
        if( val && !strcmp(name, "btn_DL_mute"))     {{if(!_f_mute) _f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_DL_pause"))    {return;}
        if( val && !strcmp(name, "btn_DL_radio"))    {return;}
        if( val && !strcmp(name, "btn_DL_fileList")) {return;}
        if( val && !strcmp(name, "btn_DL_cancel"))   {clearStationName(); btn_DL_pause.setActive(false); return;}
        if( val && !strcmp(name, "pgb_DL_progress")) {return;}
    }
    if(_state == DLNAITEMSLIST) {
        if( val && !strcmp(name, "lst_DLNA"))        {setTimeCounter(15); _f_dlnaWaitForResponse = true; return;}
    }
    if(_state == CLOCK) {
        if( val && !strcmp(name, "btn_CL_mute"))     {setTimeCounter(2); if(!_f_mute){ _f_muteIsPressed = true;} return;}
        if( val && !strcmp(name, "btn_CL_alarm"))    {return;}
        if( val && !strcmp(name, "btn_CL_radio"))    {return;}
        if( val && !strcmp(name, "clk_CL_12"))       {return;}
        if( val && !strcmp(name, "clk_CL_24"))       {return;}
        if( val && !strcmp(name, "btn_CL_off"))      {return;}
    }
    if(_state == ALARMCLOCK) {
        if( val && !strcmp(name, "clk_AC_red"))      {return;}
        if( val && !strcmp(name, "btn_AC_left"))     {return;}
        if( val && !strcmp(name, "btn_AC_right"))    {return;}
        if( val && !strcmp(name, "btn_AC_up"))       {return;}
        if( val && !strcmp(name, "btn_AC_down"))     {return;}
        if( val && !strcmp(name, "btn_AC_ready"))    {return;}
    }
    if(_state == SLEEPTIMER) {
        if( val && !strcmp(name, "btn_SL_up"))       {return;}
        if( val && !strcmp(name, "btn_SL_down"))     {return;}
        if( val && !strcmp(name, "btn_SL_ready"))    {return;}
        if( val && !strcmp(name, "btn_SL_cancel"))   {return;}
    }
    if(_state == SETTINGS) {
        if( val && !strcmp(name, "btn_SE_bright"))   {return;}
        if( val && !strcmp(name, "btn_SE_equal"))    {return;}
        if( val && !strcmp(name, "btn_SE_wifi"))    {return;}
        if( val && !strcmp(name, "btn_SE_radio"))    {return;}
    }
    if(_state == BRIGHTNESS){
        if( val && !strcmp(name, "btn_BR_ready"))    {return;}
        if( val && !strcmp(name, "pic_BR_logo"))     {return;}
    }
    if(_state == EQUALIZER) {
        if( val && !strcmp(name, "btn_E_LP"))        {sdr_EQ_lowPass.setValue(0);  return;}
        if( val && !strcmp(name, "btn_E_BP"))        {sdr_EQ_bandPass.setValue(0); return;}
        if( val && !strcmp(name, "btn_E_HP"))        {sdr_EQ_highPass.setValue(0); return;}
        if( val && !strcmp(name, "btn_E_BAL"))       {sdr_EQ_balance.setValue(0);  return;}
        if( val && !strcmp(name, "btn_EQ_Radio"))    {return;}
        if( val && !strcmp(name, "btn_EQ_Player"))   {return;}
        if( val && !strcmp(name, "btn_EQ_mute"))     {{if(!_f_mute) _f_muteIsPressed = true;} return;}
    }
    if(_state == BLUETOOTH) {
        if( val && !strcmp(name, "btn_BT_pause"))    {bt_emitter.pauseResume(); return;}
        if( val && !strcmp(name, "btn_BT_radio"))    {return;}
        if( val && !strcmp(name, "btn_BT_volDown"))  {if(_BTvolume > 0)  {_BTvolume--; bt_emitter.downvolume();} return;}
        if( val && !strcmp(name, "btn_BT_volUp"))    {if(_BTvolume < 31) {_BTvolume++; bt_emitter.upvolume();}  return;}
        if( val && !strcmp(name, "btn_BT_mode"))     {bt_emitter.changeMode(); return;}
        if( val && !strcmp(name, "btn_BT_power"))    {return;}
    }
    if(_state == IR_SETTINGS) {
        if( val && !strcmp(name, "btn_IR_radio"))    {return;}
    }
    if(_state == WIFI_SETTINGS) {
        if( val && !strcmp(name, "key_WI_input"))    {log_e("val %i", val); if(val == 13){ _radioSubMenue = 0; changeState(RADIO); return;}}
    }
    log_d("unused event: graphicObject %s was clicked", name);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnRelease(const char* name, releasedArg ra) {

    // all state
        if(!strcmp(name, "dispFooter"))      {return;}

    if(_state == RADIO) {
        if(!strcmp(name, "btn_RA_mute"))     {muteChanged(btn_RA_mute.getValue()); return;}
        if(!strcmp(name, "btn_RA_prevSta"))  {prevFavStation(); dispFooter.updateStation(_cur_station); return;}
        if(!strcmp(name, "btn_RA_nextSta"))  {nextFavStation(); dispFooter.updateStation(_cur_station); return;}
        if(!strcmp(name, "btn_RA_staList"))  {_radioSubMenue = 0; changeState(STATIONSLIST); return;}
        if(!strcmp(name, "btn_RA_player"))   {_playerSubMenue = 0; stopSong(); _playerSubMenue = 0; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_RA_dlna"))     {_dlnaSubMenue = 0; stopSong(); _dlnaSubMenue = 0; changeState(DLNA); return;}
        if(!strcmp(name, "btn_RA_clock"))    {_clockSubMenue = 0; _clockSubMenue = 0; changeState(CLOCK); return;}
        if(!strcmp(name, "btn_RA_sleep"))    {_radioSubMenue = 0; changeState(SLEEPTIMER); return;}
        if(!strcmp(name, "btn_RA_settings")) {_settingsSubMenue = 0; changeState(SETTINGS); return;}
        if(!strcmp(name, "btn_RA_equal"))    {_radioSubMenue = 0; changeState(EQUALIZER); return;}
        if(!strcmp(name, "btn_RA_bt"))       {_radioSubMenue = 0; changeState(BLUETOOTH); return;}
        if(!strcmp(name, "btn_RA_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "VUmeter_RA"))      {return;}
        if(!strcmp(name, "sdr_RA_volume"))   {return;}
    }
    if(_state == STATIONSLIST) {
        if(!strcmp(name, "lst_RADIO"))       {if(ra.val1){_radioSubMenue = 0; setStationByNumber(ra.val1); _radioSubMenue = 0; changeState(RADIO);} return;}
    }
    if(_state == PLAYER) {
        if(!strcmp(name, "btn_PL_mute"))     {muteChanged(btn_PL_mute.getValue()); return;}
        if(!strcmp(name, "btn_PL_pause"))    {if(_f_isFSConnected) {_f_pauseResume = audio.pauseResume();} return;}
        if(!strcmp(name, "btn_PL_cancel"))   {_playerSubMenue = 0; stopSong(); changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_prevFile")) {return;}
        if(!strcmp(name, "btn_PL_nextFile")) {return;}
        if(!strcmp(name, "btn_PL_ready"))    {stopSong(); SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); _playerSubMenue = 1; changeState(PLAYER); showAudioFileNumber(); return;}
        if(!strcmp(name, "btn_PL_playAll"))  { _f_shuffle = false; preparePlaylistFromSDFolder(_cur_AudioFolder); processPlaylist(true); _playerSubMenue = 1; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_shuffle"))  { _f_shuffle = true; preparePlaylistFromSDFolder(_cur_AudioFolder); processPlaylist(true); _playerSubMenue = 1; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_PL_fileList")) {_SD_content.listFilesInDir(_cur_AudioFolder, true, false); changeState(AUDIOFILESLIST); return;}
        if(!strcmp(name, "btn_PL_radio"))    {setStation(_cur_station);_playerSubMenue = 0; _radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_PL_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "sdr_PL_volume"))   {return;}
        if(!strcmp(name, "btn_PL_playNext")) {SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); return;}
        if(!strcmp(name, "btn_PL_playPrev")) {SD_playFile(_cur_AudioFolder, _SD_content.getColouredSStringByIndex(_cur_AudioFileNr)); showAudioFileNumber(); return;}
        if(!strcmp(name, "pgb_PL_progress")) {audio.setTimeOffset(ra.val2); return;}
    }
    if(_state == AUDIOFILESLIST){
        if(!strcmp(name, "lst_PLAYER"))      {if(ra.val1 == 1){;} // wipe up/down
                                              if(ra.val1 == 2){x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = strdup(ra.arg1); _cur_AudioFileNr = ra.val2; lst_PLAYER.show(_cur_AudioFolder, _cur_AudioFileNr);   } // next prev folder
                                              if(ra.val1 == 3){x_ps_free(&_cur_AudioFolder); _cur_AudioFolder = strdup(ra.arg1); _cur_AudioFileNr = ra.val2; stopSong(); SD_playFile(ra.arg3);} return;}
    }
    if(_state == DLNA) {
        if(!strcmp(name, "btn_DL_mute") )    {muteChanged(btn_DL_mute.getValue()); return;}
        if(!strcmp(name, "btn_DL_pause"))    {_f_pauseResume = audio.pauseResume(); return;}
        if(!strcmp(name, "btn_DL_radio"))    {setStation(_cur_station); txt_DL_fName.setText(""); _radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_DL_fileList")) {changeState(DLNAITEMSLIST); txt_DL_fName.setText(""); return;}
        if(!strcmp(name, "btn_DL_cancel"))   {stopSong(); txt_DL_fName.setText(""); txt_DL_fName.show(true, false); pgb_DL_progress.reset(); btn_DL_pause.setActive(false); btn_DL_pause.show(); return;}
        if(!strcmp(name, "sdr_DL_volume"))   {return;}
        if(!strcmp(name, "pgb_DL_progress")) {audio.setTimeOffset(ra.val2); return;}
    }
    if(_state == DLNAITEMSLIST) {
        if(!strcmp(name, "lst_DLNA"))        {if(ra.val1 == 1){txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(ra.arg2); connecttohost(ra.arg1); _dlnaSubMenue = 0; changeState(DLNA);} // play a file
                                              if(ra.val1 == 2){dlna.browseServer(ra.val2, ra.arg1, 0, 50); _f_dlnaMakePlaylistOTF = true; } // browse dlna object, waiting for content and create a playlist
                                              return;}
    }
    if(_state == CLOCK) {
        if(!strcmp(name, "btn_CL_mute"))     {muteChanged(btn_CL_mute.getValue()); return;}
        if(!strcmp(name, "btn_CL_alarm"))    {changeState(ALARMCLOCK); return;}
        if(!strcmp(name, "btn_CL_radio"))    {_clockSubMenue = 0; _radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "clk_CL_12"))       {_clockSubMenue = 1; changeState(CLOCK); return;}
        if(!strcmp(name, "clk_CL_24"))       {_clockSubMenue = 1; changeState(CLOCK); return;}
        if(!strcmp(name, "btn_CL_off"))      {fall_asleep(); return;}
        if(!strcmp(name, "sdr_CL_volume"))   {return;}
    }
    if(_state == ALARMCLOCK) {
        if(!strcmp(name, "clk_AC_red"))      {return;}
        if(!strcmp(name, "btn_AC_left"))     {clk_AC_red.shiftLeft(); return;}
        if(!strcmp(name, "btn_AC_right"))    {clk_AC_red.shiftRight(); return;}
        if(!strcmp(name, "btn_AC_up"))       {clk_AC_red.digitUp(); return;}
        if(!strcmp(name, "btn_AC_down"))     {clk_AC_red.digitDown(); return;}
        if(!strcmp(name, "btn_AC_ready"))    {updateSettings(); _clockSubMenue = 0; changeState(CLOCK); logAlarmItems(); return;}
    }
    if(_state == SLEEPTIMER) {
        if(!strcmp(name, "btn_SL_up"))       {display_sleeptime(1); return;}
        if(!strcmp(name, "btn_SL_down"))     {display_sleeptime(-1); return;}
        if(!strcmp(name, "btn_SL_ready"))    {dispFooter.updateOffTime(_sleeptime); _radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_SL_cancel"))   {_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(_state == BRIGHTNESS){
        if(!strcmp(name, "btn_BR_ready"))    {_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "pic_BR_logo"))     {return;}
    }
    if(_state == EQUALIZER) {
        if(!strcmp(name, "btn_EQ_Radio"))    {setStation(_cur_station); _radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_EQ_Player"))   {_playerSubMenue = 0; changeState(PLAYER); return;}
        if(!strcmp(name, "btn_EQ_mute"))     {muteChanged(btn_EQ_mute.getValue()); return;}
    }
    if(_state == SETTINGS) {
        if(!strcmp(name, "btn_SE_bright"))   { changeState(BRIGHTNESS);    return;}
        if(!strcmp(name, "btn_SE_equal"))    { changeState(EQUALIZER);     return;}
        if(!strcmp(name, "btn_SE_wifi"))     { changeState(WIFI_SETTINGS); return;}
        if(!strcmp(name, "btn_SE_radio"))    {_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(_state == BLUETOOTH) {
        if(!strcmp(name, "btn_BT_pause"))    {return;}
        if(!strcmp(name, "btn_BT_radio"))    {_radioSubMenue = 0; changeState(RADIO); return;}
        if(!strcmp(name, "btn_BT_volDown"))  {return;}
        if(!strcmp(name, "btn_BT_volUp"))    {return;}
        if(!strcmp(name, "btn_BT_mode"))     {return;}
        if(!strcmp(name, "btn_BT_power"))    {_f_BTpower = !_f_BTcurPowerState; BTpowerChanged(!_f_BTcurPowerState); return;}
    }
    if(_state == IR_SETTINGS) {
        if(!strcmp(name, "btn_IR_radio"))    {_radioSubMenue = 0; changeState(RADIO); return;}
    }
    if(_state == WIFI_SETTINGS) {
        if(!strcmp(name, "wifiSettings"))    {setWiFiCredentials(ra.arg1, ra.arg2); ESP.restart();}
    }
    log_d("unused event: graphicObject %s was released", name);
}
// clang-format on