#include "common.h"
// clang-format off
/*****************************************************************************************************************************************************
    MiniWebRadio -- Webradio receiver for ESP32

    first release on 03/2017                                                                                                      */String Version ="\
    Version 3.3e Aug 03/2024                                                                                                                       ";

/*  2.8" color display (320x240px) with controller ILI9341 or HX8347D (SPI) or
    3.5" color display (480x320px) with controller ILI9486 or ILI9488 (SPI)


    SD_MMC is mandatory
    IR remote is optional
    BT Transmitter is optional

*****************************************************************************************************************************************************/

// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT. FOR PERSONAL USE IT IS SUPPLIED
// WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

// clang-format on

SET_LOOP_TASK_STACK_SIZE(10 * 1024);

// global variables

enum status {
    NONE = 0,
    RADIO = 1,
    PLAYER = 2,
    DLNA = 3,
    CLOCK = 4,
    BRIGHTNESS = 5,
    ALARM = 6,
    SLEEP = 7,
    STATIONSLIST = 8,
    AUDIOFILESLIST = 9,
    DLNAITEMSLIST = 10,
    BLUETOOTH = 11,
    EQUALIZER = 12,
    UNDEFINED = 255
};

char _hl_item[16][40]{"",                 // none
                      "Internet Radio",   // "* интернет-радио *"  "ραδιόφωνο Internet"
                      "Audio player",     // "** цифрово́й плеер **
                      "DLNA",             // Digital Living Network Alliance
                      "Clock",            // Clock "** часы́ **"  "** ρολόι **"
                      "Brightness",       // Brightness яркость λάμψη
                      "Alarm (hh:mm)",    // Alarm
                      "Off Timer (h:mm)", // "Sleeptimer" "Χρονομετρητής" "Таймер сна"
                      "Stations List",
                      "Audio Files",
                      "DLNA List",
                      "Bluetooth",
                      "Equalizer",
                      "",
                      ""
                      ""};

const uint16_t      _max_stations = 1000;
int8_t              _currDLNAsrvNr = -1;
uint8_t             _alarmdays = 0;
uint8_t             _cur_volume = 0;     // will be set from stored preferences
uint8_t             _BTvolume = 16;      // KCX-BT_Emitter volume
uint8_t             _ringVolume = 21;
uint8_t             _volumeAfterAlarm = 12;
uint8_t             _volumeSteps = 21;
uint8_t             _brightness = 0;
uint8_t             _state = UNDEFINED;  // statemaschine
uint8_t             _commercial_dur = 0; // duration of advertising
uint8_t             _cur_Codec = 0;
uint8_t             _numServers = 0;     //
uint8_t             _level = 0;
uint8_t             _timeFormat = 24;    // 24 or 12
uint8_t             _sleepMode = 0;      // 0 display off,     1 show the clock
uint8_t             _staListPos = 0;
uint8_t             _reconnectCnt = 0;
uint8_t             _WiFi_disconnectCnt = 0;
uint16_t            _staListNr = 0;
uint8_t             _fileListPos = 0;
uint8_t             _radioSubmenue = 0;
uint8_t             _playerSubmenue = 0;
uint8_t             _clockSubMenue = 0;
uint8_t             _ambientValue = 50;
uint16_t            _fileListNr = 0;
uint16_t            _irNumber = 0;
uint8_t             _itemListPos = 0; // DLNA items
uint16_t            _dlnaItemNr = 0;
uint8_t             _dlnaLevel = 0;
int8_t              _rssi_bt = -127;
int16_t             _alarmtime[7] = {0};  // in minutes (23:59 = 23 *60 + 59) [0] Sun, [1] Mon
int16_t             _toneLP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t             _toneBP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t             _toneHP = 0;          // -40 ... +6 (dB)        audioI2S
int16_t             _toneBAL = 0;         // -16...0....+16         audioI2S
uint16_t            _icyBitRate = 0;      // from http response header via event
uint32_t            _decoderBitRate = 0;  // from decoder via getBitRate(false)
uint16_t            _cur_station = 0;     // current station(nr), will be set later
uint16_t            _cur_AudioFileNr = 0; // position inside _SD_content
uint16_t            _sleeptime = 0;       // time in min until MiniWebRadio goes to sleep
uint16_t            _sum_stations = 0;
uint16_t            _plsCurPos = 0;
uint16_t            _totalNumberReturned = 0;
uint16_t            _dlnaMaxItems = 0;
uint16_t            _bh1750Value = 50;
uint32_t            _resumeFilePos = 0; //
uint32_t            _playlistTime = 0;  // playlist start time millis() for timeout
uint32_t            _settingsHash = 0;
uint32_t            _audioFileSize = 0;
uint32_t            _media_downloadPort = 0;
uint32_t            _audioCurrentTime = 0;
uint32_t            _audioFileDuration = 0;
uint8_t             _resetResaon = (esp_reset_reason_t)ESP_RST_UNKNOWN;
const char*         _pressBtn[8];
const char*         _releaseBtn[8];
const char*         _time_s = "";
char                _chbuf[512];
char                _fName[256];
char                _myIP[25] = {0};
char                _path[128];
char                _prefix[5] = "/s";
char                _commercial[25];
char                _icyDescription[512] = {};
char                _streamTitle[512] = {};
char*               _curAudioFolder = NULL;
char*               _lastconnectedfile = NULL;
char*               _stationURL = NULL;
char*               _JSONstr = NULL;
char*               _BT_metaData = NULL;
char*               _playlistPath = NULL;
bool                _f_rtc = false; // true if time from ntp is received
bool                _f_100ms = false;
bool                _f_1sec = false;
bool                _f_10sec = false;
bool                _f_1min = false;
bool                _f_mute = false;
bool                _f_muteIsPressed = false;
bool                _f_volumeDownIsPressed = false;
bool                _f_volumeUpIsPressed = false;
bool                _f_sleeping = false;
bool                _f_isWebConnected = false;
bool                _f_isFSConnected = false;
bool                _f_eof = false;
bool                _f_reconnect = false;
bool                _f_eof_alarm = false;
bool                _f_alarm = false;
bool                _f_irNumberSeen = false;
bool                _f_newIcyDescription = false;
bool                _f_newStreamTitle = false;
bool                _f_newBitRate = false;
bool                _f_newStationName = false;
bool                _f_newCommercial = false;
bool                _f_volBarVisible = false;
bool                _f_switchToClock = false;    // jump into CLOCK mode at the next opportunity
bool                _f_hpChanged = false;        // true, if HeadPhone is plugged or unplugged
bool                _f_timeAnnouncement = false; // time announcement every full hour
bool                _f_playlistEnabled = false;
bool                _f_playlistNextFile = false;
bool                _f_logoUnknown = false;
bool                _f_pauseResume = false;
bool                _f_accessPoint = false;
bool                _f_SD_Upload = false;
bool                _f_PSRAMfound = false;
bool                _f_FFatFound = false;
bool                _f_SD_MMCfound = false;
bool                _f_ESPfound = false;
bool                _f_BH1750_found = false;
bool                _f_clearLogo = false;
bool                _f_clearStationName = false;
bool                _f_shuffle = false;
bool                _f_dlnaBrowseServer = false;
bool                _f_dlnaWaitForResponse = false;
bool                _f_dlnaSeekServer = false;
bool                _f_dlnaMakePlaylistOTF = false; // notify callback that this browsing was to build a On-The_fly playlist
bool                _f_dlna_browseReady = false;
bool                _f_BtEmitterFound = false;
bool                _f_BTEmitterConnected = false;
bool                _f_brightnessIsChangeable = false;
bool                _f_connectToLasthost = false;
bool                _f_BTpower = false;
bool                _f_BTcurPowerState = false;
bool                _f_timeSpeech = false;
String              _station = "";
String              _stationName_nvs = "";
char*               _stationName_air = NULL;
String              _homepage = "";
String              _filename = "";
String              _lastconnectedhost = "";
String              _scannedNetworks = "";
String              _TZName = "Europe/Berlin";
String              _TZString = "CET-1CEST,M3.5.0,M10.5.0/3";
String              _media_downloadIP = "";
dlnaHistory         _dlnaHistory[10];
timecounter         _timeCounter;
SD_content          _SD_content;
std::vector<char*>  _PLS_content;

const char* codecname[10] = {"unknown", "WAV", "MP3", "AAC", "M4A", "FLAC", "AACP", "OPUS", "OGG", "VORBIS"};


Preferences    pref;
Preferences    stations;
WebSrv         webSrv;
WiFiMulti      wifiMulti;
RTIME          rtc;
Ticker         ticker100ms;
IR             ir(IR_PIN); // do not change the objectname, it must be "ir"
TP             tp(TP_CS, TP_IRQ);
File           audioFile;
FtpServer      ftpSrv;
WiFiClient     client;
WiFiUDP        udp;
DLNA_Client    dlna;
KCX_BT_Emitter bt_emitter(BT_EMITTER_RX, BT_EMITTER_TX, BT_EMITTER_LINK, BT_EMITTER_MODE);
TwoWire        i2cBusOne = TwoWire(0); // additional HW, sensors, buttons, encoder etc
TwoWire        i2cBusTwo = TwoWire(1); // external DAC, AC101 or ES8388
hp_BH1750      BH1750(&i2cBusOne);     // create the sensor

#if DECODER == 2 // ac101
AC101 dac(&i2cBusTwo);
#endif
#if DECODER == 3 // es8388
ES8388 dac(&i2cBusTwo);
#endif

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

const uint8_t _fonts[9] = { 15, 16, 21, 25, 27, 34, 38, 43, 156};


struct w_h  {uint16_t x =   0; uint16_t y =   0; uint16_t w = 320; uint16_t h =  20;} const _winHeader;
struct w_l  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 100; uint16_t h = 100;} const _winLogo;
struct w_n  {uint16_t x = 100; uint16_t y =  20; uint16_t w = 220; uint16_t h = 100;} const _winName;
struct w_e  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 100;} const _winFName;
struct w_j  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 100; uint16_t h =  46;} const _winFileNr;
struct w_t  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;} const _winTitle;
struct w_c  {uint16_t x =   0; uint16_t y = 120; uint16_t w = 296; uint16_t h = 100;} const _winSTitle;
struct w_g  {uint16_t x = 296; uint16_t y = 120; uint16_t w =  24; uint16_t h = 100;} const _winVUmeter;
struct w_f  {uint16_t x =   0; uint16_t y = 220; uint16_t w = 320; uint16_t h =  20;} const _winFooter;
struct w_s  {uint16_t x =   0; uint16_t y = 220; uint16_t w =  60; uint16_t h =  20;} const _winStaNr;
struct w_p  {uint16_t x =  60; uint16_t y = 220; uint16_t w =  65; uint16_t h =  20;} const _winSleep;
struct w_r  {uint16_t x = 125; uint16_t y = 220; uint16_t w =  25; uint16_t h =  20;} const _winRSSID;
struct w_b  {uint16_t x =   0; uint16_t y = 150; uint16_t w = 320; uint16_t h =  30;} const _sdrOvBtns;
struct w_o  {uint16_t x =   0; uint16_t y = 180; uint16_t w =  40; uint16_t h =  40;} const _winButton;
struct w_d  {uint16_t x =   0; uint16_t y =  50; uint16_t w = 320; uint16_t h = 120;} const _winDigits;    // clock
struct w_y  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 160;} const _winAlarm;
struct w_w  {uint16_t x =   0; uint16_t y =  20; uint16_t w = 320; uint16_t h = 200;} const _winWoHF;      // without Header and Footer
struct w_s1 {uint16_t x =  80; uint16_t y =  30; uint16_t w = 150; uint16_t h =  34;} const _sdrLP;        // slider lowpass in equalizer
struct w_s2 {uint16_t x =  80; uint16_t y =  64; uint16_t w = 150; uint16_t h =  34;} const _sdrBP;        // slider bandpass in equalizer
struct w_s3 {uint16_t x =  80; uint16_t y =  98; uint16_t w = 150; uint16_t h =  34;} const _sdrHP;        // slider highpass in equalizer
struct w_s4 {uint16_t x =  80; uint16_t y = 132; uint16_t w = 150; uint16_t h =  34;} const _sdrBAL;       // slider balance in equalizer

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

const uint8_t _fonts[9] = {21, 25, 27, 34, 38, 43, 56, 66, 156};

struct w_h  {uint16_t x =   0; uint16_t y =   0; uint16_t w = 480; uint16_t h =  30;} const _winHeader;
struct w_l  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 130; uint16_t h = 132;} const _winLogo;
struct w_n  {uint16_t x = 132; uint16_t y =  30; uint16_t w = 348; uint16_t h = 132;} const _winName;
struct w_e  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 132;} const _winFName;
struct w_j  {uint16_t x =   0; uint16_t y = 162; uint16_t w = 130; uint16_t h =  60;} const _winFileNr;
struct w_t  {uint16_t x =   0; uint16_t y = 162; uint16_t w = 480; uint16_t h = 128;} const _winTitle;
struct w_c  {uint16_t x =   0; uint16_t y = 162; uint16_t w = 448; uint16_t h = 128;} const _winSTitle;
struct w_g  {uint16_t x = 448; uint16_t y = 162; uint16_t w =  32; uint16_t h = 128;} const _winVUmeter;
struct w_f  {uint16_t x =   0; uint16_t y = 290; uint16_t w = 480; uint16_t h =  30;} const _winFooter;
struct w_s  {uint16_t x =   0; uint16_t y = 290; uint16_t w =  85; uint16_t h =  30;} const _winStaNr;
struct w_p  {uint16_t x =  85; uint16_t y = 290; uint16_t w =  87; uint16_t h =  30;} const _winSleep;
struct w_r  {uint16_t x = 172; uint16_t y = 290; uint16_t w =  32; uint16_t h =  30;} const _winRSSID;
struct w_b  {uint16_t x =   0; uint16_t y = 194; uint16_t w = 480; uint16_t h =  40;} const _sdrOvBtns;   // slider over buttons, max width
struct w_o  {uint16_t x =   0; uint16_t y = 234; uint16_t w =  56; uint16_t h =  56;} const _winButton;
struct w_d  {uint16_t x =   0; uint16_t y =  70; uint16_t w = 480; uint16_t h = 160;} const _winDigits;
struct w_y  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 200;} const _winAlarm;
struct w_w  {uint16_t x =   0; uint16_t y =  30; uint16_t w = 480; uint16_t h = 260;} const _winWoHF;      // without Header and Footer
struct w_s1 {uint16_t x = 140; uint16_t y =  30; uint16_t w = 200; uint16_t h =  50;} const _sdrLP;        // slider lowpass in equalizer
struct w_s2 {uint16_t x = 140; uint16_t y =  80; uint16_t w = 200; uint16_t h =  50;} const _sdrBP;        // slider bandpass in equalizer
struct w_s3 {uint16_t x = 140; uint16_t y = 130; uint16_t w = 200; uint16_t h =  50;} const _sdrHP;        // slider highpass in equalizer
struct w_s4 {uint16_t x = 140; uint16_t y = 180; uint16_t w = 200; uint16_t h =  50;} const _sdrBAL;       // slider balance in equalizer

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

// ALL STATE
displayHeader dispHeader("dispHeader", _fonts[1]);
displayFooter dispFooter("dispFooter", _fonts[1]);
// RADIO
button2state  btn_RA_Mute("btn_RA_Mute");
button1state  btn_RA_prevSta("btn_RA_prevSta"), btn_RA_nextSta("btn_RA_nextSta");
button1state  btn_RA_staList("btn_RA_staList"), btn_RA_player("btn_RA_player"), btn_RA_dlna("btn_RA_dlna"), btn_RA_clock("btn_RA_clock");
button1state  btn_RA_sleep("btn_RA_sleep"), btn_RA_bright("btn_RA_bright"), btn_RA_equal("btn_RA_equal"), btn_RA_bt("btn_RA_bt");
button1state  btn_RA_off("btn_RA_off");
pictureBox    pic_RA_logo("pic_RA_logo");
textbox       txt_RA_sTitle("txt_RA_sTitle"), txt_RA_staName("txt_RA_staName"), txt_RA_irNum("txt_RA_irNum");
vuMeter       VUmeter_RA("VUmeter_RA");
slider        sdr_RA_volume("sdr_RA_volume");
// STATIONSLIST
stationsList  lst_RADIO("lst_RADIO");
// PLAYER
button2state  btn_PL_Mute("btn_PL_Mute"), btn_PL_pause("btn_PL_pause");
button1state  btn_PL_ready("btn_PL_ready"), btn_PL_shuffle("btn_PL_shuffle");
button1state  btn_PL_playAll("btn_PL_playAll"), btn_PL_fileList("btn_PL_fileList"), btn_PL_radio("btn_PL_radio"), btn_PL_cancel("btn_PL_cancel");
button1state  btn_PL_prevFile("btn_PL_prevFile"), btn_PL_nextFile("btn_PL_nextFile"), btn_PL_off("btn_PL_off");
textbox       txt_PL_fName("txt_PL_fName");
slider        sdr_PL_volume("sdr_PL_volume");
// AUDIOFILESLIST
fileList      lst_PLAYER("lst_PLAYER");
// DLNA
button2state  btn_DL_Mute("btn_DL_Mute"), btn_DL_pause("btn_DL_pause");
button1state  btn_DL_radio("btn_DL_radio"), btn_DL_fileList("btn_DL_fileList"), btn_DL_cancel("btn_DL_cancel");
textbox       txt_DL_fName("txt_DL_fName");
slider        sdr_DL_volume("sdr_DL_volume");
// DLNAITEMSLIST
dlnaList      lst_DLNA("lst_DLNA", &dlna, &_dlnaHistory[0], 10);
// CLOCK
imgClock      clk_CL_green("clk_CL_green");
button2state  btn_CL_Mute("btn_CL_Mute");
button1state  btn_CL_alarm("btn_CL_alarm"), btn_CL_radio("btn_CL_radio"), btn_CL_off("btn_CL_off");
slider        sdr_CL_volume("sdr_CL_volume");
// ALARM
alarmClock    clk_AL_red("clk_CL_green");
button1state  btn_AL_left("btn_AL_left"), btn_AL_right("btn_AL_right"), btn_AL_up("btn_AL_up"), btn_AL_down("btn_AL_down");
button1state  btn_AL_ready("btn_AL_ready");
// BRIGHTNESS
button1state  btn_BR_ready("btn_BR_ready");
pictureBox    pic_BR_logo("pic_BR_logo");
slider        sdr_BR_value("sdr_BR_value");
textbox       txt_BR_value("txt_BR_value");
// SLEEP
button1state  btn_SL_up("btn_SL_up"), btn_SL_down("btn_SL_down"), btn_SL_ready("btn_SL_ready"), btn_SL_cancel("btn_SL_cancel");
// EQUALIZER
slider        sdr_EQ_lowPass("sdr_E_LP"), sdr_EQ_bandPass("sdr_E_BP"), sdr_EQ_highPass("sdr_E_HP"), sdr_EQ_balance("sdr_E_BAL");
textbox       txt_EQ_lowPass("txt_E_LP"), txt_EQ_bandPass("txt_E_BP"), txt_EQ_highPass("txt_E_HP"), txt_EQ_balance("txt_E_BAL");
button1state  btn_EQ_lowPass("btn_E_LP");
button1state  btn_EQ_bandPass("btn_E_BP"), btn_EQ_highPass("btn_E_HP"), btn_EQ_balance("btn_E_BAL");
button1state  btn_EQ_Radio("btn_EQ_Radio"), btn_EQ_Player("btn_EQ_Player");
button2state  btn_EQ_Mute("btn_EQ_Mute");
// BLUETOOTH
button2state  btn_BT_pause("btn_BT_pause"), btn_BT_power("btn_BT_power");
button1state  btn_BT_volDown("btn_BT_volDown"), btn_BT_volUp("btn_BT_volUp"), btn_BT_radio("btn_BT_radio"), btn_BT_mode("btn_BT_mode");
pictureBox    pic_BT_mode("pic_BT_mode");
textbox       txt_BT_mode("txt_BT_mode");
textbox       txt_BT_volume("txt_BT_volume");

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     D E F A U L T S E T T I N G S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format off
boolean defaultsettings(){
    if(!SD_MMC.exists("/settings.json")){
        File file = SD_MMC.open("/settings.json","w", true);
        char*  jO = x_ps_malloc(1024); // JSON Object
        strcpy(jO, "{");
        strcat(jO, "\"volume\":");            strcat(jO, "12,"); // 0...volumeSteps
        strcat(jO, "\"volumeSteps\":");       strcat(jO, "21,"); // 21...255
        strcat(jO, "\"ringVolume\":");        strcat(jO, "21,"); // 0...volumeSteps
        strcat(jO, "\"volumeAfterAlarm\":");  strcat(jO, "16,"); // 0...volumeSteps
        strcat(jO, "\"BTpower\":");           strcat(jO, "\"false\","); // assume KCX_BT_Emitter not exists or is off
        strcat(jO, "\"alarmtime_sun\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarmtime_mon\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarmtime_tue\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarmtime_wed\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarmtime_fri\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarmtime_sat\":");     strcat(jO, "00:00,");
        strcat(jO, "\"alarm_weekdays\":");    strcat(jO, "0,");
        strcat(jO, "\"timeAnnouncing\":");    strcat(jO, "\"true\",");
        strcat(jO, "\"mute\":");              strcat(jO, "\"false\","); // no mute
        strcat(jO, "\"brightness\":");        strcat(jO, "100,");  // 0...100
        strcat(jO, "\"sleeptime\":");         strcat(jO, "0,");
        strcat(jO, "\"lastconnectedhost\":"); strcat(jO, "\"\",");
        strcat(jO, "\"station\":");           strcat(jO, "1,");
        strcat(jO, "\"sumstations\":");       strcat(jO, "0,");
        strcat(jO, "\"Timezone_Name\":");     strcat(jO, "\"Europe/Berlin\",");
        strcat(jO, "\"Timezone_String\":");   strcat(jO, "\"CET-1CEST,M3.5.0,M10.5.0/3\",");
        strcat(jO, "\"toneLP\":");            strcat(jO, "0,"); // -40 ... +6 (dB)        audioI2S
        strcat(jO, "\"toneBP\":");            strcat(jO, "0,"); // -40 ... +6 (dB)        audioI2S
        strcat(jO, "\"toneHP\":");            strcat(jO, "0,"); // -40 ... +6 (dB)        audioI2S
        strcat(jO, "\"balance\":");           strcat(jO, "0,"); // -16 ... +16            audioI2S
        strcat(jO, "\"timeFormat\":");        strcat(jO, "24,");
        strcat(jO, "\"sleepMode\":");         strcat(jO, "0}"); // 0 display off, 1 clock
        file.print(jO);
        if(jO){free(jO); jO = NULL;}
    }

    File file = SD_MMC.open("/settings.json","r", false);
    char*  jO = x_ps_calloc(1024, 1);
    char* tmp = x_ps_malloc(512);
    file.readBytes(jO, 1024);
    _settingsHash = simpleHash(jO);

    auto parseJson = [&](const char* s) { // lambda, inner function
        int16_t pos1 = 0, pos2 = 0, pos3 = 0;
        pos1 = indexOf(jO, s, 0);
        if(pos1 < 0) {log_e("index %s not found", s); return "0";}
        pos2 = indexOf(jO, ":", pos1) + 1;
        pos3 = indexOf(jO, ",\"", pos2);
        if(pos3 < 0) pos3 = indexOf(jO, "}", pos2);
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

    _cur_volume          = atoi(   parseJson("\"volume\":"));
    _volumeSteps         = atoi(   parseJson("\"volumeSteps\":"));
    _ringVolume          = atoi(   parseJson("\"ringVolume\":"));
    _volumeAfterAlarm    = atoi(   parseJson("\"volumeAfterAlarm\":"));
    _BTvolume            = atoi(   parseJson("\"BTvolume\":"));
    _f_BTpower           = (strcmp(parseJson("\"BTpower\":"), "true") == 0) ? 1 : 0;
    _alarmtime[0]        = computeMinuteOfTheDay(parseJson("\"alarmtime_sun\":"));
    _alarmtime[1]        = computeMinuteOfTheDay(parseJson("\"alarmtime_mon\":"));
    _alarmtime[2]        = computeMinuteOfTheDay(parseJson("\"alarmtime_tue\":"));
    _alarmtime[3]        = computeMinuteOfTheDay(parseJson("\"alarmtime_wed\":"));
    _alarmtime[4]        = computeMinuteOfTheDay(parseJson("\"alarmtime_thu\":"));
    _alarmtime[5]        = computeMinuteOfTheDay(parseJson("\"alarmtime_fri\":"));
    _alarmtime[6]        = computeMinuteOfTheDay(parseJson("\"alarmtime_sat\":"));
    _alarmdays           = atoi(   parseJson("\"alarm_weekdays\":"));
    _f_timeAnnouncement  = (strcmp(parseJson("\"timeAnnouncing\":"), "true") == 0) ? 1 : 0;
    _f_mute              = (strcmp(parseJson("\"mute\":"), "true") == 0) ? 1 : 0;
    _brightness          = atoi(   parseJson("\"brightness\":"));
    _sleeptime           = atoi(   parseJson("\"sleeptime\":"));
    _cur_station         = atoi(   parseJson("\"station\":"));
    _sum_stations        = atoi(   parseJson("\"sumstations\":"));
    _toneLP              = atoi(   parseJson("\"toneLP\":"));
    _toneBP              = atoi(   parseJson("\"toneBP\":"));
    _toneHP              = atoi(   parseJson("\"toneHP\":"));
    _toneBAL             = atoi(   parseJson("\"balance\":"));
    _timeFormat          = atoi(   parseJson("\"timeFormat\":"));
    _TZName              =         parseJson("\"Timezone_Name\":");
    _TZString            =         parseJson("\"Timezone_String\":");
    _lastconnectedhost   =         parseJson("\"lastconnectedhost\":");
    _sleepMode           = atoi(   parseJson("\"sleepMode\":"));


    if(!pref.isKey("stations_filled")|| _sum_stations == 0) saveStationsToNVS();  // first init
    if(pref.getShort("IR_numButtons", 0) == 0) saveDefaultIRbuttonsToNVS();
    loadIRbuttonsFromNVS();

    if(jO) {free(jO);   jO = NULL;}
    if(tmp){free(tmp); tmp = NULL;}
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
    if(file) { // try to read from SD_MMC
        stations.clear();
        currentLine = file.readStringUntil('\n'); // read the headline
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
    else return false;
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
        log_i("i=%i ir_buttons[i] %X", i, ir_buttons[i]);
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
    char*  jO = x_ps_malloc(1024 + _lastconnectedhost.length()); // JSON Object
    char tmp[40 + _lastconnectedhost.length()];
    strcpy(jO,   "{");
    sprintf(tmp, "\"volume\":%i", _cur_volume);                                             strcat(jO, tmp);
    sprintf(tmp, ",\"volumeSteps\":%i", _volumeSteps);                                      strcat(jO, tmp);
    sprintf(tmp, ",\"ringVolume\":%i", _ringVolume);                                        strcat(jO, tmp);
    sprintf(tmp, ",\"volumeAfterAlarm\":%i", _volumeAfterAlarm);                            strcat(jO, tmp);
    sprintf(tmp, ",\"BTvolume\":%i", _BTvolume);                                            strcat(jO, tmp);
    strcat(jO,   ",\"BTpower\":"); (_f_BTpower == true) ?                                   strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    sprintf(tmp, ",\"alarmtime_sun\":%02d:%02d", _alarmtime[0] / 60, _alarmtime[0] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_mon\":%02d:%02d", _alarmtime[1] / 60, _alarmtime[1] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_tue\":%02d:%02d", _alarmtime[2] / 60, _alarmtime[2] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_wed\":%02d:%02d", _alarmtime[3] / 60, _alarmtime[3] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_thu\":%02d:%02d", _alarmtime[4] / 60, _alarmtime[4] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_fri\":%02d:%02d", _alarmtime[5] / 60, _alarmtime[5] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarmtime_sat\":%02d:%02d", _alarmtime[6] / 60, _alarmtime[6] % 60);   strcat(jO, tmp);
    sprintf(tmp, ",\"alarm_weekdays\":%i", _alarmdays);                                     strcat(jO, tmp);
    strcat(jO,   ",\"timeAnnouncing\":"); (_f_timeAnnouncement == true) ?                   strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    strcat(jO,   ",\"mute\":");           (_f_mute == true)             ?                   strcat(jO, "\"true\"") : strcat(jO, "\"false\"");
    sprintf(tmp, ",\"brightness\":%i", _brightness);                                        strcat(jO, tmp);
    sprintf(tmp, ",\"sleeptime\":%i", _sleeptime);                                          strcat(jO, tmp);
    sprintf(tmp, ",\"lastconnectedhost\":\"%s\"", _lastconnectedhost.c_str());              strcat(jO, tmp);
    sprintf(tmp, ",\"station\":%i", _cur_station);                                          strcat(jO, tmp);
    sprintf(tmp, ",\"sumstations\":%i", _sum_stations);                                     strcat(jO, tmp);
    sprintf(tmp, ",\"Timezone_Name\":\"%s\"", _TZName.c_str());                             strcat(jO, tmp);
    sprintf(tmp, ",\"Timezone_String\":\"%s\"", _TZString.c_str());                         strcat(jO, tmp);
    sprintf(tmp, ",\"toneLP\":%i", _toneLP);                                                strcat(jO, tmp);
    sprintf(tmp, ",\"toneBP\":%i", _toneBP);                                                strcat(jO, tmp);
    sprintf(tmp, ",\"toneHP\":%i", _toneHP);                                                strcat(jO, tmp);
    sprintf(tmp, ",\"balance\":%i", _toneBAL);                                              strcat(jO, tmp);
    sprintf(tmp, ",\"timeFormat\":%i", _timeFormat);                                        strcat(jO, tmp);
    sprintf(tmp, ",\"sleepMode\":%i}", _sleepMode);                                          strcat(jO, tmp);

    if(_settingsHash != simpleHash(jO)) {
        File file = SD_MMC.open("/settings.json", "w", false);
        if(!file) {
            log_e("file \"settings.json\" not found");
            return;
        }
        file.print(jO);
        _settingsHash = simpleHash(jO);
    }
    if(jO){free(jO); jO = NULL;}
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
    if(_JSONstr) {
        free(_JSONstr);
        _JSONstr = NULL;
    }
    if(!_SD_content.listDir(path.c_str(), false, false)) return "[]"; // if success: result will be in _SD_content
    if(psramFound()) { _JSONstr = (char*)ps_malloc(2); }
    else { _JSONstr = (char*)malloc(2); }
    JSONstrLength += 2;
    memcpy(_JSONstr, "[\0", 2);
    if(!_SD_content.getSize()) return "[]"; // empty?

    for(int i = 0; i < _SD_content.getSize(); i++) { // build a JSON string in PSRAM, e.g. [{"name":"m","dir":true},{"name":"s","dir":true}]
        const char* fn = _SD_content.getIndex(i);
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
    static uint8_t semaphore = 0;
    _f_100ms = true;
    ms100 ++;
    if(!(ms100 % 10))   {
        _f_1sec  = true;
        _time_s = rtc.gettime_s();
        if(endsWith(_time_s, "59:53"))  _f_timeSpeech = true;
        if(!semaphore) { _f_alarm = clk_CL_green.isAlarm(_alarmdays, _alarmtime); }
        if(_f_alarm)        {semaphore++;}
        if(semaphore)       {semaphore++;}
        if(semaphore >= 65) {semaphore = 0;}
    }
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
inline void clearStaNr()              {tft.fillRect(_winStaNr.x,     _winStaNr.y,     _winStaNr.w,     _winStaNr.h,    TFT_BLACK);}
inline void clearSleep()              {tft.fillRect(_winSleep.x,     _winSleep.y,     _winSleep.w,     _winSleep.h,    TFT_BLACK);}
inline void clearDigits()             {tft.fillRect(_winDigits.x,    _winDigits.y,    _winDigits.w,    _winDigits.h,   TFT_BLACK);}
inline void clearButtonBar()          {tft.fillRect( 0,              _winButton.y,    _dispWidth,      _winButton.h,   TFT_BLACK);}
inline void clearAll()                {tft.fillScreen(TFT_BLACK);}                      // y   0...239
// clang-format on

inline uint16_t txtlen(String str) {
    uint16_t len = 0;
    for(int32_t i = 0; i < str.length(); i++)
        if(str[i] <= 0xC2) len++;
    return len;
}

void display_info(const char* str, int32_t xPos, int32_t yPos, uint16_t color, uint16_t margin_l, uint16_t margin_r, uint16_t winWidth, uint16_t winHeight) {
    tft.fillRect(xPos, yPos, winWidth, winHeight, TFT_BLACK); // Clear the space for new info
    tft.setTextColor(color);                                  // Set the requested color
    uint16_t ch_written = tft.writeText(str, xPos + margin_l, yPos, winWidth - margin_r, winHeight);
    if(ch_written < strlenUTF8(str)) {
        // If this message appears, there is not enough space on the display to write the entire text,
        // a part of the text has been cut off
        SerialPrintfln("txt overflow, winHeight=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", strlen=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", written=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE ", str=" ANSI_ESC_CYAN
                       "%s",
                       winHeight, strlenUTF8(str), ch_written, str);
    }
}

void showStreamTitle(const char* streamtitle) {
    if(_f_sleeping) return;

    char* st = x_ps_strdup(streamtitle);
    trim(st);
    replacestr(st, " | ", "\n"); // some stations use pipe as \n or
    replacestr(st, "| ", "\n");
    replacestr(st, "|", "\n");

    txt_RA_sTitle.setTextColor(TFT_CORNSILK);
    txt_RA_sTitle.writeText(st, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);

    if(st){free(st); st = NULL;}
}

void showLogoAndStationName(bool force) {
    char* SN_utf8 = NULL;
    static char* old_SN_utf8 = strdup("");
    if(force){
        if(old_SN_utf8){free(old_SN_utf8); old_SN_utf8 = strdup("");}
    }

    if(_cur_station) {
        SN_utf8 = x_ps_calloc(_stationName_nvs.length() + 12, 1);
        memcpy(SN_utf8, _stationName_nvs.c_str(), _stationName_nvs.length() + 1);
    }
    else {
        if(!_stationName_air) _stationName_air = strdup("");
        SN_utf8 = x_ps_calloc(strlen(_stationName_air) + 12, 1);
        memcpy(SN_utf8, _stationName_air,  strlen(_stationName_air) + 1);
    }
    trim(SN_utf8);
    if(strcmp(old_SN_utf8, SN_utf8) == 0) {goto exit;}
    if(old_SN_utf8){free(old_SN_utf8); old_SN_utf8 = NULL;}
    old_SN_utf8 = x_ps_strdup(SN_utf8);
    txt_RA_staName.setTextColor(TFT_CYAN);
    txt_RA_staName.writeText(SN_utf8, TFT_ALIGN_LEFT, TFT_ALIGN_TOP);

    memmove(SN_utf8  + 6, SN_utf8, strlen(SN_utf8) + 1);
    memmove(SN_utf8, "/logo/", 6);
    strcat(SN_utf8, ".jpg");
    pic_RA_logo.setPicturePath(SN_utf8);
    pic_RA_logo.setAlternativPicturePath("/common/unknown.jpg");
    pic_RA_logo.show();
exit:
    if(SN_utf8){free(SN_utf8); SN_utf8 = NULL;}
}

void showFileLogo(uint8_t state) {
    String logo;
    if(state == RADIO) {
        if(endsWith(_stationURL, "m3u8")) logo = "/common/" + (String) "M3U8" + ".jpg";
        else logo = "/common/" + (String)codecname[_cur_Codec] + ".jpg";
        drawImage(logo.c_str(), 0, _winName.y + 2);
        webSrv.send("stationLogo=", logo);
        return;
    }
    else if(state == DLNA) {
        logo = "/common/DLNA.jpg";
        drawImage(logo.c_str(), 0, _winName.y + 2);
        webSrv.send("stationLogo=", logo);
        return;
    }
    if(state == PLAYER) { // _state PLAYER
        if(_cur_Codec == 0) logo = "/common/AudioPlayer.jpg";
        else if(_playerSubmenue == 0) logo = "/common/AudioPlayer.jpg";
        else logo = "/common/" + (String)codecname[_cur_Codec] + ".jpg";
        drawImage(logo.c_str(), 0, _winName.y + 2);
        return;
    }
}

void showFileName(const char* fname) {
    if(!fname) return;
    txt_PL_fName.setTextColor(TFT_CYAN);
    txt_PL_fName.writeText(fname, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
}

void showPlsFileNumber() {
    tft.setFont(_fonts[3]);
    char buf[15];
    sprintf(buf, "%03u/%03u", _plsCurPos, _PLS_content.size());
    display_info(buf, _winFileNr.x, _winFileNr.y, TFT_ORANGE, 10, 0, _winFileNr.w, _winFileNr.h);
}

void showAudioFileNumber() {
    tft.setFont(_fonts[3]);
    char buf[15];
    sprintf(buf, "%03u/%03u", _cur_AudioFileNr, _SD_content.getSize());
    display_info(buf, _winFileNr.x, _winFileNr.y, TFT_ORANGE, 10, 0, _winFileNr.w, _winFileNr.h);
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
    if(endsWith(scImg, "bmp")) { return tft.drawBmpFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth); }
    if(endsWith(scImg, "jpg")) { return tft.drawJpgFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth); }
    if(endsWith(scImg, "gif")) { return tft.drawGifFile(SD_MMC, scImg, posX, posY, 0); }

    SerialPrintfln(ANSI_ESC_RED "the file \"%s\" contains neither a bmp, a gif nor a jpj graphic", scImg);
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
    if(_playlistPath) {
        free(_playlistPath);
        _playlistPath = NULL;
    }
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
    _playlistPath = strdup(playlistFile.path());
    int idx = lastIndexOf((const char*)_playlistPath, '/');
    if(idx < 0) log_e("wrong playlist path");
    _playlistPath[idx] = '\0';
    playlistFile.close();
    if(buff1) {
        free(buff1);
        buff1 = NULL;
    }
    if(buff2) {
        free(buff2);
        buff2 = NULL;
    }
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
    if(_PLS_content.size() == 0) {
        log_e("playlist is empty");
        return;
    } // guard
    int16_t idx = 0;
    boolean f_has_EXTINF = false;
    if(first) {
        _plsCurPos = 0;
        _f_playlistEnabled = true;
    }

    if(_plsCurPos == _PLS_content.size()) goto exit;

    _playerSubmenue = 1;
    if(_state != PLAYER) changeState(PLAYER);

    // now read from vector _PLS_content

    idx = indexOf(_PLS_content[_plsCurPos], "\n", 0);
    if(idx > 0) { // has additional infos: duration, title
        f_has_EXTINF = true;
        int16_t idx1 = indexOf(_PLS_content[_plsCurPos], ",", idx);
        SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "Title: %s", _PLS_content[_plsCurPos] + idx1 + 1);
        clearLogo();
        showFileName(_PLS_content[_plsCurPos] + idx1 + 1);
        int8_t len = idx1 - (idx + 1);
        if(len > 0 && len < 6) { // song playtime
            char tmp[7] = {0};
            memcpy(tmp, _PLS_content[_plsCurPos] + idx + 1, len);
            tmp[len] = '\0';
            SerialPrintfln("Playlist:    " ANSI_ESC_GREEN "playtime: %is", atoi(tmp));
        }
        _PLS_content[_plsCurPos][idx] = '\0';
    }
    if(startsWith(_PLS_content[_plsCurPos], "http")) {
        SerialPrintflnCut("Playlist:    ", ANSI_ESC_YELLOW, _PLS_content[_plsCurPos]);
        if(!f_has_EXTINF) clearLogoAndStationname();
        webSrv.send("SD_playFile=", _PLS_content[_plsCurPos]);
        _cur_Codec = 0;
        connecttohost(_PLS_content[_plsCurPos]);
    }
    else if(startsWith(_PLS_content[_plsCurPos], "file://")) { // root
        urldecode(_PLS_content[_plsCurPos]);
        SerialPrintfln("Playlist:    " ANSI_ESC_YELLOW "%s", _PLS_content[_plsCurPos] + 7);
        webSrv.send("SD_playFile=", _PLS_content[_plsCurPos] + 7);
        SD_playFile(_PLS_content[_plsCurPos] + 7, 0, false);
    }
    else {
        urldecode(_PLS_content[_plsCurPos]);
        char* playFile = NULL;
        if(_playlistPath) { // path of m3u file
            playFile = x_ps_malloc(strlen(_playlistPath) + strlen(_PLS_content[_plsCurPos]) + 5);
            strcpy(playFile, _playlistPath);
            if(_PLS_content[_plsCurPos][0] != '/') strcat(playFile, "/");
            strcat(playFile, _PLS_content[_plsCurPos]);
        }
        else { // have no playlistpath
            playFile = x_ps_malloc(strlen(_PLS_content[_plsCurPos]) + 5);
            strcpy(playFile, _PLS_content[_plsCurPos]);
        }
        SerialPrintfln("Playlist:    " ANSI_ESC_YELLOW "%s", playFile);
        webSrv.send("SD_playFile=", playFile);
        if(f_has_EXTINF) SD_playFile(playFile, 0, false);
        else SD_playFile(playFile, 0, true);
        if(playFile) {
            free(playFile);
            playFile = NULL;
        }
    }
    _plsCurPos++;
    showPlsFileNumber();
    return;

exit:
    SerialPrintfln("Playlist:    " ANSI_ESC_BLUE "end of playlist");
    webSrv.send("SD_playFile=", "end of playlist");
    _f_playlistEnabled = false;
    _playerSubmenue = 0;
    changeState(PLAYER);
    _plsCurPos = 0;
    if(_playlistPath) {
        free(_playlistPath);
        _playlistPath = NULL;
    }
    return;
}
/*****************************************************************************************************************************************************
 *                                         C O N N E C T   TO   W I F I     /     A C C E S S P O I N T                                              *
 *****************************************************************************************************************************************************/
bool connectToWiFi() {

    char* line = x_ps_malloc(512);
    uint16_t idx = 0;
    wifiMulti.addAP(_SSID, _PW);                        // SSID and PW in code
    if(pref.isKey("ap_ssid") && pref.isKey("ap_pw")) {  // exists?
        String ap_ssid = pref.getString("ap_ssid", ""); // credentials from accesspoint
        String ap_pw = pref.getString("ap_pw", "");
        if(ap_ssid.length() > 0 && ap_pw.length() > 0) wifiMulti.addAP(ap_ssid.c_str(), ap_pw.c_str());
    }
    File file = SD_MMC.open("/networks.csv"); // try credentials given in "/networks.txt"
    if(file) {                                // try to read from SD_MMC
        while(file.available()) {
            char emptyStr[1] = "";
            char* s_ssid = emptyStr;
            char* s_password = emptyStr;
            char* s_info = emptyStr;
            idx = file.readBytesUntil('\n', line, 512);     // read the line
            if(idx == 0) break;
            line[idx] = '\0';                               // terminate the line
            if(line[0] == '*') continue;                    // ignore this, goto next line
            if(line[0] == '\n') continue;                   // empty line
            if(line[0] == ' ') continue;                    // space as first char

            uint8_t p = 0;
            uint16_t slen = strlen(line);
            for(int16_t i = 0; i < slen; i++) {
                if(line[i] == '\n') {                       // make LF ineffective
                    line[i] = '\0';
                    continue;
                }
                if(i == 0) s_ssid = line;                   // string at pos 0 is ssid
                if(line[i] == '\t') {
                    line[i] = '\0';
                    if(p == 0) {s_password = line + i + 1;} // string after first tab is passwort (if available)
                    if(p == 1) {    s_info = line + i + 1;} // string after second tab is info (if available)
                    p++;
                }
            }
            if(strlen(s_ssid) == 0) continue;               // ssid is empty
        //    log_i("%s, %s, %s",  s_ssid, s_password, s_info);
            wifiMulti.addAP(s_ssid, s_password);
            (void) s_info; // unused
        }
        file.close();
        if(line){free(line); line = NULL;}
    }
    int16_t n = WiFi.scanNetworks();
    SerialPrintfln("setup: ....  " ANSI_ESC_WHITE "%i WiFi networks found", n);
    for(int i = 0; i < n; i++){
        SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)", WiFi.SSID(i).c_str(), (int16_t)WiFi.RSSI(i));
    }
    wifiMulti.run();
    if(WiFi.isConnected()) {
        SerialPrintfln("WiFI_info:   Connecting WiFi...");
        if(!MDNS.begin("MiniWebRadio")) { SerialPrintfln("WiFI_info:   " ANSI_ESC_YELLOW "Error starting mDNS"); }
        else {
           MDNS.addService("esp32", "tcp", 80);
           SerialPrintfln("WiFI_info:   mDNS name: " ANSI_ESC_CYAN "MiniWebRadio");
        }
        WiFi.setAutoReconnect(true);
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
    setTFTbrightness(80);
    _f_accessPoint = true;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.softAP("MiniWebRadio");
    IPAddress myIP = WiFi.softAPIP();
    String    AccesspointIP = myIP.toString();
    char      buf[100];
    sprintf(buf, "WiFi credentials are not correct \nAccesspoint IP: " ANSI_ESC_CYAN "%s", AccesspointIP.c_str());
    tft.writeText(buf, 0, 0, _dispWidth, _dispHeight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, false);
    SerialPrintfln("Accesspoint: " ANSI_ESC_RED "IP: %s", AccesspointIP.c_str());
    int16_t n = WiFi.scanNetworks();
    if(n == 0) {
        SerialPrintfln("setup: ....  no WiFi networks found");
        while(true) { ; }
    }
    else {
        SerialPrintfln("setup: ....  %d WiFi networks found", n);
        for(int32_t i = 0; i < n; ++i) {
            SerialPrintfln("setup: ....  " ANSI_ESC_GREEN "%s (%d)", WiFi.SSID(i).c_str(), (int16_t)WiFi.RSSI(i));
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
    int32_t idx1, idx2;
    char*   url = nullptr;
    char*   user = nullptr;
    char*   pwd = nullptr;

    dispFooter.updateBitRate(0);
    _cur_Codec = 0;
    //    if(_state == RADIO) clearStreamTitle();
    _icyBitRate = 0;
    _decoderBitRate = 0;

    idx1 = indexOf(host, "|", 0);
    if(idx1 == -1) { // no pipe found
        _f_isWebConnected = audioConnecttohost(host);
        if(_f_isWebConnected) _reconnectCnt = 0;
        _f_isFSConnected = false;
        return;
    }
    else { // pipe found
        idx2 = indexOf(host, "|", idx1 + 1);
        // log_i("idx2 = %i", idx2);
        if(idx2 == -1) { // second pipe not found
            _f_isWebConnected = audioConnecttohost(host);
            if(_f_isWebConnected) _reconnectCnt = 0;
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
    dispFooter.updateBitRate(0);
    _icyBitRate = 0;
    _decoderBitRate = 0;
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
    _f_playlistNextFile = false;
    _f_shuffle = false;
}

/*****************************************************************************************************************************************************
 *                                                                    S E T U P                                                                      *
 *****************************************************************************************************************************************************/
void setup() {
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
    Version = Version.substring(0, 30);
    Serial.printf("MiniWebRadio %s\n", Version.c_str());
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
    _resetResaon = esp_reset_reason();
    switch(_resetResaon) {
        case ESP_RST_UNKNOWN: rr = "Reset reason can not be determined"; break;
        case ESP_RST_POWERON: rr = "Reset due to power-on event"; break;
        case ESP_RST_EXT: rr = "Reset by external pin (not applicable for ESP32)"; break;
        case ESP_RST_SW: rr = "Software reset via esp_restart"; break;
        case ESP_RST_PANIC: rr = "Software reset due to exception/panic"; break;
        case ESP_RST_INT_WDT: rr = "Reset (software or hardware) due to interrupt watchdog"; break;
        case ESP_RST_TASK_WDT: rr = "Reset due to task watchdog"; break;
        case ESP_RST_WDT:
            rr = "Reset due to other watchdogs";
            _resetResaon = 1;
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
    SerialPrintfln(ANSI_ESC_YELLOW "       *     MiniWebRadio V3     *    ");
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

    _curAudioFolder = x_ps_malloc(1024);
    strcpy(_curAudioFolder, "/audiofiles");

    if(TFT_CONTROLLER < 2) strcpy(_prefix, "/s");
    else                   strcpy(_prefix, "/m");
    stations.begin("Stations", false); // instance of preferences for stations (name, url ...)
    pref.begin("Pref", false);         // instance of preferences from AccessPoint (SSID, PW ...)

#if CONFIG_IDF_TARGET_ESP32
    tft.begin(TFT_CS, TFT_DC, VSPI, TFT_MOSI, TFT_MISO, TFT_SCK); // Init TFT interface ESP32
#else
    tft.begin(TFT_CS, TFT_DC, FSPI, TFT_MOSI, TFT_MISO, TFT_SCK); // Init TFT interface ESP32S3
#endif

    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tft.setBackGoundColor(TFT_BLACK);
    tp.setVersion(TP_VERSION);
    tp.setRotation(TP_ROTATION);
    tp.setMirror(TP_H_MIRROR, TP_V_MIRROR);
    tp.TP_Send(0xD0);
    tp.TP_Send(0x90); // Remove any blockage

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
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed");
        return;
    }
    float cardSize = ((float)SD_MMC.cardSize()) / (1024 * 1024);
    float freeSize = ((float)SD_MMC.cardSize() - SD_MMC.usedBytes()) / (1024 * 1024);
    SerialPrintfln(ANSI_ESC_WHITE "setup: ....  SD card found, %.1f MB by %.1f MB free", freeSize, cardSize);
    _f_SD_MMCfound = true;
    if(ESP.getFlashChipSize() > 80000000) { FFat.begin(); }
    defaultsettings();
    if(TFT_BL >= 0){_f_brightnessIsChangeable = true;}
#if ESP_IDF_VERSION_MAJOR == 5
    if(TFT_BL >= 0) ledcAttach(TFT_BL, 1200, 8); // 1200 Hz PWM and 8 bit resolution
#endif
    if(_brightness < 5) _brightness = 5;
    setTFTbrightness(80);
    if(TFT_CONTROLLER > 6) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid");
    drawImage("/common/MiniWebRadioV3.jpg", 0, 0); // Welcomescreen
    SerialPrintfln("setup: ....  seek for stations.csv");
    File file = SD_MMC.open("/stations.csv");
    if(!file) {
        clearAll();
        tft.setFont(_fonts[6]);
        tft.setTextColor(TFT_YELLOW);
        tft.writeText("stations.csv not found", 0, 50, _dispWidth, _dispHeight, TFT_ALIGN_CENTER, TFT_ALIGN_TOP, false, false);
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "stations.csv not found");
        while(1) {}; // endless loop, MiniWebRadio does not work without stations.csv
    }
    file.close();
    SerialPrintfln("setup: ....  stations.csv found");
    updateSettings();
    SerialPrintfln("setup: ....  seek for WiFi networks");
    while(true){
        if(!connectToWiFi()){
            _reconnectCnt++;
            SerialPrintfln("RECONNECTION " ANSI_ESC_RED "try %i", _reconnectCnt);
            if(_reconnectCnt == 3){
                openAccessPoint();
                return;
            }
        }
        else{
            break;
        }
    }
    _reconnectCnt = 0;

    strcpy(_myIP, WiFi.localIP().toString().c_str());
    SerialPrintfln("setup: ....  connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE ", IP address is " ANSI_ESC_CYAN "%s", WiFi.SSID().c_str(), _myIP);
    ArduinoOTA.setHostname("MiniWebRadio");
    ArduinoOTA.begin();

    ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); // username, password for ftp.

    setRTC(_TZString.c_str());

#if DECODER > 1 // DAC controlled by I2C
    if(!dac.begin(I2C_DAC_SDA, I2C_DAC_SCL, 400000)) { SerialPrintfln(ANSI_ESC_RED "The DAC was not be initialized"); }
#endif

    placingGraphicObjects();
    // audio.setAudioTaskCore(AUDIOTASK_CORE);
    // audio.setConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);
    // audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK);
    // audio.setVolumeSteps(_volumeSteps);

    audioInit();
    audioSetCoreID(0);
    audioConnectionTimeout(CONN_TIMEOUT, CONN_TIMEOUT_SSL);
    audioSetVolumeSteps(_volumeSteps);


    SerialPrintfln("setup: ....  Number of saved stations: " ANSI_ESC_CYAN "%d", _sum_stations);
    SerialPrintfln("setup: ....  current station number: " ANSI_ESC_CYAN "%d", _cur_station);
    SerialPrintfln("setup: ....  current volume: " ANSI_ESC_CYAN "%d", _cur_volume);
    SerialPrintfln("setup: ....  volume steps: " ANSI_ESC_CYAN "%d", _volumeSteps);
    SerialPrintfln("setup: ....  last connected host: " ANSI_ESC_CYAN "%s", _lastconnectedhost.c_str());
    SerialPrintfln("setup: ....  connection timeout: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT);
    SerialPrintfln("setup: ....  connection timeout SSL: " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " ms", CONN_TIMEOUT_SSL);

    ir.begin(); // Init InfraredDecoder

    webSrv.begin(80, 81); // HTTP port, WebSocket port

    if(HP_DETECT != -1) {
        pinMode(HP_DETECT, INPUT);
        attachInterrupt(HP_DETECT, headphoneDetect, CHANGE);
    }
    if(AMP_ENABLED != -1) { // enable onboard amplifier
        pinMode(AMP_ENABLED, OUTPUT);
        digitalWrite(AMP_ENABLED, HIGH);
    }

    ticker100ms.attach(0.1, timer100ms);
    if(BT_EMITTER_CONNECT != -1){
        pinMode(BT_EMITTER_CONNECT, OUTPUT);
        digitalWrite(BT_EMITTER_CONNECT, LOW); vTaskDelay(100); digitalWrite(BT_EMITTER_CONNECT, HIGH); // POWER_ON
        _f_BTcurPowerState = true;
    }
    bt_emitter.begin();

    _dlnaLevel = 0;
    _dlnaHistory[0].name = strdup("Media Server");
    _dlnaHistory[0].objId = strdup("");
    _dlnaHistory[1].objId = strdup("0");
    _f_dlnaSeekServer = true;


    tft.fillScreen(TFT_BLACK); // Clear screen
    muteChanged(_f_mute);

    dispFooter.setIpAddr(WiFi.localIP().toString().c_str());
    dispFooter.updateStation(_cur_station);
    dispFooter.updateOffTime(_sleeptime);
    dispFooter.show();

    dispHeader.updateItem(_hl_item[RADIO]);
    dispHeader.updateVolume(_cur_volume);
    dispHeader.show();

    _radioSubmenue = 0;
    _state = NONE;
    changeState(RADIO);

    if( _resetResaon == ESP_RST_POWERON ||   // Simply switch on the operating voltage
        _resetResaon == ESP_RST_SW ||        // ESP.restart()
        _resetResaon == ESP_RST_SDIO ||      // The boot button was pressed
        _resetResaon == ESP_RST_DEEPSLEEP) { // Wake up
        if(_cur_station > 0) setStation(_cur_station);
        else { setStationViaURL(_lastconnectedhost.c_str()); }
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
}
/*****************************************************************************************************************************************************
 *                                                                   C O M M O N                                                                     *
 *****************************************************************************************************************************************************/

const char* scaleImage(const char* path) {
    if((!endsWith(path, "bmp")) && (!endsWith(path, "jpg")) && (!endsWith(path, "gif"))) { // not a image
        return path;
    }
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    char* pch = strstr(path + 1, "/");
    if(pch) {
        strncpy(pathBuff, path, (pch - path));
        strcat(pathBuff, _prefix);
        strcat(pathBuff, pch);
    }
    else {
        return path; // invalid path
    }
    return pathBuff;
}

void setVolume(uint8_t vol) {
    static int16_t oldVol = -1;
    log_i("vol %i   oldVol %i", vol, oldVol);
    if(vol == oldVol) return;
    _cur_volume = vol;
    oldVol = vol;
    dispHeader.updateVolume(_cur_volume);
    sdr_CL_volume.setValue(_cur_volume);
    sdr_DL_volume.setValue(_cur_volume);
    sdr_PL_volume.setValue(_cur_volume);
    sdr_RA_volume.setValue(_cur_volume);
    SerialPrintfln("action: ...  current volume is " ANSI_ESC_CYAN "%d", _cur_volume);

#if DECODER > 1 // ES8388, AC101 ...
    if(HP_DETECT == -1) {
        if(_f_mute){
            dac.SetVolumeSpeaker(0);
            dac.SetVolumeHeadphone(0);
        }
        else{
            dac.SetVolumeSpeaker(_cur_volume * 3);
            dac.SetVolumeHeadphone(_cur_volume * 3);
        }
    }
    else {
        if(_f_mute){
            dac.SetVolumeSpeaker(0);
            dac.SetVolumeHeadphone(0);
        }
        else{
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
    }
#endif
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
    // SerialPrintfln("sta %d, _cur_station %d", sta, _cur_station );
    if(sta == 0) return;
    if(sta > _sum_stations) sta = _cur_station;
    sprintf(_chbuf, "station_%03d", sta);
    String content = stations.getString(_chbuf, " #not_found");
    //    SerialPrintfln("content %s", content.c_str());
    _stationName_nvs = content.substring(0, content.indexOf("#"));           // get stationname
    content = content.substring(content.indexOf("#") + 1, content.length()); // get URL
    content.trim();
    free(_stationURL);
    _stationURL = x_ps_strdup(content.c_str());
    _homepage = "";
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
    if(_state == RADIO) showLogoAndStationName(true);
    dispFooter.updateStation(_cur_station);
}
void nextStation() {
    if(_cur_station >= _sum_stations) setStation(1);
    else setStation(_cur_station + 1);
}
void prevStation() {
    if(_cur_station > 1) setStation(_cur_station - 1);
    else setStation(_sum_stations);
}

void StationsItems() {
    if(!_stationName_air)  _stationName_air = (char*)calloc(1, 1);
    char* stationLogo_air = x_ps_malloc(strlen(_stationName_air) + 15);
    strcpy(stationLogo_air, "/logo/");
    strcat(stationLogo_air, _stationName_air);
    strcat(stationLogo_air, ".jpg");
    char cur_station[10]; itoa(_cur_station, cur_station, 10);

    if(_cur_station == 0){
        webSrv.send("stationLogo=", stationLogo_air);
        webSrv.send("stationNr=", cur_station);
        //    webSrv.send("", "stationURL=" + _lastconnectedhost);
    }
    else{
        webSrv.send("stationLogo=", "/logo/" + _stationName_nvs + ".jpg");
        webSrv.send("stationNr=", cur_station);
        if(_stationURL) webSrv.send("stationURL=", String(_stationURL));
    }
    if(stationLogo_air){free(stationLogo_air); stationLogo_air = NULL;}
}

void setStationViaURL(const char* url) {
    if(_stationName_air) {free(_stationName_air); _stationName_air = NULL;}
    _stationName_nvs = "";
    _cur_station = 0;
    free(_stationURL);
    _stationURL = x_ps_strdup(url);
    connecttohost(url);
    StationsItems();
    if(_state == RADIO) {
        clearStreamTitle();
        showLogoAndStationName(true);
    }
    dispFooter.updateStation(0); // set 000
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
        else webSrv.sendStatus(400);
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

String setI2STone() {
    int8_t LP = _toneLP;
    int8_t BP = _toneBP;
    int8_t HP = _toneHP;
    int8_t BAL = _toneBAL;
    audioSetTone(LP, BP, HP, BAL);
    sprintf(_chbuf, "LowPass=%i\nBandPass=%i\nHighPass=%i\nBalance=%i\n", LP, BP, HP, BAL);
    String tone = String(_chbuf);
    return tone;
}

void SD_playFile(const char* path, const char* fileName) {
    sprintf(_chbuf, "%s/%s", path, fileName);
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
    _playerSubmenue = 1;
    changeState(PLAYER);
    int32_t idx = lastIndexOf(path, '/');
    if(idx < 0) return;
    if(showFN) {
        clearLogo();
        showFileName(path + idx + 1);
    }

    SerialPrintfln("AUDIO_FILE:  " ANSI_ESC_MAGENTA "%s", path + idx + 1);
    connecttoFS((const char*)path, resumeFilePos);
    if(_f_playlistEnabled) showPlsFileNumber();
    if(_f_isFSConnected) {
        free(_lastconnectedfile);
        _lastconnectedfile = x_ps_strdup(path);
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
        else success = SD_MMC.remove(itemPath);
    }
    return success;
}

void headphoneDetect() { // called via interrupt
    _f_hpChanged = true;
}

void fall_asleep() {
    _f_sleeping = true;
    _f_playlistEnabled = false;
    _f_isFSConnected = false;
    _f_isWebConnected = false;
    audioStopSong();
    if(AMP_ENABLED != -1) {digitalWrite(AMP_ENABLED, LOW);}
    if(_sleepMode == 0){
        clearAll();
        setTFTbrightness(0);
    }
    else{
        changeState(CLOCK);
    }
    if(_f_BTpower) BTpowerChanged(false);
    SerialPrintfln("falling asleep");
    dispHeader.hide();
    dispFooter.hide();
}

void wake_up() {
    _f_sleeping = false;
    SerialPrintfln("awake");
    _f_mute = true;
    muteChanged(false);
    clearAll();
    setTFTbrightness(_brightness);
    if(AMP_ENABLED != -1) {digitalWrite(AMP_ENABLED, HIGH);}
    connecttohost(_lastconnectedhost.c_str());
    _radioSubmenue = 0;
    changeState(RADIO);
    showLogoAndStationName(true);
    dispHeader.show();
    dispFooter.show();
    if(_f_BTpower) BTpowerChanged(true);
}

void setRTC(const char* TZString) {
    rtc.stop();
    _f_rtc = rtc.begin(_TZString.c_str());
    if(!_f_rtc) {
        SerialPrintfln(ANSI_ESC_RED "connection to NTP failed, trying again");
        ESP.restart();
    }
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
    btn_CL_Mute.setValue(m);
    btn_DL_Mute.setValue(m);
    btn_EQ_Mute.setValue(m);
    btn_PL_Mute.setValue(m);
    btn_RA_Mute.setValue(m);
    if(m) {_f_mute = true;}
    else  {_f_mute = false;}
    if(m) webSrv.send("mute=", "1");
    else webSrv.send("mute=", "0");
    if(_f_mute) dispHeader.setVolumeColor(TFT_RED);
    else        dispHeader.setVolumeColor(TFT_DEEPSKYBLUE);
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
    dispHeader.begin(         _winHeader.x, _winHeader.y, _winHeader.w, _winHeader.h);
    dispFooter.begin(         _winFooter.x, _winFooter.y, _winFooter.w, _winFooter.h);
    // RADIO -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_RA_volume.begin(      _sdrOvBtns.x,  _sdrOvBtns.y, _sdrOvBtns.w, _sdrOvBtns.h, 0, _volumeSteps); sdr_RA_volume.setValue(_cur_volume);
    btn_RA_Mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_Mute.setOffPicturePath("/btn/Button_Mute_Green.jpg");
                                                                                         btn_RA_Mute.setOnPicturePath("/btn/Button_Mute_Red.jpg");
                                                                                         btn_RA_Mute.setClickedOffPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_RA_Mute.setClickedOnPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_RA_Mute.setValue(_f_mute);
    btn_RA_prevSta.begin( 1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_prevSta.setDefaultPicturePath("/btn/Button_Previous_Green.jpg");
                                                                                         btn_RA_prevSta.setClickedPicturePath("/btn/Button_Previous_Yellow.jpg");
    btn_RA_nextSta.begin( 2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_nextSta.setDefaultPicturePath("/btn/Button_Next_Green.jpg");
                                                                                         btn_RA_nextSta.setClickedPicturePath("/btn/Button_Next_Yellow.jpg");
    btn_RA_staList.begin( 3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_staList.setDefaultPicturePath("/btn/Button_List_Green.jpg");
                                                                                         btn_RA_staList.setClickedPicturePath("/btn/Button_List_Yellow.jpg");
    btn_RA_player.begin(  0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_player.setDefaultPicturePath("/btn/Player_Green.jpg");
                                                                                         btn_RA_player.setClickedPicturePath("/btn/Player_Yellow.jpg");
    btn_RA_dlna.begin(    1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_dlna.setDefaultPicturePath("/btn/Button_DLNA_Green.jpg");
                                                                                         btn_RA_dlna.setClickedPicturePath("/btn/Button_DLNA_Yellow.jpg");
    btn_RA_clock.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_clock.setDefaultPicturePath("/btn/Clock_Green.jpg");
                                                                                         btn_RA_clock.setClickedPicturePath("/btn/Clock_Yellow.jpg");
    btn_RA_sleep.begin(   3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_sleep.setDefaultPicturePath("/btn/Button_Sleep_Green.jpg");
                                                                                         btn_RA_sleep.setClickedPicturePath("/btn/Button_Sleep_Yellow.jpg");
    btn_RA_bright.begin(  4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_bright.setDefaultPicturePath("/btn/Bulb_Green.jpg");
                                                                                         btn_RA_bright.setClickedPicturePath("/btn/Bulb_Yellow.jpg");
                                                                                         btn_RA_bright.setInactivePicturePath("/btn/Bulb_Grey.jpg");
    btn_RA_equal.begin(   5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_equal.setDefaultPicturePath("/btn/Button_EQ_Green.jpg");
                                                                                         btn_RA_equal.setClickedPicturePath("/btn/Button_EQ_Yellow.jpg");
    btn_RA_bt.begin(      6 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_bt.setDefaultPicturePath("/btn/BT_Green.jpg");
                                                                                         btn_RA_bt.setClickedPicturePath("/btn/BT_Yellow.jpg");
                                                                                         btn_RA_bt.setInactivePicturePath("/btn/BT_Grey.jpg");
    btn_RA_off.begin(     7 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_RA_off.setDefaultPicturePath("/btn/Button_Off_Red.jpg");
                                                                                         btn_RA_off.setClickedPicturePath("/btn/Button_Off_Yellow.jpg");
    txt_RA_sTitle.begin(      _winSTitle.x, _winSTitle.y, _winSTitle.w, _winSTitle.h);   txt_RA_sTitle.setFont(0); // 0 -> auto
    txt_RA_staName.begin(       _winName.x,   _winName.y,   _winName.w,   _winName.h);   txt_RA_staName.setFont(0); // 0 -> auto
    txt_RA_irNum.begin(         _winWoHF.x,   _winWoHF.y,   _winWoHF.w,   _winWoHF.h);   txt_RA_irNum.setTextColor(TFT_GOLD); txt_RA_irNum.setFont(_fonts[8]);
    pic_RA_logo.begin(          _winLogo.x,   _winLogo.y);
    VUmeter_RA.begin(        _winVUmeter.x,_winVUmeter.y,_winVUmeter.w,_winVUmeter.h);
    // STATIONSLIST ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_RADIO.begin(          _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _fonts[0], &_cur_station, _sum_stations);
    // PLAYER-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_PL_Mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_Mute.setOffPicturePath("/btn/Button_Mute_Green.jpg");
                                                                                         btn_PL_Mute.setOnPicturePath("/btn/Button_Mute_Red.jpg");
                                                                                         btn_PL_Mute.setClickedOffPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_PL_Mute.setClickedOnPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_PL_Mute.setValue(_f_mute);
    btn_PL_pause.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_pause.setOffPicturePath("/btn/Button_Pause_Blue.jpg");
                                                                                         btn_PL_pause.setOnPicturePath("/btn/Button_Right_Blue.jpg");
                                                                                         btn_PL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.jpg");
                                                                                         btn_PL_pause.setClickedOnPicturePath("/btn/Button_Right_Yellow.jpg");
                                                                                         btn_PL_pause.setValue(false);
    btn_PL_cancel.begin(  2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.jpg");
                                                                                         btn_PL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.jpg");
    sdr_PL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _winButton.w * 3 - 10, _winButton.h, 0, _volumeSteps); sdr_PL_volume.setValue(_cur_volume);
    btn_PL_prevFile.begin(0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_prevFile.setDefaultPicturePath("/btn/Button_Left_Blue.jpg");
                                                                                         btn_PL_prevFile.setClickedPicturePath("/btn/Button_Left_Yellow.jpg");
    btn_PL_nextFile.begin(1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_nextFile.setDefaultPicturePath("/btn/Button_Right_Blue.jpg");
                                                                                         btn_PL_nextFile.setClickedPicturePath("/btn/Button_Right_Yellow.jpg");
    btn_PL_ready.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.jpg");
                                                                                         btn_PL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.jpg");
    btn_PL_playAll.begin( 3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_playAll.setDefaultPicturePath("/btn/Button_PlayAll_Blue.jpg");
                                                                                         btn_PL_playAll.setClickedPicturePath("/btn/Button_PlayAll_Yellow.jpg");
    btn_PL_shuffle.begin( 4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_shuffle.setDefaultPicturePath("/btn/Button_Shuffle_Blue.jpg");
                                                                                         btn_PL_shuffle.setClickedPicturePath("/btn/Button_Shuffle_Yellow.jpg");
    btn_PL_fileList.begin(5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_fileList.setDefaultPicturePath("/btn/Button_List_Green.jpg");
                                                                                         btn_PL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.jpg");
    btn_PL_radio.begin(   6 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_radio.setDefaultPicturePath("/btn/Radio_Green.jpg");
                                                                                         btn_PL_radio.setClickedPicturePath("/btn/Radio_Yellow.jpg");
    btn_PL_off.begin(     7 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_PL_off.setDefaultPicturePath("/btn/Button_Off_Red.jpg");
                                                                                         btn_PL_off.setClickedPicturePath("/btn/Button_Off_Yellow.jpg");
    txt_PL_fName.begin(         _winName.x,   _winName.y,   _winName.w,   _winName.h);   txt_PL_fName.setFont(0); // 0 -> auto
    // AUDIOFILESLIST-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_PLAYER.begin(         _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _fonts[0], _curAudioFolder, &_cur_AudioFileNr);
    // DLNA --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_DL_Mute.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_Mute.setOffPicturePath("/btn/Button_Mute_Green.jpg");
                                                                                         btn_DL_Mute.setOnPicturePath("/btn/Button_Mute_Red.jpg");
                                                                                         btn_DL_Mute.setClickedOffPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_DL_Mute.setClickedOnPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_DL_Mute.setValue(_f_mute);
    btn_DL_pause.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_pause.setOffPicturePath("/btn/Button_Pause_Blue.jpg");
                                                                                         btn_DL_pause.setOnPicturePath("/btn/Button_Right_Blue.jpg");
                                                                                         btn_DL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.jpg");
                                                                                         btn_DL_pause.setClickedOnPicturePath("/btn/Button_Right_Yellow.jpg");
                                                                                         btn_DL_pause.setInactivePicturePath("/btn/Button_Pause_Grey.jpg");
                                                                                         btn_DL_pause.setValue(false);
    btn_DL_cancel.begin(  2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.jpg");
                                                                                         btn_DL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.jpg");
    btn_DL_fileList.begin(3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_fileList.setDefaultPicturePath("/btn/Button_List_Green.jpg");
                                                                                         btn_DL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.jpg");
    btn_DL_radio.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_DL_radio.setDefaultPicturePath("/btn/Radio_Green.jpg");
                                                                                         btn_DL_radio.setClickedPicturePath("/btn/Radio_Yellow.jpg");
    sdr_DL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _winButton.w * 3 - 10, _winButton.h, 0, _volumeSteps); sdr_DL_volume.setValue(_cur_volume);
    txt_DL_fName.begin(         _winName.x,   _winName.y,   _winName.w,   _winName.h);   txt_DL_fName.setFont(0); // 0 -> auto)
    // DLNAITEMSLIST -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_DLNA.begin(           _winWoHF.x, _winWoHF.y, _winWoHF.w, _winWoHF.h, _fonts[0]);
    // CLOCK -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_CL_green.begin(       _winDigits.x, _winDigits.y, _winDigits.w, _winDigits.h);   clk_CL_green.setTimeFormat(_timeFormat);
    btn_CL_alarm.begin(   0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_alarm.setDefaultPicturePath("/btn/Bell_Green.jpg");
                                                                                         btn_CL_alarm.setClickedPicturePath("/btn/Bell_Yellow.jpg");
    btn_CL_radio.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_radio.setDefaultPicturePath("/btn/Radio_Green.jpg");
                                                                                         btn_CL_radio.setClickedPicturePath("/btn/Radio_Yellow.jpg");
    btn_CL_Mute.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_Mute.setOffPicturePath("/btn/Button_Mute_Green.jpg");
                                                                                         btn_CL_Mute.setOnPicturePath("/btn/Button_Mute_Red.jpg");
                                                                                         btn_CL_Mute.setClickedOffPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_CL_Mute.setClickedOnPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_CL_Mute.setValue(_f_mute);
    btn_CL_off.begin(     3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_CL_off.setDefaultPicturePath("/btn/Button_Off_Red.jpg");
                                                                                         btn_CL_off.setClickedPicturePath("/btn/Button_Off_Yellow.jpg");
    sdr_CL_volume.begin(  5 * _winButton.w + 10, _winButton.y, _winButton.w * 3 - 10, _winButton.h, 0, _volumeSteps); sdr_CL_volume.setValue(_cur_volume);
    // ALARM -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_AL_red.begin(          _winAlarm.x, _winAlarm.y, _winAlarm.w, _winAlarm.h);      clk_AL_red.setAlarmTimeAndDays(&_alarmdays, _alarmtime);
    btn_AL_left.begin(    0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AL_left.setDefaultPicturePath("/btn/Button_Left_Blue.jpg");
                                                                                         btn_AL_left.setClickedPicturePath("/btn/Button_Left_Yellow.jpg");
    btn_AL_right.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AL_right.setDefaultPicturePath("/btn/Button_Right_Blue.jpg");
                                                                                         btn_AL_right.setClickedPicturePath("/btn/Button_Right_Yellow.jpg");
    btn_AL_up.begin(      2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AL_up.setDefaultPicturePath("/btn/Button_Up_Blue.jpg");
                                                                                         btn_AL_up.setClickedPicturePath("/btn/Button_Up_Yellow.jpg");
    btn_AL_down.begin(    3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AL_down.setDefaultPicturePath("/btn/Button_Down_Blue.jpg");
                                                                                         btn_AL_down.setClickedPicturePath("/btn/Button_Down_Yellow.jpg");
    btn_AL_ready.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_AL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.jpg");
                                                                                         btn_AL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.jpg");
    // SLEEP -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SL_up.begin(      0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_up.setDefaultPicturePath("/btn/Button_Up_Blue.jpg");
                                                                                         btn_SL_up.setClickedPicturePath("/btn/Button_Up_Yellow.jpg");
    btn_SL_down.begin(    1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_down.setDefaultPicturePath("/btn/Button_Down_Blue.jpg");
                                                                                         btn_SL_down.setClickedPicturePath("/btn/Button_Down_Yellow.jpg");
    btn_SL_ready.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.jpg");
                                                                                         btn_SL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.jpg");
    btn_SL_cancel.begin(  4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_SL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Blue.jpg");
                                                                                         btn_SL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.jpg");
    // BRIGHTNESS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_BR_value.begin(      _sdrOvBtns.x,  _sdrOvBtns.y, _sdrOvBtns.w, _sdrOvBtns.h, 5, 100); sdr_BR_value.setValue(_brightness);
    btn_BR_ready.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BR_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.jpg");
                                                                                         btn_BR_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.jpg");
    pic_BR_logo.begin(    0,  _winName.y) ;                                              pic_BR_logo.setPicturePath("/common/Brightness.jpg");
    txt_BR_value.begin(   0, _winButton.y, _winButton.w * 2, _winButton.h);              txt_BR_value.setFont(_fonts[4]);
    // EQUALIZER ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_EQ_lowPass.begin(  _sdrLP.x,  _sdrLP.y,  _sdrLP.w,  _sdrLP.h, -40,  6);          sdr_EQ_lowPass.setValue(_toneLP);
    sdr_EQ_bandPass.begin( _sdrBP.x,  _sdrBP.y,  _sdrBP.w,  _sdrBP.h, -40,  6);          sdr_EQ_bandPass.setValue(_toneBP);
    sdr_EQ_highPass.begin( _sdrHP.x,  _sdrHP.y,  _sdrHP.w,  _sdrHP.h, -40,  6);          sdr_EQ_highPass.setValue(_toneHP);
    sdr_EQ_balance.begin( _sdrBAL.x, _sdrBAL.y, _sdrBAL.w, _sdrBAL.h, -16, 16);          sdr_EQ_balance.setValue(_toneBAL);
    txt_EQ_lowPass.begin(  _sdrLP.x +  _sdrLP.w + 2,  _sdrLP.y, 80, _sdrLP.h);           txt_EQ_lowPass.setFont(_fonts[2]);
    txt_EQ_bandPass.begin( _sdrBP.x +  _sdrBP.w + 2,  _sdrBP.y, 80, _sdrBP.h);           txt_EQ_bandPass.setFont(_fonts[2]);
    txt_EQ_highPass.begin( _sdrHP.x +  _sdrHP.w + 2,  _sdrHP.y, 80, _sdrHP.h);           txt_EQ_highPass.setFont(_fonts[2]);
    txt_EQ_balance.begin( _sdrBAL.x + _sdrBAL.w + 2, _sdrBAL.y, 80, _sdrBAL.h);          txt_EQ_balance.setFont(_fonts[2]);
    btn_EQ_lowPass.begin( _sdrLP.x - 60, _sdrLP.y + 1, 48, 48);                          btn_EQ_lowPass.setDefaultPicturePath("/btn/LP_green.jpg");
                                                                                         btn_EQ_lowPass.setClickedPicturePath("/btn/LP_yellow.jpg");
    btn_EQ_bandPass.begin(_sdrBP.x - 60, _sdrBP.y + 1, 48, 48);                          btn_EQ_bandPass.setDefaultPicturePath("/btn/BP_green.jpg");
                                                                                         btn_EQ_bandPass.setClickedPicturePath("/btn/BP_yellow.jpg");
    btn_EQ_highPass.begin(_sdrHP.x - 60, _sdrHP.y + 1, 48, 48);                          btn_EQ_highPass.setDefaultPicturePath("/btn/HP_green.jpg");
                                                                                         btn_EQ_highPass.setClickedPicturePath("/btn/HP_yellow.jpg");
    btn_EQ_balance.begin(_sdrBAL.x - 60, _sdrBAL.y + 1, 48, 48);                         btn_EQ_balance.setDefaultPicturePath("/btn/BAL_green.jpg");
                                                                                         btn_EQ_balance.setClickedPicturePath("/btn/BAL_yellow.jpg");
    btn_EQ_Radio.begin(   0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_Radio.setDefaultPicturePath("/btn/Radio_Green.jpg");
                                                                                         btn_EQ_Radio.setClickedPicturePath("/btn/Radio_Yellow.jpg");
    btn_EQ_Player.begin(  1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_Player.setDefaultPicturePath("/btn/Player_Green.jpg");
                                                                                         btn_EQ_Player.setClickedPicturePath("/btn/Player_Yellow.jpg");
    btn_EQ_Mute.begin(    2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_EQ_Mute.setOffPicturePath("/btn/Button_Mute_Green.jpg");
                                                                                         btn_EQ_Mute.setOnPicturePath("/btn/Button_Mute_Red.jpg");
                                                                                         btn_EQ_Mute.setClickedOffPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_EQ_Mute.setClickedOnPicturePath("/btn/Button_Mute_Yellow.jpg");
                                                                                         btn_EQ_Mute.setValue(_f_mute);
    // BLUETOOTH ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_BT_volDown.begin( 0 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_volDown.setDefaultPicturePath("/btn/Button_Volume_Down_Blue.jpg");
                                                                                         btn_BT_volDown.setClickedPicturePath("/btn/Button_Volume_Down_Yellow.jpg");
    btn_BT_volUp.begin(   1 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_volUp.setDefaultPicturePath("/btn/Button_Volume_Up_Blue.jpg");
                                                                                         btn_BT_volUp.setClickedPicturePath("/btn/Button_Volume_Up_Yellow.jpg");
    btn_BT_pause.begin(   2 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_pause.setOffPicturePath("/btn/Button_Pause_Blue.jpg");
                                                                                         btn_BT_pause.setOnPicturePath("/btn/Button_Right_Blue.jpg");
                                                                                         btn_BT_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.jpg");
                                                                                         btn_BT_pause.setClickedOnPicturePath("/btn/Button_Right_Yellow.jpg");
    btn_BT_mode.begin(    3 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_mode.setDefaultPicturePath("/btn/Button_RxTx_Blue.jpg");
                                                                                         btn_BT_mode.setClickedPicturePath("/btn/Button_RxTx_Yellow.jpg");
    btn_BT_radio.begin(   4 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_radio.setDefaultPicturePath("/btn/Radio_Green.jpg");
                                                                                         btn_BT_radio.setClickedPicturePath("/btn/Radio_Yellow.jpg");
    btn_BT_power.begin(   5 * _winButton.w, _winButton.y, _winButton.w, _winButton.h);   btn_BT_power.setOffPicturePath("/btn/BT_Red.jpg");
                                                                                         btn_BT_power.setOnPicturePath("/btn/BT_Blue.jpg");
                                                                                         btn_BT_power.setClickedOffPicturePath("/btn/BT_Yellow.jpg");
                                                                                         btn_BT_power.setClickedOnPicturePath("/btn/BT_Yellow.jpg");
                                                                                         btn_BT_power.setValue(_f_BTpower);
                                                                                         pic_BT_mode.setPicturePath("/common/BTnc.jpg");
    pic_BT_mode.begin(        _winLogo.x,   _winLogo.y);                                 pic_BT_mode.setAlternativPicturePath("/common/BTnc.jpg");
    txt_BT_volume.begin(      _winFileNr.x, _winFileNr.y, _winFileNr.w, _winFileNr.h);   txt_BT_volume.setFont(_fonts[2]);
    txt_BT_mode.begin(        _winName.x,   _winName.y,   _winName.w,   _winName.h);     txt_BT_mode.setFont(_fonts[5]);
}
// clang-format off

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                              C H A N G E    S T A T E                                                                       ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// clang-format on
void changeState(int32_t state){
//    log_i("new state %i, current state %i radioSubMenue %i", state, _state, _radioSubmenue);
    switch(_state){
        case RADIO:      sdr_RA_volume.disable();
                         btn_RA_Mute.disable();     btn_RA_prevSta.disable();  btn_RA_nextSta.disable();
                         btn_RA_staList.disable();  btn_RA_player.disable();   btn_RA_dlna.disable();     btn_RA_clock.disable();   btn_RA_sleep.disable();
                         btn_RA_bright.disable();   btn_RA_equal.disable();    pic_RA_logo.disable();     btn_RA_bt.disable();      btn_RA_off.disable();
                         txt_RA_sTitle.disable();   txt_RA_staName.disable();  txt_RA_irNum.disable();    VUmeter_RA.disable(); break;
        case STATIONSLIST:
                         lst_RADIO.disable();
                         break;
        case PLAYER:     btn_PL_Mute.disable();     btn_PL_pause.disable();   btn_PL_cancel.disable();    btn_PL_off.disable();
                         btn_PL_prevFile.disable(); btn_PL_nextFile.disable(); btn_PL_ready.disable();    btn_PL_playAll.disable(); btn_PL_shuffle.disable();
                         btn_PL_fileList.hide();    btn_PL_radio.hide();       txt_PL_fName.disable();
                         sdr_PL_volume.hide();
                         break;
        case AUDIOFILESLIST: lst_PLAYER.disable();
                         break;
        case DLNA:       btn_DL_Mute.disable();     btn_DL_pause.disable();   btn_DL_cancel.disable();
                         btn_DL_fileList.disable(); btn_DL_radio.disable();    txt_DL_fName.disable();
                         sdr_DL_volume.hide();
                         break;
        case DLNAITEMSLIST: lst_DLNA.disable();
                         break;
        case CLOCK:      btn_CL_Mute.disable();     btn_CL_alarm.disable();    btn_CL_radio.disable();
                         clk_CL_green.disable();    sdr_CL_volume.hide();      btn_CL_off.disable();
                         break;
        case ALARM:      clk_AL_red.disable();      btn_AL_left.disable();     btn_AL_right.disable();    btn_AL_up.disable();      btn_AL_down.disable();
                         btn_AL_ready.disable();
                         break;
        case SLEEP:      btn_SL_up.disable();       btn_SL_down.disable();     btn_SL_ready.disable();    btn_SL_cancel.disable();
                         break;
        case BRIGHTNESS: sdr_BR_value.disable();    btn_BR_ready.disable();    pic_BR_logo.disable();     txt_BR_value.disable();
                         break;
        case EQUALIZER:  sdr_EQ_lowPass.disable();  sdr_EQ_bandPass.disable(); sdr_EQ_highPass.disable(); sdr_EQ_balance.disable();
                         btn_EQ_lowPass.disable();  btn_EQ_bandPass.disable(); btn_EQ_highPass.disable(); btn_EQ_balance.disable(); btn_EQ_Radio.disable();
                         txt_EQ_lowPass.disable();  txt_EQ_bandPass.disable(); txt_EQ_highPass.disable(); txt_EQ_balance.disable(); btn_EQ_Player.disable();
                         btn_EQ_Mute.disable();
                         break;
        case BLUETOOTH:  btn_BT_volDown.disable();  btn_BT_volUp.disable();    btn_BT_pause.disable();    btn_BT_radio.disable();   btn_BT_mode.disable();
                         btn_BT_power.disable(); pic_BT_mode.disable();     txt_BT_volume.disable();   txt_BT_mode.disable();
    }
    _f_volBarVisible = false;
    if(_timeCounter.timer){
        setTimeCounter(0);
    }

    dispHeader.updateItem(_hl_item[state]);
    switch(state) {
        case RADIO:{
            txt_RA_staName.enable();
            pic_RA_logo.enable();
            if(_state != RADIO) clearWithOutHeaderFooter();
            if(_radioSubmenue == 0){
                if(_f_irNumberSeen){txt_RA_irNum.hide(); setStation(_irNumber); _f_irNumberSeen = false;} // ir_number, valid between 1 ... 999
                if(_state != RADIO) {showLogoAndStationName(true);}
                setTimeCounter(0);
                VUmeter_RA.show();
                txt_RA_sTitle.setText("");
                txt_RA_sTitle.show();
                _f_newStreamTitle = true;
            }
            if(_radioSubmenue == 1){ // Mute, Vol+, Vol-, Sta+, Sta-, StaList
                clearTitle();
                sdr_RA_volume.show();
                btn_RA_Mute.show();
                btn_RA_prevSta.show();   btn_RA_nextSta.show();          btn_RA_staList.show();
            //    txt_RA_staName.show();
                setTimeCounter(2);
            }
            if(_radioSubmenue == 2){ // Player, DLNA, Clock, SleepTime, Brightness, EQ, BT, Off
                sdr_RA_volume.hide();
                btn_RA_player.show();    btn_RA_dlna.show();             btn_RA_clock.show();
                btn_RA_sleep.show();     btn_RA_bright.show(!_f_brightnessIsChangeable); btn_RA_equal.show();
                btn_RA_bt.show(!_f_BtEmitterFound); btn_RA_off.show();
                setTimeCounter(2);
            }
            if(_radioSubmenue == 3){ // show Numbers from IR
                char buf[10];
                itoa(_irNumber, buf, 10);
                txt_RA_irNum.setText(buf, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
                txt_RA_irNum.show();
                setTimeCounter(1);
            }
            if(_state != RADIO) webSrv.send("changeState=", "RADIO");
            break;
        }

        case STATIONSLIST:{
            clearWithOutHeaderFooter();
            lst_RADIO.show();
            setTimeCounter(4);
            break;
        }

        case PLAYER: {
            if(_state != PLAYER) clearWithOutHeaderFooter();
            if(_playerSubmenue == 0){
                _SD_content.listDir(_curAudioFolder, true, true);
                _cur_Codec = 0;
                showFileLogo(PLAYER);
                showFileName(_SD_content.getIndex(_cur_AudioFileNr));
           //     showAudioFileNumber();
                if(_state != PLAYER) webSrv.send("changeState=", "PLAYER");
                showAudioFileNumber();
                txt_PL_fName.show();
                btn_PL_prevFile.show(); btn_PL_nextFile.show(); btn_PL_ready.show(); btn_PL_playAll.show(); btn_PL_shuffle.show(); btn_PL_fileList.show(); btn_PL_radio.show(); btn_PL_off.show();
            }
            if(_playerSubmenue == 1){
                sdr_PL_volume.show();
            //    txt_PL_fName.setText("");
                btn_PL_Mute.show(); btn_PL_pause.setOff(); btn_PL_pause.show(); btn_PL_cancel.show(); txt_PL_fName.show();
            }
            break;
        }
        case AUDIOFILESLIST: {
            clearWithOutHeaderFooter();
            lst_PLAYER.show();
            setTimeCounter(4);
            break;
        }
        case DLNA:{
            clearWithOutHeaderFooter();
            showFileLogo(state);
            btn_DL_Mute.show();    btn_DL_pause.show();   btn_DL_cancel.show(); btn_DL_fileList.show(); btn_DL_radio.show();
            txt_DL_fName.show();
            sdr_DL_volume.show();
            break;
        }
        case DLNAITEMSLIST:{
            lst_DLNA.show(_currDLNAsrvNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
            setTimeCounter(4);
            break;
        }
        case CLOCK:{
            if(_clockSubMenue == 0){
                setTimeCounter(0);
                if(_state != CLOCK){
                    clearWithOutHeaderFooter();
                    clk_CL_green.updateTime(rtc.getMinuteOfTheDay(), rtc.getweekday());
                }
                else{
                    btn_CL_Mute.hide(); btn_CL_alarm.hide(); btn_CL_radio.hide(); sdr_CL_volume.hide(); btn_CL_off.hide();
                }
                clk_CL_green.show();
            }
            if(_clockSubMenue == 1){
                btn_CL_Mute.show();     btn_CL_alarm.show();    btn_CL_radio.show(); sdr_CL_volume.show(); btn_CL_off.show();
                setTimeCounter(2);
            }
            break;
        }
        case ALARM:{
            clearWithOutHeaderFooter();
            clk_AL_red.show();          btn_AL_left.show();     btn_AL_right.show();    btn_AL_up.show();      btn_AL_down.show();
            btn_AL_ready.show();
            break;
        }
        case SLEEP:{
            clearWithOutHeaderFooter();
            display_sleeptime();
            btn_SL_up.show();           btn_SL_down.show();       btn_SL_ready.show();    btn_SL_cancel.show();
            if(TFT_CONTROLLER < 2) drawImage("/common/Night_Gown.bmp", 198, 23);
            else                   drawImage("/common/Night_Gown.bmp", 280, 45);
            break;
        }
        case BRIGHTNESS:{
            clearWithOutHeaderFooter();
            btn_BR_ready.show(); pic_BR_logo.show();
            sdr_BR_value.show();
            txt_BR_value.setText(int2str(_brightness), TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_BR_value.show();
            break;
        }
        case EQUALIZER:
            if(_state != EQUALIZER) clearWithOutHeaderFooter();
            sdr_EQ_lowPass.show(); sdr_EQ_bandPass.show(); sdr_EQ_highPass.show(); sdr_EQ_balance.show();
            btn_EQ_lowPass.show(); btn_EQ_bandPass.show(); btn_EQ_highPass.show(); btn_EQ_balance.show(); btn_EQ_Radio.show(); btn_EQ_Player.show(); btn_EQ_Mute.show();
            txt_EQ_lowPass.show(); txt_EQ_bandPass.show(); txt_EQ_highPass.show(); txt_EQ_balance.show();
            break;

        case BLUETOOTH:
            clearWithOutHeaderFooter();
            btn_BT_volUp.show(); btn_BT_volDown.show(); btn_BT_pause.show(); btn_BT_mode.show(); btn_BT_radio.show(); btn_BT_power.show(); pic_BT_mode.show();
            char* mode = strdup(bt_emitter.getMode());
            if(strcmp(mode, "RX") == 0){
                txt_BT_mode.writeText("RECEIVER", TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
                if(bt_emitter.isConnected()) muteChanged(true);
            }
            else {
                txt_BT_mode.writeText("EMITTER", TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            }
            txt_BT_mode.setBGcolor(TFT_BROWN); txt_BT_mode.show();
            char c[10]; sprintf(c, "Vol: %02i", bt_emitter.getVolume()); txt_BT_volume.writeText(c, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); txt_BT_volume.show();
            if(_state != BLUETOOTH) webSrv.send("changeState=", "BLUETOOTH");
            if(mode){ free(mode); mode = NULL;}
            break;
    }
    _state = state;
}
// clang-format on

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                    L O O P                                                                                  ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

void loop() {
    if(!_f_ESPfound) return;    // Guard:  wrong chip?
    if(!_f_SD_MMCfound) return; // Guard:  SD_MMC could not be initialisized
    vTaskDelay(3); // feed the watchdog
    webSrv.loop();
    ir.loop();
    tp.loop();
    ftpSrv.handleFTP();
    ArduinoOTA.handle();
    dlna.loop();
    bt_emitter.loop();

    if(_f_dlnaBrowseServer) {_f_dlnaBrowseServer = false; dlna.browseServer(_currDLNAsrvNr, _dlnaHistory[_dlnaLevel].objId, _totalNumberReturned);}
    if(_f_clearLogo)        {_f_clearLogo = false; clearLogo();}
    if(_f_clearStationName) { _f_clearStationName = false; clearStationName();}

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
    //-----------------------------------------------------0.1 SEC------------------------------------------------------------------------------------
    if(_f_100ms) { // calls every 0.1 second
        _f_100ms = false;

        if(_state == RADIO && _radioSubmenue == 0) VUmeter_RA.update(audioGetVUlevel());

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
                        _radioSubmenue = 0;
                        changeState(RADIO);
                    }
                    else if(_state == CLOCK) {
                        _clockSubMenue = 0;
                        changeState(CLOCK);
                    }
                    //    else if(_state == RADIO && _f_switchToClock) { changeState(CLOCK); _f_switchToClock = false; }
                    else if(_state == STATIONSLIST) { changeState(RADIO); }
                    else if(_state == AUDIOFILESLIST) { _playerSubmenue = 0; changeState(PLAYER); }
                    else if(_state == DLNAITEMSLIST) { changeState(DLNA); }
                    else { ; } // all other, do nothing
                }
            }
        }

        uint8_t vol = audioGetVolume();
        uint8_t steps = _volumeSteps / 20;
        if(_f_mute){
            if(vol){
                if(vol >= steps)  vol -= steps;
                else vol--;
                audioSetVolume(vol);
            }
        }
        else{
            if(vol != _cur_volume){
                if      (vol > _cur_volume + steps) vol -= steps;
                else if (vol > _cur_volume) vol --;
                else if (vol < _cur_volume - steps) vol += steps;
                else if (vol < _cur_volume) vol ++;
                audioSetVolume(vol);
            }
        }
    }
    //-----------------------------------------------------1 SEC--------------------------------------------------------------------------------------

    if(_f_1sec) { // calls every second
        _f_1sec = false;
        clk_CL_green.updateTime(rtc.getMinuteOfTheDay(), rtc.getweekday());

        //------------------------------------------ALARM MANAGEMENT----------------------------------------------------------------------------------
        if(_f_alarm) {
            _f_alarm = false;
            clearAll();
            dispHeader.updateItem("ALARM");
            dispHeader.updateTime(_time_s);
            dispFooter.show();
            if(_ringVolume > 0){ // alarm with bell
                showFileName("ALARM");
                drawImage("/common/Alarm.jpg", _winLogo.x, _winLogo.y);
                setTFTbrightness(_brightness);
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm");
                if(AMP_ENABLED != -1) {digitalWrite(AMP_ENABLED, HIGH);}
                setVolume(_ringVolume);
                audioSetVolume(_ringVolume);
                muteChanged(false);
                connecttoFS("/ring/alarm_clock.mp3");
            }
            else{ // alarm without bell
                _f_eof_alarm = true;
                return;
            }
        }
        if(_f_eof_alarm) { // AFTER RINGING
            _f_eof_alarm = false;
            _cur_volume = _volumeAfterAlarm;
            setVolume(_cur_volume);
            audioSetVolume(_cur_volume);
            dispHeader.updateVolume(_cur_volume);
            wake_up();
        }
        //------------------------------------------UPDATE DISPLAY------------------------------------------------------------------------------------
        if(!_f_sleeping) {
            dispHeader.updateTime(_time_s, false);
            dispFooter.updateRSSI(WiFi.RSSI());

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
                sprintf(_chbuf, "/voice_time/%d_00.mp3", hour);
                SerialPrintfln("Time: ...... play Audiofile %s", _chbuf) connecttoFS(_chbuf);
                return;
            }
            else { SerialPrintfln("Time: ...... Announcement at %d o'clock is silent", hour); }
        }
        if(f_resume && _f_eof){
            f_resume = false;
            _f_eof = false;
            connecttohost(_lastconnectedhost.c_str());
            return;
        }
        //------------------------------------------HEADPHONE / LOUDSPEAKER --------------------------------------------------------------------------
        if(_f_hpChanged) {
            setVolume(_cur_volume);
            if(!digitalRead(HP_DETECT)) { SerialPrintfln("Headphone plugged in"); }
            else { SerialPrintfln("Headphone unplugged"); }
            _f_hpChanged = false;
        }
        //------------------------------------------AUDIO_CURRENT_TIME - DURATION---------------------------------------------------------------------
        if(audioIsRunning() && _f_isFSConnected) {
            _audioCurrentTime = audioGetCurrentTime();
            _audioFileDuration = audioGetFileDuration();
            if(_audioFileDuration) {
                SerialPrintfcr("AUDIO_FILE:  " ANSI_ESC_GREEN "AudioCurrentTime " ANSI_ESC_GREEN "%li:%02lis, " ANSI_ESC_GREEN "AudioFileDuration " ANSI_ESC_GREEN "%li:%02lis",
                               (long int)_audioCurrentTime / 60, (long int)_audioCurrentTime % 60, (long int)_audioFileDuration / 60, (long int)_audioFileDuration % 60);
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
            if((_commercial_dur == 2) && (_state == RADIO)) clearStreamTitle(); // end of commercial? clear streamtitle
        }
        //------------------------------------------DETERMINE AUDIOCODEC------------------------------------------------------------------------------
        if(_cur_Codec == 0) {
            uint8_t c = audioGetCodec();
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
        if(_f_connectToLasthost){
            _f_connectToLasthost = false;
            connecttohost(_lastconnectedhost.c_str());
        }
        //------------------------------------------RECONNECT AFTER FAIL------------------------------------------------------------------------------
        if(_f_reconnect){
            _f_reconnect = false;
            _reconnectCnt ++;
            if(_reconnectCnt < 3){
                SerialPrintfln("RECONNECTION " ANSI_ESC_RED "to %s, try %i", _lastconnectedhost.c_str(), _reconnectCnt);
                connectToWiFi();
                connecttohost(_lastconnectedhost.c_str());
                if(audioIsRunning()) _reconnectCnt = 0;
            }
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
        if((WiFi.status() != WL_CONNECTED)) {
            _WiFi_disconnectCnt ++;
            if(_WiFi_disconnectCnt == 15){
                _WiFi_disconnectCnt = 1;
                SerialPrintfln("WiFi         :  " ANSI_ESC_YELLOW "Reconnecting to WiFi...");
                WiFi.disconnect();
                WiFi.reconnect();
            }
        }
        else{
            if(_WiFi_disconnectCnt){
                _WiFi_disconnectCnt = 0;
                if(_state == RADIO) audioConnecttohost(_lastconnectedhost.c_str());
            }
        }
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
            uint16_t ambVal = BH1750.getBrightness();
            if(ambVal > 1500) ambVal = 1500;
            _bh1750Value = map_l(ambVal, 0, 1500, 5, 100);

            BH1750.start();
        //    log_i("_bh1750Value %i, _brightness %i", _bh1750Value, _brightness);
            if(!_f_sleeping){
                if(_bh1750Value >= _brightness) setTFTbrightness(_bh1750Value);
                else setTFTbrightness(_brightness);
            }
        }
    } //  END _f_1sec


    if(_f_10sec == true) { // calls every 10 seconds
        _f_10sec = false;
        if(_state == RADIO && !_icyBitRate && !_f_sleeping) {
            _decoderBitRate = audioGetBitRate();
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
    }

    //-------------------------------------------------DEBUG / TEST ----------------------------------------------------------------------------------
    if(Serial.available()) { // input: serial terminal
        String r = Serial.readString();
        r.replace("\n", "");
        SerialPrintfln("Terminal  :  " ANSI_ESC_YELLOW "%s", r.c_str());
        if(r.startsWith("p")) {
            bool res = audioPauseResume();
            if(res) { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume"); }
            else { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "Pause-Resume not possible"); }
        }
        if(r.startsWith("h")) { // A hardcopy of the display is created and written to the SD card
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "create hardcopy"); }
            hardcopy();
        }
        if(r.startsWith("rts")) {
            char timeStatsBuffer[1024 * 2];
            GetRunTimeStats(timeStatsBuffer);
            { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "task statistics\n\n%s", timeStatsBuffer); }
        }
        if(r.startsWith("a")) {
            audioConnecttospeech("Hallo, wie geht es dir?", "de");
        }

        if(r.toInt() != 0) { // is integer?
            if(audioSetTimeOffset(r.toInt())) { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "TimeOffset %li", r.toInt()); }
            else { SerialPrintfln("Terminal   : " ANSI_ESC_YELLOW "TimeOffset not possible"); }
        }
        if(r.startsWith("bfi")){ // buffer filled
            SerialPrintfln("inBuffer  :  filled %lu bytes", (long unsigned)audioInbuffFilled());
            SerialPrintfln("inBuffer  :  free   %lu bytes", (long unsigned)audioInbuffFree());
        }
        if(r.startsWith("st")){ // testtext for streamtitle
            if(r[2] == '0') strcpy(_streamTitle, "A B C D E F G H I");
            if(r[2] == '1') strcpy(_streamTitle, "A B C D E F G H I");
            if(r[2] == '2') strcpy(_streamTitle, "A B C D E F G H I J K L");
            if(r[2] == '4') strcpy(_streamTitle, "A B C D E F G H I J K J M Q O");
            if(r[2] == '5') strcpy(_streamTitle, "A B C D E F G H I K L J M y O P Q R");
            if(r[2] == '6') strcpy(_streamTitle, "A B C D E F G H I K L J M g O P Q R S T V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '7') strcpy(_streamTitle, "A B C D E F G H I K L J M j O P Q R S T U V A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '8') strcpy(_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            if(r[2] == '9') strcpy(_streamTitle, "A B C D E F G H I K L J M p O P Q R S T U V W K J Q p O P Q R S T U V W K J Q");
            log_w("st: %s", _streamTitle);
            _f_newStreamTitle = true;
        }
    }
}

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                  E V E N T S                                                                                ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

// Events from audioI2S library
void audio_info(const char* info) {
    if(endsWith(  info, "failed!"))                {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED, info); WiFi.disconnect();  WiFi.begin();}
    if(startsWith(info, "FLAC"))                   {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN, info); return;}
    if(endsWith(  info, "Stream lost"))            {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED, info); return;}
    if(startsWith(info, "authent"))                {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_GREEN, info); return;}
    if(startsWith(info, "StreamTitle="))           {return;}
    if(startsWith(info, "HTTP/") && info[9] > '3') {SerialPrintflnCut("AUDIO_info:  ", ANSI_ESC_RED, info); return;}
//    if(startsWith(info, "connect to"))             {IPAddress dns1(8, 8, 8, 8); IPAddress dns2(8, 8, 4, 4); WiFi.setDNS(dns1, dns2);}
    if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN) {{SerialPrintfln("AUDIO_info:  " ANSI_ESC_GREEN "%s", info);} return;} // all other
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_showstation(const char* info) {
    if(!info) return;
    if(_stationName_air) {free(_stationName_air); _stationName_air = NULL;}
    _stationName_air = x_ps_strdup(info);
    SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", info);
    _f_newStationName = true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_showstreamtitle(const char* info) {
    strcpy(_streamTitle, info);
    if(!_f_irNumberSeen) _f_newStreamTitle = true;
    SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", info);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
void audio_commercial(const char* info) { show_ST_commercial(info); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_eof_mp3(const char* info) { // end of mp3 file (filename)
    if(startsWith(info, "alarm")) _f_eof_alarm = true;
    SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, info);
    if(_state == PLAYER) {
        if(!_f_playlistEnabled) {
            _f_clearLogo = true;
            _f_clearStationName = true;
        }
    }
    webSrv.send("SD_playFile=", "end of audiofile");
    _f_eof = true;
    _f_isFSConnected = false;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_eof_stream(const char* info) {
    _f_isWebConnected = false;
    SerialPrintflnCut("end of file: ", ANSI_ESC_YELLOW, info);
    if(_state == PLAYER) {
        if(!_f_playlistEnabled) {
            _f_clearLogo = true;
            _f_clearStationName = true;
        }
    }
    if(_state == DLNA) { showFileName(""); }
    if(_state == RADIO) { clearWithOutHeaderFooter(); }
    _f_eof = true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_lasthost(const char* info) { // really connected URL
    if(_f_playlistEnabled) return;
    _lastconnectedhost = info;
    SerialPrintflnCut("lastURL: ..  ", ANSI_ESC_WHITE, _lastconnectedhost.c_str());
    webSrv.send("stationURL=", _lastconnectedhost);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_icyurl(const char* info) { // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5) {
        SerialPrintflnCut("icy-url: ..  ", ANSI_ESC_WHITE, info);
        _homepage = String(info);
        if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_icylogo(const char* info) { // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5) { SerialPrintflnCut("icy-logo:    ", ANSI_ESC_WHITE, info); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_id3data(const char* info) { SerialPrintfln("id3data: ..  " ANSI_ESC_GREEN "%s", info); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_id3image(File& audiofile, const size_t APIC_pos, const size_t APIC_size) { SerialPrintfln("CoverImage:  " ANSI_ESC_GREEN "Position %i, Size %i bytes", APIC_pos, APIC_size); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_oggimage(File& audiofile, std::vector<uint32_t> vec) { // OGG blockpicture
    SerialPrintfln("oggimage:..  " ANSI_ESC_GREEN "---------------------------------------------------------------------------");
    SerialPrintfln("oggimage:..  " ANSI_ESC_GREEN "ogg metadata blockpicture found:");
    for(int i = 0; i < vec.size(); i += 2) { SerialPrintfln("oggimage:..  " ANSI_ESC_GREEN "segment %02i, pos %07ld, len %05ld", i / 2, (long unsigned int)vec[i], (long unsigned int)vec[i + 1]); }
    SerialPrintfln("oggimage:..  " ANSI_ESC_GREEN "---------------------------------------------------------------------------");
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_icydescription(const char* info) {
    strcpy(_icyDescription, info);
    _f_newIcyDescription = true;
    if(strlen(info)) SerialPrintfln("icy-descr:   %s", info);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_bitrate(const char* info) {
    if(!strlen(info)) return; // guard
    _icyBitRate = str2int(info) / 1000;
    _f_newBitRate = true;
    SerialPrintfln("bitRate:     " ANSI_ESC_CYAN "%iKbit/s", _icyBitRate);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void audio_eof_speech(const char*) {
    _f_connectToLasthost = true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ftp_debug(const char* info) {
    if(startsWith(info, "File Name")) return;
    SerialPrintfln("ftpServer:   %s", info);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void RTIME_info(const char* info) { SerialPrintfln("rtime_info:  %s", info); }

// Events from tft library
void tft_info(const char* info) { SerialPrintfln("tft_info: .  %s", info); }

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
    return;
}
void ir_number(uint16_t num) {
    if(_state != RADIO) return;
    if(_f_sleeping) return;
    _f_irNumberSeen = true;
    _irNumber = num;
    _radioSubmenue = 3;
    changeState(RADIO);
}
void ir_key(uint8_t key) {
    if(_f_sleeping == true && key != 10) return;
    if(_f_sleeping == true && key == 10) {
        wake_up();
        return;
    } // awake

    switch(key) {
        case 15:    if(_state == SLEEP) {changeState(RADIO); break;} // CLOCK <-> RADIO
                    if(_state == RADIO) {changeState(CLOCK); break;}
                    if(_state == CLOCK) {changeState(RADIO); break;}
                    break;
        case 11:    upvolume(); // VOLUME+
                    break;
        case 12:    downvolume(); // VOLUME-
                    break;
        case 14:    if(_state == RADIO) {nextStation(); break;} // NEXT STATION
                    if(_state == CLOCK) {nextStation(); changeState(RADIO); _f_switchToClock = true; break;}
                    if(_state == SLEEP) {display_sleeptime(1); break;}
                    break;
        case 13:    if(_state == RADIO) {prevStation(); break;} // PREV STATION
                    if(_state == CLOCK) {prevStation(); changeState(RADIO); _f_switchToClock = true; break;}
                    if(_state == SLEEP) {display_sleeptime(-1); break;}
                    break;
        case 10:    muteChanged(!_f_mute);
                    break;
        case 16:    if(_state == RADIO) {changeState(SLEEP); break;} // OFF TIMER
                    if(_state == SLEEP) {changeState(RADIO); break;}
                    break;
        default:    break;
    }
}
void ir_long_key(int8_t key) {
    SerialPrintfln("ir_code: ..  " ANSI_ESC_YELLOW "long pressed key nr: " ANSI_ESC_BLUE "%02i", key);
    if(key == 10) fall_asleep(); // long mute
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Event from TouchPad
// clang-format off
void tp_pressed(uint16_t x, uint16_t y) {
    //  SerialPrintfln("tp_pressed, state is: %i", _state);
    //  SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint  x=%d, y=%d", x, y);

    if(_f_sleeping) return;  // awake in tp_released()

    // all state
    if(dispFooter.positionXY(x, y)) return;

    switch(_state) {
        case RADIO:
            if(_radioSubmenue == 1){
                if(sdr_RA_volume.positionXY(x,y)) return;
                if(btn_RA_Mute.positionXY(x, y)) return;
                if(btn_RA_prevSta.positionXY(x, y)) return;
                if(btn_RA_nextSta.positionXY(x, y)) return;
                if(btn_RA_staList.positionXY(x, y)) return;
                if(VUmeter_RA.positionXY(x, y)) return;
            }
            if(_radioSubmenue == 2){
                if(btn_RA_player.positionXY(x, y)) return;
                if(btn_RA_dlna.positionXY(x, y)) return;
                if(btn_RA_clock.positionXY(x, y)) return;
                if(btn_RA_sleep.positionXY(x, y)) return;
                if(btn_RA_bright.positionXY(x, y)) return;
                if(btn_RA_equal.positionXY(x, y)) return;
                if(btn_RA_bt.positionXY(x, y)) return;
                if(btn_RA_off.positionXY(x, y)) return;
            }
            _radioSubmenue++;
            if(_radioSubmenue == 3) _radioSubmenue = 0;
            changeState(RADIO);
            return;
            break;
        case PLAYER:
            if(_playerSubmenue == 0){
                if(btn_PL_prevFile.positionXY(x, y)) return;
                if(btn_PL_nextFile.positionXY(x, y)) return;
                if(btn_PL_ready.positionXY(x, y)) return;;
                if(btn_PL_playAll.positionXY(x, y)) return;
                if(btn_PL_shuffle.positionXY(x, y)) return;
                if(btn_PL_fileList.positionXY(x, y)) return;
                if(btn_PL_radio.positionXY(x, y)) return;
                if(btn_PL_off.positionXY(x, y)) return;
            }
            if(_playerSubmenue == 1){
                if(sdr_PL_volume.positionXY(x,y)) return;
                if(btn_PL_Mute.positionXY(x, y)) return;
                if(btn_PL_pause.positionXY(x, y)) return;
                if(btn_PL_cancel.positionXY(x, y)) return;
            }
            break;
        case AUDIOFILESLIST:
                if(lst_PLAYER.positionXY(x, y)) return;
                break;
        case DLNA:
                if(sdr_DL_volume.positionXY(x,y)) return;
                if(btn_DL_Mute.positionXY(x, y)) return;
                if(btn_DL_pause.positionXY(x, y)) return;
                if(btn_DL_radio.positionXY(x, y)) return;
                if(btn_DL_fileList.positionXY(x, y)) return;
                if(btn_DL_cancel.positionXY(x, y)) return;
                break;
        case DLNAITEMSLIST:
                if(lst_DLNA.positionXY(x, y)) return;
                break;
        case CLOCK:
            if(_clockSubMenue == 0){
                if(clk_CL_green.positionXY(x, y)) return;
            }
            if(_clockSubMenue == 1){
                if(btn_CL_Mute.positionXY(x, y)) return;
                if(btn_CL_alarm.positionXY(x, y)) return;
                if(btn_CL_radio.positionXY(x, y)) return;
                if(sdr_CL_volume.positionXY(x,y)) return;
                if(btn_CL_off.positionXY(x, y)) return;
            }
            break;
        case ALARM:
                if(clk_AL_red.positionXY(x, y)) return;
                if(btn_AL_left.positionXY(x, y)) return;
                if(btn_AL_right.positionXY(x, y)) return;
                if(btn_AL_up.positionXY(x, y)) return;
                if(btn_AL_down.positionXY(x, y)) return;
                if(btn_AL_ready.positionXY(x, y)) return;
                break;
        case SLEEP:
                if(btn_SL_up.positionXY(x, y)) return;
                if(btn_SL_down.positionXY(x, y)) return;
                if(btn_SL_ready.positionXY(x, y)) return;
                if(btn_SL_cancel.positionXY(x, y)) return;
                break;
        case BRIGHTNESS:
                if(sdr_BR_value.positionXY(x,y)) return;
                if(btn_BR_ready.positionXY(x, y)) return;
                if(pic_BR_logo.positionXY(x, y)) return;
                break;
        case EQUALIZER:
                if(sdr_EQ_lowPass.positionXY(x,y)) return;
                if(sdr_EQ_bandPass.positionXY(x,y)) return;
                if(sdr_EQ_highPass.positionXY(x,y)) return;
                if(sdr_EQ_balance.positionXY(x,y)) return;
                if(btn_EQ_lowPass.positionXY(x, y)) return;
                if(btn_EQ_bandPass.positionXY(x, y)) return;
                if(btn_EQ_highPass.positionXY(x, y)) return;
                if(btn_EQ_balance.positionXY(x, y)) return;
                if(txt_EQ_lowPass.positionXY(x, y)) return;
                if(txt_EQ_bandPass.positionXY(x, y)) return;
                if(txt_EQ_highPass.positionXY(x, y)) return;
                if(txt_EQ_balance.positionXY(x, y)) return;
                if(btn_EQ_Radio.positionXY(x, y)) return;
                if(btn_EQ_Player.positionXY(x,y)) return;
                if(btn_EQ_Mute.positionXY(x, y)) return;
                break;
        case BLUETOOTH:
                if(btn_BT_volUp.positionXY(x, y)) return;
                if(btn_BT_volDown.positionXY(x, y)) return;
                if(btn_BT_pause.positionXY(x, y)) return;
                if(btn_BT_mode.positionXY(x, y)) return;
                if(btn_BT_radio.positionXY(x, y)) return;
                if(btn_BT_power.positionXY(x, y)) return;
                break;
        default:
                break;
    }
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
        lst_DLNA.longPressed(x, y);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_released(uint16_t x, uint16_t y){

    if(_f_sleeping){ wake_up(); return;}   // if sleeping

    // all state
    dispHeader.released();
    dispFooter.released();

    switch(_state){
        case RADIO:
            if(_radioSubmenue == 0){ VUmeter_RA.released();}
            if(_radioSubmenue == 1){ sdr_RA_volume.released(); btn_RA_Mute.released(); btn_RA_prevSta.released(); btn_RA_nextSta.released(); btn_RA_staList.released();}
            if(_radioSubmenue == 2){ btn_RA_player.released(); btn_RA_dlna.released(); btn_RA_clock.released(); btn_RA_sleep.released(); btn_RA_bright.released(); btn_RA_equal.released();
                                     btn_RA_bt.released(); btn_RA_off.released();}
            break;
        case STATIONSLIST:
            lst_RADIO.released();
            break;
        case PLAYER:
            if(_playerSubmenue == 0){btn_PL_prevFile.released(); btn_PL_nextFile.released(); btn_PL_ready.released(); btn_PL_playAll.released(); btn_PL_shuffle.released(); btn_PL_fileList.released();
                                     btn_PL_radio.released(); btn_PL_off.released();}
            if(_playerSubmenue == 1){btn_PL_Mute.released(); btn_PL_pause.released(); btn_PL_cancel.released(); sdr_PL_volume.released();}
            break;
        case AUDIOFILESLIST:
            lst_PLAYER.released(x, y);
            break;
        case DLNA:
            sdr_DL_volume.released(); btn_DL_Mute.released(); btn_DL_pause.released(); btn_DL_radio.released(); btn_DL_fileList.released(); btn_DL_cancel.released();
            break;
        case DLNAITEMSLIST:
            lst_DLNA.released(x, y);
            break;
        case CLOCK:
            btn_CL_Mute.released(); btn_CL_alarm.released(); btn_CL_radio.released(); clk_CL_green.released(); sdr_CL_volume.released(); btn_CL_off.released();
            break;
        case ALARM:
            clk_AL_red.released(); btn_AL_left.released(); btn_AL_right.released(); btn_AL_up.released(); btn_AL_down.released(); btn_AL_ready.released();
            break;
        case SLEEP:
            btn_SL_up.released(); btn_SL_down.released(); btn_SL_ready.released(); btn_SL_cancel.released();
            break;
        case BRIGHTNESS:
            sdr_BR_value.released();  btn_BR_ready.released(); pic_BR_logo.released();
            break;
        case EQUALIZER:
            sdr_EQ_lowPass.released(); sdr_EQ_bandPass.released(); sdr_EQ_highPass.released(); sdr_EQ_balance.released(); btn_EQ_lowPass.released(); btn_EQ_bandPass.released();
            btn_EQ_highPass.released(); btn_EQ_balance.released(); txt_EQ_lowPass.released(); txt_EQ_bandPass.released(); txt_EQ_highPass.released(); txt_EQ_balance.released();
            btn_EQ_Radio.released(); btn_EQ_Player.released(); btn_EQ_Mute.released();
            break;
        case BLUETOOTH:
            btn_BT_pause.released(); btn_BT_radio.released(); btn_BT_volDown.released(); btn_BT_volUp.released(); btn_BT_mode.released(); btn_BT_power.released();
            break;
    }
    // SerialPrintfln("tp_released, state is: %i", _state);
}

void tp_long_released(){
//    log_w("long released)");
    if(_state == DLNAITEMSLIST) {lst_DLNA.longReleased();}
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void tp_positionXY(uint16_t x, uint16_t y){
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
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//Events from websrv
void WEBSRV_onCommand(const String cmd, const String param, const String arg){  // called from html

    if(CORE_DEBUG_LEVEL == ARDUHAL_LOG_LEVEL_DEBUG){
        SerialPrintfln("WS_onCmd:    " ANSI_ESC_YELLOW "cmd=\"%s\", params=\"%s\", arg=\"%s\"",
                                                        cmd.c_str(),param.c_str(), arg.c_str());
    }

    String  str;

    if(cmd == "ping"){              webSrv.send("pong"); return;}                                                                                     // via websocket

    if(cmd == "index.html"){        if(_f_accessPoint) {SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "accesspoint.html");                           // via XMLHttpRequest
                                                        webSrv.show(accesspoint_html, webSrv.TEXT);}
                                    else               {SerialPrintfln("Webpage:     " ANSI_ESC_ORANGE "index.html");
                                                        webSrv.show(index_html, webSrv.TEXT);      }
                                    return;}

    if(cmd == "index.js"){          SerialPrintfln("Script:      " ANSI_ESC_ORANGE "index.js");                                                       // via XMLHttpRequest
                                    webSrv.show(index_js, webSrv.JS); return;}

    if(cmd == "favicon.ico"){       webSrv.streamfile(SD_MMC, "/favicon.ico"); return;}                                                               // via XMLHttpRequest

    if(cmd == "test"){              sprintf(_chbuf, "free heap: %lu, Inbuff filled: %lu, Inbuff free: %lu, PSRAM filled %lu, PSRAM free %lu",
                                        (long unsigned)ESP.getFreeHeap(), (long unsigned)audioInbuffFilled(), (long unsigned)audioInbuffFree(),
                                        (long unsigned) (ESP.getPsramSize() - ESP.getFreePsram()), (long unsigned)ESP.getFreePsram());
                                    webSrv.send("test=", _chbuf);
                                //    SerialPrintfln("audiotask .. stackHighWaterMark: %lu bytes", (long unsigned)audioGetStackHighWatermark() * 4);
                                    SerialPrintfln("looptask ... stackHighWaterMark: %lu bytes", (long unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
                                    return;}

    if(cmd == "getmute"){           if(_f_mute) webSrv.send("mute=", "1");
                                    else        webSrv.send("mute=", "0");
                                    return;}

    if(cmd == "setmute"){           muteChanged(!_f_mute); return;}

    if(cmd == "upvolume"){          webSrv.send("volume=", int2str(upvolume()));  return;}                                                            // via websocket
    if(cmd == "downvolume"){        webSrv.send("volume=", int2str(downvolume())); return;}                                                           // via websocket

    if(cmd == "getVolumeSteps"){    webSrv.send("volumeSteps=", int2str(_volumeSteps)); return;}
    if(cmd == "setVolumeSteps"){    _cur_volume = map_l(_cur_volume, 0, _volumeSteps, 0, param.toInt());
                                    _ringVolume = map_l(_ringVolume, 0, _volumeSteps, 0, param.toInt()); webSrv.send("ringVolume=", int2str(_ringVolume));
                                    _volumeAfterAlarm = map_l(_volumeAfterAlarm, 0, _volumeSteps, 0, param.toInt()); webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm));
                                    _volumeSteps = param.toInt(); webSrv.send("volumeSteps=", param); audioSetVolumeSteps(_volumeSteps);
                                    // log_w("_volumeSteps  %i", _volumeSteps);
                                    sdr_CL_volume.setNewMinMaxVal(0, _volumeSteps);
                                    sdr_DL_volume.setNewMinMaxVal(0, _volumeSteps);
                                    sdr_PL_volume.setNewMinMaxVal(0, _volumeSteps);
                                    sdr_RA_volume.setNewMinMaxVal(0, _volumeSteps);
                                    setVolume(_cur_volume);
                                    SerialPrintfln("action: ...  new volume steps: " ANSI_ESC_CYAN "%d", _volumeSteps);
                                    return;}

    if(cmd == "getRingVolume"){     webSrv.send("ringVolume=", int2str(_ringVolume)); return;}
    if(cmd == "setRingVolume"){     _ringVolume = param.toInt(); webSrv.send("ringVolume=", int2str(_ringVolume));
                                    SerialPrintfln("action: ...  new ring volume: " ANSI_ESC_CYAN "%d", _ringVolume); return;}

    if(cmd == "getVolAfterAlarm"){  webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm)); return;}
    if(cmd == "setVolAfterAlarm"){  _volumeAfterAlarm = param.toInt(); webSrv.send("volAfterAlarm=", int2str(_volumeAfterAlarm));
                                    SerialPrintfln("action: ...  new volume after alarm: " ANSI_ESC_CYAN "%d", _volumeAfterAlarm); return;}

    if(cmd == "homepage"){          webSrv.send("homepage=", _homepage);
                                    return;}

    if(cmd == "to_listen"){         StationsItems(); // via websocket, return the name and number of the current station
                                    return;}

    if(cmd == "gettone"){           webSrv.send("settone=", setI2STone());
                                    return;}

    if(cmd == "getstreamtitle"){    webSrv.reply(_streamTitle, webSrv.TEXT);
                                    return;}

    if(cmd == "LowPass"){           _toneLP = param.toInt();                           // audioI2S tone
                                    char lp[30] = "Lowpass set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                    webSrv.send("tone=", lp); setI2STone(); return;}

    if(cmd == "BandPass"){          _toneBP = param.toInt();                           // audioI2S tone
                                    char bp[30] = "Bandpass set to "; strcat(bp, param.c_str()); strcat(bp, "dB");
                                    webSrv.send("tone=", bp); setI2STone(); return;}

    if(cmd == "HighPass"){          _toneHP = param.toInt();                           // audioI2S tone
                                    char hp[30] = "Highpass set to "; strcat(hp, param.c_str()); strcat(hp, "dB");
                                    webSrv.send("tone=", hp); setI2STone(); return;}

    if(cmd == "Balance"){           _toneBAL = param.toInt();
                                    char bal[30] = "Balance set to "; strcat(bal, param.c_str());
                                    webSrv.send("tone=", bal); setI2STone(); return;}

    if(cmd == "uploadfile"){        _filename = param;  return;}

    if(cmd == "prev_station"){      prevStation(); return;}                                                                                           // via websocket

    if(cmd == "next_station"){      nextStation(); return;}                                                                                           // via websocket

    if(cmd == "set_station"){       setStation(param.toInt()); return;}                                                                               // via websocket

    if(cmd == "stationURL"){        setStationViaURL(param.c_str()); audio_showstation(param.c_str()); return;}                                                                         // via websocket

    if(cmd == "getnetworks"){       webSrv.send("networks=", WiFi.SSID()); return;}                                                  // via websocket

    if(cmd == "get_tftSize"){       if(_tftSize){webSrv.send("tftSize=", "m");} else{webSrv.send("tftSize=", "s");} return;};

    if(cmd == "getTimeZones"){      webSrv.streamfile(SD_MMC, "/timezones.csv"); return;}

    if(cmd == "setTimeZone"){       _TZName = param;  _TZString = arg;
                                    SerialPrintfln("Timezone: .. " ANSI_ESC_BLUE "%s, %s", param.c_str(), arg.c_str());
                                    setRTC(_TZString.c_str());
                                    updateSettings(); // write new TZ items to settings.json
                                    return;}

    if(cmd == "getTimeZoneName"){   webSrv.reply(_TZName, webSrv.TEXT); return;}

    if(cmd == "change_state"){      if(_state != CLOCK){
                                        if     (!strcmp(param.c_str(), "RADIO")      && _state != RADIO)     {setStation(_cur_station); changeState(RADIO); return;}
                                        else if(!strcmp(param.c_str(), "PLAYER")     && _state != PLAYER)    {stopSong(); changeState(PLAYER); return;}
                                        else if(!strcmp(param.c_str(), "DLNA")       && _state != DLNA)      {stopSong(); changeState(DLNA);   return;}
                                        else if(!strcmp(param.c_str(), "BLUETOOTH")  && _state != BLUETOOTH) {changeState(BLUETOOTH); return;}
                                        else return;
                                    }}
    if(cmd == "stopfile"){          _resumeFilePos = audioStopSong(); webSrv.send("stopfile=", "audiofile stopped");
                                    return;}

    if(cmd == "resumefile"){        if(!_lastconnectedfile) webSrv.send("resumefile=", "nothing to resume");
                                    else {
                                        SD_playFile(_lastconnectedfile, _resumeFilePos);
                                        webSrv.send("resumefile=", "audiofile resumed");
                                    }
                                    return;}

    if(cmd == "get_alarmdays"){     webSrv.send("alarmdays=", String(_alarmdays, 10)); return;}

    if(cmd == "set_alarmdays"){     _alarmdays = param.toInt(); updateSettings(); return;}

    if(cmd == "get_alarmtime"){     return;} // not used yet

    if(cmd == "set_alarmtime"){     return;}

    if(cmd == "get_timeAnnouncement"){ if(_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "1");
                                    if(  !_f_timeAnnouncement) webSrv.send("timeAnnouncement=", "0");
                                    return;}

    if(cmd == "set_timeAnnouncement"){ if(param == "true" ) {_f_timeAnnouncement = true;}
                                    if(   param == "false") {_f_timeAnnouncement = false;}
                                    SerialPrintfln("Timespeech   " ANSI_ESC_YELLOW "hourly time announcement " ANSI_ESC_BLUE "%s", (_f_timeAnnouncement == 1) ? "on" : "off");
                                    return;}

    if(cmd == "DLNA_getServer")  {  webSrv.send("DLNA_Names=", dlna.stringifyServer()); _currDLNAsrvNr = -1; return;}

    if(cmd == "DLNA_getRoot")    {  _currDLNAsrvNr = param.toInt(); dlna.browseServer(_currDLNAsrvNr, "0"); return;}

    if(cmd == "DLNA_getContent") {  if(param.startsWith("http")) {connecttohost(param.c_str()); showFileName(arg.c_str()); return;}
                                    if(_dlnaHistory[_dlnaLevel].objId){free(_dlnaHistory[_dlnaLevel].objId); _dlnaHistory[_dlnaLevel].objId = NULL;} _dlnaHistory[_dlnaLevel].objId = strdup(param.c_str());
                                    _totalNumberReturned = 0;
                                    dlna.browseServer(_currDLNAsrvNr, _dlnaHistory[_dlnaLevel].objId);
                                    return;}

    if(cmd == "AP_ready"){          webSrv.send("networks=", _scannedNetworks); return;}                                                              // via websocket

    if(cmd == "credentials"){       String AP_SSID = param.substring(0, param.indexOf("\n"));                                                         // via websocket
                                    String AP_PW =   param.substring(param.indexOf("\n") + 1);
                                    SerialPrintfln("credentials: SSID " ANSI_ESC_BLUE "%s" ANSI_ESC_WHITE ", PW " ANSI_ESC_BLUE "%s",
                                    AP_SSID.c_str(), AP_PW.c_str());
                                    pref.putString("ap_ssid", AP_SSID);
                                    pref.putString("ap_pw", AP_PW);
                                    ESP.restart();}

    if(cmd.startsWith("SD/")){      String str = cmd.substring(2);                                                                                    // via XMLHttpRequest
                                    if(!webSrv.streamfile(SD_MMC, scaleImage(str.c_str()))){
                                        SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "File not found " ANSI_ESC_RED "\"%s\"", str.c_str());
                                        webSrv.streamfile(SD_MMC, scaleImage("/common/unknown.jpg"));}
                                    return;}

    if(cmd == "SD_Download"){       webSrv.streamfile(SD_MMC, param.c_str());
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Download  " ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                    return;}

    if(cmd == "SD_GetFolder"){      webSrv.reply(SD_stringifyDirContent(param), webSrv.JS);
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

    if(cmd == "SD_playAllFiles"){   webSrv.send("SD_playFolder=", "" + param);
                                    SerialPrintfln("webSrv: ...  " ANSI_ESC_YELLOW "Play Folder" ANSI_ESC_ORANGE "\"%s\"", param.c_str());
                                    preparePlaylistFromSDFolder(param.c_str());
                                    processPlaylist(true);
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
                                   SerialPrintfln("set_IR_cmd:  " ANSI_ESC_YELLOW "IR command " ANSI_ESC_BLUE "0x%02lx, "
                                   ANSI_ESC_YELLOW "IR Button Number " ANSI_ESC_BLUE "0x%02lx", (long signed)command, (long signed)btnNr);
                                   ir.set_irButtons(btnNr,  command);
                                   return;}
    if(cmd == "setIRadr"){         SerialPrintfln("set_IR_adr:  " ANSI_ESC_YELLOW "IR address " ANSI_ESC_BLUE "%s",
                                   param.c_str());
                                   int32_t address = (int32_t)strtol(param.c_str(), NULL, 16);
                                   ir.set_irAddress(address);
                                   return;}

    if(cmd == "saveIRbuttons"){    saveIRbuttonsToNVS(); return;}

    if(cmd == "getTimeFormat"){    webSrv.send("timeFormat=", String(_timeFormat, 10));
                                   return;}

    if(cmd == "setTimeFormat"){    _timeFormat = param.toInt();
                                   clk_CL_green.setTimeFormat(_timeFormat);
                                   if(_state == CLOCK){
                                        clearWithOutHeaderFooter();
                                   }
                                   SerialPrintfln("TimeFormat:  " ANSI_ESC_YELLOW "new time format: " ANSI_ESC_BLUE "%sh", param.c_str());
                                   return;}

    if(cmd == "getSleepMode"){     webSrv.send("sleepMode=", String(_sleepMode, 10));
                                   return;}

    if(cmd == "setSleepMode"){     _sleepMode = param.toInt();
                                   if(_sleepMode == 0) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Display off");
                                   if(_sleepMode == 1) SerialPrintfln("SleepMode:   " ANSI_ESC_YELLOW "Show the time");
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
    if(cmd == "KCX_BT_connected") {if     (!_f_BTpower)              webSrv.send("KCX_BT_connected=", "-1");
                                   else if(bt_emitter.isConnected()) webSrv.send("KCX_BT_connected=",  "1");
                                   else                              webSrv.send("KCX_BT_connected=",  "0");
                                   return;}
    if(cmd == "KCX_BT_clearItems"){bt_emitter.deleteVMlinks(); return;}
    if(cmd == "KCX_BT_addName"){   bt_emitter.addLinkName(param.c_str()); return;}
    if(cmd == "KCX_BT_addAddr"){   bt_emitter.addLinkAddr(param.c_str()); return;}
    if(cmd == "KCX_BT_mem"){       bt_emitter.getVMlinks(); return;}
    if(cmd == "KCX_BT_scanned"){   webSrv.send("KCX_BT_SCANNED=", bt_emitter.stringifyScannedItems()); return;}
    if(cmd == "KCX_BT_getMode"){   webSrv.send("KCX_BT_MODE=", bt_emitter.getMode()); return;}
    if(cmd == "KCX_BT_changeMode"){bt_emitter.changeMode(); return;}
    if(cmd == "KCX_BT_pause"){     bt_emitter.pauseResume(); return;}
    if(cmd == "KCX_BT_downvolume"){if(_BTvolume > 0)  {_BTvolume--; bt_emitter.downvolume();} return;}
    if(cmd == "KCX_BT_upvolume")  {if(_BTvolume < 31) {_BTvolume++; bt_emitter.upvolume();}   return;}
    if(cmd == "KCX_BT_getPower")  {if(_f_BTpower) webSrv.send("KCX_BT_power=", "1"); else webSrv.send("KCX_BT_power=", "0"); return;}
    if(cmd == "KCX_BT_power")     {_f_BTpower = !_f_BTcurPowerState; BTpowerChanged(!_f_BTcurPowerState); return;}

    if(cmd == "hardcopy") {SerialPrintfln("Webpage: ... " ANSI_ESC_YELLOW "create a display hardcopy"); hardcopy(); webSrv.send("hardcopy=", "/hardcopy.bmp"); return;}

    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s, param=%s", cmd.c_str(), param.c_str());
    webSrv.sendStatus(400);
}
// clang-format on
void WEBSRV_onRequest(const String request, uint32_t contentLength) {
    if(CORE_DEBUG_LEVEL > ARDUHAL_LOG_LEVEL_INFO) { SerialPrintfln("WS_onReq:    " ANSI_ESC_YELLOW "%s contentLength %lu", request.c_str(), (long unsigned)contentLength); }

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
    if(startsWith(info, "WebSocket")) return;      // suppress WebSocket client available
    if(!strcmp("ping", info)) return;              // suppress ping
    if(!strcmp("to_listen", info)) return;         // suppress to_isten
    if(startsWith(info, "Command client")) return; // suppress Command client available
    if(startsWith(info, "Content-D")) return;      // Content-Disposition
    if(CORE_DEBUG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG) {
        SerialPrintfln("HTML_info:   " ANSI_ESC_YELLOW "\"%s\"", info); // infos for debug
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  Events from DLNA
void dlna_info(const char* info) {
    if(endsWith(info, "is not responding after request")) { // timeout
        _f_dlnaBrowseServer = false;
        if(_dlnaLevel > 0) _dlnaLevel--;
        lst_DLNA.show(_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
        setTimeCounter(4);
    }
    SerialPrintfln("DLNA_info:    %s", info);
}

void dlna_server(uint8_t serverId, const char* IP_addr, uint16_t port, const char* friendlyName, const char* controlURL) {
    SerialPrintfln("DLNA_server: [%d] " ANSI_ESC_CYAN "%s:%d " ANSI_ESC_YELLOW " %s", serverId, IP_addr, port, friendlyName);
}

void dlna_seekReady(uint8_t numberOfServer) { SerialPrintfln("DLNA_server: %i media server found", numberOfServer); }

void dlna_browseResult(const char* objectId, const char* parentId, uint16_t childCount, const char* title, bool isAudio, uint32_t itemSize, const char* duration, const char* itemURL) {
    SerialPrintfln("DLNA_server: " ANSI_ESC_YELLOW "title %s, childCount %d, itemSize %ld, duration %s", title, childCount, (long unsigned int)itemSize, duration);
}

void dlna_browseReady(uint16_t numberReturned, uint16_t totalMatches) {
    SerialPrintfln("DLNA_server: returned %i from %i", numberReturned + _totalNumberReturned, totalMatches);
    _dlnaMaxItems = totalMatches;
    _totalNumberReturned += numberReturned;
    if(numberReturned == 50 && !_f_dlnaMakePlaylistOTF) { // next round
        if(_totalNumberReturned < totalMatches && _totalNumberReturned < 500) { _f_dlnaBrowseServer = true; }
    }
    if(_f_dlnaWaitForResponse) {
        _f_dlnaWaitForResponse = false;
        lst_DLNA.show(_dlnaItemNr, dlna.getServer(), dlna.getBrowseResult(), &_dlnaLevel, _dlnaMaxItems);
        setTimeCounter(4);
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
        bt_emitter.userCommand("AT+GMR?");                      // get version
        bt_emitter.userCommand("AT+VOL?");                      // get volume (in receiver mode 0 ... 31)
        bt_emitter.userCommand("AT+BT_MODE?");                  // transmitter or receiver
        if(!_f_BTpower) bt_emitter.userCommand("AT+POWER_OFF"); // forced by user
    }

    if(startsWith(info, "Volume")){
        char c[10]; sprintf(c, "Vol: %s", val); txt_BT_volume.writeText(c, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        if(_BTvolume != atoi(val)) bt_emitter.setVolume(_BTvolume);
    }
    if(startsWith(info, "Mode")){
        txt_BT_mode.writeText(val, TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
    }
    if(startsWith(info, "POWER OFF")){
        _f_BTcurPowerState = false;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
        webSrv.send("KCX_BT_power=", "0");
        if(_state == BLUETOOTH) {
            btn_BT_power.setValue(false);
            pic_BT_mode.setPicturePath("/common/BToff.jpg");
            pic_BT_mode.show();
        }
        webSrv.send("KCX_BT_connected=", "-1");
    }
    if(startsWith(info, "POWER ON")) {
        _f_BTcurPowerState = true;
        SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
        webSrv.send("KCX_BT_power=", "1");
        if(_state == BLUETOOTH) {
            btn_BT_power.setValue(true);
            pic_BT_mode.setPicturePath("/common/BTnc.jpg");
            pic_BT_mode.show();
        }
        return;
    }
    if(_f_BTcurPowerState) SerialPrintfln("BT-Emitter:  %s " ANSI_ESC_YELLOW "%s", info, val);
}

void kcx_bt_status(bool status) { // is always called when the status changes from disconnected to connected and vice versa

    if(status) {
        if(!_f_BTcurPowerState) return;
        const char* mode = bt_emitter.getMode();
        webSrv.send("KCX_BT_connected=", "1");
        if(strcmp(mode, "TX") == 0) pic_BT_mode.setPicturePath("/common/BTgold.jpg");
        else                      { pic_BT_mode.setPicturePath("/common/BTblue.jpg"); muteChanged(true);}
    }
    else {
        webSrv.send("KCX_BT_connected=", "0");
        pic_BT_mode.setPicturePath("/common/BTnc.jpg"); // not connected
    }
    if(_state == BLUETOOTH) pic_BT_mode.show();
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
    if(strcmp(name, "sdr_BR_value") == 0)  {_brightness = arg1; setTFTbrightness(arg1); txt_BR_value.writeText(int2str(arg1), TFT_ALIGN_CENTER, TFT_ALIGN_CENTER); return;}
    if(strcmp(name, "sdr_E_LP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_lowPass.writeText(c);  _toneLP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_bandPass.writeText(c); _toneBP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_HP") == 0)  {itoa(arg1, c, 10); strcat(c, " dB"); txt_EQ_highPass.writeText(c); _toneHP = arg1;  webSrv.send("settone=", setI2STone()); return;}
    if(strcmp(name, "sdr_E_BAL") == 0) {itoa(arg1, c, 10); strcat(c, " ");   txt_EQ_balance.writeText(c);  _toneBAL = arg1; webSrv.send("settone=", setI2STone()); return;}

    log_d("unused event: graphicObject %s was changed, val %li", name, arg1);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnClick(const char* name, uint8_t val) { // val = 0 --> is inactive
    // all state
    if(val == 3 && strcmp(name, "dispFooter") == 0)     {setTimeCounter(0); return;} // pos 3 is RSSI or TC

    if(_state == RADIO) {
        if( val && strcmp(name, "btn_RA_Mute") == 0)    {setTimeCounter(2); {if(!_f_mute) _f_muteIsPressed = true;} return;}
        if( val && strcmp(name, "btn_RA_prevSta") == 0) {setTimeCounter(2); return;}
        if( val && strcmp(name, "btn_RA_nextSta") == 0) {setTimeCounter(2); return;}
        if( val && strcmp(name, "btn_RA_staList") == 0) {return;}
        if( val && strcmp(name, "btn_RA_player") == 0)  {return;}
        if( val && strcmp(name, "btn_RA_dlna") == 0)    {return;}
        if( val && strcmp(name, "btn_RA_clock") == 0)   {return;}
        if( val && strcmp(name, "btn_RA_sleep") == 0)   {return;}
        if( val && strcmp(name, "btn_RA_bright") == 0)  {return;}
        if(!val && strcmp(name, "btn_RA_bright") == 0)  {setTimeCounter(2); return;}
        if( val && strcmp(name, "btn_RA_equal") == 0)   {return;}
        if( val && strcmp(name, "btn_RA_bt") == 0)      {return;}
        if(!val && strcmp(name, "btn_RA_bt") == 0)      {setTimeCounter(2); return;}
        if( val && strcmp(name, "btn_RA_off") == 0)     {return;}
        if( val && strcmp(name, "VUmeter_RA") == 0)     {return;}
    }
    if(_state == STATIONSLIST) {
        if( val && strcmp(name, "lst_RADIO") == 0)      {setTimeCounter(4); return;}
    }
    if(_state == PLAYER) {
        if( val && strcmp(name, "btn_PL_Mute") == 0)    {{if(!_f_mute) _f_muteIsPressed = true;} return;}
        if( val && strcmp(name, "btn_PL_pause") == 0)   {return;}
        if( val && strcmp(name, "btn_PL_cancel") == 0)  {return;}
        if( val && strcmp(name, "btn_PL_prevFile") == 0){if(_cur_AudioFileNr > 0) {_cur_AudioFileNr--; showFileName(_SD_content.getIndex(_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && strcmp(name, "btn_PL_nextFile") == 0){if(_cur_AudioFileNr + 1 < _SD_content.getSize()) {_cur_AudioFileNr++; showFileName(_SD_content.getIndex(_cur_AudioFileNr)); showAudioFileNumber();} return;}
        if( val && strcmp(name, "btn_PL_ready") == 0)   {return;}
        if( val && strcmp(name, "btn_PL_playAll") == 0) {return;}
        if( val && strcmp(name, "btn_PL_shuffle") == 0) {return;}
        if( val && strcmp(name, "btn_PL_fileList") == 0){return;}
        if( val && strcmp(name, "btn_PL_radio") == 0)   {return;}
        if( val && strcmp(name, "btn_PL_off") == 0)     {return;}
    }
    if(_state == AUDIOFILESLIST) {
        if( val && strcmp(name, "lst_PLAYER") == 0)     {setTimeCounter(4); return;}
    }
    if(_state == DLNA) {
        if( val && strcmp(name, "btn_DL_Mute") == 0)    {{if(!_f_mute) _f_muteIsPressed = true;} return;}
        if( val && strcmp(name, "btn_DL_pause") == 0)   {return;}
        if( val && strcmp(name, "btn_DL_radio") == 0)   {return;}
        if( val && strcmp(name, "btn_DL_fileList") == 0){return;}
        if( val && strcmp(name, "btn_DL_cancel") == 0)  {clearStationName(); btn_DL_pause.setInactive(); return;}
    }
    if(_state == DLNAITEMSLIST) {
        if( val && strcmp(name, "lst_DLNA") == 0)       {setTimeCounter(15); _f_dlnaWaitForResponse = true; return;}
    }
    if(_state == CLOCK) {
        if( val && strcmp(name, "btn_CL_Mute") == 0)    {setTimeCounter(2); if(!_f_mute){ _f_muteIsPressed = true;} return;}
        if( val && strcmp(name, "btn_CL_alarm") == 0)   {return;}
        if( val && strcmp(name, "btn_CL_radio") == 0)   {return;}
        if( val && strcmp(name, "clk_CL_green") == 0)   {return;}
        if( val && strcmp(name, "btn_CL_off") == 0)     {return;}
    }
    if(_state == ALARM) {
        if( val && strcmp(name, "clk_AL_red") == 0)     {return;}
        if( val && strcmp(name, "btn_AL_left") == 0)    {return;}
        if( val && strcmp(name, "btn_AL_right") == 0)   {return;}
        if( val && strcmp(name, "btn_AL_up") == 0)      {return;}
        if( val && strcmp(name, "btn_AL_down") == 0)    {return;}
        if( val && strcmp(name, "btn_AL_ready") == 0)   {return;}
    }
    if(_state == SLEEP) {
        if( val && strcmp(name, "btn_SL_up") == 0)      {return;}
        if( val && strcmp(name, "btn_SL_down") == 0)    {return;}
        if( val && strcmp(name, "btn_SL_ready") == 0)   {return;}
        if( val && strcmp(name, "btn_SL_cancel") == 0)  {return;}
    }
    if(_state == BRIGHTNESS){
        if( val && strcmp(name, "btn_BR_ready") == 0)   {return;}
        if( val && strcmp(name, "pic_BR_logo") == 0)    {return;}
    }
    if(_state == EQUALIZER) {
        if( val && strcmp(name, "btn_E_LP") == 0)       {sdr_EQ_lowPass.setValue(0);  return;}
        if( val && strcmp(name, "btn_E_BP") == 0)       {sdr_EQ_bandPass.setValue(0); return;}
        if( val && strcmp(name, "btn_E_HP") == 0)       {sdr_EQ_highPass.setValue(0); return;}
        if( val && strcmp(name, "btn_E_BAL") == 0)      {sdr_EQ_balance.setValue(0);  return;}
        if( val && strcmp(name, "btn_EQ_Radio") == 0)   {return;}
        if( val && strcmp(name, "btn_EQ_Player") == 0)  {return;}
        if( val && strcmp(name, "btn_EQ_Mute") == 0)    {{if(!_f_mute) _f_muteIsPressed = true;} return;}
    }
    if(_state == BLUETOOTH) {
        if( val && strcmp(name, "btn_BT_pause") == 0)   {bt_emitter.pauseResume(); return;}
        if( val && strcmp(name, "btn_BT_radio") == 0)   {return;}
        if( val && strcmp(name, "btn_BT_volDown") == 0) {if(_BTvolume > 0)  {_BTvolume--; bt_emitter.downvolume();} return;}
        if( val && strcmp(name, "btn_BT_volUp") == 0)   {if(_BTvolume < 31) {_BTvolume++; bt_emitter.upvolume();}  return;}
        if( val && strcmp(name, "btn_BT_mode") == 0)    {bt_emitter.changeMode(); return;}
        if( val && strcmp(name, "btn_BT_power") == 0)    {return;}
    }
    log_d("unused event: graphicObject %s was clicked", name);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void graphicObjects_OnRelease(const char* name, releasedArg ra) {

    // all state
        if(strcmp(name, "dispFooter") == 0)      {return;}

    if(_state == RADIO) {
        if(strcmp(name, "btn_RA_Mute") == 0)     {muteChanged(btn_RA_Mute.getValue()); return;}
        if(strcmp(name, "btn_RA_prevSta") == 0)  {prevStation(); dispFooter.updateStation(_cur_station); return;}
        if(strcmp(name, "btn_RA_nextSta") == 0)  {nextStation(); dispFooter.updateStation(_cur_station); return;}
        if(strcmp(name, "btn_RA_staList") == 0)  {_radioSubmenue = 0; changeState(STATIONSLIST); return;}
        if(strcmp(name, "btn_RA_player") == 0)   {_radioSubmenue = 0; stopSong(); changeState(PLAYER); return;}
        if(strcmp(name, "btn_RA_dlna") == 0)     {_radioSubmenue = 0; stopSong(); changeState(DLNA); return;}
        if(strcmp(name, "btn_RA_clock") == 0)    {_radioSubmenue = 0; changeState(CLOCK); return;}
        if(strcmp(name, "btn_RA_sleep") == 0)    {_radioSubmenue = 0; changeState(SLEEP); return;}
        if(strcmp(name, "btn_RA_bright") == 0)   {_radioSubmenue = 0; changeState(BRIGHTNESS); return;}
        if(strcmp(name, "btn_RA_equal") == 0)    {_radioSubmenue = 0; changeState(EQUALIZER); return;}
        if(strcmp(name, "btn_RA_equal") == 0)    {_radioSubmenue = 0; changeState(EQUALIZER); return;}
        if(strcmp(name, "btn_RA_bt") == 0)       {_radioSubmenue = 0; changeState(BLUETOOTH); return;}
        if(strcmp(name, "btn_RA_off") == 0)      {fall_asleep(); return;}
        if(strcmp(name, "VUmeter_RA") == 0)      {return;}
    }
    if(_state == STATIONSLIST) {
        if(strcmp(name, "lst_RADIO") == 0)       {if(ra.val1){_radioSubmenue = 0; setStation(ra.val1); changeState(RADIO);} return;}
    }
    if(_state == PLAYER) {
        if(strcmp(name, "btn_PL_Mute") == 0)     {muteChanged(btn_PL_Mute.getValue()); return;}
        if(strcmp(name, "btn_PL_pause") == 0)    {if(_f_isFSConnected) {audioPauseResume();} return;}
        if(strcmp(name, "btn_PL_cancel") == 0)   {_playerSubmenue = 0; stopSong(); changeState(PLAYER); return;}
        if(strcmp(name, "btn_PL_prevFile") == 0) {return;}
        if(strcmp(name, "btn_PL_nextFile") == 0) {return;}
        if(strcmp(name, "btn_PL_ready") == 0)    {_playerSubmenue = 1; SD_playFile(_curAudioFolder, _SD_content.getIndex(_cur_AudioFileNr)); changeState(PLAYER); showAudioFileNumber(); return;}
        if(strcmp(name, "btn_PL_playAll") == 0)  {_playerSubmenue = 1; _f_shuffle = false; preparePlaylistFromSDFolder(_curAudioFolder); processPlaylist(true); changeState(PLAYER); return;}
        if(strcmp(name, "btn_PL_shuffle") == 0)  {_playerSubmenue = 1; _f_shuffle = true; preparePlaylistFromSDFolder(_curAudioFolder); processPlaylist(true); changeState(PLAYER); return;}
        if(strcmp(name, "btn_PL_fileList") == 0) {_playerSubmenue = 1; _SD_content.listDir(_curAudioFolder, true, false); changeState(AUDIOFILESLIST); return;}
        if(strcmp(name, "btn_PL_radio") == 0)    {_playerSubmenue = 0; setStation(_cur_station); changeState(RADIO); return;}
        if(strcmp(name, "btn_PL_off") == 0)      {fall_asleep(); return;}
    }
    if(_state == AUDIOFILESLIST){
        if(strcmp(name, "lst_PLAYER") == 0)      {if(ra.val1 == 1){lst_PLAYER.show();} if(ra.val1 == 2){SD_playFile(ra.arg1);} return;}
    }
    if(_state == DLNA) {
        if(strcmp(name, "btn_DL_Mute") == 0)     {muteChanged(btn_DL_Mute.getValue()); return;}
        if(strcmp(name, "btn_DL_pause") == 0)    {audioPauseResume(); return;}
        if(strcmp(name, "btn_DL_radio") == 0)    {setStation(_cur_station); txt_DL_fName.setText(""); changeState(RADIO); return;}
        if(strcmp(name, "btn_DL_fileList") == 0) {changeState(DLNAITEMSLIST); txt_DL_fName.setText(""); return;}
        if(strcmp(name, "btn_DL_cancel") == 0)   {stopSong(); txt_DL_fName.setText(""); return;}
    }
    if(_state == DLNAITEMSLIST) {
        if(strcmp(name, "lst_DLNA") == 0)        {if(ra.val1 == 1){txt_DL_fName.setTextColor(TFT_CYAN); txt_DL_fName.setText(ra.arg2, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER); changeState(DLNA); connecttohost(ra.arg1);} // play a file
                                                  if(ra.val1 == 2){dlna.browseServer(ra.val2, ra.arg1, 0, 50); _f_dlnaMakePlaylistOTF = true; } // browse dlna object, waiting for content and create a playlist
                                                  return;}
    }
    if(_state == CLOCK) {
        if(strcmp(name, "btn_CL_Mute") == 0)     {muteChanged(btn_CL_Mute.getValue()); return;}
        if(strcmp(name, "btn_CL_alarm") == 0)    {changeState(ALARM); return;}
        if(strcmp(name, "btn_CL_radio") == 0)    {_clockSubMenue = 0; changeState(RADIO); return;}
        if(strcmp(name, "clk_CL_green") == 0)    {_clockSubMenue = 1; changeState(CLOCK); return;}
        if(strcmp(name, "btn_CL_off") == 0)      {fall_asleep(); return;}
    }
    if(_state == ALARM) {
        if(strcmp(name, "clk_AL_red") == 0)      {return;}
        if(strcmp(name, "btn_AL_left") == 0)     {clk_AL_red.shiftLeft(); return;}
        if(strcmp(name, "btn_AL_right") == 0)    {clk_AL_red.shiftRight(); return;}
        if(strcmp(name, "btn_AL_up") == 0)       {clk_AL_red.digitUp(); return;}
        if(strcmp(name, "btn_AL_down") == 0)     {clk_AL_red.digitDown(); return;}
        if(strcmp(name, "btn_AL_ready") == 0)    {updateSettings(); _clockSubMenue = 0; changeState(CLOCK); logAlarmItems(); return;}
    }
    if(_state == SLEEP) {
        if(strcmp(name, "btn_SL_up") == 0)       {display_sleeptime(1); return;}
        if(strcmp(name, "btn_SL_down") == 0)     {display_sleeptime(-1); return;}
        if(strcmp(name, "btn_SL_ready") == 0)    {dispFooter.updateOffTime(_sleeptime);  changeState(RADIO); return;}
        if(strcmp(name, "btn_SL_cancel") == 0)   {changeState(RADIO); return;}
    }
    if(_state == BRIGHTNESS){
        if(strcmp(name, "btn_BR_ready") == 0)    {changeState(RADIO); return;}
        if(strcmp(name, "pic_BR_logo") == 0)     {return;}
    }
    if(_state == EQUALIZER) {
        if(strcmp(name, "btn_EQ_Radio") == 0)    {setStation(_cur_station); changeState(RADIO); return;}
        if(strcmp(name, "btn_EQ_Player") == 0)   {changeState(PLAYER); return;}
        if(strcmp(name, "btn_EQ_Mute") == 0)     {muteChanged(btn_EQ_Mute.getValue()); return;}
    }
    if(_state == BLUETOOTH) {
        if(strcmp(name, "btn_BT_pause") == 0)    {return;}
        if(strcmp(name, "btn_BT_radio") == 0)    {changeState(RADIO); return;}
        if(strcmp(name, "btn_BT_volDown") == 0)  {return;}
        if(strcmp(name, "btn_BT_volUp") == 0)    {return;}
        if(strcmp(name, "btn_BT_mode") == 0)     {return;}
        if(strcmp(name, "btn_BT_power") == 0)    {_f_BTpower = !_f_BTcurPowerState; BTpowerChanged(!_f_BTcurPowerState); return;}
    }
    log_d("unused event: graphicObject %s was released", name);
}
// clang-format on
