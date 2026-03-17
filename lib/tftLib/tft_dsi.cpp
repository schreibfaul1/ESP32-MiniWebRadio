// first release on 01/2025
// updated on Mar 17 2026

#include "Arduino.h"
#ifdef CONFIG_IDF_TARGET_ESP32P4
    #include "tft_dsi.h"

    #define __malloc_heap_psram(size) heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL)

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_DSI::TFT_DSI() { // Constructor
    m_panel = NULL;
    m_h_res = 0;
    m_v_res = 0;
    m_framebuffer[0] = NULL;
    m_framebuffer[1] = NULL;
    m_framebuffer[2] = NULL;
    m_vsync_semaphore = xSemaphoreCreateBinary();
    m_vsyncCounter = 0;
    xSemaphoreGive(m_vsync_semaphore);
}
void TFT_DSI::loop() {
    GIF_loop();
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::begin(const Timing& newTiming) {

    const char* controller = "unknown";

    // Hintergrundbeleuchtung einschalten
    if (TFT_BL >= 0) {
        //    gpio_set_direction((gpio_num_t)TFT_BL, GPIO_MODE_OUTPUT);
        //    gpio_set_level((gpio_num_t)TFT_BL, 1); // Enable backlight
    }
    m_timing = newTiming;
    m_h_res = m_timing.h_res;
    m_v_res = m_timing.v_res;
    m_rowBuffer = (uint8_t*)ps_malloc(m_rowBufferSize);

    // --------------------------------------------------
    // 1. Acquire Channel
    // --------------------------------------------------
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = 3,
        .voltage_mv = 2500,
    };
    m_err = esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy);
    if (m_err != 0) { log_e("LDO Power not enabled, err: %i\n", m_err); }

    // --------------------------------------------------
    // 2. Reset Display
    // --------------------------------------------------
    reset();

    // --------------------------------------------------
    // 3. Create MIPI DSI Bus
    // --------------------------------------------------
    esp_lcd_dsi_bus_handle_t dsi_bus = NULL;
    esp_lcd_dsi_bus_config_t dsi_cfg = {
        .bus_id = 0,
        .num_data_lanes = 2,
        .phy_clk_src = MIPI_DSI_PHY_PLLREF_CLK_SRC_DEFAULT_LEGACY,
        .lane_bit_rate_mbps = m_timing.lane_bit_rate_mbps, // Anpassen für Dein Display!
    };
    m_err = esp_lcd_new_dsi_bus(&dsi_cfg, &dsi_bus);
    if (m_err != 0) { log_e("con't create DSI Bus, err: %i\n", m_err); }

    // --------------------------------------------------
    // 3. Create DBI-IO for Display-Commands
    // --------------------------------------------------
    esp_lcd_panel_io_handle_t mipi_dbi_io;
    esp_lcd_dbi_io_config_t   dbi_cfg = {
          .virtual_channel = 0,
          .lcd_cmd_bits = 8,
          .lcd_param_bits = 8,
    };
    m_err = esp_lcd_new_panel_io_dbi(dsi_bus, &dbi_cfg, &mipi_dbi_io);
    if (m_err != 0) { log_e("can't create DBI-IO, err: %i\n", m_err); }
    m_mipi_dbi_io = mipi_dbi_io;

    #if TFT_CONTROLLER == 8
    // --------------------------------------------------
    // 4. Init Display-Controller (EK97001)
    // --------------------------------------------------
    controller = "EK97001";
    /*
     * R00h: NOP ( No Operation)
     * R01h: GRB (Software Reset)
     * R0Ah: GET_POWER_MODE (Read Display Power Mode)
     * R0Dh: GET_DISPLAY_MODE ( Read the Current Display Mode)
     * R0Eh: GET_SIGNAL_MODE (TBD)
     * R10h: ENTER_SLEEP_MODE (Enter the Sleep-In Mode)
     * R11h: EXIT_SLEEP_MODE (Exit the Sleep-In Mode)
     * R20h:EXIT_INVERT_MODE (Display Inversion Off)
     * R21h: ENTER_INVERT_MODE (Display Inversion On)
     * R36h: SET_ADDRESS_MODE (Data Access Control)
     * R78h/R79h/R7Ah/R7Bh :GIP timing control register
     * R80h ... R86h: Gamma Control Register
     * RB0h: Panel Control Register
     * RB1h: Panel Control Register
     * RB2h: Panel Control Register
     * RB3h: Panel Control Register(non GIP mode)
     */
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB2, (uint8_t[]){0x10}, 1); // 2 LANE

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x80, (uint8_t[]){0x8B}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x81, (uint8_t[]){0x78}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x82, (uint8_t[]){0x84}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x83, (uint8_t[]){0x88}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x84, (uint8_t[]){0xA8}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x85, (uint8_t[]){0xE3}, 1); // Gamma
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x86, (uint8_t[]){0x88}, 1); // Gamma

    // esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x20, (uint8_t[]){0x00}, 0); // invert mode off
    // esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x21, (uint8_t[]){0x00}, 0); // invert mode on

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x11, (uint8_t[]){0x00}, 0); // exit sleep
    vTaskDelay(pdMS_TO_TICKS(100));
    #endif // TFT_CONTROLLER == 8

    #if TFT_CONTROLLER == 9
    // --------------------------------------------------
    // 4. Init Display-Controller (JD9165)
    // --------------------------------------------------
    controller = "JD9165";
    /*

     */
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xF7, (uint8_t[]){0x49, 0x61, 0x02, 0x00}, 4);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x01}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x04, (uint8_t[]){0x0C}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x05, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x06, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0B, (uint8_t[]){0x11}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x17, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x20, (uint8_t[]){0x04}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x1F, (uint8_t[]){0x05}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x23, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x25, (uint8_t[]){0x19}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x28, (uint8_t[]){0x18}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x29, (uint8_t[]){0x04}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x2A, (uint8_t[]){0x01}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x2B, (uint8_t[]){0x04}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x2C, (uint8_t[]){0x01}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x02}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x01, (uint8_t[]){0x22}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x03, (uint8_t[]){0x12}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x04, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x05, (uint8_t[]){0x64}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0A, (uint8_t[]){0x08}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0B, (uint8_t[]){0x0A, 0x1A, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x06, 0x08, 0x1F, 0x1D}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0C, (uint8_t[]){0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0D, (uint8_t[]){0x16, 0x1B, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x07, 0x09, 0x1E, 0x1C}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0E, (uint8_t[]){0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0F, (uint8_t[]){0x16, 0x1B, 0x0D, 0x0B, 0x0D, 0x11, 0x10, 0x1C, 0x1E, 0x09, 0x07}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x10, (uint8_t[]){0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x11, (uint8_t[]){0x0A, 0x1A, 0x0D, 0x0B, 0x0D, 0x11, 0x10, 0x1D, 0x1F, 0x08, 0x06}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x12, (uint8_t[]){0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x14, (uint8_t[]){0x00, 0x00, 0x11, 0x11}, 4);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x18, (uint8_t[]){0x99}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x06}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x12, (uint8_t[]){0x36, 0x2C, 0x2E, 0x3C, 0x38, 0x35, 0x35, 0x32, 0x2E, 0x1D, 0x2B, 0x21, 0x16, 0x29}, 14);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x13, (uint8_t[]){0x36, 0x2C, 0x2E, 0x3C, 0x38, 0x35, 0x35, 0x32, 0x2E, 0x1D, 0x2B, 0x21, 0x16, 0x29}, 14);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x0A}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x02, (uint8_t[]){0x4F}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0B, (uint8_t[]){0x40}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x12, (uint8_t[]){0x3E}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x13, (uint8_t[]){0x78}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x0D}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x0D, (uint8_t[]){0x04}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x10, (uint8_t[]){0x0C}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x11, (uint8_t[]){0x0C}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x12, (uint8_t[]){0x0C}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x13, (uint8_t[]){0x0C}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x30, (uint8_t[]){0x00}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x11, (uint8_t[]){0x00}, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x29, (uint8_t[]){0x00}, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelay(pdMS_TO_TICKS(100));

    #endif // TFT_CONTROLLER == 9

    #if TFT_CONTROLLER == 10
    // --------------------------------------------------
    // 4. Init Display-Controller (ST7701)
    // --------------------------------------------------
    controller = "ST7701";
    /*

     */
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xEF, (uint8_t[]){0x08}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xC0, (uint8_t[]){0x63, 0x00}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xC1, (uint8_t[]){0x0D, 0x02}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xC2, (uint8_t[]){0x10, 0x08}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xCC, (uint8_t[]){0x10}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB0, (uint8_t[]){0x80, 0x09, 0x53, 0x0C, 0xD0, 0x07, 0x0C, 0x09, 0x09, 0x28, 0x06, 0xD4, 0x13, 0x69, 0x2B, 0x71}, 16);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB1, (uint8_t[]){0x80, 0x94, 0x5A, 0x10, 0xD3, 0x06, 0x0A, 0x08, 0x08, 0x25, 0x03, 0xD3, 0x12, 0x66, 0x6A, 0x0D}, 16);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB0, (uint8_t[]){0x5D}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB1, (uint8_t[]){0x58}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB2, (uint8_t[]){0x87}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB3, (uint8_t[]){0x80}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB5, (uint8_t[]){0x4E}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB7, (uint8_t[]){0x85}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB8, (uint8_t[]){0x21}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xB9, (uint8_t[]){0x10, 0x1F}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xBB, (uint8_t[]){0x03}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xBC, (uint8_t[]){0x00}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xC1, (uint8_t[]){0x78}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xC2, (uint8_t[]){0x78}, 1);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xD0, (uint8_t[]){0x88}, 1);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE0, (uint8_t[]){0x00, 0x3A, 0x02}, 3);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE1, (uint8_t[]){0x04, 0xA0, 0x00, 0xA0, 0x05, 0xA0, 0x00, 0xA0, 0x00, 0x40, 0x40}, 11);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE2, (uint8_t[]){0x30, 0x00, 0x40, 0x40, 0x32, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00, 0xA0, 0x00}, 13);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE3, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE4, (uint8_t[]){0x44, 0x44}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE5, (uint8_t[]){0x09, 0x2E, 0xA0, 0xA0, 0x0B, 0x30, 0xA0, 0xA0, 0x05, 0x2A, 0xA0, 0xA0, 0x07, 0x2C, 0xA0, 0xA0}, 16);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE6, (uint8_t[]){0x00, 0x00, 0x33, 0x33}, 4);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE7, (uint8_t[]){0x44, 0x44}, 2);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xE8, (uint8_t[]){0x08, 0x2D, 0xA0, 0xA0, 0x0A, 0x2F, 0xA0, 0xA0, 0x04, 0x29, 0xA0, 0xA0, 0x06, 0x2B, 0xA0, 0xA0}, 16);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xEB, (uint8_t[]){0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00}, 7);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xEC, (uint8_t[]){0x08, 0x01}, 2);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xED, (uint8_t[]){0xB0, 0x2B, 0x98, 0xA4, 0x56, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0x65, 0x4A, 0x89, 0xB2, 0x0B}, 16);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xEF, (uint8_t[]){0x08, 0x08, 0x08, 0x45, 0x3F, 0x54}, 6);
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5);

    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x11, (uint8_t[]){0x00}, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    esp_lcd_panel_io_tx_param(mipi_dbi_io, 0x29, (uint8_t[]){0x00}, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    vTaskDelay(pdMS_TO_TICKS(100));
    #endif // TFT_CONTROLLER == 10

    // --------------------------------------------------
    // 5. Create DPI-Panel
    // --------------------------------------------------

    esp_lcd_dpi_panel_config_t dpi_cfg = {.virtual_channel = 0,
                                          .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
                                          .dpi_clock_freq_mhz = m_timing.pixel_clock_mhz,
                                          .pixel_format = LCD_COLOR_PIXEL_FORMAT_RGB565,
                                          .in_color_format = LCD_COLOR_FMT_RGB565,
                                          .out_color_format = LCD_COLOR_FMT_RGB565,

                                          .num_fbs = 3,

                                          .video_timing =
                                              {
                                                  .h_size = m_h_res,
                                                  .v_size = m_v_res,
                                                  .hsync_pulse_width = m_timing.hsync_pulse_width,
                                                  .hsync_back_porch = m_timing.hsync_back_porch,
                                                  .hsync_front_porch = m_timing.hsync_front_porch,
                                                  .vsync_pulse_width = m_timing.hsync_pulse_width,
                                                  .vsync_back_porch = m_timing.hsync_back_porch,
                                                  .vsync_front_porch = m_timing.vsync_front_porch,
                                              },

                                          .flags = {
                                              .use_dma2d = true,
                                              .disable_lp = false,
                                          }};

    m_err = esp_lcd_new_panel_dpi(dsi_bus, &dpi_cfg, &m_panel);
    if (m_err != 0) { log_e("can't create new panel, err: %i\n", m_err); }

    m_err = esp_lcd_panel_init(m_panel);
    if (m_err != 0) {
        log_e("init panel, err: %i\n", m_err);
    } else if (tft_info) {
        char buff[100];
        sprintf(buff, "TFT Controller: " ANSI_ESC_CYAN "%s" ANSI_ESC_RESET " initialized", controller);
        tft_info(buff);
    }

    // --------------------------------------------------
    // 6. Fetch Framebuffer-Pointer from Driver
    // --------------------------------------------------

    void *fb0, *fb1, *fb2;
    esp_lcd_dpi_panel_get_frame_buffer(m_panel, 3, &fb0, &fb1, &fb2);
    m_framebuffer[0] = (uint16_t*)fb0;
    m_framebuffer[1] = (uint16_t*)fb1;
    m_framebuffer[2] = (uint16_t*)fb2;

    char buff[256];
    sprintf(buff, "Resolution: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET " x " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET, m_h_res, m_v_res);
    if (tft_info) tft_info(buff);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::reset() {
    #ifdef LCD_RESET
    pinMode(LCD_RESET, OUTPUT);
    digitalWrite(LCD_RESET, 1);
    vTaskDelay(20);
    digitalWrite(LCD_RESET, 0);
    vTaskDelay(20);
    digitalWrite(LCD_RESET, 1);
    vTaskDelay(100);
    // ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panel));
    #endif
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_DSI::panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void* bitmap) {
    bool res = false;
    if (x0 >= x1 || y0 >= y1) {
        log_w("%s %s  x0 %i, y0 %i, x1 %i, y1 %i", __FILE__, __LINE__, x0, y0, x1, y1);
        return false;
    }
    xSemaphoreTake(m_vsync_semaphore, 0.3 * configTICK_RATE_HZ);
    res = esp_lcd_panel_draw_bitmap(m_panel, x0, y0, x1, y1, (const uint16_t*)bitmap);
    xSemaphoreGive(m_vsync_semaphore);
    return res;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::setDisplayInversion(bool invert) {
    if (m_invert == invert) return;
    m_invert = invert;
    esp_lcd_panel_invert_color(m_panel, invert);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::setRotation(uint8_t r) {
    if (r >= 4) return;
    m_rotation = r;
    uint16_t deg = r * 90;
    char     buff[256];
    sprintf(buff, "Rotation: " ANSI_ESC_CYAN "%d° CW" ANSI_ESC_RESET " logical width: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET " logical height: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET, deg, logicalWidth(),
            logicalHeight());
    if (tft_info) tft_info(buff);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data) {
    // Check whether parameters are within the valid range
    if (x < 0 || y < 0 || w <= 0 || h <= 0) return;
    if (x + w > m_h_res || y + h > m_v_res) return; // logicalWidth() = vertical resolution
    if (!data || !m_framebuffer[0]) return;

    uint16_t* dst = data;
    uint16_t* src = m_framebuffer[0] + y * m_h_res + x;

    for (int32_t row = 0; row < h; row++) {
        memcpy(dst, src, w * sizeof(uint16_t));
        src += m_h_res;
        dst += w;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::afterTextDraw(uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H) {
    panelDrawBitmap(win_X, win_Y, win_X + win_W, win_Y + win_H, m_framebuffer[0]);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#endif // CONFIG_IDF_TARGET_ESP32P4
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
