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
*/

#ifndef KCX_BT_EMITTER_H_
#define KCX_BT_EMITTER_H_
#include "Audio.h"
#include "Ticker.h"
#include "vector"

extern __attribute__((weak)) void kcx_bt_info(const char* info, const char* val);
extern __attribute__((weak)) void kcx_bt_status(bool);
extern __attribute__((weak)) void kcx_bt_memItems(const char*);
extern __attribute__((weak)) void kcx_bt_scanItems(const char*);
extern __attribute__((weak)) void kcx_bt_modeChanged(const char*);

class KCX_BT_Emitter {

    struct msg_queue {
        int32_t      fileSize;
        ps_ptr<char> fileName;
        ps_ptr<char> filePath;

        msg_queue(int32_t fs, const char* fn, const char* fp) : fileSize(fs), fileName(fn), filePath(fp) {}

        ~msg_queue() = default;
        msg_queue(const msg_queue&) = default;
        msg_queue& operator=(const msg_queue&) = default;
    };
    std::vector<msg_queue> m_msg_queue;

    static void            isr0();
    static void            t1s();
    static KCX_BT_Emitter* objPtr;
    static KCX_BT_Emitter* tckPtr;
    void                   handleInterrupt();
    void                   handleTicker();
    volatile bool          m_f_linkChanged;
    volatile bool          m_f_ticker1s;

  public:
    KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin);
    ~KCX_BT_Emitter();
    void begin();
    void loop();
    void deleteVMlinks();               // all saved VM links will be deleted, return: "Delete_Vmlink"
    void getVMlinks();                  // get all saved VM links
    void addLinkName(const char* name); // up to 10 names can be saved
    void addLinkAddr(const char* addr); // up to 10 MAC addresses can be saved
    bool isConnected() {
        if (!m_f_KCX_BT_Emitter_isInit)
            return false;
        else
            return digitalRead(BT_EMITTER_LINK);
    }
    uint8_t     getVolume() { return m_bt_volume; }
    void        setVolume(uint8_t vol);
    const char* getMode();
    void        setMode(const char* mode);
    void        changeMode();
    void        pauseResume();
    void        downvolume();
    void        upvolume();
    const char* getMyName();
    void        cmd_PowerOff();
    void        userCommand(const char* cmd);
    const char* stringifyScannedItems();
    const char* list_protokol(uint16_t elementNr);

    enum btmode { BT_MODE_RECEIVER = 0, BT_MODE_EMITTER = 1 };
    enum btconn { BT_NOT_CONNECTED = 0, BT_CONNECTED = 1 };
    enum btstate { BT_PAUSE = 0, BT_PLAY = 1 };

  private:
    Ticker                   tck1s;
    const uint8_t            m_chbufSize = 100;
    std::vector<char*>       m_bt_names;
    std::vector<char*>       m_bt_addr;
    std::vector<char*>       m_bt_scannedItems;
    std::deque<ps_ptr<char>> m_messageQueue;
    std::deque<ps_ptr<char>> m_RX_TX_protocol;
    int8_t                   BT_EMITTER_LINK = -1;
    int8_t                   BT_EMITTER_MODE = -1;
    int8_t                   BT_EMITTER_RX = -1;
    int8_t                   BT_EMITTER_TX = -1;
    int8_t                   m_Cmd = 0;  // question
    int8_t                   m_Answ = 0; // answer
    bool                     m_f_PSRAMfound = false;
    bool                     m_f_btEmitter_found = false;
    bool                     m_f_waitForBtEmitter = false;
    bool                     m_f_bt_mode = BT_MODE_EMITTER; // 0: BT_MODE_EMITTER, 1: BT_MODE_RECEIVER
    bool                     m_f_bt_state = BT_PLAY;        // 0: BT_PAUSE, 1: BT_PLAY
    bool                     m_f_status = BT_NOT_CONNECTED; // scan, connected or not
    bool                     m_f_bt_inUse = false;          // waiting for response
    bool                     m_f_scan = false;
    bool                     m_f_KCX_BT_Emitter_isInit = false;
    char*                    m_chbuf;
    uint32_t                 m_timeStamp = 0;
    uint32_t                 m_timeCounter = 0;
    uint8_t                  m_bt_volume = 0;
    uint8_t                  m_bt_add_num = 0;
    uint8_t                  m_bt_name_num = 0;
    uint8_t                  m_bt_add_cnt = 0;
    uint8_t                  m_bt_name_cnt = 0;
    char*                    m_bt_version = NULL;
    char*                    m_lastMsg = NULL;
    char*                    m_autoLink = NULL;
    char*                    m_jsonMemItemsStr = NULL;
    char*                    m_jsonScanItemsStr = NULL;
    char*                    m_myName = NULL;

    void         readCmd();
    void         detectOKcmd();
    void         parseATcmds();
    void         handle1sEvent();
    void         writeCommand(ps_ptr<char> cmd);
    void         bt_Version();
    void         timeout();
    void         stillInUse(const char* cmd); // command comes too fast
    void         responseError();
    void         warning(const char* w);
    void         cmd_Wrong(); //
    void         cmd_PowerOn();
    void         cmd_Mode();
    void         cmd_AutoLink();
    void         cmd_Volume();
    void         cmd_Delete();
    void         cmd_AddNum();
    void         cmd_NameNum();
    void         cmd_MemName();
    void         cmd_MemAddr();
    void         cmd_connectedName();
    void         cmd_connectedAddr();
    void         cmd_scannedItems();
    void         cmd_statePause();
    void         cmd_statePlay();
    void         cmd_ScanMode();
    ps_ptr<char> getQueueItem();
    void         addQueueItem(const char* item);
    void         stringifyMemItems();
    void         protocol_addElement(const char* RX_TX, const char* str);

    bool startsWith(const char* base, const char* searchString) {
        char c;
        while ((c = *searchString++) != '\0')
            if (c != *base++) return false;
        return true;
    }

    int32_t indexOf(const char* haystack, const char* needle, int32_t startIndex) {
        const char* p = haystack;
        for (; startIndex > 0; startIndex--)
            if (*p++ == '\0') return -1;
        char* pos = strstr(p, needle);
        if (pos == nullptr) return -1;
        return pos - haystack;
    }

    void vector_clear_and_shrink(std::vector<char*>& vec) {
        uint size = vec.size();
        for (int32_t i = 0; i < size; i++) {
            if (vec[i]) {
                free(vec[i]);
                vec[i] = NULL;
            }
        }
        vec.clear();
        vec.shrink_to_fit();
    }

    char* x_ps_strdup(const char* str) {
        char* ps_str = NULL;
        if (m_f_PSRAMfound) {
            ps_str = (char*)ps_malloc(strlen(str) + 1);
        } else {
            ps_str = (char*)malloc(strlen(str) + 1);
        }
        strcpy(ps_str, str);
        return ps_str;
    }

    char* x_ps_calloc(uint16_t len, uint8_t size) {
        char* ps_str = NULL;
        if (psramFound()) {
            ps_str = (char*)ps_calloc(len, size);
        } else {
            ps_str = (char*)calloc(len, size);
        }
        return ps_str;
    }
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