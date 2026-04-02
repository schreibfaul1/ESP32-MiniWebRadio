/*
 *  KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 27.03.2026
 *      Author: Wolle
 */

#include "kcx_bt_emitter.h"

__attribute__((weak)) void kcx_bt_memItems(const char*) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

__attribute__((weak)) void kcx_bt_scanItems(const char*) {
    // Default: do nothing. User can provide their own implementation to process audio data.
}

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
KCX_BT_Emitter::KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t connect_pin, int8_t mode_pin) {
    BT_MODE_PIN = mode_pin;       // low RX, high TX
    BT_CONNECT_PIN = connect_pin; // 100ms low -> awake from POWER_OFF
    BT_RX_PIN = RX_pin;
    BT_TX_PIN = TX_pin;
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    pinMode(BT_CONNECT_PIN, OUTPUT);
    pinMode(BT_MODE_PIN, OUTPUT);
    digitalWrite(BT_MODE_PIN, LOW);
    m_mode_pin = LOW;
    m_bt_found = false;
}

KCX_BT_Emitter::~KCX_BT_Emitter() {}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::begin() {
    KCX_LOG_DEBUG("KCX_BT_Emitter begin");
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    Serial2.begin(115200, SERIAL_8N1, BT_TX_PIN, BT_RX_PIN);
    add_tx_queue_item("AT+");
    m_bt_found = false;
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::decrement_timer() {

    if (m_tx_process_timer == 1) {
        KCX_LOG_WARN("timeout, last command was: '%s'", m_last_tx_command.c_get());
        m_last_tx_command = "";
    }

    if (m_powerOn_in_progress == 1) { KCX_LOG_WARN("timeout in powerOn"); }
    if (m_powerOff_in_progress == 1) { KCX_LOG_WARN("timeout in powerOff"); }

    if (m_powerOn_in_progress > 0) m_powerOn_in_progress--;
    if (m_powerOff_in_progress > 0) m_powerOff_in_progress--;
    if (m_tx_process_timer > 0) m_tx_process_timer--;
    if (m_setMode_in_progress > 0) m_setMode_in_progress--;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool KCX_BT_Emitter::compare_request(ps_ptr<char> answer) {
    bool result = false;
    bool extended_answer = false;
    if (m_last_tx_command.equals("AT+") && answer.equals("OK+")) result = true;
    if (m_last_tx_command.starts_with("AT+POWER_ON") && answer.starts_with("POWER ON")) result = true;
    if (m_last_tx_command.starts_with("AT+RESET") && answer.starts_with("POWER ON")) result = true;
    if (m_last_tx_command.starts_with("AT+GMR") && answer.starts_with("OK+VERS")) result = true;
    if (m_last_tx_command.starts_with("AT+BT_MODE") && answer.starts_with("OK+BT")) result = true;
    if (m_last_tx_command.starts_with("AT+VOL") && answer.starts_with("OK+VOL")) result = true;
    if (m_last_tx_command.starts_with("AT+NAME") && answer.starts_with("OK+NAME")) result = true;
    if (m_last_tx_command.starts_with("AT+NAME") && answer.starts_with("Not in receiver mode!")) result = true;
    if (m_last_tx_command.starts_with("AT+DELVMLINK") && answer.starts_with("Delete_Vmlink")) result = true;
    if (m_last_tx_command.starts_with("AT+VMLINK") && answer.starts_with("OK+VMLINK")) result = true;
    if (m_last_tx_command.starts_with("AT+VMLINK") && answer.starts_with("Not in emitter mode!")) result = true;
    if (m_last_tx_command.starts_with("AT+ADDLINKNAME") && answer.starts_with("OK+ADDLINKNAME")) result = true;
    if (m_last_tx_command.starts_with("AT+ADDLINKADD") && answer.starts_with("OK+ADDLINKADD")) result = true;
    if (m_last_tx_command.starts_with("AT+STATUS") && answer.starts_with("OK+STATUS")) result = true;
    if (m_last_tx_command.starts_with("AT+PAUSE") && answer.starts_with("OK+PAUSE")) result = true;
    if (m_last_tx_command.starts_with("AT+PAUSE") && answer.starts_with("OK+PLAY")) result = true;
    if (m_last_tx_command.starts_with("AT+PAUSE") && answer.starts_with("no connect!")) result = true;
    if (m_last_tx_command.starts_with("AT+POWER_OFF") && answer.starts_with("OK+POWEROFF_MODE")) result = true;
    if (m_last_tx_command.starts_with("AT+POWER_OFF") && answer.starts_with("OK+POWEROFF_MODE")) result = true;
    if (m_last_tx_command.starts_with("AT+POWER_OFF") && answer.starts_with("OK+POWEROFF_MODE")) result = true;

    if (!result) {
        if (answer.starts_with("OK+RESET")) extended_answer = true; // after AT+RESET
        if (answer.starts_with("Auto_link")) extended_answer = true;
        if (answer.starts_with("POWER ON")) extended_answer = true;
        if (answer.starts_with("SCAN")) extended_answer = true;
        if (answer.starts_with("CON MATCH")) extended_answer = true;
        if (answer.starts_with("CON ONE")) extended_answer = true;
        if (answer.starts_with("CON LAST")) extended_answer = true;
        if (answer.starts_with("CONNECT")) extended_answer = true;
        if (answer.starts_with("VM Reset2")) extended_answer = true;     // after AT+RESET
        if (answer.starts_with("Delete_Vmlink")) extended_answer = true; // after AT+RESET
        if (answer.starts_with("BT_ADD")) return true;
        if (answer.starts_with("BT_NAME")) return true;
        if (answer.starts_with("MEM_Name")) return true;
        if (answer.starts_with("MEM_MacAdd")) return true;
        if (answer.starts_with("MacAdd")) return true;
    }
    if (!result && !extended_answer) {
        KCX_LOG_WARN("unknown answer, request: %s, response %s", m_last_tx_command.c_get(), answer.c_get());
        return false;
    }

    if (result) {
        m_last_tx_command = "";
        m_tx_process_timer = 0;
    }

    return result;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::loop() {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;

    if (millis() > m_timeStamp + 1000) {
        m_cnt++;
        m_f_1s = true;
        m_timeStamp = millis();
        if (m_cnt == 10) {
            m_cnt = 0;
            m_f_10s = true;
        }
    }

    if (m_f_1s) {
        m_f_1s = false;
        if (m_TX_queue.size()) {
            ps_ptr<char> cmd = get_tx_queue_item();
            if (cmd.starts_with("AT+MODE_")) {
                setMode_intern(cmd.substr(8));
            } else {
                writeCommand(cmd);
            }
            return;
        }
        decrement_timer();
    }

    if (m_f_10s) {
        m_f_10s = false;
        if (m_f_power) {                // keep alive?
            userCommand("AT+STATUS?");  // nothing in queue? ask for status ==> 0 disconnected, 1 connected
            userCommand("AT+BT_MODE?"); // OK+BT_EMITTER or OK+BT_RECEIVER
        }
    }

    if (Serial2.available()) readCmd();
    if (m_RX_queue.size()) {
        parseATcmds();
        m_f_1s = false;
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
        if (extracted_message.starts_with("OK+STATUS")) return; // do no protokol heart beat
        if (extracted_message.starts_with("OK+BT_")) return;    // do no protokol heart beat
        m_last_rx_command = extracted_message;
        m_msg.e = evt_info;
        m_msg.arg = extracted_message.c_get();
        if (m_bt_callback) { m_bt_callback(m_msg); }
        return;
    };

    while (true) {
        if (t < millis()) {
            if (!buff.is_utf8()) return;
            KCX_LOG_WARN("timeout while reading from KCX_BT_Emitter, received: %s", buff.c_get());
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
    Serial2.printf("%s%s", cmd.c_get(), "\r\n"); // kcx write
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::parseATcmds() {
    ps_ptr<char> item;
    item = get_rx_queue_item();
    KCX_LOG_DEBUG("%s", item.c_get());
    compare_request(item);

    if (item.equals("OK+")) {
        m_bt_found = true;
        m_f_power = true;
        m_msg.e = evt_found;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("OK+RESET")) {
        m_f_connected = BT_NOT_CONNECTED;
        m_msg.e = evt_disconnect;
        if (m_bt_callback) { m_bt_callback(m_msg); }
        m_msg.e = evt_reset;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("POWER ON")) {
        KCX_LOG_DEBUG("%s", item.c_get());
        m_f_power = true;
        m_powerOn_in_progress = 0;
        // m_setMode_in_progress = 0;
        m_msg.e = evt_power_on;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("OK+POWEROFF_MODE")) {
        m_f_power = false;
        m_powerOff_in_progress = 0;
        m_f_connected = BT_NOT_CONNECTED;
        m_msg.e = evt_power_off;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("OK+VERS:")) { // OK+VERS:KCX_BT_RTX_V1.4
        m_bt_mode = "TX";
        m_ibt_mode = 1;
        m_bt_version.copy_from(item.get() + 8);
        m_msg.arg = m_bt_version.c_get();
        m_msg.e = evt_version;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("OK+BT_EMITTER")) {
        if (!m_bt_mode.equals("TX")) {
            m_bt_mode = "TX";
            m_ibt_mode = 1;
            m_msg.arg = "TX";
            m_msg.e = evt_mode;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("OK+BT_RECEIVER")) {
        if (!m_bt_mode.equals("RX")) {
            m_bt_mode = "RX";
            m_ibt_mode = 0;
            m_msg.arg = "RX";
            m_msg.e = evt_mode;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.starts_with("OK+VOL=")) {
        m_bt_volume = atoi(item.get() + 7); // OK+VOL=31
        m_msg.val = m_bt_volume;
        m_msg.e = evt_volume;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("OK+NAME=")) {
        m_msg.e = evt_info;
        m_msg.arg = ""; // todo
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("Delete_Vmlink")) {
        m_msg.e = evt_info;
        m_msg.arg = "Vmlink deleted";
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("MEM_Name")) { // MEM_Name 00: Pebble V3
        auto num = item.substr(9, 2);
        auto name = item.substr(12);
        m_MEM_Name[num.to_uint32()] = name;
        stringifyMemItems();
        m_msg.e = evt_info;
        m_msg.arg = item.c_get();
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("MEM_MacAdd")) {
        auto num = item.substr(11, 2);
        auto addr = item.substr(14);
        m_MEM_MacAdd[num.to_uint32()] = addr;
        stringifyMemItems();
        m_msg.e = evt_info;
        m_msg.arg = item.c_get();
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.starts_with("MacAdd")) {
        if (m_f_power) {
            bool found = false;
            for (auto sc : m_bt_scannedItems) {
                if (item.equals(sc)) { found = true; }
            }
            if (!found) { m_bt_scannedItems.push_back(item); }
        }
    } else if (item.starts_with("CONNECT=>")) {
        if (m_f_connected == BT_NOT_CONNECTED) {
            m_f_connected = BT_CONNECTED;
            m_msg.e = evt_connect;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("SCAN....")) {
        if (m_f_power) {
            if (m_f_connected == BT_CONNECTED) {
                m_f_connected = BT_NOT_CONNECTED;
                m_msg.e = evt_disconnect;
                if (m_bt_callback) { m_bt_callback(m_msg); }
            }
        }
        m_msg.e = evt_scan;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("OK+STATUS:0")) {
        if (m_f_connected == BT_CONNECTED) {
            m_f_connected = BT_NOT_CONNECTED;
            m_msg.e = evt_disconnect;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("OK+STATUS:1")) {
        if (m_f_connected == BT_NOT_CONNECTED) {
            m_f_connected = BT_CONNECTED;
            m_msg.e = evt_connect;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("CON ONE")) {
        if (m_f_connected == BT_CONNECTED) {
            m_msg.e = evt_connect;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("CON LAST")) {
        if (m_f_connected == BT_CONNECTED) {
            m_msg.e = evt_connect;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
    } else if (item.equals("OK+PAUSE")) {
        m_msg.e = evt_pause;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else if (item.equals("OK+PLAY")) {
        m_msg.e = evt_play;
        if (m_bt_callback) { m_bt_callback(m_msg); }
    } else {
        if (item.starts_with("Auto_link")) {
            ;
        } else if (item.starts_with("BT_NAME")) {
            ;
        } else if (item.starts_with("BT_ADD")) {
            ;
        } else if (item.starts_with("CON MATCH ADD")) {
            ;
        } else if (item.starts_with("OK+VMLINK")) {
            ;
        } else if (item.starts_with("VM Reset2")) {
            ;
        } else if (item.starts_with("no connect!")) {
            ;
        } else if (item.starts_with("Not in receiver mode!")) {
            ;
        } else if (item.starts_with("Not in emitter mode!")) {
            ;
        } else {
            KCX_LOG_WARN("unknown RX command: %s", item.c_get());
        }
    }
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::protocol_addElement(ps_ptr<char> RX_TX, ps_ptr<char> str) {
    if (!str) return;
    ps_ptr<char> buff;
    buff = RX_TX;
    buff.append(": ");
    buff.append(str);
    m_RX_TX_protocol.push_back(buff);
    if (m_RX_TX_protocol.size() > 200) { m_RX_TX_protocol.erase(m_RX_TX_protocol.begin()); } // remove the last element
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool KCX_BT_Emitter::already_in_TX_list(ps_ptr<char> command) {
    bool result = false;
    for (int i = 0; i < m_TX_queue.size(); i++) {
        if (command.equals(m_TX_queue[i])) result = true; // item already in list
    }
    if (command == m_last_tx_command) result = true;
    return result;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void KCX_BT_Emitter::add_tx_queue_item(ps_ptr<char> item) {
    if (already_in_TX_list(item)) {
        KCX_LOG_DEBUG("commamd '%s' already in tx list", item.c_get());
        return;
    }
    m_TX_queue.push_back(item);
    KCX_LOG_DEBUG("add_tx_queue_item %s", item.c_get());
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
ps_ptr<char> KCX_BT_Emitter::get_tx_queue_item() {
    ps_ptr<char> queue_item;
    if (m_TX_queue.size() == 0) return queue_item;
    queue_item = m_TX_queue[0];
    m_last_tx_command = m_TX_queue[0];
    m_tx_process_timer = 5; // max 5 sec
    KCX_LOG_DEBUG("get_tx_queue_item %s", queue_item.c_get());
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
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    add_tx_queue_item("AT+DELVMLINK");
}
void KCX_BT_Emitter::getVMlinks() {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    add_tx_queue_item("AT+VMLINK?");
}
void KCX_BT_Emitter::addLinkName(ps_ptr<char> name) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    name.insert("AT+ADDLINKNAME=", 0);
    add_tx_queue_item(name);
}
void KCX_BT_Emitter::addLinkAddr(ps_ptr<char> addr) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    addr.insert("AT+ADDLINKADD=", 0);
    add_tx_queue_item(addr);
}
void KCX_BT_Emitter::setVolume(uint8_t vol) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    if (vol > 31) { vol = 31; }
    ps_ptr<char> v = "AT+VOL=";
    v.appendf("%i", vol);
    add_tx_queue_item(v);
}
void KCX_BT_Emitter::setMode(ps_ptr<char> mode) {
    if (!m_f_power) {
        m_msg.e = evt_info;
        m_msg.arg = "Power On  first";
        if (m_bt_callback) m_bt_callback(m_msg);
        return;
    }
    if (mode == m_bt_mode) return;
    if (m_setMode_in_progress) return;
    m_setMode_in_progress = 15;
    ps_ptr<char> m = "AT+MODE_";
    m.append(mode);
    add_tx_queue_item(m);
}
void KCX_BT_Emitter::setMode_intern(ps_ptr<char> mode) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    if (mode.equals("RX")) {
        if (m_ibt_mode != 0) {
            m_ibt_mode = 0;
            digitalWrite(BT_MODE_PIN, LOW);
            m_mode_pin = LOW;
            m_msg.arg = "RX";
            m_msg.e = evt_mode;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
        m_tx_process_timer = 0;
    } else if (mode.equals("TX")) {
        if (m_ibt_mode != 1) {
            m_ibt_mode = 1;
            digitalWrite(BT_MODE_PIN, HIGH);
            m_mode_pin = HIGH;
            m_msg.arg = "TX";
            m_msg.e = evt_mode;
            if (m_bt_callback) { m_bt_callback(m_msg); }
        }
        m_tx_process_timer = 0;
    } else {
        KCX_LOG_ERROR("unknown mode %s", mode.c_get());
        return;
    }
    add_tx_queue_item("AT+RESET");
}
void KCX_BT_Emitter::pauseResume() {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    add_tx_queue_item("AT+PAUSE");
}
void KCX_BT_Emitter::downvolume() {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
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
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
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
    return m_myName.c_get();
}
void KCX_BT_Emitter::power_off() {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    if (!m_f_power || m_powerOff_in_progress > 0) return;
    m_powerOff_in_progress = 5;
    if (m_f_power) add_tx_queue_item("AT+POWER_OFF");
}
void KCX_BT_Emitter::power_on(ps_ptr<char> mode) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    if (m_f_power || m_powerOn_in_progress > 0) return;
    m_powerOn_in_progress = 10;
    m_last_tx_command = "AT+POWER_ON"; // fake AT command
    if (mode.equals("TX")) {
        m_mode_pin = true;
        digitalWrite(BT_MODE_PIN, m_mode_pin);
    } else {
        m_mode_pin = false;
        digitalWrite(BT_MODE_PIN, m_mode_pin);
    }
    digitalWrite(BT_CONNECT_PIN, LOW);
    m_connect_pin = LOW;
    vTaskDelay(10); // H>L>H 10ms pre impules
    digitalWrite(BT_CONNECT_PIN, HIGH);
    m_connect_pin = HIGH;
    vTaskDelay(10); // wait 10ms
    digitalWrite(BT_CONNECT_PIN, LOW);
    m_connect_pin = LOW;
    vTaskDelay(100); // H>L>H 100ms main impules
    digitalWrite(BT_CONNECT_PIN, HIGH);
    m_connect_pin = HIGH;
}
void KCX_BT_Emitter::userCommand(ps_ptr<char> cmd) {
    if (BT_MODE_PIN < 0 || BT_CONNECT_PIN < 0 || BT_RX_PIN < 0 || BT_TX_PIN < 0) return;
    add_tx_queue_item(cmd);
}
const char* KCX_BT_Emitter::list_protokol() {
    for (auto& item : m_RX_TX_protocol) { KCX_LOG_INFO("RX_TX_protocol %s", item.c_get()); }
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
    kcx_bt_memItems(m_jsonMemItemsStr.c_get());
    return;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* KCX_BT_Emitter::stringifyScannedItems() { // returns the last three scanned BT devices as jsonStr

    for (auto& item : m_bt_scannedItems) { KCX_LOG_DEBUG("RX_TX_protocol %s", item.c_get()); }

    m_jsonScanItemsStr.assign("[");
    // [{"name":"btName","addr":"82435181cc6a"},{"name":"btsecondName","addr":"82435181cc6b"},{name":"btthirdName","addr":"82435181cc6c"}]
    int idx1, idx2, idx3;
    for (int i = 0; i < m_bt_scannedItems.size(); i++) {
        KCX_LOG_DEBUG("m_bt_scannedItems[i]   %s", m_bt_scannedItems[i].c_get());
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
    KCX_LOG_DEBUG("%s", m_jsonScanItemsStr.c_get());
    return m_jsonScanItemsStr.c_get();
}
