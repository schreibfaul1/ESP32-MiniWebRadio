/***********************************************************************************************************************
    MiniWebRadio -- Webradio receiver for ESP32

    first release on 03/2017
    Version 2.2h, Apr 10/2022

    2.8" color display (320x240px) with controller ILI9341 or HX8347D (SPI) or
    3.5" color display (480x320px) wiht controller ILI9486 or ILI9488 (SPI)

    HW decoder VS1053 or
    SW decoder with external DAC over I2S

    SD_MMC is mandatory
    IR remote is optional

***********************************************************************************************************************/

// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE

#include "common.h"

//global variables
const uint8_t  _max_volume     = 21;
const uint16_t _max_stations   = 1000;
uint8_t        _alarmdays      = 0;
uint16_t       _cur_station    = 0;      // current station(nr), will be set later
uint16_t       _sleeptime      = 0;      // time in min until MiniWebRadio goes to sleep
uint16_t       _sum_stations   = 0;
uint8_t        _cur_volume     = 0;      // will be set from stored preferences
uint8_t        _mute_volume    = 0;      // decrement to 0 or increment to _cur_volume
uint8_t        _state          = 0;      // statemaschine
uint8_t        _timeCounter       = 0;
uint8_t        _commercial_dur = 0;      // duration of advertising
uint16_t       _alarmtime      = 0;      // in minutes (23:59 = 23 *60 + 59)
int8_t         _releaseNr      = -1;
uint32_t       _resumeFilePos  = 0;
char           _chbuf[512];
char           _myIP[25];
char           _afn[256];                // audioFileName
char           _path[128];
char           _prefix[5]      = "/s";
char*          _lastconnectedfile = nullptr;
char*          _lastconnectedhost = nullptr;
char*          _stationURL = nullptr;
const char*    _pressBtn[5];
const char*    _releaseBtn[5];
boolean        _f_rtc  = false;             // true if time from ntp is received
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
boolean        _f_switchToClock = false;  // jump into CLOCK mode at the next opportunity
boolean        _f_hpChanged = false; // true, if HeadPhone is plugged or unplugged
boolean        _f_muteIncrement = false; // if set increase Volume (from 0 to _cur_volume)
boolean        _f_muteDecrement = false; // if set decrease Volume (from _cur_volume to 0)

String         _station = "";
String         _stationName_nvs = "";
String         _stationName_air = "";
String         _homepage = "";
String         _streamTitle = "";
String         _filename = "";
String         _icydescription = "";

char _hl_item[10][40]{                          // Title in headline
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


Preferences pref;
Preferences stations;
WebSrv webSrv;
WiFiMulti wifiMulti;
RTIME rtc;
Ticker ticker;
IR ir(IR_PIN);                  // do not change the objectname, it must be "ir"
TP tp(TP_CS, TP_IRQ);
File audioFile;
FtpServer ftpSrv;
#if DECODER == 2 // ac101
    AC101 dac;
#endif
#if DECODER == 3 // es8388
    ES8388 dac;
#endif
#if DECODER == 4 // wm8978
    WM8978 dac;
#endif

SemaphoreHandle_t  mutex_rtc;
SemaphoreHandle_t  mutex_display;


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
    const unsigned short* _fonts[7] = {
        Times_New_Roman15x14,
        Times_New_Roman21x17,
        Times_New_Roman27x21,
        Times_New_Roman34x27,
        Times_New_Roman38x31,
        Times_New_Roman43x35,
        Big_Numbers133x156      // ASCII 32...64 only
    };

    struct w_h {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 320; uint16_t h = 20; } const _winHeader;
    struct w_l {uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 100; uint16_t h = 100;} const _winLogo;
    struct w_n {uint16_t x = 100; uint16_t y = 20;  uint16_t w = 220; uint16_t h = 100;} const _winName;
    struct w_e {uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 320; uint16_t h = 100;} const _winFName;
    struct w_t {uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;} const _winTitle;
    struct w_f {uint16_t x = 0;   uint16_t y = 220; uint16_t w = 320; uint16_t h = 20; } const _winFooter;
    struct w_i {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 180; uint16_t h = 20; } const _winItem;
    struct w_v {uint16_t x = 180; uint16_t y = 0;   uint16_t w =  50; uint16_t h = 20; } const _winVolume;
    struct w_m {uint16_t x = 260; uint16_t y = 0;   uint16_t w =  60; uint16_t h = 20; } const _winTime;
    struct w_s {uint16_t x = 0;   uint16_t y = 220; uint16_t w =  60; uint16_t h = 20; } const _winStaNr;
    struct w_p {uint16_t x = 60;  uint16_t y = 220; uint16_t w = 120; uint16_t h = 20; } const _winSleep;
    struct w_a {uint16_t x = 180; uint16_t y = 220; uint16_t w = 160; uint16_t h = 20; } const _winIPaddr;
    struct w_b {uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 14; } const _winVolBar;
    struct w_o {uint16_t x = 0;   uint16_t y = 154; uint16_t w =  64; uint16_t h = 64; } const _winButton;
    uint16_t _alarmdaysXPos[7] = {3, 48, 93, 138, 183, 228, 273};
    uint16_t _alarmtimeXPos[5] = {2, 75, 173, 246, 148}; // last is colon
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
#endif //TFT_CONTROLLER == 0 || TFT_CONTROLLER == 1


#if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3
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
        Times_New_Roman27x21,
        Times_New_Roman34x27,
        Times_New_Roman38x31,
        Times_New_Roman43x35,
        Times_New_Roman56x46,
        Times_New_Roman66x53,
        Big_Numbers133x156      // ASCII 32...64 only
    };

    struct w_h {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 480; uint16_t h = 30; } const _winHeader;
    struct w_l {uint16_t x = 0;   uint16_t y = 30;  uint16_t w = 130; uint16_t h = 130;} const _winLogo;
    struct w_n {uint16_t x = 130; uint16_t y = 30;  uint16_t w = 350; uint16_t h = 130;} const _winName;
    struct w_e {uint16_t x = 0;   uint16_t y = 30;  uint16_t w = 480; uint16_t h = 130;} const _winFName;
    struct w_t {uint16_t x = 0;   uint16_t y = 160; uint16_t w = 480; uint16_t h = 130;} const _winTitle;
    struct w_f {uint16_t x = 0;   uint16_t y = 290; uint16_t w = 480; uint16_t h = 30; } const _winFooter;
    struct w_m {uint16_t x = 390; uint16_t y = 0;   uint16_t w =  90; uint16_t h = 30; } const _winTime;
    struct w_i {uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 280; uint16_t h = 30; } const _winItem;
    struct w_v {uint16_t x = 280; uint16_t y = 0;   uint16_t w = 110; uint16_t h = 30; } const _winVolume;
    struct w_a {uint16_t x = 260; uint16_t y = 290; uint16_t w = 220; uint16_t h = 30; } const _winIPaddr;
    struct w_s {uint16_t x = 0;   uint16_t y = 290; uint16_t w = 100; uint16_t h = 30; } const _winStaNr;
    struct w_p {uint16_t x = 100; uint16_t y = 290; uint16_t w = 160; uint16_t h = 30; } const _winSleep;
    struct w_b {uint16_t x = 0;   uint16_t y = 160; uint16_t w = 480; uint16_t h = 30; } const _winVolBar;
    struct w_o {uint16_t x = 0;   uint16_t y = 190; uint16_t w =  96; uint16_t h = 96; } const _winButton;
    uint16_t _alarmdaysXPos[7] = {2, 70, 138, 206, 274, 342, 410};
    uint16_t _alarmtimeXPos[5] = {12, 118, 266, 372, 224}; // last is colon
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
#endif  // #if TFT_CONTROLLER == 2 || TFT_CONTROLLER == 3



/***********************************************************************************************************************
*                                        D E F A U L T S E T T I N G S                                                 *
***********************************************************************************************************************/
boolean defaultsettings(){
    if(pref.getUInt("default", 0) != 1000){
        SerialPrintfln("first init, set defaults");
		if(!saveStationsToNVS()) return false;
        pref.clear();
        //
        pref.putUShort("alarm_weekday",0); // for alarmclock
        pref.putUInt("alarm_time", 0);
        pref.putUShort("ringvolume",21);
        //
        pref.putUShort("volume",12); // 0...21
        pref.putUShort("mute",   0); // no mute

        pref.putUShort("brightness", 100); // 0...100
        pref.putUInt("sleeptime", 0);

        pref.putUShort("toneha", 0); // BassFreq 0...15        VS1053
        pref.putUShort("tonehf", 0); // TrebleGain 0...14      VS1053
        pref.putUShort("tonela", 0); // BassGain 0...15        VS1053
        pref.putUShort("tonelf", 0); // BassFreq 0...13        VS1053

        pref.putShort("toneLP", 0); // -40 ... +6 (dB)         audioI2S
        pref.putShort("toneBP", 0); // -40 ... +6 (dB)         audioI2S
        pref.putShort("toneHP", 0); // -40 ... +6 (dB)         audioI2S
        //
        pref.putUInt("station", 1);
        //
        pref.putUInt("default", 1000);
    }
	return true;
}

boolean saveStationsToNVS(){
    String X="", Cy="", StationName="", StreamURL="", currentLine="", tmp="";
    uint16_t cnt = 0;
    // StationList
	if(!SD_MMC.exists("/stations.csv")){
		SerialPrintfln(ANSI_ESC_RED "SD_MMC/stations.csv not found");
		return false;
	}

    File file = SD_MMC.open("/stations.csv");
    if(file){  // try to read from SD_MMC
        stations.clear();
        currentLine = file.readStringUntil('\n');             // read the headline
        while(file.available()){
            currentLine = file.readStringUntil('\n');         // read the line
            uint p = 0, q = 0;
            X=""; Cy=""; StationName=""; StreamURL="";
            for(int i = 0; i < currentLine.length() + 1; i++){
                if(currentLine[i] == '\t' || i == currentLine.length()){
                    if(p == 0) X            = currentLine.substring(q, i);
                    if(p == 1) Cy           = currentLine.substring(q, i);
                    if(p == 2) StationName  = currentLine.substring(q, i);
                    if(p == 3) StreamURL    = currentLine.substring(q, i);
                    p++;
                    i++;
                    q = i;
                }
            }
            if(X == "*") continue;
            if(StationName == "") continue; // is empty
            if(StreamURL   == "") continue; // is empty
            //SerialPrintfln("Cy=%s, StationName=%s, StreamURL=%s",Cy.c_str(), StationName.c_str(), StreamURL.c_str());
            cnt++;
            if(cnt ==_max_stations){
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
        stations.putUInt("sumstations", cnt);
        SerialPrintfln("stationlist internally loaded");
        SerialPrintfln("number of stations: " ANSI_ESC_CYAN "%i", cnt);
        return true;
    }
    else return false;
}
/***********************************************************************************************************************
*                                        T F T   B R I G H T N E S S                                                   *
***********************************************************************************************************************/
void setTFTbrightness(uint8_t duty){ //duty 0...100 (min...max)
    if(TFT_BL == -1) return;
    ledcAttachPin(TFT_BL, 1);        //Configure variable led, TFT_BL pin to channel 1
    ledcSetup(1, 12000, 8);          // 12 kHz PWM and 8 bit resolution
    ledcWrite(1, duty * 2.55);
}
inline uint32_t getTFTbrightness(){
    return ledcRead(1);
}
inline uint8_t downBrightness(){
    uint8_t br; br = pref.getUShort("brightness");
    if(br>5) {
        br-=5;
        pref.putUShort("brightness", br);
        setTFTbrightness(br);
        showBrightnessBar();
    } return br;
}
inline uint8_t upBrightness(){
    uint8_t br; br = pref.getUShort("brightness");
    if(br < 100){
        br += 5;
        pref.putUShort("brightness", br);
        setTFTbrightness(br);
        showBrightnessBar();
    }
    return br;
}
inline uint8_t getBrightness(){
    return pref.getUShort("brightness");
}
/***********************************************************************************************************************
*                                                     A S C I I                                                        *
***********************************************************************************************************************/
const char* UTF8toASCII(const char* str){
    uint16_t i = 0, j = 0;
    char tab[96] = {
          96, 173, 155, 156,  32, 157,  32,  32,  32,  32, 166, 174, 170,  32,  32,  32, 248, 241, 253,  32,
          32, 230,  32, 250,  32,  32, 167, 175, 172, 171,  32, 168,  32,  32,  32,  32, 142, 143, 146, 128,
          32, 144,  32,  32,  32,  32,  32,  32,  32, 165,  32,  32,  32,  32, 153,  32,  32,  32,  32,  32,
         154,  32,  32, 225, 133, 160, 131,  32, 132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139,
          32, 164, 149, 162, 147,  32, 148, 246,  32, 151, 163, 150, 129,  32,  32, 152
       };
      while((str[i] != 0) && (j < 1020)){
        _chbuf[j] = str[i];
        if(str[i] == 0xC2){ // compute unicode from utf8
            i++;
            if((str[i] > 159) && (str[i] < 192)) _chbuf[j] = tab[str[i] - 160];
            else _chbuf[j] = 32;
        }
        else if(str[i] == 0xC3){
            i++;
            if((str[i] > 127) && (str[i] < 192)) _chbuf[j] = tab[str[i] - 96];
            else _chbuf[j] = 32;
        }
        i++; j++;
    }
    _chbuf[j] = 0;
    return (_chbuf);
}
const char* ASCIItoUTF8(const char* str){
    uint16_t i = 0, j = 0, uni = 0;
    uint16_t tab[128] = {
         199,  252,  233,  226,  228,  224,  229,  231,  234,  235,  232,  239,  238,  236,  196,  197,
         201,  230,  198,  244,  246,  242,  251,  249,  255,  214,  220,  162,  163,  165, 8359,  402,
         225,  237,  243,  250,  241,  209,  170,  186,  191, 8976,  172,  189,  188,  161,  171,  187,
        9617, 9618, 9619, 9474, 9508, 9569, 9570, 9558, 9557, 9571, 9553, 9559, 9565, 9564, 9563, 9488,
        9492, 9524, 9516, 9500, 9472, 9532, 9566, 9567, 9562, 9556, 9577, 9574, 9568, 9552, 9580, 9575,
        9576, 9572, 9573, 9561, 9560, 9554, 9555, 9579, 9578, 9496, 9484, 9608, 9604, 9612, 9616, 9600,
         945,  223,  915,  960,  931,  963,  181,  964,  934,  920,  937,  948, 8734,  966,  949, 8745,
        8801,  177, 8805, 8804, 8992, 8993,  247, 8776,  176, 8729,  183, 8730, 8319,  178, 9632,  160
    };
    while((str[i] != 0) && (j < 1020)){
        uni = str[i];
        if(uni >= 128){uni -= 128; uni = tab[uni];}
//            uni=UTF8fromASCII(str[i]);
            switch(uni){
                case   0 ... 127:{_chbuf[j] = str[i]; i++; j++; break;}
                case 160 ... 191:{_chbuf[j] = 0xC2; _chbuf[j+1] = uni; j += 2; i++; break;}
                case 192 ... 255:{_chbuf[j] = 0xC3; _chbuf[j+1] = uni - 64; j += 2; i++; break;}
                default:{_chbuf[j] = ' '; i++; j++; break;} // ignore all other
            }
    }
    _chbuf[j] = 0;
    return _chbuf;
}
/***********************************************************************************************************************
*                                                     T I M E R                                                        *
***********************************************************************************************************************/
void timer1sec() {
    static volatile uint8_t sec=0;
    _f_1sec = true;
    sec++;
    //SerialPrintfln("sec=%i", sec);
    if(sec==60){sec=0; _f_1min = true;}
}
/***********************************************************************************************************************
*                                                   D I S P L A Y                                                      *
***********************************************************************************************************************/
inline void clearHeader() {tft.fillRect(_winHeader.x, _winHeader.y, _winHeader.w, _winHeader.h, TFT_BLACK);}
inline void clearLogo()   {tft.fillRect(_winLogo.x,   _winLogo.y,   _winLogo.w,   _winLogo.h,   TFT_BLACK);}
inline void clearStation(){tft.fillRect(_winName.x,   _winName.y,   _winName.w,   _winName.h,   TFT_BLACK);}
inline void clearFName()  {tft.fillRect(_winFName.x,  _winFName.y,  _winFName.w,  _winFName.h,  TFT_BLACK);}
inline void clearTitle()  {tft.fillRect(_winTitle.x,  _winTitle.y,  _winTitle.w,  _winTitle.h,  TFT_BLACK);}
inline void clearFooter() {tft.fillRect(_winFooter.x, _winFooter.y, _winFooter.w, _winFooter.h, TFT_BLACK);}
inline void clearTime()   {tft.fillRect(_winTime.x,   _winTime.y,   _winTime.w,   _winTime.h,   TFT_BLACK);}
inline void clearItem()   {tft.fillRect(_winItem.x,   _winItem.y,   _winItem.w,   _winTime.h,   TFT_BLACK);}
inline void clearVolume() {tft.fillRect(_winVolume.x, _winVolume.y, _winVolume.w, _winVolume.h, TFT_BLACK);}
inline void clearIPaddr() {tft.fillRect(_winIPaddr.x, _winIPaddr.y, _winIPaddr.w, _winIPaddr.h, TFT_BLACK);}
inline void clearStaNr()  {tft.fillRect(_winStaNr.x,  _winStaNr.y,  _winStaNr.w,  _winStaNr.h,  TFT_BLACK);}
inline void clearSleep()  {tft.fillRect(_winSleep.x,  _winSleep.y,  _winSleep.w,  _winSleep.h,  TFT_BLACK);}
inline void clearVolBar() {tft.fillRect(_winVolBar.x, _winVolBar.y, _winVolBar.w, _winVolBar.h, TFT_BLACK);}
inline void clearAll()    {tft.fillScreen(TFT_BLACK);}                      // y   0...239

inline uint16_t txtlen(String str) {uint16_t len=0; for(int i=0; i<str.length(); i++) if(str[i]<=0xC2) len++; return len;}

void showHeadlineVolume(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_DEEPSKYBLUE);
    clearVolume();

    int vol = 0;
    if(_f_mute || _f_muteDecrement || _f_muteIncrement){
        vol = _mute_volume;
    }
    else{
        vol = _cur_volume;
    }

    sprintf(_chbuf, "Vol %02d", vol);
    tft.setCursor(_winVolume.x + 6, _winVolume.y + 2);
    tft.print(_chbuf);
    xSemaphoreGive(mutex_display);
}
void showHeadlineTime(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    clearTime();
    if(!_f_rtc) {xSemaphoreGive(mutex_display); return;} // has rtc the correct time? no -> return
    tft.setCursor(_winTime.x + 2, _winTime.y + 2);
    tft.print(rtc.gettime_s());
    xSemaphoreGive(mutex_display);
}
void showHeadlineItem(uint8_t idx){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    clearItem();
    tft.setCursor(_winItem.x + 6 , _winItem.y + 2);
    tft.print(_hl_item[idx]);
    xSemaphoreGive(mutex_display);
}
void showFooterIPaddr(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    char myIP[30] = "myIP:";
    strcpy(myIP + 5, _myIP);
    tft.setFont(_fonts[1]);
    tft.setTextColor(TFT_GREENYELLOW);
    clearIPaddr();
    tft.setCursor(_winIPaddr.x + 6 , _winIPaddr.y + 2);
    tft.print(myIP);
    xSemaphoreGive(mutex_display);
}
void showFooterStaNr(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    clearStaNr();
    tft.setFont(_fonts[1]);
    tft.setCursor(_winStaNr.x + 6 , _winStaNr.y + 2);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.print("STA:");
    tft.setTextColor(TFT_LAVENDER);
    tft.printf("%03d", _cur_station);
    xSemaphoreGive(mutex_display);
}
void updateSleepTime(boolean noDecrement){  // decrement and show new value in footer
    if(_f_sleeping) return;
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    boolean sleep = false;
    if(_sleeptime == 1) sleep = true;
    if(_sleeptime > 0 && !noDecrement) _sleeptime--;
    if(_state != ALARM){
        char Slt[15];
        sprintf(Slt,"S  %d:%02d", _sleeptime / 60, _sleeptime % 60);
        tft.setFont(_fonts[1]);
        if(!_sleeptime) tft.setTextColor(TFT_DEEPSKYBLUE);
        else tft.setTextColor(TFT_RED);
        clearSleep();
        tft.setCursor(_winSleep.x + 25 , _winSleep.y + 2);
        tft.print(Slt);
    }
    if(sleep){ // fall asleep
        audioStopSong();
        clearAll();
        setTFTbrightness(0);
        _f_sleeping = true;
        SerialPrintfln("falling asleep");
    }
    xSemaphoreGive(mutex_display);

}
void showVolumeBar(){
    uint16_t vol = tft.width() * getvolume()/21;
    clearVolBar();
    tft.fillRect(_winVolBar.x, _winVolBar.y + 14, vol, 8, TFT_RED);
    tft.fillRect(vol+1, _winVolBar.y + 14, tft.width()-vol+1, 8, TFT_GREEN);
    _f_volBarVisible = true;
}
void showBrightnessBar(){
    uint16_t vol = tft.width() * getBrightness()/100;
    clearVolBar();
    tft.fillRect(_winVolBar.x, _winVolBar.y + 14, vol, 8, TFT_RED);
    tft.fillRect(vol+1, _winVolBar.y + 14, tft.width()-vol+1, 8, TFT_GREEN);
    _f_volBarVisible = true;
}
void showFooter(){  // stationnumber, sleeptime, IPaddress
    showFooterStaNr();
    updateSleepTime();
    showFooterIPaddr();
}
void display_info(const char *str, int xPos, int yPos, uint16_t color, uint16_t indent, uint16_t winHeight){
    tft.fillRect(xPos, yPos, tft.width() - xPos, winHeight, TFT_BLACK);  // Clear the space for new info
    tft.setTextColor(color);                                // Set the requested color
    tft.setCursor(xPos + indent, yPos);                            // Prepare to show the info
    // SerialPrintfln("cursor x=%d, y=%d, winHeight=%d", xPos+indent, yPos, winHeight);
    uint16_t ch_written = tft.writeText((const uint8_t*) str, winHeight); // todo winHeight
    if(ch_written < strlen(str)){
        // If this message appears, there is not enough space on the display to write the entire text,
        // a part of the text has been cut off
        SerialPrintfln("txt overflow, winHeight=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE
                                      ", strlen=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE
                                     ", written=" ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE
                                         ", str=" ANSI_ESC_CYAN "%s", winHeight, strlen(str), ch_written, str);
    }
}
void showStreamTitle(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    String ST = _streamTitle;
    ST.trim();  // remove all leading or trailing whitespaces
    if(ST.length() == 0 && _icydescription.length() != 0) ST = _icydescription;
    webSrv.send("streamtitle=" + ST);
    ST.replace(" | ", "\n");   // some stations use pipe as \n or
    ST.replace("| ", "\n");    // or
    ST.replace("|", "\n");
    switch(ST.length()){
        case   0 ... 30:  tft.setFont(_fonts[5]); break;
        case  31 ... 45:  tft.setFont(_fonts[4]); break;
        case  46 ... 65:  tft.setFont(_fonts[3]); break;
        case  66 ... 130: tft.setFont(_fonts[2]); break;
        case 131 ... 200: tft.setFont(_fonts[1]); break;
        default:          tft.setFont(_fonts[0]); break;
    }
    display_info(ST.c_str(), _winTitle.x, _winTitle.y, TFT_CORNSILK, 5, _winTitle.h);
    xSemaphoreGive(mutex_display);
}
void showLogoAndStationName(){
    xSemaphoreTake(mutex_display, portMAX_DELAY);
    clearFName();
    String  SN_utf8 = "";
    String  SN_ascii = "";
    if(_cur_station){
        SN_utf8  = _stationName_nvs;
        SN_ascii = _stationName_nvs;
    }
    else{
        SN_utf8  = _stationName_air;
        SN_ascii = _stationName_air;
    }
    int16_t idx = SN_ascii.indexOf('|');
    if(idx>0){
        SN_ascii = SN_ascii.substring(idx + 1); // before pipe
        SN_utf8 = SN_utf8.substring(0, idx);
    }
    SN_ascii.trim();
    SN_utf8.trim();

    switch(SN_utf8.length()){
        case   0 ... 20:  tft.setFont(_fonts[5]); break;
        case  21 ... 32:  tft.setFont(_fonts[4]); break;
        case  33 ... 45:  tft.setFont(_fonts[3]); break;
        case  46 ... 60:  tft.setFont(_fonts[2]); break;
        case  61 ... 90:  tft.setFont(_fonts[1]); break;
        default:          tft.setFont(_fonts[0]); break;
    }
    display_info(SN_utf8.c_str(), _winName.x, _winName.y, TFT_CYAN, 10, _winName.h);

    String logo = "/logo/" + String(UTF8toASCII(SN_ascii.c_str())) +".jpg";
    if(drawImage(logo.c_str(), 0, _winName.y + 2) == false){
        drawImage("/common/unknown.jpg", 0, _winName.y + 2);  // if no draw unknown
    }
    xSemaphoreGive(mutex_display);
}

void showFileName(const char* fname){
    switch(strlen(fname)){
        case   0 ... 25:  tft.setFont(_fonts[5]); break;
        case  26 ... 42:  tft.setFont(_fonts[4]); break;
        case  43 ... 90:  tft.setFont(_fonts[3]); break;
        case  91 ... 120: tft.setFont(_fonts[2]); break;
        case 121 ... 150: tft.setFont(_fonts[1]); break;
        default:          tft.setFont(_fonts[0]); break;
    }
    display_info(fname, _winFName.x, _winFName.y, TFT_CYAN, 0, _winFName.h);
}

void display_time(boolean showall){ //show current time on the TFT Display
    static String t, oldt = "";
    static boolean k = false;
    uint8_t  i = 0, yOffset = 0;
    uint16_t x, y, space, imgHeigh, imgWidth_l, imgWidth_s;
    if(TFT_CONTROLLER < 2){
        x = 0;
        y = _winFName.y +33;
        yOffset = 8;
        space = 2;
        imgHeigh = 120;
        imgWidth_s = 24;
        imgWidth_l = 72;
    }
    else{
        x = 11;
        y = _winFName.y + 50;
        yOffset = 0;
        space = 10; // 10px between jpgs
        imgHeigh = 160;
        imgWidth_s = 32;
        imgWidth_l = 96;
    }
    if(showall == true) oldt = "";
    if((_state == CLOCK) || (_state == CLOCKico)){
        t = rtc.gettime_s();
        for(i = 0; i < 5; i++) {
            if(t[i] == ':') {if(k == false) {k = true; t[i] = 'd';} else{t[i] = 'e'; k = false;}}
            if(t[i] != oldt[i]) {
                if(TFT_CONTROLLER < 2){
                    sprintf(_chbuf,"/digits/%cgn.jpg",t[i]);
                }
                else{
                    sprintf(_chbuf,"/digits/%cgn.jpg",t[i]);
                }
                //log_i("drawImage %s, x=%i, y=%i", _chbuf, x, y);
                if(_state == CLOCKico) drawImage(_chbuf, x, _winFName.y);
                else drawImage(_chbuf, x, y + yOffset);
            }
            if((t[i]=='d')||(t[i]=='e'))x += imgWidth_s + space; else x += imgWidth_l + space;
        }
        oldt=t;
    }
}

void display_alarmDays(uint8_t ad, boolean showall){ // Sun ad=0, Mon ad=1, Tue ad=2 ....
    uint8_t i = 0;
    String str="";

    if(showall){
        clearHeader();
    }
    else{
        _alarmdays ^= (1 << ad);     // toggle bit
    }

    for(i=0;i<7;i++){
        str = "/day/" + String(i);
        if(_alarmdays & (1 << i))  str+="_rt_en.bmp";    // l<<i instead pow(2,i)
        else                       str+="_gr_en.bmp";
        drawImage(str.c_str(), _alarmdaysXPos[i], 0);
    }
}

void display_alarmtime(int8_t xy, int8_t ud, boolean showall){
    uint16_t j[4] = {5, 77, 173, 245};
    static int8_t pos, h, m;
    int8_t updatePos = -1, oldPos = -1;
    uint8_t corrY = 0;
    if(TFT_CONTROLLER < 2){
        corrY = 8;
    }
    else {
        corrY = 3;
    }

    if(showall){
        h = _alarmtime / 60;
        m = _alarmtime % 60;
    }

    if(ud == 1){
        if(pos == 0) if(((h / 10) == 1 && (h % 10) < 4) || ((h / 10) == 0))                {h += 10; updatePos = 0;}
        if(pos == 1) if(((h / 10) == 2 && (h % 10) < 3) || ((h / 10) < 2 && (h % 10) < 9)) {h++;     updatePos = 1;}
        if(pos == 2) if((m / 10) < 5) {m += 10; updatePos = 2;}
        if(pos == 3) if((m % 10) < 9) {m++;     updatePos = 3;}
        _alarmtime = h * 60 + m;
    }
    if(ud == -1){
        if(pos == 0) if((h / 10) > 0) {h -= 10; updatePos = 0;}
        if(pos == 1) if((h % 10) > 0) {h--;     updatePos = 1;}
        if(pos == 2) if((m / 10) > 0) {m -= 10; updatePos = 2;}
        if(pos == 3) if((m % 10) > 0) {m--;     updatePos = 3;}
        _alarmtime = h * 60 + m;
    }

    if(xy == 1) {
        oldPos = pos++;
        if(pos == 4)pos = 0;
        updatePos = pos; //pos 0...3 only
    }
    if(xy == -1){
        oldPos = pos--;
        if(pos ==-1) pos = 3;
        updatePos = pos;
    }

    char hhmm[15];
    sprintf(hhmm,"%d%d%d%d", h / 10, h %10, m /10, m %10);

    if(showall){
        drawImage("/digits/drt.jpg", _alarmtimeXPos[4], _alarmdays_h + corrY);
    }

    for(uint8_t i = 0; i < 4; i++){
        strcpy(_path, "/digits/");
        strncat(_path, (const char*) hhmm + i, 1);
        if(showall){
            if(i == pos) strcat(_path, "or.jpg");   //show orange number
            else         strcat(_path, "rt.jpg");   //show red numbers
            drawImage(_path, _alarmtimeXPos[i], _alarmdays_h + corrY);
        }

        else{
            if(i == updatePos){
                strcat(_path, "or.jpg");
                drawImage(_path, _alarmtimeXPos[i], _alarmdays_h + corrY);
            }
            if(i == oldPos){
                strcat(_path, "rt.jpg");
                drawImage(_path, _alarmtimeXPos[i], _alarmdays_h + corrY);
            }
        }
    }
}

void display_sleeptime(int8_t ud){  // set sleeptimer
    uint8_t xpos[4] = {5,54,71,120};

    if(ud == 1){
        switch(_sleeptime){
            case  0 ...  14:  _sleeptime = (_sleeptime /  5) *  5 +  5; break;
            case 15 ...  59:  _sleeptime = (_sleeptime / 15) * 15 + 15; break;
            case 60 ... 359:  _sleeptime = (_sleeptime / 60) * 60 + 60; break;
            default: _sleeptime = 360; break; // max 6 hours
        }
    }
    if(ud == -1){
        switch(_sleeptime){
            case  1 ...  15:  _sleeptime = ((_sleeptime - 1) /  5) *  5; break;
            case 16 ...  60:  _sleeptime = ((_sleeptime - 1) / 15) * 15; break;
            case 61 ... 360:  _sleeptime = ((_sleeptime - 1) / 60) * 60; break;
            default: _sleeptime = 0; break; // min
        }
    }
    char tmp[10];
    sprintf(tmp, "%d%02d", _sleeptime / 60, _sleeptime % 60);
    char path[128] = "/digits/";

    for(uint8_t i = 0; i < 4; i ++){
        strcpy(path, "/digits/");
        if(i == 3){
            if(!_sleeptime) strcat(path, "dsgn.jpg");
            else            strcat(path, "dsrt.jpg");
        }
        else{
            strncat(path, (tmp + i), 1);
            if(!_sleeptime) strcat(path, "sgn.jpg");
            else            strcat(path, "srt.jpg");
        }
        drawImage(path, _sleeptimeXPos[i], 48);
    }
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxWidth , uint16_t maxHeigth){
    const char* scImg = scaleImage(path);
    if(!SD_MMC.exists(scImg)){
        SerialPrintfln(ANSI_ESC_RED "file \"%s\" not found", scImg);
        return false;
    }
    if(endsWith(scImg, "bmp")){
        // log_i("drawImage %s, x=%i, y=%i, mayWidth=%i, maxHeight=%i", scImg, posX, posY, maxWidth, maxHeigth);
        return tft.drawBmpFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth);
    }
    if(endsWith(scImg, "jpg")){
        return tft.drawJpgFile(SD_MMC, scImg, posX, posY, maxWidth, maxHeigth);
    }
    return false; // neither jpg nor bmp
}
/***********************************************************************************************************************
*                                         L I S T A U D I O F I L E                                                    *
***********************************************************************************************************************/
bool setAudioFolder(const char* audioDir){
    if(audioFile) audioFile.close();  // same as rewind()
    if(!SD_MMC.exists(audioDir)){SerialPrintfln(ANSI_ESC_RED "%s not exist", audioDir); return false;}
    audioFile = SD_MMC.open(audioDir);
    if(!audioFile.isDirectory()){SerialPrintfln(ANSI_ESC_RED "%s is not a directory", audioDir); return false;}
    return true;
}
const char* listAudioFile(){
    File file = audioFile.openNextFile();
    if(!file) {
        // SerialPrintfln(ANSI_ESC_BLUE "no more files found");
        audioFile.close();
        return nullptr;
    }
    while(file){
        const char* name = file.name();
        if(endsWith(name, ".mp3") || endsWith(name, ".aac") || endsWith(name, ".m4a") ||
                                     endsWith(name, ".wav") || endsWith(name, ".flac")){
            return name;
        }
        else{
            return nullptr;
        }
    }
    return nullptr;
}

bool sendAudioList2Web(const char* audioDir){
    if(!setAudioFolder(audioDir)) return false;
    const char* FileName = NULL;
    String str = "AudioFileList=";
    uint8_t i = 0;
    while(true){
        FileName = listAudioFile();
        if(FileName){
            if(i) str += ",";
            str += (String)FileName;
            i++;
        }
        else break;
    }
    SerialPrintfln(ANSI_ESC_GREEN "%s", str.c_str());
    webSrv.send((const char*)str.c_str());
    return true;
}
/***********************************************************************************************************************
*                                         C O N N E C T   TO   W I F I                                                 *
***********************************************************************************************************************/
bool connectToWiFi(){
    String s_ssid = "", s_password = "", s_info = "";
    wifiMulti.addAP(_SSID, _PW);                // SSID and PW in code
    WiFi.setHostname("MiniWebRadio");
    File file = SD_MMC.open("/networks.csv"); // try credentials given in "/networks.txt"
    if(file){                                         // try to read from SD_MMC
        String str = "";
        while(file.available()){
            str = file.readStringUntil('\n');         // read the line
            if(str[0] == '*' ) continue;              // ignore this, goto next line
            if(str[0] == '\n') continue;              // empty line
            if(str[0] == ' ')  continue;              // space as first char
            if(str.indexOf('\t') < 0) continue;       // no tab
            str += "\t";
            uint p = 0, q = 0;
            s_ssid = "", s_password = "", s_info = "";
            for(int i = 0; i < str.length(); i++){
                if(str[i] == '\t'){
                    if(p == 0) s_ssid     = str.substring(q, i);
                    if(p == 1) s_password = str.substring(q, i);
                    if(p == 2) s_info     = str.substring(q, i);
                    p++;
                    i++;
                    q = i;
                }
            }
            //log_i("s_ssid=%s  s_password=%s  s_info=%s", s_ssid.c_str(), s_password.c_str(), s_info.c_str());
            if(s_ssid == "") continue;
            if(s_password == "") continue;
            wifiMulti.addAP(s_ssid.c_str(), s_password.c_str());
        }
        file.close();
    }
    SerialPrintfln("WiFI_info: Connecting WiFi...");
    if(wifiMulti.run() == WL_CONNECTED){
        WiFi.setSleep(false);
        return true;
    }else{
        SerialPrintfln(ANSI_ESC_RED "WiFi credentials are not correct\n");
        return false;  // can't connect to any network
    }
}
/***********************************************************************************************************************
*                                                    A U D I O                                                        *
***********************************************************************************************************************/
void connecttohost(const char* host){
    _f_isWebConnected = audioConnecttohost(host);
    _f_isFSConnected = false;
}
void connecttoFS(const char* filename, uint32_t resumeFilePos){
    _f_isFSConnected = audioConnecttoFS(filename, resumeFilePos);
    _f_isWebConnected = false;
}
void stopSong(){
    audioStopSong();
    _f_isFSConnected = false;
    _f_isWebConnected = false;
}

/***********************************************************************************************************************
*                                                    S E T U P                                                         *
***********************************************************************************************************************/
void setup(){
    Serial.begin(115200);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); // VSPI
    mutex_rtc     = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    SerialPrintfln("");
    SerialPrintfln(ANSI_ESC_YELLOW "***************************");
    SerialPrintfln(ANSI_ESC_YELLOW "*     MiniWebRadio V2     *");
    SerialPrintfln(ANSI_ESC_YELLOW "***************************");
    SerialPrintfln("");
    SerialPrintfln("setup() is pinned to core " ANSI_ESC_CYAN "%d", xPortGetCoreID());
    if(TFT_CONTROLLER < 2)  strcpy(_prefix, "/s");
    else                    strcpy(_prefix, "/m");
    pref.begin("MiniWebRadio", false);  // instance of preferences for defaults (tone, volume ...)
    stations.begin("Stations", false);  // instance of preferences for stations (name, url ...)


    tft.begin(TFT_CS, TFT_DC, VSPI);    // Init TFT interface
    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tp.setVersion(TP_VERSION);
    tp.setRotation(TP_ROTATION);

    SerialPrintfln("setup: Init SD card");
    pinMode(SD_MMC_D0, INPUT_PULLUP);
    if(!SD_MMC.begin("/sdcard", true)){
        clearAll();
        tft.setFont(_fonts[5]);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(50,100);
        tft.print("SD Card Mount Failed");
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "SD Card Mount Failed");
        while(1){};  // endless loop, MiniWebRadio does not work without SD
    }
    SerialPrintfln(ANSI_ESC_WHITE "setup: SD card found");

    defaultsettings();  // first init
    setTFTbrightness(getBrightness());

    if(TFT_CONTROLLER > 3) SerialPrintfln(ANSI_ESC_RED "The value in TFT_CONTROLLER is invalid");
    drawImage("/common/MiniWebRadioV2.jpg", 0, 0); // Welcomescreen
    SerialPrintfln("setup: seek for stations.csv");
    File file=SD_MMC.open("/stations.csv");
    if(!file){
        clearAll();
        tft.setFont(_fonts[5]);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(50,100);
        tft.print("stations.csv not found");
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "stations.csv not found");
        while(1){};  // endless loop, MiniWebRadio does not work without stations.csv
    }
    file.close();
    SerialPrintfln("setup: stations.csv found");

    SerialPrintfln("setup: seek for WiFi networks");
    if(!connectToWiFi()){
        clearAll();
        tft.setFont(_fonts[5]);
        tft.setTextColor(TFT_YELLOW);
        tft.setCursor(50,100);
        tft.print("WiFi credentials are not correct");
        setTFTbrightness(80);
        SerialPrintfln(ANSI_ESC_RED "WiFi credentials are not correct");
        while(1){};
    }
    strcpy(_myIP, WiFi.localIP().toString().c_str());
    SerialPrintfln("setup: connected to " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE
                   ", IP address is " ANSI_ESC_CYAN "%s", WiFi.SSID().c_str(), _myIP);

    ftpSrv.begin(SD_MMC, FTP_USERNAME, FTP_PASSWORD); //username, password for ftp.

    _f_rtc = rtc.begin(TZName);
    if(!_f_rtc){
        SerialPrintfln(ANSI_ESC_RED "connection to NTP failed, trying again");
        ESP.restart();
    }

    #if DECODER > 1 // DAC controlled by I2C
        if(!dac.begin(I2C_DATA, I2C_CLK)){
            SerialPrintfln(ANSI_ESC_RED "The DAC was not be initialized");
        }
    #endif

    audioInit();

    _sum_stations = stations.getUInt("sumstations", 0);
    SerialPrintfln("Number of saved stations: " ANSI_ESC_CYAN "%d", _sum_stations);
    _cur_station =  pref.getUInt("station", 1);
    SerialPrintfln("current station number: " ANSI_ESC_CYAN "%d", _cur_station);
    _cur_volume = getvolume();
    SerialPrintfln("current volume: " ANSI_ESC_CYAN "%d", _cur_volume);

    _alarmdays = pref.getUShort("alarm_weekday");
    _alarmtime = pref.getUInt("alarm_time");
    _state = RADIO;

     ir.begin();  // Init InfraredDecoder

    webSrv.begin(80, 81); // HTTP port, WebSocket port

    ticker.attach(1, timer1sec);
    if(HP_DETECT != -1){
        pinMode(HP_DETECT, INPUT);
        attachInterrupt(HP_DETECT, headphoneDetect, CHANGE);
    }
    if(AMP_ENABLED != -1){           // enable onboard amplifier
        pinMode(AMP_ENABLED, OUTPUT);
        digitalWrite(AMP_ENABLED, HIGH);
    }

    tft.fillScreen(TFT_BLACK); // Clear screen
    _f_mute = pref.getUShort("mute", 0);
    if(_f_mute) {
        SerialPrintfln("volume is muted: %d", _cur_volume);
        audioSetVolume(0);
        showHeadlineVolume();
    }
    else {
        setVolume(_cur_volume);
        _mute_volume = _cur_volume;
    }
    showHeadlineItem(RADIO);
    setStation(_cur_station);
    if(DECODER == 0) setTone();    // HW Decoder
    else             setI2STone(); // SW Decoder
    showFooter();
}
/***********************************************************************************************************************
*                                                  C O M M O N                                                         *
***********************************************************************************************************************/
const char* byte_to_binary(int8_t x){
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1){
        strcat(b, ((x & z) == z) ? "1" : "0");
    }
    return b;
}
bool startsWith (const char* base, const char* str) {
    char c;
    while ( (c = *str++) != '\0' )
      if (c != *base++) return false;
    return true;
}
bool endsWith (const char* base, const char* str) {
    int slen = strlen(str) - 1;
    const char *p = base + strlen(base) - 1;
    while(p > base && isspace(*p)) p--;  // rtrim
    p -= slen;
    if (p < base) return false;
    return (strncmp(p, str, slen) == 0);
}
int indexOf (const char* base, const char* str, int startIndex) {
    const char *p = base;
    for (; startIndex > 0; startIndex--)
        if (*p++ == '\0') return -1;
    char* pos = strstr(p, str);
    if (pos == nullptr) return -1;
    return pos - base;
}
boolean strCompare(char* str1, char* str2){
    return strCompare((const char*) str1, str2);
}
boolean strCompare(const char* str1, char* str2){ // returns true if str1 == str2
    if(!str1) return false;
    if(!str2) return false;
    if(strlen(str1) != strlen(str2)) return false;
    boolean f = true;
    uint16_t i = strlen(str1);
    while(i){
        i--;
        if(str1[i] != str2[i]){f = false; break;}
    }
    return f;
}
const char* scaleImage(const char* path){
    if((!endsWith(path, "bmp")) && (!endsWith(path, "jpg"))){ // not a image
        return UTF8toASCII(path);
    }
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    char* pch = strstr(path + 1, "/");
    if(pch){
        strncpy(pathBuff, path, (pch - path));
        if(TFT_CONTROLLER < 2) strcat(pathBuff, _prefix); // small pic,  320x240px
        else                   strcat(pathBuff, _prefix); // medium pic, 480x320px
        strcat(pathBuff, pch);
    }
    else{
        strcpy(pathBuff, "/common");
        if(TFT_CONTROLLER < 2) strcat(pathBuff, _prefix); // small pic,  320x240px
        else                   strcat(pathBuff, _prefix); // medium pic, 480x320px
        strcat(pathBuff, path);
    }
    return UTF8toASCII(pathBuff);
}
inline uint8_t getvolume(){
    return pref.getUShort("volume");
}
void setVolume(uint8_t vol){
    pref.putUShort("volume", vol);
    if(_f_mute==false){
        audioSetVolume(vol);
        showHeadlineVolume();
    }
    else{
        showHeadlineVolume();
    }
    _cur_volume = vol;
    SerialPrintfln("current volume: " ANSI_ESC_CYAN "%d", _cur_volume);

    #if DECODER > 1 // ES8388, AC101 ...
        if(HP_DETECT == -1){
            dac.SetVolumeSpeaker(_cur_volume * 3);
            dac.SetVolumeHeadphone(_cur_volume * 3);
        }
        else{
            if(digitalRead(HP_DETECT) ==  HIGH){
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
uint8_t downvolume(){
    if(_cur_volume == 0) return _cur_volume;
    _cur_volume --;
    setVolume(_cur_volume);
    return _cur_volume;
}
uint8_t upvolume(){
    if(_cur_volume == _max_volume) return _cur_volume;
    _cur_volume++;
    setVolume(_cur_volume);
    return _cur_volume;
}
inline void mute(){
    if(_f_mute==false){_f_muteDecrement = true; _f_muteIncrement = false;}
    else              {_f_muteIncrement = true; _f_muteDecrement = false; digitalWrite(AMP_ENABLED, HIGH);}
}

void setStation(uint16_t sta){
    //SerialPrintfln("sta %d, _cur_station %d", sta, _cur_station );
    if(sta > _sum_stations) sta = _cur_station;
    sprintf (_chbuf, "station_%03d", sta);
    String content = stations.getString(_chbuf);
    //SerialPrintfln("content %s", content.c_str());
    _stationName_nvs = content.substring(0, content.indexOf("#")); //get stationname
    content = content.substring(content.indexOf("#") + 1, content.length()); //get URL
    content.trim();
    free(_stationURL);
    _stationURL = strdup(content.c_str());
    _homepage = "";
    if(_state != RADIOico) clearTitle();

    SerialPrintfln("switch to station " ANSI_ESC_CYAN "%d", sta);

    if(_f_isWebConnected && sta == _cur_station && _state == RADIO){ // Station is already selected
        showStreamTitle();
    }
    else{
        _streamTitle = "";
        _icydescription = "";
        connecttohost(_stationURL);
    }
    _cur_station = sta;
    pref.putUInt("station", sta);
    if(_state == RADIO || _state == RADIOico) showLogoAndStationName();
    StationsItems();
    showFooterStaNr();
}
void nextStation(){
    if(_cur_station >= _sum_stations) setStation(1);
    else setStation(_cur_station + 1);
}
void prevStation(){
    if(_cur_station > 1) setStation(_cur_station - 1);
    else setStation(_sum_stations);
}

void StationsItems(){
    webSrv.send("stationNr=" + String(pref.getUInt("station")));
    webSrv.send("stationURL=" + String(_stationURL));
    webSrv.send("stationName=" + _stationName_nvs);
}

void changeBtn_pressed(uint8_t btnNr){
    if(_state == ALARM) drawImage(_pressBtn[btnNr], btnNr * _winButton.w , _dispHeight - _winButton.h);
    else                drawImage(_pressBtn[btnNr], btnNr * _winButton.w , _winButton.y);
}
void changeBtn_released(uint8_t btnNr){
    if(_state == RADIOico || _state == PLAYERico){
        if(_f_mute)  _releaseBtn[0] = "/btn/Button_Mute_Red.jpg";
        else         _releaseBtn[0] = "/btn/Button_Mute_Green.jpg";
    }
    if(_state == CLOCKico){
        if(_f_mute)  _releaseBtn[2] = "/btn/Button_Mute_Red.jpg";
        else         _releaseBtn[2] = "/btn/Button_Mute_Green.jpg";
    }
    if(_state == ALARM) drawImage(_releaseBtn[btnNr], btnNr * _winButton.w , _dispHeight - _winButton.h);
    else                drawImage(_releaseBtn[btnNr], btnNr * _winButton.w , _winButton.y);
}

void savefile(const char* fileName, uint32_t contentLength){ //save the uploadfile on SD_MMC
    char fn[256];

    if(endsWith(fileName, "jpg")){
        strcpy(fn, "/logo");
        strcat(fn, _prefix);
        if(!startsWith(fileName, "/")) strcat(fn, "/");
        strcat(fn, fileName);
        if(webSrv.uploadB64image(SD_MMC, UTF8toASCII(fn), contentLength)){
            SerialPrintfln("save image " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.reply("OK");
        }
        else webSrv.reply("failure");
    }
    else{
        if(!startsWith(fileName, "/")){
            strcpy(fn, "/");
            strcat(fn, fileName);
        }
        else{
            strcpy(fn, fileName);
        }
        if(webSrv.uploadfile(SD_MMC, UTF8toASCII(fn), contentLength)){
            SerialPrintfln("save file " ANSI_ESC_CYAN "%s" ANSI_ESC_WHITE " to SD card was successfully", fn);
            webSrv.reply("OK");
        }
        else webSrv.reply("failure");
        if(strcmp(fn, "/stations.csv") == 0) saveStationsToNVS();
    }
}
String setTone(){ // vs1053
    uint8_t ha =pref.getUShort("toneha");
    uint8_t hf =pref.getUShort("tonehf");
    uint8_t la =pref.getUShort("tonela");
    uint8_t lf =pref.getUShort("tonelf");
    audioSetTone(ha, hf, la, lf);
    sprintf(_chbuf, "toneha=%i\ntonehf=%i\ntonela=%i\ntonelf=%i\n", ha, hf, la, lf);
    String tone = String(_chbuf);
    return tone;
}

String setI2STone(){
    int8_t LP = pref.getShort("toneLP");
    int8_t BP = pref.getShort("toneBP");
    int8_t HP = pref.getShort("toneHP");
    audioSetTone(LP, BP, HP);
    sprintf(_chbuf, "LowPass=%i\nBandPass=%i\nHighPass=%i\n", LP, BP, HP);
    String tone = String(_chbuf);
    return tone;
}

void audiotrack(const char* fileName, uint32_t resumeFilePos){
    char* path = (char*)malloc(strlen(fileName) + 20);
    strcpy(path, "/audiofiles/");
    strcat(path, fileName);
    clearFName();
    showVolumeBar();
    showFileName(fileName);
    changeState(PLAYERico);
    connecttoFS((const char*) path, resumeFilePos);
    if(_f_isFSConnected){
        free(_lastconnectedfile);
        _lastconnectedfile = strdup(fileName);
        _resumeFilePos = 0;
    }
    if(path) free(path);
}

void IRAM_ATTR headphoneDetect(){ // called via interrupt
    _f_hpChanged = true;
}

/***********************************************************************************************************************
*                                          M E N U E / B U T T O N S                                                   *
***********************************************************************************************************************/
void changeState(int state){
    if(state == _state) return;  //nothing todo
    _f_volBarVisible = false;
    switch(state) {
        case RADIO:{
            showHeadlineItem(RADIO);
            if(_state == RADIOico || _state == RADIOmenue){
                showStreamTitle();
            }
            else if(_state == PLAYER  || _state == PLAYERico){
                setStation(_cur_station);
                showLogoAndStationName();
                showStreamTitle();
            }
            else if(_state == CLOCKico){
                showLogoAndStationName();
                showStreamTitle();
            }
            else if(_state == SLEEP){
                clearFName();
                clearTitle();
                connecttohost(_lastconnectedhost);
                showLogoAndStationName();
                showFooter();
                showHeadlineVolume();
            }
            else{
                showLogoAndStationName();
                showStreamTitle();
            }
            break;
        }
        case RADIOico:{
            showHeadlineItem(RADIOico);
            _pressBtn[0] = "/btn/Button_Mute_Yellow.jpg";        _releaseBtn[0] =  _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[1] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2] = "/btn/Button_Volume_Up_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Previous_Yellow.jpg";    _releaseBtn[3] = "/btn/Button_Previous_Green.jpg";
            _pressBtn[4] = "/btn/Button_Next_Yellow.jpg";        _releaseBtn[4] = "/btn/Button_Next_Green.jpg";
            clearTitle();
            showVolumeBar();
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case RADIOmenue:{
            showHeadlineItem(RADIOmenue);
            _pressBtn[0] = "/btn/MP3_Yellow.jpg";                _releaseBtn[0] = "/btn/MP3_Green.jpg";
            _pressBtn[1] = "/btn/Clock_Yellow.jpg";              _releaseBtn[1] = "/btn/Clock_Green.jpg";
            _pressBtn[2] = "/btn/Radio_Yellow.jpg";              _releaseBtn[2] = "/btn/Radio_Green.jpg";
            _pressBtn[3] = "/btn/Button_Sleep_Yellow.jpg";       _releaseBtn[3] = "/btn/Button_Sleep_Green.jpg";
            if(TFT_BL != -1){
                _pressBtn[4]="/btn/Bulb_Yellow.jpg";           _releaseBtn[4]="/btn/Bulb_Green.jpg";
            }
            else{
                _pressBtn[4]="/btn/Black.jpg";                 _releaseBtn[4]="/btn/Black.jpg";
            }
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            clearVolBar();
            break;
        }
        case CLOCK:{
            if(_state == ALARM){
                pref.putUInt("alarm_time", _alarmtime);
                pref.putUShort("alarm_weekday", _alarmdays);
                SerialPrintfln("Alarm set to " ANSI_ESC_CYAN "%2d:%2d" ANSI_ESC_WHITE " on " ANSI_ESC_CYAN
                               "%s\n", _alarmtime / 60, _alarmtime % 60, byte_to_binary(_alarmdays));
                clearHeader();
            }
            _state = CLOCK;
            showHeadlineItem(CLOCK);
            showHeadlineVolume();
            showHeadlineTime();
            showFooter();
            clearFName();
            clearTitle();
            display_time(true);
            break;
        }
        case CLOCKico:{
            _state = CLOCKico;
            showHeadlineItem(CLOCKico);
            clearFName();
            clearTitle();
            display_time(true);
            _pressBtn[0] = "/btn/Bell_Yellow.jpg";                _releaseBtn[0] = "/btn/Bell_Green.jpg";
            _pressBtn[1] = "/btn/Radio_Yellow.jpg";               _releaseBtn[1] = "/btn/Radio_Green.jpg";
            _pressBtn[2] = "/btn/Button_Mute_Red.jpg";            _releaseBtn[2] = _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[3] = "/btn/Button_Volume_Down_Yellow.jpg";  _releaseBtn[3] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[4] = "/btn/Button_Volume_Up_Yellow.jpg";    _releaseBtn[4] = "/btn/Button_Volume_Up_Blue.jpg";
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case BRIGHTNESS:{
            showHeadlineItem(BRIGHTNESS);
            _pressBtn[0] = "/btn/Button_Left_Yellow.jpg";        _releaseBtn[0] = "/btn/Button_Left_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
            _pressBtn[4] = "/btn/Black.jpg";                     _releaseBtn[4] = "/btn/Black.jpg";
            drawImage("/common/Brightness.jpg", 0, _winName.y);
            showBrightnessBar();
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case PLAYER:{
            if(_state == RADIO){
                clearFName();
                clearTitle();
            }
            showHeadlineItem(PLAYER);
            _pressBtn[0] = "/btn/Radio_Yellow.jpg";              _releaseBtn[0] = "/btn/Radio_Green.jpg";
            _pressBtn[1] = "/btn/Button_First_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_First_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[3] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[4] = "/btn/Black.jpg";                     _releaseBtn[4] = "/btn/Black.jpg";
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case PLAYERico:{
            showHeadlineItem(PLAYERico);
            _pressBtn[0] = "/btn/Button_Mute_Red.jpg";           _releaseBtn[0] = _f_mute? "/btn/Button_Mute_Red.jpg":"/btn/Button_Mute_Green.jpg";
            _pressBtn[1] = "/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1] = "/btn/Button_Volume_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2] = "/btn/Button_Volume_Up_Blue.jpg";
            _pressBtn[3] = "/btn/MP3_Yellow.jpg";                _releaseBtn[3] = "/btn/MP3_Green.jpg";
            _pressBtn[4] = "/btn/Radio_Yellow.jpg";              _releaseBtn[4] = "/btn/Radio_Green.jpg";
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
        case ALARM:{
            _pressBtn[0] = "/btn/Button_Left_Yellow.jpg";        _releaseBtn[0] = "/btn/Button_Left_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Right_Yellow.jpg";       _releaseBtn[1] = "/btn/Button_Right_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Up_Yellow.jpg";          _releaseBtn[2] = "/btn/Button_Up_Blue.jpg";
            _pressBtn[3] = "/btn/Button_Down_Yellow.jpg";        _releaseBtn[3] = "/btn/Button_Down_Blue.jpg";
            _pressBtn[4] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[4] = "/btn/Button_Ready_Blue.jpg";
            clearFName();
            clearTitle();
            display_alarmtime(0, 0, true);
            display_alarmDays(0, true);
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w,  _dispHeight - _winButton.h);}
            break;
        }
        case SLEEP:{
            showHeadlineItem(SLEEP);
            _pressBtn[0] = "/btn/Button_Up_Yellow.jpg";          _releaseBtn[0] = "/btn/Button_Up_Blue.jpg";
            _pressBtn[1] = "/btn/Button_Down_Yellow.jpg";        _releaseBtn[1] = "/btn/Button_Down_Blue.jpg";
            _pressBtn[2] = "/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2] = "/btn/Button_Ready_Blue.jpg";
            _pressBtn[3] = "/btn/Black.jpg";                     _releaseBtn[3] = "/btn/Black.jpg";
            _pressBtn[4] = "/btn/Button_Cancel_Yellow.jpg";      _releaseBtn[4] = "/btn/Button_Cancel_Blue.jpg";
            clearFName();
            clearTitle();
            display_sleeptime();
            if(TFT_CONTROLLER < 2) drawImage("/common/Night_Gown.bmp", 198, 23);
            else                   drawImage("/common/Night_Gown.bmp", 280, 45);
            for(int i = 0; i < 5 ; i++) {drawImage(_releaseBtn[i], i * _winButton.w, _winButton.y);}
            break;
        }
    }
    _state = state;
}
/***********************************************************************************************************************
*                                                      L O O P                                                         *
***********************************************************************************************************************/
void loop() {
    if(webSrv.loop()) return; // if true: ignore all other for faster response to web
    ir.loop();
    tp.loop();
    ftpSrv.handleFTP();

    if(_f_muteDecrement){
        if(_mute_volume > 0){
            _mute_volume--;
            audioSetVolume(_mute_volume);
            showHeadlineVolume();
        }
        else{
            digitalWrite(AMP_ENABLED, LOW);
            _f_muteDecrement = false;
            _f_mute = true;
            webSrv.send("mute=1");
            pref.putUShort("mute", _f_mute);
            if(_state == RADIOico){
                drawImage("/btn/Button_Mute_Red.jpg", 0, _winButton.y);
            }
        }
    }

    if(_f_muteIncrement){
        if(_mute_volume < _cur_volume){
            _mute_volume++;
            audioSetVolume(_mute_volume);
            showHeadlineVolume();
        }
        else{
            _f_muteIncrement = false;
            _f_mute = false;
            webSrv.send("mute=0");
            pref.putUShort("mute", _f_mute);
            if(_state == RADIOico){
                drawImage("/btn/Button_Mute_Green.jpg", 0, _winButton.y);
            }
        }
    }

    if(_f_1sec){
         _f_1sec = false;
        if(_state != ALARM && !_f_sleeping) showHeadlineTime();
        if(_state == CLOCK || _state == CLOCKico) display_time();

        if(_timeCounter){
            _timeCounter--;
            if(!_timeCounter){
                if(_state == RADIOico)                        changeState(RADIO);
                else if(_state == RADIOmenue)                 changeState(RADIO);
                else if(_state == CLOCKico)                   changeState(CLOCK);
                else if(_state == RADIO && _f_switchToClock) {changeState(CLOCK); _f_switchToClock = false;}
                else ;  // all other, do nothing
            }
        }
        if(_f_rtc==true){ // true -> rtc has the current time
            int8_t h=0;
            String time_s;
            xSemaphoreTake(mutex_rtc, portMAX_DELAY);
            time_s = rtc.gettime_s();
            xSemaphoreGive(mutex_rtc);
            if(_f_eof && (_state == RADIO || _f_eof_alarm)){
                _f_eof = false;
                if(_f_eof_alarm){
                    _f_eof_alarm = false;
                    if(_f_mute){
                        mute(); // mute off
                    }
                }
                connecttohost(_lastconnectedhost);
            }
            if((_f_mute==false)&&(!_f_sleeping)){
                if(time_s.endsWith("59:53") && _state == RADIO) { // speech the time 7 sec before a new hour is arrived
                    String hour = time_s.substring(0,2); // extract the hour
                    h = hour.toInt();
                    h++;
                    if( h== 24) h=0;
                    sprintf (_chbuf, "/voice_time/%d_00.mp3", h);
                    connecttoFS(_chbuf);
                }
            }

            if(_alarmtime == rtc.getMinuteOfTheDay()){ //is alarmtime?
                SerialPrintfln("is alarmtime");
                if((_alarmdays>>rtc.getweekday())&1){ //is alarmday?
                    if(!_f_semaphore) {_f_alarm = true;  _f_semaphore = true;} //set alarmflag
                }
            }
            else _f_semaphore=false;

            if(_f_alarm){
                SerialPrintfln(ANSI_ESC_MAGENTA "Alarm");
                _f_alarm=false;
                connecttoFS("/ring/alarm_clock.mp3");
                audioSetVolume(21);
            }
            if(_f_hpChanged){
                setVolume(_cur_volume);
                if(!digitalRead(HP_DETECT)) {SerialPrintfln("Headphone plugged in");}
                else                        {SerialPrintfln("Headphone unplugged");}
                _f_hpChanged = false;
            }
        }
        if(_commercial_dur > 0){
            _commercial_dur--;
            if((_commercial_dur == 2) && (_state == RADIO)) clearTitle();// end of commercial? clear streamtitle
        }
        if(_f_newIcyDescription){
            _f_newIcyDescription = false;
            webSrv.send("icy_description=" +_icydescription);
        }
    }
    if(_f_1min == true){
        updateSleepTime();
        _f_1min = false;
    }
}
/***********************************************************************************************************************
*                                                    E V E N T S                                                       *
***********************************************************************************************************************/
//Events from vs1053_ext library
void vs1053_info(const char *info){
    // SerialPrintfln("%s", info);
    if(endsWith(info, "Stream lost")) SerialPrintfln("%s", info);
}
void audio_info(const char *info){
    //  SerialPrintfln("%s", info);
    if(startsWith(info, "FLAC")) SerialPrintfln("%s", info);
    if(endsWith(info, "Stream lost")) SerialPrintfln("%s", info);
}
//----------------------------------------------------------------------------------------
void vs1053_showstation(const char *info){
    _stationName_air = info;
    SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", info);
    if(!_cur_station) showLogoAndStationName();
}
void audio_showstation(const char *info){
    _stationName_air = info;
    SerialPrintfln("StationName: " ANSI_ESC_MAGENTA "%s", info);
    if(!_cur_station) showLogoAndStationName();
}
//----------------------------------------------------------------------------------------
void vs1053_showstreamtitle(const char *info){
    if(_f_irNumberSeen) return; // discard streamtitle
    _streamTitle = info;
    if(_state == RADIO) showStreamTitle();
    SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", info);
}
void audio_showstreamtitle(const char *info){
    if(_f_irNumberSeen) return; // discard streamtitle
    _streamTitle = info;
    if(_state == RADIO) showStreamTitle();
    SerialPrintfln("StreamTitle: " ANSI_ESC_YELLOW "%s", info);
}
//----------------------------------------------------------------------------------------
void vs1053_commercial(const char *info){
    _commercial_dur = atoi(info) / 1000;                // info is the duration of advertising in ms
    _streamTitle = "Advertising: " + (String) _commercial_dur + "s";
    showStreamTitle();
    SerialPrintfln("StreamTitle: %s", info);
}
void audio_commercial(const char *info){
    _commercial_dur = atoi(info) / 1000;                // info is the duration of advertising in ms
    _streamTitle = "Advertising: " + (String) _commercial_dur + "s";
    showStreamTitle();
    SerialPrintfln("StreamTitle: %s", info);
}
//----------------------------------------------------------------------------------------
void vs1053_eof_mp3(const char *info){                  // end of mp3 file (filename)
    _f_eof = true;
    if(startsWith(info, "alarm")) _f_eof_alarm = true;
    SerialPrintfln("end of file: %s", info);
}
void audio_eof_mp3(const char *info){                  // end of mp3 file (filename)
    _f_eof = true;
    if(startsWith(info, "alarm")) _f_eof_alarm = true;
    SerialPrintfln("end of file: %s", info);
}
//----------------------------------------------------------------------------------------
void vs1053_lasthost(const char *info){                 // really connected URL
    free(_lastconnectedhost);
    _lastconnectedhost = strdup(info);
    SerialPrintfln("lastURL: ... %s", _lastconnectedhost);
}
void audio_lasthost(const char *info){                 // really connected URL
    free(_lastconnectedhost);
    _lastconnectedhost = strdup(info);
    SerialPrintfln("lastURL: ... %s", _lastconnectedhost);
}
//----------------------------------------------------------------------------------------
void vs1053_icyurl(const char *info){                   // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5){
        SerialPrintfln("icy-url: ... %s", info);
        _homepage = String(info);
        if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
    }
}
void audio_icyurl(const char *info){                   // if the Radio has a homepage, this event is calling
    if(strlen(info) > 5){
        SerialPrintfln("icy-url: ... %s", info);
        _homepage = String(info);
        if(!_homepage.startsWith("http")) _homepage = "http://" + _homepage;
    }
}
//----------------------------------------------------------------------------------------
void vs1053_id3data(const char *info){
    SerialPrintfln("id3data: " ANSI_ESC_GREEN "%s", info);
}
void audio_id3data(const char *info){
    SerialPrintfln("id3data: " ANSI_ESC_GREEN "%s", info);
}
//----------------------------------------------------------------------------------------
void vs1053_icydescription(const char *info){
    _icydescription = String(info);
    if(_streamTitle.length() == 0){
        _streamTitle = String(info);
        if(_state == RADIO){
            showStreamTitle();
        }
    }
    if(strlen(info)){
        _f_newIcyDescription = true;
        SerialPrintfln("icy-descr: . %s", info);
    }
}
void audio_icydescription(const char *info){
    _icydescription = String(info);
    if(_streamTitle.length() == 0){
        _streamTitle = String(info);
        if(_state == RADIO){
            showStreamTitle();
        }
    }
    if(strlen(info)){
        _f_newIcyDescription = true;
        SerialPrintfln("icy-descr: . %s", info);
    }
}
//----------------------------------------------------------------------------------------
void ftp_debug(const char* info) {
    if(startsWith(info, "File Name")) return;
    SerialPrintfln("ftpsrv: %s", info);
}
//----------------------------------------------------------------------------------------
void RTIME_info(const char *info){
    SerialPrintfln("rtime_info: %s", info);
}

//Events from tft library
void tft_info(const char *info){
    SerialPrintfln("tft_info: %s", info);
}

// Events from IR Library
void ir_res(uint32_t res){
    _f_irNumberSeen = false;
    if(_state != RADIO) return;
    if(_f_sleeping == true) return;
    if(res != 0){
        setStation(res);
    }
    else{
        setStation(_cur_station); // valid between 1 ... 999
    }
    return;
}
void ir_number(const char* num){
    _f_irNumberSeen = true;
    if(_state != RADIO) return;
    if(_f_sleeping) return;
    tft.fillRect(_winLogo.x, _winLogo.y, _dispWidth , _winName.h + _winTitle.h, TFT_BLACK);
    tft.setFont(_fonts[6]);
    tft.setTextColor(TFT_GOLD);
    tft.setCursor(_irNumber_x, _irNumber_y);
    tft.print(num);
}
void ir_key(const char* key){
    if(_f_sleeping == true && key[0] != 'k') return;
    if(_f_sleeping == true && key[0] == 'k'){ //awake
        _f_sleeping = false;
        SerialPrintfln("awake");
        setTFTbrightness(pref.getUShort("brightness"));
        changeState(RADIO);
        connecttohost(_lastconnectedhost);
        showLogoAndStationName();
        showFooter();
        showHeadlineItem(RADIO);
        showHeadlineVolume();
        return;
    }

    switch(key[0]){
        case 'k':       if(_state == SLEEP) {                                                   // OK
                            updateSleepTime(true); changeState(RADIO); break;}
                        if(_state == RADIO){changeState(CLOCK); break;}
                        if(_state == CLOCK){changeState(RADIO); break;}
        case 'r':       upvolume();                                                             // right
                        break;
        case 'l':       downvolume();                                                           // left
                        break;
        case 'u':       if(_state == RADIO){nextStation(); break;}                              // up
                        if(_state == CLOCK){nextStation(); changeState(RADIO);
                                            _timeCounter = 5; _f_switchToClock = true; break;}
                        if(_state == SLEEP){display_sleeptime(1); break;}
        case 'd':       if(_state == RADIO){prevStation(); break;}                              // down
                        if(_state == CLOCK){prevStation(); changeState(RADIO);
                                            _timeCounter = 5; _f_switchToClock = true; break;}
                        if(_state == SLEEP) display_sleeptime(-1);
                        break;
        case '#':       mute();                                                                 // #
                        break;
        case '*':       if(_state == RADIO){changeState(SLEEP); break;}                                 // *
                        if(_state == SLEEP){changeState(RADIO); break;}
        default:        break;
    }
}
// Event from TouchPad
void tp_pressed(uint16_t x, uint16_t y){
    //SerialPrintfln("tp_pressed, state is: %i", _state);
    _timeCounter = 5;
    enum : int8_t{none = -1, RADIO_1, RADIOico_1, RADIOico_2, RADIOmenue_1,
                             PLAYER_1, PLAYERico_1, ALARM_1, BRIGHTNESS_1,
                             CLOCK_1, CLOCKico_1, ALARM_2, SLEEP_1};
    int8_t yPos    = none;
    int8_t btnNr   = none; // buttonnumber

    if(_f_sleeping) return; // awake in tp_released()

    switch(_state){
        case RADIO:         if(                     y <= _winTitle.y)                 {yPos = RADIO_1;}
                            break;
        case RADIOico:      if(                     y <= _winTitle.y)                 {yPos = RADIOico_1;}
                            if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = RADIOico_2;   btnNr = x / _winButton.w;}
                            break;
        case RADIOmenue:    if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = RADIOmenue_1; btnNr = x / _winButton.w;}
                            break;
        case PLAYER:        if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = PLAYER_1;     btnNr = x / _winButton.w;}
                            break;
        case PLAYERico:     if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = PLAYERico_1;  btnNr = x / _winButton.w;}
                            break;
        case CLOCK:         if(                     y <= _winTitle.y)                 {yPos = CLOCK_1;}
                            break;
        case CLOCKico:      if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = CLOCKico_1; btnNr = x / _winButton.w;}
                            break;
        case ALARM:         if(                     y <= _alarmdays_h)                {yPos = ALARM_1; btnNr = (x - 2) / _alarmdays_w;} //weekdays
                            if(                     y >= _winButton.y + _winFooter.h) {yPos = ALARM_2; btnNr = x / _winButton.w;}
                            break;
        case SLEEP:         if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = SLEEP_1; btnNr = x / _winButton.w;}
                            break;
        case BRIGHTNESS:    if(_winButton.y <= y && y <= _winButton.y + _winButton.h) {yPos = BRIGHTNESS_1; btnNr = x / _winButton.w;}
        default:            break;
    }
    if(yPos == none) {SerialPrintfln(ANSI_ESC_YELLOW "Touchpoint not valid x=%d, y=%d", x, y); return;}

    switch(yPos){
        case RADIO_1:       changeState(RADIOico); break;
        case RADIOico_1:    changeState(RADIOmenue); break;
        case CLOCK_1:       changeState(CLOCKico);   break;
        case RADIOico_2:    if(btnNr == 0){_releaseNr =  0; mute();}
                            if(btnNr == 1){_releaseNr =  1; } // Vol-
                            if(btnNr == 2){_releaseNr =  2; } // Vol+
                            if(btnNr == 3){_releaseNr =  3; } // station--
                            if(btnNr == 4){_releaseNr =  4; } // station++
                            changeBtn_pressed(btnNr); break;
        case RADIOmenue_1:  if(btnNr == 0){_releaseNr = 10; audioStopSong(); listAudioFile();} // AudioPlayer
                            if(btnNr == 1){_releaseNr = 11;} // Clock
                            if(btnNr == 2){_releaseNr = 12;} // Radio
                            if(btnNr == 3){_releaseNr = 13;} // Sleep
                            if(TFT_BL != -1){
                            if(btnNr == 4){_releaseNr = 14;} // Brightness
                            }
                            changeBtn_pressed(btnNr); break;
        case CLOCKico_1:    if(btnNr == 0){_releaseNr = 20;} // Bell
                            if(btnNr == 1){_releaseNr = 21;} // Radio
                            if(btnNr == 2){_releaseNr = 22; mute();}
                            if(btnNr == 3){_releaseNr = 23; } // Vol-
                            if(btnNr == 4){_releaseNr = 24; } // Vol+
                            changeBtn_pressed(btnNr); break;
        case ALARM_2:       if(btnNr == 0){_releaseNr = 30;} // left
                            if(btnNr == 1){_releaseNr = 31;} // right
                            if(btnNr == 2){_releaseNr = 32;} // up
                            if(btnNr == 3){_releaseNr = 33;} // down
                            if(btnNr == 4){_releaseNr = 34;} // ready (return to CLOCK)
                            changeBtn_pressed(btnNr); break;
        case PLAYER_1:      if(btnNr == 0){_releaseNr = 40;} // RADIO
                            if(btnNr == 1){_releaseNr = 41;} // first audiofile
                            if(btnNr == 2){_releaseNr = 42;} // next audiofile
                            if(btnNr == 3){_releaseNr = 43;} // ready
                            changeBtn_pressed(btnNr); break;
        case PLAYERico_1:   if(btnNr == 0){_releaseNr = 50; mute();}
                            if(btnNr == 1){_releaseNr = 51; } // Vol-
                            if(btnNr == 2){_releaseNr = 52; } // Vol+
                            if(btnNr == 3){_releaseNr = 53;} // PLAYER
                            if(btnNr == 4){_releaseNr = 54;} // RADIO
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
    }
}
void tp_released(){
    //SerialPrintfln("tp_released, state is: %i", _state);
    const char* chptr = NULL;
    char path[256 + 12] = "/audiofiles/";
    if(_f_sleeping == true){ //awake
        _f_sleeping = false;
        SerialPrintfln("awake");
        setTFTbrightness(pref.getUShort("brightness"));
        changeState(RADIO);
        connecttohost(_lastconnectedhost);
        showLogoAndStationName();
        showFooter();
        showHeadlineItem(RADIO);
        showHeadlineVolume();
        return;
    }

    switch(_releaseNr){
        /* RADIOico ******************************/
        case  0:    changeBtn_released(0); break; // Mute
        case  1:    changeBtn_released(1); downvolume(); showVolumeBar();  break;  // Vol-
        case  2:    changeBtn_released(2); upvolume();   showVolumeBar();  break;  // Vol+
        case  3:    prevStation(); showFooterStaNr(); changeBtn_released(3); break;  // previousstation
        case  4:    nextStation(); showFooterStaNr(); changeBtn_released(4); break;  //  nextstation

        /* RADIOmenue ******************************/
        case 10:    changeState(PLAYER);
                    if(setAudioFolder("/audiofiles")) chptr = listAudioFile();
                    if(chptr) strcpy(_afn, chptr);
                    showFileName(_afn); break;
        case 11:    changeState(CLOCK); break;
        case 12:    changeState(RADIO); break;
        case 13:    changeState(SLEEP); break;
        case 14:    changeState(BRIGHTNESS); break;

        /* CLOCKico ******************************/
        case 20:    changeState(ALARM); break;
        case 21:    changeState(RADIO); break;
        case 22:    changeBtn_released(2); break; // Mute
        case 23:    changeBtn_released(3); downvolume(); showVolumeBar(); break;
        case 24:    changeBtn_released(4); upvolume();   showVolumeBar(); break;

        /* ALARM ******************************/
        case 30:    changeBtn_released(0); display_alarmtime(-1 ,  0); break;
        case 31:    changeBtn_released(1); display_alarmtime( 1 ,  0); break;
        case 32:    changeBtn_released(2); display_alarmtime( 0 ,  1); break;
        case 33:    changeBtn_released(3); display_alarmtime( 0 , -1); break;
        case 34:    changeState(CLOCK); break;

        /* AUDIOPLAYER ******************************/
        case 40:    changeBtn_released(0); changeState(RADIO); break; // RADIO
        case 41:    changeBtn_released(1); // first audiofile
                    if(setAudioFolder("/audiofiles")) chptr = listAudioFile();
                    if(chptr) strcpy(_afn, chptr);
                    showFileName(_afn); break;
        case 42:    changeBtn_released(2); // next audiofile
                    chptr = listAudioFile();
                    if(chptr) strcpy(_afn ,chptr);
                    showFileName(_afn); break;
        case 43:    changeState(PLAYERico); showVolumeBar(); // ready
                    strcat(path, _afn);
                    connecttoFS((const char*) path);
                    if(_f_isFSConnected){
                        free(_lastconnectedfile);
                        _lastconnectedfile = strdup(path);
                    } break;
        case 44:    break;

        /* AUDIOPLAYERico ******************************/
        case 50:    changeBtn_released(0); break; // Mute
        case 51:    changeBtn_released(1); downvolume(); showVolumeBar(); break; // Vol-
        case 52:    changeBtn_released(2); upvolume();   showVolumeBar(); break; // Vol+
        case 53:    changeState(PLAYER);   showFileName(_afn); break;
        case 54:    changeState(RADIO); break;

        /* ALARM (weekdays) ******************************/
        case 60:    display_alarmDays(0); break;
        case 61:    display_alarmDays(1); break;
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
    }
    _releaseNr = -1;
}

//Events from websrv
void WEBSRV_onCommand(const String cmd, const String param, const String arg){                       // called from html
//    SerialPrintfln("HTML_cmd=%s params=%s arg=%s", cmd.c_str(),param.c_str(), arg.c_str());
    String  str;
    if(cmd == "homepage"){          webSrv.send("homepage=" + _homepage);
                                    return;}

    if(cmd == "to_listen"){         StationsItems(); // via websocket, return the name and number of the current station
                                    return;}

    if(cmd == "gettone"){           if(DECODER) webSrv.reply(setI2STone().c_str());
                                    else webSrv.reply(setTone().c_str());
                                    return;}

    if(cmd == "getmute"){           webSrv.reply(String(int(_f_mute)).c_str());
                                    return;}

    if(cmd == "getstreamtitle"){    webSrv.reply(_streamTitle.c_str());
                                    return;}

    if(cmd == "mute"){              mute();
                                    if(_f_mute) webSrv.reply("Mute on\n");
                                    else webSrv.reply("Mute off\n");
                                    return;}

    if(cmd == "toneha"){            pref.putUShort("toneha",(param.toInt()));                             // vs1053 tone
                                    webSrv.reply("Treble Gain set");
                                    setTone(); return;}

    if(cmd == "tonehf"){            pref.putUShort("tonehf",(param.toInt()));                             // vs1053 tone
                                    webSrv.reply("Treble Freq set");
                                    setTone(); return;}

    if(cmd == "tonela"){            pref.putUShort("tonela",(param.toInt()));                             // vs1053 tone
                                    webSrv.reply("Bass Gain set");
                                    setTone(); return;}

    if(cmd == "tonelf"){            pref.putUShort("tonelf",(param.toInt()));                             // vs1053 tone
                                    webSrv.reply("Bass Freq set");
                                    setTone(); return;}

    if(cmd == "LowPass"){           pref.putShort("toneLP", (param.toInt()));                           // audioI2S tone
                                    char lp[25] = "Lowpass set to "; strcat(lp, param.c_str()); strcat(lp, "dB");
                                    webSrv.reply(lp); setI2STone(); return;}

    if(cmd == "BandPass"){          pref.putShort("toneBP", (param.toInt()));                           // audioI2S tone
                                    char bp[25] = "Bandpass set to "; strcat(bp, param.c_str()); strcat(bp, "dB");
                                    webSrv.reply(bp); setI2STone(); return;}

    if(cmd == "HighPass"){          pref.putShort("toneHP", (param.toInt()));                           // audioI2S tone
                                    char hp[25] = "Highpass set to "; strcat(hp, param.c_str()); strcat(hp, "dB");
                                    webSrv.reply(hp); setI2STone(); return;}

    if(cmd == "audiolist"){         sendAudioList2Web("/audiofiles");                                   // via websocket
                                    return;}

    if(cmd == "audiotrack"){        audiotrack(param.c_str());
                                    webSrv.reply("OK\n"); return;}

    if(cmd == "uploadfile"){        _filename = param;  return;}

    if(cmd == "upvolume"){          str = "Volume is now " + (String)upvolume(); webSrv.reply(str.c_str()); return;}

    if(cmd == "downvolume"){        str = "Volume is now " + (String)downvolume(); webSrv.reply(str.c_str()); return;}

    if(cmd == "prev_station"){      prevStation(); return;}                                             // via websocket

    if(cmd == "next_station"){      nextStation(); return;}                                             // via websocket

    if(cmd == "set_station"){       setStation(param.toInt()); StationsItems(); return;}                // via websocket

    if(cmd == "stationURL"){        connecttohost(param.c_str());webSrv.reply("OK\n"); return;}

    if(cmd == "getnetworks"){       webSrv.reply(WiFi.SSID().c_str()); return;}

    if(cmd == "ping"){              webSrv.send("pong"); return;}

    if(cmd == "index.html"){        webSrv.show(index_html); return;}

    if(cmd == "get_tftSize"){       webSrv.send(_tftSize? "tftSize=m": "tftSize=s"); return;}

    if(cmd == "get_decoder"){       webSrv.send( DECODER? "decoder=s": "decoder=h"); return;}

    if(cmd == "favicon.ico"){       webSrv.streamfile(SD_MMC, "/favicon.ico"); return;}

    if(cmd.startsWith("SD")){       str = cmd.substring(2); webSrv.streamfile(SD_MMC, scaleImage(str.c_str())); return;}

    if(cmd == "change_state"){      changeState(param.toInt()); return;}

    if(cmd == "stop"){              _resumeFilePos = audioStopSong(); webSrv.reply("OK\n"); return;}

    if(cmd == "resumefile"){        if(!_lastconnectedfile) webSrv.reply("nothing to resume\n");
                                    else {audiotrack(_lastconnectedfile, _resumeFilePos);
                                    webSrv.reply("OK\n");} return;}

    if(cmd == "test"){              sprintf(_chbuf, "free heap: %u, Inbuff filled: %u, Inbuff free: %u\n",
                                    ESP.getFreeHeap(), audioInbuffFilled(), audioInbuffFree());
                                    webSrv.reply(_chbuf); return;}

    SerialPrintfln(ANSI_ESC_RED "unknown HTMLcommand %s", cmd.c_str());
}
void WEBSRV_onRequest(const String request, uint32_t contentLength){
    SerialPrintfln("request %s contentLength %d", request.c_str(), contentLength);
    if(request.startsWith("------")) return;      // uninteresting WebKitFormBoundaryString
    if(request.indexOf("form-data") > 0) return;  // uninteresting Info
    if(request == "fileUpload"){savefile(_filename.c_str(), contentLength);  return;}
    SerialPrintfln(ANSI_ESC_RED "unknown request: %s",request.c_str());
}
void WEBSRV_onInfo(const char* info){
    // if(startsWith(info, "WebSocket")) return;       // suppress WebSocket client available
    // if(!strcmp("ping", info)) return;               // suppress ping
    // if(!strcmp("to_listen", info)) return;          // suppress to_isten
    // if(startsWith(info, "Command client"))return;   // suppress Command client available
    // SerialPrintfln("HTML_info  : %s", info);    // infos for debug
}