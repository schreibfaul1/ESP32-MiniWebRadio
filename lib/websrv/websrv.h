/*
 * websrv.h
 *
 *  Created on: 09.07.2017
 *  updated on: 05.05.2025
 *      Author: Wolle
 */

#ifndef WEBSRV_H_
#define WEBSRV_H_
#include "Audio.h"
#include "base64.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"

extern __attribute__((weak)) void WEBSRV_onCommand(const char* cmd, const String param, const String arg);
extern __attribute__((weak)) void WEBSRV_onRequest(const char* cmd, const char* param, const char* arg, const char* contentType, uint32_t contentLength);
extern __attribute__((weak)) void WEBSRV_onDelete(const char* cmd, const char* param, const char* arg);

#define ANSI_ESC_RED "\033[31m"

class WebSrv {
  public:
    WebSrv(String Name = "WebSrv library", String Version = "1.0");
    ~WebSrv();

  protected:
    NetworkClient cmdclient; // An instance of the client for commands
    NetworkClient webSocketClient;
    NetworkServer cmdserver;
    NetworkServer webSocketServer;

    // callbacks ---------------------------------------------------------
  public:
    typedef enum { evt_info = 0, evt_error, evt_command } event_t;
    struct msg_s {
        const char*  msg = nullptr;
        const char*  s = nullptr;
        const char*  param;
        const char*  command;
        ps_ptr<char> arg;
        ps_ptr<char> param1;
        ps_ptr<char> cmd;
        ps_ptr<char> arg1;
        event_t      e = (event_t)0; // event type
    };

    using BrowseCallback = std::function<void(const msg_s&)>;
    void websrv_callbak(BrowseCallback cb) { m_websrv_callback = std::move(cb); }

  private:
    BrowseCallback m_websrv_callback;
    // -------------------------------------------------------------------

  private:
    msg_s   m_msg;
    bool    http_reponse_flag = false;    // Response required
    bool    ws_conn_request_flag = false; // websocket connection attempt
    bool    hasclient_WS = false;
    bool    cmdClientAccept = true;
    String  http_rqfile; // Requested file
    String  http_cmd;    // Content of command
    String  http_param;  // Content of parameter
    String  http_arg;    // Content of argument
    String  _Name;
    String  _Version;
    String  contenttype;
    uint8_t method;
    String  WS_sec_Key;
    String  WS_resp_Key;
    String  WS_sec_conKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    bool    m_handle_upload = false;

    struct upload_items {
        File         uploadfile{};
        ps_ptr<char> endBoundary{};
        uint16_t     max_endBoundary_length{};
        uint32_t     bytes_left{};

        void reset() { *this = upload_items{}; }
    };
    upload_items m_upload_items;

  protected:
    String      calculateWebSocketResponseKey(String sec_WS_key);
    void        printWebSocketHeader(String wsRespKey);
    const char* getContentType(ps_ptr<char>& filename);
    boolean     handlehttp();
    boolean     handleWS();
    void        parseWsMessage(uint32_t len);
    String      URLdecode(String str);
    void        url_decode_in_place(char* url);
    String      UTF8toASCII(String str);
    String      responseCodeToString(int32_t code);
    void        handle_upload_file();

  public:
    enum { HTTP_NONE = 0, HTTP_GET = 1, HTTP_POST = 2, HTTP_PUT = 3 };
    enum { Continuation_Frame = 0x00, Text_Frame = 0x01, Binary_Frame = 0x02, Connection_Close_Frame = 0x08, Ping_Frame = 0x09, Pong_Frame = 0x0A };
    void begin(uint16_t http_port = 80, uint16_t websocket_port = 81);
    void stop();
    void loop();
    void show(const char* pagename, const char* MIMEType, int16_t len = -1);
    void show_not_found();
    bool streamfile(fs::FS& fs, ps_ptr<char> path);
    bool send(const char* cmd, int msg, uint8_t opcode = Text_Frame);
    bool send(ps_ptr<char> cmd, ps_ptr<char> msg = "", uint8_t opcode = Text_Frame);
    void sendPing();
    void sendPong();
    bool uploadfile(fs::FS& fs, ps_ptr<char> path, uint32_t contentLength, ps_ptr<char> contentType);
    bool uploadB64image(fs::FS& fs, const char* path, uint32_t contentLength);
    void reply(ps_ptr<char> response, const char* MIMEType, bool header = true);
    void sendStatus(uint16_t HTTPstatusCode);

    const char JSON[17] = "application/json";
    const char TEXT[10] = "text/html";
    const char JS[23] = "application/javascript";
    const char BMP[15] = "image/bmp";
    const char JPG[15] = "image/jpeg";
    const char PNG[15] = "image/png";

  private:
    static std::string sanitize_utf8_replace(const char* input, size_t len);

    int32_t      indexOf(const char* base, char ch, int32_t startIndex = 0) {
        // fb
        const char* p = base;
        for (; startIndex > 0; startIndex--)
            if (*p++ == '\0') return -1;
        char* pos = strchr(p, ch);
        if (pos == nullptr) return -1;
        return pos - base;
    }

    int indexOf(const char* base, const char* str, int startIndex = 0) {
        // fb
        const char* p = base;
        for (; startIndex > 0; startIndex--)
            if (*p++ == '\0') return -1;
        char* pos = strstr(p, str);
        if (pos == nullptr) return -1;
        return pos - base;
    }

    bool startsWith(const char* base, const char* str) {
        // fb
        char c;
        while ((c = *str++) != '\0')
            if (c != *base++) return false;
        return true;
    }

    int32_t lastIndexOf(const char* haystack, const char needle) {
        // fb
        const char* p = strrchr(haystack, needle);
        return (p ? p - haystack : -1);
    }

    char* x_ps_malloc(uint16_t len) {
        char* ps_str = NULL;
        if (psramFound()) {
            ps_str = (char*)ps_malloc(len);
        } else {
            ps_str = (char*)malloc(len);
        }
        return ps_str;
    }
    void trim(char* str) {
        char* start = str; // keep the original pointer
        char* end;
        while (isspace((unsigned char)*start)) start++; // find the first non-space character

        if (*start == 0) { // all characters were spaces
            str[0] = '\0'; // return a empty string
            return;
        }

        end = start + strlen(start) - 1; // find the end of the string

        while (end > start && isspace((unsigned char)*end)) end--;
        end[1] = '\0'; // Null-terminate the string after the last non-space character

        // Move the trimmed string to the beginning of the memory area
        memmove(str, start, strlen(start) + 1); // +1 for '\0'
    }

    int32_t min3(int32_t a, int32_t b, int32_t c) {
        uint32_t min_val = a;
        if (b < min_val) min_val = b;
        if (c < min_val) min_val = c;
        return min_val;
    }

    //--------------------------------------------------------------------------------------------------------------

// Macro for comfortable calls
#define WS_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WS_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WS_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WS_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define WS_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
};

#endif /* WEBSRV_H_ */
