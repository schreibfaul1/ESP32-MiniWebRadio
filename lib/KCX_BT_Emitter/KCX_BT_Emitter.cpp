/*
 * KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 25.01.2024
 *      Author: Wolle
 */

#include "KCX_BT_Emitter.h"

KCX_BT_Emitter::KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin){
    BT_EMITTER_MODE = mode_pin;
    BT_EMITTER_LINK = link_pin;
    BT_EMITTER_RX   = RX_pin;
    BT_EMITTER_TX   = TX_pin;
    pinMode(BT_EMITTER_LINK, INPUT_PULLUP);
    pinMode(BT_EMITTER_MODE, OUTPUT);
    digitalWrite(BT_EMITTER_MODE, HIGH);
    m_f_status = digitalRead(BT_EMITTER_LINK);
    m_f_linkChanged = false;
    m_f_waitForBtEmitter = false;
}

KCX_BT_Emitter::~KCX_BT_Emitter(){
    if(m_bt_version){free(m_bt_version); m_bt_version = NULL;}
    if(m_lastMsg){free(m_lastMsg); m_lastMsg = NULL;}
    if(m_autoLink){free(m_autoLink); m_autoLink = NULL;}
    if(m_lastCommand){free(m_lastCommand); m_lastCommand = NULL;}
    if(m_jsonMemItemsStr){free(m_jsonMemItemsStr); m_jsonMemItemsStr = NULL;}
    vector_clear_and_shrink(m_bt_addr);
    vector_clear_and_shrink(m_bt_names);
}

void KCX_BT_Emitter::begin(){
    if(psramInit()) m_f_PSRAMfound = true;
    if(m_f_PSRAMfound){
        m_chbuf  = (char*) ps_malloc(100);
        m_msgbuf = (char*) ps_malloc(100);
    }
    else{
        m_chbuf  = (char*) malloc(100);
        m_msgbuf = (char*) malloc(100);
    }

    Serial2.begin(115200, SERIAL_8N1, BT_EMITTER_TX, BT_EMITTER_RX);
    attachInterrupt(BT_EMITTER_LINK, isr0, CHANGE);
    objPtr = this;
    tckPtr = this;
    writeCommand("AT+");
    m_timeStamp = millis();
    tck1s.attach(1, t1s);
    m_f_waitForBtEmitter = true;
    m_f_btEmitter_found = false;
    m_bt_add_num = 0;
    m_bt_name_num = 0;
    m_f_powerOn = true;
}

void KCX_BT_Emitter::loop(){
    if(m_f_ticker1s){
        m_f_ticker1s = false;
        handle1sEvent();
        return;
    }
    if(!Serial2.available()){ return;}
    else                    { readCmd();}
    return;
}

void KCX_BT_Emitter::readCmd() {
    uint32_t t = millis() + 500;
    uint8_t  idx = 0;
    while(true) {
        if(t < millis()) {
            timeout();
            break;
        }
        char ch = Serial2.read();
        if(ch == -1) continue;
        if(ch == '\n') {
            // log_i("%s", m_chbuf);
            break;
        }
        if(idx == 63) return;
        if(!isascii(ch)) { continue; }
        m_chbuf[idx] = ch;
        idx++;
    }
    m_chbuf[idx] = '\0';
    if(!idx) return; // nothing to parse

    if(!m_f_btEmitter_found) { detectOKcmd(); }
    else                     { parseATcmds(); }
}

void KCX_BT_Emitter::detectOKcmd(){
    if(strcmp(m_chbuf, "OK+") == 0){
        m_f_btEmitter_found = true;
        if(kcx_bt_info) kcx_bt_info("KCX_BT_Emitter found");
        m_timeCounter = 0;
        m_f_bt_inUse = false; // task completed
    }
}

void KCX_BT_Emitter::parseATcmds(){
//  log_i("%s", m_chbuf);
    if(startsWith(m_chbuf, "OK+VERS:"))         { bt_Version();        goto exit;}
    if(startsWith(m_chbuf, "POWER ON"))         { cmd_PowerOn();       goto exit;}
    if(startsWith(m_chbuf, "OK+BT"))            { cmd_Mode();          goto exit;}
    if(startsWith(m_chbuf, "Auto_link_Add:"))   { cmd_AutoLink();      goto exit;}
    if(startsWith(m_chbuf, "OK+VOL"))           { cmd_Volume();        goto exit;}
    if(startsWith(m_chbuf, "Delete_Vmlink"))    { cmd_Delete();        goto exit;}
    if(startsWith(m_chbuf, "BT_ADD_NUM"))       { cmd_AddNum();        goto exit;}
    if(startsWith(m_chbuf, "BT_NAME_NUM"))      { cmd_NameNum();       goto exit;}
    if(startsWith(m_chbuf, "MEM_Name"))         { cmd_MemName();       goto exit;}
    if(startsWith(m_chbuf, "MEM_MacAdd"))       { cmd_MemAddr();       goto exit;}
    if(startsWith(m_chbuf, "MacAdd"))           { cmd_scannedItems();  goto exit;}
    if(startsWith(m_chbuf, "OK+NAME"))          { cmd_connectedName(); goto exit;}
    if(startsWith(m_chbuf, "OK+MAC"))           { cmd_connectedAddr(); goto exit;}
    if(startsWith(m_chbuf, "OK+PAUSE"))         { cmd_statePause();    goto exit;}
    if(startsWith(m_chbuf, "OK+PLAY"))          { cmd_statePlay();     goto exit;}


    if(startsWith(m_chbuf, "Name More than 10")){ warning("more than 10 names are not allowed"); return;}
    if(startsWith(m_chbuf, "Addr More than 10")){ warning("more than 10 MAC Ardesses are not allowed"); return;}
    if(startsWith(m_chbuf, "CMD ERR!"))         { cmd_Wrong(); return;}

exit:
    if(m_lastMsg && strcmp(m_chbuf, m_lastMsg) == 0) return;
    else{
        if(m_lastMsg){free(m_lastMsg); m_lastMsg = NULL;} // don't repeat messages twice
        m_lastMsg = x_ps_strdup(m_chbuf);
        if(kcx_bt_info) kcx_bt_info(m_lastMsg);
        m_f_bt_inUse = false; // task completed
    }
}

void KCX_BT_Emitter::handle1sEvent(){
    m_timeCounter++;  // counts the seconds since KCX_BT_EMITTER found
    if(!m_f_btEmitter_found && m_f_waitForBtEmitter){
        if(m_timeStamp + 2000 < millis()){
            m_f_waitForBtEmitter = false;
            if(kcx_bt_info) kcx_bt_info("KCX_BT_Emitter not found");
        }
    }
    if(!m_f_btEmitter_found){
        writeCommand("AT+"); // another try
        return;
    }
    static uint32_t busyCounter = 0;
    if(m_f_bt_inUse) busyCounter++;
    if(busyCounter > 1){ // bt busy surveillance
        responseError();
        busyCounter = 0;
        m_f_bt_inUse = false;
    }

    if(m_timeCounter == 1) { writeCommand("AT+GMR?");     return;}  // get version
    if(m_timeCounter == 3) { writeCommand("AT+VMLINK?");  return;}  // get all mem vmlinks
    if(m_timeCounter == 5) { writeCommand("AT+VOL?");     return;}  // get volume (in receiver mode 0 ... 31)
    if(m_timeCounter == 7) { writeCommand("AT+BT_MODE?"); return;}  // transmitter or receiver
//  if(m_timeCounter == 9) { writeCommand("AT+PAUSE?");   return;}

    if(m_f_linkChanged){
        m_f_linkChanged = false;
        if(digitalRead(BT_EMITTER_LINK) == HIGH) m_f_status = BT_CONNECTED;
        else m_f_status = BT_NOT_CONNECTED;
        if(kcx_bt_status) kcx_bt_status(m_f_status);
        if(m_f_status == BT_CONNECTED && m_f_bt_mode == BT_MODE_RECEIVER) {writeCommand("AT+NAME?"); return;}
    }
    if(m_f_getMacAddr){ // in RECEIVER mode after AT+NAME?
        m_f_getMacAddr = false;
        writeCommand("AT+MAC?");
    }
}

void KCX_BT_Emitter::writeCommand(const char* cmd){
    if(!m_f_btEmitter_found){
        if(strcmp(cmd, "AT+") == 0) Serial2.printf("%s%s", cmd, "\r\n");
        return;
    }
    sprintf(m_msgbuf, "new command " ANSI_ESC_GREEN "%s", cmd);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    if(m_f_bt_inUse) {stillInUse(cmd); return;}
    m_f_bt_inUse = true;
    if(m_lastCommand){free(m_lastCommand); m_lastCommand = NULL;}
    m_lastCommand = x_ps_strdup(cmd);
    Serial2.printf("%s%s", m_lastCommand, "\r\n");
}

void KCX_BT_Emitter::bt_Version(){
    if(m_bt_version){free(m_bt_version); m_bt_version = NULL;}
    m_bt_version = x_ps_strdup(m_chbuf + 8);
    sprintf(m_msgbuf, "Version " ANSI_ESC_YELLOW "%s", m_bt_version);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::timeout(){
    if(!m_f_btEmitter_found) return;
    if(kcx_bt_info) kcx_bt_info(ANSI_ESC_RED "timeout while reading from KCX_BT_Emitter");
}

void KCX_BT_Emitter::stillInUse(const char* cmd){
    sprintf(m_msgbuf, ANSI_ESC_YELLOW "KCX_BT_Emitter is still in use, the current command could not be processed: %s", cmd);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
}

void KCX_BT_Emitter::responseError(){
    sprintf(m_msgbuf, ANSI_ESC_RED "no response: last command was %s", m_lastCommand);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
}

void KCX_BT_Emitter::warning(const char* w){
    sprintf(m_msgbuf, "warning " ANSI_ESC_YELLOW "%s", w);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    m_f_bt_inUse = false;
}

void KCX_BT_Emitter::cmd_Wrong(){
    sprintf(m_msgbuf, "wrong command: " ANSI_ESC_RED "%s", m_lastCommand);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::cmd_PowerOn(){
    if(kcx_bt_info) kcx_bt_info("POWER ON");
    m_f_powerOn = true;
    writeCommand("AT+BT_MODE?");
}

void KCX_BT_Emitter::cmd_Mode() {
    sprintf(m_msgbuf, "Mode -> " ANSI_ESC_YELLOW "%s", m_chbuf + 6);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);

    if(strcmp(m_chbuf + 6, "EMITTER") == 0) {
        if(!m_f_bt_mode) { // mode has changed
            if(kcx_bt_modeChanged) kcx_bt_modeChanged("TX");
        }
        m_f_bt_mode = true;
    }
    else if(strcmp(m_chbuf + 6, "RECEIVER") == 0) {
        if(m_f_bt_mode) { // mode has changed
            if(kcx_bt_modeChanged) kcx_bt_modeChanged("RX");
        }
        m_f_bt_mode = false;
    }
    else {
        log_e("unknown BT answer  %s", m_chbuf);
        return;
    }
    m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::cmd_AutoLink(){
    if(m_autoLink) { free(m_autoLink); m_autoLink = NULL; }
    m_autoLink = x_ps_strdup(m_chbuf + 14);
    sprintf(m_msgbuf, "Autolink -> " ANSI_ESC_YELLOW "%s", m_autoLink);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    if(m_bt_add_num + m_bt_name_num == 0) m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::cmd_Volume(){
    m_bt_volume = atoi(m_chbuf + 7);
    sprintf(m_msgbuf, "Volume -> " ANSI_ESC_BLUE "%02d", m_bt_volume);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::cmd_Delete(){
    if(kcx_bt_info) kcx_bt_info("all saved VM links are deleted");
    m_f_bt_inUse = false; // task completed
}

void KCX_BT_Emitter::cmd_AddNum(){
    m_bt_add_num = atoi(m_chbuf + 11);
    vector_clear_and_shrink(m_bt_addr);
    m_bt_add_cnt = 0;
}

void KCX_BT_Emitter::cmd_NameNum(){
    m_bt_name_num = atoi(m_chbuf + 12);
    vector_clear_and_shrink(m_bt_names);
    m_bt_name_cnt = 0;
}

void KCX_BT_Emitter::cmd_MemAddr(){
    m_bt_addr.push_back(strdup(m_chbuf + 14));
    if(kcx_bt_info) kcx_bt_info(m_chbuf);
    m_bt_add_cnt++;
    if(m_bt_add_num + m_bt_name_num == m_bt_add_cnt + m_bt_name_cnt){
        stringifyMemItems();
        m_f_bt_inUse = false; // task completed
    }
}

void KCX_BT_Emitter::cmd_MemName(){
    m_bt_names.push_back(strdup(m_chbuf + 12));
    if(kcx_bt_info) kcx_bt_info(m_chbuf);
    m_bt_name_cnt++;
    if(m_bt_add_num + m_bt_name_num == m_bt_add_cnt + m_bt_name_cnt){
        stringifyMemItems();
        m_f_bt_inUse = false; // task completed
    }
}

void KCX_BT_Emitter::cmd_scannedItems(){
    bool f_insert = true;
    for(uint i = 0; i < m_bt_scannedItems.size(); i++){
        if(strcmp(m_bt_scannedItems[i], m_chbuf) == 0){
           f_insert = false;
        }
    }
    if(f_insert){
        m_bt_scannedItems.insert(m_bt_scannedItems.begin(), strdup(m_chbuf));
        sprintf(m_msgbuf, "scanned: " ANSI_ESC_YELLOW "%s", m_bt_scannedItems[0]);
        if(kcx_bt_info) kcx_bt_info(m_msgbuf);
        const char* s = stringifyScannedItems();
        if(kcx_bt_scanItems) kcx_bt_scanItems(s);
    }
}
void KCX_BT_Emitter::cmd_connectedName(){
    sprintf(m_msgbuf, "connected with name: " ANSI_ESC_YELLOW "%s", m_chbuf + 8);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
    m_f_getMacAddr = true;
}

void KCX_BT_Emitter::cmd_connectedAddr(){
    sprintf(m_msgbuf, "connected with MAC addr: " ANSI_ESC_YELLOW "%s", m_chbuf + 7);
    if(kcx_bt_info) kcx_bt_info(m_msgbuf);
}

void KCX_BT_Emitter::cmd_statePause(){
    m_f_bt_state = BT_PAUSE;
    if(kcx_bt_info) kcx_bt_info(ANSI_ESC_YELLOW "PLAY -> PAUSE");
}

void KCX_BT_Emitter::cmd_statePlay(){
    m_f_bt_state = BT_PLAY;
    if(kcx_bt_info) kcx_bt_info(ANSI_ESC_YELLOW "PAUSE -> PLAY");
}

// -------------------------- user commands -----------------------------------
void KCX_BT_Emitter::deleteVMlinks(){
    writeCommand("AT+DELVMLINK");
}
void KCX_BT_Emitter::getVMlinks(){
    writeCommand("AT+VMLINK?");
}
void KCX_BT_Emitter::addLinkName(const char* name){
    sprintf(m_chbuf, "AT+ADDLINKNAME=%s", name);
    writeCommand(m_chbuf);
}
void KCX_BT_Emitter::addLinkAddr(const char* addr){
    sprintf(m_chbuf, "AT+ADDLINKADD=%s", addr);
    writeCommand(m_chbuf);
}
void KCX_BT_Emitter::setVolume(uint8_t vol){
    if(vol > 31){ vol = 31;}
    sprintf(m_chbuf, "AT+VOL=%d", vol);
    writeCommand(m_chbuf);
}

const char* KCX_BT_Emitter::getMode(){ // returns RX or TX
    if(m_f_bt_mode) return("RX");
    return("TX");
}

void KCX_BT_Emitter::changeMode(){
    if(!m_f_powerOn){
        warning("mode can't be changed, waiting for \"POWER ON\"");
        return;
    }
    digitalWrite(BT_EMITTER_MODE, !m_f_bt_mode);
    writeCommand("AT+RESET");
    m_f_powerOn = false;
}

// -------------------------- JSON relevant ----------------------------------
void KCX_BT_Emitter::stringifyMemItems() {
    // "AT+VMLINK" returns:
    // "OK+VMLINK"
    // "BT_ADD_NUM=01"              --> save in btAddNum
    // "BT_NAME_NUM=01"             --> save in btNameNum
    // "MEM_Name 00:MyName"         --> save in _KCX_BT_names vector
    // "MEM_MacAdd 00:82435181cc6a" --> save in _KCX_BT_addr vector
    uint16_t JSONstrLength = 0;
    if(m_jsonMemItemsStr) {
        free(m_jsonMemItemsStr);
        m_jsonMemItemsStr = NULL;
    }
    if(m_f_PSRAMfound) { m_jsonMemItemsStr = (char*)ps_malloc(2); }
    else { m_jsonMemItemsStr = (char*)malloc(2); }
    JSONstrLength += 2;
    memcpy(m_jsonMemItemsStr, "[\0", 2);
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6a"},{....}]
    for(int i = 0; i < 10; i++) {
        int a = 0, b = 0;
        if(m_bt_names.size() > i) a = strlen(m_bt_names[i]);
        if(m_bt_addr.size() > i) b = strlen(m_bt_addr[i]);
        JSONstrLength += 22 + a + b; // {"name":"a","addr":"b"},
        if(m_f_PSRAMfound) { m_jsonMemItemsStr = (char*)ps_realloc(m_jsonMemItemsStr, JSONstrLength); }
        else { m_jsonMemItemsStr = (char*)realloc(m_jsonMemItemsStr, JSONstrLength); }
        strcat(m_jsonMemItemsStr, "{\"name\":\"");
        if(a) strcat(m_jsonMemItemsStr, m_bt_names[i]);
        else strcat(m_jsonMemItemsStr, "");
        strcat(m_jsonMemItemsStr, "\",\"addr\":\"");
        if(b) strcat(m_jsonMemItemsStr, m_bt_addr[i]);
        else strcat(m_jsonMemItemsStr, "");
        strcat(m_jsonMemItemsStr, "\"},");
    }
    m_jsonMemItemsStr[JSONstrLength - 2] = ']';  // replace comma by square bracket close
    m_jsonMemItemsStr[JSONstrLength - 1] = '\0'; // and terminate
    if(kcx_bt_memItems) kcx_bt_memItems(m_jsonMemItemsStr);
    return;
}

const char* KCX_BT_Emitter::stringifyScannedItems(){ // returns the last three scanned BT devices as jsonStr
    uint16_t JSONstrLength = 0;
    if(m_jsonScanItemsStr){free(m_jsonScanItemsStr); m_jsonScanItemsStr = NULL;}
    JSONstrLength += 2;
    if(m_f_PSRAMfound) { m_jsonScanItemsStr = (char*)ps_malloc(2); }
    else               { m_jsonScanItemsStr = (char*)malloc(2);}
    memcpy(m_jsonScanItemsStr, "[\0", 2);
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6b"},{name":"btthirdName","addr":"82435181cc6c"}]
    for(int i = 0; i < 3; i++) {
        int a = 0, idx1 = 0, idx2 = 0;
        if(m_bt_scannedItems.size() > i){
            a = strlen(m_bt_scannedItems[i]) - 13;     // MacAdd:82435181cc6a,Name:VHM-314
            if(a < 12) {log_e("wrong scanned items %s", m_bt_scannedItems[i]); return "null";} // can't be, MacAddr must have 12 chars
            idx1 = indexOf(m_bt_scannedItems[i], ":", 0);
            idx2 = indexOf(m_bt_scannedItems[i], ",", idx1);
        }
        JSONstrLength += 22 + a; // {"name":"a","addr":"b"},
        if(m_f_PSRAMfound) { m_jsonScanItemsStr = (char*)ps_realloc(m_jsonScanItemsStr, JSONstrLength); }
        else { m_jsonScanItemsStr = (char*)realloc(m_jsonScanItemsStr, JSONstrLength); }
        strcat(m_jsonScanItemsStr, "{\"addr\":\"");
        if(a) strncat(m_jsonScanItemsStr, m_bt_scannedItems[i] + idx1 + 1, idx2 - idx1 - 1);
        else strcat(m_jsonScanItemsStr, "");
        strcat(m_jsonScanItemsStr, "\",\"name\":\"");
        if(a) strcat(m_jsonScanItemsStr, m_bt_scannedItems[i] + idx2 + 1 + 5);
        else strcat(m_jsonScanItemsStr, "");
        strcat(m_jsonScanItemsStr, "\"},");
    }
    m_jsonScanItemsStr[JSONstrLength - 2] = ']';  // replace comma by square bracket close
    m_jsonScanItemsStr[JSONstrLength - 1] = '\0'; // and terminate
    return m_jsonScanItemsStr;
}

//-------------------------interrupt handling ---------------------------------
void KCX_BT_Emitter::isr0(){
    objPtr->handleInterrupt();
}

void KCX_BT_Emitter::t1s(){
    tckPtr->handleTicker();
}

KCX_BT_Emitter* KCX_BT_Emitter::objPtr;
KCX_BT_Emitter* KCX_BT_Emitter::tckPtr;

void KCX_BT_Emitter::handleInterrupt(){
    m_f_linkChanged = true;
}

void KCX_BT_Emitter::handleTicker(){
    m_f_ticker1s = true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
