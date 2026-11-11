
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

    // --------------------------------------------------
    // Public API
    // --------------------------------------------------

    esp_err_t begin() {

        if (m_eth_handle) return ESP_ERR_INVALID_STATE;

        // Initialize TCP/IP stack
        esp_err_t err = esp_netif_init();

        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

        // Initialize default event loop
        err = esp_event_loop_create_default();

        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

        // Initialize MAC and PHY
        err = initEthernet();

        if (err != ESP_OK) {
            end();
            return err;
        }

        // Create network interface
        esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH();

        m_eth_netif = esp_netif_new(&netif_config);

        if (!m_eth_netif) {
            end();
            return ESP_ERR_NO_MEM;
        }

        // Connect Ethernet driver to TCP/IP stack
        m_eth_glue = esp_eth_new_netif_glue(m_eth_handle);

        if (!m_eth_glue) {
            end();
            return ESP_ERR_NO_MEM;
        }

        err = esp_netif_attach(m_eth_netif, m_eth_glue);

        if (err != ESP_OK) {
            end();
            return err;
        }

        // Register events
        err = esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, ethEventHandler, this, &m_eth_event_instance);

        if (err != ESP_OK) {
            end();
            return err;
        }

        err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, ipEventHandler, this, &m_ip_event_instance);

        if (err != ESP_OK) {
            end();
            return err;
        }

        // Start Ethernet
        err = esp_eth_start(m_eth_handle);

        if (err != ESP_OK) {
            end();
            return err;
        }

        m_started = true;

        return ESP_OK;
    }

    // --------------------------------------------------

    esp_err_t end() {

        esp_err_t result = ESP_OK;
        esp_err_t err;

        if (m_eth_handle && m_started) {
            err = esp_eth_stop(m_eth_handle);

            if (err != ESP_OK && result == ESP_OK) result = err;

            m_started = false;
        }

        // Unregister event handlers
        if (m_eth_event_instance) {
            esp_event_handler_instance_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, m_eth_event_instance);

            m_eth_event_instance = nullptr;
        }

        if (m_ip_event_instance) {
            esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, m_ip_event_instance);

            m_ip_event_instance = nullptr;
        }

        // Delete netif glue
        if (m_eth_glue) {
            err = esp_eth_del_netif_glue(m_eth_glue);

            if (err != ESP_OK && result == ESP_OK) result = err;

            m_eth_glue = nullptr;
        }

        // Destroy network interface
        if (m_eth_netif) {
            esp_netif_destroy(m_eth_netif);
            m_eth_netif = nullptr;
        }

        // Uninstall Ethernet driver
        if (m_eth_handle) {
            err = esp_eth_driver_uninstall(m_eth_handle);

            if (err == ESP_OK) {
                m_eth_handle = nullptr;
            } else if (result == ESP_OK) {
                result = err;
            }
        }

        m_link_up = false;
        m_has_ip = false;

        return result;
    }

    // --------------------------------------------------
    // Status
    // --------------------------------------------------

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

    esp_err_t initYT8531() {

        bool auto_nego = true;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_S_AUTONEGO, &auto_nego), TAG, "Auto negotiation failed");

        esp_eth_phy_reg_rw_data_t reg = {};
        uint32_t                  value = 0;

        reg.reg_value_p = &value;

        // Enable RX clock delay
        value = 0xA001;
        reg.reg_addr = 0x1E;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg), TAG, "Write RX delay address failed");

        reg.reg_addr = 0x1F;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg), TAG, "Read RX delay failed");

        value |= (1U << 8);

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg), TAG, "Write RX delay failed");

        // Configure TX clock delay
        value = 0xA003;
        reg.reg_addr = 0x1E;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg), TAG, "Write TX delay address failed");

        reg.reg_addr = 0x1F;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg), TAG, "Read TX delay failed");

        value = (value & ~0x00FFU) | (13U << 4) | 13U;

        ESP_RETURN_ON_ERROR(esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg), TAG, "Write TX delay failed");

        ESP_LOGI(TAG, "YT8531 initialized");

        return ESP_OK;
    }

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

    static void ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        auto* self = static_cast<ETH_IDF*>(arg);

        switch (event_id) {

            case ETHERNET_EVENT_START:
                s_instance->m_msg.event = Event::Started;
                if(s_instance->m_ethernet_callback)s_instance->m_ethernet_callback(s_instance->m_msg);
                ESP_LOGD(TAG, "Ethernet started");
                break;

            case ETHERNET_EVENT_CONNECTED: {

                self->m_link_up = true;

                eth_speed_t  speed;
                eth_duplex_t duplex;

                self->getSpeed(speed);
                self->getDuplex(duplex);

                s_instance->m_msg.event = Event::Connected;
                s_instance->m_msg.txt.assignf("Speed: {} Mbps, {} duplex", speed == ETH_SPEED_1000M ? 1000 : speed == ETH_SPEED_100M ? 100 : 10, duplex == ETH_DUPLEX_FULL ? "full" : "half");
                if(s_instance->m_ethernet_callback)s_instance->m_ethernet_callback(s_instance->m_msg);
                break;
            }

            case ETHERNET_EVENT_DISCONNECTED:
                self->m_link_up = false;
                self->m_has_ip = false;

                ESP_LOGI(TAG, "Ethernet link down");

                s_instance->m_msg.event = Event::Disconnected;
                if(s_instance->m_ethernet_callback)s_instance->m_ethernet_callback(s_instance->m_msg);
                break;


                break;

            case ETHERNET_EVENT_STOP:
                ESP_LOGI(TAG, "Ethernet stopped");
                self->notify(Event::Stopped);
                break;
        }
    }

    // --------------------------------------------------

    static void ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        auto* self = static_cast<ETH_IDF*>(arg);

        auto* event = static_cast<ip_event_got_ip_t*>(event_data);

        self->m_has_ip = true;

        ESP_LOGI(TAG, "Got IP address");

        ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));

        ESP_LOGI(TAG, "Mask: " IPSTR, IP2STR(&event->ip_info.netmask));

        ESP_LOGI(TAG, "Gateway: " IPSTR, IP2STR(&event->ip_info.gw));

        self->notify(Event::GotIP);
    }
};
