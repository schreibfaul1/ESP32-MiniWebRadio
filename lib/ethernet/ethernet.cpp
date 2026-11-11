#include "ethernet.hpp"

ETH_IDF* ETH_IDF::s_instance = nullptr;

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
esp_err_t ETH_IDF::begin() {
    if (m_eth_handle) return ESP_ERR_INVALID_STATE;

    esp_err_t err = esp_netif_init(); // Initialize TCP/IP stack
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    err = esp_event_loop_create_default(); // Initialize default event loop
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;

    err = initEthernet(); // Initialize MAC and PHY
    if (err != ESP_OK) {
        end();
        return err;
    }

    esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_ETH(); // Create network interface

    m_eth_netif = esp_netif_new(&netif_config);
    if (!m_eth_netif) {
        end();
        return ESP_ERR_NO_MEM;
    }

    m_eth_glue = esp_eth_new_netif_glue(m_eth_handle); // Connect Ethernet driver to TCP/IP stack
    if (!m_eth_glue) {
        end();
        return ESP_ERR_NO_MEM;
    }

    err = esp_netif_attach(m_eth_netif, m_eth_glue);
    if (err != ESP_OK) {
        end();
        return err;
    }

    err = esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, ethEventHandler, this, &m_eth_event_instance); // Register events
    if (err != ESP_OK) {
        end();
        return err;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, ipEventHandler, this, &m_ip_event_instance);
    if (err != ESP_OK) {
        end();
        return err;
    }

    err = esp_eth_start(m_eth_handle); // Start Ethernet
    if (err != ESP_OK) {
        end();
        return err;
    }

    m_started = true;

    return ESP_OK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
esp_err_t ETH_IDF::end() {

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
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
 esp_err_t ETH_IDF::initYT8531() {

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
    // —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
