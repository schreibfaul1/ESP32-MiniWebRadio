// Created on: 30.11.2023
// Updated on: 30.09.2025

#pragma once

#include "Audio.h"

#define SSDP_MULTICAST_IP   239, 255, 255, 250
#define SSDP_LOCAL_PORT     8888
#define SSDP_MULTICAST_PORT 1900
#define SEEK_TIMEOUT        8000
#define READ_TIMEOUT        2500
#define CONNECT_TIMEOUT     6000
#define AVAIL_TIMEOUT       2000

class DLNA_Client {

  public:
    struct dlnaServer {
        ps_ptr<char> ip{};
        uint16_t     port{};
        ps_ptr<char> location{};
        ps_ptr<char> friendlyName{};
        ps_ptr<char> controlURL{};
        uint16_t     presentationPort{};
        ps_ptr<char> presentationURL{};
    };

  private:
    std::deque<dlnaServer> m_dlnaServer;

  public:
    struct srvItem {
        ps_ptr<char> objectId{};
        ps_ptr<char> parentId{};
        bool         isAudio{};
        ps_ptr<char> itemURL{};
        int32_t      itemSize{};
        ps_ptr<char> duration{};
        ps_ptr<char> title{};
        int16_t      childCount{};
    };

  private:
    std::deque<srvItem> m_srv_items;

    // callbacks ---------------------------------------------------------
  public:
    typedef enum { evt_content = 0, evt_server = 1 } event_t;
    struct msg_s { // used in info(audio_info_callback());
        const char*             msg = nullptr;
        const char*             s = nullptr;
        event_t                 e = (event_t)0; // event type
        std::deque<srvItem>*    items = nullptr;
        std::deque<dlnaServer>* server = nullptr;
        int16_t                 numberReturned = 0;
        int16_t                 totalMatches = 0;
        const char*             IP_addr = "";
        const char*             friendlyName = "";
    };
    using BrowseCallback = std::function<void(const msg_s&)>;
    void dlna_client_callbak(BrowseCallback cb) { m_dlna_callback = std::move(cb); }

  private:
    BrowseCallback m_dlna_callback;
    // -------------------------------------------------------------------

  public:
    DLNA_Client();
    ~DLNA_Client();

  private:
    NetworkClient m_tcp_client;
    WiFiUDP       m_udp_client;
    uint8_t       m_state = IDLE;
    uint32_t      m_timeStamp = 0;
    int16_t       m_numberReturned = -1;
    int16_t       m_totalMatches = -1;
    ps_ptr<char>  m_JSONstr;

    std::deque<ps_ptr<char>> m_content;

  public:
    bool                          seekServer();
    int8_t                        listServer();
    const std::deque<dlnaServer>& getServer() const;
    const std::deque<srvItem>&    getBrowseResult() const;
    int8_t                        browseServer(uint8_t srvNr, const char* objectId, const uint16_t startingIndex = 0, const uint16_t maxCount = 50);
    const char*                   stringifyContent();
    const char*                   stringifyServer();
    uint8_t                       getState();
    int16_t                       getTotalMatches() {
        if (m_state == IDLE)
            return m_totalMatches;
        else
            return -1;
    }
    int8_t getNrOfServers() {
        if (m_state == IDLE)
            return m_dlnaServer.size();
        else
            return -1;
    }
    void loop();

    enum { IDLE, SEEK_SERVER, GET_SERVER_ITEMS, READ_HTTP_HEADER, BROWSE_SERVER };

  private:
    void    parseDlnaServer(uint16_t len);
    bool    getServerItems(uint8_t srvNr);
    bool    browseResult();
    bool    srvGet(uint8_t srvNr);
    bool    readHttpHeader();
    int32_t getChunkSize(uint16_t* readedBytes);
    bool    readContent();
    bool    srvPost(uint8_t srvNr, const char* objectId, const uint16_t startingIndex, const uint16_t maxCount);

  private:
    bool     m_chunked = false;
    bool     m_skipCRLF = false;
    char     m_objectId[60];
    uint8_t  m_srvNr = 0;
    uint32_t m_contentlength = 0;
    uint16_t m_startingIndex = 0;
    uint16_t m_maxCount = 100;
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    int32_t indexOf(const char* haystack, const char* needle, int32_t startIndex) {
        const char* p = haystack;
        for (; startIndex > 0; startIndex--)
            if (*p++ == '\0') return -1;
        char* pos = strstr(p, needle);
        if (pos == nullptr) return -1;
        return pos - haystack;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Macro for comfortable calls
#define DLNA_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLNA_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLNA_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLNA_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DLNA_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

