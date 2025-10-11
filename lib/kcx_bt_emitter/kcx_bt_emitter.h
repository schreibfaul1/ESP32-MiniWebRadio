/*
 * KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 11.11.2024
 *      Author: Wolle
 */

/*
Command             Answer
AT+RESET            OK+RESET
                    POWER ON
AT+GMR?             OK+VERS :KCX_BT_RTX_V1.x
AT+BT_MODE?         OK+BT_EMITTER or OK+BT_RECEIVER
AT+VOL?             OK+VOL=xx
AT+VOL=xx           OK+VOL=xx
AT+NAME?            OK+NAME=xx
AT+NAME+xx          OK+NAME=xx
AT+DELVMLINK        Delete_Vmlink
AT+VMLINK?          OK+VMLINK
                    BT_ADD_NUM=xx
                    BT_NAME_NUM=xx
                    Auto_link_Add:xx
                    MEM_MacAdd 00:xx
                    MEM_Name 00:xx
AT+ADDLINKNAME=xx   OK+ADDLINKNAME=xx
                    BT_ADD_NUM=xx
                    BT_NAME_NUM=xx
                    Auto_link_Add:xx
                    MEM_MacAdd 00:xx
                    MEM_Name 00:xx
AT+ADDLINKADD=xx    OK+ADDLINKADD=xx
                    BT_ADD_NUM=xx
                    BT_NAME_NUM=xx
                    Auto_link_Add:xx
                    MEM_MacAdd 00:xx
                    MEM_Name 00:xx
POWER ON
SCAN....
MacAdd:2c63c23c0200,Name: Pebble V3
CON MATCH ADD
CONNECT=>MacAdd:2c63c23c0200,Name: Pebble V3
*/

#ifndef KCX_BT_EMITTER_H_
#define KCX_BT_EMITTER_H_
#include "Audio.h"
#include "Ticker.h"
#include "vector"

extern __attribute__((weak)) void kcx_bt_info(const char* info, const char* val);
extern __attribute__((weak)) void kcx_bt_memItems(const char*);
extern __attribute__((weak)) void kcx_bt_scanItems(const char*);
extern __attribute__((weak)) void kcx_bt_modeChanged(const char*);

class KCX_BT_Emitter {
  public:
    enum { FOUND, VERSION, CONNECTED, MODE, VOLUME };
    enum { BT_MODE_RECEIVER = 0, BT_MODE_EMITTER = 1 };
    enum { BT_NOT_CONNECTED, BT_CONNECTED };
    enum { BT_PAUSE = 0, BT_PLAY = 1 };

    // callbacks ---------------------------------------------------------
  public:
    typedef enum { evt_found = 0, evt_connect, evt_disconnect, evt_reset, evt_power_on, evt_power_off, evt_scan, evt_volume, evt_version, evt_mode } event_t;
    struct msg_s { // used in info(audio_info_callback());
        const char* msg = nullptr;
        const char* s = nullptr;
        event_t     e = (event_t)0; // event type
        int32_t     val = 0;
        const char* arg = nullptr;
    };
    using BT_Callback = std::function<void(const msg_s&)>;
    void kcx_bt_emitter_callback(BT_Callback cb) { m_bt_callback = std::move(cb); }

  private:
    BT_Callback m_bt_callback;
    // -------------------------------------------------------------------

  public:
    KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin);
    ~KCX_BT_Emitter();
    void begin();
    void loop();
    void deleteVMlinks();                // all saved VM links will be deleted, return: "Delete_Vmlink"
    void getVMlinks();                   // get all saved VM links
    void addLinkName(ps_ptr<char> name); // up to 10 names can be saved
    void addLinkAddr(ps_ptr<char> addr); // up to 10 MAC addresses can be saved

    bool         isConnected() { return m_f_connected; }
    uint8_t      getVolume() { return m_bt_volume; }
    void         setVolume(uint8_t vol);           // 0 ... 31
    ps_ptr<char> getMode() { return m_f_bt_mode; } // 0: RECEIVER, 1: EMITTER
    void         setMode(ps_ptr<char> mode);       // RX: RECEIVER, TX: EMITTER
    void         pauseResume();
    void         downvolume();
    void         upvolume();
    const char*  getMyName();
    ps_ptr<char> get_bt_Version() { return m_bt_version; } // KCX_BT_RTX_V1.4
    void         power_off();
    void         power_on();
    void         userCommand(const char* cmd);
    const char*  stringifyScannedItems();
    const char*  list_protokol();

  private:
    msg_s                    m_msg;
    std::deque<ps_ptr<char>> m_TX_queue;
    std::deque<ps_ptr<char>> m_RX_queue;
    std::deque<ps_ptr<char>> m_RX_TX_protocol;
    ps_ptr<char>             m_MEM_MacAdd[10];
    ps_ptr<char>             m_MEM_Name[10];
    std::deque<ps_ptr<char>> m_bt_addr;
    std::deque<ps_ptr<char>> m_bt_scannedItems;
    ps_ptr<char>             m_last_tx_command;
    ps_ptr<char>             m_last_rx_command;
    ps_ptr<char>             m_jsonMemItemsStr;
    ps_ptr<char>             m_jsonScanItemsStr;
    ps_ptr<char>             get_tx_queue_item();
    void                     add_tx_queue_item(ps_ptr<char> item);
    ps_ptr<char>             get_rx_queue_item();
    void                     add_rx_queue_item(ps_ptr<char> item);
    ps_ptr<char>             m_bt_version;
    uint8_t                  m_bt_volume = 0;
    bool                     m_bt_found = false;

    int8_t BT_LINK_PIN = -1;
    int8_t BT_MODE_PIN = -1;
    int8_t BT_RX_PIN = -1;
    int8_t BT_TX_PIN = -1;
    bool   m_f_btEmitter_found = false;

    ps_ptr<char> m_f_bt_mode = "NA";
    bool         m_f_bt_state = BT_PLAY;           // 0: BT_PAUSE, 1: BT_PLAY
    bool         m_f_connected = BT_NOT_CONNECTED; // scan, connected or not
    bool         m_f_scan = false;
    bool         m_f_KCX_BT_Emitter_isInit = false;
    uint32_t     m_timeStamp = 0;
    bool         m_lock = false;

    uint8_t m_bt_add_num = 0;
    uint8_t m_bt_name_num = 0;
    uint8_t m_bt_add_cnt = 0;
    uint8_t m_bt_name_cnt = 0;

    ps_ptr<char> m_myName = "unknown";

    void readCmd();
    void parseATcmds();
    void writeCommand(ps_ptr<char> cmd);

    void stringifyMemItems();
    void protocol_addElement(const char* RX_TX, const char* str);
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Macro for comfortable calls
#define KCX_LOG_ERROR(fmt, ...)   Audio::AUDIO_LOG_IMPL(1, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define KCX_LOG_WARN(fmt, ...)    Audio::AUDIO_LOG_IMPL(2, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define KCX_LOG_INFO(fmt, ...)    Audio::AUDIO_LOG_IMPL(3, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define KCX_LOG_DEBUG(fmt, ...)   Audio::AUDIO_LOG_IMPL(4, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define KCX_LOG_VERBOSE(fmt, ...) Audio::AUDIO_LOG_IMPL(5, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#endif // KCX_BT_EMITTER_H_