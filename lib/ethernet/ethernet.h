
#pragma once
#include "../../src/settings.h"
#include "Audio.h"
#include "esp_check.h"
#include "esp_eth.h"
#include "esp_eth_mac_esp.h"
#include "esp_eth_netif_glue.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include <Arduino.h>
#include <atomic>
#include <functional>
#include <utility>

class ETH_IDF {

  public:
    enum class Event : uint8_t {
        Started, //
        Connected,
        Disconnected,
        Stopped,
        GotIP,
        Info
    };

    // callbacks ---------------------------------------------------------
  public:
    struct msg_s {
        Event        event = {};
        ps_ptr<char> txt = {};
        void         reset() { *this = msg_s{}; }
    };
    using EthernetCallback = std::function<void(const msg_s)>;
    void ethernet_callback(EthernetCallback cb) { m_ethernet_callback = std::move(cb); }

  private:
    EthernetCallback m_ethernet_callback;

    // -------------------------------------------------------------------

  public:
    ETH_IDF() { ; }
    ETH_IDF(const ETH_IDF&) = delete;
    ETH_IDF& operator=(const ETH_IDF&) = delete;
    ~ETH_IDF() { end(); }

    // --------------------------------------------------
    // Public API
    // --------------------------------------------------

    bool        begin();
    void        end();
    bool        isStarted() const { return m_started.load(); }
    bool        linkUp() const { return m_link_up.load(); }
    bool        hasIP() const { return m_has_ip.load(); }
    const char* getMacAddress();
    const char* getIPaddress() { return m_ip.c_get(); }
    const char* getNetmask() { return m_mask.c_get(); }
    const char* getGateway() { return m_gateway.c_get(); }
    uint16_t    getSpeed() { return m_speed; }
    const char* getDuplex() { return m_duplex.c_get(); }

    // --------------------------------------------------

  private:
    msg_s                        m_msg;
    ps_ptr<char>                 m_mac;
    ps_ptr<char>                 m_ip;
    ps_ptr<char>                 m_mask;
    ps_ptr<char>                 m_gateway;
    ps_ptr<char>                 m_duplex;
    uint16_t                     m_speed = 0;
    esp_eth_handle_t             m_eth_handle = nullptr;
    esp_netif_t*                 m_eth_netif = nullptr;
    esp_eth_netif_glue_handle_t  m_eth_glue = nullptr;
    esp_event_handler_instance_t m_eth_event_instance = nullptr;
    esp_event_handler_instance_t m_ip_event_instance = nullptr;

    //   event_callback_t m_callback = nullptr;
    std::atomic<bool> m_started = false;
    std::atomic<bool> m_link_up = false;
    std::atomic<bool> m_has_ip = false;

    bool        initYT8531();   // YT8531 initialization
    esp_err_t   initEthernet(); // MAC / PHY initialization
    int16_t     fetchSpeed();
    const char* fetchDuplex();

    static void ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    // --------------------------------------------------
};
