/*
 * html.cpp
 *
 *  Created on: 09.07.2017
 *  updated on: 18.07.2021
 *      Author: Wolle
 */

#include "html.h"

//--------------------------------------------------------------------------------------------------------------
HTML::HTML(String Name, String Version){
    _Name=Name; _Version=Version;
    method = HTTP_NONE;
}
//--------------------------------------------------------------------------------------------------------------
void HTML::show_not_found(){
    cmdclient.print("HTTP/1.1 404 Not Found\n\n");
    return;
}
//--------------------------------------------------------------------------------------------------------------
void HTML::show(const char* pagename, int16_t len){
    uint TCPCHUNKSIZE = 1024;   // Max number of bytes per write
    size_t pagelen=0, res=0;                    // Size of requested page
    const unsigned char* p;
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
    httpheader += "Content-type: text/html\r\n";
    httpheader += "Content-Length: " + String(pagelen, 10) + "\r\n";
    httpheader += "Server: " + _Name+ "\r\n";
    httpheader += "Cache-Control: max-age=3600\r\n";
    httpheader += "Last-Modified: " + _Version + "\r\n\r\n";

    cmdclient.print(httpheader) ;             // header sent



    if (HTML_info)  HTML_info(String("Length of page is ") +String(pagelen, 10));
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
boolean HTML::streamfile(fs::FS &fs,const char* path){ // transfer file from SD to webbrowser
    size_t bytesPerTransaction = 1024;
    uint8_t transBuf[bytesPerTransaction], i=0;
    size_t wIndex = 0, res=0, leftover=0;
    if(!cmdclient.connected()){log_e("not connected"); return false;}
    while(path[i]!=0){     // protect SD for invalid signs to avoid a crash!!
        if(path[i]<32)return false;
        i++;
    }

    File file = fs.open(path, "r");
    if(!file){
        String str=String("Failed to open file for reading ") + path;
        if (HTML_info)  HTML_info(str);
        show_not_found();
        return false;
    }
    String LOF="Length of file " + String(path) + " is " + String(file.size(),10);
    if (HTML_info)  HTML_info(LOF);

    // HTTP header
    String httpheader="";
    httpheader += "HTTP/1.1 200 OK\r\n";
    httpheader += "Connection: close\r\n";
    httpheader += "Content-type: " + getContentType(String(path)) +"\r\n";
    httpheader += "Content-Length: " + String(file.size(),10) + "\r\n";
    httpheader += "Server: " + _Name+ "\r\n";
    httpheader += "Cache-Control: max-age=3600\r\n";
    httpheader += "Last-Modified: " + _Version + "\r\n\r\n";

    cmdclient.print(httpheader) ;             // header sent

    while(wIndex+bytesPerTransaction < file.size()){
        file.read(transBuf, bytesPerTransaction);
        res=cmdclient.write(transBuf, bytesPerTransaction);
        wIndex+=res;
        if(res!=bytesPerTransaction){
            log_i("write error %s", path);
            cmdclient.clearWriteError();
            return false;
        }
    }
    leftover=file.size()-wIndex;
    file.read(transBuf, leftover);
    res=cmdclient.write(transBuf, leftover);
    wIndex+=res;
    if(res!=leftover){
        log_i("write error %s", path);
        cmdclient.clearWriteError();
        return false;
    }
    if(wIndex!=file.size()) log_e("file %s not correct sent", path);
    file.close();
    return true;
}
//--------------------------------------------------------------------------------------------------------------
boolean HTML::uploadB64image(fs::FS &fs,const char* path, uint32_t contentLength){ // transfer imagefile from webbrowser to SD
    size_t   bytesPerTransaction = 1024;
    uint8_t  tBuf[bytesPerTransaction];
    uint16_t av, i, j;
    uint32_t len = contentLength;
    boolean f_werror=false;
    String str="";
    int n=0;
    File file;
    fs.remove(path); // Remove a previous version, otherwise data is appended the file again
    file = fs.open(path, FILE_WRITE);  // Open the file for writing (create it, if doesn't exist)

    log_i("ContentLength %i", contentLength);
    str = str + cmdclient.readStringUntil(','); // data:image/jpeg;base64,
    len -= str.length();
    while(cmdclient.available()){
        av=cmdclient.available();
        if(av==0) break;
        if(av>bytesPerTransaction) av=bytesPerTransaction;
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
        str=String("File: ") + String(path) + "write error";
        if (HTML_info) HTML_info(str.c_str());
        return false;
    }
    str=String("File: ") + String(path) + " written,  FileSize: " +  String(contentLength, 10);
    log_i("str =%s", str.c_str());
    if (HTML_info) HTML_info(str.c_str());
    return true;
}
//--------------------------------------------------------------------------------------------------------------
boolean HTML::uploadfile(fs::FS &fs,const char* path, uint32_t contentLength){ // transfer file from webbrowser to sd
    size_t   bytesPerTransaction = 1024;
    uint8_t  tBuf[bytesPerTransaction];
    uint16_t av;
    uint32_t len = contentLength;
    boolean f_werror=false;
    String str="";
    File file;
    fs.remove(path); // Remove a previous version, otherwise data is appended the file again
    file = fs.open(path, FILE_WRITE);  // Open the file for writing in SD (create it, if doesn't exist)

    while(cmdclient.available()){
        av=cmdclient.available();
        if(av>bytesPerTransaction) av=bytesPerTransaction;
        if(av>len) av=len;
        len -= av;
        cmdclient.read(tBuf, av);
        if(file.write(tBuf, av)!=av) f_werror=true;  // write error?
        if(len == 0) break;
    }
    cmdclient.readStringUntil('\n'); // read the remains, first \n
    cmdclient.readStringUntil('\n'); // read the remains  webkit\n
    file.close();
    if(f_werror) {
        str=String("File: ") + String(path) + "write error";
        if (HTML_info) HTML_info(str.c_str());
        return false;
    }
    str=String("File: ") + String(path) + " written,  FileSize: " +  String(contentLength, 10);
    if (HTML_info)  HTML_info(str.c_str());
    return true;
}
//--------------------------------------------------------------------------------------------------------------
void HTML::begin() {
    cmdserver.begin();
}
//--------------------------------------------------------------------------------------------------------------
void HTML::stop() {
    cmdclient.stop();
}
//--------------------------------------------------------------------------------------------------------------
String HTML::getContentType(String filename){
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
    return "text/plain" ;
}
//--------------------------------------------------------------------------------------------------------------
boolean HTML::handlehttp() {
    bool wswitch=true;
    char c;                                 // Next character from http input
    int16_t inx0, inx1, inx2, inx3;         // Pos. of search string in currenLine
    String currentLine = "";                // Build up to complete line
    String ct;                              // contentType
    uint32_t cl = 0;                        // contentLength
    uint8_t count = 0;

    if (!cmdclient.connected()){
        log_e("cmdclient schould be connected but is not!");
        return false;
    }
    while (wswitch==true){                  // first while
        if(!cmdclient.available()){
            log_e("Command client schould be available but is not!");
            return false;
        }
        currentLine = cmdclient.readStringUntil('\n');
        // If the current line is blank, you got two newline characters in a row.
        // that's the end of the client HTTP request, so send a response:
        if (currentLine.length() == 1) { // contains '\n' only
            wswitch=false; // use second while
            if (http_cmd.length()) {
                if (HTML_info) HTML_info(URLdecode(http_cmd));
                if (HTML_command) HTML_command(URLdecode(http_cmd), URLdecode(http_param), URLdecode(http_arg));
            }
            if (http_rqfile.length()) {
                String FN = "Filename is: " + http_rqfile;
                if (HTML_info) HTML_info(URLdecode(FN));
                if (HTML_file) HTML_file(URLdecode(http_rqfile));
            }
            if(http_rqfile.length() == 0 && http_cmd.length() == 0 ){   // An empty "GET"?
                if (HTML_info) HTML_info("Filename is: index.html");
                if (HTML_file) HTML_file("index.html");
            }
            currentLine = "";
            http_cmd    = "";
            http_param  = "";
            http_arg    = "";
            http_rqfile = "";
            method = HTTP_NONE;
            break;
        } else {
            // Newline seen
            inx0 = 0;
            if (currentLine.startsWith("Content-Length:")) cl = currentLine.substring(15).toInt();
            if (currentLine.startsWith("GET /")) {method = HTTP_GET; inx0 = 5;}  // GET request?
            if (currentLine.startsWith("POST /")){method = HTTP_PUT; inx0 = 6;}  // POST request?
            if (inx0 == 0) method = HTTP_NONE;

            if(inx0>0){
                inx1 = currentLine.indexOf("?");    // Search for 1st parameter
                inx2 = currentLine.indexOf("&");    // Search for 2nd parameter
                inx3 = currentLine.indexOf(" HTTP");// Search for 3th parameter

                if(inx1 > inx0){     // it is a command
                    http_cmd = currentLine.substring(inx0, inx1);//isolate the command
                    http_rqfile = "";               // No file
                }
                if((inx1>0) && (inx1+1 < inx3)){        // it is a parameter
                    http_param = currentLine.substring(inx1+1, inx3);//isolate the parameter

                    if(inx2>0){
                        http_arg = currentLine.substring(inx2+1, inx3);//isolate the arguments
                        http_param = currentLine.substring(inx1+1, inx2);//cut the parameter
                    }
                    http_rqfile = "";               // No file
                }
                if(inx1 < 0 && inx2 < 0){   // it is a filename
                    http_rqfile = currentLine.substring(inx0, inx3);
                    http_cmd =   "";
                    http_param = "";
                    http_arg =   "";
                }
            }
            currentLine = "";
        }
    } //end first while
    while(wswitch==false){                          // second while
        if(cmdclient.available()) {
            currentLine = cmdclient.readStringUntil('\n');
            cl -= currentLine.length();
        }
        else{
            currentLine = "";
        }
        if(!currentLine.length()) return true;
        if ((currentLine.length() == 1 && count == 0) || count >= 2){
            wswitch=true;  // use first while
            currentLine = "";
            count = 0;
            break;
        }
        else {   // its the requestbody
            if(currentLine.length() > 1){
                if (HTML_request) HTML_request(currentLine, 0);
                if (HTML_info) HTML_info(currentLine);
            }

            if(currentLine.startsWith("------")) {
                count++; // WebKitFormBoundary header
                cl -= (currentLine.length() + 2); // WebKitFormBoundary footer ist 2 chars longer
            }
            if(currentLine.length() == 1 && count == 1){
                cl -= 6; // "\r\n\r\n..."
                if (HTML_request) HTML_request("fileUpload", cl);
                count++;
            }
        }

    } // end second while
    return true;
}
//--------------------------------------------------------------------------------------------------------------
boolean HTML::loop() {
    cmdclient = cmdserver.available();                  // Check Input from client?
    if (cmdclient.available()){                                      // Client connected?
        if(HTML_info) HTML_info("Command client available");
        return handlehttp();
    }
    return false;
}
//--------------------------------------------------------------------------------------------------------------
void HTML::reply(const String &response, bool header){
    if(header==true) {
        int l= response.length();
        // HTTP header
        String httpheader="";
        httpheader += "HTTP/1.1 200 OK\r\n";
        httpheader += "Connection: close\r\n";
        httpheader += "Content-type: text/html\r\n";
        httpheader += "Content-Length: " + String(l, 10) + "\r\n";
        httpheader += "Server: " + _Name+ "\r\n";
        httpheader += "Cache-Control: max-age=3600\r\n";
        httpheader += "Last-Modified: " + _Version + "\r\n\r\n";

        cmdclient.print(httpheader) ;             // header sent
    }
    cmdclient.print(response);
}
//--------------------------------------------------------------------------------------------------------------
String HTML::UTF8toASCII(String str){
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
String HTML::URLdecode(String str){
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
String HTML::responseCodeToString(int code) {
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
