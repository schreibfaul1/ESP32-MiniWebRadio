/*
 * rtime.cpp
 *
 *  Created on: 04.08.2017
 *      Author: Wolle
 */

#include "rtime.h"

RTIME::RTIME(){
	timeinfo = { 0,0,0,0,0,0,0,0,0 };
	now=0;
}
RTIME::~RTIME(){
	sntp_stop();
}
boolean RTIME::begin(){
    log_i("Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    char sbuf[20]="pool.ntp.org";
    sntp_setservername(0, sbuf);
    sntp_init();
    return obtain_time();
}

boolean RTIME::obtain_time(){
    time_t now = 0;
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        //log_i("Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
    setenv("TZ","CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1); // automatic daylight saving time
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //log_i( "The current date/time in Berlin is: %s", strftime_buf);
    if(retry < retry_count) return true;
    else return false;
}

const char* RTIME::gettime(){
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    //log_i( "The current date/time in Berlin is: %s", strftime_buf);
    return strftime_buf;
}

const char* RTIME::gettime_l(){  // Montag, 04. August 2017 13:12:44
	time(&now);
	localtime_r(&now, &timeinfo);
//    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
//    log_i( "The current date/time in Beriln is: %s", strftime_buf);
	sprintf(strftime_buf,"%s, %02d.%s ", w_day_l[timeinfo.tm_wday].c_str(), timeinfo.tm_mday, month_l[timeinfo.tm_mon].c_str());
	sprintf(strftime_buf,"%s%d %02d:", strftime_buf, timeinfo.tm_year+1900, timeinfo.tm_hour);
	sprintf(strftime_buf,"%s%02d:%02d ", strftime_buf, timeinfo.tm_min, timeinfo.tm_sec);
	return strftime_buf;
}

const char* RTIME::gettime_s(){  // hh:mm:ss
	time(&now);
	localtime_r(&now, &timeinfo);
	sprintf(strftime_buf,"%02d:%02d:%02d",  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
	return strftime_buf;
}

const char* RTIME::gettime_xs(){  // hh:mm
    time(&now);
    localtime_r(&now, &timeinfo);
    sprintf(strftime_buf,"%02d:%02d",  timeinfo.tm_hour, timeinfo.tm_min);
    return strftime_buf;
}

uint8_t RTIME::getweekday(){ //So=0, Mo=1 ... Sa=6
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo.tm_wday;
}


