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

#ifndef SoapESP32_h
#define SoapESP32_h

#include <Arduino.h>
#include "MiniXPath.h"
#include <vector>

// Please uncomment if you use an Ethernet board/shield instead of builtin WiFi
// #define USE_ETHERNET

#if defined(ethernet_h_) && !defined(USE_ETHERNET)
    #warning "== ATTENTION == Did you forget to define USE_ETHERNET ?????  Please read Readme.md !!!!!"
#endif

#ifdef USE_ETHERNET
    #include <Ethernet.h>
#else
    #include <WiFi.h>
#endif

#define TMP_BUFFER_SIZE_200  200
#define TMP_BUFFER_SIZE_400  400
#define TMP_BUFFER_SIZE_1000 1000

// network communication timeouts
#define SERVER_RESPONSE_TIMEOUT 4000  // ms
#define SERVER_READ_TIMEOUT     4000  // ms

// SSDP UDP - seeking media servers
#define SSDP_MULTICAST_IP         239, 255, 255, 250
#define SSDP_MULTICAST_PORT       1900
#define SSDP_MAX_REPLY_TIMEOUT    4000  // ms
#define SSDP_LOCATION_BUF_SIZE    150
#define SSDP_CONTROL_URL_BUF_SIZE 200
#define SSDP_TMP_BUFFER_SIZE      500
#define SSDP_M_SEARCH_TX                                                                       \
    "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\nMX: 3\r\n" \
    "ST: urn:schemas-upnp-org:device:MediaServer:1\r\n\r\n"

#define SSDP_LOCATION              "Location: http://"
#define SSDP_SERVICE_TYPE          "ST: urn:schemas-upnp-org:device:MediaServer:1"
#define SSDP_NOTIFICATION          "NOTIFY * HTTP/1.1"
#define SSDP_NOTIFICATION_TYPE     "NT: urn:schemas-upnp-org:device:MediaServer:1"
#define SSDP_NOTIFICATION_SUB_TYPE "NTS: ssdp:alive"

// HTTP header lines
#define HTTP_VERSION                 "HTTP/1.1"
#define HTTP_HEADER_200_OK           "HTTP/1.1 200 OK"
#define HEADER_CONTENT_LENGTH        "Content-Length: "
#define HEADER_HOST                  "Host: %s:%d\r\n"
#define HEADER_CONTENT_TYPE          "Content-Type: text/xml; charset=\"utf-8\"\r\n"
#define HEADER_TRANS_ENC_CHUNKED     "Transfer-Encoding: chunked"
#define HEADER_CONTENT_LENGTH_D      "Content-Length: %d\r\n"
#define HEADER_SOAP_ACTION           "SOAPAction: \"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"\r\n"
#define HEADER_USER_AGENT            "User-Agent: ESP32/Player/UPNP1.0\r\n"
#define HEADER_CONNECTION_CLOSE      "Connection: close\r\n"
#define HEADER_CONNECTION_KEEP_ALIVE "Connection: keep-alive\r\n"
#define HEADER_EMPTY_LINE            "\r\n"

// SOAP tag data
#define SOAP_ENVELOPE_START                                              \
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" " \
    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
#define SOAP_ENVELOPE_END         "</s:Envelope>\r\n\r\n"
#define SOAP_BODY_START           "<s:Body>"
#define SOAP_BODY_END             "</s:Body>\r\n"
#define SOAP_BROWSE_START         "<u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">\r\n"
#define SOAP_BROWSE_END           "</u:Browse>\r\n"
#define SOAP_OBJECTID_START       "<ObjectID>"
#define SOAP_OBJECTID_END         "</ObjectID>\r\n"
#define SOAP_BROWSEFLAG_START     "<BrowseFlag>"
#define SOAP_BROWSEFLAG_END       "</BrowseFlag>\r\n"
#define SOAP_FILTER_START         "<Filter>"
#define SOAP_FILTER_END           "</Filter>\r\n"
#define SOAP_STARTINGINDEX_START  "<StartingIndex>"
#define SOAP_STARTINGINDEX_END    "</StartingIndex>\r\n"
#define SOAP_REQUESTEDCOUNT_START "<RequestedCount>"
#define SOAP_REQUESTEDCOUNT_END   "</RequestedCount>\r\n"
#define SOAP_SORTCRITERIA_START   "<SortCriteria>"
#define SOAP_SORTCRITERIA_END     "</SortCriteria>\r\n"

// UPnP service data
#define UPNP_URN_SCHEMA                   "schemas-upnp-org:service:"
#define UPNP_URN_SCHEMA_CONTENT_DIRECTORY "urn:schemas-upnp-org:service:ContentDirectory:1"

// SOAP default browse parameters
#define SOAP_DEFAULT_BROWSE_FLAG           "BrowseDirectChildren"
#define SOAP_DEFAULT_BROWSE_FILTER         "*"
#define SOAP_DEFAULT_BROWSE_STARTING_INDEX 0
#define SOAP_DEFAULT_BROWSE_MAX_COUNT      1000
#define SOAP_DEFAULT_BROWSE_SORT_CRITERIA  ""

// selected DIDL attributes for scanning
#define DIDL_ATTR_ID          "id="
#define DIDL_ATTR_PARENT_ID   "parentID="
#define DIDL_ATTR_CHILD_COUNT "childCount="
#define DIDL_ATTR_SEARCHABLE  "searchable="
#define DIDL_ATTR_SIZE        "size="
#define DIDL_ATTR_BITRATE     "bitrate="
#define DIDL_ATTR_SAMPLEFREQU "sampleFrequency="

extern __attribute__((weak)) void dlna_info(const char *);
extern __attribute__((weak)) void dlna_server(uint8_t serverId, size_t serverSize, String IP_addr, uint16_t port,
                                              String friendlyName, String controlURL);
extern __attribute__((weak)) void dlna_folder(bool lastItem, String name = "", String id = "", size_t childCount = 0);
extern __attribute__((weak)) void dlna_file(bool lastItem, String name = "", String id = "", size_t size = 0, String uri = "", bool isAudio = false);
extern __attribute__((weak)) void dlna_item(bool lastItem, String name, String id, size_t size, String uri, bool isDir, bool isAudio);

// for replacing predefined XML entities in server reply
enum eXmlReplaceState
{
    xmlPassthrough = 0,
    xmlAmpDetected,
    xmlTakeFromBuffer
};
struct replaceWith_t {
    const char *replace;
    const char  with;
};

// defines the data content of a reported item (file/stream)
enum eFileType{
    fileTypeOther = 0,
    fileTypeAudio,
    fileTypeImage,
    fileTypeVideo
};

// info collection of a single SOAP object (<container> or <item>)
struct soapObject_t {
    bool      isDirectory;  // true if directory
    uint64_t  size;         // directory child count or item size, zero in case of missing size/child count attribute
    bool      sizeMissing;  // true in case server did not provide size
    int32_t       bitrate;      // bitrate (music files only)
    int32_t       sampleFrequency;  // sample frequency (music files only)
    bool      searchable;       // only used for directories, some media servers don't provide it
    eFileType fileType;         // audio, picture, movie, stream or other
    String    parentId;         // parent directory id
    String    id;               // unique id of directory/file on media server
    String    name;             // directory name or file name
    String    artist;           // for music files
    String    album;            // for music files (sometimes folder name when picture file)
    String    uri;              // item URI on server, needed for download with GET
    IPAddress downloadIp;       // download IP can differ from server IP
    uint16_t  downloadPort;     // download port can differ from server control port
};
typedef std::vector<soapObject_t> soapObjectVect_t;

// keeps vital infos of each media server
struct soapServer_t {
    IPAddress ip;
    uint16_t  port;
    String    location;
    String    friendlyName;
    String    controlURL;
};
typedef std::vector<soapServer_t> soapServerVect_t;

// SoapESP32 class
class SoapESP32 {

  public:
#ifdef USE_ETHERNET
    SoapESP32(EthernetClient *client, EthernetUDP *udp = NULL, SemaphoreHandle_t *sem = NULL);
#else
    SoapESP32(WiFiClient *client, WiFiUDP *udp = NULL);
#endif
    bool        wakeUpServer(const char *macWOL);
    void        clearServerList(void);
    bool        addServer(IPAddress ip, uint16_t port, const char *controlURL, const char *name = "My Media Server");
    uint8_t     seekServer(void);
    uint8_t     getServerCount(void);
    bool        getServerInfo(uint8_t srv, soapServer_t *serverInfo);
    bool        readStart(soapObject_t *object, size_t *size);
    int32_t         read(uint8_t *buf, size_t size, uint32_t timeout = SERVER_READ_TIMEOUT);
    int32_t         read(void);
    void        readStop(void);
    size_t      available(void);
    const char *getFileTypeName(eFileType fileType);

  private:
#ifdef USE_ETHERNET
    EthernetClient    *m_client;  // pointer to EthernetClient object
    EthernetUDP       *m_udp;     // pointer to EthernetUDP object
    SemaphoreHandle_t *m_SPIsem;  // only needed if we use Ethernet lib instead of builtin WiFi
#else
    WiFiClient *m_client;  // pointer to WiFiClient object
    WiFiUDP    *m_udp;     // pointer to WiFiUDP object
#endif

    enum : int32_t{IDLE = 0, GET_SERVER_INFO = 1, BROWSE_SERVER = 2};

    bool             m_clientDataConOpen;    // marker: socket open for reading file
    size_t           m_clientDataAvailable;  // file read count
    soapServerVect_t m_server;               // list of usable media servers in local network
    int32_t              m_xmlChunkCount;    // nr of bytes left of chunk (0 = end of chunk, next line delivers chunk size)
    eXmlReplaceState m_xmlReplaceState;  // state machine for replacing XML entities
    uint8_t          m_xmlReplaceOffset;
    char             m_xmlReplaceBuffer[15];  // Fits longest string in replaceWith[] array
    uint8_t          m_status = IDLE;
    uint8_t          m_idx = 0; // universal counter
    char             m_chbuf[512]; //universal use
    uint8_t          m_currentServer = 0;
    String           m_objectId = "";
    bool             m_firstCall = false;
    uint16_t         m_downloadPort = 0;


    MiniXPath* m_xPathContainer;
    MiniXPath* m_xPathContainerAlt1;
    MiniXPath* m_xPathContainerAlt2;
    MiniXPath* m_xPathItem;
    MiniXPath* m_xPathItemAlt1;
    MiniXPath* m_xPathItemAlt2;
    MiniXPath* m_xPathNumberReturned;
    MiniXPath* m_xPathNumberReturnedAlt1;
    MiniXPath* m_xPathNumberReturnedAlt2;


    bool browseServer1(const uint32_t startingIndex = SOAP_DEFAULT_BROWSE_STARTING_INDEX,
                       const uint16_t maxCount = SOAP_DEFAULT_BROWSE_MAX_COUNT);
    int32_t  soapClientTimedRead(void);
    bool soapUDPmulticast(uint8_t repeats = 0);
    bool soapSSDPquery(std::vector<soapServer_t> *rcvd, int32_t msWait = SSDP_MAX_REPLY_TIMEOUT);
    bool soapGet(const IPAddress ip, const uint16_t port, const char *uri);
    bool soapPost(const IPAddress ip, const uint16_t port, const char *uri, const char *objectId,
                  const uint32_t startingIndex, const uint16_t maxCount);
    bool soapReadHttpHeader(uint64_t *contentLength, bool *chunked = NULL);
    int32_t  soapReadXML(bool chunked = false, bool replace = false);
    bool soapScanAttribute(const String *attributes, String *result, const char *searchFor);
    bool soapScanContainer(const String *parentId, const String *attributes, const String *container);
    bool soapScanItem(const String *parentId, const String *attributes, const String *item);
    bool allocate_MiniXPath();
    void release_MiniXPath();
public:
    void loop();
    bool listServer();
    bool browseServer(const uint8_t srv, const char *objectId);
    uint16_t getMediaDownloadPort();
    String getMediaDownloadIP();
};

#endif