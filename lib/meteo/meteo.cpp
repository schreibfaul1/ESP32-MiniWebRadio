#include "meteo.h"

METEO::METEO() {}

METEO::~METEO() {}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void METEO::begin() {
    m_client.setInsecure();
    m_httpRespHdrBuff.alloc(4096, "m_httpRespHdrBuff"); // enough space to store http response header
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::set_coordinates(ps_ptr<char> latitude, ps_ptr<char> longitude) {
    m_latitude = latitude;
    m_longitude = longitude;
    if (m_latitude.valid() && m_longitude.valid()) return true;
    return false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void METEO::set_timeZone(ps_ptr<char> timeZone) {
    m_timeZone = timeZone;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::send_request(ps_ptr<char> req) {
    if (m_status != IDLE) return false;

    uint16_t     port = 443;
    ps_ptr<char> rqh;
    ps_ptr<char> tz = "timezone=" + m_timeZone;

    ps_ptr<char> hourly;
    hourly.assign("temperature_2m,");
    hourly.append("precipitation,precipitation_probability,");
    hourly.append("cloud_cover,");
    hourly.append("wind_speed_10m,");
    hourly.append("sunshine_duration,");
    hourly.append("weather_code");

    ps_ptr<char> daily;
    daily.assign("temperature_2m_max,temperature_2m_min,");
    daily.append("weather_code,");
    daily.append("precipitation_sum,precipitation_probability_max,");
    daily.append("sunrise,sunset,");
    daily.append("wind_speed_10m_max");

    ps_ptr<char> user_agent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/152.0.0.0 Safari/537.36";

    rqh.assign("GET /v1/forecast?");
    rqh.appendf("latitude={}&longitude={}", m_latitude, m_longitude);
    rqh.appendf("&hourly={}", hourly);
    rqh.appendf("&daily={}", daily);
    rqh.append(" HTTP/1.1\r\n");
    rqh.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n");
    rqh.append("Accept-Encoding: identity;q=1,*;q=0\r\n");
    rqh.append("Cache-Control: no-cache\r\n");
    rqh.append("Connection: keep-alive\r\n");
    rqh.append("Host: api.open-meteo.com\r\n");
    rqh.appendf("User-Agent: {}\r\n", user_agent);
    rqh.append("\r\n");

    bool res = m_client.connect("api.open-meteo.com", port);
    if (res) {
        // rqh.println();
        m_client.print(rqh.get());
    }

    m_status = RESPONSE_HEADER;
    return res;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::parseHttpResponseHeader() {

    ps_ptr<char> name;
    ps_ptr<char> value;
    m_f_chunked = false;
    m_contentLength = 0;
    auto header = readHeader();

    for (auto& rhl : header) { // read the header line for line
        // rhl.println();
        int colon = rhl.index_of(':');
        if (colon < 0) {
            name = rhl;
            value.reset();
        } else {
            name = rhl.substr(0, colon);
            value = rhl.substr(colon + 1);
            value.trim();
        }
        if (name.starts_with_icase("http/")) { // HTTP status error code
            int sc = atoi(name.get() + 9);
            if (sc > 310) { // e.g. HTTP/1.1 301 Moved Permanently, HTTP/1.1 302 Found
                log_d("%s", name.get());
                m_client.stop();
                m_status = IDLE;
                return false;
            }
        }
        if (name.equals_icase("transfer-encoding")) {
            if (value.ends_with_icase("chunked")) { // Station provides chunked transfer
                m_f_chunked = true;
                m_chunkcount = 0; // Expect chunkcount in DATA
            }
        }
        if (name.equals_icase("content-length")) { m_contentLength = value.to_uint32(); }
        if (name.equals_icase("content-type")) { // content-type: text/html; charset=UTF-8
            int idx = value.index_of(';');
            if (idx > 0) value[idx] = '\0';
            m_contentType = value;
        }
    }

    if (m_f_chunked) log_d("chunked");
    if (m_contentLength) log_d("m_contentLength %i", m_contentLength);
    if (m_contentType.valid()) log_d("ct: %s", m_contentType.c_get());

    m_status = READ_CONTENT;
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void METEO::loop() {
    if (m_status == RESPONSE_HEADER) parseHttpResponseHeader();
    if (m_status == READ_CONTENT) readContent();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
std::vector<ps_ptr<char>> METEO::readHeader() {

    uint16_t                  pos = 0;
    uint32_t                  t = millis() + 3000;
    std::vector<ps_ptr<char>> hdr_lines;
    m_httpRespHdrBuff.clear();

    while (true) { // read the header first and store it in m_httpRespHdrBuff
        int c = m_client.read();
        if (c < 0) {
            vTaskDelay(10);
            if (t < millis()) {
                log_e("timeout");
                hdr_lines.clear();
                return hdr_lines;
            }
            continue;
        }

        if (pos >= m_httpRespHdrBuff.size() - 1) {
            log_w("responseHeaderline overflow");
            m_httpRespHdrBuff[pos] = '\0';
            break;
        }
        m_httpRespHdrBuff[pos++] = c;
        if (m_httpRespHdrBuff.ends_with("\r\n\r\n") || m_httpRespHdrBuff.ends_with("\n\n")) break;
    }

    pos = 0;

    while (true) { // m_httpRespHdrBuff -> vec hdr_lines
        int idx = m_httpRespHdrBuff.index_of('\n', pos);
        if (idx < 0) break;
        ps_ptr<char> line = m_httpRespHdrBuff.substr(pos, idx - pos);
        line.remove_chars("\r");
        if (line.valid()) hdr_lines.push_back(std::move(line));
        pos = idx + 1;
    }
    return hdr_lines;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::readContent() {

    ps_ptr<char> buff;

    //-------------------------------------------------------------------------------------------------
    // HTTP chunked transfer
    //-------------------------------------------------------------------------------------------------
    if (m_f_chunked) {

        // Start with a reasonable size. It will grow automatically if necessary.
        buff.calloc(1024);

        uint32_t pos = 0;

        while (true) {

            uint16_t readedBytes = 0;
            int32_t  chunkSize = getChunkSize(&readedBytes);

            if (chunkSize < 0) {
                log_e("[%s:%i] invalid chunk size", __FILE__, __LINE__);
                goto error;
            }

            // Last chunk
            if (chunkSize == 0) { break; }

            // Make sure there is enough space
            if (pos + chunkSize + 1 > buff.size()) { buff.realloc(pos + chunkSize + 1); }

            uint32_t chunkPos = 0;
            uint32_t timeoutStart = millis();

            while (chunkPos < static_cast<uint32_t>(chunkSize)) {

                if (m_client.available()) {
                    int b = m_client.read();

                    if (b < 0) { continue; }

                    buff[pos++] = static_cast<char>(b);
                    chunkPos++;
                    timeoutStart = millis();
                } else {
                    if ((millis() - timeoutStart) > 2000) {
                        log_e("[%s:%i] timeout while reading chunk", __FILE__, __LINE__);
                        goto error;
                    }
                    vTaskDelay(1);
                }
            }

            // getChunkSize() has set this for a non-zero chunk.
            // Skip CRLF after the chunk data.
            if (m_skipCRLF) {
                uint8_t  count = 0;
                uint32_t timeoutStart = millis();

                while (count < 2) {
                    if (m_client.available()) {
                        int b = m_client.read();
                        if (b < 0) { continue; }
                        count++;
                    } else {
                        if ((millis() - timeoutStart) > 2000) {
                            log_e("[%s:%i] timeout while skipping chunk CRLF", __FILE__, __LINE__);
                            goto error;
                        }
                        vTaskDelay(1);
                    }
                }
                m_skipCRLF = false;
            }
        }
        buff[pos] = '\0';
        //  log_e("[%s:%i] chunked content: %u bytes", __FILE__, __LINE__, pos);
    }

    //-------------------------------------------------------------------------------------------------
    // Normal Content-Length transfer
    //-------------------------------------------------------------------------------------------------
    else {
        buff.calloc(m_contentLength + 4);

        uint8_t  b = 0;
        uint32_t pos = 0;
        uint32_t cnt = 0;

        while (pos < m_contentLength) {
            if (m_client.available()) {
                cnt = 0;
                b = m_client.read();
                buff[pos] = b;
                pos++;
            } else {
                vTaskDelay(10);
                if (pos == m_contentLength) { break; }
                cnt++;
                if (cnt == 300) {
                    log_e("[%s:%i] timeout in readContent", __FILE__, __LINE__);
                    goto error;
                }
            }
        }
        buff[pos] = '\0';
    }

    //-------------------------------------------------------------------------------------------------
    // Process content
    //-------------------------------------------------------------------------------------------------

    // buff.println();

    if (parseForecast(buff)) {
        log_d("Hourly entries: %u", m_hourly.size());
        log_d("Daily entries : %u", m_daily.size());
    }

    m_status = IDLE;
    m_client.stop();
    return true;

error:
    m_status = IDLE;
    m_client.stop();
    return false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t METEO::getChunkSize(uint16_t* readedBytes) {
    std::string chunkLine;
    uint32_t    timeout = 2000; // ms
    uint32_t    ctime = millis();
    int32_t     transportLimit = 0;

    while (true) {
        if ((millis() - ctime) > timeout) {
            log_e("[%s:%i] chunkedDataTransfer: timeout", __FILE__, __LINE__);
            return 0;
        }
        if (!m_client.available()) continue;
        int b = m_client.read();

        if (b < 0) continue;

        (*readedBytes)++;

        if (b == '\n') break; // End of the line
        if (b == '\r') continue;

        chunkLine += static_cast<char>(b);

        // Detection: if signs are not hexadecimal and not ';'→ No http chunk
        if (!isxdigit(b) && b != ';') {
            // We have no valid HTTP chunk line → assume transport chunking
            m_f_chunked = false;
            // determine limit from the current data volume + already read bytes
            transportLimit = m_client.available() + *readedBytes;
            log_i("[%s:%i] No http chunked recognized-switch to transport chunking with limit %i", __FILE__, __LINE__, transportLimit);
            return transportLimit;
        }
    }

    // Extract the hex number (before possibly ';')
    size_t      semicolonPos = chunkLine.find(';');
    std::string hexSize = (semicolonPos != std::string::npos) ? chunkLine.substr(0, semicolonPos) : chunkLine;

    size_t chunksize = strtoul(hexSize.c_str(), nullptr, 16);

    if (chunksize > 0) {
        m_skipCRLF = true; // skip next CRLF after data
    } else {
        // last chunk: read the final CRLF
        uint8_t idx = 0;
        ctime = millis();
        while (idx < 2 && (millis() - ctime) < timeout) {
            int ch = m_client.read();
            if (ch < 0) continue;
            idx++;
        }
    }
    return chunksize;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* METEO::findJsonArray(const char* json, const char* object, const char* name) {
    if (!json || !object || !name) return nullptr;

    char objectSearch[32];
    snprintf(objectSearch, sizeof(objectSearch), "\"%s\":{", object);

    const char* objectStart = strstr(json, objectSearch);
    if (!objectStart) return nullptr;

    char arraySearch[64];
    snprintf(arraySearch, sizeof(arraySearch), "\"%s\":[", name);

    const char* arrayStart = strstr(objectStart, arraySearch);
    if (!arrayStart) return nullptr;

    return arrayStart + strlen(arraySearch);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::readJsonFloat(const char*& p, float& value) {
    if (!p) return false;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') { p++; }
    if (*p == ']') return false;
    char* endPtr = nullptr;
    value = strtof(p, &endPtr);
    if (endPtr == p) return false;
    p = endPtr;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') { p++; }
    if (*p == ',') { p++; }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::readJsonString(const char*& p, MeteoTime& dest) {
    if (!p) return false;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') { p++; }
    if (*p != '"') return false;
    p++; // "

    // YYYY-MM-DD
    if (!isdigit(p[0]) || !isdigit(p[1]) || !isdigit(p[2]) || !isdigit(p[3]) || p[4] != '-' || !isdigit(p[5]) || !isdigit(p[6]) || p[7] != '-' || !isdigit(p[8]) || !isdigit(p[9])) { return false; }
    dest.year = (p[0] - '0') * 1000 + (p[1] - '0') * 100 + (p[2] - '0') * 10 + (p[3] - '0');
    dest.month = (p[5] - '0') * 10 + (p[6] - '0');
    dest.day = (p[8] - '0') * 10 + (p[9] - '0');
    p += 10;

    // Daily: "YYYY-MM-DD"
    if (*p == '"') {
        dest.hour = 0;
        dest.minute = 0;
    }
    // Hourly: "YYYY-MM-DDTHH:MM"
    else if (*p == 'T') {
        p++; // T
        if (!isdigit(p[0]) || !isdigit(p[1]) || p[2] != ':' || !isdigit(p[3]) || !isdigit(p[4])) { return false; }
        dest.hour = (p[0] - '0') * 10 + (p[1] - '0');
        dest.minute = (p[3] - '0') * 10 + (p[4] - '0');
        p += 5;
    } else {
        return false;
    }

    // closing "
    if (*p != '"') return false;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') { p++; }
    if (*p == ',') { p++; }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::parseHourly(const char* json) {

    const char* pTime = findJsonArray(json, "hourly", "time");
    const char* pTemp = findJsonArray(json, "hourly", "temperature_2m");
    const char* pPrec = findJsonArray(json, "hourly", "precipitation_probability");
    const char* pCloud = findJsonArray(json, "hourly", "cloud_cover");
    const char* pWind = findJsonArray(json, "hourly", "wind_speed_10m");
    const char* pSun = findJsonArray(json, "hourly", "sunshine_duration");
    const char* pCode = findJsonArray(json, "hourly", "weather_code");

    if (!pTime || !pTemp || !pPrec || !pCloud || !pWind || !pSun || !pCode) { return false; }

    m_hourly.clear();
    m_hourly.reserve(168);

    while (true) {

        METEO_HOURLY item{};
        // time
        if (!readJsonString(pTime, item.time)) { break; }
        float value;
        // temperature
        if (!readJsonFloat(pTemp, value)) return false;
        item.temperature = value;
        // precipitation probability
        if (!readJsonFloat(pPrec, value)) return false;
        item.precipitationProbability = static_cast<uint8_t>(value);
        // cloud cover
        if (!readJsonFloat(pCloud, value)) return false;
        item.cloudCover = static_cast<uint8_t>(value);
        // wind speed
        if (!readJsonFloat(pWind, value)) return false;
        item.windSpeed = value;
        // sunshine duration
        if (!readJsonFloat(pSun, value)) return false;
        item.sunshineDuration = static_cast<uint8_t>(value / 60);
        // weather code
        if (!readJsonFloat(pCode, value)) return false;
        item.weatherCode = static_cast<uint8_t>(value);
        m_hourly.push_back(item);
    }
    return !m_hourly.empty();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::parseDaily(const char* json) {

    const char* pDate = findJsonArray(json, "daily", "time");
    const char* pMax = findJsonArray(json, "daily", "temperature_2m_max");
    const char* pMin = findJsonArray(json, "daily", "temperature_2m_min");
    const char* pCode = findJsonArray(json, "daily", "weather_code");
    const char* pPrec = findJsonArray(json, "daily", "precipitation_sum");
    const char* pProb = findJsonArray(json, "daily", "precipitation_probability_max");
    const char* pSunrise = findJsonArray(json, "daily", "sunrise");
    const char* pSunset = findJsonArray(json, "daily", "sunset");
    const char* pWind = findJsonArray(json, "daily", "wind_speed_10m_max");

    if (!pDate || !pMax || !pMin || !pCode || !pPrec || !pProb || !pSunrise || !pSunset || !pWind) {
        if (!pDate) log_e("\"time\" not found");
        if (!pMax) log_e("\"temperature_2m_max\" not found");
        if (!pMin) log_e("\"temperature_2m_min\" not found");
        if (!pCode) log_e("\"weather_code\" not found");
        if (!pPrec) log_e("\"precipitation_sum\" not found");
        if (!pProb) log_e("\"precipitation_probability_max\" not found");
        if (!pSunrise) log_e("\"sunrise\" not found");
        if (!pSunset) log_e("\"sunset\" not found");
        if (!pWind) log_e("\"wind_speed_10m_max\" not found");
        return false;
    }

    m_daily.clear();
    m_daily.reserve(7);

    while (true) {

        METEO_DAILY item{};
        // date
        // if (!readJsonString(pDate, item.date, sizeof(item.date))) { break; }
        if (*pDate == ']') { break; } // If time is already at ] -> done
        if (!readJsonString(pDate, item.date)) { return false; }
        float value;
        // max temperature
        if (*pMax == ']') { break; } // If time is already at ] -> done
        if (!readJsonFloat(pMax, value)) { return false; }
        item.temperatureMax = value;
        // min temperature
        if (!readJsonFloat(pMin, value)) { return false; }
        item.temperatureMin = value;
        // weather code
        if (!readJsonFloat(pCode, value)) { return false; }
        item.weatherCode = static_cast<uint8_t>(value);
        // precipitation
        if (!readJsonFloat(pPrec, value)) { return false; }
        item.precipitationSum = value;
        // precipitation probability
        if (!readJsonFloat(pProb, value)) { return false; }
        item.precipitationProbabilityMax = static_cast<uint8_t>(value);
        // sunrise
        if (!readJsonString(pSunrise, item.sunrise)) { return false; }
        // sunset
        if (!readJsonString(pSunset, item.sunset)) { return false; }
        // max wind
        if (!readJsonFloat(pWind, value)) { return false; }
        item.windSpeedMax = value;
        m_daily.push_back(item);
    }
    return !m_daily.empty();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool METEO::parseForecast(ps_ptr<char>& buff) {
    const char* json = buff.get();
    if (!json) return false;
    if (!parseHourly(json)) {
        log_e("Hourly parsing failed");
        return false;
    }
    if (!parseDaily(json)) {
        log_e("Daily parsing failed");
        return false;
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void METEO::protocol() {

    for (int i = 0; i < 100; i++) printf("—");
    printf("\nCoor lat: %s, long:%s, Timezone: %s\n", m_latitude.c_get(), m_longitude.c_get(), m_timeZone.c_get());

    printf(ANSI_ESC_CYAN "\n date       sunrise  sunset   t-max(°C)  t-min(°C)  prec(%%)  prec-sum  wid-speed-max  w-code\n");
    for (int i = 0; i < m_daily.size(); i++) {
        printf(ANSI_ESC_GREEN);
        printf("%04u-%02u-%02u  ", m_daily[i].date.year, m_daily[i].date.month, m_daily[i].date.day);
        printf("%2u:%2u    ", m_daily[i].sunrise.hour, m_daily[i].sunrise.minute);
        printf("%2u:%2u      ", m_daily[i].sunset.hour, m_daily[i].sunset.minute);
        printf("%5.2f     %5.2f      ", m_daily[i].temperatureMax, m_daily[i].temperatureMin);
        printf("%3u     %5.2f        ", m_daily[i].precipitationProbabilityMax, m_daily[i].precipitationSum);
        printf("%5.2f         %2u", m_daily[i].windSpeedMax, m_daily[i].weatherCode);
        printf("\n");
    };
    printf(ANSI_ESC_RESET);

    printf(ANSI_ESC_CYAN "\nYYYY-MM-DD hh:mm cloud(%%) temp(°C)  rain(%%) sun(min)  wind(km/h) w-code\n");
    for (int i = 0; i < min((size_t)24, m_hourly.size()); i++) {
        printf(ANSI_ESC_GREEN "%04i-%02i-%02i %02i:%02i", m_hourly[i].time.year, m_hourly[i].time.month, m_hourly[i].time.day, m_hourly[i].time.hour, m_hourly[i].time.minute);
        printf("   %3i     %5.2f     %3i     %4i     %6.2f      %2u\n", m_hourly[i].cloudCover, m_hourly[i].temperature, m_hourly[i].precipitationProbability, m_hourly[i].sunshineDuration,
               m_hourly[i].windSpeed, m_hourly[i].weatherCode);
    }
    printf(ANSI_ESC_RESET);
    for (int i = 0; i < 90; i++) printf("—");
    printf("\n");

}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
