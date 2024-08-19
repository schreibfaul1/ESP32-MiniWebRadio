/*
 * websrv.cpp
 *
 *  Created on: 09.07.2017
 *  updated on: 19.08.2024
 *      Author: Wolle
 */

#include "websrv.h"

//--------------------------------------------------------------------------------------------------------------
WebSrv::WebSrv(String Name, String Version){
    _Name=Name; _Version=Version;
    method = HTTP_NONE;
    m_bytesPerTransaction = 16384;
    m_transBuf = x_ps_malloc(m_bytesPerTransaction);
    if(!m_transBuf){log_e("WebServer: not enough memory");}
    m_pathLength = 255;
    m_path = x_ps_malloc(m_pathLength);
}
//--------------------------------------------------------------------------------------------------------------
WebSrv::~WebSrv(){
    if(m_transBuf){free(m_transBuf); m_transBuf = NULL;}
    if(m_path){free(m_path); m_path = NULL;}
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::show_not_found(){
    cmdclient.print("HTTP/1.1 404 Not Found\n\n");
    return;
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::calculateWebSocketResponseKey(String sec_WS_key){
    // input  Sec-WebSocket-Key from client
    // output Sec-WebSocket-Accept-Key (used in response message to client)
    uint8_t sha1_result[20];
    String concat = sec_WS_key + WS_sec_conKey;
    mbedtls_sha1((unsigned char*)concat.c_str(), concat.length(), (unsigned char*) sha1_result );
    return base64::encode(sha1_result, 20);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::printWebSocketHeader(String wsRespKey){
    String wsHeader = (String)"HTTP/1.1 101 Switching Protocols\r\n"  +
                              "Upgrade: websocket\r\n"  +
                              "Connection: Upgrade\r\n" +
                              "Sec-WebSocket-Accept: "  + wsRespKey + "\r\n" +
                              "Access-Control-Allow-Origin: \r\n\r\n";
                             // "Sec-WebSocket-Protocol: chat\r\n\r\n";
    //log_i("wsheader %s", wsHeader.c_str());
    webSocketClient.print(wsHeader) ;             // header sent
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::show(const char* pagename, const char* MIMEType, int16_t len){
    uint TCPCHUNKSIZE = 1024;   // Max number of bytes per write
    size_t pagelen=0, res=0;                    // Size of requested page
    const uint8_t*p;
    p = reinterpret_cast<const unsigned char*>(pagename);
    if(len==-1){
        pagelen=strlen(pagename);
    }
    else{
        if(len>0) pagelen = len;
    }
    while((*p=='\n') && (pagelen>0)){         // If page starts with newline:
        p++;                            // Skip first character
        pagelen--;
    }
    // HTTP header
    String httpheader="";
    httpheader += "HTTP/1.1 200 OK\r\n";
    httpheader += "Connection: close\r\n";
    httpheader += "Content-type: " + (String)MIMEType + "\r\n";
    httpheader += "Content-Length: " + String(pagelen, 10) + "\r\n";
    httpheader += "Server: " + _Name+ "\r\n";
    httpheader += "Cache-Control: max-age=86400\r\n";
    httpheader += "Last-Modified: " + _Version + "\r\n\r\n";

    cmdclient.print(httpheader) ;             // header sent

    sprintf(buff, "Length of page is %d", pagelen);
    if(WEBSRV_onInfo) WEBSRV_onInfo(buff);
    // The content of the HTTP response follows the header:

    while(pagelen){                       // Loop through the output page
        if (pagelen <= TCPCHUNKSIZE){     // Near the end?
            res=cmdclient.write(p, pagelen);  // Yes, send last part
            if(res!=pagelen){
                log_e("write error in webpage");
                cmdclient.clearWriteError();
                return;
            }
            pagelen = 0;
        }
        else{

            res=cmdclient.write(p, TCPCHUNKSIZE);   // Send part of the page

            if(res!=TCPCHUNKSIZE){
                log_e("write error in webpage");
                cmdclient.clearWriteError();
                return;
            }
            p += TCPCHUNKSIZE;                  // Update startpoint and rest of bytes
            pagelen -= TCPCHUNKSIZE;
        }
    }
    return;
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::streamfile(fs::FS &fs,const char* path){ // transfer file from SD to webbrowser
    if(path == NULL){ log_e("path is NULL"); return false;}                                                     // guard1
    if(strlen(path) > m_pathLength){ log_e("pathLength > %i", m_pathLength); return false;}                     // guard2
    int i = 0; while(path[i] != 0){ if(path[i] < 32) {log_e("illegal character in path"); return false;}; i++;} // guard3
    if(!cmdclient.connected()){ /* log_e("cmdclient is not connected"); */  return false;}                              // guard4

    size_t wIndex = 0, res=0, leftover=0;
    String httpheader="";
    File file;
    int idx = 0;

    strcpy(m_path, path);
    idx = indexOf(m_path, '?', 0);
    if(idx != -1) m_path[idx] = '\0';  // remobe all after '?'
    if(!fs.exists(m_path)){ /* log_e("path not found %s", m_path); */  return false;}

    file = fs.open(m_path, "r");
    if(!file){
        sprintf(buff, "Failed to open file for reading %s", m_path);
        if(WEBSRV_onInfo) WEBSRV_onInfo(buff);
        show_not_found();
        return false;
    }

    sprintf(buff, "Length of file %s is %d", m_path, file.size());
    if(WEBSRV_onInfo) WEBSRV_onInfo(buff);

    // HTTP header
    httpheader += "HTTP/1.1 200 OK\r\n";
    httpheader += "Connection: close\r\n";
    httpheader += "Content-type: " + getContentType(String(m_path)) + "\r\n";
    httpheader += "Content-Length: " + String(file.size(),10) + "\r\n";
    httpheader += "Cache-Control: max-age=86400\r\n\r\n";

    cmdclient.print(httpheader) ;             // header sent

    while(wIndex+m_bytesPerTransaction < file.size()){
        file.read((uint8_t*)m_transBuf, m_bytesPerTransaction);
        res=cmdclient.write(m_transBuf, m_bytesPerTransaction);
        wIndex+=res;
        if(res!=m_bytesPerTransaction){
            log_i("write error %s", m_path);
            cmdclient.clearWriteError();
            goto error;
        }
    }
    leftover=file.size()-wIndex;
    file.read((uint8_t*)m_transBuf, leftover);
    res=cmdclient.write(m_transBuf, leftover);
    wIndex+=res;
    if(res!=leftover){
        log_i("write error %s", m_path);
        cmdclient.clearWriteError();
        goto error;
    }
    if(wIndex!=file.size()) log_e("file %s not correct sent", m_path);
    file.close();
    cmdclient.stop();
    return true;

error:
    return false;
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::send(const char* cmd, String msg, uint8_t opcode) {  // sends text messages via websocket
    return send(cmd, msg.c_str(), opcode);
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::send(const char* cmd, const char *msg, uint8_t opcode) {  // sends text messages via websocket
    uint8_t headerLen = 2;

    if(!hasclient_WS) {
//      log_e("can't send, websocketserver not connected");
        return false;
    }
    size_t cmdLen = strlen(cmd);
    size_t msgLen = strlen(msg);

    if(msgLen > UINT16_MAX) {
        log_e("send: message too long, greater than 64kB");
        return false;
    }

    uint8_t fin = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;

    buff[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + opcode;
    if(msgLen + cmdLen < 126) {
        buff[1] = (128 * mask) + cmdLen + msgLen ;
    }
    else {
        headerLen = 4;
        buff[1] = (128 * mask) + 126;
        buff[2] = ((cmdLen + msgLen) >> 8) & 0xFF;
        buff[3] = (cmdLen + msgLen) & 0xFF;
    }

    webSocketClient.write(buff, headerLen);
    webSocketClient.write(cmd, cmdLen);
    webSocketClient.write(msg, msgLen);

    return true;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::sendPing(){  // heartbeat, keep alive via websockets

    if(!hasclient_WS) {
        return;
    }
    uint8_t fin  = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;

    buff[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + Ping_Frame;
    buff[1] = (128 * mask) + 0;
    webSocketClient.write(buff,2);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::sendPong(){  // heartbeat, keep alive via websockets

    if(!hasclient_WS) {
        return;
    }
    uint8_t fin  = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;

    buff[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + Pong_Frame;
    buff[1] = (128 * mask) + 0;
    webSocketClient.write(buff,2);
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::uploadB64image(fs::FS &fs,const char* path, uint32_t contentLength){ // transfer imagefile from webbrowser to SD
    size_t   m_bytesPerTransaction = 1024;
    uint8_t  tBuf[m_bytesPerTransaction];
    uint16_t av, i, j;
    uint32_t len = contentLength;
    boolean f_werror=false;
    String str="";
    int32_t n=0;
    File file;
    fs.remove(path); // Remove a previous version, otherwise data is appended the file again
    file = fs.open(path, FILE_WRITE);  // Open the file for writing (create it, if doesn't exist)

    log_i("ContentLength %i", contentLength);
    str = str + cmdclient.readStringUntil(','); // data:image/jpeg;base64,
    len -= str.length();
    while(cmdclient.available()){
        av=cmdclient.available();
        if(av==0) break;
        if(av>m_bytesPerTransaction) av=m_bytesPerTransaction;
        if(av>len) av=len;
        len -= av;
        i=0; j=0;
        cmdclient.read(tBuf, av); // b64 decode
        while(i<av){
            if(tBuf[i]==0)break; // ignore all other stuff
            n=B64index[tBuf[i]]<<18 | B64index[tBuf[i+1]]<<12 | B64index[tBuf[i+2]]<<6 | B64index[tBuf[i+3]];
            tBuf[j  ]= n>>16;
            tBuf[j+1]= n>>8 & 0xFF;
            tBuf[j+2]= n & 0xFF;
            i+=4;
            j+=3;
        }
        if(tBuf[j]=='=') j--;
        if(tBuf[j]=='=') j--; // remove =

        if(file.write(tBuf, j)!=j) f_werror=true;  // write error?
        if(len == 0) break;
    }
    cmdclient.readStringUntil('\n'); // read the remains, first \n
    cmdclient.readStringUntil('\n'); // read the remains  webkit\n
    file.close();
    if(f_werror) {
        sprintf(buff, "File: %s write error", path);
        if(WEBSRV_onInfo) WEBSRV_onInfo(buff);
        return false;
    }
    sprintf(buff, "File: %s written, FileSize: %ld", path, (unsigned long)contentLength);
    //log_i(buff);
    if(WEBSRV_onInfo) WEBSRV_onInfo(buff);
    return true;
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::uploadfile(fs::FS &fs,const char* path, uint32_t contentLength){ // transfer file from webbrowser to sd

    uint16_t av;
    uint32_t len = contentLength;
    boolean f_werror=false;
    String str="";
    File file;
    if(fs.exists(path)) fs.remove(path); // Remove a previous version, otherwise data is appended the file again

    file = fs.open(path, FILE_WRITE);  // Open the file for writing in SD (create it, if doesn't exist)
    uint32_t t = millis();
    while(true){
        if(cmdclient.available()){
            t = millis();
            av=cmdclient.available();
            if(av>m_bytesPerTransaction) av=m_bytesPerTransaction;
            if(av>len) av=len;
            len -= av;
            cmdclient.read((uint8_t*)m_transBuf, av);
            if(file.write((uint8_t*)m_transBuf, av)!=av) f_werror=true;  // write error?
        }
        if((t + 2000) < millis()) { log_e("timeout"); goto exit;}
        if(len == 0) break;
        if(f_werror) {log_e("error while writing on SD"); goto exit;}
    }
    cmdclient.readStringUntil('\n'); // read the remains, first \n
    cmdclient.readStringUntil('\n'); // read the remains  webkit\n
    file.close();
    if(f_werror) {
        sprintf(buff, "File: %s write error", path);
        if(WEBSRV_onInfo) WEBSRV_onInfo(buff);
        goto exit;
    }
    sprintf(buff, "File: %s written, FileSize %ld: ", path, (long unsigned int)contentLength);
    if(WEBSRV_onInfo)  WEBSRV_onInfo(buff);
    return true;
exit:
    return false;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::begin(uint16_t http_port, uint16_t websocket_port) {
    cmdserver.begin(http_port);
    webSocketServer.begin(websocket_port);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::stop() {
    webSocketClient.stop();
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::getContentType(String filename){
    if      (filename.endsWith(".html")) return "text/html" ;
    else if (filename.endsWith(".htm" )) return "text/html";
    else if (filename.endsWith(".css" )) return "text/css";
    else if (filename.endsWith(".txt" )) return "text/plain";
    else if (filename.endsWith(".js"  )) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".svg" )) return "image/svg+xml";
    else if (filename.endsWith(".ttf" )) return "application/x-font-ttf";
    else if (filename.endsWith(".otf" )) return "application/x-font-opentype";
    else if (filename.endsWith(".xml" )) return "text/xml";
    else if (filename.endsWith(".pdf" )) return "application/pdf";
    else if (filename.endsWith(".png" )) return "image/png" ;
    else if (filename.endsWith(".bmp" )) return "image/bmp" ;
    else if (filename.endsWith(".gif" )) return "image/gif" ;
    else if (filename.endsWith(".jpg" )) return "image/jpeg" ;
    else if (filename.endsWith(".ico" )) return "image/x-icon" ;
    else if (filename.endsWith(".css" )) return "text/css" ;
    else if (filename.endsWith(".zip" )) return "application/x-zip" ;
    else if (filename.endsWith(".gz"  )) return "application/x-gzip" ;
    else if (filename.endsWith(".xls" )) return "application/msexcel" ;
    else if (filename.endsWith(".mp3" )) return "audio/mpeg" ;
    else if (filename.endsWith(".csv" )) return "text/csv";
    return "text/plain" ;
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::handlehttp() {                // HTTPserver, message received
    bool wswitch=true;
    int16_t inx1 = 0, inx2 = 0;               // Pos. of search string in currenLine
    String currentLine = "";                  // Build up to complete line
    String request = "";
    String ct;                                // contentType
    String xFilename = "";
    uint32_t contentLength = 0;               // contentLength
    uint8_t count = 0;
    uint8_t method = HTTP_NONE;

    currentLine = "";
    http_cmd    = "";
    http_param  = "";
    http_arg    = "";
    http_rqfile = "";

    while (wswitch==true &&  cmdclient.connected()){            // first while
        while(cmdclient.available()){
            currentLine = cmdclient.readStringUntil('\n');
            // log_i("currLine %s", currentLine.c_str());
            // If the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() <= 1) { // contains '\n' only

                request = URLdecode(request);
                inx1 = request.indexOf("?");        // Search for 1st parameter
                inx2 = request.lastIndexOf("&");    // Search for 2nd parameter

                if(inx1 > 0){     // it is a command
                    http_cmd = request.substring(0, inx1); //isolate the command
                    http_rqfile = "";               // No file
                }
                if((inx1 > 0) && (inx2 > inx1 + 1)){        // it is a parameter
                    http_param = request.substring(inx1 + 1, inx2);//isolate the parameter
                    if(inx2 < request.length()){
                        http_arg = request.substring(inx2 + 1, request.length());
                    }
                }
                if(inx1 > 0 && inx2 < 0){
                    http_param = request.substring(inx1 + 1, request.length());
                    http_arg = "";
                }
                if(inx1 < 0 && inx2 < 0){   // it is a filename
                //    log_w("cl %i", contentLength);
                    if(xFilename.length()){ http_rqfile = xFilename; http_arg = String(contentLength, DEC); http_cmd = request;}
                    else                  { http_rqfile = request; http_cmd =   "";}
                    http_param = "";
                    http_arg =   "";
                }

                if (http_cmd.length()) {
                    if(xFilename.length()) http_param = xFilename;
                    if(WEBSRV_onInfo) WEBSRV_onInfo(URLdecode(http_cmd).c_str());
                    if(WEBSRV_onCommand) WEBSRV_onCommand(URLdecode(http_cmd), URLdecode(http_param), URLdecode(http_arg));
                }
                else if(http_rqfile.length()){
                    http_cmd = http_rqfile;
                    if(WEBSRV_onInfo) WEBSRV_onInfo(URLdecode(http_rqfile).c_str());
                    if(WEBSRV_onCommand) WEBSRV_onCommand(URLdecode(http_cmd), URLdecode(http_rqfile), URLdecode(http_arg));
                }
                else {   // An empty "GET"?
                    if(WEBSRV_onInfo) WEBSRV_onInfo("Filename is: index.html");
                    if(WEBSRV_onCommand) WEBSRV_onCommand("index.html", URLdecode(http_param), URLdecode(http_arg));
                }
                wswitch=false; // use second while
                break;
            } else {
                // Newline seen
                if (currentLine.startsWith("GET /")) {     // GET request?
                    method = HTTP_GET;
                    request    = currentLine.substring(5);
                    int32_t posHTTP = request.indexOf(" HTTP/");
                    if(posHTTP >= 0) request = request.substring(0, posHTTP);
                    request.trim();
                    continue;
                }
                else if (currentLine.startsWith("POST /")){ // POST request?
                    method = HTTP_POST;
                    request = currentLine.substring(6);
                    int32_t posHTTP = request.indexOf(" HTTP/");
                    if(posHTTP >= 0) request = request.substring(0, posHTTP);
                    request.trim();
                    continue;
                }

                if(method > 0){
                    currentLine = URLdecode(currentLine);
                //    log_w("%s",currentLine.c_str());

                    if(currentLine.startsWith("Content-Type:")) {ct = currentLine.substring(13); ct.trim();}
                    if(currentLine.startsWith("X-Filename:")){xFilename = currentLine.substring(11); xFilename.trim();}
                    if(currentLine.startsWith("Content-Length:")) contentLength = currentLine.substring(15).toInt();
                }
            }
        }
    } //end first while
    while(wswitch==false){                          // second while
        if(cmdclient.available()) {
            //log_i("%i", cmdclient.available());
            currentLine = cmdclient.readStringUntil('\n');
            int32_t idx = currentLine.indexOf("\r");
            if(idx > 0) currentLine[idx] = ' ';
        //    log_w("currLine %s", currentLine.c_str());
        }
        else{
            currentLine = "";
        }
        if(!currentLine.length()){
            return true;
        }
        if((currentLine.length() == 1 && count == 0) || count >= 2){
            wswitch=true;  // use first while
            currentLine = "";
            count = 0;
            break;
        }

        else{   // its the requestbody
            if(method == HTTP_GET){
                if(currentLine.length() > 1){
                    if(WEBSRV_onRequest) WEBSRV_onRequest(currentLine, 0);
                    if(WEBSRV_onInfo) WEBSRV_onInfo(currentLine.c_str());
                }

                if(currentLine.startsWith("------")) {
                    count++; // WebKitFormBoundary header
                    contentLength -= (currentLine.length() + 2); // WebKitFormBoundary footer is 2 chars longer
                }
                if(currentLine.length() == 1 && count == 1){
                    contentLength -= 7; // "\r\n\r\n..."
                    if(WEBSRV_onRequest) WEBSRV_onRequest("fileUpload", contentLength);
                    count++;
                }
            }
            if(method == HTTP_POST){
                if(WEBSRV_onRequest) WEBSRV_onRequest("SD/" + currentLine, contentLength);
            }
        }

    } // end second while
    return true;
}
//--------------------------------------------------------------------------------------------------------------
boolean WebSrv::handleWS() {                  // Websocketserver, receive messages
    String currentLine = "";                // Build up to complete line

    if (!webSocketClient.connected()){
        log_e("webSocketClient should be connected but is not!");
        hasclient_WS = false;
        return false;
    }

    if(!hasclient_WS){
        while(true){
            currentLine = webSocketClient.readStringUntil('\n');

            if (currentLine.length() == 1) { // contains '\n' only
                if(ws_conn_request_flag){
                    ws_conn_request_flag = false;
                    printWebSocketHeader(WS_resp_Key);
                    hasclient_WS = true;
                }
                break;
            }

            if (currentLine.startsWith("Sec-WebSocket-Key:")) { // Websocket connection request
                WS_sec_Key = currentLine.substring(18);
                WS_sec_Key.trim();
                WS_resp_Key = calculateWebSocketResponseKey(WS_sec_Key);
                ws_conn_request_flag = true;
            }
        }
    }
    int32_t av = webSocketClient.available();

    if(av){
        parseWsMessage(av);
    }
    return true;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::parseWsMessage(uint32_t len){
    uint8_t  headerLen = 2;
    uint16_t paylodLen;
    uint8_t  maskingKey[4];

    if(len > UINT16_MAX){
        log_e("Websocketmessage too long");
        return;
    }

    webSocketClient.readBytes(buff, 1);
    uint8_t fin       = ((buff[0] >> 7) & 0x01); (void)fin;
    uint8_t rsv1      = ((buff[0] >> 6) & 0x01); (void)rsv1;
    uint8_t rsv2      = ((buff[0] >> 5) & 0x01); (void)rsv2;
    uint8_t rsv3      = ((buff[0] >> 4) & 0x01); (void)rsv3;
    uint8_t opcode    = ( buff[0]  & 0x0F);

    webSocketClient.readBytes(buff, 1);
    uint8_t mask      = ((buff[0]>>7) & 0x01);
    paylodLen = (buff[0] & 0x7F);

    if(paylodLen == 126){
        headerLen = 4;
        webSocketClient.readBytes(buff, 2);
        paylodLen  = buff[0] << 8;
        paylodLen += buff[1];

    }

    uint16_t msgBuffPtr = 0;
    msgBuff = x_ps_malloc(paylodLen + 1);
    if(!msgBuff){log_e("oom"); goto exit;}

    (void)headerLen;

    if(mask){
        maskingKey[0] = webSocketClient.read();
        maskingKey[1] = webSocketClient.read();
        maskingKey[2] = webSocketClient.read();
        maskingKey[3] = webSocketClient.read();
    }

    if(opcode == 0x08) {  // denotes a connection close
        hasclient_WS = false;
        webSocketClient.stop();
        goto exit;
    }

    if(opcode == 0x09) {  // denotes a ping
        if(WEBSRV_onCommand) WEBSRV_onCommand("ping received, send pong", "", "");
        if(WEBSRV_onInfo) WEBSRV_onInfo("pong received, send pong");
        sendPong();
    }

    if(opcode == 0x0A) {  // denotes a pong
        if(WEBSRV_onCommand) WEBSRV_onCommand("pong received", "", "");
        if(WEBSRV_onInfo) WEBSRV_onInfo("pong received");
        goto exit;
    }

    if(opcode == 0x01) { // denotes a text frame
        int32_t plen = 0;
        uint32_t pll = paylodLen;
        while(paylodLen){
            if(paylodLen > 255){
                plen = 255;
                plen = webSocketClient.readBytes(msgBuff + msgBuffPtr, plen);
                paylodLen -= plen;
                msgBuffPtr += plen;
            }
            else{
                plen = paylodLen;
                paylodLen = 0;
                webSocketClient.readBytes(msgBuff + msgBuffPtr, plen);
                msgBuffPtr += plen;
            }
        }
        if(mask){
            for(int32_t i = 0; i < pll; i++){
                msgBuff[i] = (msgBuff[i] ^ maskingKey[i % 4]);
            }
        }
        msgBuff[pll] = 0;
        if(WEBSRV_onInfo) WEBSRV_onInfo(msgBuff);
        const char* cmd = msgBuff;
        const char* param = NULL;
        const char* arg = NULL;
        int32_t idx1 = indexOf(msgBuff, '=');
        if(idx1 > 0){
            msgBuff[idx1] = '\0';
            const char* cmd = msgBuff;
            int32_t offset = idx1 + 1;
            int32_t idx2 = lastIndexOf(msgBuff + offset, '&');
            if (idx2 > 0){
                *(msgBuff + offset + idx2) = '\0';
                param = msgBuff + offset;
                arg = msgBuff + offset + idx2 + 1;
                if(WEBSRV_onCommand) WEBSRV_onCommand(cmd, param, arg);
                goto exit;
            }
            else{
                param = msgBuff + offset;
                if(WEBSRV_onCommand) WEBSRV_onCommand(cmd, param, "");
                goto exit;
            }
        }
        else{
            if(WEBSRV_onCommand) WEBSRV_onCommand(cmd, "", "");
            goto exit;
        }
        if(WEBSRV_onCommand) WEBSRV_onCommand((const char*) msgBuff, "", "");
    }
    else{
        log_e("opcode != 0x01");
    }
exit:
    if(msgBuff){free(msgBuff); msgBuff = NULL;}
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::loop() {

    cmdclient = cmdserver.accept();
    if(cmdclient){
        handlehttp();
    }

    if (webSocketClient.available()){
        if(WEBSRV_onInfo) WEBSRV_onInfo("WebSocket client available");
        handleWS();
    }

    if(!webSocketClient.connected()){
        hasclient_WS = false;
    }
    if(!hasclient_WS) webSocketClient = webSocketServer.accept();

    return;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::reply(const String response, const char* MIMEType, boolean header){
    if(header==true) {
        int32_t l= response.length();
        // HTTP header
        String httpheader="";
        httpheader += "HTTP/1.1 200 OK\r\n";
        httpheader += "Content-type: " + (String)MIMEType + "\r\n";
        httpheader += "Content-Length: " + String(l, 10) + "\r\n\r\n";

        cmdclient.print(httpheader) ;             // header sent
    }
    cmdclient.print(response);
}
void WebSrv::sendStatus(uint16_t HTTPstatusCode){
    int32_t l= 0; // respunse length
    // HTTP header
    String httpheader="";
    httpheader += "HTTP/1.1 " +  String(HTTPstatusCode, 10) + "\r\n";
    httpheader += "Connection: close\r\n";
    httpheader += "Content-type: text/html\r\n";
    httpheader += "Content-Length: " + String(l, 10) + "\r\n";
    httpheader += "Server: " + _Name+ "\r\n";
    httpheader += "Cache-Control: max-age=3600\r\n";
    httpheader += "Last-Modified: " + _Version + "\r\n\r\n";
    cmdclient.print(httpheader) ;             // header sent
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::UTF8toASCII(String str){
    uint16_t i=0;
    String res="";
    char tab[96]={
          96,173,155,156, 32,157, 32, 32, 32, 32,166,174,170, 32, 32, 32,248,241,253, 32,
          32,230, 32,250, 32, 32,167,175,172,171, 32,168, 32, 32, 32, 32,142,143,146,128,
          32,144, 32, 32, 32, 32, 32, 32, 32,165, 32, 32, 32, 32,153, 32, 32, 32, 32, 32,
         154, 32, 32,225,133,160,131, 32,132,134,145,135,138,130,136,137,141,161,140,139,
          32,164,149,162,147, 32,148,246, 32,151,163,150,129, 32, 32,152
    };
    while(str[i]!=0){
        if(str[i]==0xC2){ // compute unicode from utf8
            i++;
            if((str[i]>159)&&(str[i]<192)) res+=char(tab[str[i]-160]);
            else res+=char(32);
        }
        else if(str[i]==0xC3){
            i++;
            if((str[i]>127)&&(str[i]<192)) res+=char(tab[str[i]-96]);
            else res+=char(32);
        }
        else res+=str[i];
        i++;
    }
    return res;
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::URLdecode(String str){
    String hex="0123456789ABCDEF";
    String res="";
    uint16_t i=0;
    while(str[i]!=0){
        if((str[i]=='%') && isHexadecimalDigit(str[i+1]) && isHexadecimalDigit(str[i+2])){
            res+=char((hex.indexOf(str[i+1])<<4) + hex.indexOf(str[i+2])); i+=3;}
        else{res+=str[i]; i++;}
    }
    return res;
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::responseCodeToString(int32_t code) {
  switch (code) {
    case 100: return F("Continue");
    case 101: return F("Switching Protocols");
    case 200: return F("OK");
    case 201: return F("Created");
    case 202: return F("Accepted");
    case 203: return F("Non-Authoritative Information");
    case 204: return F("No Content");
    case 205: return F("Reset Content");
    case 206: return F("Partial Content");
    case 300: return F("Multiple Choices");
    case 301: return F("Moved Permanently");
    case 302: return F("Found");
    case 303: return F("See Other");
    case 304: return F("Not Modified");
    case 305: return F("Use Proxy");
    case 307: return F("Temporary Redirect");
    case 400: return F("Bad Request");
    case 401: return F("Unauthorized");
    case 402: return F("Payment Required");
    case 403: return F("Forbidden");
    case 404: return F("Not Found");
    case 405: return F("Method Not Allowed");
    case 406: return F("Not Acceptable");
    case 407: return F("Proxy Authentication Required");
    case 408: return F("Request Time-out");
    case 409: return F("Conflict");
    case 410: return F("Gone");
    case 411: return F("Length Required");
    case 412: return F("Precondition Failed");
    case 413: return F("Request Entity Too Large");
    case 414: return F("Request-URI Too Large");
    case 415: return F("Unsupported Media Type");
    case 416: return F("Requested range not satisfiable");
    case 417: return F("Expectation Failed");
    case 500: return F("Internal Server Error");
    case 501: return F("Not Implemented");
    case 502: return F("Bad Gateway");
    case 503: return F("Service Unavailable");
    case 504: return F("Gateway Time-out");
    case 505: return F("HTTP Version not supported");
    default:  return "";
  }
}
//--------------------------------------------------------------------------------------------------------------
