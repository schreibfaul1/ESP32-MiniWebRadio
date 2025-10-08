/*
 *  KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 26.04.2025
 *      Author: Wolle
 */

#include "kcx_bt_emitter.h"

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
KCX_BT_Emitter::KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin) {
    BT_MODE_PIN = mode_pin;
    BT_LINK_PIN = link_pin;
    BT_RX_PIN = RX_pin;
    BT_TX_PIN = TX_pin;
    if (BT_MODE_PIN < 0 || BT_LINK_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    pinMode(BT_LINK_PIN, INPUT_PULLUP);
    pinMode(BT_MODE_PIN, OUTPUT);
    digitalWrite(BT_MODE_PIN, LOW);
    m_f_bt_inUse = false;
}

KCX_BT_Emitter::~KCX_BT_Emitter() {}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::begin() {
    KCX_LOG_WARN("begin");
    Serial2.begin(115200, SERIAL_8N1, BT_TX_PIN, BT_RX_PIN);
    add_tx_queue_item("AT+");
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::loop() {
    if (m_TX_queue.size()) {
        if (!m_f_bt_inUse) {
            m_f_bt_inUse = true;
            writeCommand(get_tx_queue_item());
            m_timeStamp = millis();
        }
    }
    if (Serial2.available()) readCmd();

    if (m_RX_queue.size()) {
        if (m_f_bt_inUse) {
            m_f_bt_inUse = false;
            parseATcmds();
        }
    }

    if (m_f_bt_inUse) {
        if (m_timeStamp + 3000 < millis()) {
            KCX_LOG_ERROR("timeout while waiting for response");
            m_f_bt_inUse = false;
        }
    }
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::readCmd() {

    uint32_t     t = millis() + 500;
    uint16_t     idx = 0;
    uint8_t      ch = 0;
    uint16_t     buff_size = 1024;
    ps_ptr<char> buff;
    buff.calloc(buff_size);

    auto process_message = [&](ps_ptr<char> msg, uint16_t len) {
        ps_ptr<char>extracted_message;
        if(len < 3) return; // min OK+
        int start = 0;
        for (int i = 0; i < len; i++) {
            if (msg[i] < 32) msg[i] = '\0';
            if (msg[i] > 127) msg[i] = '\0';
        }
        for(int i = len - 2; i >= 0; i--){
            if(msg[i] == '\0') {start = i + 1; break;}
        }
        extracted_message.copy_from(msg.get() + start);
        if(extracted_message.strlen() < 3) return;
        protocol_addElement("RX", extracted_message.c_get());
        add_rx_queue_item(extracted_message);
        m_last_rx_command = extracted_message;
        KCX_LOG_WARN("readCmd %s", extracted_message.c_get());
        return;
    };

    while (true) {
        if (t < millis()) {
            KCX_LOG_ERROR("timeout while reading from KCX_BT_Emitter, received: %s", buff.c_get());
            return;
        }
        if(ch == '\n'){
            process_message(buff, idx);
            buff.clear();
            idx = 0;
            ch = '\0';
        }

        if (!Serial2.available()) {
            if (ch == '\0') { break; }
            vTaskDelay(10);
            continue;
        }
        ch = Serial2.read();
        if (ch == '\r') continue;
        if (ch == '\0') ch = 255;
        buff[idx] = ch;

        idx++;
        if (idx == buff_size) break;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::writeCommand(ps_ptr<char> cmd) {
    protocol_addElement("TX", cmd.c_get());
    Serial2.printf("%s%s", cmd.c_get(), "\r\n");
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::parseATcmds() {
    ps_ptr<char> item;
    item = get_rx_queue_item();
    KCX_LOG_ERROR("%s", item.c_get());

    if (item.equals("OK+")) {
        if (kcx_bt_info) kcx_bt_info("KCX_BT_Emitter found", "");
        m_TX_queue.push_back("AT+BT_MODE?"); // transmitter or receiver
        m_TX_queue.push_back("AT+VOL?");     // get volume (in receiver mode 0 ... 31)
        m_TX_queue.push_back("AT+GMR?");     // get version
    }
    if (item.equals("OK+RESET")) {
        if (kcx_bt_info) kcx_bt_info("Reset", "");
    }
    if (item.equals("POWER ON")) {
        if (kcx_bt_info) kcx_bt_info("Power On", "");
    }
    if (item.starts_with("OK+VERS:")) { // OK+VERS:KCX_BT_RTX_V1.4
        m_version.copy_from(item.get() + 8);
        if (kcx_bt_info) kcx_bt_info("Version", m_version.c_get());
    }
    if (item.starts_with("OK+BT_")) { // OK+BT_EMITTER or OK+BT_RECEIVER
        if (item.ends_with("EMITTER")) {
            m_f_bt_mode = BT_MODE_EMITTER;
            if (kcx_bt_info) kcx_bt_info("Mode", "EMITTER");
        }
        if (item.ends_with("RECEIVER")) {
            m_f_bt_mode = BT_MODE_RECEIVER;
            if (kcx_bt_info) kcx_bt_info("Mode", "RECEIVER");
        }
    }
    if (item.starts_with("OK+VOL=")) {
        m_bt_volume = atoi(item.get() + 7); // OK+VOL=31
        if (kcx_bt_info) kcx_bt_info("Volume", item.get() + 7);
    }
    if (item.starts_with("OK+NAME=")) {
        if (kcx_bt_info) kcx_bt_info("Name", item.get() + 8);
    }
    if (item.starts_with("Delete_Vmlink")) {
        if (kcx_bt_info) kcx_bt_info("Vmlink", "deleted");
    }
    if (item.starts_with("MEM_Name")) {
        m_bt_names.push_back(item.get() + 12);
        if (kcx_bt_info) kcx_bt_info("MEM_Name", item.get() + 9);
    }
    if (item.starts_with("MEM_MacAdd")) {
        m_bt_names.push_back(item.get() + 14);
        if (kcx_bt_info) kcx_bt_info("MEM_MacAdd", item.get() + 11);
    }

    m_f_bt_inUse = false;
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::protocol_addElement(const char* RX_TX, const char* str) {
    if (!str) return;
    ps_ptr<char> buff;
    buff.assign(RX_TX);
    buff.append(": ");
    buff.append(str);
    m_RX_TX_protocol.push_back(buff);
    if (m_RX_TX_protocol.size() > 200) { m_RX_TX_protocol.erase(m_RX_TX_protocol.begin()); } // remove the last element
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::add_tx_queue_item(ps_ptr<char> item) {
    m_TX_queue.insert(m_TX_queue.begin(), item);
    KCX_LOG_WARN("%s", item.c_get());
    return;
}

ps_ptr<char> KCX_BT_Emitter::get_tx_queue_item() {
    ps_ptr<char> queue_item;
    if (m_TX_queue.size() == 0) return queue_item;
    queue_item = m_TX_queue[m_TX_queue.size() - 1];
    KCX_LOG_DEBUG("%s", queue_item.c_get());
    m_TX_queue.pop_back();
    return queue_item;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::add_rx_queue_item(ps_ptr<char> item) {
    m_RX_queue.insert(m_RX_queue.begin(), item);
    return;
}

ps_ptr<char> KCX_BT_Emitter::get_rx_queue_item() {
    ps_ptr<char> queue_item;
    if (m_RX_queue.size() == 0) return "";
    queue_item = m_RX_queue[m_RX_queue.size() - 1];
    KCX_LOG_DEBUG("%s", queue_item.c_get());
    m_RX_queue.pop_back();
    return queue_item;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::deleteVMlinks() {
    add_tx_queue_item("AT+DELVMLINK");
}
void KCX_BT_Emitter::getVMlinks() {
    add_tx_queue_item("AT+VMLINK?");
}
void KCX_BT_Emitter::addLinkName(ps_ptr<char> name) {
    name.insert("AT+ADDLINKNAME=", 0);
    add_tx_queue_item(name);
}
void KCX_BT_Emitter::addLinkAddr(ps_ptr<char> addr) {
    addr.insert("AT+ADDLINKADD=", 0);
}
void KCX_BT_Emitter::setVolume(uint8_t vol) {
    if (vol > 31) { vol = 31; }
    ps_ptr<char> v = "AT+VOL=";
    v.appendf("%i", vol);
    add_tx_queue_item(v);
}
const char* KCX_BT_Emitter::getMode() { // returns RX or TX
    if (m_f_bt_mode == BT_MODE_RECEIVER) return ("RX");
    return ("TX");
}
void KCX_BT_Emitter::changeMode() {
    digitalWrite(BT_MODE_PIN, !m_f_bt_mode);
    add_tx_queue_item("AT+RESET");
}
void KCX_BT_Emitter::setMode(btmode mode) {
    if (mode == BT_MODE_RECEIVER)
        m_f_bt_mode = BT_MODE_RECEIVER;
    else
        m_f_bt_mode = BT_MODE_EMITTER;
    add_tx_queue_item("AT+RESET");
}
void KCX_BT_Emitter::pauseResume() {
    add_tx_queue_item("AT+PAUSE");
}
void KCX_BT_Emitter::downvolume() {
    if (m_bt_volume == 0) {
        KCX_LOG_WARN("The volume is already 0");
        return;
    }
    m_bt_volume--;
    ps_ptr<char> v = "AT+VOL=";
    v.appendf("%02d", m_bt_volume);
    add_tx_queue_item(v);
}
void KCX_BT_Emitter::upvolume() {
    if (m_bt_volume == 31) {
        KCX_LOG_WARN("The maximum volume has been reached");
        return;
    }
    m_bt_volume++;
    ps_ptr<char> v = "AT+VOL=";
    v.appendf("%02d", m_bt_volume);
    add_tx_queue_item(v);
}
const char* KCX_BT_Emitter::getMyName() {
    return m_myName;
}
void KCX_BT_Emitter::cmd_PowerOff() {
    KCX_LOG_INFO("POWER OFF", "");
    add_tx_queue_item("AT+POWER_OFF");
}
void KCX_BT_Emitter::cmd_PowerOn() {
    KCX_LOG_INFO("POWER ON", "");
    add_tx_queue_item("AT+BT_MODE?");
}
void KCX_BT_Emitter::userCommand(const char* cmd) {
    add_tx_queue_item(cmd);
}
const char* KCX_BT_Emitter::list_protokol() {
    for (auto& item : m_RX_TX_protocol) { KCX_LOG_ERROR("RX_TX_protocol %s", item.c_get()); }
}
// -------------------------- JSON relevant ----------------------------------------------------------------------------------------------------------
void KCX_BT_Emitter::stringifyMemItems() {
    // "AT+VMLINK" returns:
    // "OK+VMLINK"
    // "BT_ADD_NUM=01"              --> save in btAddNum
    // "BT_NAME_NUM=01"             --> save in btNameNum
    // "MEM_Name 00:MyName"         --> save in _KCX_BT_names vector
    // "MEM_MacAdd 00:82435181cc6a" --> save in _KCX_BT_addr vector

    for (auto& item : m_bt_names) { KCX_LOG_ERROR("RX_TX_protocol %s", item.c_get()); }

    m_jsonMemItemsStr.assign("[");
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6a"},{....}]
    for (int i = 0; i < 10; i++) {
        m_jsonMemItemsStr.append("{\"name\":\"");
        if (m_bt_names.size() > i) {
            m_jsonMemItemsStr.append(m_bt_names[i].c_get());
        } else {
            m_jsonMemItemsStr.append("");
        }
        m_jsonMemItemsStr.append("\",\"addr\":\"");
        if (m_bt_names.size() > i) {
            m_jsonMemItemsStr.append(m_bt_addr[i].c_get());
        } else {
            m_jsonMemItemsStr.append("");
        }
        m_jsonMemItemsStr.append("\"},");
    }
    int posComma = m_jsonMemItemsStr.last_index_of(',');
    m_jsonMemItemsStr[posComma] = ']'; // replace comma by square bracket close
    if (kcx_bt_memItems) kcx_bt_memItems(m_jsonMemItemsStr.c_get());
    return;
}

const char* KCX_BT_Emitter::stringifyScannedItems() { // returns the last three scanned BT devices as jsonStr

    for (auto& item : m_bt_scannedItems) { KCX_LOG_ERROR("RX_TX_protocol %s", item.c_get()); }

    m_jsonScanItemsStr.assign("[");
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6b"},{name":"btthirdName","addr":"82435181cc6c"}]
    int idx1, idx2;
    for (int i = 0; i < 3; i++) {
        if (m_bt_scannedItems.size() > i) {
            idx1 = m_bt_scannedItems[i].index_of(':');
            idx2 = m_bt_scannedItems[i].index_of(',');
            // if (m_bt_scannedItems[i].strlen() < 13) { // MacAdd:82435181cc6a,Name:VHM-314
            //     KCX_LOG_ERROR("wrong scanned items %s", m_bt_scannedItems[i]);
            //     return "null";
            // } // can't be, MacAddr must have 12 chars

            m_jsonScanItemsStr.append("{\"addr\":\"");
            m_jsonScanItemsStr.append(m_bt_scannedItems[i].substr(idx1, idx2 - idx1).c_get());

        } else {
            m_jsonScanItemsStr.append("{\"addr\":\"\",\"name\":\"\"},");
        }
    }
    int posComma = m_jsonScanItemsStr.last_index_of(',');
    m_jsonScanItemsStr[posComma] = ']'; // and terminate
    KCX_LOG_ERROR("%s", m_jsonScanItemsStr.c_get());
    return m_jsonScanItemsStr.c_get();
}
