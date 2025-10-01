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

extern __attribute__((weak)) void dlna_info(const char*);
extern __attribute__((weak)) void dlna_server(uint8_t serverId, const char* IP_addr, uint16_t port, const char* friendlyName, const char* controlURL);
extern __attribute__((weak)) void dlna_seekReady(uint8_t numberOfServer);
extern __attribute__((weak)) void dlna_browseResult(const char* objectId, const char* parentId, uint16_t childCount, const char* title, bool isAudio, uint32_t itemSize, const char* duration,
                                                    const char* itemURL);
extern __attribute__((weak)) void dlna_browseReady(uint16_t numberReturned, uint16_t totalMatches);

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

  public:
    struct browseReady_s {
        uint16_t numberReturned;
        uint16_t totalMatches;
    };

  public:
    DLNA_Client();
    ~DLNA_Client();

  private:
    browseReady_s m_browseReady;

  private:
    NetworkClient m_client;
    WiFiUDP       m_udp;
    uint8_t       m_state = IDLE;
    uint32_t      m_timeStamp = 0;
    uint16_t      m_numberReturned = 0;
    uint16_t      m_totalMatches = 0;
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
    void parseDlnaServer(uint16_t len);
    bool getServerItems(uint8_t srvNr);
    bool browseResult();
    bool srvGet(uint8_t srvNr);
    bool readHttpHeader();
    bool readContent();
    bool srvPost(uint8_t srvNr, const char* objectId, const uint16_t startingIndex, const uint16_t maxCount);

  private:
    bool     m_PSRAMfound = false;
    bool     m_chunked = false;
    char*    m_chbuf = NULL;
    char     m_objectId[60];
    uint8_t  m_srvNr = 0;
    uint16_t m_chbufSize = 0;
    uint32_t m_contentlength = 0;
    uint16_t m_startingIndex = 0;
    uint16_t m_maxCount = 100;

    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    void vector_clear_and_shrink(std::vector<char*>& vec) {
        uint size = vec.size();
        for (int32_t i = 0; i < size; i++) {
            if (vec[i]) {
                free(vec[i]);
                vec[i] = NULL;
            }
        }
        vec.clear();
        vec.shrink_to_fit();
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    void srvContent_clear_and_shrink() {
        // m_srvContent.size = 0;
        // vector_clear_and_shrink(m_srvContent.objectId);
        // vector_clear_and_shrink(m_srvContent.parentId);
        // m_srvContent.isAudio.clear();
        // m_srvContent.isAudio.shrink_to_fit();
        // vector_clear_and_shrink(m_srvContent.itemURL);
        // m_srvContent.itemSize.clear();
        // m_srvContent.itemSize.shrink_to_fit();
        // vector_clear_and_shrink(m_srvContent.duration);
        // vector_clear_and_shrink(m_srvContent.title);
        // m_srvContent.childCount.clear();
        // m_srvContent.childCount.shrink_to_fit();
    }
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
    bool startsWith(const char* base, const char* searchString) {
        char c;
        while ((c = *searchString++) != '\0')
            if (c != *base++) return false;
        return true;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    bool endsWith(const char* base, const char* searchString) {
        int32_t slen = strlen(searchString);
        if (slen == 0) return false;
        const char* p = base + strlen(base);
        p -= slen;
        if (p < base) return false;
        return (strncmp(p, searchString, slen) == 0);
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    int replacestr(char* line, const char* search, const char* replace) { /* returns number of strings replaced.*/
        int   count;
        char* sp; // start of pattern
        // printf("replacestr(%s, %s, %s)\n", line, search, replace);
        if ((sp = strstr(line, search)) == NULL) { return (0); }
        count = 1;
        int sLen = strlen(search);
        int rLen = strlen(replace);
        if (sLen > rLen) {
            // move from right to left
            char* src = sp + sLen;
            char* dst = sp + rLen;
            while ((*dst = *src) != '\0') {
                dst++;
                src++;
            }
        } else if (sLen < rLen) {
            // move from left to right
            int   tLen = strlen(sp) - sLen;
            char* stop = sp + rLen;
            char* src = sp + sLen + tLen;
            char* dst = sp + rLen + tLen;
            while (dst >= stop) {
                *dst = *src;
                dst--;
                src--;
            }
        }
        memcpy(sp, replace, rLen);
        count += replacestr(sp + rLen, search, replace);
        return (count);
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    inline char* x_ps_malloc(uint16_t len) {
        char* ps_str = NULL;
        if (psramFound()) {
            ps_str = (char*)ps_malloc(len);
        } else {
            ps_str = (char*)malloc(len);
        }
        if (!ps_str) { log_e("oom"); }
        ps_str[0] = '\0';
        return ps_str;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    char* x_ps_strdup(const char* str) {
        if (!str) { log_e("given str is NULL"); }
        char*    ps_str = NULL;
        uint16_t len = strlen(str);
        if (m_PSRAMfound) {
            ps_str = (char*)ps_malloc(len + 1);
        } else {
            ps_str = (char*)malloc(len + 1);
        }
        if (!ps_str) { log_e("oom"); }
        strcpy(ps_str, str);
        ps_str[len] = '\0';
        return ps_str;
    }
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    char* x_ps_strndup(const char* str, uint16_t len) {
        if (!str) {
            log_e("given str is NULL");
            return NULL;
        }
        size_t str_len = strlen(str);
        if (len > str_len) len = str_len;
        char* ps_str = NULL;
        if (m_PSRAMfound) {
            ps_str = (char*)ps_malloc(len + 1);
        } else {
            ps_str = (char*)malloc(len + 1);
        }
        if (!ps_str) {
            log_e("oom");
            return NULL;
        }
        strlcpy(ps_str, str, len + 1); // len+1 guarantees zero termination (ps_str + '\0')
        return ps_str;
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
