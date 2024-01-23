/*
 * KCX_BT_Emitter.cpp
 *
 *  Created on: 21.01.2024
 *  updated on: 
 *      Author: Wolle
 */

#ifndef KCX_BT_EMITTER_H_
#define KCX_BT_EMITTER_H_
#include "Arduino.h"
#include "Ticker.h"
#include "vector"

extern __attribute__((weak)) void kcx_bt_info(const char*);
extern __attribute__((weak)) void kcx_bt_status(bool);
extern __attribute__((weak)) void kcx_bt_items(const char*);
extern __attribute__((weak)) void kcx_bt_scanned(const char*);

#define ANSI_ESC_BLACK      "\033[30m"
#define ANSI_ESC_RED        "\033[31m"
#define ANSI_ESC_GREEN      "\033[32m"
#define ANSI_ESC_YELLOW     "\033[33m"
#define ANSI_ESC_BLUE       "\033[34m"
#define ANSI_ESC_MAGENTA    "\033[35m"
#define ANSI_ESC_CYAN       "\033[36m"
#define ANSI_ESC_WHITE      "\033[37m"
#define ANSI_ESC_RESET      "\033[0m"
#define ANSI_ESC_BROWN      "\033[38;5;130m"
#define ANSI_ESC_ORANGE     "\033[38;5;214m"

class KCX_BT_Emitter{

    static void isr0();
    static void t1s();
    static KCX_BT_Emitter* objPtr;
    static KCX_BT_Emitter* tckPtr;
    void handleInterrupt();
    void handleTicker();
    volatile bool m_f_linkChanged;
    volatile bool m_f_ticker1s;

public:
    KCX_BT_Emitter(int8_t RX_pin, int8_t TX_pin, int8_t link_pin, int8_t mode_pin);
    ~KCX_BT_Emitter();
    void begin();
    void loop();
    void deleteVMlinks(); // all saved VM links will be deleted, return: "Delete_Vmlink"
    void addLinkName(const char* name);   // up to 10 names can be saved
    void addLinkAddr(const char* addr);   // up to 10 MAC addresses can be saved
    bool isConnected(){return m_f_status;}
    uint8_t getVolume(){return m_bt_volume;}
    void setVolume(uint8_t vol);

private:
    Ticker   tck1s;
    std::vector<char*>  m_bt_names;
    std::vector<char*>  m_bt_addr;
    int8_t   BT_EMITTER_LINK = -1;
    int8_t   BT_EMITTER_MODE = -1;
    int8_t   BT_EMITTER_RX   = -1;
    int8_t   BT_EMITTER_TX   = -1;
    bool     m_f_PSRAMfound = false;
    bool     m_f_status = false;   // connected or not
    bool     m_f_btEmitter_found = false;
    bool     m_f_waitForBtEmitter = false;
    bool     m_f_bt_mode = false;  // 0: BT_EMITTER, 1: BT_RECEIVER
    bool     m_f_bt_inUse = false; // waiting for response
    char*    m_chbuf;
    char*    m_msgbuf;
    uint32_t m_timeStamp = 0;
    uint32_t m_timeCounter = 0;
    uint8_t  m_bt_volume = 0;
    uint8_t  m_bt_add_num = 0;
    uint8_t  m_bt_name_num = 0;
    uint8_t  m_bt_add_cnt= 0;
    uint8_t  m_bt_name_cnt = 0;
    char*    m_bt_version = NULL;
    char*    m_lastCommand = NULL;
    char*    m_lastMsg = NULL;
    char*    m_autoLink = NULL;
    char*    m_JSONstr = NULL;

    void     readCmd();
    void     detectOKcmd();
    void     parseATcmds();
    void     handle1sEvent();
    void     writeCommand(const char* cmd);
    void     bt_Version();
    void     timeout();
    void     stillInUse(const char* cmd);   // command comes too fast
    void     responseError();
    void     warning(const char* w);
    void     cmd_Wrong();    //
    void     cmd_PowerOn();
    void     cmd_Mode();
    void     cmd_AutoLink();
    void     cmd_Volume();
    void     cmd_Delete();
    void     cmd_AddNum();
    void     cmd_NameNum();
    void     cmd_MemName();
    void     cmd_MemAddr();
    void     cmd_scannedItems();
    void     stringifyMemItems();

    bool startsWith(const char* base, const char* searchString) {
        char c;
        while((c = *searchString++) != '\0')
        if(c != *base++) return false;
        return true;
    }

    void vector_clear_and_shrink(std::vector<char*>& vec) {
        uint size = vec.size();
        for(int32_t i = 0; i < size; i++) {
            if(vec[i]) {
                free(vec[i]);
                vec[i] = NULL;
            }
        }
        vec.clear();
        vec.shrink_to_fit();
    }

    char* x_ps_strdup(const char* str) {
        char* ps_str = NULL;
        if(m_f_PSRAMfound) { ps_str = (char*)ps_malloc(strlen(str) + 1); }
        else { ps_str = (char*)malloc(strlen(str) + 1); }
        strcpy(ps_str, str);
        return ps_str;
    }
};

#endif // KCX_BT_EMITTER_H_