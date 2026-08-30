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
    METEO();
    ~METEO();
    void begin();
    bool set_coordinates(ps_ptr<char> latitude, ps_ptr<char> longitude);
    bool send_request(ps_ptr<char> req);
    bool parseHttpResponseHeader();
    void loop();

  private:
    enum { IDLE, RESPONSE_HEADER, DATA };

    std::vector<ps_ptr<char>> readHeader();
    ps_ptr<char>              m_latitude;
    ps_ptr<char>              m_longitude;
    ps_ptr<char>              m_httpRespHdrBuff;
    ps_ptr<char>              m_contentType;
    NetworkClientSecure       m_client;
    uint8_t                   m_status = IDLE;
    bool                      m_f_chunked = false;
    uint32_t                  m_chunkcount = 0;
    uint32_t                  m_contentLength = 0;
};