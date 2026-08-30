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
    enum { IDLE, REQUEST, DATA };

    ps_ptr<char>        m_latitude;
    ps_ptr<char>        m_longitudde;
    NetworkClientSecure m_client;
};