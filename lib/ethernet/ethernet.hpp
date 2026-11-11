
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
        Event event;
        ps_ptr<char>txt;
    };
    using EthernetCallback = std::function<void(const msg_s&)>;
    void ethernet_callback(EthernetCallback cb) { m_ethernet_callback = std::move(cb); }

  private:
    EthernetCallback m_ethernet_callback;
    static ETH_IDF*  s_instance;
    // -------------------------------------------------------------------

  public:
    ETH_IDF() {s_instance = this;};
    ~ETH_IDF() {s_instance = nullptr;};
    // --------------------------------------------------
    // Public API
    // --------------------------------------------------

    esp_err_t begin();
    esp_err_t end();
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
    msg_s m_msg;

    esp_eth_handle_t            m_eth_handle = nullptr;
    esp_netif_t*                m_eth_netif = nullptr;
    esp_eth_netif_glue_handle_t m_eth_glue = nullptr;

    esp_event_handler_instance_t m_eth_event_instance = nullptr;
    esp_event_handler_instance_t m_ip_event_instance = nullptr;

 //   event_callback_t m_callback = nullptr;
    void*            m_context = nullptr;

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

    esp_err_t initEthernet() {

        eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
        eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

        phy_config.phy_addr = -1;
        phy_config.reset_gpio_num = ETH_PHY_RST;

        eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();

        emac_config.interface = EMAC_DATA_INTERFACE_RGMII;

        emac_config.smi_gpio.mdc_num = ETH_PHY_MDC;
        emac_config.smi_gpio.mdio_num = ETH_PHY_MDIO;

        emac_config.clock_config.rgmii.clock_rx_gpio = ETH_RMII_RX_CLK;
        emac_config.clock_config.rgmii.clock_tx_gpio = ETH_RMII_TX_CLK;
        emac_config.clock_config.rgmii.clock_phy_ref_gpio = -1;

        emac_config.emac_dataif_gpio.rgmii.tx_ctl_num = ETH_RMII_TX_CTL;

        emac_config.emac_dataif_gpio.rgmii.txd0_num = ETH_RMII_TX0;
        emac_config.emac_dataif_gpio.rgmii.txd1_num = ETH_RMII_TX1;
        emac_config.emac_dataif_gpio.rgmii.txd2_num = ETH_RMII_TX2;
        emac_config.emac_dataif_gpio.rgmii.txd3_num = ETH_RMII_TX3;

        emac_config.emac_dataif_gpio.rgmii.rx_ctl_num = ETH_RMII_RX_CTL;

        emac_config.emac_dataif_gpio.rgmii.rxd0_num = ETH_RMII_RX0;
        emac_config.emac_dataif_gpio.rgmii.rxd1_num = ETH_RMII_RX1;
        emac_config.emac_dataif_gpio.rgmii.rxd2_num = ETH_RMII_RX2;
        emac_config.emac_dataif_gpio.rgmii.rxd3_num = ETH_RMII_RX3;

        // Create MAC
        esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

        if (!mac) {
            ESP_LOGE(TAG, "MAC creation failed");
            return ESP_FAIL;
        }

        // Create PHY
        esp_eth_phy_t* phy = esp_eth_phy_new_generic(&phy_config);

        if (!phy) {
            mac->del(mac);
            ESP_LOGE(TAG, "PHY creation failed");
            return ESP_FAIL;
        }

        // Install driver
        esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);

        esp_err_t err = esp_eth_driver_install(&config, &m_eth_handle);

        if (err != ESP_OK) {
            mac->del(mac);
            phy->del(phy);
            return err;
        }

        // Configure YT8531
        return initYT8531();
    }

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
