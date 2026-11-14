
#pragma once
#include "../../src/settings.h"
#include "Audio.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_eth.h"
#include "esp_eth_mac_esp.h"
#include "esp_eth_netif_glue.h"
#include "esp_eth_phy_ip101.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_rom_sys.h"
#include <Arduino.h>
#include <atomic>
#include <functional>
#include <utility>

// ETH_PHY_IP101
#ifndef ETH_PHY_MDC
    #define ETH_PHY_MDC 31
#endif
#ifndef ETH_PHY_MDIO
    #define ETH_PHY_MDIO 52
#endif
#ifndef ETH_PHY_RST
    #define ETH_PHY_RST 51
#endif
#ifndef ETH_RMII_REF_CLK
    #define ETH_RMII_REF_CLK 50
#endif
#ifndef ETH_RMII_TX_EN
    #define ETH_RMII_TX_EN 49
#endif
#ifndef ETH_RMII_TX0
    #define ETH_RMII_TX0 34
#endif
#ifndef ETH_RMII_TX1
    #define ETH_RMII_TX1 35
#endif
#ifndef ETH_RMII_RX0
    #define ETH_RMII_RX0 29
#endif
#ifndef ETH_RMII_RX1
    #define ETH_RMII_RX1 30
#endif
#ifndef ETH_RMII_CRS_DV
    #define ETH_RMII_CRS_DV 28
#endif

// ETH_PHY_YT8531
#ifndef ETH_PHY_MDC
    #define ETH_PHY_MDC 5
#endif
#ifndef ETH_PHY_MDIO
    #define ETH_PHY_MDIO 6
#endif
#ifndef ETH_PHY_RST
    #define ETH_PHY_RST 7
#endif
#ifndef ETH_RGMII_TX0
    #define ETH_RGMII_TX0 8
#endif
#ifndef ETH_RGMII_TX1
    #define ETH_RGMII_TX1 9
#endif
#ifndef ETH_RGMII_TX2
    #define ETH_RGMII_TX2 10
#endif
#ifndef ETH_RGMII_TX3
    #define ETH_RGMII_TX3 11
#endif
#ifndef ETH_RGMII_TX_CTL
    #define ETH_RGMII_TX_CTL 12
#endif
#ifndef ETH_RGMII_TX_CLK
    #define ETH_RGMII_TX_CLK 13
#endif
#ifndef ETH_RGMII_RX_CLK
    #define ETH_RGMII_RX_CLK 14
#endif
#ifndef ETH_RGMII_RX_CTL
    #define ETH_RGMII_RX_CTL 15
#endif
#ifndef ETH_RGMII_RX3
    #define ETH_RGMII_RX3 16
#endif
#ifndef ETH_RGMII_RX2
    #define ETH_RGMII_RX2 17
#endif
#ifndef ETH_RGMII_RX1
    #define ETH_RGMII_RX1 18
#endif
#ifndef ETH_RGMII_RX0
    #define ETH_RGMII_RX0 19
#endif

class ETH_IDF {

  private:
#define YT8531_PHY_ID1 0x4F51
#define YT8531_PHY_ID2 0xE91B
#define IP101_PHY_ID1  0x0243
#define IP101_PHY_ID2  0x0C54
#define MDIO_DELAY_US  1
#define ANSI_ESC_RED   "\033[31m"
#define ANSI_ESC_GREEN "\033[32m"
#define ANSI_ESC_RESET "\033[0m"

    enum class ETH_PHY_TYPE : uint8_t {
        NONE, //
        YT8531,
        IP101
    };

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
    bool        isConnected() { return m_isConnected; }

    // --------------------------------------------------

  private:
    msg_s                        m_msg;
    int8_t                       m_phy_addr = -1;
    ps_ptr<char>                 m_mac;
    ps_ptr<char>                 m_ip;
    ps_ptr<char>                 m_mask;
    ps_ptr<char>                 m_gateway;
    ps_ptr<char>                 m_duplex;
    uint16_t                     m_speed = 0;
    bool                         m_isConnected = false;
    esp_eth_handle_t             m_eth_handle = nullptr;
    esp_netif_t*                 m_eth_netif = nullptr;
    esp_eth_netif_glue_handle_t  m_eth_glue = nullptr;
    esp_event_handler_instance_t m_eth_event_instance = nullptr;
    esp_event_handler_instance_t m_ip_event_instance = nullptr;
    ETH_PHY_TYPE                 m_phy_type = ETH_PHY_TYPE::NONE;

    //   event_callback_t m_callback = nullptr;
    std::atomic<bool> m_started = false;
    std::atomic<bool> m_link_up = false;
    std::atomic<bool> m_has_ip = false;

    bool        mdioInit();
    void        mdioDeinit();
    bool        mdioRead(uint8_t phyAddr, uint8_t regAddr, uint16_t& value);
    bool        detectPHY();
    bool        initYT8531();   // YT8531 initialization
    esp_err_t   initEthernet(); // MAC / PHY initialization
    int16_t     fetchSpeed();
    const char* fetchDuplex();

    static void ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    // --------------------------------------------------
};


/*
FOR IP101GR HAS TO CHANGE:
in //.platformio/packages/framework-espidf/components/esp_eth/src/mac/esp_eth_mac_esp_gpio.c

case GPIO_MODE_INPUT_OUTPUT:
    ESP_RETURN_ON_FALSE(signal_in_idx != SIG_GPIO_OUT_IDX, ESP_ERR_NOT_SUPPORTED,
                        TAG, "requested periph signal cannot be connect via GPIO Matrix");
    ESP_RETURN_ON_FALSE(signal_out_idx != SIG_GPIO_OUT_IDX, ESP_ERR_NOT_SUPPORTED,
                        TAG, "requested periph signal cannot be connect via GPIO Matrix");
    ESP_RETURN_ON_FALSE((esp_gpio_reserve(BIT64(gpio_num)) & BIT64(gpio_num)) == 0,
                        ESP_ERR_INVALID_STATE, TAG, "GPIO %i is already reserved", gpio_num);

    gpio_input_enable(gpio_num);
    gpio_od_disable(gpio_num);  // TEST: OpenDrain explizit deaktivieren

    esp_rom_gpio_connect_out_signal(gpio_num, signal_out_idx, false, false);
    esp_rom_gpio_connect_in_signal(gpio_num, signal_in_idx, false);
    break;

*/
