/*
 * RTC.h
 *
 *  Created on: 04.08.2017
 *  Updated on: 04.08.2026
 *      Author: Wolle
 */

#ifndef RTIME_H_
#define RTIME_H_

#include "Arduino.h"
#include "Audio.h"
#include "WiFi.h"
#include "time.h"
#include "esp_sntp.h"

extern __attribute__((weak)) void RTIME_info(const char*);

class RTIME{

public:
	RTIME();
	~RTIME();
	void begin(ps_ptr<char> TimeZone);
	bool hasValidTime();
	void stop();
	ps_ptr<char>  gettime_s();
	uint8_t getweekday();
	uint16_t getMinuteOfTheDay();
private:
    std::mutex mutex_rtc;
	ps_ptr<char> RTIME_TZ;
	ps_ptr<char> time_s;
	struct tm timeinfo;
	time_t now;
};

#endif /* RTIME_H_ */
