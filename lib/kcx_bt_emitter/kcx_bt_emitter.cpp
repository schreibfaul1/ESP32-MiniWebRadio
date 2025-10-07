/*
 *  KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 26.04.2025
 *      Author: Wolle
 */

#include "kcx_bt_emitter.h"

//----------------------------------------------------------------------------------------------------------------------------------------------------
KCX_BT_Emitter::KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin) {
    BT_EMITTER_MODE = mode_pin;
    BT_EMITTER_LINK = link_pin;
    BT_EMITTER_RX = RX_pin;
    BT_EMITTER_TX = TX_pin;
    if (BT_EMITTER_MODE < 0 || BT_EMITTER_LINK < 0 || BT_EMITTER_RX < 0 || BT_EMITTER_TX < 0) return;
    m_f_KCX_BT_Emitter_isInit = true;
    pinMode(BT_EMITTER_LINK, INPUT_PULLUP);
    pinMode(BT_EMITTER_MODE, OUTPUT);
    digitalWrite(BT_EMITTER_MODE, LOW);
    m_f_status = digitalRead(BT_EMITTER_LINK);
    m_f_linkChanged = false;
    m_f_waitForBtEmitter = false;
    m_myName = strdup("Only in receive mode");
    m_f_bt_inUse = false;
}

KCX_BT_Emitter::~KCX_BT_Emitter() {
    if (m_bt_version) {
        free(m_bt_version);
        m_bt_version = NULL;
    }
    if (m_lastMsg) {
        free(m_lastMsg);
        m_lastMsg = NULL;
    }
    if (m_autoLink) {
        free(m_autoLink);
        m_autoLink = NULL;
    }
    if (m_jsonMemItemsStr) {
        free(m_jsonMemItemsStr);
        m_jsonMemItemsStr = NULL;
    }
    if (m_myName) {
        free(m_myName);
        m_myName = NULL;
    }
    if (m_chbuf) {
        free(m_chbuf);
        m_chbuf = NULL;
    }
    vector_clear_and_shrink(m_bt_addr);
    vector_clear_and_shrink(m_bt_names);
    m_RX_TX_protocol.clear();

    tck1s.detach();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::begin() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (psramInit()) m_f_PSRAMfound = true;
    if (m_f_PSRAMfound) {
        m_chbuf = (char*)ps_malloc(m_chbufSize);
    } else {
        m_chbuf = (char*)malloc(m_chbufSize);
    }
    //    digitalWrite(BT_EMITTER_MODE, HIGH);
    Serial2.begin(115200, SERIAL_8N1, BT_EMITTER_TX, BT_EMITTER_RX);
    attachInterrupt(BT_EMITTER_LINK, isr0, CHANGE);
    objPtr = this;
    tckPtr = this;

    vTaskDelay(100);
    while (Serial2.available()) { Serial2.read(); } // empty readbuffer
    userCommand("AT+");
    m_timeStamp = millis();
    tck1s.attach(1, t1s);
    m_f_waitForBtEmitter = true;
    m_f_btEmitter_found = false;
    m_bt_add_num = 0;
    m_bt_name_num = 0;
    m_f_bt_inUse = false;
}

void KCX_BT_Emitter::loop() {
    if (!m_f_KCX_BT_Emitter_isInit) return;

    if (!m_f_bt_inUse) {
        if (m_messageQueue.size()) {
            ps_ptr<char> qi;
            qi = getQueueItem();
            KCX_LOG_ERROR("%s", qi.c_get());
            writeCommand(qi);
            m_f_bt_inUse = true;
            m_timeStamp = millis();
        }
    }
    if (Serial2.available()) { readCmd(); }
    if (m_f_ticker1s) {
        m_f_ticker1s = false;
        handle1sEvent();
    }
}

void KCX_BT_Emitter::readCmd() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    uint32_t t = millis() + 500;
    uint8_t  idx = 0;
    int16_t  ch = 0;
    memset(m_chbuf, 0, m_chbufSize);
    while (true) {
        if (t < millis()) {
            timeout();
            return;
        }
        ch = Serial2.read();
        vTaskDelay(1);
        if (ch == '\n') {
            //    log_w("%s", m_chbuf);
            break;
        }
        if (ch > 127) { continue; }
        if (ch < 32) { continue; }
        m_chbuf[idx] = ch;
        idx++;
    }
    if (!idx) return; // nothing to parse
    protocol_addElement("RX", m_chbuf);
    if (!m_f_btEmitter_found) {
        detectOKcmd();
    } else {
        parseATcmds();
    }
}

void KCX_BT_Emitter::detectOKcmd() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (strcmp(m_chbuf, "OK+") == 0) {
        m_f_btEmitter_found = true;
        if (kcx_bt_info) kcx_bt_info("KCX_BT_Emitter found", "");
        m_timeCounter = 0;
        m_f_bt_inUse = false; // task completed
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::writeCommand(ps_ptr<char> cmd) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (!cmd.valid()) return; // guard
    protocol_addElement("TX", cmd.c_get());

    if (cmd.equals("AT+")) {
        Serial2.printf("%s%s", cmd.c_get(), "\r\n");
        //        m_f_bt_inUse = true;
        return;
    }
    //  if(kcx_bt_info) kcx_bt_info("new command", cmd);
    if (m_f_bt_inUse) {
        stillInUse(cmd.c_get());
        return;
    }
    //    m_f_bt_inUse = true;

    KCX_LOG_ERROR("%s", cmd.c_get());
    Serial2.printf("%s%s", cmd.c_get(), "\r\n");
    if (cmd.starts_with("AT+GMR")) {
        m_Cmd = 1;
        goto exit;
    }
    if (cmd.starts_with("AT+RESET")) {
        m_Cmd = 2;
        goto exit;
    }
    if (cmd.starts_with("AT+BT_MODE")) {
        m_Cmd = 3;
        goto exit;
    }
    if (cmd.starts_with("AT+VOL")) {
        m_Cmd = 4;
        goto exit;
    }
    if (cmd.starts_with("AT+DELVMLINK")) {
        m_Cmd = 5;
        goto exit;
    }
    if (cmd.starts_with("AT+NAME")) {
        m_Cmd = 6;
        goto exit;
    }
    if (cmd.starts_with("AT+MAC")) {
        m_Cmd = 7;
        goto exit;
    }
    if (cmd.starts_with("AT+PAUSE")) {
        m_Cmd = 8;
        goto exit;
    }
    if (cmd.starts_with("AT+VMLINK")) {
        m_Cmd = 9;
        goto exit;
    }
    if (cmd.starts_with("AT+POWER_OFF")) {
        m_Cmd = 10;
        goto exit;
    }

    KCX_LOG_WARN("unknown command %s", cmd.c_get());
exit:
    return;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::parseATcmds() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    //  log_i("%s", m_chbuf);
    bool ie = 0; // if true throw info event message
    if (startsWith(m_chbuf, "OK+VERS:")) {
        bt_Version();
        m_Answ = 1;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "POWER ON")) {
        cmd_PowerOn();
        m_Answ = 2;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "OK+BT")) {
        cmd_Mode();
        m_Answ = 3;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "Auto_link_Add:")) {
        cmd_AutoLink();
        m_Answ = -1;
        ie = 0;
        goto P1;
    }
    if (startsWith(m_chbuf, "OK+VOL")) {
        cmd_Volume();
        m_Answ = 4;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "Delete_Vmlink")) {
        cmd_Delete();
        m_Answ = 5;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "BT_ADD_NUM")) {
        cmd_AddNum();
        m_Answ = -1;
        ie = 0;
        goto exit;
    }
    if (startsWith(m_chbuf, "BT_NAME_NUM")) {
        cmd_NameNum();
        m_Answ = -1;
        ie = 0;
        goto exit;
    }
    if (startsWith(m_chbuf, "MEM_Name")) {
        cmd_MemName();
        m_Answ = -1;
        ie = 0;
        goto P0;
    }
    if (startsWith(m_chbuf, "MEM_MacAdd")) {
        cmd_MemAddr();
        m_Answ = -1;
        ie = 0;
        goto P0;
    }
    if (startsWith(m_chbuf, "MacAdd")) {
        cmd_scannedItems();
        m_Answ = -1;
        ie = 0;
        goto exit;
    }
    if (startsWith(m_chbuf, "OK+NAME")) {
        cmd_connectedName();
        m_Answ = 6;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "OK+MAC")) {
        cmd_connectedAddr();
        m_Answ = 7;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "OK+PAUSE")) {
        cmd_statePause();
        m_Answ = 8;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "OK+PLAY")) {
        cmd_statePlay();
        m_Answ = 8;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "SCAN")) {
        cmd_ScanMode();
        m_Answ = -1;
        ie = 0;
        goto exit;
    }
    if (startsWith(m_chbuf, "OK+VMLINK")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "OK+RESET")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "CON ONE")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "CON LAST")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "CON MATCH ADD")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "CONNECT")) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }
    if (startsWith(m_chbuf, "OK+POWEROFF_MODE")) {
        m_Answ = 10;
        ie = 1;
        goto exit;
    }

    if (startsWith(m_chbuf, "Name More than 10")) {
        warning("more than 10 names are not allowed");
        m_Answ = -90;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "Addr More than 10")) {
        warning("more than 10 MAC Ardesses are not allowed");
        m_Answ = -90;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "Not in emitter mode!")) {
        m_Answ = 9;
        ie = 0;
        goto P2;
    }
    if (startsWith(m_chbuf, "no connect!")) {
        m_Answ = -90;
        ie = 1;
        goto P2;
    }

    if (startsWith(m_chbuf, "CMD ERR!")) {
        cmd_Wrong();
        m_Answ = -90;
        ie = 0;
        goto P2;
    }
    if (strlen(m_chbuf) < 4) {
        m_Answ = -1;
        ie = 1;
        goto exit;
    }

    // log_w("unknown message %s", m_chbuf);
    goto exit;

P0:
    if (m_bt_add_num + m_bt_name_num == m_bt_add_cnt + m_bt_name_cnt) {
        stringifyMemItems();
        m_Answ = 9;
    }

P1:
    if (m_bt_add_num + m_bt_name_num == 0) m_f_bt_inUse = false; // task completed

P2:
    if (m_Answ == m_Cmd) {
        m_f_bt_inUse = false; // task completed
    }
    if (m_Answ == -90) {
        m_f_bt_inUse = false; // task ended due by warning
    }
    if (m_Answ == -100) {
        m_f_bt_inUse = false; // task ended due to an error
    }

exit:
    if (m_lastMsg && strcmp(m_chbuf, m_lastMsg) != 0) {
        if (kcx_bt_info && ie && strlen(m_chbuf) > 3) kcx_bt_info(m_chbuf, "");
    }
    if (m_lastMsg) {
        free(m_lastMsg);
        m_lastMsg = NULL;
    } // don't repeat messages twice
    m_lastMsg = x_ps_strdup(m_chbuf);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::handle1sEvent() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    m_timeCounter++; // counts the seconds since KCX_BT_EMITTER found
    if (!m_f_btEmitter_found && m_f_waitForBtEmitter) {
        if (m_timeStamp + 2000 < millis()) {
            m_f_waitForBtEmitter = false;
            if (kcx_bt_info) kcx_bt_info("KCX_BT_Emitter not found", "");
            Serial2.end();
        }
    }
    if (m_timeCounter > 60) {
        // userCommand("AT+"); // alive check
        //  if(kcx_bt_info) kcx_bt_info("KCX_BT_Emitter alive check", "");
        m_timeCounter = 0;
    }
    static uint32_t busyCounter = 0;
    if (m_f_bt_inUse)
        busyCounter++;
    else
        busyCounter = 0;
    if (busyCounter > 5) { // bt busy surveillance
        responseError();
        busyCounter = 0;
        m_f_bt_inUse = false;
    }

    if (m_f_linkChanged) {
        m_f_linkChanged = false;
        if (digitalRead(BT_EMITTER_LINK) == HIGH) {
            m_f_status = BT_CONNECTED;
            m_f_scan = false;
            if (kcx_bt_info) kcx_bt_info("Status ->", "Connected");
        } else {
            m_f_status = BT_NOT_CONNECTED;
            if (kcx_bt_info) kcx_bt_info("Status ->", "Disconnected");
        }
        if (kcx_bt_status) kcx_bt_status(m_f_status);
        if (m_f_status == BT_CONNECTED && m_f_bt_mode == BT_MODE_RECEIVER) {
            addQueueItem("AT+NAME?");
            addQueueItem("AT+MAC?");
            return;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::bt_Version() {
    if (m_bt_version) {
        free(m_bt_version);
        m_bt_version = NULL;
    }
    m_bt_version = x_ps_strdup(m_chbuf + 8);
    if (kcx_bt_info) kcx_bt_info("Version", m_bt_version);
}

void KCX_BT_Emitter::timeout() {
    if (!m_f_btEmitter_found) return;
    if (kcx_bt_info && m_f_status) kcx_bt_info("timeout while reading from KCX_BT_Emitter", "");
}

void KCX_BT_Emitter::stillInUse(const char* cmd) {
    if (kcx_bt_info) kcx_bt_info("KCX_BT_Emitter is still in use, the current command could not be processed", cmd);
}

void KCX_BT_Emitter::responseError() {
    if (m_RX_TX_protocol.size()) {
        KCX_LOG_WARN("no response: last command was:", m_RX_TX_protocol[m_RX_TX_protocol.size() - 1].c_get());
    } else {
        KCX_LOG_WARN("m_RX_TX_protocol is empty");
    }
}

void KCX_BT_Emitter::warning(const char* w) {
    if (kcx_bt_info) kcx_bt_info("warning", w);
}

void KCX_BT_Emitter::cmd_Wrong() {
    if (m_RX_TX_protocol.size()) {
        KCX_LOG_WARN("wrong command:", m_RX_TX_protocol[m_RX_TX_protocol.size() - 1].c_get());
    } else {
        KCX_LOG_WARN("m_RX_TX_protocol is empty");
    }
}

void KCX_BT_Emitter::cmd_PowerOn() {
    if (kcx_bt_info) kcx_bt_info("POWER ON", "");
    addQueueItem("AT+BT_MODE?");
}

void KCX_BT_Emitter::cmd_PowerOff() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (kcx_bt_info) kcx_bt_info("POWER OFF", "");
    addQueueItem("AT+POWER_OFF");
}

void KCX_BT_Emitter::cmd_Mode() {
    if (strcmp(m_chbuf + 6, "EMITTER") == 0) {
        if (m_f_bt_mode == BT_MODE_RECEIVER) { // mode has changed
            if (kcx_bt_info) kcx_bt_info("Mode ->", m_chbuf + 6);
            if (kcx_bt_modeChanged) kcx_bt_modeChanged("TX");
        }
        m_f_bt_mode = BT_MODE_EMITTER;
    } else if (strcmp(m_chbuf + 6, "RECEIVER") == 0) {
        if (m_f_bt_mode == BT_MODE_EMITTER) { // mode has changed
            if (kcx_bt_modeChanged) kcx_bt_modeChanged("RX");
            if (kcx_bt_info) kcx_bt_info("Mode ->", m_chbuf + 6);
        }
        m_f_bt_mode = BT_MODE_RECEIVER;
    } else {
        KCX_LOG_ERROR("unknown BT answer  %s", m_chbuf);
        return;
    }
}

void KCX_BT_Emitter::cmd_AutoLink() {
    if (m_autoLink) {
        free(m_autoLink);
        m_autoLink = NULL;
    }
    m_autoLink = x_ps_strdup(m_chbuf + 14);
    if (kcx_bt_info) kcx_bt_info("Autolink ->", m_autoLink);
}

void KCX_BT_Emitter::cmd_Volume() {
    m_bt_volume = atoi(m_chbuf + 7);
    if (kcx_bt_info) kcx_bt_info("Volume ->", m_chbuf + 7);
}

void KCX_BT_Emitter::cmd_Delete() {
    if (kcx_bt_info) kcx_bt_info("all saved VM links are deleted", "");
}

void KCX_BT_Emitter::cmd_AddNum() {
    m_bt_add_num = atoi(m_chbuf + 11);
    vector_clear_and_shrink(m_bt_addr);
    m_bt_add_cnt = 0;
}

void KCX_BT_Emitter::cmd_NameNum() {
    m_bt_name_num = atoi(m_chbuf + 12);
    vector_clear_and_shrink(m_bt_names);
    m_bt_name_cnt = 0;
}

void KCX_BT_Emitter::cmd_MemAddr() {
    m_bt_addr.push_back(x_ps_strdup(m_chbuf + 14));
    if (kcx_bt_info) kcx_bt_info(m_chbuf, "");
    m_bt_add_cnt++;
}

void KCX_BT_Emitter::cmd_MemName() {
    m_bt_names.push_back(x_ps_strdup(m_chbuf + 12));
    if (kcx_bt_info) kcx_bt_info(m_chbuf, "");
    m_bt_name_cnt++;
}

void KCX_BT_Emitter::cmd_scannedItems() {
    bool f_insert = true;
    for (uint i = 0; i < m_bt_scannedItems.size(); i++) {
        if (strcmp(m_bt_scannedItems[i], m_chbuf) == 0) { f_insert = false; }
    }
    if (f_insert) {
        m_bt_scannedItems.insert(m_bt_scannedItems.begin(), x_ps_strdup(m_chbuf));
        if (kcx_bt_info) kcx_bt_info("scanned:", m_bt_scannedItems[0]);
        const char* s = stringifyScannedItems();
        if (kcx_bt_scanItems) kcx_bt_scanItems(s);
    }
}
void KCX_BT_Emitter::cmd_connectedName() {
    if (kcx_bt_info) kcx_bt_info("my name:", m_chbuf + 8);
    if (m_myName) {
        free(m_myName);
        m_myName = NULL;
    }
    m_myName = x_ps_strdup(m_chbuf + 8);
}

void KCX_BT_Emitter::cmd_connectedAddr() {
    if (kcx_bt_info) kcx_bt_info("my MAC addr:", m_chbuf + 7);
}

void KCX_BT_Emitter::cmd_statePause() {
    m_f_bt_state = BT_PAUSE;
    if (kcx_bt_info) kcx_bt_info("state:", "PLAY -> PAUSE");
}

void KCX_BT_Emitter::cmd_statePlay() {
    m_f_bt_state = BT_PLAY;
    if (kcx_bt_info) kcx_bt_info("state:", "PAUSE -> PLAY");
}

void KCX_BT_Emitter::cmd_ScanMode() {
    if (m_f_scan == true) return;
    if (kcx_bt_info) kcx_bt_info("Status ->", "Scan...");
    m_f_scan = true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::addQueueItem(const char* item) {
    if (!item) return; // guard
    m_messageQueue.insert(m_messageQueue.begin(), item);
}

ps_ptr<char> KCX_BT_Emitter::getQueueItem() {
    ps_ptr<char> queue_item;
    if (m_messageQueue.size() == 0) return queue_item;
    queue_item = m_messageQueue[m_messageQueue.size() - 1];
    KCX_LOG_ERROR("%s", queue_item.c_get());
    m_messageQueue.pop_back();
    return queue_item;
}

// -------------------------- user commands ----------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::deleteVMlinks() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    addQueueItem("AT+DELVMLINK");
}
void KCX_BT_Emitter::getVMlinks() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    addQueueItem("AT+VMLINK?");
}
void KCX_BT_Emitter::addLinkName(const char* name) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    sprintf(m_chbuf, "AT+ADDLINKNAME=%s", name);
    addQueueItem(m_chbuf);
}
void KCX_BT_Emitter::addLinkAddr(const char* addr) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    sprintf(m_chbuf, "AT+ADDLINKADD=%s", addr);
    addQueueItem(m_chbuf);
}
void KCX_BT_Emitter::setVolume(uint8_t vol) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (vol > 31) { vol = 31; }
    sprintf(m_chbuf, "AT+VOL=%d", vol);
    addQueueItem(m_chbuf);
}

const char* KCX_BT_Emitter::getMode() { // returns RX or TX
    if (m_f_bt_mode == BT_MODE_RECEIVER) return ("RX");
    return ("TX");
}

void KCX_BT_Emitter::changeMode() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    digitalWrite(BT_EMITTER_MODE, !m_f_bt_mode);
    addQueueItem("AT+RESET");
}

void KCX_BT_Emitter::setMode(const char* mode) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    if (!strcmp(mode, "RX")) {
        digitalWrite(BT_EMITTER_MODE, LOW);
        addQueueItem("AT+RESET");
        m_f_bt_mode = BT_MODE_RECEIVER;
    }
    if (!strcmp(mode, "TX")) {
        digitalWrite(BT_EMITTER_MODE, HIGH);
        addQueueItem("AT+RESET");
        m_f_bt_mode = BT_MODE_EMITTER;
    }
}

void KCX_BT_Emitter::pauseResume() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    addQueueItem("AT+PAUSE");
}

void KCX_BT_Emitter::downvolume() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    int v = m_bt_volume;
    v--;
    if (v < 0) {
        v = 0;
        warning("The volume is already 0");
    }
    sprintf(m_chbuf, "AT+VOL=%02d", v);
    addQueueItem(m_chbuf);
}

void KCX_BT_Emitter::upvolume() {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    int v = m_bt_volume;
    v++;
    if (v > 31) {
        v = 31;
        warning("The maximum volume has been reached");
    }
    sprintf(m_chbuf, "AT+VOL=%02d", v);
    addQueueItem(m_chbuf);
}

const char* KCX_BT_Emitter::getMyName() {
    if (!m_f_KCX_BT_Emitter_isInit) return "";
    return m_myName;
}

void KCX_BT_Emitter::userCommand(const char* cmd) {
    if (!m_f_KCX_BT_Emitter_isInit) return;
    addQueueItem(cmd);
}
// -------------------------- JSON relevant ----------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::stringifyMemItems() {
    // "AT+VMLINK" returns:
    // "OK+VMLINK"
    // "BT_ADD_NUM=01"              --> save in btAddNum
    // "BT_NAME_NUM=01"             --> save in btNameNum
    // "MEM_Name 00:MyName"         --> save in _KCX_BT_names vector
    // "MEM_MacAdd 00:82435181cc6a" --> save in _KCX_BT_addr vector
    uint16_t JSONstrLength = 0;
    if (m_jsonMemItemsStr) {
        free(m_jsonMemItemsStr);
        m_jsonMemItemsStr = NULL;
    }
    if (m_f_PSRAMfound) {
        m_jsonMemItemsStr = (char*)ps_malloc(2);
    } else {
        m_jsonMemItemsStr = (char*)malloc(2);
    }
    JSONstrLength += 2;
    memcpy(m_jsonMemItemsStr, "[\0", 2);
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6a"},{....}]
    for (int i = 0; i < 10; i++) {
        int a = 0, b = 0;
        if (m_bt_names.size() > i) a = strlen(m_bt_names[i]);
        if (m_bt_addr.size() > i) b = strlen(m_bt_addr[i]);
        JSONstrLength += 22 + a + b; // {"name":"a","addr":"b"},
        if (m_f_PSRAMfound) {
            m_jsonMemItemsStr = (char*)ps_realloc(m_jsonMemItemsStr, JSONstrLength);
        } else {
            m_jsonMemItemsStr = (char*)realloc(m_jsonMemItemsStr, JSONstrLength);
        }
        strcat(m_jsonMemItemsStr, "{\"name\":\"");
        if (a)
            strcat(m_jsonMemItemsStr, m_bt_names[i]);
        else
            strcat(m_jsonMemItemsStr, "");
        strcat(m_jsonMemItemsStr, "\",\"addr\":\"");
        if (b)
            strcat(m_jsonMemItemsStr, m_bt_addr[i]);
        else
            strcat(m_jsonMemItemsStr, "");
        strcat(m_jsonMemItemsStr, "\"},");
    }
    m_jsonMemItemsStr[JSONstrLength - 2] = ']';  // replace comma by square bracket close
    m_jsonMemItemsStr[JSONstrLength - 1] = '\0'; // and terminate
    if (kcx_bt_memItems) kcx_bt_memItems(m_jsonMemItemsStr);
    return;
}

const char* KCX_BT_Emitter::stringifyScannedItems() { // returns the last three scanned BT devices as jsonStr
    if (!m_f_KCX_BT_Emitter_isInit) return "";
    uint16_t JSONstrLength = 0;
    if (m_jsonScanItemsStr) {
        free(m_jsonScanItemsStr);
        m_jsonScanItemsStr = NULL;
    }
    JSONstrLength += 2;
    if (m_f_PSRAMfound) {
        m_jsonScanItemsStr = (char*)ps_malloc(2);
    } else {
        m_jsonScanItemsStr = (char*)malloc(2);
    }
    memcpy(m_jsonScanItemsStr, "[\0", 2);
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6b"},{name":"btthirdName","addr":"82435181cc6c"}]
    for (int i = 0; i < 3; i++) {
        int a = 0, idx1 = 0, idx2 = 0;
        if (m_bt_scannedItems.size() > i) {
            a = strlen(m_bt_scannedItems[i]) - 13; // MacAdd:82435181cc6a,Name:VHM-314
            if (a < 12) {
                KCX_LOG_ERROR("wrong scanned items %s", m_bt_scannedItems[i]);
                return "null";
            } // can't be, MacAddr must have 12 chars
            idx1 = indexOf(m_bt_scannedItems[i], ":", 0);
            idx2 = indexOf(m_bt_scannedItems[i], ",", idx1);
        }
        JSONstrLength += 22 + a; // {"name":"a","addr":"b"},
        if (m_f_PSRAMfound) {
            m_jsonScanItemsStr = (char*)ps_realloc(m_jsonScanItemsStr, JSONstrLength);
        } else {
            m_jsonScanItemsStr = (char*)realloc(m_jsonScanItemsStr, JSONstrLength);
        }
        strcat(m_jsonScanItemsStr, "{\"addr\":\"");
        if (a)
            strncat(m_jsonScanItemsStr, m_bt_scannedItems[i] + idx1 + 1, idx2 - idx1 - 1);
        else
            strcat(m_jsonScanItemsStr, "");
        strcat(m_jsonScanItemsStr, "\",\"name\":\"");
        if (a)
            strcat(m_jsonScanItemsStr, m_bt_scannedItems[i] + idx2 + 1 + 5);
        else
            strcat(m_jsonScanItemsStr, "");
        strcat(m_jsonScanItemsStr, "\"},");
    }
    m_jsonScanItemsStr[JSONstrLength - 2] = ']';  // replace comma by square bracket close
    m_jsonScanItemsStr[JSONstrLength - 1] = '\0'; // and terminate
    return m_jsonScanItemsStr;
}

void KCX_BT_Emitter::protocol_addElement(const char* RX_TX, const char* str) {
    if (!str) return;
    ps_ptr<char> buff;
    buff.assign(RX_TX);
    buff.append(": ");
    buff.append(str);
    m_RX_TX_protocol.push_back(buff);
    if (m_RX_TX_protocol.size() > 200) { m_RX_TX_protocol.erase(m_RX_TX_protocol.begin()); } // remove the last element
}

const char* KCX_BT_Emitter::list_protokol(uint16_t elementNr) {
    if (elementNr >= m_RX_TX_protocol.size()) return NULL;
    return m_RX_TX_protocol[elementNr].c_get();
}

//-------------------------interrupt handling --------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::isr0() {
    objPtr->handleInterrupt();
}

void KCX_BT_Emitter::t1s() {
    tckPtr->handleTicker();
}

KCX_BT_Emitter* KCX_BT_Emitter::objPtr;
KCX_BT_Emitter* KCX_BT_Emitter::tckPtr;

void KCX_BT_Emitter::handleInterrupt() {
    m_f_linkChanged = true;
}

void KCX_BT_Emitter::handleTicker() {
    m_f_ticker1s = true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
