#include "ethernet.h"

// Created on: 11.11.2026
// Updated on: 13.11.2026

void ETH_IDF::ethEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto* self = static_cast<ETH_IDF*>(arg);

    switch (event_id) {

        case ETHERNET_EVENT_START:
            self->m_msg.reset();
            self->m_msg.event = Event::Started;
            if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);
            // log_w("[%s:%i] Ethernet started", __FILE__, __LINE__);
            break;

        case ETHERNET_EVENT_CONNECTED: {
            self->m_msg.reset();
            self->m_link_up = true;

            self->fetchSpeed();
            self->fetchDuplex();

            self->m_msg.event = Event::Connected;

            if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);
            break;
        }

        case ETHERNET_EVENT_DISCONNECTED:
            self->m_msg.reset();
            self->m_link_up = false;
            self->m_has_ip = false;
            self->m_speed = 0;
            self->m_duplex = "?";

            self->m_msg.event = Event::Info;
            self->m_msg.txt = "Ethernet link down";
            self->m_isConnected = false;
            if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);

            self->m_msg.event = Event::Disconnected;
            if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);
            break;

        case ETHERNET_EVENT_STOP:
            self->m_msg.reset();
            self->m_msg.event = Event::Stopped;
            self->m_isConnected = false;
            if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ETH_IDF::ipEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto* self = static_cast<ETH_IDF*>(arg);
    self->m_msg.reset();

    auto* event = static_cast<ip_event_got_ip_t*>(event_data);

    self->m_has_ip = true;
    self->m_ip.assignf("{}.{}.{}.{}", IP2STR(&event->ip_info.ip));
    self->m_mask.assignf("{}.{}.{}.{}", IP2STR(&event->ip_info.netmask));
    self->m_gateway.assignf("{}.{}.{}.{}", IP2STR(&event->ip_info.gw));

    self->m_msg.event = Event::GotIP;
    self->m_isConnected = true;
    if (self->m_ethernet_callback) self->m_ethernet_callback(self->m_msg);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::begin() {

    if (m_eth_handle) {
        esp_err_t err = esp_eth_driver_uninstall(m_eth_handle);

        if (err == ESP_OK) {
            m_eth_handle = nullptr;
        } else {
            printf(ANSI_ESC_RED "[%s:%i] uninstall esp_eth driver failed: %s\n" ANSI_ESC_RESET, __FILE__, __LINE__, esp_err_to_name(err));
        }
    }

    // Detect YT8531 before initializing the EMAC
    if (!detectPHY()) { return false; }

    esp_err_t err = esp_netif_init(); // Initialize TCP/IP stack
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] netif initializing failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    err = esp_event_loop_create_default(); // Initialize default event loop
    if (err != ESP_OK) {
        if (err == ESP_ERR_NO_MEM) {
            printf(ANSI_ESC_RED "[%s:%i] Cannot allocate memory for event loops list\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        } else if (err == ESP_ERR_INVALID_STATE) {
            printf(ANSI_ESC_RED "[%s:%i] Default event loop has already been created\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        } else {
            printf(ANSI_ESC_RED "[%s:%i] Failed to create task loop\n" ANSI_ESC_RESET, __FILE__, __LINE__);
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
        printf(ANSI_ESC_RED "[%s:%i] no esp-netif object created", __FILE__, __LINE__);
        return false;
    }

    m_eth_glue = esp_eth_new_netif_glue(m_eth_handle); // Connect Ethernet driver to TCP/IP stack
    if (!m_eth_glue) {
        end();
        printf(ANSI_ESC_RED "[%s:%i] no new netif_glue object created\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    err = esp_netif_attach(m_eth_netif, m_eth_glue);
    if (err != ESP_OK) {
        end();
        printf(ANSI_ESC_RED "[%s:%i] driver's pot_attach callback failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    err = esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, ethEventHandler, this, &m_eth_event_instance); // Register ethEventHandler
    if (err != ESP_OK) {
        end();
        printf(ANSI_ESC_RED "[%s:%i] Register ethEventHandler failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, ipEventHandler, this, &m_ip_event_instance); // Register ipEventHandler
    if (err != ESP_OK) {
        end();
        printf(ANSI_ESC_RED "[%s:%i] Register ipEventHandler failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    err = esp_eth_start(m_eth_handle); // Start Ethernet
    if (err != ESP_OK) {
        end();
        printf(ANSI_ESC_RED "[%s:%i] Ethernet stop failed: %s\n" ANSI_ESC_RESET, __FILE__, __LINE__, esp_err_to_name(err));
        return false;
    }

    m_started = true;

    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ETH_IDF::end() {

    esp_err_t err;

    if (m_eth_handle && m_started) {
        err = esp_eth_stop(m_eth_handle);

        if (err == ESP_OK) {
            m_started = false;
        } else {
            printf(ANSI_ESC_RED "[%s:%i] Ethernet stop failed: %s\n" ANSI_ESC_RESET, __FILE__, __LINE__, esp_err_to_name(err));
        }
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

        if (err == ESP_OK) {
            m_eth_handle = nullptr;
        } else {
            printf(ANSI_ESC_RED "[%s:%i] Ethernet uninstall failed: %s\n" ANSI_ESC_RESET, __FILE__, __LINE__, esp_err_to_name(err));
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
    m_msg.reset();

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_S_AUTONEGO, &auto_nego);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Auto negotiation failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
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
        printf(ANSI_ESC_RED "[%s:%i] Write RX delay address failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    reg.reg_addr = 0x1F;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Read RX delay failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    value |= (1U << 8);

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Write RX delay failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    // Configure TX clock delay
    value = 0xA003;
    reg.reg_addr = 0x1E;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Write TX delay address failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    reg.reg_addr = 0x1F;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_READ_PHY_REG, &reg);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Read TX delay failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    value = (value & ~0x00FFU) | (13U << 4) | 13U;

    err = esp_eth_ioctl(m_eth_handle, ETH_CMD_WRITE_PHY_REG, &reg);
    if (err != ESP_OK) {
        printf(ANSI_ESC_RED "[%s:%i] Write TX delay failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return false;
    }

    m_msg.event = Event::Info;
    m_msg.txt = "YT8531 initialized";
    if (m_ethernet_callback) m_ethernet_callback(m_msg);

    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
esp_err_t ETH_IDF::initEthernet() {

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();

    phy_config.phy_addr = m_phy_addr;

    phy_config.reset_gpio_num = ETH_PHY_RST;

    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();

    // Common SMI configuration
    emac_config.smi_gpio.mdc_num = ETH_PHY_MDC;
    emac_config.smi_gpio.mdio_num = ETH_PHY_MDIO;

    // Configure MAC interface depending on PHY
    switch (m_phy_type) {

        case ETH_PHY_TYPE::YT8531:

            emac_config.interface = EMAC_DATA_INTERFACE_RGMII;

            emac_config.clock_config.rgmii.clock_rx_gpio = ETH_RGMII_RX_CLK;
            emac_config.clock_config.rgmii.clock_tx_gpio = ETH_RGMII_TX_CLK;
            emac_config.clock_config.rgmii.clock_phy_ref_gpio = -1;

            emac_config.emac_dataif_gpio.rgmii.tx_ctl_num = ETH_RGMII_TX_CTL;

            emac_config.emac_dataif_gpio.rgmii.txd0_num = ETH_RGMII_TX0;
            emac_config.emac_dataif_gpio.rgmii.txd1_num = ETH_RGMII_TX1;
            emac_config.emac_dataif_gpio.rgmii.txd2_num = ETH_RGMII_TX2;
            emac_config.emac_dataif_gpio.rgmii.txd3_num = ETH_RGMII_TX3;

            emac_config.emac_dataif_gpio.rgmii.rx_ctl_num = ETH_RGMII_RX_CTL;

            emac_config.emac_dataif_gpio.rgmii.rxd0_num = ETH_RGMII_RX0;
            emac_config.emac_dataif_gpio.rgmii.rxd1_num = ETH_RGMII_RX1;
            emac_config.emac_dataif_gpio.rgmii.rxd2_num = ETH_RGMII_RX2;
            emac_config.emac_dataif_gpio.rgmii.rxd3_num = ETH_RGMII_RX3;

            break;

        case ETH_PHY_TYPE::IP101:

            emac_config.interface = EMAC_DATA_INTERFACE_RMII;

            // External 50 MHz REF_CLK from PHY
            emac_config.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
            emac_config.clock_config.rmii.clock_gpio = ETH_RMII_REF_CLK;

            emac_config.emac_dataif_gpio.rmii.tx_en_num = ETH_RMII_TX_EN;
            emac_config.emac_dataif_gpio.rmii.txd0_num = ETH_RMII_TX0;
            emac_config.emac_dataif_gpio.rmii.txd1_num = ETH_RMII_TX1;

            emac_config.emac_dataif_gpio.rmii.crs_dv_num = ETH_RMII_CRS_DV;
            emac_config.emac_dataif_gpio.rmii.rxd0_num = ETH_RMII_RX0;
            emac_config.emac_dataif_gpio.rmii.rxd1_num = ETH_RMII_RX1;

            break;

        default: printf(ANSI_ESC_RED "[%s:%i] Unknown PHY type\n" ANSI_ESC_RESET, __FILE__, __LINE__); return ESP_FAIL;
    }

    // Create MAC
    esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

    if (!mac) {
        printf(ANSI_ESC_RED "[%s:%i] MAC creation failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        return ESP_FAIL;
    }

    // Create PHY
    esp_eth_phy_t* phy = nullptr;

    switch (m_phy_type) {

        case ETH_PHY_TYPE::YT8531: phy = esp_eth_phy_new_generic(&phy_config); break;
        case ETH_PHY_TYPE::IP101: phy = esp_eth_phy_new_ip101(&phy_config); break;
        default: break;
    }

    if (!phy) {
        mac->del(mac);
        printf(ANSI_ESC_RED "[%s:%i] PHY creation failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
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

    // PHY-specific initialization
    if (m_phy_type == ETH_PHY_TYPE::YT8531) {
        if (!initYT8531()) {
            esp_eth_driver_uninstall(m_eth_handle);
            m_eth_handle = nullptr;
            return ESP_FAIL;
        }
    }

    return ESP_OK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* ETH_IDF::getMacAddress() {
    uint8_t mac[6];
    if (!m_eth_handle) return "";
    esp_err_t err = esp_eth_ioctl(m_eth_handle, ETH_CMD_G_MAC_ADDR, mac);
    if (err == ESP_OK) {
        m_mac.assignf("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return m_mac.c_get();
    } else {
        return "";
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int16_t ETH_IDF::fetchSpeed() {
    eth_speed_t speed;
    if (!m_eth_handle) return -1;
    esp_err_t err = esp_eth_ioctl(m_eth_handle, ETH_CMD_G_SPEED, &speed);
    if (err == ESP_OK) {
        switch (speed) {
            case ETH_SPEED_10M: m_speed = 10; break;
            case ETH_SPEED_100M: m_speed = 100; break;
            case ETH_SPEED_1000M: m_speed = 1000; break;
            default: m_speed = 0; break;
        }
        return m_speed;
    } else {
        return -1;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
const char* ETH_IDF::fetchDuplex() {
    eth_duplex_t duplex;
    if (!m_eth_handle) return "";
    m_duplex = "?";
    esp_err_t err = esp_eth_ioctl(m_eth_handle, ETH_CMD_G_DUPLEX_MODE, &duplex);
    if (err == ESP_OK) {
        if (duplex == ETH_DUPLEX_FULL) m_duplex = "full";
        if (duplex == ETH_DUPLEX_HALF) m_duplex = "half";
    }
    return m_duplex.c_get();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::mdioInit() {

    gpio_config_t mdc_config = {};
    mdc_config.pin_bit_mask = (1ULL << ETH_PHY_MDC);
    mdc_config.mode = GPIO_MODE_OUTPUT;
    mdc_config.pull_up_en = GPIO_PULLUP_DISABLE;
    mdc_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    mdc_config.intr_type = GPIO_INTR_DISABLE;

    if (gpio_config(&mdc_config) != ESP_OK) { return false; }

    gpio_config_t mdio_config = {};
    mdio_config.pin_bit_mask = (1ULL << ETH_PHY_MDIO);
    mdio_config.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    mdio_config.pull_up_en = GPIO_PULLUP_ENABLE;
    mdio_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    mdio_config.intr_type = GPIO_INTR_DISABLE;

    if (gpio_config(&mdio_config) != ESP_OK) { return false; }

    gpio_set_level((gpio_num_t)ETH_PHY_MDC, 0);
    gpio_set_level((gpio_num_t)ETH_PHY_MDIO, 1); // Release MDIO

    esp_rom_delay_us(10);

    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::mdioRead(uint8_t phyAddr, uint8_t regAddr, uint16_t& value) {

    auto writeBit = [](uint8_t bit) {
        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 0);
        gpio_set_level((gpio_num_t)ETH_PHY_MDIO, bit ? 1 : 0);

        esp_rom_delay_us(MDIO_DELAY_US);

        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 1);
        esp_rom_delay_us(MDIO_DELAY_US);

        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 0);
    };

    auto writeBits = [&](uint32_t data, uint8_t count) {
        for (int8_t i = count - 1; i >= 0; --i) { writeBit((data >> i) & 1); }
    };

    auto readBit = []() -> uint8_t {
        // MDC LOW
        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 0);

        // Release MDIO
        gpio_set_level((gpio_num_t)ETH_PHY_MDIO, 1);

        esp_rom_delay_us(2);

        // Read BEFORE rising MDC edge!
        uint8_t bit = gpio_get_level((gpio_num_t)ETH_PHY_MDIO);

        // Generate rising edge
        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 1);
        esp_rom_delay_us(2);

        // Generate falling edge
        gpio_set_level((gpio_num_t)ETH_PHY_MDC, 0);
        esp_rom_delay_us(2);

        return bit;
    };

    // 32-bit preamble
    writeBits(0xFFFFFFFF, 32);

    // Start of frame: 01
    writeBits(0b01, 2);

    // Operation: READ = 10
    writeBits(0b10, 2);

    // PHY address
    writeBits(phyAddr & 0x1F, 5);

    // Register address
    writeBits(regAddr & 0x1F, 5);

    // Release MDIO
    gpio_set_level((gpio_num_t)ETH_PHY_MDIO, 1);

    // Turnaround
    uint8_t ta1 = readBit();
    (void)ta1;
    uint8_t ta2 = readBit();

    // Read 16 data bits regardless of TA
    value = 0;
    for (uint8_t i = 0; i < 16; i++) { value = (value << 1) | readBit(); }

    // Release MDIO
    gpio_set_level((gpio_num_t)ETH_PHY_MDIO, 1);
    return (ta2 == 0);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool ETH_IDF::detectPHY() {

    m_phy_type = ETH_PHY_TYPE::NONE;
    m_phy_addr = -1;

    if (!mdioInit()) {
        printf(ANSI_ESC_RED "[%s:%i] MDIO initialization failed\n" ANSI_ESC_RESET, __FILE__, __LINE__);
        mdioDeinit();
        return false;
    }

    uint16_t id1 = 0;
    uint16_t id2 = 0;

    bool found = false;

    for (uint8_t addr = 0; addr < 32; addr++) {

        if (!mdioRead(addr, 2, id1)) continue;
        if (!mdioRead(addr, 3, id2)) continue;

        // Detect YT8531
        if (id1 == YT8531_PHY_ID1 && id2 == YT8531_PHY_ID2) {

            m_phy_type = ETH_PHY_TYPE::YT8531;
            m_phy_addr = addr;
            found = true;

            m_msg.event = Event::Info;
            m_msg.txt.assignf("YT8531 detected at PHY address {}", addr);
            if (m_ethernet_callback) m_ethernet_callback(m_msg);

            break;
        }

        // Detect IP101GR
        if (id1 == IP101_PHY_ID1 && (id2 & 0xFFF0) == (IP101_PHY_ID2 & 0xFFF0)) {

            m_phy_type = ETH_PHY_TYPE::IP101;
            m_phy_addr = addr;
            found = true;

            m_msg.event = Event::Info;
            m_msg.txt.assignf("IP101GR detected at PHY address {}", addr);
            if (m_ethernet_callback) m_ethernet_callback(m_msg);

            break;
        }
    }

    // Release GPIOs for ESP-IDF EMAC
    mdioDeinit();

    if (!found) {
        m_phy_type = ETH_PHY_TYPE::NONE;
        m_phy_addr = -1;

        // log_w("No supported Ethernet PHY detected");
    }

    return found;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void ETH_IDF::mdioDeinit() {

    gpio_reset_pin((gpio_num_t)ETH_PHY_MDC);
    gpio_reset_pin((gpio_num_t)ETH_PHY_MDIO);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
