/*
 * RTC.h
 *
 *  Created on: 04.08.2017
 *  Updated on: 06.01.2019
 *      Author: Wolle
 */

#ifndef RTIME_H_
#define RTIME_H_

#include "WiFi.h"
#include "time.h"
#include "apps/sntp/sntp.h"     // #warning "This header file is deprecated, please include lwip/apps/sntp.h instead." [-Wcpp]
//#include "lwip/apps/sntp.h"   // espressif esp32/arduino V1.0.1-rc2 or higher

extern __attribute__((weak)) void RTIME_info(const char*);

class RTIME{

public:
	RTIME();
	~RTIME();
	boolean begin(String TimeZone="CET-1CEST,M3.5.0,M10.5.0/3");
	const char* gettime();
	const char* gettime_l();
	const char* gettime_s();
	const char* gettime_xs();
	uint8_t getweekday();
protected:
	boolean obtain_time();
private:
	char sbuf[256];
	String RTIME_TZ="";
	struct tm timeinfo;
	time_t now;
	char strftime_buf[64];
	String w_day_l[7]={"Sonntag","Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag"};
	String w_day_s[7]={"So","Mo","Di","Mi","Do","Fr","Sa"};
	String month_l[12]={"Januar","Februar","März","April","Mai","Juni","Juli","August","September","Oktober","November","Dezember"};
	String month_s[12]={"Jan","Feb","März","Apr","Mai","Jun","Jul","Sep","Okt","Nov","Dez"};
};


#endif /* RTIME_H_ */
