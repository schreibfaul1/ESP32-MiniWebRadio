
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

class ETH_IDF {

  public:
    enum class Event : uint8_t {
        Started, //
        Connected,
        Disconnected,
        Stopped,
        GotIP
    };

    // callbacks ---------------------------------------------------------
  public:
    struct msg_s {
        Event        event;
        ps_ptr<char> txt;
    };
    using EthernetCallback = std::function<void(const msg_s&)>;
    void ethernet_callback(EthernetCallback cb) { m_ethernet_callback = std::move(cb); }

  private:
    EthernetCallback m_ethernet_callback;
    static ETH_IDF*  s_instance;
    // -------------------------------------------------------------------

  public:
    ETH_IDF() {
        s_instance = this;
        m_eth_handle = nullptr;
    };
    ~ETH_IDF() { s_instance = nullptr; };
    // --------------------------------------------------
    // Public API
    // --------------------------------------------------

    bool begin();
    void end();
    bool isStarted() const { return m_started; }
    bool linkUp() const { return m_link_up; }
    bool hasIP() const { return m_has_ip; }

    // --------------------------------------------------
    // Network information
    // --------------------------------------------------

    esp_netif_ip_info_t ipInfo() const {

        esp_netif_ip_info_t info = {};

        if (m_eth_netif) esp_netif_get_ip_info(m_eth_netif, &info);

        return info;
    }

    // --------------------------------------------------

    esp_err_t getMacAddress(uint8_t mac[6]) const {

        if (!m_eth_handle || !mac) return ESP_ERR_INVALID_STATE;

        return esp_eth_ioctl(m_eth_handle, ETH_CMD_G_MAC_ADDR, mac);
    }

    // --------------------------------------------------

    esp_err_t getSpeed(eth_speed_t& speed) const {

        if (!m_eth_handle) return ESP_ERR_INVALID_STATE;

        return esp_eth_ioctl(m_eth_handle, ETH_CMD_G_SPEED, &speed);
    }

    // --------------------------------------------------

    esp_err_t getDuplex(eth_duplex_t& duplex) const {

        if (!m_eth_handle) return ESP_ERR_INVALID_STATE;

        return esp_eth_ioctl(m_eth_handle, ETH_CMD_G_DUPLEX_MODE, &duplex);
    }

  private:
    static constexpr const char* TAG = "ETH_IDF";
    msg_s                        m_msg;

    esp_eth_handle_t            m_eth_handle = nullptr;
    esp_netif_t*                m_eth_netif = nullptr;
    esp_eth_netif_glue_handle_t m_eth_glue = nullptr;

    esp_event_handler_instance_t m_eth_event_instance = nullptr;
    esp_event_handler_instance_t m_ip_event_instance = nullptr;

    //   event_callback_t m_callback = nullptr;
    void* m_context = nullptr;

    bool          m_started = false;
    volatile bool m_link_up = false;
    volatile bool m_has_ip = false;

    // --------------------------------------------------
    // YT8531 initialization
    // --------------------------------------------------

    esp_err_t initYT8531();
    // --------------------------------------------------
    // MAC / PHY initialization
    // --------------------------------------------------

    esp_err_t initEthernet();

    // --------------------------------------------------
    // Event handlers
    // --------------------------------------------------

    void notify(Event event) {
        msg_s msg;
        if (m_ethernet_callback) m_ethernet_callback(msg);
    }

    static void ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    // --------------------------------------------------
};
