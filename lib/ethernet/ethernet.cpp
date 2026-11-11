#include "ethernet.h"

ETH_IDF* ETH_IDF::s_instance = nullptr;

void ETH_IDF::ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto* self = static_cast<ETH_IDF*>(arg);

    switch (event_id) {

        case ETHERNET_EVENT_START:
            s_instance->m_msg.event = Event::Started;
            if (s_instance->m_ethernet_callback) s_instance->m_ethernet_callback(s_instance->m_msg);
            log_d("[%s:%i] Ethernet started", __FILE__, __LINE__);
            break;

        case ETHERNET_EVENT_CONNECTED: {

            self->m_link_up = true;

            eth_speed_t  speed;
            eth_duplex_t duplex;

            self->getSpeed(speed);
            self->getDuplex(duplex);

            s_instance->m_msg.event = Event::Connected;
            s_instance->m_msg.txt.assignf("Speed: {} Mbps, {} duplex", speed == ETH_SPEED_1000M ? 1000 : speed == ETH_SPEED_100M ? 100 : 10, duplex == ETH_DUPLEX_FULL ? "full" : "half");
            if (s_instance->m_ethernet_callback) s_instance->m_ethernet_callback(s_instance->m_msg);
            break;
        }

        case ETHERNET_EVENT_DISCONNECTED:
            self->m_link_up = false;
            self->m_has_ip = false;

            log_i("[%s:%i] Ethernet link down", __FILE__, __LINE__);

            s_instance->m_msg.event = Event::Disconnected;
            if (s_instance->m_ethernet_callback) s_instance->m_ethernet_callback(s_instance->m_msg);
            break;

            break;

        case ETHERNET_EVENT_STOP:
            log_i("[%s:%i] Ethernet stopped", __FILE__, __LINE__);
            self->notify(Event::Stopped);
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ETH_IDF::ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto* self = static_cast<ETH_IDF*>(arg);

    auto* event = static_cast<ip_event_got_ip_t*>(event_data);

    self->m_has_ip = true;

    log_i("[%s:%i] Got IP address", __FILE__, __LINE__);

    log_i("[%s:%i] IP: ", __FILE__, __LINE__, IPSTR, IP2STR(&event->ip_info.ip));

    log_i("[%s:%i] Mask: ", __FILE__, __LINE__, IPSTR, IP2STR(&event->ip_info.netmask));

    log_i("[%s:%i] Gateway: ", __FILE__, __LINE__, IPSTR, IP2STR(&event->ip_info.gw));

    self->notify(Event::GotIP);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::begin() {

    if (m_eth_handle) {
        log_e("[%s:%i] eth_handle is not NULL", __FILE__, __LINE__);
        return false;
    }

    esp_err_t err = esp_netif_init(); // Initialize TCP/IP stack
    if (err != ESP_OK) {
        log_e("[%s:%i] netif initializing failed", __FILE__, __LINE__);
        return false;
    }

    err = esp_event_loop_create_default(); // Initialize default event loop
    if (err != ESP_OK) {
        if (err == ESP_ERR_NO_MEM) {
            log_e("[%s:%i] Cannot allocate memory for event loops list", __FILE__, __LINE__);
        } else if (err == ESP_ERR_INVALID_STATE) {
            log_e("[%s:%i] Default event loop has already been created", __FILE__, __LINE__);
        } else {
            log_e("[%s:%i] Failed to create task loop", __FILE__, __LINE__);
        }
        return false;
    };

    err = initEthernet(); // Initialize MAC and PHY
    if (err != ESP_OK) {
        end();
        return false;
    }

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH(); // Create network interface

    m_eth_netif = esp_netif_new(&netif_config);
    if (!m_eth_netif) {
        end();
        log_e("[%s:%i] no esp-netif object created", __FILE__, __LINE__);
        return false;
    }

    m_eth_glue = esp_eth_new_netif_glue(m_eth_handle); // Connect Ethernet driver to TCP/IP stack
    if (!m_eth_glue) {
        end();
        log_e("[%s:%i] no new netif_glue object created", __FILE__, __LINE__);
        return false;
    }

    err = esp_netif_attach(m_eth_netif, m_eth_glue);
    if (err != ESP_OK) {
        end();
        log_e("[%s:%i] driver's pot_attach callback failed", __FILE__, __LINE__);
        return false;
    }

    err = esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, ethEventHandler, this, &m_eth_event_instance); // Register ethEventHandler
    if (err != ESP_OK) {
        end();
        log_e("[%s:%i] Register ethEventHandler failed", __FILE__, __LINE__);
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, ipEventHandler, this, &m_ip_event_instance); // Register ipEventHandler
    if (err != ESP_OK) {
        end();
        log_e("[%s:%i] Register ipEventHandler failed", __FILE__, __LINE__);
        return false;
    }

    err = esp_eth_start(m_eth_handle); // Start Ethernet
    if (err != ESP_OK) {
        end();
        if (err == ESP_ERR_INVALID_ARG) {
            log_e("[%s:%i] start esp_eth driver failed because of some invalid argument", __FILE__, __LINE__);
        } else if (err == ESP_ERR_INVALID_STATE) {
            log_e("[%s:%i] start esp_eth driver failed because driver has started already", __FILE__, __LINE__);
        } else {
            log_e("[%s:%i] start esp_eth driver failed because some other error occurred", __FILE__, __LINE__);
        }
        return false;
    }

    m_started = true;

    return ESP_OK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ETH_IDF::end() {

    esp_err_t err;

    if (m_eth_handle && m_started) {
        err = esp_eth_stop(m_eth_handle);

        if (err != ESP_OK) {
            if (err == ESP_ERR_INVALID_ARG) {
                log_e("[%s:%i] stop esp_eth driver failed because of some invalid argument", __FILE__, __LINE__);
            } else if (err == ESP_ERR_INVALID_STATE) {
                log_e("[%s:%i] stop esp_eth driver failed because driver  ", __FILE__, __LINE__);
            } else {
                log_e("[%s:%i] stop esp_eth driver failed because some other error ", __FILE__, __LINE__);
            }
        }

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
        esp_eth_del_netif_glue(m_eth_glue);
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

        if (err != ESP_OK) {
            if (err == ESP_ERR_INVALID_ARG) {
                log_e("[%s:%i] uninstall esp_eth driver failed because of some invalid argument", __FILE__, __LINE__);
            } else if (err == ESP_ERR_INVALID_STATE) {
                log_e("[%s:%i] uninstall esp_eth driver failed because it has more than one reference", __FILE__, __LINE__);
            } else {
                log_e("[%s:%i] uninstall esp_eth driver failed because some other error occurred", __FILE__, __LINE__);
            }
        }
    }

    m_link_up = false;
    m_has_ip = false;

    return;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::initYT8531() {
    esp_err_t err;
    bool      auto_nego = true;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_S_AUTONEGO, &auto_nego);
    if (err != ESP_OK) {
        log_e("[%s:%i] Auto negotiation failed", __FILE__, __LINE__);
        return false;
    }

    esp_eth_phy_reg_rw_data_t reg = {};
    uint32_t                  value = 0;

    reg.reg_value_p = &value;

    // Enable RX clock delay
    value = 0xA001;
    reg.reg_addr = 0x1E;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Write RX delay address failed", __FILE__, __LINE__);
        return false;
    }

    reg.reg_addr = 0x1F;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Read RX delay failed", __FILE__, __LINE__);
        return false;
    }

    value |= (1U << 8);

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Write RX delay failed", __FILE__, __LINE__);
        return false;
    }

    // Configure TX clock delay
    value = 0xA003;
    reg.reg_addr = 0x1E;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Write TX delay address failed", __FILE__, __LINE__);
        return false;
    }

    reg.reg_addr = 0x1F;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Read TX delay failed", __FILE__, __LINE__);
        return false;
    }

    value = (value & ~0x00FFU) | (13U << 4) | 13U;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        log_e("[%s:%i] Write TX delay failed", __FILE__, __LINE__);
        return false;
    }

    m_msg.event = Event::Info;
    m_msg.txt = "YT8531 initialized";
    if (m_ethernet_callback) m_ethernet_callback(m_msg);

    return ESP_OK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
esp_err_t ETH_IDF::initEthernet() {

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
        log_e("[%s:%i] MAC creation failed", __FILE__, __LINE__);
        return ESP_FAIL;
    }

    // Create PHY
    esp_eth_phy_t* phy = esp_eth_phy_new_generic(&phy_config);

    if (!phy) {
        mac->del(mac);
        log_e("[%s:%i] PHY creation failed", __FILE__, __LINE__);
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
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
