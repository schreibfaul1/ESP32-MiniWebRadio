/*
 * html.h
 *
 *  Created on: 09.07.2017
 *  updated on: 24.06.2021
 *      Author: Wolle
 */

#ifndef HTML_H_
#define HTML_H_
#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "FS.h"
#include "mbedtls/sha1.h"
#include "base64.h"

extern __attribute__((weak)) void HTML_info(const String) ;
extern __attribute__((weak)) void HTML_command(const String, const String, const String);
extern __attribute__((weak)) void HTML_file(String);
extern __attribute__((weak)) void HTML_request(const String, uint32_t contentLength);



class HTML
{
protected:
    WiFiClient      cmdclient;                               // An instance of the client for commands
    WiFiClient      webSocketClient ;
    WiFiServer      cmdserver;
    WiFiServer      webSocketServer;

private:
    bool            http_reponse_flag = false ;               // Response required
    bool            ws_conn_request_flag = false;             // websocket connection attempt
    bool            hasclient = false;
    String          http_rqfile ;                             // Requested file
    String          http_cmd ;                                // Content of command
    String          http_param;                               // Content of parameter
    String          http_arg;                                 // Content of argument
    String          _Name;
    String          _Version;
    String          contenttype;
    uint8_t         buff[256];
    uint8_t         method;
    String          WS_sec_Key;
    String          WS_resp_Key;
    String          WS_sec_conKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

protected:
    String calculateWebSocketResponseKey(String sec_WS_key);
    void    printWebSocketHeader(String wsRespKey);
    String  getContentType(String filename);
    boolean handlehttp();
    boolean handleWS();
    void    parseWsMessage(uint16_t len);
    uint8_t inbyte();
    String  URLdecode(String str);
    String  UTF8toASCII(String str);
    String  responseCodeToString(int code);


public:
    enum { HTTP_NONE = 0, HTTP_GET = 1, HTTP_PUT = 2 };
    HTML(String Name="HTML library", String Version="1.0");
    void begin();
    void stop();
    boolean loop();
    void show(const char* pagename, int16_t len=-1);
    void show_not_found();
    boolean streamfile(fs::FS &fs,const char* path);
    boolean send(String msg);
    boolean send(const char* msg);
    void    sendPing();
    void    sendPong();
    boolean uploadfile(fs::FS &fs,const char* path, uint32_t contentLength);
    boolean uploadB64image(fs::FS &fs,const char* path, uint32_t contentLength);
    void reply(const String &response, boolean header=true);
    const char* ASCIItoUTF8(const char* str);

private:
    const int B64index[123] ={
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
        0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
        0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };
};


#endif /* HTML_H_ */
