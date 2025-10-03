#include "DLNAClient.h"

// Created on: 30.11.2023
// Updated on: 02.10.2025

DLNA_Client::DLNA_Client() {
    m_state = IDLE;
    m_chunked = false;
}

DLNA_Client::~DLNA_Client() {
    m_dlnaServer.clear();
    m_srv_items.clear();
    m_content.clear();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::seekServer() {
    if (WiFi.status() != WL_CONNECTED) return false; // guard

    uint8_t    ret = 0;
    const char searchTX[] = "M-SEARCH * HTTP/1.1\r\n"
                            "HOST: 239.255.255.250:1900\r\n"
                            "MAN: \"ssdp:discover\"\r\n"
                            "MX: 3\r\n"
                            "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n\r\n";

    {
        ret = m_udp_client.beginMulticast(IPAddress(SSDP_MULTICAST_IP), SSDP_LOCAL_PORT);
        if (!ret) {
            m_udp_client.stop();
            DLNA_LOG_ERROR("error sending SSDP multicast packets");
            return false;
        }
        for (int i = 0; i < 3; i++) {
            ret = m_udp_client.beginPacket(IPAddress(SSDP_MULTICAST_IP), SSDP_MULTICAST_PORT);
            if (!ret) {
                DLNA_LOG_ERROR("udp beginPacket error");
                return false;
            }
            ret = m_udp_client.write((const uint8_t*)searchTX, strlen(searchTX));
            if (!ret) {
                DLNA_LOG_ERROR("udp write error");
                return false;
            }
            ret = m_udp_client.endPacket();
            if (!ret) {
                DLNA_LOG_ERROR("endPacket error");
                return false;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    m_state = SEEK_SERVER;
    m_timeStamp = millis();
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t DLNA_Client::listServer() {
    if (m_state == SEEK_SERVER) return -1; // seek in progress

    msg_s msg;
    msg.e = evt_server;
    msg.server = &m_dlnaServer;
    if (m_dlna_callback) { m_dlna_callback(msg); }
    return m_dlnaServer.size();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const std::deque<DLNA_Client::dlnaServer>& DLNA_Client::getServer() const {
    return m_dlnaServer;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const std::deque<DLNA_Client::srvItem>& DLNA_Client::getBrowseResult() const {
    return m_srv_items;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void DLNA_Client::parseDlnaServer(uint16_t len) {

    ps_ptr<char> buff;
    buff.calloc(len + 1);

    vTaskDelay(200);
    m_udp_client.read((uint8_t*)buff.get(), len); // read packet into the buffer
    char* p = strcasestr(buff.get(), "Location: http");
    if (!p) return;
    int idx1 = indexOf(p, "://", 0) + 3; // pos IP
    int idx2 = indexOf(p, ":", idx1);    // pos ':'
    int idx3 = indexOf(p, "/", idx2);    // pos '/'
    int idx4 = indexOf(p, "\r", idx3);   // pos '\r'
    *(p + idx2) = '\0';
    *(p + idx3) = '\0';
    *(p + idx4) = '\0';
    for (int i = 0; i < m_dlnaServer.size(); i++) {
        if (strcmp(m_dlnaServer[i].ip.c_get(), p + idx1) == 0) { // same IP
            if (m_dlnaServer[i].port == atoi(p + idx2 + 1)) {    // same port
                return;
            }
        }
    }
    if (strcmp(p + idx1, "0.0.0.0") == 0) {
        DLNA_LOG_ERROR("invalid IP address found %s", p + idx1);
        return;
    }

    dlnaServer item;
    item.ip.assign(p + idx1);
    item.port = atoi(p + idx2 + 1);
    item.location.assign(p + idx3 + 1);
    item.controlURL.assign("?");
    item.friendlyName.assign("?");
    item.presentationPort = 0;
    item.presentationURL.assign("?");
    m_dlnaServer.push_back(std::move(item));
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::srvGet(uint8_t srvNr) {
    ps_ptr<char> out_msg;
    ps_ptr<char> buff;
    bool         ret = false;
    m_tcp_client.stop();
    m_tcp_client.setTimeout(CONNECT_TIMEOUT);
    uint32_t t = millis();
    ret = m_tcp_client.connect(m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port);
    if (!ret) {
        m_tcp_client.stop();
        DLNA_LOG_ERROR("The server %s:%d did not answer within %lums", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port, millis() - t);
        return false;
    }
    t = millis() + 250;
    while (true) {
        if (m_tcp_client.connected()) break;
        if (t < millis()) {
            DLNA_LOG_ERROR("The server %s:%d refuses the connection [%s:%d]", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port, __FILENAME__, __LINE__);
            return false;
        }
    }
    // assemble HTTP header
    buff.assignf("GET /%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nUser-Agent: ESP32/Player/UPNP1.0\r\n\r\n", m_dlnaServer[srvNr].location.c_get(), m_dlnaServer[srvNr].ip.c_get(),
                 m_dlnaServer[srvNr].port);
    m_tcp_client.clear();
    m_tcp_client.print(buff.get());
    t = millis() + AVAIL_TIMEOUT;
    while (true) {
        if (m_tcp_client.available()) break;
        if (t < millis()) {
            DLNA_LOG_ERROR("The server %s:%d is not responding after request [%s:%d]", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port, __FILENAME__, __LINE__);
            return false;
        }
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::readHttpHeader() {

    m_timeStamp = millis();
    uint32_t ctime = millis();
    uint32_t stime = millis();
    uint16_t timeout = 4500; // ms
    bool     f_time = false;

    m_chunked = false;
    m_contentlength = 0;

    if (m_tcp_client.available() == 0) {
        if (!f_time) {
            stime = millis();
            f_time = true;
        }
        if ((millis() - stime) > timeout) {
            DLNA_LOG_ERROR("timeout");
            f_time = false;
            return false;
        }
    }
    f_time = false;

    ps_ptr<char> rhl;
    rhl.alloc(1024, "rhl"); // responseHeaderline
    rhl.clear();
    bool ct_seen = false;

    while (true) { // outer while
        uint16_t pos = 0;
        if ((millis() - ctime) > timeout) {
            DLNA_LOG_ERROR("timeout");
            goto exit;
        }
        while (m_tcp_client.available()) {
            uint8_t b = m_tcp_client.read();
            if (b == '\n') {
                if (!pos) { // empty line received, is the last line of this responseHeader
                    if (ct_seen)
                        goto lastToDo;
                    else {
                        if (!m_tcp_client.available()) {
                            vTaskDelay(10);
                            goto exit;
                        }
                    }
                }
                break;
            }
            if (b == '\r') rhl[pos] = 0;
            if (b < 0x20) continue;
            rhl[pos] = b;
            pos++;
            if (pos == 1023) {
                pos = 1022;
                continue;
            }
            if (pos == 1022) {
                rhl[pos] = '\0';
                DLNA_LOG_WARN("responseHeaderline overflow");
            }
        } // inner while
        if (!pos) {
            vTaskDelay(5);
            continue;
        }

        DLNA_LOG_DEBUG("%s", rhl.get());
        if (rhl.starts_with_icase("content-length:")) {
            rhl.remove_before(':', false);
            m_contentlength = rhl.to_uint32();
            DLNA_LOG_DEBUG("content-length: %lu", (long unsigned int)m_contentlength);
        } else if (rhl.starts_with_icase("content-type:")) { // content-type: text/html; charset=UTF-8
            int idx = indexOf(rhl.get() + 13, ";", 0);
            if (idx > 0) rhl[13 + idx] = '\0';
            if (indexOf(rhl.get() + 13, "text/xml", 0) > 0)
                ct_seen = true;
            else if (indexOf(rhl.get() + 13, "text/html", 0) > 0)
                ct_seen = true;
            else {
                DLNA_LOG_ERROR("content type expected: text/xml or text/html, got %s", rhl.get() + 13);
                goto exit; // wrong content type
            }
        } else if ((rhl.starts_with_icase("transfer-encoding:"))) {
            if (rhl.ends_with_icase("chunked")) { // Station provides chunked transfer
                m_chunked = true;
            }
        } else {
            ;
        }
    } // outer while

exit:
    if (!m_contentlength) DLNA_LOG_ERROR("contentlength is not given");
    if (!ct_seen) DLNA_LOG_ERROR("content type not found");
    return true;

lastToDo:
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t DLNA_Client::getChunkSize(uint16_t* readedBytes) {
    std::string chunkLine;
    uint32_t    timeout = 2000; // ms
    uint32_t    ctime = millis();
    int32_t     transportLimit = 0;

    while (true) {
        if ((millis() - ctime) > timeout) {
            DLNA_LOG_ERROR("chunkedDataTransfer: timeout");
            return 0;
        }
        if (!m_tcp_client.available()) continue;
        int b = m_tcp_client.read();
        if (b < 0) continue;

        (*readedBytes)++;

        if (b == '\n') break; // End of the line
        if (b == '\r') continue;

        chunkLine += static_cast<char>(b);

        // Detection: if signs are not hexadecimal and not ';'→ No http chunk
        if (!isxdigit(b) && b != ';') {
            // We have no valid HTTP chunk line → assume transport chunking
            m_chunked = false;
            // determine limit from the current data volume + already read bytes
            transportLimit = m_tcp_client.available() + *readedBytes;
            DLNA_LOG_INFO("No http chunked recognized-switch to transport chunking with limit %u", (unsigned)transportLimit);
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
            int ch = m_tcp_client.read();
            if (ch < 0) continue;
            idx++;
        }
    }
    return chunksize;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::readContent() {

    if (!m_contentlength && !m_chunked) {
        DLNA_LOG_ERROR("content_length not given");
        return false;
    }

    uint16_t readedBytes = 0;
    if (m_chunked) { m_contentlength = getChunkSize(&readedBytes); }

    //-------------------------------------------------------------------------------------------------
    auto split_lines = [&](const ps_ptr<char>& buff) -> std::deque<ps_ptr<char>> {
        std::deque<ps_ptr<char>> result;
        const char*              text = buff.get();
        size_t                   len = std::strlen(text);

        size_t start = 0;
        for (size_t i = 0; i <= len; ++i) {
            if (text[i] == '\n' || text[i] == '\0') {
                size_t line_len = (i > start && text[i - 1] == '\r') ? i - start - 1 : i - start;
                if (line_len > 0) { result.emplace_back(text + start, line_len); }
                start = i + 1;
            }
        }
        return result;
    };
    //-------------------------------------------------------------------------------------------------

    ps_ptr<char> buff("chbuff, readContent");
    buff.calloc(m_contentlength + 4);
    m_timeStamp = millis();
    uint8_t  b = 0;
    uint16_t pos = 0;
    uint8_t  cnt = 0;

    while (pos < m_contentlength) { // outer while

        if (m_tcp_client.available()) {
            cnt = 0;
            b = m_tcp_client.read();
            buff[pos] = b;
            pos++;
        } else {
            vTaskDelay(10);
            if (pos == m_contentlength) break;
            cnt++;
            if (cnt == 100) {
                // buff.hex_dump(m_contentlength));
                DLNA_LOG_ERROR("timeout in readContent");
                goto error;
                break;
            }
        }
    }

    buff.replace("</", "\n</");     // also new line
    buff.replace("\r", "");         // remove '\r'
    buff.replace("\n\n", "\n");     // remove emtpy lines
    buff.replace("&lt;", "<");      // lower than
    buff.replace("&gt;", ">");      // greater than
    buff.replace("> ", ">\n");      // new line
    buff.replace("</", "\n</");     // new line
    buff.replace("><", ">\n<");     // also new line
    buff.replace("&quot", "\"");    // quota sign
    buff.replace("&ampamp", "&");   // ampersand
    buff.replace("&ampapos", "'");  // apostrophe
    buff.replace("&ampquot", "\""); // quotation
    buff.trim();
    DLNA_LOG_DEBUG("%s", buff.get());

    m_content.clear(); // Delete all old entries
    m_content = split_lines(buff);

    // for (size_t i = 0; i < m_content.size(); i++) { DLNA_LOG_INFO(m_content[i].get()); }

    return true;

error:
    return false;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::getServerItems(uint8_t srvNr) {
    if (m_dlnaServer.size() == 0) return 0; // return if none detected

    bool gotFriendlyName = false;
    bool gotServiceType = false;
    bool URNschemaFound = false;

    for (int i = 0; i < m_content.size(); i++) {
        m_content[i].trim();
        if (!gotFriendlyName) {
            if (m_content[i].starts_with("<friendlyName>")) {
                m_dlnaServer[srvNr].friendlyName.assign(m_content[i].get() + 14);
                gotFriendlyName = true;
            }
        }
        if (!gotServiceType) {
            if (m_content[i].contains("urn:schemas-upnp-org:service:ContentDirectory:1")) {
                URNschemaFound = true;
                continue;
            }
            if (URNschemaFound) {
                if (m_content[i].starts_with("<controlURL>")) {
                    m_dlnaServer[srvNr].controlURL.assign(m_content[i].get() + 13);
                    gotServiceType = true;
                }
            }
        }
        if (m_content[i].starts_with("<presentationURL>")) {
            m_content[i].remove_before('>', false);
            if (!m_content[i].starts_with("http://")) continue;
            int8_t posColon = (m_content[i].last_index_of(':'));
            if (posColon > 0) { // we have ip and port
                m_content[i][posColon] = '\0';
                m_dlnaServer[srvNr].presentationURL.assign(m_content[i].get() + 7); // add presentationURL(IP)
                m_dlnaServer[srvNr].presentationPort = atoi(m_content[i].get() + posColon + 1);
            } // only ip is given
            else {
                m_dlnaServer[srvNr].presentationURL.assign(m_content[i].get());
            }
        }
    }

    // we finally got all infos we need
    if (m_dlnaServer[srvNr].location && m_dlnaServer[srvNr].location.ends_with("/")) {
        char* tmp = (char*)malloc(strlen(m_dlnaServer[srvNr].location.c_get()) + strlen(m_dlnaServer[srvNr].controlURL.c_get()) + 1);
        strcpy(tmp, m_dlnaServer[srvNr].location.c_get()); // location string becomes first part of controlURL
        strcat(tmp, m_dlnaServer[srvNr].controlURL.c_get());
        m_dlnaServer[srvNr].controlURL.assign(tmp);
        free(tmp);
    }
    if (m_dlnaServer[srvNr].controlURL.c_get() && m_dlnaServer[srvNr].controlURL.starts_with("http://")) { // remove "http://ip:port/" from begin of string
        m_dlnaServer[srvNr].controlURL.remove_before('/', false);
    }
    if (strcmp(m_dlnaServer[srvNr].friendlyName.c_get(), "?") == 0) {
        DLNA_LOG_ERROR("friendlyName %s, [%i]", m_dlnaServer[srvNr].friendlyName.c_get(), srvNr);
        return false;
    }
    if (strcmp(m_dlnaServer[srvNr].controlURL.c_get(), "?") == 0) {
        DLNA_LOG_ERROR("controlURL %s, [%i]", m_dlnaServer[srvNr].controlURL.c_get(), srvNr);
        return false;
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::browseResult() {
    if (!m_content.size()) return false; // guard

    auto makeContentPushBack = [&]() { // lambda, inner function
        srvItem item;
        item.objectId.assign("?");
        item.parentId.assign("?");
        item.isAudio = false;
        item.itemURL.assign("?");
        item.itemSize = 0;
        item.duration.assign("?");
        item.title.assign("?");
        item.childCount = 0;

        m_srv_items.push_back(std::move(item));
    };
    //-----------------------------------------------------------------------------------------
    auto extractAttr = [&](const std::string& line, const std::string& attr) -> ps_ptr<char> {
        ps_ptr<char> res;
        std::string  key = attr + "=\"";
        auto         start = line.find(key);
        if (start == std::string::npos) return res;
        start += key.size();
        auto end = line.find('"', start);
        if (end == std::string::npos) return res;
        res.copy_from(line.substr(start, end - start).c_str());
        return res;
    };
    //-----------------------------------------------------------------------------------------

    m_numberReturned = -1;
    m_totalMatches = 0;
    bool     item1 = false;
    bool     item2 = false;
    uint16_t cNr = 0;
    m_srv_items.clear();

    for (int i = 0; i < m_content.size(); i++) {
        m_content[i].trim();
        DLNA_LOG_DEBUG("%s", m_content[i].get());
        m_content[i].trim();
        /*------C O N T A I N E R -------*/

        if (m_content[i].starts_with("<container id=")) {
            item1 = true;
            makeContentPushBack();
            cNr = m_srv_items.size() - 1;
        }
        if (item1) {
            if (m_content[i].starts_with("</container")) { item1 = false; }
            if (m_content[i].starts_with("<container id=")) {
                std::string line = m_content[i].get();                     // <container id="3" parentID="0" restricted="1" searchable="1" childCount="5">
                auto        idStr = extractAttr(line, "id");               // "3"
                auto        parentStr = extractAttr(line, "parentID");     // "0"
                auto        childCountS = extractAttr(line, "childCount"); // 5

                if (idStr.valid()) {
                    idStr.replace(";", "");
                    m_srv_items[cNr].objectId.clone_from(idStr);
                } // Container-ID als String
                if (parentStr.valid()) m_srv_items[cNr].parentId.clone_from(parentStr); // Parent-ID als String
                if (childCountS.valid()) {
                    childCountS.replace(";", "");
                    m_srv_items[cNr].childCount = childCountS.to_uint32();
                }
            }
            if (m_content[i].starts_with("<dc:title>")) {
                std::string line = m_content[i].get(); // <dc:title>Bilder
                m_srv_items[cNr].title.assign(line.substr(10).c_str());
            }
            continue;
        }

        /*------ I T E M -------*/
        if (m_content[i].starts_with("<item id=")) {
            item2 = true;
            item1 = false;
            makeContentPushBack();
            cNr = m_srv_items.size() - 1;
        }
        if (item2) {
            if (m_content[i].starts_with("</item")) { item2 = false; }
            std::string item = m_content[i].get();                 // <item id="1$5$2C$1$0" parentID="1$5$2C$1" restricted="1" refID="64$1$38">
            auto        idStr = extractAttr(item, "id");           // "1$5$2C$1$0"
            auto        parentStr = extractAttr(item, "parentID"); // "1$5$2C$1"
            if (idStr.valid()) {
                idStr.replace(";", ""); // <ObjectID>;4:cont1:20:0:0:</ObjectID>
                m_srv_items[cNr].objectId.clone_from(idStr);
            }
            if (parentStr.valid()) m_srv_items[cNr].parentId.clone_from(idStr);

            if (m_content[i].starts_with("<dc:title>")) {
                std::string line = m_content[i].get(); // <dc:title>Bilder
                m_srv_items[cNr].title.assign(line.substr(10).c_str());
            }

            if (m_content[i].starts_with("<upnp:class")) {
                if (m_content[i].index_of("audioItem")) m_srv_items[cNr].isAudio = true; // <upnp:class>object.item.audioItem.musicTrack
            }
            if (m_content[i].starts_with("<res") && m_content[i].contains("audio/")) { // maybe items contains more than one <res>
                std::string res = m_content[i].get();                // <res size="7365" duration="0:00:00.287" bitrate="127706" sampleFrequency="44100" nrAudioChannels="2 ... >http://..."
                auto        itemSize = extractAttr(res, "size");     // "7365"
                auto        duration = extractAttr(res, "duration"); // "0:00:00.287"
                if (itemSize.valid()) {
                    itemSize.replace(";", "");
                    m_srv_items[cNr].itemSize = itemSize.to_uint32();
                } // size as int
                if (duration.valid()) {
                    duration.replace(";", "");
                    if (!duration.equals("?")) m_srv_items[cNr].duration.clone_from(duration); // Duration as String

                    int s = m_content[i].index_of("http:");
                    if (s > 0) m_srv_items[cNr].itemURL.copy_from(m_content[i].get() + s);
                }
            }
        }

        if (m_content[i].starts_with("<NumberReturned>")) {
            std::string line = m_content[i].get(); // <NumberReturned>4
            auto        nr_returned = line.substr(16);
            m_numberReturned = static_cast<int16_t>(std::stoi(nr_returned));
        }
        if (m_content[i].starts_with("<TotalMatches>")) {
            std::string line = m_content[i].get(); // <TotalMatches>4
            auto        total_matches = line.substr(14);
            m_totalMatches = static_cast<int16_t>(std::stoi(total_matches));
        }
    }
    msg_s msg;
    if (m_srv_items.size()) {
        msg.e = evt_content;
        if (m_numberReturned == -1)
            msg.numberReturned = m_srv_items.size();
        else
            msg.numberReturned = m_numberReturned;
        msg.totalMatches = m_totalMatches;
        msg.items = &m_srv_items;

        if (m_dlna_callback) { m_dlna_callback(msg); }
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool DLNA_Client::srvPost(uint8_t srvNr, const char* objectId, const uint16_t startingIndex, const uint16_t maxCount) {

    bool         ret;
    uint8_t      cnt = 0;
    ps_ptr<char> chbuff("chbuff, srvPost");
    ps_ptr<char> message("message");
    m_tcp_client.stop();
    uint32_t t = millis();
    m_tcp_client.setTimeout(CONNECT_TIMEOUT);
    ret = m_tcp_client.connect(m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port);

    if (!ret) {
        m_tcp_client.stop();
        DLNA_LOG_ERROR("The server %s:%d is not responding after %lums", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port, millis() - t);
        return false;
    }
    while (true) {
        if (m_tcp_client.connected()) break;
        delay(100);
        cnt++;
        if (cnt == 10) {
            DLNA_LOG_ERROR("The server %s:%d refuses the connection", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port);
            return false;
        }
    }

    chbuff.assignf("POST /%s HTTP/1.1\r\n"
                   "Host: %s:%d\r\n"
                   "CACHE-CONTROL: no-cache\r\nPRAGMA: no-cache\r\n"
                   "Connection: close\r\n"
                   "Content-Length: 000\r\n" /* dummy length, determine later*/
                   "Content-Type: text/xml; charset=\"utf-8\"\r\n"
                   "SOAPAction: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n"
                   "User-Agent: ESP32/Player/UPNP1.0\r\n"
                   "\r\n", /*end header*/
                   m_dlnaServer[srvNr].controlURL.get(), m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port);

    message.assignf("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
                    "<s:Body>"
                    "<u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n"
                    "<ObjectID>%s</ObjectID>\r\n"
                    "<BrowseFlag>BrowseDirectChildren</BrowseFlag>\r\n"
                    "<Filter>*</Filter>\r\n"
                    "<StartingIndex>%i</StartingIndex>\r\n"   /* startingIndex */
                    "<RequestedCount>%i</RequestedCount>\r\n" /* max count*/
                    "<SortCriteria></SortCriteria>\r\n"
                    "</u:Browse>\r\n"
                    "</s:Body>\r\n"
                    "</s:Envelope>\r\n"
                    "\r\n", /* end message */
                    objectId, startingIndex, maxCount);

    uint16_t msgLength = message.size();
    char     tmp[10];
    itoa(msgLength, tmp, 10);
    chbuff.replace("000", tmp);
    chbuff.append(message.get());

    // DLNA_LOG_DRBUG("%s", chbuff.get());

    m_tcp_client.print(chbuff.get());

    t = millis() + AVAIL_TIMEOUT;
    while (true) {
        if (m_tcp_client.available()) break;
        if (t < millis()) {
            DLNA_LOG_DEBUG("The server %s:%d is not responding after request", m_dlnaServer[srvNr].ip.c_get(), m_dlnaServer[srvNr].port);
            return false;
        }
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t DLNA_Client::browseServer(uint8_t srvNr, const char* objectId, const uint16_t startingIndex, const uint16_t maxCount) {
    if (!objectId) {
        DLNA_LOG_ERROR("objectId is NULL");
        return -1;
    } // no objectId given
    if (srvNr >= m_dlnaServer.size()) {
        DLNA_LOG_ERROR("server index too high");
        return -2;
    } // srvNr too high
    if (m_state != IDLE) {
        DLNA_LOG_ERROR("state is not idle");
        return -3;
    }

    m_srvNr = srvNr;
    strcpy(m_objectId, objectId);
    m_startingIndex = startingIndex;
    m_maxCount = maxCount;
    m_state = BROWSE_SERVER;
    return 0;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* DLNA_Client::stringifyServer() {

    if (m_dlnaServer.size() == 0) {
        m_JSONstr.assign("[]");
        return m_JSONstr.c_get(); // send dummy
    }

    m_JSONstr.assign("[");

    char id[5];
    char port[6];

    for (int i = 0; i < m_dlnaServer.size(); i++) { // build a JSON string in PSRAM, e.g. [{"name":"m","dir":true},{"name":"s","dir":false}]
        itoa(i, id, 10);
        itoa(m_dlnaServer[i].port, port, 10);

        //  [{"srvId":"1","friendlyName":"minidlna","ip":"192.168.178.1","port":"49000""}]

        m_JSONstr.append("{\"srvId\":\"");
        m_JSONstr.append(id);
        m_JSONstr.append("\",\"friendlyName\":\"");
        m_JSONstr.append(m_dlnaServer[i].friendlyName.c_get());
        m_JSONstr.append("\",\"ip\":\"");
        m_JSONstr.append(m_dlnaServer[i].ip.c_get());
        m_JSONstr.append("\",\"port\":\"");
        m_JSONstr.append(port);
        m_JSONstr.append("\"},");
    }
    int posLastComma = m_JSONstr.last_index_of(',');
    m_JSONstr[posLastComma] = ']'; // replace comma by square bracket close
    DLNA_LOG_DEBUG("%s", m_JSONstr.c_get());
    return m_JSONstr.c_get();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* DLNA_Client::stringifyContent() {

    if (m_srv_items.size() == 0) {
        m_JSONstr.assign("[]");
        return m_JSONstr.c_get();
    }

    m_JSONstr.assign("[");

    char childCount[5];
    char isAudio[6];
    char itemSize[12];

    for (int i = 0; i < m_srv_items.size(); i++) { // build a JSON string in PSRAM, e.g. [{"name":"m","dir":true},{"name":"s","dir":false}]
        itoa(m_srv_items[i].childCount, childCount, 10);
        if (m_srv_items[i].isAudio)
            strcpy(isAudio, "true");
        else
            strcpy(isAudio, "false");
        ltoa(m_srv_items[i].itemSize, itemSize, 10);

        // [{"objectId":"1$4","parentId":"1","childCount":"5","title":"Bilder","isAudio":"false","itemSize":"342345","itemURL":"http://myPC/Pictues/myPicture.jpg"},{"objectId ...."}]

        m_JSONstr.append("{\"objectId\":\"");
        m_JSONstr.append(m_srv_items[i].objectId.c_get());
        m_JSONstr.append("\",\"parentId\":\"");
        m_JSONstr.append(m_srv_items[i].parentId.c_get());
        m_JSONstr.append("\",\"childCount\":\"");
        m_JSONstr.append(childCount);
        m_JSONstr.append("\",\"title\":\"");
        m_JSONstr.append(m_srv_items[i].title.c_get());
        m_JSONstr.append("\",\"isAudio\":\"");
        m_JSONstr.append(isAudio);
        m_JSONstr.append("\",\"itemSize\":\"");
        m_JSONstr.append(itemSize);
        m_JSONstr.append("\",\"dur\":\"");
        m_JSONstr.append(m_srv_items[i].duration.c_get());
        m_JSONstr.append("\",\"itemURL\":\"");
        m_JSONstr.append(m_srv_items[i].itemURL.c_get());
        m_JSONstr.append("\"},");
    }
    int posLastComma = m_JSONstr.last_index_of(',');
    m_JSONstr[posLastComma] = ']'; // replace comma by square bracket close
    DLNA_LOG_DEBUG("%s", m_JSONstr.c_get());
    return m_JSONstr.c_get();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t DLNA_Client::getState() {
    return m_state;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void DLNA_Client::loop() {
    static uint8_t cnt = 0;
    static uint8_t fail = 0;
    bool           res;
    switch (m_state) {
        case IDLE: break;
        case SEEK_SERVER:
            if (m_timeStamp + SEEK_TIMEOUT > millis()) {
                int len = m_udp_client.parsePacket();
                if (len > 0) {
                    parseDlnaServer(len); // registers all media servers that respond within the time until the timeout
                }
                cnt = 0;
                fail = 0;
            } else {
                m_udp_client.stop();
                m_state = GET_SERVER_ITEMS;
            }
            break;
        case GET_SERVER_ITEMS:
            if (cnt < m_dlnaServer.size()) {
                if (fail == 3) {
                    fail = 0;
                    DLNA_LOG_ERROR("no response from svr [%i]", cnt);
                    cnt++;
                    break;
                }
                res = srvGet(cnt);
                if (!res) {
                    DLNA_LOG_ERROR("error in srvGet");
                    m_state = IDLE;
                    fail++;
                    break;
                }
                res = readHttpHeader();
                if (!res) {
                    DLNA_LOG_ERROR("error in readHttpHeader");
                    m_state = IDLE;
                    fail++;
                    break;
                }
                res = readContent();
                if (!res) {
                    DLNA_LOG_ERROR("error in readContent");
                    m_state = IDLE;
                    fail++;
                    break;
                }
                res = getServerItems(cnt);
                if (!res) {
                    DLNA_LOG_ERROR("error in readContent");
                    m_state = IDLE;
                    fail++;
                    break;
                }
                cnt++;
                break;
            }
            cnt = 0;
            listServer();
            m_state = IDLE;
            break;
        case BROWSE_SERVER:
            res = srvPost(m_srvNr, m_objectId, m_startingIndex, m_maxCount);
            if (!res) {
                m_state = IDLE;
                break;
            }
            res = readHttpHeader();
            if (!res) {
                m_state = IDLE;
                break;
            }
            res = readContent();
            if (!res) {
                m_state = IDLE;
                break;
            }
            res = browseResult();
            if (!res) {
                m_state = IDLE;
                break;
            }
            cnt = 0;
            m_state = IDLE;
            break;
        default: break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
