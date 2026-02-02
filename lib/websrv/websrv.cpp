/*
 * websrv.cpp
 *
 *  Created on: 09.07.2017
 *  updated on: 10.12.2025
 *      Author: Wolle
 */

#include "websrv.h"
#include "esp_memory_utils.h"
//--------------------------------------------------------------------------------------------------------------
WebSrv::WebSrv(String Name, String Version) {
    _Name = Name;
    _Version = Version;
}
//--------------------------------------------------------------------------------------------------------------
WebSrv::~WebSrv() {
    ;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::show_not_found() {
    cmdclient.print("HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "404 Not Found");
    return;
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::calculateWebSocketResponseKey(String sec_WS_key) {
    // input  Sec-WebSocket-Key from client
    // output Sec-WebSocket-Accept-Key (used in response message to client)
    uint8_t sha1_result[20];
    String  concat = sec_WS_key + WS_sec_conKey;
    mbedtls_sha1((unsigned char*)concat.c_str(), concat.length(), (unsigned char*)sha1_result);
    return base64::encode(sha1_result, 20);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::printWebSocketHeader(String wsRespKey) {
    String wsHeader = (String) "HTTP/1.1 101 Switching Protocols\r\n" + "Upgrade: websocket\r\n" + "Connection: Upgrade\r\n" + "Sec-WebSocket-Accept: " + wsRespKey + "\r\n" +
                      "Access-Control-Allow-Origin: \r\n\r\n";
    // "Sec-WebSocket-Protocol: chat\r\n\r\n";
    // log_i("wsheader %s", wsHeader.c_str());
    webSocketClient.print(wsHeader); // header sent
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::show(const char* pagename, const char* MIMEType, int16_t len) {
    constexpr size_t TCP_CHUNK_SIZE = 4096; // optimal für WiFi/Ethernet
    size_t pagelen = 0;

    // --- Check whether source comes from PROGMEM ---
    bool isProgmem = !esp_ptr_in_dram(pagename);

    // --- Seitengröße bestimmen ---
    if (len < 0)
        pagelen = isProgmem ? strlen_P(pagename) : strlen(pagename);
    else if (len > 0)
        pagelen = len;

  // --- Skip leading newlines ---
    while (pagelen && (isProgmem ? pgm_read_byte(pagename) : *pagename) == '\n') {
        ++pagename;
        --pagelen;
    }

    // --- HTTP Header ---
    String header;
    header.reserve(160);
    header += F("HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Cache-Control: max-age=86400\r\n");
    header += F("Content-Type: ");
    header += MIMEType;
    header += F("\r\nContent-Length: ");
    header += String(pagelen);
    header += F("\r\nServer: ");
    header += _Name;
    header += F("\r\nLast-Modified: ");
    header += _Version;
    header += F("\r\n\r\n");

    cmdclient.print(header);

    if (m_websrv_callback) {
        m_msg.e = evt_info;
        m_msg.arg.assignf("%s %s %u",  isProgmem ? "PROGMEM" : "RAM", ", page length:", pagelen);
        m_websrv_callback(m_msg);
    }

    // --- Main transmission ---
    size_t sent = 0;
    while (sent < pagelen) {
        size_t chunk = std::min(TCP_CHUNK_SIZE, pagelen - sent);

        size_t res = isProgmem
                     ? cmdclient.write_P(pagename + sent, chunk)
                     : cmdclient.write((const uint8_t*)pagename + sent, chunk);

        if (res != chunk) {
            m_msg.e = evt_error;
            m_msg.arg = "write error in webpage";
            if (m_websrv_callback) m_websrv_callback(m_msg);
            cmdclient.clearWriteError();
            return;
        }
        sent += chunk;
    }

    cmdclient.clear();
}
//--------------------------------------------------------------------------------------------------------------
bool WebSrv::streamfile(fs::FS& fs, ps_ptr<char> path) { // transfer file from SD to webbrowser

    if (path.strlen() == 0) {
        m_msg.e = evt_error;
        m_msg.arg.assignf(ANSI_ESC_RED "SD path is null");
        if (m_websrv_callback) m_websrv_callback(m_msg);
        return false;
    } // guard
    if (path.strlen() > 1024) {
        m_msg.e = evt_info;
        m_msg.arg.assignf(ANSI_ESC_RED "SD path is too long %i bytes", path.strlen());
        if (m_websrv_callback) m_websrv_callback(m_msg);
        return false;
    } // guard
    for (int i = 0; path[i] != '\0'; ++i) {  // Validate path for illegal characters
        if (path[i] < 32) {
            m_msg.e = evt_info;
            m_msg.arg.assignf(ANSI_ESC_RED "Illegal character in path");
            if (m_websrv_callback) m_websrv_callback(m_msg);
            return false;
        }
    } // guard
    if (!fs.exists(path.c_get())) {
        if(path.ends_with(".jpg")){
            return false;
        }
        show_not_found();
        return false;
    } // guard

    ps_ptr<char> c_path;
    c_path.copy_from(path.c_get());
    c_path.truncate_at('?'); // Remove query string

    File file = fs.open(path.c_get(), "r");
    if (!file) {
        m_msg.e = evt_info;
        m_msg.arg.assignf("Failed to open file for reading: %s", c_path.c_get());
        if (m_websrv_callback) m_websrv_callback(m_msg);
        show_not_found();
        return false;
    }

    m_msg.e = evt_info;
    m_msg.arg.assignf("Length of file %s is %d", c_path.c_get(), file.size());
    if (m_websrv_callback) m_websrv_callback(m_msg);

    // HTTP header
    ps_ptr<char> httpheader;
    httpheader.assign("HTTP/1.1 200 OK\r\n");
    httpheader.append("Connection: keep-alive\r\n");
    httpheader.append("Content-type: ");
    httpheader.append(getContentType(c_path));
    httpheader.append("\r\n");
    httpheader.appendf("Content-Length: %i\r\n", file.size());
    httpheader.appendf("Cache-Control: public, max-age=86400\r\n\r\n");

    cmdclient.print(httpheader.c_get()); // header sent
    // log_i("%s", httpheader.c_get());

    size_t          bytesTransmitted = 0, bytesInBuff = 0, bytesToSend = file.size();
    ps_ptr<uint8_t> transBuff;
    transBuff.alloc(INT16_MAX);

    while (bytesTransmitted < file.size()) {
        bytesInBuff = file.read(transBuff.get(), INT16_MAX);
        int16_t bytesWritten = 0, buffPtr = 0;
        while (bytesWritten < bytesInBuff) {
            bytesWritten = cmdclient.write(transBuff.get() + buffPtr, bytesInBuff);
            if (bytesWritten <= 0) {
                goto error; // reset by peer while sending
            }
            bytesTransmitted += bytesWritten;
            buffPtr += bytesWritten;
            bytesToSend -= bytesWritten;
            bytesInBuff -= bytesWritten;
            vTaskDelay(1);
        }
    }

    file.close();
    return true;

error:
    file.close();
    return false;
}
//--------------------------------------------------------------------------------------------------------------
bool WebSrv::send(const char* cmd, int msg, uint8_t opcode) { // sends text messages via websocket
    char nr_txt[10];
    itoa(msg, nr_txt, 10);
    return send(cmd, nr_txt, opcode);
}
bool WebSrv::send(ps_ptr<char> cmd, ps_ptr<char> msg, uint8_t opcode) { // sends text messages via websocket
    uint8_t headerLen = 2;

    if (!hasclient_WS) {
        //      log_e("can't send, websocketserver not connected");
        return false;
    }
    size_t cmdLen = cmd.strlen();
    size_t msgLen = msg.strlen();

    std::string sanitized;

    if (opcode == 1) {
        // Nur prüfen/reinigen bei Textnachrichten
        std::string combined = std::string(cmd.c_get(), cmdLen) + std::string(msg.c_get(), msgLen);
        sanitized = sanitize_utf8_replace(combined.c_str(), combined.size());
        cmdLen = sanitized.size();
        msgLen = 0;  // alles in sanitized
    }

    if (cmdLen + msgLen > UINT16_MAX) {
        log_e("send: message too long, greater than 64kB");
        return false;
    }

    uint8_t fin = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;
    char    header[4] = {0};

    header[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + opcode;
    if (msgLen + cmdLen < 126) {
        header[1] = (128 * mask) + cmdLen + msgLen;
    } else {
        headerLen = 4;
        header[1] = (128 * mask) + 126;
        header[2] = ((cmdLen + msgLen) >> 8) & 0xFF;
        header[3] = (cmdLen + msgLen) & 0xFF;
    }

    webSocketClient.write(header, headerLen);
    if (opcode == 1)
        webSocketClient.write(sanitized.c_str(), sanitized.size());
    else {
        webSocketClient.write(cmd.c_get(), cmdLen);
        webSocketClient.write(msg.c_get(), msgLen);
    }

    return true;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::sendPing() { // heartbeat, keep alive via websockets

    if (!hasclient_WS) { return; }
    uint8_t fin = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;
    char    ping[2] = {0};

    ping[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + Ping_Frame;
    ping[1] = (128 * mask) + 0;
    webSocketClient.write(ping, 2);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::sendPong() { // heartbeat, keep alive via websockets

    if (!hasclient_WS) { return; }
    uint8_t fin = 1;
    uint8_t rsv1 = 0;
    uint8_t rsv2 = 0;
    uint8_t rsv3 = 0;
    uint8_t mask = 0;
    char    pong[2] = {0};

    pong[0] = (128 * fin) + (64 * rsv1) + (32 * rsv2) + (16 * rsv3) + Pong_Frame;
    pong[1] = (128 * mask) + 0;
    webSocketClient.write(pong, 2);
}
//--------------------------------------------------------------------------------------------------------------
// e.g.  startBoundary        ------WebKitFormBoundaryi52Pv7aBYloXIuZB\r\n
//                            Content-Disposition: form-data; name="hidden_data"\r\n\r\n
//                            data:image/jpeg;base64,
//
//       endBoundary          ------WebKitFormBoundaryi52Pv7aBYloXIuZB--
//
bool WebSrv::uploadB64image(fs::FS& fs, const char* path, uint32_t contentLength) {
    File           file;
    uint32_t       t = millis();
    const uint32_t TIMEOUT = 2000;
    int16_t        idx = 0, pos = 0, bytesRead = 0, startPos = 0, base64Start = 0;
    int32_t        base64Length = 0, endPos = 0;
    size_t         decodedLen = 0;
    int            ret = 0;
    const uint16_t bytesPerTransaction = 16384; // multiple of 4
    uint32_t       totalRead = 0;
    uint32_t       totalDecoded = 0;
    uint32_t       remaining = 0;
    uint32_t       chunkSize = 0;

    ps_ptr<char>    msg;
    ps_ptr<char>    data;
    ps_ptr<char>    b64buff;
    ps_ptr<uint8_t> decoded;
    ps_ptr<char>    boundary;
    ps_ptr<char>    boundaryString;
    ps_ptr<char>    endMarker;

    boundary.calloc(256);

    // === 1. Read start boundary until first ',' ===
    while (true) {
        if ((t + TIMEOUT) < millis()) {
            msg = "Timeout while reading startBoundary (uploadB64image)";
            goto exit;
        }
        if (cmdclient.available()) {
            boundary[idx] = cmdclient.read();
            if (boundary[idx] == ',') {
                boundary[idx + 1] = '\0';
                break;
            }
            idx++;
            if (idx == 255) {
                msg = "Buffer overflow (buff[256], uploadB64image)";
                goto exit;
            }
        }
    }

    pos = boundary.index_of("\r\n");
    if (pos < 0) {
        msg = "No CRLF found in boundary (uploadB64image)";
        goto exit;
    }
    boundaryString = boundary.substr(0, pos);
    // boundaryString.println();

    // === Check image header markers ===
    pos = boundary.index_of("\r\n\r\ndata:image/");
    if (pos < 0) {
        msg = "No 'data:image/' marker found";
        goto exit;
    }

    base64Start = boundary.index_of("base64,", startPos);
    if (base64Start < 0) {
        msg = "No 'base64,' marker found";
        goto exit;
    }
    base64Start += 7; // skip comma

    // === 2. Read rest of content ===
    data.calloc(contentLength - base64Start + 1);
    while (bytesRead < contentLength - base64Start && (t + TIMEOUT) > millis()) {
        if (cmdclient.available()) { data[bytesRead++] = cmdclient.read(); }
    }
    if (bytesRead < contentLength - base64Start) {
        msg = "Timeout while reading form data (uploadB64image)";
        goto exit;
    }

    // === 3. Find end boundary ===
    endMarker = "\r\n" + boundaryString + "--";
    endPos = data.index_of(endMarker.get());
    if (endPos < 0) {
        msg = "End boundary not found";
        goto exit;
    }

    // base64 data without end boundary
    base64Length = endPos;
    log_i("Base64 data length: %d", base64Length);

    // === 4. Basic checks ===
    if (base64Length % 4 != 0) {
        msg = "Base64 data length not divisible by 4 — possibly truncated";
        goto exit;
    }

    if (fs.exists(path)) fs.remove(path);
    file = fs.open(path, FILE_WRITE);
    if (!file) {
        msg = "Cannot open file for writing";
        goto exit;
    }

    // === 5. Streaming Base64 decode ===
    b64buff.alloc(bytesPerTransaction + 4);
    decoded.alloc((bytesPerTransaction * 3) / 4 + 4);

    totalRead = 0;
    totalDecoded = 0;
    while (totalRead < (uint32_t)base64Length) {
        remaining = base64Length - totalRead;
        chunkSize = (remaining > bytesPerTransaction) ? bytesPerTransaction : remaining;

        // make sure chunkSize is multiple of 4
        chunkSize &= ~0x03;

        // copy base64 chunk
        memcpy(b64buff.get(), data.get() + totalRead, chunkSize);
        totalRead += chunkSize;

        // === if this is the last chunk, check for end boundary ===
        if (remaining <= bytesPerTransaction) {
            int boundaryPos = b64buff.index_of(endMarker.get());
            if (boundaryPos >= 0) {
                chunkSize = boundaryPos;
                b64buff[chunkSize] = '\0';
            }
        }

        // decode chunk
        decodedLen = 0;
        ret = mbedtls_base64_decode(decoded.get(), decoded.size(), &decodedLen, (const unsigned char*)b64buff.get(), chunkSize);
        if (ret != 0) {
            msg.assignf("Base64 decode error at offset %u", totalRead);
            goto exit;
        }

        // write decoded data to SD
        size_t written = file.write(decoded.get(), decodedLen);
        if (written != decodedLen) {
            msg = "Error while writing on SD card";
            goto exit;
        }

        totalDecoded += decodedLen;
        log_i("Decoded %u / %u bytes so far", totalDecoded, decodedLen);
    }

    file.close();
    msg.assignf("File '%s' written successfully, size %lu bytes", path, (unsigned long)totalDecoded);
    m_msg.e = evt_info;
    m_msg.arg = msg;
    if (m_websrv_callback) m_websrv_callback(m_msg);
    return true;

exit:
    m_msg.e = evt_error;
    m_msg.arg = msg;
    if (m_websrv_callback) m_websrv_callback(m_msg);
    cmdclient.stop();
    return false;
}
//--------------------------------------------------------------------------------------------------------------
/* example:
   startBoundary    ------WebKitFormBoundary6R1gey0yfb0yh8Ih\r\n
                    Content-Disposition: form-data; name="audio"; filename="test.txt"\r\n
                    Content-Type: text/plain\r\n\r\n

    endBoundary     \r\n------WebKitFormBoundary6R1gey0yfb0yh8Ih--\r\n
*/
bool WebSrv::uploadfile(fs::FS& fs, ps_ptr<char> path, uint32_t contentLength, ps_ptr<char> contentType) {
    ps_ptr<char> msg;
    File file;
    uint32_t av;
    int32_t  bytesInTransBuf = 0;
    int32_t  startBoundaryEndPos = 0;
    int32_t  startBoundaryLength = 0;
    ps_ptr<char> transBuf;
    ps_ptr<char> startBoundary;
    uint32_t t = 0;
    // m_upload_items.reset();

    // check whether multipart/form-data
    bool multipart = contentType.starts_with("multipart/form-data");

    // delete old file if there is one
    if (fs.exists(path.c_get())) fs.remove(path.c_get());

    file = fs.open(path.c_get(), FILE_WRITE);
    if (!file) {
        WS_LOG_ERROR("cannot open file %s for writing", path.c_get());
        return false;
    }

    if (!multipart) {
        // === simple Upload (no boundaries, e.g. json, text, binary) ===
        uint8_t buffer[1024];
        uint32_t bytesRemaining = contentLength;
        uint32_t bytesRead = 0;
        uint32_t totalWritten = 0;
        uint32_t t = millis();

        while (bytesRemaining > 0) {
            if ((t + 2000) < millis()) {
                msg.assign("timeout in uploadfile() (simple mode)");
                goto exit;
            }

            uint32_t toRead = std::min<uint32_t>(sizeof(buffer), bytesRemaining);
            if (!cmdclient.available()) {
                vTaskDelay(10);
                continue;
            }

            bytesRead = cmdclient.readBytes(buffer, toRead);
            if (bytesRead == 0) {
                msg.assignf("read error in simple upload (%lu bytes left)", bytesRemaining);
                goto exit;
            }

            int written = file.write(buffer, bytesRead);
            if (written != bytesRead) {
                msg.assignf("write error, written %i/%i bytes", written, bytesRead);
                goto exit;
            }

            bytesRemaining -= bytesRead;
            totalWritten += written;
        }

        file.close();
        m_msg.e = evt_info;
        m_msg.arg.assignf("upload of %s successful (%lu bytes)", path.c_get(), totalWritten);
        if (m_websrv_callback) m_websrv_callback(m_msg);
        return true;
    }

    // === Multipart upload ===

    transBuf.alloc(256);

    t = millis();
    while (true) {
        if ((t + 2000) < millis()) {
            msg.assignf("timeout in webSrv uploadfile()");
            goto exit;
        }

        av = min3(cmdclient.available(), 256, contentLength);
        if (av < contentLength && av < 256) {
            vTaskDelay(10);
            continue;
        }

        bytesInTransBuf = cmdclient.readBytes(transBuf.get(), 256);
        if (bytesInTransBuf != av) {
            msg.assignf("read error in %s, available %lu bytes, read %li bytes\n", path, av, bytesInTransBuf);
            goto exit;
        }

        if (!transBuf.starts_with("------")) {
            msg.assignf("startBoundary not found");
            goto exit;
        }

        startBoundaryEndPos = indexOf(transBuf.get(), "\r\n\r\n") + 4;
        if (startBoundaryEndPos < 0) {
            msg.assignf("startBoundaryEndPos not found");
            goto exit;
        }

        startBoundaryLength = transBuf.index_of("\r\n");
        startBoundary.copy_from(transBuf.get(), startBoundaryLength);
        m_upload_items.endBoundary = "\r\n" + startBoundary + "--";
        m_upload_items.max_endBoundary_length = startBoundaryLength + 6;
        m_upload_items.uploadfile = file;
        m_upload_items.bytes_left = contentLength - startBoundaryEndPos;

        int written = file.write((uint8_t*)transBuf.get() + startBoundaryEndPos,
                                 bytesInTransBuf - startBoundaryEndPos);
        if (written > 0) m_upload_items.bytes_left -= written;
        m_handle_upload = true;
        break;
    }

    return true; // multipart upload in progress

exit:
    file.close();
    m_msg.e = evt_error;
    m_msg.arg = msg;
    if (m_websrv_callback) m_websrv_callback(m_msg);
    return false;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::handle_upload_file() {
    uint32_t     t = millis();
    uint32_t     bytesWritten = 0;
    uint32_t     bytesInTransBuf = 0;
    uint32_t     bytes_per_transaction = 4096;
    ps_ptr<char> transBuff;
    transBuff.alloc(bytes_per_transaction + m_upload_items.max_endBoundary_length);

    while (true) {
        //    WS_LOG_INFO("bytes_left %i", m_upload_items.bytes_left);
        if ((t + 2000) < millis()) {
            WS_LOG_ERROR("timeout while file upload");
            m_handle_upload = false;
            m_upload_items.uploadfile.close();
            return;
        }
        if (cmdclient.available()) {
            if (m_upload_items.bytes_left <= bytes_per_transaction + m_upload_items.max_endBoundary_length) { // last round
                bytesInTransBuf = cmdclient.readBytes((uint8_t*)transBuff.get(), m_upload_items.bytes_left);
                int idx = transBuff.special_index_of(m_upload_items.endBoundary.get(), bytesInTransBuf);
                if (idx < 0) {
                    WS_LOG_ERROR("endBoundary not found");
                    m_handle_upload = false;
                    m_upload_items.uploadfile.close();
                    return;
                } else { // remove endBoundary
                    m_upload_items.bytes_left -= (bytesInTransBuf - idx);
                    bytesInTransBuf = idx;
                    WS_LOG_DEBUG("endBoundary found at %i, bytes_left %i, bytesInTransBuf %i", idx, m_upload_items.bytes_left, bytesInTransBuf);
                }

            } else {
                bytesInTransBuf = cmdclient.readBytes((uint8_t*)transBuff.get(), bytes_per_transaction);
            }
            if (bytesInTransBuf > 0) {
                bytesWritten = m_upload_items.uploadfile.write((uint8_t*)transBuff.get(), bytesInTransBuf);
                if (bytesWritten != bytesInTransBuf) { WS_LOG_ERROR("write error, bytes %i, written %i", bytesInTransBuf, bytesWritten); }
                m_upload_items.bytes_left -= bytesWritten;
                WS_LOG_DEBUG("bytes_left %i, bytesWritten %i", m_upload_items.bytes_left, bytesWritten);
            }
            break;
        }
    }

    if (!m_upload_items.bytes_left) { // file successfully written
        m_handle_upload = false;
        m_msg.e = evt_info;
        m_msg.arg.assignf(ANSI_ESC_RESET "upload " ANSI_ESC_CYAN "%s" ANSI_ESC_RESET " was successful", m_upload_items.uploadfile.name());
        if (m_websrv_callback) m_websrv_callback(m_msg);
        m_upload_items.uploadfile.close();
    }

    return;
}

//--------------------------------------------------------------------------------------------------------------
void WebSrv::begin(uint16_t http_port, uint16_t websocket_port) {
    method = HTTP_NONE;
    cmdserver.stop();
    cmdserver.begin(http_port);
    webSocketServer.stop();
    webSocketServer.begin(websocket_port);
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::stop() {
    webSocketClient.stop();
}
//--------------------------------------------------------------------------------------------------------------
const char* WebSrv::getContentType(ps_ptr<char>& filename) {
    if (filename.ends_with(".html"))
        return "text/html";
    else if (filename.ends_with(".htm"))
        return "text/html";
    else if (filename.ends_with(".css"))
        return "text/css";
    else if (filename.ends_with(".txt"))
        return "text/plain";
    else if (filename.ends_with(".js"))
        return "application/javascript";
    else if (filename.ends_with(".json"))
        return "application/json";
    else if (filename.ends_with(".svg"))
        return "image/svg+xml";
    else if (filename.ends_with(".ttf"))
        return "application/x-font-ttf";
    else if (filename.ends_with(".otf"))
        return "application/x-font-opentype";
    else if (filename.ends_with(".xml"))
        return "text/xml";
    else if (filename.ends_with(".pdf"))
        return "application/pdf";
    else if (filename.ends_with(".png"))
        return "image/png";
    else if (filename.ends_with(".bmp"))
        return "image/bmp";
    else if (filename.ends_with(".gif"))
        return "image/gif";
    else if (filename.ends_with(".jpg"))
        return "image/jpeg";
    else if (filename.ends_with(".ico"))
        return "image/x-icon";
    else if (filename.ends_with(".css"))
        return "text/css";
    else if (filename.ends_with(".zip"))
        return "application/x-zip";
    else if (filename.ends_with(".gz"))
        return "application/x-gzip";
    else if (filename.ends_with(".xls"))
        return "application/msexcel";
    else if (filename.ends_with(".mp3"))
        return "audio/mpeg";
    else if (filename.ends_with(".csv"))
        return "text/csv";
    return "text/plain";
}
//--------------------------------------------------------------------------------------------------------------
bool WebSrv::handlehttp() { // HTTPserver, message received

    bool     method_GET = false;
    bool     method_POST = false;
    bool     method_DELETE = false;
    int16_t  posColon = 0;
    uint32_t ctime = millis();
    uint32_t timeout = 4500; // ms
    uint32_t contentLength = 0;
    char     rhl[1024] = {0}; // requestHeaderline
    char     http_cmd[1024] = {0};
    char     http_param[1024] = {0};
    char     http_arg[1024] = {0};
    char     contentType[50] = {0};

    static uint32_t stime;
    static bool     f_time = false;
    if (cmdclient.available() == 0) {
        if (!f_time) {
            stime = millis();
            f_time = true;
        }
        if ((millis() - stime) > timeout) {
            log_e("timeout");
            f_time = false;
            return false;
        }
    }
    f_time = false;

    while (true) { // outer while
        uint16_t pos = 0;
        if ((millis() - ctime) > timeout) {
            log_e("timeout");
            goto exit;
        }
        if (!cmdclient.available()) goto exit;

        while (cmdclient.available()) {
            uint8_t b = cmdclient.read();
            if (b == '\n') {
                if (!pos) { // empty line received, is the last line of this responseHeader
                    cmdClientAccept = false;
                    goto lastToDo;
                }
                break;
            }
            if (b == '\r') rhl[pos] = 0;
            if (b < 0x20) continue;
            rhl[pos] = b;
            pos++;
            if (pos == 1023) {
                pos = 510;
                continue;
            }
            if (pos == 1022) {
                rhl[pos] = '\0';
                cmdclient.stop();
                log_i("requestHeaderline overflow");
            }
        } // inner while

        if (!pos) {
            vTaskDelay(1);
            continue;
        }
        // log_w("rhl %s", rhl);

        posColon = indexOf(rhl, ":", 0); // lowercase all letters up to the colon
        if (posColon >= 0) {
            for (int i = 0; i < posColon; i++) { rhl[i] = toLowerCase(rhl[i]); }
        }

        if (startsWith(rhl, "HTTP/")) { // HTTP status error code
            char statusCode[5];
            statusCode[0] = rhl[9];
            statusCode[1] = rhl[10];
            statusCode[2] = rhl[11];
            statusCode[3] = '\0';
            int sc = atoi(statusCode);
            if (sc > 310) { // e.g. HTTP/1.1 301 Moved Permanently
                log_e("%s", rhl);
                goto exit;
            }
        } else if (startsWith(rhl, "content-type:")) { // content-type: text/html; charset=UTF-8
            // log_i("cT: %s", rhl);
            int idx = indexOf(rhl + 13, ";");
            if (idx > 0) rhl[13 + idx] = '\0';
            strlcpy(contentType, rhl + 13, sizeof(contentType));
            trim(contentType);
        } else if (startsWith(rhl, "GET /")) {
            method_GET = true;
            int pos_http = indexOf(rhl, "HTTP/", 0);
            int pos_question = indexOf(rhl, "?", 0);  // questionmark
            int pos_ampersand = indexOf(rhl, "&", 0); // ampersand
            if (pos_http == -1) {
                log_w("GET without HTTP?");
                goto exit;
            }

            // cmd between "GET /" and "?" or "HTTP"
            int start_part1 = 5; // after "GET /"
            int end_part1 = (pos_question != -1) ? pos_question : (pos_ampersand != -1) ? pos_ampersand : pos_http;
            strncpy(http_cmd, rhl + start_part1, end_part1 - start_part1);

            // param between "?" and "&" or HTTP, if "?" exists
            if (pos_question != -1) {
                int start_part2 = pos_question + 1;
                int end_part2 = (pos_ampersand != -1) ? pos_ampersand : pos_http;
                strncpy(http_param, rhl + start_part2, end_part2 - start_part2);
            }

            // arg between "&" and "HTTP" if "&" exists
            if (pos_ampersand != -1) {
                int start_part3 = pos_ampersand + 1;
                strncpy(http_arg, rhl + start_part3, pos_http - start_part3);
            }
        } else if (startsWith(rhl, "POST /")) {
            method_POST = true;
            int pos_http = indexOf(rhl, "HTTP/", 0);
            int pos_question = indexOf(rhl, "?", 0);  // questionmark
            int pos_ampersand = indexOf(rhl, "&", 0); // ampersand
            if (pos_http == -1) {
                log_w("GET without HTTP?");
                goto exit;
            }

            // cmd between "GET /" and "?" or "HTTP"
            int start_part1 = 6; // after "GET /"
            int end_part1 = (pos_question != -1) ? pos_question : pos_http;
            strncpy(http_cmd, rhl + start_part1, end_part1 - start_part1);

            // param between "?" and "&" or HTTP, if "?" exists
            if (pos_question != -1) {
                int start_part2 = pos_question + 1;
                int end_part2 = (pos_ampersand != -1) ? pos_ampersand : pos_http;
                strncpy(http_param, rhl + start_part2, end_part2 - start_part2);
            }

            // arg between "&" and "HTTP" if "&" exists
            if (pos_ampersand != -1) {
                int start_part3 = pos_ampersand + 1;
                strncpy(http_arg, rhl + start_part3, pos_http - start_part3);
            }
        } else if (startsWith(rhl, "DELETE /")) {
            method_DELETE = true;
            int pos_http = indexOf(rhl, "HTTP/", 0);
            int pos_question = indexOf(rhl, "?", 0);  // questionmark
            int pos_ampersand = indexOf(rhl, "&", 0); // ampersand
            if (pos_http == -1) {
                log_w("GET without HTTP?");
                goto exit;
            }

            // cmd between "GET /" and "?" or "HTTP"
            int start_part1 = 8; // after "GET /"
            int end_part1 = (pos_question != -1) ? pos_question : pos_http;
            strncpy(http_cmd, rhl + start_part1, end_part1 - start_part1);

            // param between "?" and "&" or HTTP, if "?" exists
            if (pos_question != -1) {
                int start_part2 = pos_question + 1;
                int end_part2 = (pos_ampersand != -1) ? pos_ampersand : pos_http;
                strncpy(http_param, rhl + start_part2, end_part2 - start_part2);
            }

            // arg between "&" and "HTTP" if "&" exists
            if (pos_ampersand != -1) {
                int start_part3 = pos_ampersand + 1;
                strncpy(http_arg, rhl + start_part3, pos_http - start_part3);
            }
        } else if (startsWith(rhl, "content-length:")) {
            const char* c_cl = (rhl + 15);
            contentLength = atoi(c_cl);

        } else {
            ;
        }
    }

lastToDo:
    if (method_GET) {
        url_decode_in_place(http_cmd);
        trim(http_cmd);
        url_decode_in_place(http_param);
        trim(http_param);
        url_decode_in_place(http_arg);
        trim(http_arg);
        if (strlen(http_cmd) == 0) strcpy(http_cmd, "index.html");
        if (startsWith(http_cmd, "SD/")) { // SD/logo/0N 90s.jpg ->  http_cmd = SD/    http_param = /logo/0N 90s.jpg
            strcpy(http_param, http_cmd + 2);
            http_cmd[3] = '\0';
        }
        m_msg.e = evt_command;
        m_msg.cmd.assignf("%s", http_cmd);
        m_msg.param1.assignf("%s", http_param);
        m_msg.arg1.assignf("%s", http_arg);
        if (m_websrv_callback) m_websrv_callback(m_msg);
        if (WEBSRV_onCommand) WEBSRV_onCommand(http_cmd, http_param, http_arg);
    }
    if (method_POST) {
        url_decode_in_place(http_cmd);
        trim(http_cmd);
        url_decode_in_place(http_param);
        trim(http_param);
        url_decode_in_place(http_arg);
        trim(http_arg);
        m_msg.e = evt_info;
        //    m_msg.arg = http_cmd;
        if (m_websrv_callback) m_websrv_callback(m_msg);
        if (WEBSRV_onRequest) WEBSRV_onRequest(http_cmd, http_param, http_arg, contentType, contentLength);
    }
    if (method_DELETE) {
        url_decode_in_place(http_cmd);
        trim(http_cmd);
        url_decode_in_place(http_param);
        trim(http_param);
        url_decode_in_place(http_arg);
        trim(http_arg);
        m_msg.e = evt_info;
        //    m_msg.arg = http_cmd;
        if (m_websrv_callback) m_websrv_callback(m_msg);
        if (WEBSRV_onDelete) WEBSRV_onDelete(http_cmd, http_param, http_arg);
    }
exit:
    cmdClientAccept = true;
    return true;
}
//--------------------------------------------------------------------------------------------------------------
bool WebSrv::handleWS() {    // Websocketserver, receive messages
    String currentLine = ""; // Build up to complete line

    if (!webSocketClient.connected()) {
        log_e("webSocketClient should be connected but is not!");
        hasclient_WS = false;
        return false;
    }

    if (!hasclient_WS) {
        while (true) {
            currentLine = webSocketClient.readStringUntil('\n');

            if (currentLine.length() == 1) { // contains '\n' only
                if (ws_conn_request_flag) {
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

    if (av) { parseWsMessage(av); }
    return true;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::parseWsMessage(uint32_t len) {
    uint8_t      headerLen = 2;
    uint16_t     paylodLen = 0;
    uint8_t      maskingKey[4];
    char         c[2] = {0};
    ps_ptr<char> msgBuff;

    if (len > UINT16_MAX) {
        log_e("Websocketmessage too long");
        return;
    }

    webSocketClient.readBytes(c, 1);
    uint8_t fin = ((c[0] >> 7) & 0x01);
    (void)fin;
    uint8_t rsv1 = ((c[0] >> 6) & 0x01);
    (void)rsv1;
    uint8_t rsv2 = ((c[0] >> 5) & 0x01);
    (void)rsv2;
    uint8_t rsv3 = ((c[0] >> 4) & 0x01);
    (void)rsv3;
    uint8_t opcode = (c[0] & 0x0F);

    webSocketClient.readBytes(c, 1);
    uint8_t mask = ((c[0] >> 7) & 0x01);
    paylodLen = (c[0] & 0x7F);

    if (paylodLen == 126) {
        headerLen = 4;
        webSocketClient.readBytes(c, 2);
        paylodLen = c[0] << 8;
        paylodLen += c[1];
    }

    uint16_t msgBuffPtr = 0;
    msgBuff.calloc(paylodLen + 1);
    if (!msgBuff.valid()) {
        log_e("oom");
        goto exit;
    }

    (void)headerLen;

    if (mask) {
        maskingKey[0] = webSocketClient.read();
        maskingKey[1] = webSocketClient.read();
        maskingKey[2] = webSocketClient.read();
        maskingKey[3] = webSocketClient.read();
    }

    if (opcode == 0x08) { // denotes a connection close
        m_msg.e = evt_info;
        m_msg.arg = "websocket connection closed";
        if (m_websrv_callback) m_websrv_callback(m_msg);
        hasclient_WS = false;
        webSocketClient.stop();
        goto exit;
    }

    if (opcode == 0x09) { // denotes a ping
        m_msg.e = evt_command;
        m_msg.cmd = "ping received, send pong";
        m_msg.param1 = "";
        m_msg.arg1 = "";
        if (m_websrv_callback) m_websrv_callback(m_msg);
        if (WEBSRV_onCommand) WEBSRV_onCommand("ping received, send pong", "", "");
        m_msg.e = evt_info;
        m_msg.arg = "ping received, send pong";
        if (m_websrv_callback) m_websrv_callback(m_msg);
        sendPong();
    }

    if (opcode == 0x0A) { // denotes a pong
        m_msg.e = evt_command;
        m_msg.cmd = "pong received";
        m_msg.param1 = "";
        m_msg.arg1 = "";
        if (m_websrv_callback) m_websrv_callback(m_msg);
        if (WEBSRV_onCommand) WEBSRV_onCommand("pong received", "", "");
        m_msg.e = evt_info;
        m_msg.arg = "pong received";
        if (m_websrv_callback) m_websrv_callback(m_msg);
        goto exit;
    }

    if (opcode == 0x01) { // denotes a text frame
        int32_t  plen = 0;
        uint32_t pll = paylodLen;
        while (paylodLen) {
            if (paylodLen > 255) {
                plen = 255;
                plen = webSocketClient.readBytes(msgBuff.get() + msgBuffPtr, plen);
                paylodLen -= plen;
                msgBuffPtr += plen;
            } else {
                plen = paylodLen;
                paylodLen = 0;
                webSocketClient.readBytes(msgBuff.get() + msgBuffPtr, plen);
                msgBuffPtr += plen;
            }
        }
        if (mask) {
            for (int32_t i = 0; i < pll; i++) { msgBuff[i] = (msgBuff[i] ^ maskingKey[i % 4]); }
        }
        msgBuff[pll] = '\0';

        int pos1 = msgBuff.index_of('=');
        int pos2 = msgBuff.index_of('&', pos1 + 1);
        if (pos1 < 0) {
            m_msg.cmd = msgBuff;
            m_msg.param1.assign("");
            m_msg.arg1.assign("");
        } else if (pos2 < 0) {
            m_msg.cmd = msgBuff.substr(0, pos1);
            m_msg.param1 = msgBuff.substr(pos1 + 1);
            m_msg.arg1.assign("");
        } else {
            m_msg.cmd = msgBuff.substr(0, pos1);
            m_msg.param1 = msgBuff.substr(pos1 + 1, pos2 - pos1 - 1);
            m_msg.arg1 = msgBuff.substr(pos2 + 1);
        }
        m_msg.e = evt_command;
        if (m_websrv_callback) m_websrv_callback(m_msg);

        m_msg.e = evt_info;
        m_msg.arg = msgBuff;
        if (m_websrv_callback) m_websrv_callback(m_msg);
        const char* cmd = msgBuff.get();
        const char* param = NULL;
        const char* arg = NULL;
        int32_t     idx1 = msgBuff.index_of('=');
        if (idx1 > 0) {
            msgBuff[idx1] = '\0';
            const char* cmd = msgBuff.get();
            int32_t     offset = idx1 + 1;
            int32_t     idx2 = lastIndexOf(msgBuff.get() + offset, '&');
            if (idx2 > 0) {
                *(msgBuff.get() + offset + idx2) = '\0';
                param = msgBuff.get() + offset;
                arg = msgBuff.get() + offset + idx2 + 1;
                if (WEBSRV_onCommand) WEBSRV_onCommand(cmd, param, arg);
                goto exit;
            } else {
                param = msgBuff.get() + offset;
                if (WEBSRV_onCommand) WEBSRV_onCommand(cmd, param, "");
                goto exit;
            }
        } else {
            if (WEBSRV_onCommand) WEBSRV_onCommand(cmd, "", "");
            goto exit;
        }
        if (WEBSRV_onCommand) WEBSRV_onCommand((const char*)msgBuff.c_get(), "", "");
    } else {
        log_e("opcode != 0x01");
    }
exit:
    return;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::loop() {
    static uint32_t timer;
    if (cmdClientAccept) timer = millis();
    if (timer + 2000 < millis()) {
        log_e("cmdClient timeout");
        cmdClientAccept = true;
    }

    if (m_handle_upload) {
        handle_upload_file();
    } else if (cmdclient.available()) {
        handlehttp();
        return;
    } else if (cmdClientAccept) {
        cmdclient = cmdserver.accept();
    } else {
        ;
    }

    if (webSocketClient.available()) {
        m_msg.e = evt_info;
        m_msg.arg.assign("WebSocket client available");
        if (m_websrv_callback) m_websrv_callback(m_msg);
        handleWS();
    }

    if (!webSocketClient.connected()) { hasclient_WS = false; }
    if (!hasclient_WS) webSocketClient = webSocketServer.accept();

    return;
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::reply(ps_ptr<char> response, const char* MIMEType, bool header) {
    ps_ptr<char> http_header;
    http_header.set_name("http_header");
    if (header == true) {
        int32_t l = response.strlen();
        http_header.assign("HTTP/1.1 200 OK\r\n");
        http_header.appendf("Content-type: %s\r\n", MIMEType);
        http_header.appendf("Content-Length: %i\r\n\r\n", l);
        cmdclient.print(http_header.c_get()); // header sent
    }
    cmdclient.print(response.c_get());

    // http_header.println();
    // response.println();
}
//--------------------------------------------------------------------------------------------------------------
void WebSrv::sendStatus(uint16_t HTTPstatusCode) {
    int32_t l = 0; // respunse length
    // HTTP header
    String httpheader = "";
    httpheader += "HTTP/1.1 " + String(HTTPstatusCode, 10) + "\r\n";
    httpheader += "Connection: close\r\n";
    httpheader += "Content-type: text/html\r\n";
    httpheader += "Content-Length: " + String(l, 10) + "\r\n";
    httpheader += "Server: " + _Name + "\r\n";
    httpheader += "Cache-Control: max-age=3600\r\n";
    httpheader += "Last-Modified: " + _Version + "\r\n\r\n";
    cmdclient.print(httpheader); // header sent
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::UTF8toASCII(String str) {
    uint16_t i = 0;
    String   res = "";
    char     tab[96] = {96,  173, 155, 156, 32,  157, 32,  32,  32,  32,  166, 174, 170, 32,  32,  32,  248, 241, 253, 32,  32,  230, 32,  250, 32, 32,  167, 175, 172, 171, 32, 168,
                        32,  32,  32,  32,  142, 143, 146, 128, 32,  144, 32,  32,  32,  32,  32,  32,  32,  165, 32,  32,  32,  32,  153, 32,  32, 32,  32,  32,  154, 32,  32, 225,
                        133, 160, 131, 32,  132, 134, 145, 135, 138, 130, 136, 137, 141, 161, 140, 139, 32,  164, 149, 162, 147, 32,  148, 246, 32, 151, 163, 150, 129, 32,  32, 152};
    while (str[i] != 0) {
        if (str[i] == 0xC2) { // compute unicode from utf8
            i++;
            if ((str[i] > 159) && (str[i] < 192))
                res += char(tab[str[i] - 160]);
            else
                res += char(32);
        } else if (str[i] == 0xC3) {
            i++;
            if ((str[i] > 127) && (str[i] < 192))
                res += char(tab[str[i] - 96]);
            else
                res += char(32);
        } else
            res += str[i];
        i++;
    }
    return res;
}
//--------------------------------------------------------------------------------------------------------------
// replaces invalid UTF-8 sequences (only allows valid characters)
std::string WebSrv::sanitize_utf8_replace(const char* input, size_t len) {
    std::string output;
    output.reserve(len);

    const unsigned char* s = reinterpret_cast<const unsigned char*>(input);
    const unsigned char* end = s + len;

    const char replacement[] = "\xEF\xBF\xBD"; // UTF-8 für U+FFFD → „�“

    while (s < end) {
        unsigned char c = *s;

        // 1-Byte (ASCII)
        if (c <= 0x7F) {
            output.push_back(c);
            s++;
        }
        // 2-Byte Sequenz
        else if ((c >> 5) == 0x6 &&
                 s + 1 < end &&
                 (s[1] & 0xC0) == 0x80) {
            output.append(reinterpret_cast<const char*>(s), 2);
            s += 2;
        }
        // 3-Byte Sequenz
        else if ((c >> 4) == 0xE &&
                 s + 2 < end &&
                 (s[1] & 0xC0) == 0x80 &&
                 (s[2] & 0xC0) == 0x80) {
            output.append(reinterpret_cast<const char*>(s), 3);
            s += 3;
        }
        // 4-Byte Sequenz
        else if ((c >> 3) == 0x1E &&
                 s + 3 < end &&
                 (s[1] & 0xC0) == 0x80 &&
                 (s[2] & 0xC0) == 0x80 &&
                 (s[3] & 0xC0) == 0x80) {
            output.append(reinterpret_cast<const char*>(s), 4);
            s += 4;
        }
        // Ungültig → Ersatzzeichen einfügen
        else {
            output.append(replacement, 3);
            s++;
        }
    }

    return output;
}
//--------------------------------------------------------------------------------------------------------------
String WebSrv::URLdecode(String str) {
    String   hex = "0123456789ABCDEF";
    String   res = "";
    uint16_t i = 0;
    while (str[i] != 0) {
        if ((str[i] == '%') && isHexadecimalDigit(str[i + 1]) && isHexadecimalDigit(str[i + 2])) {
            res += char((hex.indexOf(str[i + 1]) << 4) + hex.indexOf(str[i + 2]));
            i += 3;
        } else {
            res += str[i];
            i++;
        }
    }
    return res;
}

void WebSrv::url_decode_in_place(char* url) {
    int length = strlen(url);
    int write_pos = 0; // Die Position, an die das dekodierte Zeichen geschrieben wird

    auto from_hex = [](char ch) { return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10; };

    for (int i = 0; i < length; ++i) {
        if (url[i] == '%') {
            if (i + 2 < length) {
                // Dekodiere die beiden folgenden Hex-Zeichen
                int hex_value = from_hex(url[i + 1]) * 16 + from_hex(url[i + 2]);
                url[write_pos++] = static_cast<char>(hex_value); // Schreibe dekodiertes Zeichen
                i += 2;                                          // Überspringe die beiden Hex-Zeichen
            }
        } else if (url[i] == '+') {
            // do nothing
            url[write_pos++] = '+';
        } else {
            // Normales Zeichen einfach kopieren
            url[write_pos++] = url[i];
        }
    }
    url[write_pos] = '\0'; // Add a null termination character to mark the end of the string
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
        default: return "";
    }
}
//--------------------------------------------------------------------------------------------------------------