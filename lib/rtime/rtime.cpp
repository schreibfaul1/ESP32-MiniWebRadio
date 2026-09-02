/*
 * rtime.cpp
 *
 */

#include "rtime.h"

extern __attribute__((weak)) void RTIME_info(const char*) {}

RTIME::RTIME() {
    timeinfo = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    now = 0;
}
RTIME::~RTIME() {
    esp_sntp_stop();
}
void RTIME::begin(ps_ptr<char> TimeZone) {
    stop();                                                         // if sntp is already running after restart: assert failed: sntp_setoperatingmode
    if (!TimeZone.valid()) TimeZone = "CET-1CEST,M3.5.0,M10.5.0/3"; // default
    RTIME_TZ = TimeZone;
    RTIME_info("Initializing SNTP");
    // in platformio.ini:
    // -D NTP_Pool_1='"europe.pool.ntp.org"'
    // -D NTP_Pool_2='"pool.ntp.org"'
    // -D NTP_Pool_3='"time-a-g.nist.gov"'
    esp_sntp_setoperatingmode(esp_sntp_operatingmode_t(SNTP_OPMODE_POLL));
    configTzTime(RTIME_TZ.c_get(), NTP_Pool_1, NTP_Pool_2, NTP_Pool_3);
}

void RTIME::stop() {
    esp_sntp_stop();
}

bool RTIME::hasValidTime() {
    return (timeinfo.tm_year < (2016 - 1900)) ? false : true;
}

ps_ptr<char> RTIME::gettime_s() {
    std::lock_guard<std::mutex> lock(mutex_rtc);
    time_t    now;
    struct tm timeinfo;

    if (time(&now) == -1) { // Get current time
        time_s = "Error: time failed";
        return time_s;
    }
    if (localtime_r(&now, &timeinfo) == nullptr) { // Convert Local Time
        time_s = "Error: localtime_r failed";
        return time_s;
    }
    time_s.assignf("{:02}:{:02}:{:02}", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return time_s;
}

uint8_t RTIME::getweekday() { // So=0, Mo=1 ... Sa=6
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_wday;
}

uint16_t RTIME::getMinuteOfTheDay() { // counts at 00:00, from 0...23*60+59
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_hour * 60 + timeinfo.tm_min;
}

RTIME::rtime RTIME::get_rtime(){
    m_rtime.year = timeinfo.tm_year;
    m_rtime.month = timeinfo.tm_mon;
    m_rtime.day = timeinfo.tm_mday;
    m_rtime.hour = timeinfo.tm_hour;
    m_rtime.minute = timeinfo.tm_min;
    m_rtime.second = timeinfo.tm_sec;
    return m_rtime;
}