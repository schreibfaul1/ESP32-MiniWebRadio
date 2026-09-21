
#include "Audio.h"
#include "../settings.h"
#pragma once

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Macro for comfortable calls
#define MWR_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define MWR_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define MWR_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define MWR_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define MWR_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

SemaphoreHandle_t mutex_rtc;
SemaphoreHandle_t mutex_display;
std::mutex mutex_print;
std::deque<ps_ptr<char>> s_logBuffer;

extern RTIME       rtc;

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
enum ir_shift { IR_RIGHT = +100, IR_LEFT = -100, IR_UP = +101, IR_DOWN = -101, IR_RESET = -127 };

enum {
    UNDEFINED = -1,
    NONE = 0,
    RADIO = 1,
    PLAYER = 2,
    DLNA = 3,
    CLOCK = 4,
    BRIGHTNESS = 5,
    ALARMCLOCK = 6,
    SLEEPTIMER = 7,
    STATIONSLIST = 8,
    AUDIOFILESLIST = 9,
    DLNAITEMSLIST = 10,
    BLUETOOTH = 11,
    EQUALIZER = 12,
    SETTINGS = 13,
    IR_SETTINGS = 14,
    RINGING = 15,
    WIFI_SETTINGS = 16,
    SLEEP = 17,
    WEATHER = 18,
};

struct dlnaHistory_s {
    ps_ptr<char> objId;
    ps_ptr<char> name;
    int16_t      maxItems = -1;
    int16_t      childCount = -1;
};
struct releasedArg {
    ps_ptr<char> arg1;
    ps_ptr<char> arg2;
    ps_ptr<char> arg3;
    int16_t      val1 = 0;
    int16_t      val2 = 0;
};

struct timecounter_s {
    float timer = 0;
    float factor = 0;
};

struct irButtons {
    int16_t      val = -1;
    ps_ptr<char> label = {};
};

struct settings_s {
    irButtons    irbuttons[45];
    uint8_t      numOfIrButtons = 0;
    ps_ptr<char> lastconnectedhost = {};
    ps_ptr<char> lastconnectedfile = {};
} s_settings;

struct volume_s {
    uint8_t cur_volume = 21;
    uint8_t ringVolume = 21;
    uint8_t volumeAfterAlarm = 12;
    uint8_t volumeSteps = 21;
} s_volume;

struct bt_emitter_s {
    bool         found = false;
    bool         connect = false;
    bool         enabled = false;
    bool         play = true; // play: true, pause: false
    uint8_t      volume = 0;
    ps_ptr<char> mode = {};
    ps_ptr<char> version = {};
} s_bt_emitter;

struct tone_s {
    int16_t LP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t BP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t HP = 0;  // -40 ... +6 (dB)        audioI2S
    int16_t BAL = 0; // -16...0....+16         audioI2S
} s_tone;

struct i2c_items_s {
    bool es8311_found = false;
    int  es8311_addr = -1;
    bool es7210_found = false;
    int  es7210_addr = -1;
    bool gt911_found = false;
    int  gt911_addr = -1;
    bool ft6x36u_found = false;
    int  ft6x36u_addr = -1;
    bool bh1750_found = false;
    int  bh1750_addr = -1;
    bool tca9554_found = false;
    int  tca9554_addr = -1;
    bool ch422g_found = false;
    int  ch422g_addr = -1;
} s_i2c_items;

struct tag_s {
    ps_ptr<char> none = "";
    ps_ptr<char> arduino = "Arduino:";
    ps_ptr<char> audio_info = "Audio_Info:";
    ps_ptr<char> wifi_info = "WiFi_Info:";
    ps_ptr<char> setup = "Setup:";
    ps_ptr<char> new_host = "New_Host:";
    ps_ptr<char> playlist = "Playlist:";
    ps_ptr<char> sd_card = "SD_Card:";
    ps_ptr<char> file_name = "File_Name:";
    ps_ptr<char> action = "Action:";
    ps_ptr<char> country = "Country:";
    ps_ptr<char> alarm_time = "Alarm_Time:";
    ps_ptr<char> audio_codec = "Audio_Codec:";
    ps_ptr<char> terminal = "Terminal:";
    ps_ptr<char> ftp_server = "FTP_Server:";
    ps_ptr<char> rtime_info = "RTIME_Info:";
    ps_ptr<char> tft_info = "TFT_Info:";
    ps_ptr<char> tp_info = "TP_Info:";
    ps_ptr<char> ir_info = "IR_Info:";
    ps_ptr<char> meteo_info = "Meteo_Info:";
    ps_ptr<char> webserver = "Web_Server:";
    ps_ptr<char> dlna_server = "DLNA_Server:";
    ps_ptr<char> bt_emitter = "BT_Emitter:";
    ps_ptr<char> sys_info = "System_Info:";
    ps_ptr<char> recorder = "Recorder:";
} s_tag;

struct status_items {
    ps_ptr<char> name;
    ps_ptr<char> hl_name;
    uint16_t     headline_color1;
    uint16_t     headline_color2;
};

status_items statusItems[20] = {
    {"NONE", "", TFT_BLACK, TFT_BLACK},
    {"RADIO", "Internet Radio", TFT_RED, TFT_BLACK},
    {"PLAYER", "Audio Player", TFT_DARKGREEN, TFT_BLACK},
    {"DLNA", "DLNA", TFT_DARKYELLOW, TFT_BLACK},
    {"CLOCK", "Clock", TFT_DARKBROWN, TFT_BLACK},
    {"BRIGHTNESS", "Brightness", TFT_BLACK, TFT_BLACK},
    {"ALARMCLOCK", "Alarm Clock (hh:mm)", TFT_BROWN, TFT_BROWN},
    {"SLEEPTIMER", "Off Timer (h:mm)", TFT_DARKBLUE, TFT_DARKBLUE},
    {"STATIONSLIST", "Stations List", TFT_BLACK, TFT_BLACK},
    {"AUDIOFILESLIST", "Audio Files", TFT_BLACK, TFT_BLACK},
    {"DLNAITEMSLIST", "DLNA List", TFT_BLACK, TFT_BLACK},
    {"BLUETOOTH", "Bluetooth", TFT_BLACK, TFT_BLACK},
    {"EQUALIZER", "Equalizer", TFT_BLACK, TFT_BLACK},
    {"SETTINGS", "Settings", TFT_DARKMAGENTA, TFT_BLACK},
    {"IR_SETTINGS", "IR Settings", TFT_BLACK, TFT_BLACK},
    {"RINGING", "Ringing", TFT_BLACK, TFT_BLACK},
    {"WIFI_SETTINGS", "WiFi Settings", TFT_BLACK, TFT_BLACK},
    {"SLEEP", "", TFT_BLACK, TFT_BLACK},
    {"WEATHER", "Weather Clock", TFT_BLACK, TFT_DARKBROWN},
    {"UNDEFINED", "", TFT_BLACK, TFT_BLACK},
};

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
template <typename... Args> void printfln(ps_ptr<char> tag, const char* fmt, Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_print);
    if (s_logBuffer.size() == 1024) s_logBuffer.pop_back();

    ps_ptr<char> myLog;
    myLog.reserve(200);
    rtc.hasValidTime() ? myLog.append(rtc.gettime_s()) : myLog.append("00:00:00");
    myLog.appendf(" {} ", tag);
    while (myLog.strlen() < 25) { myLog.append("."); }
    myLog.append(" ");
    myLog.append(" \033[0m");
    myLog.appendf(fmt, std::forward<Args>(args)...);
    myLog.append("\033[0m\r\n");
    printf("%s", myLog.c_get());
    s_logBuffer.insert(s_logBuffer.begin(), std::move(myLog));
    myLog.reset();
}

template <typename... Args> void printfcr(ps_ptr<char> tag, const char* fmt, Args&&... args) {
    std::lock_guard<std::mutex> lock(mutex_print);
    if (s_logBuffer.size() == 1024) s_logBuffer.pop_back();
    ps_ptr<char> myLog;
    myLog.reserve(200);
    rtc.hasValidTime() ? myLog.assignf("{}", rtc.gettime_s()) : myLog.assign("00:00:00");
    myLog.appendf(" {} ", tag);
    while (myLog.strlen() < 25) { myLog.append("."); }
    myLog.append(" ");
    myLog.append(" \033[0m");
    myLog.appendf(fmt, std::forward<Args>(args)...);
    myLog.append("\033[0m                                                                                                                                                               \r");
    printf("%s", myLog.c_get());
    s_logBuffer.insert(s_logBuffer.begin(), std::move(myLog));
    myLog.reset();
}

inline void printflnCut(ps_ptr<char> tag, ps_ptr<char> item, const char* color, ps_ptr<char> str) {
    uint8_t maxLength = 100;
    if (str.strlen() > maxLength) {
        ps_ptr<char> tmp1 = str.substr(0, 70);
        ps_ptr<char> tmp2 = str.substr(str.strlen() - 20);
        str.assignf("{}...{}", tmp1, tmp2);
    }
    printfln(tag, "{}{}{}", item, color, str);
}

int log_redirect_handler(const char* format, va_list args) {
    va_list args_len;
    va_copy(args_len, args);
    char probe[1];
    int  len = vsnprintf(probe, sizeof(probe), format, args_len);
    va_end(args_len);
    if (len < 0) return 0;
    len += 1;
    // Puffer fuer die formatierte Nachricht
    ps_ptr<char> log_buffer;
    log_buffer.alloc(len);
    char* log_dst = log_buffer.get();
    if (!log_dst) return 0;
    va_list args_msg;
    va_copy(args_msg, args);
    vsnprintf(log_dst, len, format, args_msg);
    va_end(args_msg);
    if (len > 0) {
        // 0x1B 0x5B 0x30 0x3B 0x33 0x32 0x6D 0x49 0x20 0x28    0x31 0x35 0x33 0x37 0x29 0x20 0x41 0x52 0x44 0x55    0x49 0x4E 0x4F 0x3A 0x20
        //  ESC  [    0    ;    3    2    m    I         (       1    5    3    7    )         A    R    D    U       I    N    O    :
        int  idx = log_buffer.index_of("ARDUINO:");
        char c = log_buffer[7]; // 0...7 is ANSI_ESC_CODE
        if (idx > 0) {
            idx += 9; // after "ARDUINO: "
            log_buffer.remove_before(idx, true);
            log_buffer.truncate_at(log_buffer.strlen() - 1); // remove '\n'
            if (c == 'E') log_buffer.insert(ANSI_ESC_RED, 0);
            if (c == 'W') log_buffer.insert(ANSI_ESC_YELLOW, 0);
            if (c == 'I') log_buffer.insert(ANSI_ESC_GREEN, 0);
            if (c == 'D') log_buffer.insert(ANSI_ESC_CYAN, 0);
            if (c == 'V') log_buffer.insert(ANSI_ESC_GREY, 0);
            printfln(s_tag.arduino, "{}", log_buffer);
        } else {
            printfln(s_tag.none, "{}", log_buffer);
        }
    }
    return 0;
}
