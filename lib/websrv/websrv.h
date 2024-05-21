/*
 * websrv.h
 *
 *  Created on: 09.07.2017
 *  updated on: 20.01.2024
 *      Author: Wolle
 */

#ifndef WEBSRV_H_
#define WEBSRV_H_
#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "FS.h"
#include "mbedtls/sha1.h"
#include "base64.h"

extern __attribute__((weak)) void WEBSRV_onInfo(const char*);
extern __attribute__((weak)) void WEBSRV_onCommand(const String cmd, const String param, const String arg);
extern __attribute__((weak)) void WEBSRV_onRequest(const String, uint32_t contentLength);



class WebSrv
{
protected:
    WiFiClient      cmdclient;                               // An instance of the client for commands
    WiFiClient      webSocketClient ;
    WiFiServer      cmdserver;
    WiFiServer      webSocketServer;

private:
    bool            http_reponse_flag = false ;               // Response required
    bool            ws_conn_request_flag = false;             // websocket connection attempt
    bool            hasclient_WS = false;
    String          http_rqfile ;                             // Requested file
    String          http_cmd ;                                // Content of command
    String          http_param;                               // Content of parameter
    String          http_arg;                                 // Content of argument
    String          _Name;
    String          _Version;
    String          contenttype;
    char            buff[256];
    char*           msgBuff = NULL;
    char*           m_transBuf = NULL;
    char*           m_path = NULL;
    uint8_t         m_pathLength;
    uint8_t         method;
    String          WS_sec_Key;
    String          WS_resp_Key;
    String          WS_sec_conKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    uint16_t        m_bytesPerTransaction;


protected:
    String  calculateWebSocketResponseKey(String sec_WS_key);
    void    printWebSocketHeader(String wsRespKey);
    String  getContentType(String filename);
    boolean handlehttp();
    boolean handleWS();
    void    parseWsMessage(uint32_t len);
    String  URLdecode(String str);
    String  UTF8toASCII(String str);
    String  responseCodeToString(int32_t code);

public:
    enum { HTTP_NONE = 0, HTTP_GET = 1, HTTP_POST = 2, HTTP_PUT = 3 };
    enum { Continuation_Frame = 0x00, Text_Frame = 0x01, Binary_Frame = 0x02, Connection_Close_Frame = 0x08,
           Ping_Frame = 0x09, Pong_Frame = 0x0A };
    WebSrv(String Name="WebSrv library", String Version="1.0");
    ~WebSrv();
    void begin(uint16_t http_port = 80, uint16_t websocket_port = 81);
    void stop();
    void loop();
    void show(const char* pagename, const char* MIMEType, int16_t len=-1);
    void show_not_found();
    boolean streamfile(fs::FS &fs,const char* path);
    boolean send(const char* cmd, String msg, uint8_t opcode = Text_Frame);
    boolean send(const char* cmd, const char* msg = "", uint8_t opcode = Text_Frame);
    void    sendPing();
    void    sendPong();
    boolean uploadfile(fs::FS &fs,const char* path, uint32_t contentLength);
    boolean uploadB64image(fs::FS &fs,const char* path, uint32_t contentLength);
    void reply(const String response, const char* MIMEType, boolean header=true);
    void sendStatus(uint16_t HTTPstatusCode);


    const char JSON[17]  = "application/json";
    const char TEXT[10]  = "text/html";
    const char JS[23]    = "application/javascript";
    const char BMP[15]   = "image/bmp";
    const char JPG[15]   = "image/jpeg";
    const char PNG[15]   = "image/png";

private:
    const int32_t B64index[123] ={
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
        0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
        0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };


    int32_t indexOf (const char* base, char ch, int32_t startIndex = 0) {
    //fb
        const char *p = base;
        for (; startIndex > 0; startIndex--)
            if (*p++ == '\0') return -1;
        char *pos = strchr(p, ch);
        if (pos == nullptr) return -1;
        return pos - base;
    }

    int32_t lastIndexOf(const char* haystack, const char needle) {
    //fb
        const char *p = strrchr(haystack, needle);
        return (p ? p - haystack : -1);
    }

    char* x_ps_malloc(uint16_t len) {
        char* ps_str = NULL;
        if(psramFound()){ps_str = (char*) ps_malloc(len);}
        else            {ps_str = (char*)    malloc(len);}
        return ps_str;
}



//--------------------------------------------------------------------------------------------------------------





};


#endif /* WEBSRV_H_ */
