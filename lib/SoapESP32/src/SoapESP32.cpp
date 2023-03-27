/*
  SoapESP32, a simple library for accessing DLNA media servers with ESP32 devices

  Copyright (c) 2021 Thomas Jentzsch

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
  ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
  CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
    26.03.2023 add callback events
*/

#include "SoapESP32.h"
#include "MiniXPath.h"

#ifdef USE_ETHERNET
    // usage of Wiznet W5x00 Ethernet board/shield instead of builtin WiFi
    #define claimSPI()            \
        if(m_SPIsem && *m_SPIsem) \
            while(xSemaphoreTake(*m_SPIsem, 10) != pdTRUE)
    #define releaseSPI() \
        if(m_SPIsem && *m_SPIsem) xSemaphoreGive(*m_SPIsem)
#else
    // usage of builtin WiFi
    #define claimSPI()
    #define releaseSPI()
#endif

enum eXpath
{
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

xPathParser_t xmlParserPaths[] = {
    {.num = 3, .tagNames = {"root", "device", "friendlyName"}},
    {.num = 5, .tagNames = {"root", "device", "serviceList", "service", "serviceType"}},
    {.num = 5, .tagNames = {"root", "device", "serviceList", "service", "controlURL"}},
    {.num = 6, .tagNames = {"s:Envelope", "s:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "container"}},
    {.num = 6,
     .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "m:BrowseResponse", "Result", "DIDL-Lite", "container"}},
    {.num = 6,
     .tagNames = {"SOAP-ENV:Envelope", "SOAP-ENV:Body", "u:BrowseResponse", "Result", "DIDL-Lite", "container"}},
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

//
// helper function, find the first occurrence of substring "what" in string "s", ignore case
//
#if !defined(__GNU_VISIBLE)
char *strcasestr(const char *s, const char *what) {
    char   c, sc;
    size_t len;

    if((c = *what++) != 0) {
        c = tolower((unsigned char)c);
        len = strlen(what);
        do {
            do {
                if((sc = *s++) == 0) return (NULL);
            } while((char)tolower((unsigned char)sc) != c);
        } while(strncasecmp(s, what, len) != 0);
        s--;
    }

    return ((char *)s);
}
#endif

//
// SoapESP32 Class Constructor
//
#ifdef USE_ETHERNET
SoapESP32::SoapESP32(EthernetClient *client, EthernetUDP *udp, SemaphoreHandle_t *sem)
    : m_client(client),
      m_udp(udp),
      m_SPIsem(sem),
      m_clientDataConOpen(false),
      m_clientDataAvailable(0)
#else
SoapESP32::SoapESP32(WiFiClient *client, WiFiUDP *udp)
    : m_client(client),
      m_udp(udp),
      m_clientDataConOpen(false),
      m_clientDataAvailable(0)
#endif
{
}

//
// broadcast 3 WOL packets carrying a specified MAC address
// parameter is a pointer to a C string in the format "00:1:23:Aa:bC:D4" as an unusual example
//
#define WOL_PACKET_SIZE 102
bool SoapESP32::wakeUpServer(const char *macAddress) {
    uint8_t packetBuffer[WOL_PACKET_SIZE];
    int     mac[6];
    char    lower[20];

    if(strlen(macAddress) > 10 && strlen(macAddress) < 18) {
        int i;
        for(i = 0; i < strlen(macAddress); i++) { lower[i] = tolower(macAddress[i]); }
        lower[i] = 0;
        if(sscanf(lower, "%x:%x:%x:%x:%x:%x%*c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
            log_e("could not parse parameter MAC \"%s\"", macAddress);
            return false;
        }
        // build WOL udp packet: 6 bytes 0xFF followed by 16 x mac of target device
        memset(packetBuffer, 0xFF, sizeof(packetBuffer));
        for(i = 6; i < WOL_PACKET_SIZE; i++) { packetBuffer[i] = (uint8_t) * (mac + (i % 6)); }
        // broadcast 3 WOL packets (destination IP 255.255.255.255, port 9)
        for(i = 0; i < 3; i++) {
            claimSPI();
            m_udp->begin(9);
            int ret = m_udp->beginPacket(IPAddress(255, 255, 255, 255), 9);
            releaseSPI();
            if(ret) {
                claimSPI();
                m_udp->write(packetBuffer, WOL_PACKET_SIZE);
                m_udp->endPacket();
                m_udp->stop();
                releaseSPI();
                continue;
            }
            break;
        }
        if(i == 3) return true;
    }
    else { log_e("parameter MAC is invalid"); }

    return false;
}

//
// helper function, client timed read
//
int SoapESP32::soapClientTimedRead() {
    int           c;
    unsigned long startMillis = millis();

    do {
        claimSPI();
        c = m_client->read();
        releaseSPI();
        if(c >= 0) { return c; }
    } while(millis() - startMillis < SERVER_READ_TIMEOUT);

    return -1;  // read timeout
}

//
// send SSDP/UDP multicast packets
// WiFi/Ethernet libraries handle port parameter differently !
//
bool SoapESP32::soapUDPmulticast(uint8_t repeats) {
    if(!m_udp) return false;

#ifdef USE_ETHERNET
    claimSPI();
    uint8_t ret = m_udp->beginMulticast(IPAddress(SSDP_MULTICAST_IP), 1900);  // 1900: multicast dest port
    releaseSPI();
#else
    uint8_t ret = m_udp->beginMulticast(IPAddress(SSDP_MULTICAST_IP), 8888);  // 8888: local port
#endif
    if(ret) {
        // creating socket ok
        uint8_t i = 0;
        while(true) {
            claimSPI();
            if(!m_udp->beginPacket(IPAddress(SSDP_MULTICAST_IP), SSDP_MULTICAST_PORT) ||
               !m_udp->write((const uint8_t *)SSDP_M_SEARCH_TX, sizeof(SSDP_M_SEARCH_TX) - 1) || !m_udp->endPacket()) {
                releaseSPI();
                break;
            }
            releaseSPI();
            if(++i > repeats) return true;
        }
    }
    claimSPI();
    m_udp->stop();
    releaseSPI();
    log_e("error sending SSDP multicast packets");

    return false;
}

//
// SSDP/UDP search for media servers in local network
//
bool SoapESP32::soapSSDPquery(soapServerVect_t *result, int msWait) {
    int       i, port;
    size_t    len;
    IPAddress ip;
    char      tmpBuffer[SSDP_TMP_BUFFER_SIZE], location[SSDP_LOCATION_BUF_SIZE] = "", address[20];

    // send SSDP multicast packets (parameter: nr of repeats)
    if(!soapUDPmulticast(1)) return false;

    // evaluate incoming SSDP packets (M-SEARCH replies) & NOTIFY packets if we catch them by chance
    uint32_t start = millis();
    do {
        delay(25);
        claimSPI();
        len = m_udp->parsePacket();
        releaseSPI();
        if(len) {
            char *p;

            // we received SSDP packet of size len
            log_d("received SSDP packet within %d ms: packet size: %d", millis() - start, len);
            memset(tmpBuffer, 0, SSDP_TMP_BUFFER_SIZE);  // clear buffer
            if(len >= SSDP_TMP_BUFFER_SIZE) len = SSDP_TMP_BUFFER_SIZE - 1;
            claimSPI();
            m_udp->read(tmpBuffer, len);  // read packet into the buffer
            releaseSPI();
            log_v("SSDP packet content:\n%s", tmpBuffer);

            // scan SSDP packet
            if(  // M-SEARCH reply packets
                (strstr(tmpBuffer, HTTP_HEADER_200_OK) && ((p = strcasestr(tmpBuffer, SSDP_LOCATION)) != NULL) &&
                 strcasestr(tmpBuffer, SSDP_SERVICE_TYPE)) ||
                // NOTIFY packets sent out regularly by media servers (we ignore ssdp:byebye's)
                (strstr(tmpBuffer, SSDP_NOTIFICATION) && ((p = strcasestr(tmpBuffer, SSDP_LOCATION)) != NULL) &&
                 strcasestr(tmpBuffer, SSDP_NOTIFICATION_TYPE) && strcasestr(tmpBuffer, SSDP_NOTIFICATION_SUB_TYPE))) {
                char format[30];

                strtok(p, "\r\n");
                snprintf(format, sizeof(format), "http://%%[0-9.]:%%d/%%%ds", SSDP_LOCATION_BUF_SIZE - 1);
                if(sscanf(p + 10, format, address, &port, location) < 2) continue;
                if(!ip.fromString(address)) continue;

                // scanning of ip address & port successful, location string can be missing (e.g. D-Link NAS DNS-320L)
                log_d("scanned ip=%s, port=%d, location=\"%s\"", ip.toString().c_str(), port, location);
                if(!strlen(location)) log_d("empty location string!");

                // avoid multiple entries of same server (identical ip & port)
                for(i = 0; i < result->size(); i++) {
                    if(result->operator[](i).ip == ip && result->operator[](i).port == port) break;
                }
                if(i < result->size()) continue;

                // new server found: add it to list
                soapServer_t srv = {.ip = ip,
                                    .port = (uint16_t)port,
                                    .location = location,
                                    .friendlyName = "",
                                    .controlURL = "" };
                result->push_back(srv);
            }
        }
    } while((millis() - start) < msWait);

    claimSPI();
    m_udp->stop();
    releaseSPI();

    return true;
}

//
// read & evaluate HTTP header
//
bool SoapESP32::soapReadHttpHeader(uint64_t *contentLength, bool *chunked) {
    size_t len;
    bool   ok = false;
    char  *p, tmpBuffer[TMP_BUFFER_SIZE_200];

    // first line contains status code
    claimSPI();
    len = m_client->readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);  // returns length without terminator '\n'
    releaseSPI();
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
        claimSPI();
        int av = m_client->available();
        releaseSPI();
        if(!av) break;
        claimSPI();
        len = m_client->readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);
        releaseSPI();
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

//
// read XML data, de-chunk if needed & replace predefined XML entities that spoil MiniXPath
//
const replaceWith_t replaceWith[] = {{"&lt;", '<'},      {"&gt;", '>'},        {"&quot;", '"'},
                                     {"&amp;amp;", '&'}, {"&amp;apos;", '\''}, {"&amp;quot;", '\''}};

int SoapESP32::soapReadXML(bool chunked, bool replace) {
    bool match;
    int  i, c = -10;

    if(!replace || (replace && (m_xmlReplaceState == xmlPassthrough))) {
    GET_MORE:
        if(!chunked) {
            // data is not chunked
            if((c = soapClientTimedRead()) < 0) {
                // timeout or connection closed
                return -1;
            }
        }
        else {
            // de-chunk XML data
            if(m_xmlChunkCount <= 0) {
                char tmpBuffer[10];

                // next line contains chunk size
                claimSPI();
                int len = m_client->readBytesUntil('\n', tmpBuffer, sizeof(tmpBuffer) - 1);
                releaseSPI();
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

//
// searching local network for media servers that offer media content
// returns number of servers found
//
uint8_t SoapESP32::seekServer() {
    soapServerVect_t rcvd;

    // delete old server list
    m_server.clear();

    if(dlna_info) dlna_info("SSDP search for media servers started");
    soapSSDPquery(&rcvd);

    sprintf(m_chbuf, "SSDP query discovered %d media servers", rcvd.size());
    if(dlna_info) dlna_info(m_chbuf);

    if(rcvd.size() == 0) return 0;  // return if none detected

    // if(dlna_info) dlna_info("checking all discovered media servers for service ContentDirectory");

    int          j = 0;
    uint64_t     contentSize;
    bool         chunked, gotFriendlyName, gotServiceType;
    String       result((char *)0);
    MiniXPath    xPath;
    soapServer_t srv;

    // examine all media servers that answered our SSDP multicast query
    while(j < rcvd.size()) {
        // try to establish connection to server and send GET request
        if(!soapGet(rcvd[j].ip, rcvd[j].port, rcvd[j].location.c_str())) goto end;

        if(dlna_info){
            sprintf(m_chbuf, "connected successfully to %s:%d", rcvd[j].ip.toString().c_str(), rcvd[j].port);
            dlna_info(m_chbuf);
        }

        // ok, connection established
        srv = {.ip = rcvd[j].ip,
               .port = rcvd[j].port,
               .location = rcvd[j].location,
               .friendlyName = "",
               .controlURL = "" };

        gotFriendlyName = false;
        gotServiceType = false;

        // reading HTTP header
        if(!soapReadHttpHeader(&contentSize, &chunked)) { goto end_stop_error; }
        if(!chunked && contentSize == 0) {
            log_w("announced XML size: 0, we can stop here");
            goto end_stop_error;
        }

        // scan XML block for description: friendly name, service type "ContentDirectory" & associated control URL
        xPath.reset();
        xPath.setPath(xmlParserPaths[xpFriendlyName].tagNames, xmlParserPaths[xpFriendlyName].num);
        while(true) {
            int ret = soapReadXML(chunked);
            if(ret < 0) {
                log_w("soapReadXML() returned: %d", ret);
                goto end_stop_error;
            }

            if(!gotFriendlyName) {
                if(xPath.getValue((char)ret, &result)) {
                    srv.friendlyName = (result.length() > 0) ? result : "Server name not provided";
                    log_d("scanned friendly name: %s", srv.friendlyName.c_str());
                    gotFriendlyName = true;
                    // we got friendly name and now set xPath for service type which comes next
                    xPath.setPath(xmlParserPaths[xpServiceType].tagNames, xmlParserPaths[xpServiceType].num);
                    continue;
                }
            }
            else if(!gotServiceType) {
                if(xPath.getValue((char)ret, &result)) {
                    if(strstr(result.c_str(), UPNP_URN_SCHEMA_CONTENT_DIRECTORY)) {
                        log_d("server offers service: %s", UPNP_URN_SCHEMA_CONTENT_DIRECTORY);
                        gotServiceType = true;
                        // We got service type and now set xPath for control url (follows in same <service> block)
                        xPath.setPath(xmlParserPaths[xpControlUrl].tagNames, xmlParserPaths[xpControlUrl].num);
                        continue;
                    }
                }
            }
            else if(xPath.getValue((char)ret, &result)) {
                // we finally got all infos we need
                if(srv.location.endsWith("/"))
                    srv.controlURL = srv.location;  // location string becomes first part of controlURL
                srv.controlURL += result;
                if(srv.controlURL.startsWith("http://")) {
                    // remove "http://ip:port/" from begin of string
                    srv.controlURL.replace("http://", "");
                    srv.controlURL = srv.controlURL.substring(srv.controlURL.indexOf("/") + 1);
                }
                log_d("assigned controlURL: %s", srv.controlURL.c_str());
                // if(dlna_info) dlna_info("ok, this server delivers media content");
                m_server.push_back(srv);  // add server to server list
                goto end_stop;
            }
        }
    end_stop_error:
        if(dlna_info) dlna_info("this Server does not deliver media content");
    end_stop:
        claimSPI();
        m_client->stop();
        releaseSPI();
    end:
        j++;
    }

    return m_server.size();
}

//
// add a server manually to server list
//
bool SoapESP32::addServer(IPAddress ip, uint16_t port, const char *controlURL, const char *name) {
    soapServer_t srv;
    int          i;

    // just some basic checks
    if(!ip || !port || !name || strlen(name) == 0 || !controlURL || strlen(controlURL) == 0) {
        log_e("at least on parameter is invalid");
        return false;
    }

    // refuse entry in list in case of identical ip & port
    for(i = 0; i < m_server.size(); i++) {
        if(m_server.operator[](i).ip == ip && m_server.operator[](i).port == port) break;
    }
    if(i < m_server.size()) return false;

    srv.ip = ip;
    srv.port = port;
    srv.controlURL = controlURL;
    srv.friendlyName = name;

    // add server to list
    m_server.push_back(srv);

    return true;
}

//
// erase all entries in server list
//
void SoapESP32::clearServerList() { m_server.clear(); }

//
// helper function: scan for certain attribute
//
bool SoapESP32::soapScanAttribute(const String *attributes, String *result, const char *what) {
    int begin, end;

    if((begin = attributes->indexOf(what)) >= 0 &&
       (end = attributes->indexOf("\"", begin + strlen(what) + 1)) >= begin + strlen(what) + 1) {
        *result = attributes->substring(begin + strlen(what) + 1, end);
        if(result->length() > 0) return true;
    }

    *result = "";  // empty for next call
    log_v("attribute: \"%s\" missing.", what);

    return false;
}

//
// scan <container> content in SOAP answer
//
bool SoapESP32::soapScanContainer(const String *parentId, const String *attributes, const String *container) {
    int          i = 0;
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
        if((info.size = (int)str.toInt()) == 0) {
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
        info.searchable = (1 == (int)str.toInt()) ? true : false;
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

    if(dlna_folder) dlna_folder(info.name, info.id, info.size);

    return true;
}

//
// scan <item> content in SOAP answer
//
bool SoapESP32::soapScanItem(const String *parentId, const String *attributes, const String *item) {
    soapObject_t info;
    int          i = 0, port;
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
    if(dlna_file) dlna_file(info.name, info.id, info.size, info.uri, info.fileType == fileTypeAudio);

    return true;
}

//
// browse a SOAP container object (directory) on a media server for content
//
bool SoapESP32::browseServer1( const uint32_t startingIndex,  // offset into directory content list
                               const uint16_t maxCount)       // limits number of objects in result list
{
    if(m_currentServer >= m_server.size()) {
        if(m_currentServer == 255) return false; // is -1, nothing is choosen
        log_e("invalid server number: %d", m_currentServer);
        return false;
    }

    // evaluate SOAP answer
    static uint64_t  contentSize = 0;
    static bool      chunked = false;
    static int       count = 0;
    static int       countContainer = 0;
    static int       countItem = 0;
    static String    str = "";
    static String    strAttribute = "";
    static bool      browseServerLoop = false;

    if(m_firstCall){

        m_firstCall = false;
        contentSize = 0;
        chunked = false;
        count = 0;
        countContainer = 0;
        countItem = 0;
        browseServerLoop = true;

        if(dlna_info){sprintf(m_chbuf, "new search on server: \"%s\", objectId: \"%s\"",
                                       m_server[m_currentServer].friendlyName.c_str(), m_objectId.c_str());
            dlna_info(m_chbuf);
        }
        if(startingIndex != SOAP_DEFAULT_BROWSE_STARTING_INDEX)
            log_d("special browse parameter \"startingIndex\": %d", startingIndex);
        if(maxCount != SOAP_DEFAULT_BROWSE_MAX_COUNT) log_d("special browse parameter \"maxCount\": %d", maxCount);
        // send SOAP browse request
        if(!soapPost(m_server[m_currentServer].ip, m_server[m_currentServer].port,
            m_server[m_currentServer].controlURL.c_str(), m_objectId.c_str(), startingIndex, maxCount)) {
            return false;
        }
        log_v("connected successfully to server %s:%d", m_server[m_currentServer].ip.toString().c_str(),
            m_server[m_currentServer].port);

        if(!allocate_MiniXPath()) return false;

        // reading HTTP header
        if(!soapReadHttpHeader(&contentSize, &chunked)) {
            log_e("HTTP Header not ok or reply status not 200");
            claimSPI();
            m_client->stop();
            releaseSPI();
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
        m_xPathContainerAlt1->setPath(xmlParserPaths[xpBrowseContainerAlt1].tagNames,
                                   xmlParserPaths[xpBrowseContainerAlt1].num);
        m_xPathContainerAlt2->setPath(xmlParserPaths[xpBrowseContainerAlt2].tagNames,
                                   xmlParserPaths[xpBrowseContainerAlt2].num);
        m_xPathItem->setPath(xmlParserPaths[xpBrowseItem].tagNames, xmlParserPaths[xpBrowseItem].num);
        m_xPathItemAlt1->setPath(xmlParserPaths[xpBrowseItemAlt1].tagNames, xmlParserPaths[xpBrowseItemAlt1].num);
        m_xPathItemAlt2->setPath(xmlParserPaths[xpBrowseItemAlt2].tagNames, xmlParserPaths[xpBrowseItemAlt2].num);
        m_xPathNumberReturned->setPath(xmlParserPaths[xpBrowseNumberReturned].tagNames,
                                    xmlParserPaths[xpBrowseNumberReturned].num);
        m_xPathNumberReturnedAlt1->setPath(xmlParserPaths[xpBrowseNumberReturnedAlt1].tagNames,
                                        xmlParserPaths[xpBrowseNumberReturnedAlt1].num);
        m_xPathNumberReturnedAlt2->setPath(xmlParserPaths[xpBrowseNumberReturnedAlt2].tagNames,
                                        xmlParserPaths[xpBrowseNumberReturnedAlt2].num);
        return true;
    }


    if(browseServerLoop) {
        int ret = soapReadXML(chunked, true);  // de-chunk data stream and replace XML-entities (if found)
        if(ret < 0) {
            log_e("soapReadXML() returned: %d", ret);
            goto end_stop;
        }
        // TEST
        // Serial.print((char)ret);
        //
        // only one will return a result
        if(m_xPathContainer->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathContainerAlt1->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathContainerAlt2->getValue((char)ret, &str, &strAttribute, true)) {
            log_v("container attribute (length=%d): %s", strAttribute.length(), strAttribute.c_str());
            log_v("container (length=%d): %s", str.length(), str.c_str());
            if(soapScanContainer(&m_objectId, &strAttribute, &str)) countContainer++;
            // TEST
            delay(1);  // resets task switcher watchdog, just in case it's needed
        }
        if(m_xPathItem->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathItemAlt1->getValue((char)ret, &str, &strAttribute, true) ||
           m_xPathItemAlt2->getValue((char)ret, &str, &strAttribute, true)) {
            log_v("item attribute (length=%d): %s", strAttribute.length(), strAttribute.c_str());
            log_v("item (length=%d): %s", str.length(), str.c_str());
            if(soapScanItem(&m_objectId, &strAttribute, &str)) countItem++;
            // TEST
            delay(1);  // resets task switcher watchdog, just in case it's needed
        }
        if(m_xPathNumberReturned->getValue((char)ret, &str) || m_xPathNumberReturnedAlt1->getValue((char)ret, &str) ||
           m_xPathNumberReturnedAlt2->getValue((char)ret, &str)) {
            count = str.toInt();
            log_d("announced number of folders and/or files: %d", count);

            browseServerLoop = false;
            return true; // numberReturned comes last, so we can break here
        }
        return true;
    }

    if(count == 0) { log_i("XML scanned, no elements announced"); }
    else if(count != (countContainer + countItem)) {
        log_w(
            "XML scanned, elements announced %d != found %d (possible reason: empty file or vital attributes missing)",
            count, countContainer + countItem);
    }

end_stop:
    claimSPI();
    m_client->stop();
    releaseSPI();
    if(dlna_info){
        sprintf(m_chbuf, "found %d folders and %d files", countContainer, countItem);
        dlna_info(m_chbuf);
    }

    // TEST
#if CORE_DEBUG_LEVEL > 3
    delay(10);  // allow pending serial monitor output to be sent, can be massive when verbose
#elif CORE_DEBUG_LEVEL > 0
    delay(2);
#endif
    release_MiniXPath();
    if(browseServerLoop) return false;
    m_status = IDLE;
    return true;
}
//----------------------------------------------------------------------------------------------------------------------
bool SoapESP32::allocate_MiniXPath(){
    m_xPathContainer = new MiniXPath;
    m_xPathContainerAlt1= new MiniXPath;
    m_xPathContainerAlt2= new MiniXPath;
    m_xPathItem= new MiniXPath;
    m_xPathItemAlt1= new MiniXPath;
    m_xPathItemAlt2= new MiniXPath;
    m_xPathNumberReturned= new MiniXPath;
    m_xPathNumberReturnedAlt1= new MiniXPath;
    m_xPathNumberReturnedAlt2= new MiniXPath;

    if(!m_xPathContainer || !m_xPathContainerAlt1 || !m_xPathContainerAlt2 || !m_xPathItem || !m_xPathItemAlt1 ||
       !m_xPathItemAlt2  || !m_xPathNumberReturned || !m_xPathNumberReturnedAlt1 || !m_xPathNumberReturnedAlt2){
        log_e("oom");
        release_MiniXPath();
        return false;
    }
    return true;
}

void SoapESP32::release_MiniXPath(){
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
//----------------------------------------------------------------------------------------------------------------------
//
// request object (file) from media server
//
bool SoapESP32::readStart(soapObject_t *object, size_t *size) {
    uint64_t contentSize;

    if(object->isDirectory) return false;
    m_clientDataAvailable = 0;

    log_v("server ip: %s, port: %d, uri: \"%s\"", object->downloadIp.toString().c_str(), object->downloadPort,
          object->uri.c_str());

    // just to make sure old connection is closed
    if(m_clientDataConOpen) {
        claimSPI();
        m_client->stop();
        releaseSPI();
        m_clientDataConOpen = false;
        log_w("client data connection to media server was still open. Closed now.");
    }

    // establish connection to server and send GET request
    if(!soapGet(object->downloadIp, object->downloadPort, object->uri.c_str())) { return false; }

    // connection established, read HTTP header
    if(!soapReadHttpHeader(&contentSize)) {
        // error returned
        log_e("soapReadHttpHeader() was unsuccessful.");
        claimSPI();
        m_client->stop();
        releaseSPI();
        return false;
    }

    // max allowed file size for download is 4.2GB (SIZE_MAX)
    if(contentSize > (uint64_t)SIZE_MAX) {
        log_e("file too big for download. Maximum allowed file size is 4.2GB.");
        claimSPI();
        m_client->stop();
        releaseSPI();
        return false;
    }

    m_clientDataAvailable = (size_t)contentSize;
    if(m_clientDataAvailable == 0) {
        // no data available
        log_e("announced file size: 0 !");
        claimSPI();
        m_client->stop();
        releaseSPI();
        return false;
    }

    m_clientDataConOpen = true;
    if(size) {                          // pointer valid ?
        *size = m_clientDataAvailable;  // return size of file
    }
    return true;
}

//
// read up to size bytes from server and place them into buf
// returnes number of bytes read or -1 in case nothing was read (default read timeout is 3s)
// Remarks:
// - older WiFi library versions & the Ethernet library return -1 if connection is still up but
//   momentarily no data available and return 0 in case of EOF. Newer WiFi versions return 0 in
//   both cases, so we need to treat -1 & 0 equally.
// - timeout checking is vital because client.read() can return 0 for ages in case of WiFi problems
//
int SoapESP32::read(uint8_t *buf, size_t size, uint32_t timeout) {
    // first some basic checks
    if(!buf || !size || !m_clientDataConOpen) return -1;  // clearly an error
    if(!m_clientDataAvailable) return 0;                  // most probably EOF

    int      res = -1;
    uint32_t start = millis();

    while(1) {
        // if (m_clientDataAvailable < size) size = m_clientDataAvailable;
        claimSPI();
        res = m_client->read(buf, size);
        releaseSPI();
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

//
// read a single byte from server, return -1 in case of error
//
int SoapESP32::read(void) {
    uint8_t b;
    if(read(&b, 1) > 0) return b;
    return -1;
}

//
// final stuff to be done after last read() call
//
void SoapESP32::readStop() {
    if(m_clientDataConOpen) {
        claimSPI();
        m_client->stop();
        releaseSPI();
        m_clientDataConOpen = false;
        log_d("client data connection to media server closed");
    }
    m_clientDataAvailable = 0;
}

//
// HTTP GET request
//
bool SoapESP32::soapGet(const IPAddress ip, const uint16_t port, const char *uri) {
    if(m_clientDataConOpen) {
        // should not happen...probably buggy main
        claimSPI();
        m_client->stop();
        releaseSPI();
        m_clientDataConOpen = false;
        log_w("client data connection to media server was still open. Closed now.");
    }

    for(int i = 0;;) {
        claimSPI();
        bool ret = m_client->connect(ip, port);
        releaseSPI();
        if(ret) break;
        if(++i >= 3) {
            log_e("error connecting to server ip=%s, port=%d", ip.toString().c_str(), port);
            return false;
        }
        delay(100);
    }

    // memory allocation for assembling HTTP header
    size_t length = strlen(uri) + 25;
    char  *buffer = (char *)malloc(length);
    if(!buffer) {
        log_e("malloc() couldn't allocate memory");
        return false;
    }
    String str((char *)0);

    // assemble HTTP header
    snprintf(buffer, length, "GET /%s %s", uri, HTTP_VERSION);
    str += buffer;
    log_d("%s:%d %s", ip.toString().c_str(), port, buffer);
    str += "\r\n";
    snprintf(buffer, length, HEADER_HOST, ip.toString().c_str(), port);
    str += buffer;
    str += HEADER_CONNECTION_CLOSE;
    str += HEADER_USER_AGENT;
    str += HEADER_EMPTY_LINE;  // empty line marks end of HTTP header

    // send request to server
    claimSPI();
    m_client->print(str);
    releaseSPI();

    // give server some time to answer
    uint32_t start = millis();
    while(true) {
        claimSPI();
        int av = m_client->available();
        releaseSPI();
        if(av) break;
        if(millis() > (start + SERVER_RESPONSE_TIMEOUT)) {
            claimSPI();
            m_client->stop();
            releaseSPI();
            log_e("GET: no reply from server for %d ms", SERVER_RESPONSE_TIMEOUT);
            free(buffer);
            return false;
        }
    }
    free(buffer);

    return true;
}

//
// HTTP POST request
//
bool SoapESP32::soapPost(const IPAddress ip, const uint16_t port, const char *uri, const char *objectId,
                         const uint32_t startingIndex, const uint16_t maxCount) {
    if(m_clientDataConOpen) {
        // should not get here...probably buggy main
        claimSPI();
        m_client->stop();
        releaseSPI();
        m_clientDataConOpen = false;
        log_w("client data connection to media server was still open. Closed now.");
    }

    for(int i = 0;;) {
        claimSPI();
        int ret = m_client->connect(ip, (uint16_t)port);
        releaseSPI();
        if(ret) break;
        if(++i >= 2) {
            log_e("error connecting to server ip=%s, port=%d", ip.toString().c_str(), port);
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
    messageLength +=
        sizeof(SOAP_BROWSEFLAG_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_FLAG) - 1 + sizeof(SOAP_BROWSEFLAG_END) - 1;
    messageLength +=
        sizeof(SOAP_FILTER_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_FILTER) - 1 + sizeof(SOAP_FILTER_END) - 1;
    messageLength += sizeof(SOAP_STARTINGINDEX_START) - 1 + strlen(index) + sizeof(SOAP_STARTINGINDEX_END) - 1;
    messageLength += sizeof(SOAP_REQUESTEDCOUNT_START) - 1 + strlen(count) + sizeof(SOAP_REQUESTEDCOUNT_END) - 1;
    messageLength += sizeof(SOAP_SORTCRITERIA_START) - 1 + sizeof(SOAP_DEFAULT_BROWSE_SORT_CRITERIA) - 1 +
                     sizeof(SOAP_SORTCRITERIA_END) - 1;
    messageLength += sizeof(SOAP_BROWSE_END) - 1;
    messageLength += sizeof(SOAP_BODY_END) - 1;
    messageLength += sizeof(SOAP_ENVELOPE_END) - 1;

    // assemble HTTP header
    snprintf(buffer, length, "POST /%s %s", uri, HTTP_VERSION);
    str += buffer;
    log_d("%s:%d %s", ip.toString().c_str(), port, buffer);
    str += "\r\n";
    snprintf(buffer, length, HEADER_HOST, ip.toString().c_str(), port);  // 29 bytes max
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
    log_v("send request to server:\n%s", str.c_str());
    claimSPI();
    m_client->print(str);
    releaseSPI();

    // wait for a reply until timeout
    uint32_t start = millis();
    while(true) {
        claimSPI();
        int av = m_client->available();
        releaseSPI();
        if(av) break;
        if(millis() > (start + SERVER_RESPONSE_TIMEOUT)) {
            claimSPI();
            m_client->stop();
            releaseSPI();
            log_e("POST: no reply from server within %d ms", SERVER_RESPONSE_TIMEOUT);
            free(buffer);
            return false;
        }
    }
    free(buffer);

    return true;
}

//
// returns number of usable media servers in our list
//
uint8_t SoapESP32::getServerCount(void) { return m_server.size(); }

//
// returns infos about a media servers from our internal list
//
bool SoapESP32::getServerInfo(uint8_t srv, soapServer_t *serverInfo) {
    if(srv >= m_server.size()) return false;
    *serverInfo = m_server[srv];
    return true;
}

//
// returns number of available/remaining bytes
//
size_t SoapESP32::available() { return m_clientDataConOpen ? m_clientDataAvailable : 0; }

//
// returns pointer to string (item type name)
//
const char *SoapESP32::getFileTypeName(eFileType fileType) {
    return (fileTypeAudio <= fileType && fileType <= fileTypeVideo) ? fileTypes[fileType] : fileTypes[fileTypeOther];
}

void SoapESP32::loop(){
    if(!m_status) return;
    switch(m_status){
        case GET_SERVER_INFO:
                    if(m_idx >= m_server.size()) {m_status = IDLE; m_idx = 0; break;}
                    if(dlna_server){ dlna_server(m_idx,
                                                 m_server[m_idx].ip.toString(),
                                                 m_server[m_idx].port,
                                                 m_server[m_idx].friendlyName,
                                                 m_server[m_idx].controlURL);
                    }
                    m_idx++;
                    break;
        case BROWSE_SERVER:
                    if(m_currentServer == 255) break; // is -1, no server selected
                    if(!browseServer1()){m_status = IDLE; log_e("something went wrong"); break;}
                    break;
        default: break;
    }
    return;
}

bool SoapESP32::listServer(){ // returns a list via event 'dlna_server()'
    if(!m_server.size()) return false;
    m_idx = 0;
    m_status = GET_SERVER_INFO;
    return true;
}

bool SoapESP32::browseServer(const uint8_t srv, const char *objectId){
    m_currentServer = srv;
    m_objectId = objectId;
    m_firstCall = true;
    m_status = BROWSE_SERVER;
    return true;
}

uint16_t SoapESP32::getMediaDownloadPort(){
    return m_downloadPort;
}

String SoapESP32::getMediaDownloadIP(){
    return m_server[m_currentServer].ip.toString();
}