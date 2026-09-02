#pragma once

/*
 * meteo.h
 *
 *  Created on: 30.08.2026
 *  Updated on: 30.08.2026
 *      Author: Wolle
 */

// web:  https://api.open-meteo.com

#include "Arduino.h"
#include "Audio.h"
#include "WiFi.h"

class METEO {

  public:
    struct MeteoTime {
        uint16_t year;
        uint8_t  month;
        uint8_t  day;
        uint8_t  hour;
        uint8_t  minute;
    };

    struct METEO_HOURLY {
        MeteoTime time;
        float     temperature;
        uint8_t   precipitationProbability;
        uint8_t   cloudCover;
        float     windSpeed;
        uint8_t   sunshineDuration;
        uint8_t   weatherCode;
    };

    struct METEO_DAILY {
        MeteoTime date;
        float     temperatureMax;
        float     temperatureMin;
        uint8_t   weatherCode;
        float     precipitationSum;
        uint8_t   precipitationProbabilityMax;
        MeteoTime sunrise;
        MeteoTime sunset;
        float     windSpeedMax;
    };

  public:
    METEO();
    ~METEO();
    std::vector<METEO_DAILY>  get_daily_data();
    std::vector<METEO_HOURLY> get_hourly_data();
    void                      begin();
    bool                      set_coordinates(ps_ptr<char> latitude, ps_ptr<char> longitude);
    void                      set_timeZone(ps_ptr<char> timeZone);
    bool                      send_request();
    bool                      parseHttpResponseHeader();
    void                      loop();
    bool                      readContent();
    int32_t                   getChunkSize(uint16_t* readedBytes);
    void                      protocol();

    std::vector<METEO_HOURLY> m_hourly;
    std::vector<METEO_DAILY>  m_daily;

  private:
    enum { IDLE, RESPONSE_HEADER, READ_CONTENT };

    const char* findJsonArray(const char* json, const char* object, const char* name);
    bool        readJsonFloat(const char*& p, float& value);
    bool        readJsonString(const char*& p, MeteoTime& dest);
    bool        parseHourly(const char* json);
    bool        parseDaily(const char* json);
    bool        parseForecast(ps_ptr<char>& buff);

    bool                      m_skipCRLF = false;
    std::vector<ps_ptr<char>> readHeader();
    ps_ptr<char>              m_latitude;
    ps_ptr<char>              m_longitude;
    ps_ptr<char>              m_timeZone = "Etc/GMT+0";
    ps_ptr<char>              m_httpRespHdrBuff;
    ps_ptr<char>              m_contentType;
    NetworkClientSecure       m_client;
    uint8_t                   m_status = IDLE;
    bool                      m_f_chunked = false;
    uint32_t                  m_chunkcount = 0;
    uint32_t                  m_contentLength = 0;
};