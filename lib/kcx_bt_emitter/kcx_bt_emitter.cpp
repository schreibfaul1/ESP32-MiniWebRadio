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

    if (millis() > m_timeStamp + 2000) m_lock = false;

    if (!m_lock) {
        m_lock = true;
        m_timeStamp = millis();
        if (m_TX_queue.size()) {
            writeCommand(get_tx_queue_item());
            return;
        }
    }
    if (Serial2.available()) readCmd();
    if (m_RX_queue.size()) {
        parseATcmds();
        m_lock = false;
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
        ps_ptr<char> extracted_message;
        if (len < 3) return; // min OK+
        int start = 0;
        for (int i = 0; i < len; i++) {
            if (msg[i] < 32) msg[i] = '\0';
            if (msg[i] > 127) msg[i] = '\0';
        }
        for (int i = len - 2; i >= 0; i--) {
            if (msg[i] == '\0') {
                start = i + 1;
                break;
            }
        }
        extracted_message.copy_from(msg.get() + start);
        if (extracted_message.strlen() < 3) return;
        if (!extracted_message.is_utf8()) return;
        protocol_addElement("RX", extracted_message.c_get());
        add_rx_queue_item(extracted_message);
        m_last_rx_command = extracted_message;
        KCX_LOG_INFO("readCmd %s", extracted_message.c_get());
        return;
    };

    while (true) {
        if (t < millis()) {
            KCX_LOG_ERROR("timeout while reading from KCX_BT_Emitter, received: %s", buff.c_get());
            return;
        }
        if (ch == '\n') {
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
    KCX_LOG_DEBUG("writeCmd %s", cmd.c_get());
    Serial2.printf("%s%s", cmd.c_get(), "\r\n");
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::parseATcmds() {
    ps_ptr<char> item;
    item = get_rx_queue_item();
    KCX_LOG_DEBUG("%s", item.c_get());

    if (item.equals("OK+")) {
        if (kcx_bt_info) kcx_bt_info("KCX_BT_Emitter found", "");
        add_tx_queue_item("AT+GMR?"); // get version
        vTaskDelay(1000);
        add_tx_queue_item("AT+BT_MODE?"); // transmitter or receiver
    }
    if (item.equals("OK+RESET")) {
        m_f_status = BT_NOT_CONNECTED;
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
            if (kcx_bt_modeChanged) kcx_bt_modeChanged("TX");
        }
        if (item.ends_with("RECEIVER")) {
            m_f_bt_mode = BT_MODE_RECEIVER;
            if (kcx_bt_info) kcx_bt_info("Mode", "RECEIVER");
            if (kcx_bt_modeChanged) kcx_bt_modeChanged("RX");
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
    if (item.starts_with("MEM_Name")) { // MEM_Name 00: Pebble V3
        auto num = item.substr(9, 2);
        auto name = item.substr(12);
        m_MEM_Name[num.to_uint32()] = name;
        if (kcx_bt_info) kcx_bt_info("MEM_Name", item.get() + 9);
        stringifyMemItems();
    }
    if (item.starts_with("MEM_MacAdd")) {
        auto num = item.substr(11, 2);
        auto addr = item.substr(14);
        m_MEM_MacAdd[num.to_uint32()] = addr;
        if (kcx_bt_info) kcx_bt_info("MEM_MacAdd", item.get() + 11);
        stringifyMemItems();
    }
    if (item.starts_with("MacAdd")) {
        bool found = false;
        for (auto sc : m_bt_scannedItems) {
            KCX_LOG_ERROR("item %s", item.c_get());
            if (item.equals(sc)) { found = true; }
        }
        if (!found) { m_bt_scannedItems.push_back(item); }
    }
    if (item.starts_with("CONNECT=>")) {
        m_f_status = BT_CONNECTED;
        if (kcx_bt_info) kcx_bt_info("Status ->", "Connected");
    }
    if (item.equals("SCAN....")) {
        m_f_status = BT_NOT_CONNECTED;
        if (kcx_bt_info) kcx_bt_info("Status ->", "Disconnected");
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
    m_TX_queue.push_back(item);
    KCX_LOG_ERROR("add_tx_queue_item %s", item.c_get());
    return;
}

ps_ptr<char> KCX_BT_Emitter::get_tx_queue_item() {
    ps_ptr<char> queue_item;
    if (m_TX_queue.size() == 0) return queue_item;
    queue_item = m_TX_queue[0];
    KCX_LOG_WARN("get_tx_queue_item %s", queue_item.c_get());
    m_TX_queue.pop_front();
    return queue_item;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::add_rx_queue_item(ps_ptr<char> item) {
    m_RX_queue.push_front(item);
    return;
}

ps_ptr<char> KCX_BT_Emitter::get_rx_queue_item() {
    ps_ptr<char> queue_item;
    if (m_RX_queue.size() == 0) return "";
    queue_item = m_RX_queue[0];
    KCX_LOG_DEBUG("%s", queue_item.c_get());
    m_RX_queue.pop_front();
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
    add_tx_queue_item(addr);
}
void KCX_BT_Emitter::setVolume(uint8_t vol) {
    if (vol > 31) { vol = 31; }
    ps_ptr<char> v = "AT+VOL=";
    v.appendf("%i", vol);
    add_tx_queue_item(v);
}
bool KCX_BT_Emitter::getMode() { // returns RX or TX
    return m_f_bt_mode;
}
void KCX_BT_Emitter::setMode(bool mode) {
    if (mode == BT_MODE_RECEIVER) {
        m_f_bt_mode = BT_MODE_RECEIVER;
        digitalWrite(BT_MODE_PIN, LOW);
    } else {
        m_f_bt_mode = BT_MODE_EMITTER;
        digitalWrite(BT_MODE_PIN, HIGH);
    }
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
    return "";
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::stringifyMemItems() {
    // "AT+VMLINK" returns:
    // "OK+VMLINK"
    // "BT_ADD_NUM=01"              --> save in btAddNum
    // "BT_NAME_NUM=01"             --> save in btNameNum
    // "MEM_Name 00:MyName"         --> save in _KCX_BT_names vector
    // "MEM_MacAdd 00:82435181cc6a" --> save in _KCX_BT_addr vector

    for (auto& item : m_MEM_MacAdd) { KCX_LOG_DEBUG("RX_TX_protocol %s", item.c_get()); }
    for (auto& item : m_MEM_Name) { KCX_LOG_DEBUG("RX_TX_protocol %s", item.c_get()); }

    m_jsonMemItemsStr.assign("[");
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6a"},{....}]
    for (int i = 0; i < 10; i++) {
        m_jsonMemItemsStr.append("{\"name\":\"");
        m_jsonMemItemsStr.append(m_MEM_Name[i].c_get());
        m_jsonMemItemsStr.append("\",\"addr\":\"");
        m_jsonMemItemsStr.append(m_MEM_MacAdd[i].c_get());
        m_jsonMemItemsStr.append("\"},");
    }
    int posComma = m_jsonMemItemsStr.last_index_of(',');
    m_jsonMemItemsStr[posComma] = ']'; // replace comma by square bracket close
    KCX_LOG_DEBUG("%s", m_jsonMemItemsStr.c_get());
    if (kcx_bt_memItems) kcx_bt_memItems(m_jsonMemItemsStr.c_get());
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* KCX_BT_Emitter::stringifyScannedItems() { // returns the last three scanned BT devices as jsonStr

    for (auto& item : m_bt_scannedItems) { KCX_LOG_ERROR("RX_TX_protocol %s", item.c_get()); }

    m_jsonScanItemsStr.assign("[");
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6b"},{name":"btthirdName","addr":"82435181cc6c"}]
    int idx1, idx2, idx3;
    for (int i = 0; i < m_bt_scannedItems.size(); i++) {
        KCX_LOG_WARN("m_bt_scannedItems[i]   %s", m_bt_scannedItems[i].c_get());
        idx1 = m_bt_scannedItems[i].index_of(':');
        idx2 = m_bt_scannedItems[i].index_of(',');
        idx3 = m_bt_scannedItems[i].last_index_of(':');
        if (idx1 < 0 || idx2 < 0 || idx3 <= idx1) {
            m_jsonScanItemsStr.append("{\"addr\":\"\",\"name\":\"\"},");
            continue;
        }
        ps_ptr<char> addr = m_bt_scannedItems[i].substr(idx1 + 1, idx2 - idx1 - 1).c_get();
        ps_ptr<char> name = m_bt_scannedItems[i].substr(idx3 + 1).c_get();
        if (addr.strlen() < 12) { // MacAdd:82435181cc6a,Name:VHM-314
            m_jsonScanItemsStr.append("{\"addr\":\"\",\"name\":\"\"},");
            continue;
        }
        m_jsonScanItemsStr.append("{\"addr\":\"");
        m_jsonScanItemsStr.append(addr.c_get());
        m_jsonScanItemsStr.append("\",\"name\":\"");
        m_jsonScanItemsStr.append(name.c_get());
        m_jsonScanItemsStr.append("\"},");
    }
    if (!m_bt_scannedItems.size()) m_jsonScanItemsStr.append("{\"addr\":\"\",\"name\":\"\"},");
    int posComma = m_jsonScanItemsStr.last_index_of(',');
    m_jsonScanItemsStr[posComma] = ']'; // and terminate
    KCX_LOG_ERROR("%s", m_jsonScanItemsStr.c_get());
    return m_jsonScanItemsStr.c_get();
}
