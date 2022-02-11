//*********************************************************************************************************
//*    MiniWebRadio -- Webradio receiver for ESP32, 2.8 color display (320x240px) and VS1053 MP3 module.  *
//*********************************************************************************************************
//
// first release on 03/2017
// Version 1.31, Feb 10/2022
//
// Preparations, Pos 1 and 2 are not necessary in PlatformIO,
//
// 1)  Copy the partition table "MiniWebRadio.csv" into the current esp32 package (folder esp32/tools/partitions/)
//     MiniWebRadio needs 2.3MByte flash and 200KByte nvs
//
//   # Name,     Type,   SubType,   Offset,   Size,     Flags
//     phy_init, data,   phy,       0x9000,   0x7000,
//     factory,  app,    factory,   0x10000,  0x300000,
//     nvs,      data,   nvs,       0x310000, 0x32000,
//     spiffs,   data,   spiffs,    0x342000, 0xB0000,
//     eeprom,   data,   0x99,      0x3F2000, 0xD000,
//
// 2)  Add this to boards.txt in section "ESP32 Dev Module:
/*
       esp32.menu.PartitionScheme.miniwebradio=MiniWebRadio (3MB No OTA)
       esp32.menu.PartitionScheme.miniwebradio.build.partitions=miniwebradio
       esp32.menu.PartitionScheme.miniwebradio.upload.maximum_size=3145728
*/
// 3)  set the Timezone mentioned below, examples are in rtime.cpp
//
// 4)  extract the zip file to SD Card
//
// 5)  set WiFi credentials below, more credentials can be set in networks.txt (SD Card)
//
// 6)  change GPIOs if necessary, e.g ESP32 Pico V4: GPIO16 and 17 are connected to FLASH
//
// 7)  add libraries from my repositories to this project: vs1053_ext, IR and tft
//     TFT controller can be ILI9341 or HX8347D, set tft(0) or tft(1) below
//
// 8)  translate _hl_title, entries below, in your language
//
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//
//
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

// Digital I/O used
#define VS1053_CS      2 // 33
#define VS1053_DCS     4
#define VS1053_DREQ   36
#define TFT_CS        22
#define TFT_DC        21 
#define TFT_BL        17 // 32
#define TP_IRQ        39
#define TP_CS         16 // 5
#define SD_CS          5
// #define SD_MMC_D0      2  // cannot be changed
// #define SD_MMC_CLK    14  // cannot be changed
// #define SD_MMC_CMD    15  // cannot be changed
#define IR_PIN        34
#define SPI_MOSI      23  // (VSPI)
#define SPI_MISO      19  // (VSPI)
#define SPI_SCK       18  // (VSPI)
// #define VS1053_MOSI   13  // VS1053     (HSPI)
// #define VS1053_MISO   34  // VS1053     (HSPI)
// #define VS1053_SCK    12  // VS1053     (HSPI)

// some other defines
#define TZName          "CET-1CEST,M3.5.0,M10.5.0/3"    // Timezone (more TZNames in "rtime.cpp")
#define TFT_CONTROLLER  1                               // (0)ILI9341, (1)HX8347D
#define TFT_FREQUENCY   40000000                        // 27000000, 40000000
#define TFT_ROTATION    3                               // 0 ... 3
#define TP_ROTATION     3                               // 0 ... 3

String   _SSID = "mySSID";      // Your WiFi credentials here
String   _PW   = "myWiFiPassword";

#define SerialPrintfln(...) {xSemaphoreTake(mutex_rtc, portMAX_DELAY); \
                            Serial.printf("%s ", rtc.gettime_s()); \
                            Serial.printf(__VA_ARGS__); \
                            Serial.println(""); \
                            xSemaphoreGive(mutex_rtc);}

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
// 
struct w_h{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 320; uint16_t h = 20; } const _winHeader;
struct w_n{uint16_t x = 0;   uint16_t y = 20;  uint16_t w = 320; uint16_t h = 100;} const _winName;
struct w_t{uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 100;} const _winTitle;
struct w_f{uint16_t x = 0;   uint16_t y = 220; uint16_t w = 320; uint16_t h = 20; } const _winFooter;
struct w_i{uint16_t x = 0;   uint16_t y = 0;   uint16_t w = 180; uint16_t h = 20; } const _winItem;
struct w_v{uint16_t x = 180; uint16_t y = 0;   uint16_t w =  50; uint16_t h = 20; } const _winVolume;
struct w_m{uint16_t x = 260; uint16_t y = 0;   uint16_t w =  60; uint16_t h = 20; } const _winTime;
struct w_s{uint16_t x = 0;   uint16_t y = 220; uint16_t w =  60; uint16_t h = 20; } const _winStaNr;
struct w_l{uint16_t x = 60;  uint16_t y = 220; uint16_t w = 140; uint16_t h = 20; } const _winSleep;
struct w_a{uint16_t x = 200; uint16_t y = 220; uint16_t w = 120; uint16_t h = 20; } const _winIPaddr;
struct w_b{uint16_t x = 0;   uint16_t y = 120; uint16_t w = 320; uint16_t h = 14; } const _winVolBar;





//global variables
char     _chbuf[1024];
String   _station="",   _stationname="",   _stationURL="",      _homepage="";
String   _title="",     _info="",          _alarmtime="",       _filename="";
String   _time_s="",    _hour="",          _bitrate="",         _mp3Name[10];
String   _pressBtn[5],  _releaseBtn[5],    _myIP="0.0.0.0",     _lastconnectedhost="";
uint16_t _stationnr=0;
int8_t   _mp3Index=0;           // pointer _mp3Name[]
uint8_t  _state = 0;      //statemaschine
uint8_t  _releaseNr=0;
uint8_t  _timefile=0;           // speak the time
uint8_t  _commercial_dur=0;     // duration of advertising
uint16_t _sleeptime=0;          // time in min until MiniWebRadio goes to sleep
uint32_t _millis=0;
uint32_t _alarmdays=0;
boolean  f_1sec=false;          // flag is set every second
boolean  f_1min=false;          // flag is set every minute
boolean  f_SD_okay=false;       // true if SD card in place and readable
boolean  f_mute=false;
boolean  f_rtc=false;           // true if time from ntp is received
boolean  f_mp3eof=false;        // set at the end of mp3 file
boolean  f_alarm=false;         // set if alarmday and alarmtime is equal localtime
boolean  f_timespeech=false;    // if true activate timespeech
boolean  f_has_ST=false;        // has StreamTitle?
boolean  f_sleeping=false;      // true if sleepmode
boolean  semaphore=false;
boolean  f_upload=false;        // if true next file is saved to SD

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

const unsigned short* _fonts[6] = {
    Times_New_Roman15x14,
    Times_New_Roman21x17,
    Times_New_Roman27x21,
    Times_New_Roman34x27,
    Times_New_Roman38x31,
    Times_New_Roman43x35,
//    Times_New_Roman56x46,
//    Times_New_Roman66x53,
};

// display layout
const uint16_t _yHeader =0;                     // yPos Header
const uint16_t _hHeader =20;                    // height Header
const uint16_t _yName  =_yHeader+_hHeader;      // yPos StationName
const uint16_t _hName  =100;                    // height Stationname
const uint16_t _yTitle =_yName+_hName;          // yPos StreamTitle
const uint16_t _hTitle =100;                    // height StreamTitle
const uint16_t _yFooter =_yTitle+_hTitle;       // yPos Footer
const uint16_t _hFooter=20;                     // height Footer
const uint16_t _yVolBar=_yTitle+30;             // yPos VolumeBar
const uint16_t _hVolBar=5;                      // height VolumeBar
const uint16_t _wLogo=96;                       // width Logo
const uint16_t _hLogo=96;                       // height Logo
const uint16_t _yLogo=_yName+(_hName-_hLogo)/2; // yPos Logos
const uint16_t _wBtn=64;                        // width Button
const uint16_t _hBtn=64;                        // height Button
const uint16_t _yBtn=_yVolBar+_hVolBar+10;      // yPos Buttons

//objects
TFT tft(TFT_CONTROLLER);
VS1053 vs1053(VS1053_CS, VS1053_DCS, VS1053_DREQ, VSPI, SPI_MOSI, SPI_MISO, SPI_SCK);
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

//prototypes
// boolean defaultsettings();
boolean saveStationsToNVS();
// uint8_t getvolume();
const char* UTF8toASCII(const char* str);
const char* ASCIItoUTF8(const char* str);
// void StationsItems();
// void changeState(int state);
bool endsWith (const char* base, const char* str);
void showStreamTitle(String ST);
void showFooter();
inline void StationsItems();
boolean drawImage(const char* path, uint16_t posX = 0, uint16_t posY = 0, uint16_t maxHeigth = 0, uint16_t maxWidth = 0);
inline uint8_t getvolume();
const char* scaleImage(const char* path);

//**************************************************************************************************
//                                D E F A U L T S E T T I N G S                                    *
//**************************************************************************************************
void defaultsettings(){
    String str="";

    Serial.println("set default");
    //
    pref.clear();
    //
    pref.putUInt("brightness",100); // 100% display backlight
    pref.putUInt("alarm_weekday",0); // for alarmclock
    pref.putString("alarm_time","00:00");
    pref.putUInt("ringvolume",21);
    //
    pref.putUInt("volume",12); // 0...21
    pref.putUInt("mute",   0); // no mute
    pref.putUInt("toneha", 0); // BassFreq 0...15
    pref.putUInt("tonehf", 0); // TrebleGain 0...14
    pref.putUInt("tonela", 0); // BassGain 0...15
    pref.putUInt("tonelf", 0); // BassFreq 0...13
    //
    pref.putUInt("station", 1);
    pref.putUInt("sleeptime", 1);
    //
    saveStationsToNVS();
}
boolean saveStationsToNVS(){
    String X="", Cy="", StationName="", StreamURL="", STsubstitute="", currentLine="", tmp="";
    uint16_t cnt=0;
    // StationList
    File file = SD.open("/stations.csv");
    if(file){  // try to read from SD
        stations.clear();
        currentLine = file.readStringUntil('\n');             // read the headline
        while(file.available()){
            currentLine = file.readStringUntil('\n');         // read the line
            currentLine += '\t';
            uint p = 0, q = 0;
            X=""; Cy=""; StationName=""; StreamURL=""; STsubstitute="";
            for(int i = 0; i < currentLine.length(); i++){
                if(currentLine[i] == '\t'){
                    if(p == 0) X            = currentLine.substring(q, i);
                    if(p == 1) Cy           = currentLine.substring(q, i);
                    if(p == 2) StationName  = currentLine.substring(q, i);
                    if(p == 3) StreamURL    = currentLine.substring(q, i);
                    if(p == 4) STsubstitute = currentLine.substring(q, i);
                    p++;
                    i++;
                    q = i;
                }
            }
            if(X == "*") continue;
            if(StationName == "") continue; // is empty
            if(StreamURL   == "") continue; // is empty
            //log_i("Cy=%s, StationName=%s, StreamURL=%s, STsubstitute=%s",Cy.c_str(), StationName.c_str(), StreamURL.c_str(), STsubstitute.c_str());
            cnt++;
            if(cnt==1000){
                Serial.println("No more than 999 entries in stationlist allowed!");
                cnt--; // maxstations 999
                break;
            }
            tmp = StationName + "#" + StreamURL;
            sprintf(_chbuf, "station_%03d", cnt);
            stations.putString(_chbuf, tmp);

            if(STsubstitute.length() > 0){  // is reasonable? then save additional info
                sprintf(_chbuf, "info_%03d", cnt);
                stations.putString(_chbuf, STsubstitute);
            }
        }
        stations.putLong("stations.size", file.size());
        file.close();
        stations.putUInt("maxstations", cnt);
        Serial.printf("stationlist internally loaded\n");
        Serial.printf("maxstations: %i\n", cnt);
        //log_i("nvs free entries %i", stations.freeEntries());
        return true;
    }
    else return false;
}
boolean ST_rep(){  // if station has no streamtitle: replace streamtitle, seek in info
    uint16_t station=pref.getUInt("station");
    sprintf(_chbuf, "info_%03d", station);
    if(stations.isKey(_chbuf)) _title = stations.getString(_chbuf);  // found probably replacement information
    else _title = "No streamtitle available";
    if(_title.length() > 5){showStreamTitle(_title); return true;}
    return false;
}
//**************************************************************************************************
//                                T F T   B R I G H T N E S S                                      *
//**************************************************************************************************
void setTFTbrightness(uint8_t duty){ //duty 0...100 (min...max)
    ledcAttachPin(TFT_BL, 1);        //Configure variable led, TFT_BL pin to channel 1
    ledcSetup(1, 12000, 8);          // 12 kHz PWM and 8 bit resolution
    ledcWrite(1, duty);
}
inline uint32_t getTFTbrightness(){
    return ledcRead(1);
}
inline uint8_t downBrightness(){
    uint8_t br; br=pref.getUInt("brightness");
    if(br>5) {br-=5; pref.putUInt("brightness", br); setTFTbrightness(br);} return br;
}
inline uint8_t upBrightness(){
    uint8_t br; br=pref.getUInt("brightness");
    if(br<100){br+=5; pref.putUInt("brightness", br); setTFTbrightness(br);} return br;
}
inline uint8_t getBrightness(){
    return pref.getUInt("brightness");
}
//**************************************************************************************************
//                                       A S C I I                                                *
//**************************************************************************************************
const char* UTF8toASCII(const char* str){
    uint16_t i=0, j=0;
    char tab[96]={
          96,173,155,156, 32,157, 32, 32, 32, 32,166,174,170, 32, 32, 32,248,241,253, 32,
          32,230, 32,250, 32, 32,167,175,172,171, 32,168, 32, 32, 32, 32,142,143,146,128,
          32,144, 32, 32, 32, 32, 32, 32, 32,165, 32, 32, 32, 32,153, 32, 32, 32, 32, 32,
         154, 32, 32,225,133,160,131, 32,132,134,145,135,138,130,136,137,141,161,140,139,
          32,164,149,162,147, 32,148,246, 32,151,163,150,129, 32, 32,152
     };
    while((str[i]!=0)&&(j<1020)){
        _chbuf[j]=str[i];
        if(str[i]==0xC2){ // compute unicode from utf8
            i++;
            if((str[i]>159)&&(str[i]<192)) _chbuf[j]=tab[str[i]-160];
            else _chbuf[j]=32;
        }
        else if(str[i]==0xC3){
            i++;
            if((str[i]>127)&&(str[i]<192)) _chbuf[j]=tab[str[i]-96];
            else _chbuf[j]=32;
        }
        i++; j++;
    }
    _chbuf[j]=0;
    return (_chbuf);
}
const char* ASCIItoUTF8(const char* str){
    uint16_t i=0, j=0, uni=0;
    uint16_t tab[128]={
         199, 252, 233, 226, 228, 224, 229, 231, 234, 235, 232, 239, 238, 236, 196, 197,
         201, 230, 198, 244, 246, 242, 251, 249, 255, 214, 220, 162, 163, 165,8359, 402,
         225, 237, 243, 250, 241, 209, 170, 186, 191,8976, 172, 189, 188, 161, 171, 187,
        9617,9618,9619,9474,9508,9569,9570,9558,9557,9571,9553,9559,9565,9564,9563,9488,
        9492,9524,9516,9500,9472,9532,9566,9567,9562,9556,9577,9574,9568,9552,9580,9575,
        9576,9572,9573,9561,9560,9554,9555,9579,9578,9496,9484,9608,9604,9612,9616,9600,
         945, 223, 915, 960, 931, 963, 181, 964, 934, 920, 937, 948,8734, 966, 949,8745,
        8801, 177,8805,8804,8992,8993, 247,8776, 176,8729, 183,8730,8319, 178,9632, 160
    };
    while((str[i]!=0)&&(j<1020)){
        uni=str[i];
        if(uni>=128){uni-=128; uni=tab[uni];}
//            uni=UTF8fromASCII(str[i]);
            switch(uni){
                case   0 ... 127:{_chbuf[j]=str[i]; i++; j++; break;}
                case 160 ... 191:{_chbuf[j]=0xC2; _chbuf[j+1]=uni; j+=2; i++; break;}
                case 192 ... 255:{_chbuf[j]=0xC3; _chbuf[j+1]=uni-64; j+=2; i++; break;}
                default:{_chbuf[j]=' '; i++; j++; break;} // ignore all other
            }
    }
    _chbuf[j]=0;
    return _chbuf;
}
//**************************************************************************************************
//                                        T I M E R                                                *
//**************************************************************************************************
void timer1sec() {
    static volatile uint8_t sec=0;
    f_1sec=true;
    sec++;
    //log_i("sec=%i", sec);
    if(sec==60){sec=0; f_1min=true;}
}
//**************************************************************************************************
//                                       D I S P L A Y                                             *
//**************************************************************************************************
inline void clearHeader() {tft.fillRect(0, _yHeader, tft.width(), _hHeader, TFT_BLACK);}  // y   0...19
inline void clearStation(){tft.fillRect(0, _yName,   tft.width(), _hName,   TFT_BLACK);}  // y  20...119
inline void clearTitle()  {tft.fillRect(0, _yTitle,  tft.width(), _hTitle,  TFT_BLACK);}  // y 120...219
inline void clearFooter() {tft.fillRect(0, _yFooter, tft.width(), _hFooter, TFT_BLACK);}  // y 220...239
inline void clearDisplay(){tft.fillScreen(TFT_BLACK);}                       // y   0...239
inline uint16_t txtlen(String str) {uint16_t len=0; for(int i=0; i<str.length(); i++) if(str[i]<=0xC2) len++; return len;}

void display_info(const char *str, int ypos, int height, uint16_t color, uint16_t indent){
    tft.fillRect(0, ypos, tft.width(), height, TFT_BLACK);  // Clear the space for new info
    tft.setTextColor(color);                                // Set the requested color
    tft.setCursor(indent, ypos);                            // Prepare to show the info
    tft.print(str);                                         // Show the string
}
void showStreamTitle(String ST){
    if(_state!=RADIO) return;
    webSrv.send("streamtitle=" + ST);
    ST.trim();  // remove all leading or trailing whitespaces
    ST.replace(" | ", "\n");   // some stations use pipe as \n or
    ST.replace("| ", "\n");    // or
    ST.replace("|", "\n");
    //log_i("Streamtitle=%s txtlen=%i", ST.c_str(), txtlen(ST));
    if(txtlen(ST) > 4) f_has_ST=true; else f_has_ST=false;
    tft.setFont(Times_New_Roman43x35);
    if(txtlen(ST)> 30) tft.setFont(Times_New_Roman38x31);
    if(txtlen(ST)> 45) tft.setFont(Times_New_Roman34x27);
    if(txtlen(ST)> 65) tft.setFont(Times_New_Roman27x21);
    if(txtlen(ST)>130) tft.setFont(Times_New_Roman21x17);
    if(txtlen(ST)>200) tft.setFont(Times_New_Roman15x14);
    //for(int i=0;i<ST.length(); i++) log_i("ST[%i]=%i", i, ST[i]);  // see what You get
    display_info(ST.c_str(), _yTitle, _hTitle, TFT_CYAN, 0);
}
void showStation(){
    String str1="", str2="", str3="";
    int16_t idx=0;
    if(_stationname=="") str1=_station;
    else str1=_stationname;
    str2=str1;
    str3=str1;  // now str1, str2 and str3 contains either _station or _stationname
    idx=str1.indexOf('|');
    if(idx>0){
        str2=str1.substring(0, idx); // before pipe
        str2.trim();
        str3=str1.substring(idx+1); // after pipe
        str3.trim();
    }

    tft.setFont(Times_New_Roman43x35);
    if(txtlen(str2)>20) tft.setFont(Times_New_Roman38x31);
    if(txtlen(str2)>32) tft.setFont(Times_New_Roman34x27);
    if(txtlen(str2)>45) tft.setFont(Times_New_Roman27x21);
    if(txtlen(str2)>60) tft.setFont(Times_New_Roman21x17);
    if(txtlen(str2)>90) tft.setFont(Times_New_Roman15x14);
    display_info(str2.c_str(), _yName, _hName, TFT_YELLOW, _wLogo+14);// Show station name

    showStreamTitle("");
    showFooter();

    str2 = "/logo/" + String(str3.c_str()) +".jpg";
    if(f_SD_okay){
        if(drawImage( str2.c_str(), 0, _yLogo)==false){ // filename mostly given from _stationname exist?
            drawImage("/common/unknown.jpg", 0, _yLogo);  // if no draw unknown
        }
    }
}
void showHeadlineVolume(uint8_t vol){
    if(_state == ALARM || _state== BRIGHTNESS) return;
    sprintf(_chbuf, "Vol %02d", vol);
    tft.fillRect(183, _yHeader, 67, _hHeader, TFT_BLACK);
    tft.setCursor(183, _yHeader);
    tft.setFont(Times_New_Roman27x21);
    tft.setTextColor(TFT_DEEPSKYBLUE);
    tft.print(_chbuf);
}
void showHeadlineItem(){
    tft.setFont(Times_New_Roman27x21);
    display_info(_hl_item[_state], _yHeader, _hHeader, TFT_WHITE, 0);
    if(_state!=SLEEP) showHeadlineVolume(getvolume());
}
void showHeadlineTime(){
    if(_state==CLOCK || _state==CLOCKico || _state==BRIGHTNESS || _state==ALARM || _state==SLEEP) return;
    tft.setFont(Times_New_Roman27x21);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.fillRect(250, _yHeader, 70, _hHeader, TFT_BLACK);
    if(!f_rtc) return; // has rtc the correct time? no -> return
    tft.setCursor(250, 0);
    tft.print(rtc.gettime_s());
}
void showFooter(){  // bitrate stationnumber, IPaddress
    if(_state!=RADIO) return;
    clearFooter();
    if(_bitrate.length()==0) _bitrate="   ";  // if bitrate is unknown
    tft.setFont(Times_New_Roman21x17);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.setCursor(0, _yFooter);
    tft.print("BR:");
    tft.setTextColor(TFT_LAVENDER);
    tft.print(_bitrate.c_str());
    tft.setCursor(60, _yFooter);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.print("STA:");
    tft.setTextColor(TFT_LAVENDER);
    tft.printf("%03d", _stationnr);
    tft.setCursor(130, _yFooter);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.print("myIP:");
    tft.setTextColor(TFT_LAVENDER);
    tft.print(_myIP.c_str());
    tft.setCursor(280, _yFooter);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.print("S:");
    if(_sleeptime==0 ) tft.setTextColor(TFT_LAVENDER); else tft.setTextColor(TFT_ORANGE);
    tft.printf("%03d", _sleeptime);
}
void updateSleepTime(){
    if(_sleeptime>0){
        _sleeptime--;
        if(_sleeptime==0){
            setTFTbrightness(0);    // backlight off
            vs1053.setVolume(0);       // silence
            f_sleeping=true;        // MiniWebRadio is in sleepmode now
        }
    }
    if(_state==RADIO) showFooter();
}

boolean drawImage(const char* path, uint16_t posX, uint16_t posY, uint16_t maxHeigth, uint16_t maxWidth){
    const char* scImg = scaleImage(path);
    if(endsWith(scImg, "bmp")){
        return tft.drawBmpFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
    }
    if(endsWith(scImg, "jpg")){
        return tft.drawJpgFile(SD, scImg, posX, posY, maxWidth, maxHeigth);
    }
    return false; // not jpg or bmp
}

//**************************************************************************************************
//                                T O N E / S E T  S T A T I O N S                                 *
//**************************************************************************************************
String tone(){
    String str_tone="";
    uint8_t u8_tone[4];
    u8_tone[0]=pref.getUInt("toneha"); u8_tone[1]=pref.getUInt("tonehf");
    u8_tone[2]=pref.getUInt("tonela"); u8_tone[3]=pref.getUInt("tonelf");
    sprintf(_chbuf, "toneha=%i\ntonehf=%i\ntonela=%i\ntonelf=%i\n",u8_tone[0],u8_tone[1],u8_tone[2],u8_tone[3]);
    str_tone=String(_chbuf);
    f_mute=pref.getUInt("mute");
    if(f_mute==false) vs1053.setVolume(pref.getUInt("volume"));
    else {vs1053.setVolume(0);showHeadlineVolume(0);}
    vs1053.setTone(u8_tone);
    return str_tone;
}
String setStation(int16_t stationNr) // -1: previous station, 0: next station, 1...maxstations: set stationNr
{
    uint16_t maxstations=stations.getUInt("maxstations");
    int16_t  station=pref.getUInt("station");

    if     (stationNr==0) {if(++station>maxstations)  station=1;}
    else if(stationNr==-1){if(--station<1) station=maxstations;}
    else    station=stationNr;

    sprintf (_chbuf, "station_%03d", station);// Result of search
    String  content=stations.getString(_chbuf);
    //log_i("content %s", content.c_str());
    _stationname=content.substring(0, content.indexOf("#")); //get stationname
    content=content.substring(content.indexOf("#")+1, content.length()); //get URL
    content.trim();
    _stationURL=content;
    _stationnr=station;
    _homepage="";
    pref.putUInt("station", station);
    f_has_ST=false; // will probably be set in ShowStreamtitle
    StationsItems();
    return content;
}

//**************************************************************************************************
//                                   H A N D L E   R E Q U E S T                                   *
//**************************************************************************************************
void savefile(String fileName, uint32_t contentLength){ //save the uploadfile on SD
    //log_i("request=%s",request.c_str());
    //log_i("end request seen");
    if(!fileName.startsWith("/")) fileName = "/"+fileName;

    if(fileName.endsWith("jpg")){
        fileName="/logo"+ fileName;
        if(webSrv.uploadB64image(SD, UTF8toASCII(fileName.c_str()), contentLength)) webSrv.reply("OK");
        else webSrv.reply("failure");
    }
    else{
        if(webSrv.uploadfile(SD, UTF8toASCII(fileName.c_str()), contentLength)) webSrv.reply("OK");
        else webSrv.reply("failure");
        if(fileName==String("/stations.csv")) saveStationsToNVS();
    }
}
//**************************************************************************************************
//                                       L I S T M P 3 F I L E                                     *
//**************************************************************************************************
String listmp3file(const char * dirname="/audiofiles", uint8_t levels=2, fs::FS &fs=SD){
    static String SD_outbuf="";            // Output buffer for cmdclient
    String filename;                       // Copy of filename for lowercase test
    uint8_t index=0;
    if(!f_SD_okay) return "";              // See if known card
    File root = fs.open(dirname);
    if(!root){log_e("Failed to open directory"); return ""; }
    if(!root.isDirectory()){log_e("Not a directory"); return "";}
    SD_outbuf="";
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            if(levels){
                listmp3file(file.name(), levels -1, fs);
            }
        } else {
            #if ESP_ARDUINO_VERSION_MAJOR >= 2
                filename = file.path(); // new in ESP32 Arduino V2.0.0
            #else
                filename = file.name(); // used in V1.0.0 ... V1.0.6
            #endif
            filename.substring(filename.length()-4).toLowerCase();
            filename=filename.substring(1,filename.length()); // remove first '/'
            if(filename.endsWith(".mp3")){
                filename+="\n";
                if(index<10){_mp3Name[index]=filename; index++;}  //store the first 10 Names
                SD_outbuf+=ASCIItoUTF8(filename.c_str());}
        }
        file = root.openNextFile();
    }
    if(SD_outbuf=="") SD_outbuf+"\n"; //nothing found
    return SD_outbuf;
}
//**************************************************************************************************
//                               C O N N E C T   TO   W I F I                                      *
//**************************************************************************************************
bool connectToWiFi(){
    String s_ssid = "", s_password = "", s_info = "";
    wifiMulti.addAP(_SSID.c_str(), _PW.c_str());                // SSID and PW in code
    if(f_SD_okay){  // try credentials given in "/networks.txt"
        File file = SD.open("/networks.csv");
        if(file){                                         // try to read from SD
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
    }
    Serial.println("WiFI_info  : Connecting WiFi...");
    if(wifiMulti.run() == WL_CONNECTED){
        _myIP = WiFi.localIP().toString();
        _SSID = WiFi.SSID();
        Serial.printf("WiFI_info  : WiFi connected\n");
        Serial.printf("WiFI_info  : IP address %s\n", _myIP.c_str());
        Serial.printf("WiFI_info  : connected to %s\n", _SSID.c_str());
        WiFi.setSleep(false);
        return true;
    }else{
        Serial.printf("WiFi credentials are not correct\n");
        _SSID = ""; _myIP = "0.0.0.0";
        return false;  // can't connect to any network
    }
}
//**************************************************************************************************
//                                           S E T U P                                             *
//**************************************************************************************************
void setup(){
    mutex_rtc     = xSemaphoreCreateMutex();
    mutex_display = xSemaphoreCreateMutex();
    pinMode(VS1053_CS, OUTPUT);  digitalWrite(VS1053_CS, HIGH);
    Serial.begin(115200);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); // VSPI

    Serial.println("setup      : Init SD card");
    SD.begin(SD_CS);

    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK, TFT_BL);    // Init TFT interface
    tft.setFrequency(TFT_FREQUENCY);
    tft.setRotation(TFT_ROTATION);
    tp.setRotation(TP_ROTATION);

    ir.begin();  // Init InfraredDecoder

    pref.begin("MiniWebRadio", false);  // instance of preferences for defaults (tone, volume ...)
    stations.begin("Stations", false);  // instance of preferences for stations (name, url ...)
    setTFTbrightness(pref.getUInt("brightness"));
    f_SD_okay=(SD.cardType() != CARD_NONE); // See if known card
    if(!f_SD_okay){
        Serial.println("setup       : SD card not found");
        while(1){};                     // endless loop, MiniWebRadio does not work without SD
    }
    else Serial.println("setup      : found SD card");
    File file=SD.open("/stations.csv");
    if(!file){
        Serial.println("setup       : stations.csv not found");
        while(1){};                     // endless loop, MiniWebRadio does not work without stations.csv
    }
    int size= file.size();
    file.close();
    if(stations.getLong("stations.size") != size) defaultsettings();  // first init
    drawImage("/common/MiniWebRadio.jpg", 0, 0); //Welcomescreen
    Serial.println("setup      : Init VS1053");
    vs1053.begin(); // Initialize VS1053 player
    if(!vs1053.printVersion()) Serial.println("setup      : VS1053 not found");
    else Serial.println("setup      : found VS1053");
    _alarmdays=pref.getUInt("alarm_weekday");
    _alarmtime=pref.getString("alarm_time");
    setTFTbrightness(pref.getUInt("brightness"));
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setHostname("MiniWebRadio");
    if(!connectToWiFi()){
        WiFi.disconnect(true);
        if(!connectToWiFi()){ // second try due issue #243, #401
            tft.fillScreen(TFT_BLACK);      // Clear screen
            tft.setFont(Times_New_Roman43x35);
            display_info("can't connect to WiFi, check Your credentials", 20, 220, TFT_YELLOW, 5);
            while(1){};                     // endless loop until reset
        }
    }
    webSrv.begin(80, 81); // HTTP port, WebSocket port
    f_rtc= rtc.begin(TZName);
    tft.fillScreen(TFT_BLACK); // Clear screen
    showHeadlineItem();
    tone();
    vs1053.connecttohost(setStation(pref.getUInt("station"))); //last used station
    if(_station=="") showStation(); // if station gives no icy-name display _stationname
    //startTimer();
    ticker.attach(1, timer1sec);
}
//**************************************************************************************************
//                                       C O M M O N                                               *
//**************************************************************************************************
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

const char* scaleImage(const char* path){
    if((!endsWith(path, "bmp")) && (!endsWith(path, "jpg"))){ // not a image
        return UTF8toASCII(path);
    }
    static char pathBuff[256];
    memset(pathBuff, 0, sizeof(pathBuff));
    char* pch = strstr(path + 1, "/");
    if(pch){
        strncpy(pathBuff, path, (pch - path));
        if(TFT_CONTROLLER <= 2) strcat(pathBuff, "/s"); // small pic,  320x240px
        else                    strcat(pathBuff, "/m"); // medium pic, 480x320px
        strcat(pathBuff, pch);
    }
    else{
        strcpy(pathBuff, "/common");
        if(TFT_CONTROLLER <= 2) strcat(pathBuff, "/s"); // small pic,  320x240px
        else                    strcat(pathBuff, "/m"); // medium pic, 480x320px
        strcat(pathBuff, path);
    }
    return UTF8toASCII(pathBuff);  
}

inline uint8_t downvolume(){
    uint8_t vol; vol=pref.getUInt("volume");
    if(vol>0) {vol--; pref.putUInt("volume", vol); if(f_mute==false) vs1053.setVolume(vol);}
    showHeadlineVolume(vol); return vol;
}
inline uint8_t upvolume(){
    uint8_t vol; vol=pref.getUInt("volume");
    if(vol<21){vol++; pref.putUInt("volume", vol); if(f_mute==false) vs1053.setVolume(vol);}
    showHeadlineVolume(vol); return vol;
}
inline uint8_t getvolume(){
    return pref.getUInt("volume");
}
inline void mute(){
    if(f_mute==false){f_mute=true; vs1053.setVolume(0); showHeadlineVolume(0); webSrv.send("mute=1");}
    else {f_mute=false; vs1053.setVolume(getvolume()); showHeadlineVolume(getvolume()); webSrv.send("mute=0");}
    pref.putUInt("mute", f_mute);
}
inline void showVolumeBar(){
    uint16_t vol=tft.width()* pref.getUInt("volume")/21;
    tft.fillRect(0, _yVolBar, vol, _hVolBar, TFT_RED);
    tft.fillRect(vol+1, _yVolBar, tft.width()-vol+1, _hVolBar, TFT_GREEN);
}
inline void showBrightnessBar(){
    uint16_t br=tft.width()* pref.getUInt("brightness")/100;
    tft.fillRect(0, 140, br, 5, TFT_RED); tft.fillRect(br+1, 140, tft.width()-br+1, 5, TFT_GREEN);
}
inline void StationsItems(){
    webSrv.send("stationNr=" + String(pref.getUInt("station")));
    webSrv.send("stationURL=" + _stationURL);
    webSrv.send("stationName=" + _stationname);
}
//**************************************************************************************************
//                                M E N U E / B U T T O N S                                        *
//**************************************************************************************************
void changeState(int state){
    if(!f_SD_okay) return;
    _state=static_cast<status>(state);
    switch(_state) {
    case RADIO:{
        showFooter(); showHeadlineItem();
        showStation(); showStreamTitle(_title);
        break;
    }
    case RADIOico:{
        _pressBtn[0]="/btn/Button_Mute_Red.jpg";           _releaseBtn[0]="/btn/Button_Mute_Green.jpg";
        _pressBtn[1]="/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1]="/btn/Button_Volume_Down_Blue.jpg";
        _pressBtn[2]="/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2]="/btn/Button_Volume_Up_Blue.jpg";
        _pressBtn[3]="/btn/Button_Previous_Yellow.jpg";    _releaseBtn[3]="/btn/Button_Previous_Green.jpg";
        _pressBtn[4]="/btn/Button_Next_Yellow.jpg";        _releaseBtn[4]="/btn/Button_Next_Green.jpg";
        clearTitle(); clearFooter(); showVolumeBar();
        break;}
    case RADIOmenue:{
        _pressBtn[0]="/btn/MP3_Yellow.jpg";                _releaseBtn[0]="/btn/MP3_Green.jpg";
        _pressBtn[1]="/btn/Clock_Yellow.jpg";              _releaseBtn[1]="/btn/Clock_Green.jpg";
        _pressBtn[2]="/btn/Radio_Yellow.jpg";              _releaseBtn[2]="/btn/Radio_Green.jpg";
        _pressBtn[3]="/btn/Button_Sleep_Yellow.jpg";       _releaseBtn[3]="/btn/Button_Sleep_Green.jpg";
        _pressBtn[4]="/btn/Bulb_Yellow.jpg";               _releaseBtn[4]="/btn/Bulb_Green.jpg";
        clearTitle(); clearFooter();
        break;}
    case CLOCKico:{
        _pressBtn[0]="/btn/MP3_Yellow.jpg";                _releaseBtn[0]="/btn/MP3_Green.jpg";
        _pressBtn[1]="/btn/Bell_Yellow.jpg";               _releaseBtn[1]="/btn/Bell_Green.jpg";
        _pressBtn[2]="/btn/Radio_Yellow.jpg";              _releaseBtn[2]="/btn/Radio_Green.jpg";
        _pressBtn[3]="/btn/Black.jpg";                     _releaseBtn[3]="/btn/Black.jpg";
        _pressBtn[4]="/btn/Black.jpg";                     _releaseBtn[4]="/btn/Black.jpg";
        break;}
    case BRIGHTNESS:{
        _pressBtn[0]="/btn/Button_Left_Yellow.jpg";        _releaseBtn[0]="/btn/Button_Left_Blue.jpg";
        _pressBtn[1]="/btn/Button_Right_Yellow.jpg";       _releaseBtn[1]="/btn/Button_Right_Blue.jpg";
        _pressBtn[2]="/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2]="/btn/Button_Ready_Blue.jpg";
        _pressBtn[3]="/btn/Black.jpg";                     _releaseBtn[3]="/btn/Black.jpg";
        _pressBtn[4]="/btn/Black.jpg";                     _releaseBtn[4]="/btn/Black.jpg";
        break;}
    case PLAYER:{
        _pressBtn[0]="/btn/Radio_Yellow.jpg";              _releaseBtn[0]="/btn/Radio_Green.jpg";
        _pressBtn[1]="/btn/Button_Left_Yellow.jpg";        _releaseBtn[1]="/btn/Button_Left_Blue.jpg";
        _pressBtn[2]="/btn/Button_Right_Yellow.jpg";       _releaseBtn[2]="/btn/Button_Right_Blue.jpg";
        _pressBtn[3]="/btn/Button_Ready_Yellow.jpg";       _releaseBtn[3]="/btn/Button_Ready_Blue.jpg";
        _pressBtn[4]="/btn/Black.jpg";                     _releaseBtn[4]="/btn/Black.jpg";
        break;}
    case PLAYERico:{
        _pressBtn[0]="/btn/Button_Mute_Red.jpg";           _releaseBtn[0]="/btn/Button_Mute_Green.jpg";
        _pressBtn[1]="/btn/Button_Volume_Down_Yellow.jpg"; _releaseBtn[1]="/btn/Button_Volume_Down_Blue.jpg";
        _pressBtn[2]="/btn/Button_Volume_Up_Yellow.jpg";   _releaseBtn[2]="/btn/Button_Volume_Up_Blue.jpg";
        _pressBtn[3]="/btn/MP3_Yellow.jpg";                _releaseBtn[3]="/btn/MP3_Green.jpg";
        _pressBtn[4]="/btn/Radio_Yellow.jpg";              _releaseBtn[4]="/btn/Radio_Green.jpg";
        break;}
    case ALARM:{
        _pressBtn[0]="/btn/Button_Left_Yellow.jpg";        _releaseBtn[0]="/btn/Button_Left_Blue.jpg";
        _pressBtn[1]="/btn/Button_Right_Yellow.jpg";       _releaseBtn[1]="/btn/Button_Right_Blue.jpg";
        _pressBtn[2]="/btn/Button_Up_Yellow.jpg";          _releaseBtn[2]="/btn/Button_Up_Blue.jpg";
        _pressBtn[3]="/btn/Button_Down_Yellow.jpg";        _releaseBtn[3]="/btn/Button_Down_Blue.jpg";
        _pressBtn[4]="/btn/Button_Ready_Yellow.jpg";       _releaseBtn[4]="/btn/Button_Ready_Blue.jpg";
        break;}
    case SLEEP:{
        _pressBtn[0]="/btn/Button_Up_Yellow.jpg";          _releaseBtn[0]="/btn/Button_Up_Blue.jpg";
        _pressBtn[1]="/btn/Button_Down_Yellow.jpg";        _releaseBtn[1]="/btn/Button_Down_Blue.jpg";
        _pressBtn[2]="/btn/Button_Ready_Yellow.jpg";       _releaseBtn[2]="/btn/Button_Ready_Blue.jpg";
        _pressBtn[3]="/btn/Black.jpg";                     _releaseBtn[3]="/btn/Black.jpg";
        _pressBtn[4]="/btn/Button_Cancel_Yellow.jpg";      _releaseBtn[4]="/btn/Button_Cancel_Blue.jpg";
        break;}
    case CLOCK:{ break;}

    }
    if(_state!=RADIO && _state!=CLOCK){ // RADIO and CLOCK have no Buttons
        int j=0;
        if(_state==RADIOico || _state==PLAYERico){  // show correct mute button
            if(f_mute==false) {drawImage(_releaseBtn[0].c_str(), 0, _yBtn); vs1053.loop();}
            else {drawImage(_pressBtn[0].c_str(), 0, _yBtn); vs1053.loop();}
            j=1;}
        for(int i=j; i<5; i++){drawImage(_releaseBtn[i].c_str(), i*_wBtn, _yBtn); vs1053.loop();}
    }
}
void changeBtn_pressed(uint8_t btnNr){
    if(_state!=RADIO && _state!=CLOCK) drawImage(_pressBtn[btnNr].c_str(), btnNr*_wBtn , _yBtn); vs1053.loop();
}
void changeBtn_released(uint8_t btnNr){
    if(_state!=RADIO && _state!=CLOCK) drawImage(_releaseBtn[btnNr].c_str(), btnNr*_wBtn , _yBtn); vs1053.loop();
}
void display_weekdays(uint8_t ad, boolean showall=false){
    uint8_t i=0;
    String str="";
    static uint8_t d, old_d;
    d=ad; //alarmday
    for(i=0;i<7;i++){
        if((d & (1<<i))==(old_d & (1<<i))&&!showall) continue;  //icon is alread displayed
        str="/day/"+String(i);
        if(d & (1<<i))  str+="_rt_en.bmp";    // l<<i instead pow(2,i)
        else            str+="_gr_en.bmp";
        if(f_SD_okay) drawImage(str.c_str(), 5+i*44, 0);
        vs1053.loop();
    }
    old_d=ad;
}
void display_alarmtime(int8_t xy=0, int8_t ud=0, boolean showall=false){
    uint8_t i=0, j[4]={5,77,173,245}, k[4]={0,1,3,4}, ch=0;
    String str="";
    static int8_t pos=0, oldpos=0;;
    static String oldt="";
    if(ud==1){
        ch=_alarmtime[k[pos]]; ch++;
        if(pos==0){if(_alarmtime[1]>51){if(ch==50) ch=48; _alarmtime[k[pos]]=ch;} //hour 0...1
                   else{if(ch==51) ch=48; _alarmtime[k[pos]]=ch;} //hour 0...2
        }
        if(pos==1){if(_alarmtime[0]=='2'){if(ch==52) ch=48; _alarmtime[k[pos]]=ch;} //hour*10 0...3
                   else{if(ch==58) ch=48; _alarmtime[k[pos]]=ch;} //hour*10 0...9
        }
        if(pos==2){if(ch==54) ch=48; _alarmtime[k[pos]]=ch;} //min 0...5
        if(pos==3){if(ch==58) ch=48; _alarmtime[k[pos]]=ch;} //min*10 0...9
    }
    if(ud==-1){
        ch=_alarmtime[k[pos]]; ch--;
        if(pos==0){if(_alarmtime[1]>51){if(ch==47) ch=49; _alarmtime[k[pos]]=ch;}//hour 1...0
                   else{if(ch==47) ch=50; _alarmtime[k[pos]]=ch;} //hour 2...0
        }
        if(pos==1){if(_alarmtime[0]=='2'){if(ch==47) ch=51; _alarmtime[k[pos]]=ch;} //hour*10 0...3
                   else{if(ch==47) ch=57; _alarmtime[k[pos]]=ch;} //hour*10 9...0
        }
        if(pos==2){if(ch==47) ch=53; _alarmtime[k[pos]]=ch;} //min 5...0
        if(pos==3){if(ch==47) ch=57; _alarmtime[k[pos]]=ch;} //min*10 9...0
        }

    if(xy==1) pos++; if(pos==4) pos=0; //pos only 0...3
    if(xy==-1)pos--; if(pos==-1)pos=3;

    if(showall==true){oldt="";
        if(f_SD_okay){drawImage("/digits/ert.bmp", 149, 45);vs1053.loop();}}
    String at=_alarmtime;
    //log_i("at=%s",_alarmtime.c_str());
    if(pos!=oldpos){
        str="/digits/"+String(at.charAt(k[pos]))   +"or.bmp";
        if(f_SD_okay){drawImage(str.c_str(), j[pos],    45);vs1053.loop();}
        str="/digits/"+String(at.charAt(k[oldpos]))+"rt.bmp";
        if(f_SD_okay){drawImage(str.c_str(), j[oldpos], 45);vs1053.loop();}
    }
    for(i=0;i<4;i++){
        if(at[k[i]]!=oldt[k[i]]){
            str="/digits/"+String(at.charAt(k[i]));
            if(i==pos) str+="or.bmp";   //show orange number
            else       str+="rt.bmp";   //show red numbers
            if(f_SD_okay){drawImage(str.c_str(), j[i], 45);vs1053.loop();}
        }
    }
    oldt=at; oldpos=pos;
}
void display_time(boolean showall=false){ //show current time on the TFT Display
    static String t, oldt="";
    static boolean k=false;
    uint8_t i=0;
    uint16_t j=0;
    if(showall==true) oldt="";
    if((_state==CLOCK)||(_state==CLOCKico)){
        t=rtc.gettime_s();
        for(i=0;i<5;i++){
            if(t[i]==':'){if(k==false){k=true; t[i]='d';}else{t[i]='e'; k=false;}}
            if(t[i]!=oldt[i]){
                sprintf(_chbuf,"/digits/%cgn.bmp",t[i]);
                if(f_SD_okay) drawImage(_chbuf, 5+j, 45);
                vs1053.loop();
            }
            if((t[i]=='d')||(t[i]=='e'))j+=24; else j+=72;
        }
        oldt=t;}
}
void display_sleeptime(int8_t ud=0, boolean ready=false){  // set sleeptimer
    uint8_t p=0, ypos[4]={5,54,71,120};
    String   m[]={"0:00", "0:05", "0:10", "0:15", "0:30", "0:45", "1:00", "2:00", "3:00", "4:00", "5:00", "6:00"};
    uint16_t n[]={    0,      5,     10,     15,     30,     45,     60,    120,    180,    240,    300,    360 };
    String str="", color="rt";
    p=pref.getUInt("sleeptime");
    if(ready==true){
        _sleeptime=n[p];
        return;
    }
    if(ud==1){
        if(p<11) p++;
        pref.putUInt("sleeptime", p);
    }
    if(ud==-1){
        if(p>0) p--;
        pref.putUInt("sleeptime", p);
    }
    if(p==0) color="gn";

    String st=m[p];
    str="/digits/"+String(st.charAt(0))+"s" + color + ".bmp";
    if(f_SD_okay){drawImage(str.c_str(), ypos[0],    48);vs1053.loop();}
    str="/digits/ds" + color + ".bmp"; // colon
    if(f_SD_okay){drawImage(str.c_str(), ypos[1],    48);vs1053.loop();}
    str="/digits/"+String(st.charAt(2))+"s" + color + ".bmp";
    if(f_SD_okay){drawImage(str.c_str(), ypos[2],    48);vs1053.loop();}
    str="/digits/"+String(st.charAt(3))+"s" + color + ".bmp";
    if(f_SD_okay){drawImage(str.c_str(), ypos[3],    48);vs1053.loop();}
}
//**************************************************************************************************
//                                           L O O P                                               *
//**************************************************************************************************
void loop() {
    static uint8_t sec=0;
    vs1053.loop();
    if(webSrv.loop()) return; // if true: ignore all other for faster response to web

    ir.loop();
    tp.loop();
    if(f_1sec==true){
        if(f_rtc==true){ // true -> rtc has the current time
            int8_t h=0;
            _time_s=rtc.gettime_s();
            if((f_mute==false)&&(!f_sleeping)){
                if(_time_s.endsWith("59:51")) { // speech the time 9 sec before a new hour is arrived
                    _hour=_time_s.substring(0,2); // extract the hour
                    h=_hour.toInt();
                    h++;
                    if(h==25) h=1;
                    _timefile=3;
                    sprintf (_chbuf, "/voice_time/%03d.mp3", h);
                    vs1053.connecttoSD(_chbuf);
                }
            }
            showHeadlineTime();
            if(wifiMulti.run()!=WL_CONNECTED) {
                Serial.printf("WiFi not connected! run returned");
            }
            else{
                if(WiFi.localIP().toString()!=_myIP){
                    _myIP=WiFi.localIP().toString();
                    _SSID=WiFi.SSID();
                    Serial.printf("Connected to %s\n", _SSID.c_str());
                    Serial.printf("IP is %s\n", _myIP.c_str());
                    showFooter();
                }
            }
        }
        display_time();
        if((f_has_ST==false)&&(_state==RADIO)) sec++; else sec=0; // Streamtitle==""?
        if(sec>6){
            sec=0;
            ST_rep();
        }
        if(_commercial_dur>0){
            _commercial_dur--;
            if((_commercial_dur==2)&&(_state==RADIO))showStreamTitle("");// end of commercial? clear streamtitle
        }
        f_1sec=false;
    }
    if(f_1min==true){
        updateSleepTime();
        f_1min=false;
    }
    if(_alarmtime==rtc.gettime_xs()){ //is alarmtime
        if((_alarmdays>>rtc.getweekday())&1){ //is alarmday
            if(!semaphore) {f_alarm=true; f_mute=false; semaphore=true;} //set alarmflag
        }
    }
    else semaphore=false;

    if(_millis+5000<millis()){  //5sec no touch?
        if(_state==RADIOico)  {_state=RADIO; showStreamTitle(_title); showFooter();      }
        if(_state==RADIOmenue){_state=RADIO; showStreamTitle(_title); showFooter();      }
        if(_state==CLOCKico)  {display_info("",160,79, TFT_BLACK, 0); _state=CLOCK;}
    }

    if(f_alarm){
        // log_i("Alarm");
        f_alarm=false;
        vs1053.connecttoSD("/ring/alarm_clock.mp3");
        vs1053.setVolume(21);
        setTFTbrightness(pref.getUInt("brightness"));
    }

    if(f_mp3eof){
        if(_timefile>0){
            if(_timefile==1){vs1053.connecttoSD("/voice_time/080.mp3"); _timefile--;}  // stroke
            if(_timefile==2){vs1053.connecttoSD("/voice_time/200.mp3"); _timefile--;} // precisely
            if(_timefile==3){vs1053.connecttoSD("/voice_time/O'clock.mp3"); _timefile--;}
        }
        else {
            _title="";
            changeState(RADIO);
            vs1053.connecttohost(_lastconnectedhost);
            vs1053.setVolume(pref.getUInt("volume"));
        }
        f_mp3eof=false;
    }
}
//**************************************************************************************************
//                                            E V E N T S                                          *
//**************************************************************************************************
//Events from vs1053_ext library
void vs1053_showstation(const char *info){              // called from vs1053
    if(strlen(info)) Serial.printf("icy-name   : %s\n", info);
    //log_i("_station %s",info);
    if((_station==String(info))&&((String(info)!=""))) return;
    _station=info;
    _title="";
    //avoid double logo paint, show it when last connected host is received
}
void vs1053_showstreamtitle(const char *info){          // called from vs1053
    _title=info;
    showStreamTitle(info);
}
void vs1053_showstreaminfo(const char *info){           // called from vs1053
//    s_info=info;
//    tft.setTextSize(1);                               // host and port
//    displayinfo(s_info.c_str(), 167, 55, TFT_YELLOW); // show info at position 167
}
void vs1053_eof_mp3(const char *info){                  // end of mp3 file (filename)
    f_mp3eof=true;
    Serial.printf("vs1053_eof : %s\n", info);
}
void vs1053_bitrate(const char *br){
    _bitrate=br;
    if(_state==RADIO)showFooter();
}
void vs1053_info(const char *info) {                    // called from vs1053
    String str=info;
    Serial.print("vs1053_info: ");
    if((str.startsWith("Stream lost"))&&(f_rtc)) Serial.print(String(rtc.gettime())+" ");
    Serial.println(info);                               // all infos
}
void vs1053_commercial(const char *info){               // called from vs1053
    _commercial_dur = atoi(info) / 1000;                // info is the duration of advertising in ms
    _title = "Advertising " + (String) _commercial_dur + "s";
    showStreamTitle(_title);
}
void vs1053_icyurl(const char *info){                   // if the Radio has a homepage, this event is calling
    if(strlen(info)) Serial.printf("icy-url    : %s\n", info);
    String str=info;
    if(str.length()>5){
        _homepage=String(info);
        if(!_homepage.startsWith("http")) _homepage="http://"+_homepage;
    }
}
void vs1053_lasthost(const char *info){                 // really connected URL
    _lastconnectedhost=String(info);
    showStation();
    Serial.printf("lastURL    : %s\n", info);
}
void vs1053_id3data(const char *info){
    Serial.printf("id3data    : %s\n", info);
}
void vs1053_icydescription(const char *info){
    if(strlen(info)) Serial.printf("icy-descr  : %s\n", info);
    sprintf(_chbuf, "icy_description=%s", info);
    webSrv.send(_chbuf);
}
void RTIME_info(const char *info){
    Serial.printf("rtime_info : %s\n", info);
}

//Events from tft library
void tft_info(const char *info){
    Serial.printf("tft_info   : %s\n", info);
}

// Events from IR Library
void ir_res(uint32_t res){
    if(_state==RADIO){
        if(res>stations.getUInt("maxstations")){
            tft.setTextSize(7);
            display_info(String(res).c_str(), _yName, _hName +_hTitle, TFT_RED, 100); //state RADIO
            return;
        }
        else{
            _station="";
            vs1053.connecttohost(setStation(res));//state RADIO
        }
    }
}
void ir_number(const char* num){
    if(_state==RADIO){
        tft.setTextSize(7);
        display_info(num, _yName, _hName +_hTitle, TFT_YELLOW, 100); //state RADIO
    }
}
void ir_key(const char* key){
    switch(key[0]){
        case 'k':   if(_state==SLEEP) {display_sleeptime(0, true); changeState(RADIO);} //OK
                    break;
        case 'r':   upvolume(); if((_state==RADIOico)||(_state==PLAYERico)) showVolumeBar(); // right
                    break;
        case 'l':   downvolume(); if((_state==RADIOico)||(_state==PLAYERico)) showVolumeBar(); // left
                    break;
        case 'u':   if(_state==RADIO) vs1053.connecttohost(setStation(0));  // up
                    if(_state==SLEEP) display_sleeptime(1);
                    break;
        case 'd':   if(_state==RADIO) vs1053.connecttohost(setStation(-1)); // down
                    if(_state==SLEEP) display_sleeptime(-1);
                    break;
        case '#':   if(_state==SLEEP) changeState(RADIO); // #
                    else mute();
                    break;
        case '*':   if(_state==RADIO){tft.fillScreen(TFT_BLACK); changeState(SLEEP); showHeadlineItem();
                    drawImage("common/Night_Gown.bmp",198, 25); display_sleeptime();}  // *
                    break;
        default:    break;
    }
}
// Event from TouchPad
void tp_pressed(uint16_t x, uint16_t y){
    uint8_t yPos=255, y1Pos=255, d=0;
    if(f_sleeping==true) return;    // sleepmode, awake in tp_released()
    _millis=millis();
    if(y<167){
        if(_state==RADIOico) changeState(RADIOmenue);
        if(_state==RADIO) changeState(RADIOico);
        if(_state==CLOCK) changeState(CLOCKico);
        if(_state==BRIGHTNESS){}
        if(y<40){
            switch(x){  //weekdays
                case   0 ...  48: y1Pos=0; break; //So
                case  49 ...  92: y1Pos=1; break; //Mon
                case  93 ... 136: y1Pos=2; break; //Tue
                case 137 ... 180: y1Pos=3; break; //We
                case 181 ... 224: y1Pos=4; break; //Th
                case 225 ... 268: y1Pos=5; break; //Fri
                case 269 ... 319: y1Pos=6; break;}//Sat
            }
    }
    else{
        switch(x){  // icons
            case   0 ...  63: yPos=0; break;
            case  64 ... 127: yPos=1; break;
            case 128 ... 191: yPos=2; break;
            case 192 ... 255: yPos=3; break;
            case 256 ... 319: yPos=4; break;
        }
        changeBtn_pressed(yPos);
    }
    if(_state==RADIOico){
        if(yPos==0){mute(); if(f_mute==false) changeBtn_released(yPos);}
        if(yPos==1){_releaseNr= 1; downvolume(); showVolumeBar();} // Vol-
        if(yPos==2){_releaseNr= 2; upvolume(); showVolumeBar();}   // Vol+
        if(yPos==3){_releaseNr= 3; vs1053.connecttohost(setStation(-1));} // station--
        if(yPos==4){_releaseNr= 4; vs1053.connecttohost(setStation(0)); } // station++
    }
    if(_state==RADIOmenue){
        if(yPos==0){_releaseNr= 5; vs1053.stop_mp3client(); listmp3file();} // MP3
        if(yPos==1){_releaseNr= 6;} // Clock
        if(yPos==2){_releaseNr= 7;} // Radio
        if(yPos==3){_releaseNr= 8;} // Sleep
        if(yPos==4){_releaseNr=16;} // Brightness
    }
    if(_state==CLOCKico){
        if(yPos==0){_releaseNr= 5; listmp3file();} // MP3
        if(yPos==1){_releaseNr= 9;} // Bell
        if(yPos==2){_releaseNr= 7;} // Radio
    }
    if(_state==ALARM){
        if(yPos==0){_releaseNr=11;} // left
        if(yPos==1){_releaseNr=12;} // right
        if(yPos==2){_releaseNr=13;} // up
        if(yPos==3){_releaseNr=14;} // down
        if(yPos==4){_releaseNr=15;} // ready (return to CLOCK)

        if(y1Pos<7){d=(1<<y1Pos);
        if((_alarmdays & d))_alarmdays-=d; else _alarmdays+=d; display_weekdays(_alarmdays);}
    }
    if(_state==BRIGHTNESS){
        if(yPos==0){_releaseNr=17;} // left
        if(yPos==1){_releaseNr=18;} // right
        if(yPos==2){_releaseNr= 7;} // ready (return to RADIO)
    }
    if(_state==PLAYER){
        if(yPos==0){_releaseNr=10;} // Radio
        if(yPos==1){_releaseNr=21;} // left
        if(yPos==2){_releaseNr=22;} // right
        if(yPos==3){_releaseNr=23;} // ready
    }
    if(_state==PLAYERico){
        if(yPos==0){mute(); if(f_mute==false) changeBtn_released(yPos);}
        if(yPos==1){_releaseNr=1; downvolume(); showVolumeBar();} // Vol-
        if(yPos==2){_releaseNr=2; upvolume();   showVolumeBar();} // Vol+
        if(yPos==3){_releaseNr=26;} // MP3
        if(yPos==4){_releaseNr=10; _title=""; changeState(RADIO); vs1053.connecttohost(_lastconnectedhost);} // Radio
    }
    if(_state==SLEEP){
        if(yPos==0){_releaseNr=19;} // sleeptime up
        if(yPos==1){_releaseNr=20;} // sleeptime down
        if(yPos==2){_releaseNr=7; display_sleeptime(0, true);} // ready, return to RADIO
        if(yPos==4){_releaseNr=7;}  // return to RADIO without saving sleeptime
    }
}
void tp_released(){
    static String str="";
    if(f_sleeping==true){ //awake
        setTFTbrightness(pref.getUInt("brightness"));   // restore brightness
        vs1053.setVolume(pref.getUInt("volume"));          // restore volume
        f_sleeping=false;
        return;
    }

    switch(_releaseNr){
    case  1: changeBtn_released(1); break; // Vol-
    case  2: changeBtn_released(2); break; // Vol+
    case  3: changeBtn_released(3); break; // nextstation
    case  4: changeBtn_released(4); break; // previousstation
    case  5: tft.fillScreen(TFT_BLACK);
             changeState(PLAYER); showHeadlineItem();
             tft.setTextSize(4); str=_mp3Name[_mp3Index];
             str=str.substring(str.lastIndexOf("/")+1, str.length()-5); //only filename, get rid of foldername(s) and suffix
             display_info(ASCIItoUTF8(str.c_str()), _yName, _hName, TFT_CYAN, 5); break; //MP3
    case  6: tft.fillScreen(TFT_BLACK); changeState(CLOCK);
             showHeadlineItem(); display_time(true); break;//Clock
    case  7: changeState(RADIO); break;
    case  8: tft.fillScreen(TFT_BLACK); changeState(SLEEP); showHeadlineItem();
             drawImage("common/Night_Gown.bmp",198, 25); display_sleeptime(); break;
    case  9: changeState(ALARM); showHeadlineItem();
             display_weekdays(_alarmdays, true);
             display_alarmtime(0, 0, true); break;
    case 10: _title=""; changeState(RADIO); vs1053.connecttohost(_lastconnectedhost); break;
    case 11: display_alarmtime(-1);    changeBtn_released(0);  break;
    case 12: display_alarmtime(+1);    changeBtn_released(1);  break;
    case 13: display_alarmtime(0, +1); changeBtn_released(2);  break; // alarmtime up
    case 14: display_alarmtime(0, -1); changeBtn_released(3);  break; // alarmtime down
    case 15: pref.putUInt("alarm_weekday", _alarmdays); // ready
             pref.putString("alarm_time", _alarmtime);
             tft.fillScreen(TFT_BLACK); changeState(CLOCK);
             showHeadlineItem();
             display_time(true); break;//Clock
    case 16: tft.fillScreen(TFT_BLACK); changeState(BRIGHTNESS); showHeadlineItem();
             showBrightnessBar(); vs1053.loop();
             drawImage("common/Brightness.bmp",0, 21); break;
    case 17: changeBtn_released(0); downBrightness(); showBrightnessBar(); break;
    case 18: changeBtn_released(1); upBrightness(); showBrightnessBar(); break;
    case 19: display_sleeptime(1);  changeBtn_released(0); break;
    case 20: display_sleeptime(-1); changeBtn_released(1); break;
    case 21: changeBtn_released(1); _mp3Index--; if(_mp3Index==-1) _mp3Index=9;
             str=_mp3Name[_mp3Index];
             while(str.length()==0){_mp3Index--; str=_mp3Name[_mp3Index]; if(_mp3Index==0) break;}
             str=str.substring(str.lastIndexOf("/")+1, str.length()-5); //only filename, get rid of foldername(s) and suffix
             tft.setTextSize(4);
             display_info(ASCIItoUTF8(str.c_str()), _yName, _hName, TFT_CYAN, 5);
             break; // left file--
    case 22: changeBtn_released(2); _mp3Index++; if(_mp3Index>9) _mp3Index=0;
             str=_mp3Name[_mp3Index];
             if(str.length()==0){_mp3Index=0; str=_mp3Name[_mp3Index];}
             str=str.substring(str.lastIndexOf("/")+1, str.length()-5); //only filename, get rid of foldername(s) and suffix
             tft.setTextSize(4);
             display_info(ASCIItoUTF8(str.c_str()), _yName, _hName, TFT_CYAN, 5);
             break; // right file++
    case 23: changeState(PLAYERico); showVolumeBar();
             vs1053.connecttoSD("/"+_mp3Name[_mp3Index]); break; // play mp3file
    case 26: clearTitle(); clearFooter(); changeState(PLAYER); break;
    }
    _releaseNr=0;
}

//Events from websrv
void WEBSRV_onCommand(const String cmd, const String param, const String arg){                    // called from html
    // log_i("HTML_cmd=%s params=%s arg=%s", cmd.c_str(),param.c_str(), arg.c_str());
    uint8_t vol;
    String  str;
    if(cmd=="homepage"){webSrv.send("homepage=" + _homepage); return;}
    if(cmd=="to_listen"){StationsItems(); return;} // via websocket, return the name and number of the current station
    if(cmd=="gettone"){ webSrv.reply(tone()); return;}
    if(cmd=="getmute"){ webSrv.reply(String(int(f_mute))); return;}
    if(cmd=="getstreamtitle"){webSrv.reply(_title); return;}
    if(cmd=="mute") {mute();if(f_mute==true) webSrv.reply("Mute on\n"); else webSrv.reply("Mute off\n"); return;}
    if(cmd=="toneha"){pref.putUInt("toneha",(param.toInt()));webSrv.reply("Treble Gain set"); tone(); return;}
    if(cmd=="tonehf"){pref.putUInt("tonehf",(param.toInt()));webSrv.reply("Treble Freq set"); tone(); return;}
    if(cmd=="tonela"){pref.putUInt("tonela",(param.toInt()));webSrv.reply("Bass Gain set"); tone(); return;}
    if(cmd=="tonelf"){pref.putUInt("tonelf",(param.toInt()));webSrv.reply("Bass Freq set"); tone(); return;}
    if(cmd=="mp3list"){webSrv.reply(listmp3file()); return;}
    if(cmd=="mp3track")     {vs1053.connecttoSD(param); webSrv.reply("OK\n"); return;}
    if(cmd=="uploadfile")   {_filename = param;  return;}
    if(cmd=="upvolume")     {str="Volume is now "; str.concat(vol=upvolume()); webSrv.reply(str); return;}
    if(cmd=="downvolume")   {str="Volume is now "; str.concat(downvolume()); webSrv.reply(str); return;}
    if(cmd=="prev_station") {vs1053.connecttohost(setStation(-1)); return;} // via websocket
    if(cmd=="next_station") {vs1053.connecttohost(setStation(0));  return;} // via websocket
    if(cmd=="set_station")  {vs1053.connecttohost(setStation(param.toInt())); StationsItems(); return;} // via websocket
    if(cmd=="stationURL")   {_stationnr=0; _stationname=""; _title=""; vs1053.connecttohost(param);webSrv.reply("OK\n"); return;}
    if(cmd=="getnetworks")  {webSrv.reply(_SSID+"\n"); return;}
    if(cmd=="ping")         {webSrv.send("pong"); return;}
    if(cmd=="index.html")   {webSrv.show(index_html); return;}
    if(cmd=="favicon.ico")  {webSrv.streamfile(SD, "/favicon.ico"); return;}
    if(cmd.startsWith("SD")){str = cmd.substring(2); webSrv.streamfile(SD, scaleImage(str.c_str())); return;}
    if(cmd=="test")         {sprintf(_chbuf, "free heap: %u, available stream: %d buffer filled: %d, buffer free %d\n", ESP.getFreeHeap(), 
                             vs1053.streamavail(), vs1053.bufferFilled(), vs1053.bufferFree()); webSrv.reply(_chbuf); return;}
 
    log_e("unknown HTMLcommand %s", cmd.c_str());
}
void WEBSRV_onRequest(const String request, uint32_t contentLength){
//    log_i("request %s contentLength %d", request.c_str(), contentLength);
    if(request.startsWith("------")) return;      // uninteresting WebKitFormBoundaryString
    if(request.indexOf("form-data") > 0) return;  // uninteresting Info
    if(request == "fileUpload"){savefile(_filename, contentLength);  return;}
    log_e("unknown request: %s",request.c_str());
}
void WEBSRV_onInfo(String info){                            // called from html
//    Serial.printf("HTML_info  : %s\n", info.c_str());   // infos for debug
}
