/*
  SoapESP32, a simple library for accessing DLNA media servers with ESP32 devices

  Copyright (c) 2021 Thomas Jentzsch

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished
  to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
    26.03.2023 add callback events
*/

#include "SoapESP32.h"
#include "MiniXPath.h"

// usage of builtin WiFi
#define claimSPI()
#define releaseSPI()

enum eXpath {
    xpFriendlyName = 0,
    xpServiceType,
    xpControlUrl,
    xpBrowseContainer,
    xpBrowseContainerAlt1,
    xpBrowseContainerAlt2,
    xpBrowseItem,
    xpBrowseItemAlt1,
    xpBrowseItemAlt2,
    xpBrowseNumberReturned,
    xpBrowseNumberReturnedAlt1,
    xpBrowseNumberReturnedAlt2,
    xpTitle,
    xpAlbum,
    xpArtist,
    xpClass,
    xpResource
};

xPathParser_t xmlParserPaths[] = {{.num = 3, .tagNames = {"root", "device", "friendlyName"}},
                                  {.num = 5, .tagNames = {"root", "device", "serviceList", "service", "serviceType"}},
                                  {.num = 5, .tagNames = {"root", "device", "serviceList", "service", "controlURL"}},
                                  {.num = 6, .tagNames = {"s:Envelope", "s:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "container"}},
                                  {.num = 6, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "m:BrowseResponse", "Result", "DIDL-Lite", "container"}},
                                  {.num = 6, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "container"}},
                                  {.num = 6, .tagNames = {"s:Envelope", "s:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "item"}},
                                  {.num = 6, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "m:BrowseResponse", "Result", "DIDL-Lite", "item"}},
                                  {.num = 6, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "item"}},
                                  {.num = 4, .tagNames = {"s:Envelope", "s:Body", "u:BrowseResponse", "NumberReturned"}},
                                  {.num = 4, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "m:BrowseResponse", "NumberReturned"}},
                                  {.num = 4, .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "u:BrowseResponse", "NumberReturned"}},
                                  {.num = 1, .tagNames = {"dc:title"}},
                                  {.num = 1, .tagNames = {"upnp:album"}},
                                  {.num = 1, .tagNames = {"upnp:artist"}},
                                  {.num = 1, .tagNames = {"upnp:class"}},
                                  {.num = 1, .tagNames = {"res"}}};

const char *fileTypes[] = {"other", "audio", "picture", "video", ""};

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SoapESP32 Class Constructor
SoapESP32::SoapESP32() {
    m_state = IDLE;
    m_clientDataConOpen = false;
    m_clientDataAvailable = 0;
    m_dlnaServer.size = 0;
    if(!psramInit()) {
        m_chbuf = (char*)malloc(512);
        m_chbufSize = 512;
    }
    else {
        m_PSRAMfound = true;
        m_chbuf = (char*)ps_malloc(2048);
        m_chbufSize = 2048;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// SoapESP32 Class Destructor
SoapESP32::~SoapESP32(){
    dlnaServer_clear_and_shrink();
    vector_clear_and_shrink(m_content);
    if(m_chbuf){free(m_chbuf); m_chbuf = NULL;}
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// searching local network for media servers that offer media content
bool SoapESP32::seekServer(){
    if(WiFi.status() != WL_CONNECTED) return false; // guard
    dlnaServer_clear_and_shrink();
    m_dlnaServer.size = 0;
    uint8_t ret = 0;
    ret = m_udp.beginMulticast(IPAddress(SSDP_MULTICAST_IP), SSDP_LOCAL_PORT);
    if(!ret){
        m_udp.stop(); log_e("error sending SSDP multicast packets");
        return false;
    }
    ret = m_udp.beginPacket(IPAddress(SSDP_MULTICAST_IP), SSDP_MULTICAST_PORT);
    if(!ret){
        log_e("udp beginPacket error");
        return false;
    }
    ret = m_udp.write((const uint8_t*)SSDP_M_SEARCH_TX, sizeof(SSDP_M_SEARCH_TX) - 1);
    if(!ret){
        log_e("udp write error");
        return false;
    }
    ret = m_udp.endPacket();
    if(!ret){
        log_e("endPacket error");
        return false;
    }
    m_state = SEEK_SERVER;
    m_timeStamp = millis();
    m_timeout = SEEK_TIMEOUT;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// called if a media server answered our SSDP multicast query
void SoapESP32::parseDlnaServer(uint16_t len){
    if(len > m_chbufSize - 1) len = m_chbufSize - 1; // guard
    memset(m_chbuf, 0, m_chbufSize);
    vTaskDelay(200);
    m_udp.read(m_chbuf, len); // read packet into the buffer
    char* p = strcasestr(m_chbuf, "Location: http");
    if(!p) return;
    int idx1 = indexOf(p, "://",  0) + 3;  // pos IP
    int idx2 = indexOf(p, ":",  idx1);      // pos ':'
    int idx3 = indexOf(p, "/",  idx2);      // pos '/'
    int idx4 = indexOf(p, "\r", idx3);      // pos '\r'
    *(p + idx2) = '\0';
    *(p + idx3) = '\0';
    *(p + idx4) = '\0';
    for(int i = 0; i< m_dlnaServer.size; i++){
        if(strcmp(m_dlnaServer.ip[i], p + idx1) == 0){log_i("sameIP"); return;}
    }
    m_dlnaServer.ip.push_back(strdup(p + idx1));
    m_dlnaServer.port.push_back(atoi(p + idx2 + 1));
    m_dlnaServer.location.push_back(strdup(p + idx3 + 1));
    m_dlnaServer.controlURL.push_back(NULL);
    m_dlnaServer.friendlyName.push_back(NULL);
    m_dlnaServer.size++;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SoapESP32::srvGet(uint8_t srvNr){
    bool ret;
    uint8_t cnt = 0;
    m_client.stop();
    ret = m_client.connect(m_dlnaServer.ip[srvNr], m_dlnaServer.port[srvNr]);
    if(!ret){
        log_e("The server %s:%d is not responding", m_dlnaServer.ip[srvNr], m_dlnaServer.port[srvNr]);
        return false;
    }
    while(true){
        if(m_client.connected()) break;
        delay(100);
        cnt++;
        if(cnt == 10){
            log_e("The server %s:%d refuses the connection", m_dlnaServer.ip[srvNr], m_dlnaServer.port[srvNr]);
            return false;
        }
    }
    // assemble HTTP header
    sprintf(m_chbuf, "GET /%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\nUser-Agent: ESP32/Player/UPNP1.0\r\n\r\n",
                      m_dlnaServer.location[srvNr], m_dlnaServer.ip[srvNr], m_dlnaServer.port[srvNr]);
    m_client.print(m_chbuf);
    cnt = 0;
    while(true){
        if(m_client.available()) break;
        delay(100);
        cnt++;
        if(cnt == 10){
            log_e("The server %s:%d is not responding after request", m_dlnaServer.ip[srvNr], m_dlnaServer.port[srvNr]);
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SoapESP32::readHttpHeader(){

    bool ct_seen = false;
    m_timeStamp  = millis();
    m_timeout    = 2500; // ms

    while(true){  // outer while
        uint16_t pos = 0;
        if((m_timeStamp + m_timeout) < millis()) {
            log_e("timeout");
            goto error;
        }
        while(m_client.available()) {
            uint8_t b = m_client.read();
            if(b == '\n') {
                if(!pos) {  // empty line received, is the last line of this responseHeader
                    goto exit;
                }
                break;
            }
            if(b == '\r') m_chbuf[pos] = 0;
            if(b < 0x20) continue;
            m_chbuf[pos] = b;
            pos++;
            if(pos == m_chbufSize -1) {
                pos--;
                continue;
            }
            if(pos == m_chbufSize - 2) {
                m_chbuf[pos] = '\0';
                log_i("responseHeaderline overflow");
            }
        } // inner while
        int16_t posColon = indexOf(m_chbuf, ":", 0);  // lowercase all letters up to the colon
        if(posColon >= 0) {
            for(int i = 0; i < posColon; i++) { m_chbuf[i] = toLowerCase(m_chbuf[i]); }
        }
        if(startsWith(m_chbuf, "content-length:")){
            const char* c_cl = (m_chbuf + 15);
            int32_t     i_cl = atoi(c_cl);
            m_contentlength = i_cl;
        //    log_i("content-length: %lu", (long unsigned int)m_contentlength);
        }
        else if(startsWith(m_chbuf, "content-type:")) {  // content-type: text/html; charset=UTF-8
            // log_i("cT: %s", rhl);
            int idx = indexOf(m_chbuf + 13, ";", 0);
            if(idx > 0) m_chbuf[13 + idx] = '\0';
            if(indexOf(m_chbuf + 13, "text/xml", 0) > 0) ct_seen = true;
            else if(indexOf(m_chbuf + 13, "text/html", 0) > 0) ct_seen = true;
            else{
                log_e("content type expected: text/xml or text/html, got %s", m_chbuf + 13);
                goto exit; // wrong content type
            }
        }
        else if((startsWith(m_chbuf, "transfer-encoding:"))) {
            if(endsWith(m_chbuf, "chunked") || endsWith(m_chbuf, "Chunked")) {  // Station provides chunked transfer
                m_chunked = true;
                log_i("chunked data transfer");
                m_chunkcount = 0;  // Expect chunkcount in DATA
            }
        }
        else { ; }
    //    log_w("%s", m_chbuf);
    } // outer while

exit:
    if(!m_contentlength) log_e("contentlength is not given");
    if(!ct_seen) log_e("content type not found");
    return true;

error:
    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SoapESP32::readContent(){

    m_timeStamp  = millis();
    m_timeout    = 2500; // ms
    uint32_t idx = 0;
    uint8_t lastChar = 0;
    uint8_t b = 0;
    vector_clear_and_shrink(m_content);

    while(true){  // outer while
        uint16_t pos = 0;
        if((m_timeStamp + m_timeout) < millis()) {
            log_e("timeout");
            goto error;
        }
        while(m_client.available()) {
            if(lastChar){
                b = lastChar;
                lastChar = 0;
            }
            else{
                b = m_client.read();
                idx++;
            }
            if(b == '\n') {
                m_chbuf[pos] = '\0';
                break;
            }
            if(b == '<' &&  m_chbuf[pos - 1] == '>'){
                lastChar = '<';
                m_chbuf[pos] = '\0';
                break; // simulate new line
            }
            if(b == '\r') m_chbuf[pos] = 0;
            if(b < 0x20) continue;
            m_chbuf[pos] = b;
            pos++;
            if(pos == m_chbufSize -1) {
                pos--;
                continue;
            }
            if(pos == m_chbufSize - 2) {
                m_chbuf[pos] = '\0';
                log_i("line overflow");
            }
        }
//        log_i("%s", m_chbuf);
        m_content.push_back(strdup(m_chbuf));
        if(idx == m_contentlength) break;
        if(!m_client.available()){
            if(m_chunked == true) break; // ok
            if(m_contentlength)   break; // ok
            goto error; // not ok
        }
    }
    m_client.stop();
    return true;

error:
    m_client.stop();
    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// check and examine a discovered media servers for service ContentDirectory
bool SoapESP32::getServerItems(uint8_t srvNr){
    if(m_dlnaServer.size == 0) return 0;  // return if none detected


    bool gotFriendlyName = false;
    bool gotServiceType  = false;
    bool URNschemaFound  = false;

    for(int i = 0; i < m_content.size(); i++){
        uint16_t idx = 0;
        while(*(m_content[i] + idx) == 0x20) idx++;  // same as trim left
        char* content = m_content[i] + idx;

        if(!gotFriendlyName) {
            if(startsWith(content, "<friendlyName>")){
                uint16_t pos = indexOf(content, "<", 14);
                *(content + pos) = '\0';
                if(strlen(content) == 0){
                    m_dlnaServer.friendlyName[srvNr] = (char*)"Server name not provided";
                }
                else{
                    m_dlnaServer.friendlyName[srvNr] = strdup(content + 14);
                }
                gotFriendlyName = true;
                log_w("%s", m_dlnaServer.friendlyName[srvNr]);
            }
        }
        if(!gotServiceType) {
            if(indexOf(content, "urn:schemas-upnp-org:service:ContentDirectory:1", 0) > 0){URNschemaFound = true; continue;}
            if(URNschemaFound){
                if(startsWith(content, "<controlURL>")){
                    uint16_t pos = indexOf(content, "<", 12);
                    *(content + pos) = '\0';
                    m_dlnaServer.controlURL[srvNr] = strdup(content + 13);
                    gotServiceType = true;
                //    log_w("%s", m_dlnaServer.controlURL[srvNr]);
                }
            }
        }
        if(startsWith(content, "<presentationURL>")){
            uint16_t pos = indexOf(content, "<", 17);
            *(content + pos) = '\0';
            char* presentationURL = strdup(content + 17);
        //    log_w("presentationURL %s", presentationURL);
            if(!startsWith(presentationURL, "http://")) continue;
            int8_t posColon = (indexOf(presentationURL, ":", 8));
            if(posColon > 0){ // we have ip and port
                presentationURL[posColon] = '\0';
                free(m_dlnaServer.ip[srvNr]);
                m_dlnaServer.ip[srvNr] = strdup(presentationURL + 7); // replace serverIP with presentationURL(IP)
                m_dlnaServer.port[srvNr] = atoi(presentationURL + posColon + 1);
            } // only ip is given
            else{
                free(m_dlnaServer.ip[srvNr]);
                m_dlnaServer.ip[srvNr] = strdup(presentationURL + 7);
            }
            if(presentationURL){free(presentationURL); presentationURL = NULL;}
        }
    }

    // we finally got all infos we need
    uint16_t idx = 0;
    if(m_dlnaServer.location[srvNr] && endsWith(m_dlnaServer.location[srvNr], "/")){
        char* tmp = (char*)malloc(strlen(m_dlnaServer.location[srvNr]) + strlen(m_dlnaServer.controlURL[srvNr]) + 1);
        strcpy(tmp, m_dlnaServer.location[srvNr]); // location string becomes first part of controlURL
        strcat(tmp, m_dlnaServer.controlURL[srvNr]);
        free(m_dlnaServer.controlURL[srvNr]);
        m_dlnaServer.controlURL[srvNr] = tmp;
        free(tmp);
    }
    if(m_dlnaServer.controlURL[srvNr] && startsWith(m_dlnaServer.controlURL[srvNr], "http://")) { // remove "http://ip:port/" from begin of string
        idx = indexOf(m_dlnaServer.controlURL[srvNr], "/", 7);
        memcpy(m_dlnaServer.controlURL[srvNr], m_dlnaServer.controlURL[srvNr] + idx + 1, strlen(m_dlnaServer.controlURL[srvNr]) + 1 - idx);
    }
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// helper function, client timed read
//
int32_t SoapESP32::soapClientTimedRead(void) {
    int32_t  c;
    uint32_t startMillis = millis();

    do {
        c = m_client.read();

        if(c >= 0) { return c; }
    } while(millis() - startMillis < SERVER_READ_TIMEOUT);

    return -1;  // read timeout
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool SoapESP32::soapUDPmulticast(uint8_t repeats) {                          // send SSDP/UDP multicast packets
    uint8_t ret = m_udp.beginMulticast(IPAddress(SSDP_MULTICAST_IP), 8888);  // 8888: local port
    if(ret) {                                                                // creating socket ok
        uint8_t i = 0;
        while(true) {
            if(!m_udp.beginPacket(IPAddress(SSDP_MULTICAST_IP), SSDP_MULTICAST_PORT) || !m_udp.write((const uint8_t *)SSDP_M_SEARCH_TX, sizeof(SSDP_M_SEARCH_TX) - 1) || !m_udp.endPacket()) { break; }
            if(++i > repeats) return true;
        }
    }
    m_udp.stop();
    log_e("error sending SSDP multicast packets");
    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SoapESP32::soapSSDPquery(int32_t msWait) {  // SSDP/UDP search for media servers in local network
    int32_t   i, port;
    size_t    len;
    IPAddress ip;
    char      tmpBuffer[SSDP_TMP_BUFFER_SIZE], location[SSDP_LOCATION_BUF_SIZE] = "", address[20];

    if(!soapUDPmulticast(1)) return false;  // send SSDP multicast packets (parameter: nr of repeats)

    // evaluate incoming SSDP packets (M-SEARCH replies) & NOTIFY packets if we catch them by chance
    uint32_t start = millis();
    do {
        delay(25);
        len = m_udp.parsePacket();
        if(len) {
            char *p;

            // we received SSDP packet of size len
            log_d("received SSDP packet within %d ms: packet size: %d", millis() - start, len);
            memset(tmpBuffer, 0, SSDP_TMP_BUFFER_SIZE);  // clear buffer
            if(len >= SSDP_TMP_BUFFER_SIZE) len = SSDP_TMP_BUFFER_SIZE - 1;
            m_udp.read(tmpBuffer, len);  // read packet into the buffer

            log_v("SSDP packet content:\n%s", tmpBuffer);

            // scan SSDP packet
            uint8_t scan = 0;
            if(strstr(tmpBuffer, HTTP_HEADER_200_OK)) scan += 1;
            if(strcasestr(tmpBuffer, SSDP_LOCATION)) scan += 2;
            if(strcasestr(tmpBuffer, SSDP_SERVICE_TYPE)) scan += 4;
            if(strstr(tmpBuffer, SSDP_NOTIFICATION)) scan += 8;
            if(strcasestr(tmpBuffer, SSDP_NOTIFICATION_TYPE)) scan += 16;
            if(strcasestr(tmpBuffer, SSDP_NOTIFICATION_SUB_TYPE)) scan += 32;

            if(scan == 0b00000111 || scan == 0b00111010) {  // M-SEARCH reply packets
                p = strcasestr(tmpBuffer, SSDP_LOCATION);
                char format[30];

                strtok(p, "\r\n");
                snprintf(format, sizeof(format), "http://%%[0-9.]:%%d/%%%ds", SSDP_LOCATION_BUF_SIZE - 1);
                if(sscanf(p + 10, format, address, &port, location) < 2) continue;
                if(!ip.fromString(address)) continue;

                // scanning of ip address & port successful, location string can be missing (e.g. D-Link NAS DNS-320L)
                log_d("scanned ip=%s, port=%d, location=\"%s\"", ip.toString().c_str(), port, location);
                if(!strlen(location)) log_d("empty location string!");

                // avoid multiple entries of same server (identical ip & port)
                for(i = 0; i < m_dlnaServer.size; i++) {
                    if(!strcmp(m_dlnaServer.ip[i], ip.toString().c_str()) && m_dlnaServer.port[i] == port) break;
                }
                if(i < m_dlnaServer.size) continue;

                // new server found: add it to list
                m_dlnaServer.controlURL.push_back((char*)"");
                m_dlnaServer.friendlyName.push_back((char*)"");
                m_dlnaServer.ip.push_back(strdup(ip.toString().c_str()));
                m_dlnaServer.location.push_back(strdup(location));
                m_dlnaServer.port.push_back((uint16_t)port);
                m_dlnaServer.size++;
            }
        }
    } while((millis() - start) < msWait);

    m_udp.stop();

    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// read & evaluate HTTP header
//
bool SoapESP32::soapReadHttpHeader(uint64_t *contentLength, bool *chunked) {
    size_t len;
    bool   ok = false;
    char  *p, tmpBuffer[TMP_BUFFER_SIZE_200];

    // first line contains status code

    len = m_client.readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);  // returns length without terminator '\n'

    tmpBuffer[len] = 0;
    if(!strstr(tmpBuffer, HTTP_HEADER_200_OK)) {
        log_e("header line: %s", tmpBuffer);
        return false;
    }
    else {
        log_v("header line: %s", tmpBuffer);
        // TEST
#if CORE_DEBUG_LEVEL == 5
        delay(1);  // allow pending serial monitor output to be sent
#endif
    }
    *contentLength = 0;
    if(chunked) *chunked = false;
    while(true) {
        int32_t av = m_client.available();

        if(!av) break;

        len = m_client.readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);

        tmpBuffer[len] = 0;
        log_v("header line: %s", tmpBuffer);
        if(len == 1) break;  // End of header: finishing line contains only "\r\n"
        if(!ok) {
            if((p = strcasestr(tmpBuffer, HEADER_CONTENT_LENGTH)) != NULL) {
                if(sscanf(p + strlen(HEADER_CONTENT_LENGTH), "%llu", contentLength) == 1) {
                    ok = true;
                    continue;  // continue to read rest of header
                }
            }
            else if(chunked && strcasestr(tmpBuffer, HEADER_TRANS_ENC_CHUNKED)) {
                ok = *chunked = true;
                m_xmlChunkCount = 0;  // telling chunk size comes next
                continue;             // continue to read rest of header
            }
        }
    }
    if(ok) {
        m_xmlReplaceState = xmlPassthrough;
        if(chunked && *chunked) { log_d("HTTP-Header ok, trailing content is chunked, no size announced"); }
        else { log_d("HTTP-Header ok, trailing content is not chunked, announced size: %llu", *contentLength); }
    }

    return ok;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// read XML data, de-chunk if needed & replace predefined XML entities that spoil MiniXPath
//
const replaceWith_t replaceWith[] = {{"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '"'}, {"&amp;amp;", '&'}, {"&amp;apos;", '\''}, {"&amp;quot;", '\''}};

int32_t SoapESP32::soapReadXML(bool chunked, bool replace) {
    bool    match;
    int32_t i, c = -10;

    if(!replace || (replace && (m_xmlReplaceState == xmlPassthrough))) {
    GET_MORE:
        if(!chunked) {
            // data is not chunked
            if((c = soapClientTimedRead()) < 0) {
                // timeout or connection closed
                return -1;
            }
        //    printf("%c", c);
        }
        else {
            // de-chunk XML data
            if(m_xmlChunkCount <= 0) {
                char tmpBuffer[10];

                // next line contains chunk size

                int32_t len = m_client.readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);

                if(len < 2) {
                    return -2;  // we expect at least 1 digit chunk size + '\r'
                }
                tmpBuffer[len - 1] = 0;  // replace '\r' with '\0'
                if(sscanf(tmpBuffer, "%x", &m_xmlChunkCount) != 1) { return -3; }
                log_d("announced chunk size: 0x%x, %d", m_xmlChunkCount, m_xmlChunkCount);
                if(m_xmlChunkCount <= 0) {
                    return -4;  // not necessarily an error...final chunk size can be 0
                }
            }
            if((c = soapClientTimedRead()) < 0) { return -5; }

            // check for end of chunk
            if(--m_xmlChunkCount == 0) {
                // skip "\r\n" trailing each chunk
                if(soapClientTimedRead() < 0 || soapClientTimedRead() < 0) { return -6; }
            }
        }
    }

    // replace predefined XML entities ("&lt;" becomes "<", etc.)
    if(replace) {
        if(m_xmlReplaceState == xmlPassthrough) {
            if(c == '&') {
                memset(m_xmlReplaceBuffer, 0, sizeof(m_xmlReplaceBuffer));
                m_xmlReplaceBuffer[0] = '&';
                m_xmlReplaceState = xmlAmpDetected;
                m_xmlReplaceOffset = 1;
                goto GET_MORE;
            }
        }
        else if(m_xmlReplaceState == xmlAmpDetected) {
            m_xmlReplaceBuffer[m_xmlReplaceOffset++] = c;
            // run through all predefined sequences and see if we still match
            for(match = false, i = 0; i < sizeof(replaceWith) / sizeof(replaceWith_t) && !match; i++) {
                if(strncmp(m_xmlReplaceBuffer, replaceWith[i].replace, m_xmlReplaceOffset) == 0) {
                    match = true;
                    break;
                }
            }
            if(!match) {
                // single '&' or sequence we don't replace
                c = '&';
                m_xmlReplaceState = xmlTakeFromBuffer;
                m_xmlReplaceOffset = 1;
            }
            else {  // match
                if(m_xmlReplaceOffset < strlen(replaceWith[i].replace)) { goto GET_MORE; }
                else {
                    // found full sequence to be replaced
                    c = replaceWith[i].with;
                    m_xmlReplaceState = xmlPassthrough;
                }
            }
        }
        else {
            // xmlTakeFromBuffer
            c = m_xmlReplaceBuffer[m_xmlReplaceOffset++];
            if(m_xmlReplaceBuffer[m_xmlReplaceOffset] == '\0') { m_xmlReplaceState = xmlPassthrough; }
        }
    }
    // TEST
    // Serial.printf("%c", (char)c);
    return c;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------












// erase all entries in server list
//
void SoapESP32::clearServerList() { dlnaServer_clear_and_shrink();}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// helper function: scan for certain attribute
//
bool SoapESP32::soapScanAttribute(const String *attributes, String *result, const char *what) {
    int32_t begin, end;

    if((begin = attributes->indexOf(what)) >= 0 && (end = attributes->indexOf("\"", begin + strlen(what) + 1)) >= begin + strlen(what) + 1) {
        *result = attributes->substring(begin + strlen(what) + 1, end);
        if(result->length() > 0) return true;
    }

    *result = "";  // empty for next call
    log_v("attribute: \"%s\" missing.", what);

    return false;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// scan <container> content in SOAP answer
//
bool SoapESP32::soapScanContainer(const String *parentId, const String *attributes, const String *container) {
    int32_t      i = 0;
    soapObject_t info;
    String       str((char *)0);

    log_d("function entered, parent id: %s", parentId->c_str());

    // scan container id
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_ID)) return false;  // container id is a must
    log_d("%s\"%s\"", DIDL_ATTR_ID, str.c_str());
    info.id = str;

    // scan parent id
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_PARENT_ID)) return false;  // parent id is a must
    if(!strcasestr(str.c_str(), parentId->c_str())) {
#ifdef PARENT_ID_MUST_MATCH
        log_e("scanned parent id \"%s\" != requested parent id \"%s\"", str.c_str(), parentId->c_str());
        return false;
#else
        log_w("scanned parent id \"%s\" != requested parent id \"%s\"", str.c_str(), parentId->c_str());
#endif
    }
    info.parentId = *parentId;
    info.size = 0;
    info.sizeMissing = false;

    // scan child count...not always provided (e.g. Kodi)
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_CHILD_COUNT)) { info.sizeMissing = true; }
    else {
        log_d("%s\"%s\"", DIDL_ATTR_CHILD_COUNT, str.c_str());
        if((info.size = (int32_t)str.toInt()) == 0) {
            // if(dlna_info){
            //     sprintf(m_chbuf, "container \"%s\" child count=0", info.id.c_str());
            //     dlna_info(m_chbuf);
            // }
            log_v("container \"%s\" child count=0", info.id.c_str());
        }
    }

    // scan searchable flag...not always provided (e.g. UMS)
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_SEARCHABLE)) {
        log_w("attribute \"%s\" is missing, we set it true", DIDL_ATTR_SEARCHABLE);
        info.searchable = true;
    }
    else {
        log_d("%s\"%s\"", DIDL_ATTR_SEARCHABLE, str.c_str());
        info.searchable = (1 == (int32_t)str.toInt()) ? true : false;
        if(!info.searchable) {
            // sprintf(m_chbuf, "\"%s\" attribute searchable=0", info.id.c_str());
            // if(dlna_info) dlna_info(m_chbuf);
            // else log_i("%s", m_chbuf);
            log_v("\"%s\" attribute searchable=0", info.id.c_str());
        }
    }

    // scan container name
    MiniXPath xPath;
    xPath.setPath(xmlParserPaths[xpTitle].tagNames, xmlParserPaths[xpTitle].num);
    while(i < container->length()) {
        if(xPath.getValue((char)container->operator[](i), &str)) {
            if(str.length() == 0) return false;  // valid title is a must
            info.name = str;
            log_d("title=\"%s\"", str.c_str());
            break;
        }
        i++;
    }

    // add valid container to result list
    info.isDirectory = true;

    if(dlna_folder) dlna_folder(false, info.name, info.id, info.size);
    if(dlna_item) dlna_item(false, info.name, info.id, info.size, "", true, false);

    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// scan <item> content in SOAP answer
//
bool SoapESP32::soapScanItem(const String *parentId, const String *attributes, const String *item) {
    soapObject_t info;
    int32_t      i = 0, port;
    char         address[20];
    IPAddress    ip;
    String       str((char *)0);

    log_d("function entered, parent id: %s", parentId->c_str());

    // scan item id
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_ID)) return false;  // id is a must
    log_d("%s\"%s\"", DIDL_ATTR_ID, str.c_str());
    info.id = str;

    // scan parent id
    if(!soapScanAttribute(attributes, &str, DIDL_ATTR_PARENT_ID)) return false;  // parent id is a must
    if(!strcasestr(str.c_str(), parentId->c_str())) {
#ifdef PARENT_ID_MUST_MATCH
        log_e("scanned parent id \"%s\" != requested parent id \"%s\"", str.c_str(), parentId->c_str());
        return false;
#else
        log_w("scanned parent id \"%s\" != requested parent id \"%s\"", str.c_str(), parentId->c_str());
#endif
    }

    info.parentId = *parentId;
    info.fileType = fileTypeOther;
    info.album = "";
    info.artist = "";
    info.bitrate = 0;
    info.sampleFrequency = 0;
    info.size = 0;
    info.sizeMissing = false;

    // scan for uri, size, album (sometimes dir name when picture file) and title, artist (when audio file)
    MiniXPath xPathTitle, xPathAlbum, xPathArtist, xPathClass, xPathRes;
    String    strAttr((char *)0);
    bool      gotTitle = false, gotAlbum = false, gotArtist = false, gotClass = false, gotRes = false;

    xPathTitle.setPath(xmlParserPaths[xpTitle].tagNames, xmlParserPaths[xpTitle].num);
    xPathAlbum.setPath(xmlParserPaths[xpAlbum].tagNames, xmlParserPaths[xpAlbum].num);
    xPathArtist.setPath(xmlParserPaths[xpArtist].tagNames, xmlParserPaths[xpArtist].num);
    xPathClass.setPath(xmlParserPaths[xpClass].tagNames, xmlParserPaths[xpClass].num);
    xPathRes.setPath(xmlParserPaths[xpResource].tagNames, xmlParserPaths[xpResource].num);
    while(i < item->length()) {
        if(!gotTitle && xPathTitle.getValue((char)item->operator[](i), &str)) {
            if(str.length() == 0) return false;  // title is a must
            info.name = str;
            log_d("title=\"%s\"", str.c_str());
            gotTitle = true;
        }
        if(!gotAlbum && xPathAlbum.getValue((char)item->operator[](i), &str)) {
            info.album = str;  // missing album not a showstopper
            log_d("album=\"%s\"", str.c_str());
            ;
            gotAlbum = true;
        }
        if(!gotArtist && xPathArtist.getValue((char)item->operator[](i), &str)) {
            info.artist = str;  // missing artist not a showstopper
            log_d("artist=\"%s\"", str.c_str());
            gotArtist = true;
        }
        if(!gotClass && xPathClass.getValue((char)item->operator[](i), &str)) {
            log_d("class=\"%s\"", str.c_str());
            if(str.indexOf("audioItem") >= 0) info.fileType = fileTypeAudio;
            else if(str.indexOf("imageItem") >= 0)
                info.fileType = fileTypeImage;
            else if(str.indexOf("videoItem"))
                info.fileType = fileTypeVideo;
            else
                info.fileType = fileTypeOther;
            gotClass = true;
        }
        if(!gotRes && xPathRes.getValue((char)item->operator[](i), &str, &strAttr)) {
            if(str.startsWith("http://")) {
                // scan for download ip & port
                if(sscanf(str.c_str(), "http://%[0-9.]:%d/", address, &port) != 2) return false;
                info.downloadPort = (uint16_t)port;
                if(!ip.fromString(address)) return false;
                info.downloadIp = ip;
                // now remove "http://ip:port/" from begin of string
                str.replace("http://", "");
                info.uri = str.substring(str.indexOf("/") + 1);
            }
            else { info.uri = str; }
            if(info.uri.length() == 0) return false;  // valid URI is a must
            log_d("uri=\"%s\"", info.uri.c_str());

            // scan item size
            if(!soapScanAttribute(&strAttr, &str, DIDL_ATTR_SIZE)) {
                // indicates missing attribute "size" (e.g. Kodi audio files, Fritzbox/Serviio stream items)
                info.sizeMissing = true;
            }
            else {
                info.size = strtoull(str.c_str(), NULL, 10);
                log_d("size=%llu", info.size);
            }
#if !defined(SHOW_EMPTY_FILES)
            if(info.size == 0 && !info.sizeMissing) {
                log_w("reported size=0, item ignored");
                return false;
            }
#endif

            // scan bitrate (often provided when audio file)
            if(soapScanAttribute(&strAttr, &str, DIDL_ATTR_BITRATE)) {
                info.bitrate = (size_t)str.toInt();
                if(info.bitrate == 0) { log_v("bitrate=0 !"); }
                else { log_d("bitrate=%d", info.bitrate); }
            }

            // scan sample frequency (often provided when audio file)
            if(soapScanAttribute(&strAttr, &str, DIDL_ATTR_SAMPLEFREQU)) {
                info.sampleFrequency = (size_t)str.toInt();
                if(info.sampleFrequency == 0) { log_v("sampleFrequency=0"); }
                else { log_d("sampleFrequency=%d", info.sampleFrequency); }
            }

            gotRes = true;
        }
        i++;
    }

    if(!gotTitle || !gotRes) {
        log_i("title or ressource info missing, file not added to list");
        return false;  // title & ressource info is a must
    }

    // add valid file to result list
    info.isDirectory = false;
    m_downloadPort = info.downloadPort;
    if(dlna_file) dlna_file(false, info.name, info.id, info.size, info.uri, info.fileType == fileTypeAudio);
    if(dlna_item) dlna_item(false, info.name, info.id, info.size, info.uri, false, info.fileType == fileTypeAudio);
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// browse a SOAP container object (directory) on a media server for content
//
bool SoapESP32::browseServer1(const uint32_t startingIndex,  // offset into directory content list
                              const uint16_t maxCount)       // limits number of objects in result list
{
    if(m_currentServer >= m_dlnaServer.size) {
        if(m_currentServer == 255) return false;  // is -1, nothing is choosen
        log_e("invalid server number: %d", m_currentServer);
        return false;
    }

    // evaluate SOAP answer
    static uint64_t contentSize = 0;
    static bool     chunked = false;
    static int32_t  count = 0;
    static int32_t  countContainer = 0;
    static int32_t  countItem = 0;
    static String   str = "";
    static String   strAttribute = "";
    static bool     browseServerLoop = false;

    if(m_firstCall) {
        m_firstCall = false;
        contentSize = 0;
        chunked = false;
        count = 0;
        countContainer = 0;
        countItem = 0;
        browseServerLoop = true;

        if(dlna_info) {
            sprintf(m_chbuf, "new search on server: \"%s\", objectId: \"%s\"", m_dlnaServer.friendlyName[m_currentServer], m_objectId.c_str());
            dlna_info(m_chbuf);
        }
        if(startingIndex != SOAP_DEFAULT_BROWSE_STARTING_INDEX) log_d("special browse parameter \"startingIndex\": %d", startingIndex);
        if(maxCount != SOAP_DEFAULT_BROWSE_MAX_COUNT) log_d("special browse parameter \"maxCount\": %d", maxCount);
        // send SOAP browse request
        if(!soapPost(m_dlnaServer.ip[m_currentServer],  m_dlnaServer.port[m_currentServer], m_dlnaServer.controlURL[m_currentServer], m_objectId.c_str(), startingIndex, maxCount)) { return false; }
        log_v("connected successfully to server %s:%d", m_dlnaServer.ip[m_currentServer], m_dlnaServer.port[m_currentServer]);

        if(!allocate_MiniXPath()) return false;

        // reading HTTP header
        if(!soapReadHttpHeader(&contentSize, &chunked)) {
            log_e("HTTP Header not ok or reply status not 200");

            m_client.stop();

            release_MiniXPath();
            return false;
        }
        if(!chunked && contentSize == 0) {
            log_e("announced XML size: 0 !");
            release_MiniXPath();
            return false;
        }
        log_v("scan answer from media server:");

        // HTTP header ok, now scan XML/SOAP reply
        m_xPathContainer->setPath(xmlParserPaths[xpBrowseContainer].tagNames, xmlParserPaths[xpBrowseContainer].num);
        m_xPathContainerAlt1->setPath(xmlParserPaths[xpBrowseContainerAlt1].tagNames, xmlParserPaths[xpBrowseContainerAlt1].num);
        m_xPathContainerAlt2->setPath(xmlParserPaths[xpBrowseContainerAlt2].tagNames, xmlParserPaths[xpBrowseContainerAlt2].num);
        m_xPathItem->setPath(xmlParserPaths[xpBrowseItem].tagNames, xmlParserPaths[xpBrowseItem].num);
        m_xPathItemAlt1->setPath(xmlParserPaths[xpBrowseItemAlt1].tagNames, xmlParserPaths[xpBrowseItemAlt1].num);
        m_xPathItemAlt2->setPath(xmlParserPaths[xpBrowseItemAlt2].tagNames, xmlParserPaths[xpBrowseItemAlt2].num);
        m_xPathNumberReturned->setPath(xmlParserPaths[xpBrowseNumberReturned].tagNames, xmlParserPaths[xpBrowseNumberReturned].num);
        m_xPathNumberReturnedAlt1->setPath(xmlParserPaths[xpBrowseNumberReturnedAlt1].tagNames, xmlParserPaths[xpBrowseNumberReturnedAlt1].num);
        m_xPathNumberReturnedAlt2->setPath(xmlParserPaths[xpBrowseNumberReturnedAlt2].tagNames, xmlParserPaths[xpBrowseNumberReturnedAlt2].num);
        return true;
    }

    if(browseServerLoop) {
        int32_t ret = soapReadXML(chunked, true);  // de-chunk data stream and replace XML-entities (if found)
        if(ret < 0) {
            log_e("soapReadXML() returned: %d", ret);
            goto end_stop;
        }
        // TEST
        // Serial.print((char)ret);
        //
        // only one will return a result
        if(m_xPathContainer->getValue((char)ret, &str, &strAttribute, true) || m_xPathContainerAlt1->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathContainerAlt2->getValue((char)ret, &str, &strAttribute, true)) {
            log_v("container attribute (length=%d): %s", strAttribute.length(), strAttribute.c_str());
            log_v("container (length=%d): %s", str.length(), str.c_str());
            if(soapScanContainer(&m_objectId, &strAttribute, &str)) countContainer++;
            // TEST
            delay(1);  // resets task switcher watchdog, just in case it's needed
        }
        if(m_xPathItem->getValue((char)ret, &str, &strAttribute, true) || m_xPathItemAlt1->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathItemAlt2->getValue((char)ret, &str, &strAttribute, true)) {
            log_v("item attribute (length=%d): %s", strAttribute.length(), strAttribute.c_str());
            log_v("item (length=%d): %s", str.length(), str.c_str());
            if(soapScanItem(&m_objectId, &strAttribute, &str)) countItem++;
            // TEST
            delay(1);  // resets task switcher watchdog, just in case it's needed
        }
        if(m_xPathNumberReturned->getValue((char)ret, &str) || m_xPathNumberReturnedAlt1->getValue((char)ret, &str) || m_xPathNumberReturnedAlt2->getValue((char)ret, &str)) {
            count = str.toInt();
            log_d("announced number of folders and/or files: %d", count);

            browseServerLoop = false;
            return true;  // numberReturned comes last, so we can break here
        }
        return true;
    }

    if(count == 0) { log_i("XML scanned, no elements announced"); }
    else if(count != (countContainer + countItem)) {
        log_w("XML scanned, elements announced %d != found %d (possible reason: empty file or vital attributes missing)", count, countContainer + countItem);
    }

end_stop:

    m_client.stop();

    if(dlna_info) {
        sprintf(m_chbuf, "found %d folders and %d files", countContainer, countItem);
        dlna_info(m_chbuf);
    }
    if(dlna_file) dlna_file(true);
    if(dlna_folder) dlna_folder(true);
    if(dlna_item) dlna_item(true, "", "", 0, "", true, false);

        // TEST
#if CORE_DEBUG_LEVEL > 3
    delay(10);  // allow pending serial monitor output to be sent, can be massive when verbose
#elif CORE_DEBUG_LEVEL > 0
    delay(2);
#endif
    release_MiniXPath();
    if(browseServerLoop) return false;
    m_state = IDLE;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SoapESP32::allocate_MiniXPath() {
    m_xPathContainer = new MiniXPath;
    m_xPathContainerAlt1 = new MiniXPath;
    m_xPathContainerAlt2 = new MiniXPath;
    m_xPathItem = new MiniXPath;
    m_xPathItemAlt1 = new MiniXPath;
    m_xPathItemAlt2 = new MiniXPath;
    m_xPathNumberReturned = new MiniXPath;
    m_xPathNumberReturnedAlt1 = new MiniXPath;
    m_xPathNumberReturnedAlt2 = new MiniXPath;

    if(!m_xPathContainer || !m_xPathContainerAlt1 || !m_xPathContainerAlt2 || !m_xPathItem || !m_xPathItemAlt1 || !m_xPathItemAlt2 || !m_xPathNumberReturned || !m_xPathNumberReturnedAlt1 ||
       !m_xPathNumberReturnedAlt2) {
        log_e("oom");
        release_MiniXPath();
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void SoapESP32::release_MiniXPath() {
    delete m_xPathContainer;
    delete m_xPathContainerAlt1;
    delete m_xPathContainerAlt2;
    delete m_xPathItem;
    delete m_xPathItemAlt1;
    delete m_xPathItemAlt2;
    delete m_xPathNumberReturned;
    delete m_xPathNumberReturnedAlt1;
    delete m_xPathNumberReturnedAlt2;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// read up to size bytes from server and place them into buf returnes number of bytes read or -1 in case nothing was read (default read timeout is 3s)
// Remarks:
// - older WiFi library versions & the Ethernet library return -1 if connection is still up but momentarily no data available and return 0 in case of EOF. Newer WiFi versions return 0 in
//   both cases, so we need to treat -1 & 0 equally.
// - timeout checking is vital because client.read() can return 0 for ages in case of WiFi problems
//
int32_t SoapESP32::read(uint8_t *buf, size_t size, uint32_t timeout) {
    // first some basic checks
    if(!buf || !size || !m_clientDataConOpen) return -1;  // clearly an error
    if(!m_clientDataAvailable) return 0;                  // most probably EOF

    int32_t  res = -1;
    uint32_t start = millis();

    while(1) {
        // if (m_clientDataAvailable < size) size = m_clientDataAvailable;

        res = m_client.read(buf, size);

        if(res > 0) {
            // got at least 1 byte from server
            m_clientDataAvailable -= res;
            break;
        }
        if((millis() - start) > timeout) {
            // read timeout
            log_e("error, read timeout: %d ms", timeout);
            break;
        }
    }

    return res;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//
// read a single byte from server, return -1 in case of error
//
int32_t SoapESP32::read(void) {
    uint8_t b;
    if(read(&b, 1) > 0) return b;
    return -1;
}

//
// final stuff to be done after last read() call
//
void SoapESP32::readStop() {
    if(m_clientDataConOpen) {
        m_client.stop();

        m_clientDataConOpen = false;
        log_d("client data connection to media server closed");
    }
    m_clientDataAvailable = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// HTTP POST request
//
bool SoapESP32::soapPost(const char* ip, const uint16_t port, const char *uri, const char *objectId, const uint32_t startingIndex, const uint16_t maxCount) {

    if(m_clientDataConOpen) {
        // should not get here...probably buggy main

        m_client.stop();

        m_clientDataConOpen = false;
        log_w("client data connection to media server was still open. Closed now.");
    }

    for(int32_t i = 0;;) {
        int32_t ret = m_client.connect(ip, (uint16_t)port);

        if(ret) break;
        if(++i >= 2) {
            log_e("error connecting to server ip=%s, port=%d", ip, port);
            return false;
        }
        delay(100);
    }

    // memory allocation for assembling HTTP header
    size_t length = strlen(uri) + 30;
    char  *buffer = (char *)malloc(length);
    if(!buffer) {
        log_e("malloc() couldn't allocate memory");
        return false;
    }

    uint16_t messageLength;
    char     index[12], count[6];
    String   str((char *)0);

    itoa(startingIndex, index, 10);
    itoa(maxCount, count, 10);
    // calculate XML message length
    messageLength = sizeof(SOAP_ENVELOPE_START) - 1;
    messageLength += sizeof(SOAP_BODY_START) - 1;
    messageLength += sizeof(SOAP_BROWSE_START) - 1;
    messageLength += sizeof(SOAP_OBJECTID_START) - 1 + strlen(objectId) + sizeof(SOAP_OBJECTID_END) - 1;
    messageLength += sizeof(SOAP_BROWSEFLAG_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_FLAG) - 1 + sizeof(SOAP_BROWSEFLAG_END) - 1;
    messageLength += sizeof(SOAP_FILTER_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_FILTER) - 1 + sizeof(SOAP_FILTER_END) - 1;
    messageLength += sizeof(SOAP_STARTINGINDEX_START) - 1 + strlen(index) + sizeof(SOAP_STARTINGINDEX_END) - 1;
    messageLength += sizeof(SOAP_REQUESTEDCOUNT_START) - 1 + strlen(count) + sizeof(SOAP_REQUESTEDCOUNT_END) - 1;
    messageLength += sizeof(SOAP_SORTCRITERIA_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_SORT_CRITERIA) - 1 + sizeof(SOAP_SORTCRITERIA_END) - 1;
    messageLength += sizeof(SOAP_BROWSE_END) - 1;
    messageLength += sizeof(SOAP_BODY_END) - 1;
    messageLength += sizeof(SOAP_ENVELOPE_END) - 1;

    // assemble HTTP header
    snprintf(buffer, length, "POST /%s %s", uri, HTTP_VERSION);
    str += buffer;
    log_d("%s:%d %s", ip.toString().c_str(), port, buffer);
    str += "\r\n";
    snprintf(buffer, length, HEADER_HOST, ip, port);  // 29 bytes max
    str += buffer;
    // TEST
    str += "CACHE-CONTROL: no-cache\r\nPRAGMA: no-cache\r\n";
    // str += "FRIENDLYNAME.DLNA.ORG: ESP32-Radio\r\n";
    //
    str += HEADER_CONNECTION_CLOSE;
    snprintf(buffer, length, HEADER_CONTENT_LENGTH_D, messageLength);
    str += buffer;
    str += HEADER_CONTENT_TYPE;
    str += HEADER_SOAP_ACTION;
    str += HEADER_USER_AGENT;
    str += HEADER_EMPTY_LINE;  // empty line marks end of HTTP header

    // assemble SOAP message (multiple str+= instead of a single str+=..+..+.. reduces allocation depth)
    str += SOAP_ENVELOPE_START;
    str += SOAP_BODY_START;
    str += SOAP_BROWSE_START;
    str += SOAP_OBJECTID_START;
    str += objectId;
    str += SOAP_OBJECTID_END;
    str += SOAP_BROWSEFLAG_START;
    str += SOAP_DEFAULT_BROWSE_FLAG;
    str += SOAP_BROWSEFLAG_END;
    str += SOAP_FILTER_START;
    str += SOAP_DEFAULT_BROWSE_FILTER;
    str += SOAP_FILTER_END;
    str += SOAP_STARTINGINDEX_START;
    str += index;
    str += SOAP_STARTINGINDEX_END;
    str += SOAP_REQUESTEDCOUNT_START;
    str += count;
    str += SOAP_REQUESTEDCOUNT_END;
    str += SOAP_SORTCRITERIA_START;
    str += SOAP_DEFAULT_BROWSE_SORT_CRITERIA;
    str += SOAP_SORTCRITERIA_END;
    str += SOAP_BROWSE_END;
    str += SOAP_BODY_END;
    str += SOAP_ENVELOPE_END;

    // send request to server
    log_w("send request to server:\n%s", str.c_str());

    m_client.print(str);

    // wait for a reply until timeout
    uint32_t start = millis();
    while(true) {
        int32_t av = m_client.available();

        if(av) break;
        if(millis() > (start + SERVER_RESPONSE_TIMEOUT)) {
            m_client.stop();

            log_e("POST: no reply from server within %d ms", SERVER_RESPONSE_TIMEOUT);
            free(buffer);
            return false;
        }
    }
    free(buffer);

    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// returns number of usable media servers in our list
//
uint8_t SoapESP32::getServerCount(void) { return m_dlnaServer.size; }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// returns infos about a media servers from our internal list
//
bool SoapESP32::getServerInfo(uint8_t srv, soapServer_t *serverInfo) {
    if(srv >= m_dlnaServer.size) return false;
    serverInfo->controlURL = m_dlnaServer.controlURL[srv];
    serverInfo->friendlyName = m_dlnaServer.friendlyName[srv];
    serverInfo->ip = m_dlnaServer.ip[srv];
    serverInfo->location = m_dlnaServer.location[srv];
    serverInfo->port = m_dlnaServer.port[srv];
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// returns number of available/remaining bytes
//
size_t SoapESP32::available() { return m_clientDataConOpen ? m_clientDataAvailable : 0; }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// returns pointer to string (item type name)
//
const char *SoapESP32::getFileTypeName(eFileType fileType) { return (fileTypeAudio <= fileType && fileType <= fileTypeVideo) ? fileTypes[fileType] : fileTypes[fileTypeOther]; }


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SoapESP32::listServer() {  // returns a list via event 'dlna_server()'
    if(!m_dlnaServer.size) return false;
    m_idx = 0;
    m_state = GET_SERVER_INFO;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool SoapESP32::browseServer(const uint8_t srv, const char *objectId) {
    m_currentServer = srv;
    m_objectId = objectId;
    m_firstCall = true;
    m_state = BROWSE_SERVER;
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

uint16_t SoapESP32::getMediaDownloadPort() { return m_downloadPort; }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

String SoapESP32::getMediaDownloadIP() { return m_dlnaServer.ip[m_currentServer]; }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// state machine
void SoapESP32::loop() {
    static uint8_t cnt = 0;
    bool res;
    switch(m_state) {
        case IDLE:
            break;
        case SEEK_SERVER:
            if(m_timeStamp + m_timeout > millis()){
                size_t len = m_udp.parsePacket();
                if(len){
                    parseDlnaServer(len); // registers all media servers that respond within the time until the timeout
                }
                cnt = 0;
            }
            else{
                m_udp.stop();
                m_state = GET_SERVER_ITEMS;
            }
            break;
        case GET_SERVER_ITEMS:
            if(cnt < m_dlnaServer.size){
                log_w("%s, %i, %s", m_dlnaServer.ip[cnt], m_dlnaServer.port[cnt], m_dlnaServer.location[cnt]);
                res = srvGet(cnt);
                if(!res){log_e("error in srvGet"); break;}
                res = readHttpHeader();
                if(!res){log_e("error in readHttpHeader"); break;}
                res = readContent();
                if(!res){log_e("error in readContent"); break;}

                getServerItems(cnt);
                cnt++;
                break;
            }
            cnt = 0;
            m_state = IDLE;
            break;
        case GET_SERVER_INFO:
            if(m_idx >= m_dlnaServer.size) {
                m_state = IDLE;
                m_idx = 0;
                break;
            }
            if(!m_dlnaServer.friendlyName[m_idx])m_dlnaServer.friendlyName[m_idx] = (char*)"";
            if(!m_dlnaServer.controlURL[m_idx])m_dlnaServer.controlURL[m_idx] = (char*)"";
            if(dlna_server) { dlna_server(m_idx, m_dlnaServer.size, m_dlnaServer.ip[m_idx], m_dlnaServer.port[m_idx], m_dlnaServer.friendlyName[m_idx], m_dlnaServer.controlURL[m_idx]); }
            m_idx++;
            break;
        case BROWSE_SERVER:
            if(m_currentServer == 255) break;  // is -1, no server selected
            if(!browseServer1()) {
                m_state = IDLE;
                log_e("something went wrong");
                break;
            }
            break;
        default:
            break;
    }
    return;
}