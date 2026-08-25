/*
 * websrv.h
 *
 *  Created on: 09.07.2017
 *  updated on: 22.08.2026
 *      Author: Wolle
 */

#ifndef WEBSRV_H_
#define WEBSRV_H_
#include "Audio.h"
#include "base64.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha1.h"

#define ANSI_ESC_RED "\033[31m"

class WebSrv {
  public:
    WebSrv();
    ~WebSrv();

  protected:
    NetworkClient cmdclient; // An instance of the client for commands
    NetworkClient webSocketClient;
    NetworkServer cmdserver;
    NetworkServer webSocketServer;

    // callbacks ---------------------------------------------------------
  public:
    typedef enum { evt_info = 0, evt_error, evt_warn, evt_command, evt_request , evt_delete} event_t;
    struct msg_s {
        const char*  msg = nullptr;
        const char*  s = nullptr;
        ps_ptr<char> param;
        ps_ptr<char> cmd;
        ps_ptr<char> arg;
        ps_ptr<char> ct;             // contentType
        uint32_t     cl = 0;         // contentLength
        event_t      e = (event_t)0; // event type
    };

    using BrowseCallback = std::function<void(const msg_s&)>;
    void websrv_callbak(BrowseCallback cb) { m_websrv_callback = std::move(cb); }

  private:
    BrowseCallback m_websrv_callback;
    // -------------------------------------------------------------------

  private:
    msg_s        m_msg;
    bool         http_reponse_flag = false;    // Response required
    bool         ws_conn_request_flag = false; // websocket connection attempt
    bool         hasclient_WS = false;
    bool         cmdClientAccept = true;
    ps_ptr<char> m_name;
    ps_ptr<char> m_version;
    ps_ptr<char> m_httpRespHdrBuff; // store http response header
    uint8_t      method;
    String       WS_sec_Key;
    ps_ptr<char> WS_resp_Key;
    bool         m_handle_upload = false;

    struct upload_items {
        File         uploadfile{};
        ps_ptr<char> endBoundary{};
        uint16_t     max_endBoundary_length{};
        uint32_t     bytes_left{};

        void reset() { *this = upload_items{}; }
    };
    upload_items m_upload_items;

    struct HttpRequest {
        enum class Method { Unknown, GET, POST, DELETE };

        Method       method = Method::Unknown;
        ps_ptr<char> cmd;
        ps_ptr<char> param;
        ps_ptr<char> arg;
    };

  protected:
    ps_ptr<char>              createWebSocketAccept(const ps_ptr<char>& wsSecKey);
    void                      printWebSocketHeader(ps_ptr<char> wsRespKey);
    ps_ptr<char>              getContentType(ps_ptr<char>& filename);
    int32_t                   webFileRead();
    int32_t                   webFileRead(uint16_t timeout_ms);
    int32_t                   webFileRead(uint8_t* buff, size_t len);
    int32_t                   webFileRead(uint8_t* buff, size_t len, uint16_t timeout_ms);
    std::vector<ps_ptr<char>> readHeader();
    bool                      parseRequestLine(const ps_ptr<char>& line, HttpRequest& req);
    boolean                   handlehttp();
    boolean                   handleWS();
    void                      parseWsMessage(uint32_t len);
    void                      handle_upload_file();

  public:
    enum { HTTP_NONE = 0, HTTP_GET = 1, HTTP_POST = 2, HTTP_PUT = 3 };
    enum { Continuation_Frame = 0x00, Text_Frame = 0x01, Binary_Frame = 0x02, Connection_Close_Frame = 0x08, Ping_Frame = 0x09, Pong_Frame = 0x0A };
    void begin(uint16_t http_port = 80, uint16_t websocket_port = 81, ps_ptr<char> name = "", ps_ptr<char> version = "");
    void stop();
    void loop();
    void show(ps_ptr<char> pagename, ps_ptr<char> MIMEType, int16_t len = -1);
    void show_not_found();
    bool streamfile(fs::FS& fs, ps_ptr<char> path);
    bool send(ps_ptr<char> cmd, ps_ptr<char> msg = "", uint8_t opcode = Text_Frame);
    void sendPing();
    void sendPong();
    bool uploadfile(fs::FS& fs, ps_ptr<char> path, uint32_t contentLength, ps_ptr<char> contentType);
    bool uploadB64image(fs::FS& fs, ps_ptr<char> path, uint32_t contentLength);
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

    int32_t min3(int32_t a, int32_t b, int32_t c) {
        uint32_t min_val = a;
        if (b < min_val) min_val = b;
        if (c < min_val) min_val = c;
        return min_val;
    }

    //--------------------------------------------------------------------------------------------------------------

// Macro for comfortable calls
#define WS_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define WS_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define WS_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define WS_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define WS_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
};

#endif /* WEBSRV_H_ */
