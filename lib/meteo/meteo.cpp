#include "meteo.h"

METEO::METEO() {}

METEO::~METEO() {}

void METEO::begin() {
    m_client.setInsecure();
    m_httpRespHdrBuff.alloc(4096, "m_httpRespHdrBuff"); // enough space to store http response header
}

bool METEO::set_coordinates(ps_ptr<char> latitude, ps_ptr<char> longitude) {
    return false;
}

bool METEO::send_request(ps_ptr<char> req) {
    if (m_status != IDLE) return false;

    uint16_t     port = 443;
    ps_ptr<char> rqh;
    ps_ptr<char> param = "current_weather=true";
    ps_ptr<char> user_agent = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/152.0.0.0 Safari/537.36";

    rqh.assignf("GET /v1/forecast?latitude={}&longitude={}&{} HTTP/1.1\r\n", m_latitude, m_longitude, param);
    rqh.append("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n");
    rqh.append("Accept-Encoding: identity;q=1,*;q=0\r\n");
    rqh.append("Cache-Control: no-cache\r\n");
    rqh.append("Connection: keep-alive\r\n");
    rqh.append("Host: api.open-meteo.com\r\n");
    rqh.appendf("User-Agent: {}\r\n", user_agent);
    rqh.append("\r\n");

    bool res = m_client.connect("api.open-meteo.com", port);
    if (res) { m_client.print(rqh.get()); }

    m_status = RESPONSE_HEADER;
    return res;
}

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
                log_w("%s", name.get());
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

    if (m_f_chunked) log_w("chunked");
    if (m_contentLength) log_w("m_contentLength %i", m_contentLength);
    if (m_contentType.valid()) log_w("ct: %s", m_contentType.c_get());

    m_client.stop();
    m_status = IDLE;
    return false;
}

void METEO::loop() {
    if (m_status == RESPONSE_HEADER) parseHttpResponseHeader();
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
