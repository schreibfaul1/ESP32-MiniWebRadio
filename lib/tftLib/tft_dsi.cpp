// first release on 01/2025
// updated on Mar 01 2026

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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫    T E X T    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ *
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::afterTextDraw(uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H) {
    panelDrawBitmap(win_X, win_Y, win_X + win_W, win_Y + win_H, m_framebuffer[0]);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  B I T M A P  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫              *
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  G I F  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   P N G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TFT_DSI::drawPngFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {

    png_state = PNG_NEW;
    png_error = PNG_EOK;
    png_color_type = PNG_RGBA;
    png_color_depth = 8;
    png_format = PNG_RGBA8;
    png_size = 0;
    png_pos_x = x;
    png_pos_y = y;
    png_max_width = maxWidth;
    png_max_height = maxHeight;

    if (!fs.exists(path)) {
        log_e("File not found: %s", path);
        return NULL;
    }
    png_file = fs.open(path, "r");
    if (!png_file) {
        log_e("Failed to open file for reading");
        return NULL;
    }
    int file_size = png_file.size(); /* get filesize */
    png_buffer = (char*)ps_malloc(file_size);
    png_size = file_size;
    if (!png_buffer) {
        log_e("Failed to allocate memory for file");
        png_file.close();
        return NULL;
    }
    png_file.readBytes(png_buffer, (size_t)file_size);
    png_file.close();
    int err = png_decode();
    //  log_w("png_decode err=%i",err);
    return err == PNG_EOK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
char TFT_DSI::read_bit(uint32_t* bitpointer, const char* bitstream) {
    char result = (char)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
    (*bitpointer)++;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_DSI::read_bits(uint32_t* bitpointer, const char* bitstream, uint32_t nbits) {
    unsigned result = 0, i;
    for (i = 0; i < nbits; i++) result |= ((uint16_t)read_bit(bitpointer, bitstream)) << i;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/* the buffer must be numcodes*2 in size! */
void TFT_DSI::huffman_tree_init(huffman_tree* tree, uint16_t* buffer, uint16_t numcodes, uint16_t maxbitlen) {
    tree->tree2d = buffer;
    tree->numcodes = numcodes;
    tree->maxbitlen = maxbitlen;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate. maxbitlen is the maximum
 * bits that a code in the tree can have. return value is error.*/
void TFT_DSI::huffman_tree_create_lengths(huffman_tree* tree, const uint16_t* bitlen) {
    uint16_t tree1d[MAX_SYMBOLS];
    uint16_t blcount[MAX_BIT_LENGTH];
    uint16_t nextcode[MAX_BIT_LENGTH + 1];
    uint16_t bits, n, i;
    uint16_t nodefilled = 0; /*up to which node it is filled */
    uint16_t treepos = 0;    /*position in the tree (1 of the numcodes columns) */

    /* initialize local vectors */
    memset(blcount, 0, sizeof(blcount));
    memset(nextcode, 0, sizeof(nextcode));

    /*step 1: count number of instances of each code length */
    for (bits = 0; bits < tree->numcodes; bits++) { blcount[bitlen[bits]]++; }

    /*step 2: generate the nextcode values */
    for (bits = 1; bits <= tree->maxbitlen; bits++) { nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1; }

    /*step 3: generate all the codes */
    for (n = 0; n < tree->numcodes; n++) {
        if (bitlen[n] != 0) { tree1d[n] = nextcode[bitlen[n]]++; }
    }

    /*convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2
     possible bit values (0 or 1), there are as many columns as codes - 1 a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what
     their 0 and 1 option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen */
    for (n = 0; n < tree->numcodes * 2; n++) { tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet */ }

    for (n = 0; n < tree->numcodes; n++) { /*the codes */
        for (i = 0; i < bitlen[n]; i++) {  /*the bits for this code */
            unsigned char bit = (unsigned char)((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
            /* check if oversubscribed */
            if (treepos > tree->numcodes - 2) {
                log_e("oversubscribed");
                png_error = PNG_EMALFORMED;
                return;
            }

            if (tree->tree2d[2 * treepos + bit] == 32767) { /*not yet filled in */
                if (i + 1 == bitlen[n]) {                   /*last bit */
                    tree->tree2d[2 * treepos + bit] = n;    /*put the current code in it */
                    treepos = 0;
                } else { /*put address of the next step in here, first that address has to be found of course (it's just
                            nodefilled + 1)... */
                    nodefilled++;
                    tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes; /*addresses encoded with numcodes added to it */
                    treepos = nodefilled;
                }
            } else {
                treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
            }
        }
    }

    for (n = 0; n < tree->numcodes * 2; n++) {
        if (tree->tree2d[n] == 32767) { tree->tree2d[n] = 0; /*remove possible remaining 32767's */ }
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_DSI::huffman_decode_symbol(const char* in, uint32_t* bp, const huffman_tree* codetree, uint32_t inlength) {
    uint16_t treepos = 0, ct;
    char     bit;
    for (;;) {
        /* error: end of input memory reached without endcode */
        if (((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength) {
            log_e("end of input memory reached without endcode");
            png_error = PNG_EMALFORMED;
            return 0;
        }

        bit = read_bit(bp, in);

        ct = codetree->tree2d[(treepos << 1) | bit];
        if (ct < codetree->numcodes) { return ct; }

        treepos = ct - codetree->numcodes;
        if (treepos >= codetree->numcodes) {
            log_e("error, treepos is larger than numcodes");
            png_error = PNG_EMALFORMED;
            return 0;
        }
    }
    return 0;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/* get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
void TFT_DSI::get_tree_inflate_dynamic(huffman_tree* codetree, huffman_tree* codetreeD, huffman_tree* codelengthcodetree, const char* in, uint32_t* bp, uint32_t inlength) {
    uint16_t codelengthcode[NUM_CODE_LENGTH_CODES];
    uint16_t bitlen[NUM_DEFLATE_CODE_SYMBOLS];
    uint16_t bitlenD[NUM_DISTANCE_SYMBOLS];
    uint16_t n, hlit, hdist, hclen, i;

    /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated */
    /*C-code note: use no "return" between ctor and dtor of an uivector! */
    if ((*bp) >> 3 >= inlength - 2) {
        log_e("error, bit pointer will jump past memory");
        png_error = PNG_EMALFORMED;
        return;
    }

    /* clear bitlen arrays */
    memset(bitlen, 0, sizeof(bitlen));
    memset(bitlenD, 0, sizeof(bitlenD));

    /*the bit pointer is or will go past the memory */
    hlit = read_bits(bp, in, 5) + 257; /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already */
    hdist = read_bits(bp, in, 5) + 1;  /*number of distance codes. Unlike the spec, the value 1 is added to it here already */
    hclen = read_bits(bp, in, 4) + 4;  /*number of code length codes. Unlike the spec, the value 4 is added to it here already */

    for (i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
        if (i < hclen) {
            codelengthcode[CLCL[i]] = read_bits(bp, in, 3);
        } else {
            codelengthcode[CLCL[i]] = 0; /*if not, it must stay 0 */
        }
    }

    huffman_tree_create_lengths(codelengthcodetree, codelengthcode);

    /* bail now if we encountered an error earlier */
    if (png_error != PNG_EOK) { return; }

    /*now we can use this tree to read the lengths for the tree that this function will return */
    i = 0;
    while (i < hlit + hdist) { /*i is the current symbol we're reading in the part that contains the code lengths of
                                  lit/len codes and dist codes */
        unsigned code = huffman_decode_symbol(in, bp, codelengthcodetree, inlength);
        if (png_error != PNG_EOK) { break; }

        if (code <= 15) { /*a length code */
            if (i < hlit) {
                bitlen[i] = code;
            } else {
                bitlenD[i - hlit] = code;
            }
            i++;
        } else if (code == 16) {    /*repeat previous */
            unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6) */
            unsigned value;         /*set value to the previous code */

            if ((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }
            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 2);

            if ((i - 1) < hlit) {
                value = bitlen[i - 1];
            } else {
                value = bitlenD[i - hlit - 1];
            }

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++) {
                /* i is larger than the amount of codes */
                if (i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }

                if (i < hlit) {
                    bitlen[i] = value;
                } else {
                    bitlenD[i - hlit] = value;
                }
                i++;
            }
        } else if (code == 17) {    /*repeat "0" 3-10 times */
            unsigned replength = 3; /*read in the bits that indicate repeat length */
            if ((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }

            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 3);

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++) {
                /* error: i is larger than the amount of codes */
                if (i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }

                if (i < hlit) {
                    bitlen[i] = 0;
                } else {
                    bitlenD[i - hlit] = 0;
                }
                i++;
            }
        } else if (code == 18) {     /*repeat "0" 11-138 times */
            unsigned replength = 11; /*read in the bits that indicate repeat length */
            /* error, bit pointer jumps past memory */
            if ((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }

            replength += read_bits(bp, in, 7);

            /*repeat this value in the next lengths */
            for (n = 0; n < replength; n++) {
                /* i is larger than the amount of codes */
                if (i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }
                if (i < hlit)
                    bitlen[i] = 0;
                else
                    bitlenD[i - hlit] = 0;
                i++;
            }
        } else {
            /* somehow an unexisting code appeared. This can never happen. */
            log_e("error: unexisting code");
            png_error = PNG_EMALFORMED;
            break;
        }
    }

    if (png_error == PNG_EOK && bitlen[256] == 0) {
        log_e("image data is not a valid PNG image");
        png_error = PNG_EMALFORMED;
    }

    /*the length of the end code 256 must be larger than 0 */
    /*now we've finally got hlit and hdist, so generate the code trees, and the function is done */
    if (png_error == PNG_EOK) { huffman_tree_create_lengths(codetree, bitlen); }
    if (png_error == PNG_EOK) { huffman_tree_create_lengths(codetreeD, bitlenD); }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*inflate a block with dynamic of fixed Huffman tree*/
void TFT_DSI::inflate_huffman(char* out, uint32_t outsize, const char* in, uint32_t* bp, uint32_t* pos, uint32_t inlength, uint16_t btype) {
    uint16_t codetree_buffer[DEFLATE_CODE_BUFFER_SIZE];
    uint16_t codetreeD_buffer[DISTANCE_BUFFER_SIZE];
    uint16_t done = 0;

    huffman_tree codetree;
    huffman_tree codetreeD;

    if (btype == 1) {
        /* fixed trees */
        huffman_tree_init(&codetree, (uint16_t*)FIXED_DEFLATE_CODE_TREE, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
        huffman_tree_init(&codetreeD, (uint16_t*)FIXED_DISTANCE_TREE, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
    } else if (btype == 2) {
        /* dynamic trees */
        uint16_t     codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
        huffman_tree codelengthcodetree;

        huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
        huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
        huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer, NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);
        get_tree_inflate_dynamic(&codetree, &codetreeD, &codelengthcodetree, in, bp, inlength);
    }

    while (done == 0) {
        unsigned code = huffman_decode_symbol(in, bp, &codetree, inlength);
        if (png_error != PNG_EOK) { return; }

        if (code == 256) {
            /* end code */
            done = 1;
        } else if (code <= 255) {
            /* literal symbol */
            if ((*pos) >= outsize) {
                log_e("output buffer is too small");
                png_error = PNG_EMALFORMED;
                return;
            }

            /* store output */
            out[(*pos)++] = (unsigned char)(code);
        } else if (code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX) { /*length code */
            /* part 1: get length base */
            uint32_t length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];
            unsigned codeD, distance, numextrabitsD;
            uint32_t start, forward, backward, numextrabits;

            /* part 2: get extra bits and add the value of that to length */
            numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

            /* error, bit pointer will jump past memory */
            if (((*bp) >> 3) >= inlength) {
                log_e("bit pointer will jump past memory");
                png_error = PNG_EMALFORMED;
                return;
            }
            length += read_bits(bp, in, numextrabits);

            /*part 3: get distance code */
            codeD = huffman_decode_symbol(in, bp, &codetreeD, inlength);
            if (png_error != PNG_EOK) { return; }

            /* invalid distance code (30-31 are never used) */
            if (codeD > 29) {
                log_e("invalid distance code");
                png_error = PNG_EMALFORMED;
                return;
            }

            distance = DISTANCE_BASE[codeD];

            /*part 4: get extra bits from distance */
            numextrabitsD = DISTANCE_EXTRA[codeD];

            /* error, bit pointer will jump past memory */
            if (((*bp) >> 3) >= inlength) {
                log_e("bit pointer will jump past memory");
                png_error = PNG_EMALFORMED;
                return;
            }

            distance += read_bits(bp, in, numextrabitsD);

            /*part 5: fill in all the out[n] values based on the length and dist */
            start = (*pos);
            backward = start - distance;

            if ((*pos) + length >= outsize) {
                log_e("output buffer is too small");
                png_error = PNG_EMALFORMED;
                return;
            }

            for (forward = 0; forward < length; forward++) {
                out[(*pos)++] = out[backward];
                backward++;

                if (backward >= start) { backward = start - distance; }
            }
        }
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::inflate_uncompressed(char* out, uint32_t outsize, const char* in, uint32_t* bp, uint32_t* pos, uint32_t inlength) {
    uint32_t p;
    unsigned len, nlen, n;

    /* go to first boundary of byte */
    while (((*bp) & 0x7) != 0) { (*bp)++; }
    p = (*bp) / 8; /*byte position */

    /* read len (2 bytes) and nlen (2 bytes) */
    if (p >= inlength - 4) {
        log_e("p >= inlength - 4");
        png_error = PNG_EMALFORMED;
        return;
    }

    len = in[p] + 256 * in[p + 1];
    p += 2;
    nlen = in[p] + 256 * in[p + 1];
    p += 2;

    /* check if 16-bit nlen is really the one's complement of len */
    if (len + nlen != 65535) {
        log_e("nlen is not one's complement of len");
        png_error = PNG_EMALFORMED;
        return;
    }

    if ((*pos) + len >= outsize) {
        log_e("output buffer is too small");
        png_error = PNG_EMALFORMED;
        return;
    }

    /* read the literal data: len bytes are now stored in the out buffer */
    if (p + len > inlength) {
        log_e("p + len > inlength");
        png_error = PNG_EMALFORMED;
        return;
    }

    for (n = 0; n < len; n++) { out[(*pos)++] = in[p++]; }

    (*bp) = p * 8;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*inflate the deflated data (cfr. deflate spec); return value is the error*/
int8_t TFT_DSI::uz_inflate_data(char* out, uint32_t outsize, const char* in, uint32_t insize, uint32_t inpos) {
    uint32_t bp = 0;  /*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte) */
    uint32_t pos = 0; /*byte position in the out buffer */
    uint16_t done = 0;

    while (done == 0) {
        uint16_t btype;

        /* ensure next bit doesn't point past the end of the buffer */
        if ((bp >> 3) >= insize) {
            log_e("bp >> 3 >= insize");
            return PNG_EMALFORMED;
        }

        /* read block control bits */
        done = read_bit(&bp, &in[inpos]);
        btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

        /* process control type appropriateyly */
        if (btype == 3) {
            log_e("btype == 3");
            png_error = PNG_EMALFORMED;
            return png_error;
        } else if (btype == 0) {
            inflate_uncompressed(out, outsize, &in[inpos], &bp, &pos, insize); /*no compression */
        } else {
            inflate_huffman(out, outsize, &in[inpos], &bp, &pos, insize, btype); /*compression, btype 01 or 10 */
        }

        /* stop if an error has occured */
        if (png_error != PNG_EOK) { return png_error; }
    }

    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t TFT_DSI::uz_inflate(char* out, uint32_t outsize, const char* in, uint32_t insize) {
    /* we require two bytes for the zlib data header */
    if (insize < 2) {
        log_e("insize < 2");
        return PNG_EMALFORMED;
    }

    /* 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way */
    if ((in[0] * 256 + in[1]) % 31 != 0) {
        log_e("FCHECK value is supposed to be made that way");
        return PNG_EMALFORMED;
    }

    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec */
    if ((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7) {
        log_e("only compression method 8: inflate with sliding window of 32k is supported by the PNG spec");
        return PNG_EMALFORMED;
    }

    /* the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset
     * dictionary." */
    if (((in[1] >> 5) & 1) != 0) {
        log_e("The additional flags shall not specify a preset dictionary.");
        return PNG_EMALFORMED;
    }

    /* create output buffer */
    uz_inflate_data(out, outsize, in, insize, 2);

    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Paeth predicter, used by PNG filter type 4*/
int TFT_DSI::paeth_predictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = p > a ? p - a : a - p;
    int pb = p > b ? p - b : b - p;
    int pc = p > c ? p - c : c - p;

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::unfilter_scanline(char* recon, const char* scanline, const char* precon, uint32_t bytewidth, unsigned char filterType, uint32_t length) {
    /*
       For PNG filter method 0
       unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1) precon is the previous unfiltered scanline, recon the
       result, scanline the current one the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead recon and scanline MAY be the same
       memory address! precon must be disjoint.
     */

    uint32_t i;
    switch (filterType) {
        case 0:
            for (i = 0; i < length; i++) recon[i] = scanline[i];
            break;
        case 1:
            for (i = 0; i < bytewidth; i++) recon[i] = scanline[i];
            for (i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
            break;
        case 2:
            if (precon)
                for (i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
            else
                for (i = 0; i < length; i++) recon[i] = scanline[i];
            break;
        case 3:
            if (precon) {
                for (i = 0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
                for (i = bytewidth; i < length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
            } else {
                for (i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                for (i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
            }
            break;
        case 4:
            if (precon) {
                for (i = 0; i < bytewidth; i++) recon[i] = (unsigned char)(scanline[i] + paeth_predictor(0, precon[i], 0));
                for (i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paeth_predictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
            } else {
                for (i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                for (i = bytewidth; i < length; i++) recon[i] = (unsigned char)(scanline[i] + paeth_predictor(recon[i - bytewidth], 0, 0));
            }
            break;
        default:
            log_e("recon: %s, scanline: %s, precon: %s, bytewidth: %d, length: %d, filterType: %d", recon, scanline, precon, bytewidth, length, filterType);
            png_error = PNG_EMALFORMED;
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::unfilter(char* out, const char* in, unsigned w, unsigned h, unsigned bpp) {
    /*
       For PNG filter method 0
       this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times) out must have enough bytes allocated already, in must have the
       scanlines + 1 filtertype byte per scanline w and h are image dimensions or dimensions of reduced image, bpp is bpp per pixel in and out are allowed to be the same memory address!
     */

    unsigned y;
    char*    prevline = 0;

    uint32_t bytewidth = (bpp + 7) / 8; /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise */
    uint32_t linebytes = (w * bpp + 7) / 8;

    for (y = 0; y < h; y++) {
        uint32_t      outindex = linebytes * y;
        uint32_t      inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row */
        unsigned char filterType = in[inindex];

        unfilter_scanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
        if (png_error != PNG_EOK) { return; }

        prevline = &out[outindex];
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::remove_padding_bits(char* out, const char* in, uint32_t olinebits, uint32_t ilinebits, unsigned h) {
    /*
       After filtering there are still padding bpp if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working
       with pure image buffers for the Adam7 code, the color convert code and the output to the user. in and out are allowed to be the same buffer, in may also be higher but still overlapping;
       in must have >= ilinebits*h bpp, out must have >= olinebits*h bpp, olinebits must be <= ilinebits also used to move bpp after earlier such operations happened, e.g. in a sequence of
       reduced images from Adam7 only useful if (ilinebits - olinebits) is a value in the range 1..7
     */
    unsigned y;
    uint32_t diff = ilinebits - olinebits;
    uint32_t obp = 0, ibp = 0; /*bit pointers */
    for (y = 0; y < h; y++) {
        uint32_t x;
        for (x = 0; x < olinebits; x++) {
            unsigned char bit = (unsigned char)((in[(ibp) >> 3] >> (7 - ((ibp) & 0x7))) & 1);
            ibp++;

            if (bit == 0)
                out[(obp) >> 3] &= (unsigned char)(~(1 << (7 - ((obp) & 0x7))));
            else
                out[(obp) >> 3] |= (1 << (7 - ((obp) & 0x7)));
            ++obp;
        }
        ibp += diff;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from the IDAT chunks*/
void TFT_DSI::post_process_scanlines(char* out, char* in) {
    unsigned bpp = png_get_bpp();
    unsigned w = png_width;
    unsigned h = png_height;

    if (bpp == 0) {
        log_e("bpp == 0");
        png_error = PNG_EMALFORMED;
        return;
    }

    if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
        unfilter(in, in, w, h, bpp);
        if (png_error != PNG_EOK) { return; }
        remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
    } else {
        unfilter(out, in, w, h, bpp); /*we can immediatly filter into the out buffer, no other steps needed */
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
int8_t TFT_DSI::png_decode() {
    const char* chunk;
    char*       compressed;
    char*       inflated;
    uint32_t    compressed_size = 0, compressed_index = 0;
    uint32_t    inflated_size;
    int8_t      error = 0;

    /* if we have an error state, bail now */
    if (error != PNG_EOK) { return error; }

    /* parse the main header, if necessary */
    png_read_header();
    if (error != PNG_EOK) { return error; }

    /* if the state is not HEADER (meaning we are ready to decode the image), stop now */
    if (png_state != PNG_HEADER) { return error; }
    chunk = png_buffer + 33;

    /* scan through the chunks, finding the size of all IDAT chunks, and also verify general well-formed-ness */
    while (chunk < png_buffer + png_size) {
        uint32_t    length;
        const char* data; /*the data in the chunk */
        (void)data;

        /* make sure chunk header is not larger than the total compressed */
        if ((uint32_t)(chunk - png_buffer + 12) > png_size) {
            log_e("png_decode: chunk header is not larger than the total compressed");
            error = PNG_EMALFORMED;
            return error;
        }

        /* get length; sanity check it */
        length = upng_chunk_length(chunk);
        if (length > INT_MAX) {
            log_e("png_decode: chunk length is too large");
            error = PNG_EMALFORMED;
            return error;
        }

        /* make sure chunk header+paylaod is not larger than the total compressed */
        if ((uint32_t)(chunk - png_buffer + length + 12) > png_size) {
            log_e("png_decode: chunk header+paylaod is not larger than the total compressed");
            error = PNG_EMALFORMED;
            return error;
        }

        /* get pointer to payload */
        data = chunk + 8;

        /* parse chunks */
        if (upng_chunk_type(chunk) == CHUNK_IDAT) {
            compressed_size += length;
        } else if (upng_chunk_type(chunk) == CHUNK_IEND) {
            break;
        } else if (upng_chunk_critical(chunk)) {
            log_e("png_decode: unsupported critical chunk type");
            error = PNG_EUNSUPPORTED;
            return error;
        }

        chunk += upng_chunk_length(chunk) + 12;
    }

    /* allocate enough space for the (compressed and filtered) image data */
    compressed = (char*)ps_malloc(compressed_size);
    if (compressed == NULL) {
        log_e("png_decode: out of memory");
        error = PNG_ENOMEM;
        return error;
    }

    /* scan through the chunks again, this time copying the values into
     * our compressed buffer.  there's no reason to validate anything a second time. */
    chunk = png_buffer + 33;
    while (chunk < png_buffer + png_size) {
        uint32_t    length;
        const char* data; /*the data in the chunk */

        length = upng_chunk_length(chunk);
        data = chunk + 8;

        /* parse chunks */
        if (upng_chunk_type(chunk) == CHUNK_IDAT) {
            memcpy(compressed + compressed_index, data, length);
            compressed_index += length;
        } else if (upng_chunk_type(chunk) == CHUNK_IEND) {
            break;
        }

        chunk += upng_chunk_length(chunk) + 12;
    }
    /* allocate space to store inflated (but still filtered) data */
    inflated_size = ((png_width * (png_height * png_get_bpp() + 7)) / 8) + png_height;
    inflated = (char*)ps_malloc(inflated_size);

    if (inflated == NULL) {
        free(compressed);
        log_e("png_decode: out of memory");
        error = PNG_ENOMEM;
        return error;
    }

    /* decompress image data */
    error = uz_inflate(inflated, inflated_size, compressed, compressed_size);
    if (error != PNG_EOK) {
        free(compressed);
        free(inflated);
        return error;
    }

    /* free the compressed compressed data */
    free(compressed);

    /* allocate final image buffer */
    png_outbuff_size = (png_height * png_width * png_get_bpp() + 7) / 8;
    png_outbuffer = (char*)ps_malloc(png_outbuff_size);
    if (png_outbuffer == NULL) {
        free(inflated);
        png_size = 0;
        log_e("png_decode: out of memory");
        error = PNG_ENOMEM;
        return error;
    }

    /* unfilter scanlines */
    post_process_scanlines(png_outbuffer, inflated);

    /* we are done with the inflated data */
    free(inflated);

    if (png_error != PNG_EOK) {
        if (png_outbuffer) {
            free(png_outbuffer);
            png_outbuffer = NULL;
        }
        png_size = 0;
    } else {
        png_state = PNG_DECODED;
    }

    /* we are done with our input buffer; free it */
    if (png_buffer) {
        free(png_buffer);
        png_buffer = NULL;
    }
    png_size = 0;

    png_draw_into_Framebuffer(png_pos_x, png_pos_y, png_width, png_height, png_outbuffer, png_outbuff_size, png_format);

    if (png_outbuffer) {
        free(png_outbuffer);
        png_outbuffer = NULL;
    }
    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_DSI::png_format_t TFT_DSI::png_determine_format() {
    switch (png_color_type) {
        case PNG_LUM:
            switch (png_color_depth) {
                case 1: return PNG_LUMINANCE1;
                case 2: return PNG_LUMINANCE2;
                case 4: return PNG_LUMINANCE4;
                case 8: return PNG_LUMINANCE8;
                default: return PNG_BADFORMAT;
            }
            break;
        case PNG_RGB:
            switch (png_color_depth) {
                case 8: return PNG_RGB8;
                case 16: return PNG_RGB16;
                default: return PNG_BADFORMAT;
            }
            break;
        case PNG_PAL:
            switch (png_color_depth) {
                case 1: return PNG_PALLETTE1;
                case 2: return PNG_PALLETTE2;
                case 4: return PNG_PALLETTE4;
                case 8: return PNG_PALLETTE8;
                default: return PNG_BADFORMAT;
            }
            break;
        case PNG_LUMA:
            switch (png_color_depth) {
                case 1: return PNG_LUMINANCE_ALPHA1;
                case 2: return PNG_LUMINANCE_ALPHA2;
                case 4: return PNG_LUMINANCE_ALPHA4;
                case 8: return PNG_LUMINANCE_ALPHA8;
                default: return PNG_BADFORMAT;
            }
            break;
        case PNG_RGBA:
            switch (png_color_depth) {
                case 8: return PNG_RGBA8;
                case 16: return PNG_RGBA16;
                default: return PNG_BADFORMAT;
            }
            break;
        default: return PNG_BADFORMAT; break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*read the information from the header and store it in the upng_Info. return value is error*/
bool TFT_DSI::png_read_header() {
    png_state = PNG_HEADER;
    if (png_size < 29) {
        png_error = PNG_ENOTPNG;
        log_e("png_size < 29");
        return false;
    }

    if (png_buffer[0] != 137 || png_buffer[1] != 80 || png_buffer[2] != 78 || png_buffer[3] != 71 || /* check that PNG header matches expected value */
        png_buffer[4] != 13 || png_buffer[5] != 10 || png_buffer[6] != 26 || png_buffer[7] != 10) {
        png_error = PNG_ENOTPNG;
        log_e("image data does not have a PNG header");
        return false;
    }

    /* check that the first chunk is the IHDR chunk */
    if (MAKE_DWORD_PTR(png_buffer + 12) != CHUNK_IHDR) {
        png_error = PNG_EMALFORMED;
        log_e("image data is not a valid PNG image");
        return false;
    }

    /* read the values given in the header */
    png_width = MAKE_DWORD_PTR(png_buffer + 16);
    png_height = MAKE_DWORD_PTR(png_buffer + 20);
    png_color_depth = png_buffer[24];
    png_color_type = png_buffer[25];

    /* determine our color format */
    png_format = png_determine_format();
    png_error = png_format == PNG_BADFORMAT ? PNG_EUNFORMAT : PNG_EOK;

    if (png_format == PNG_BADFORMAT) {
        log_e("image color format is not supported");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (only allowed value in spec) */
    if (png_buffer[26] != 0) {
        png_error = PNG_EMALFORMED;
        log_e("image data is not a valid PNG image");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (only allowed value in spec) */
    if (png_buffer[27] != 0) {
        png_error = PNG_EMALFORMED;
        log_e("image data is not a valid PNG image");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (spec allows 1, but uPNG does not support it) */
    if (png_buffer[28] != 0) {
        png_error = PNG_EUNINTERLACED;
        log_e("image interlacing is not supported");
        return false;
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t TFT_DSI::png_get_error() {
    return png_error;
}
uint16_t TFT_DSI::png_get_width() {
    return png_width;
}
uint16_t TFT_DSI::png_get_height() {
    return png_height;
}
uint16_t TFT_DSI::png_get_bpp() {
    return png_get_bitdepth() * png_get_components();
}
const char* TFT_DSI::png_get_outbuffer() {
    return png_outbuffer;
}
uint32_t TFT_DSI::png_get_size() {
    return png_outbuff_size;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_DSI::png_get_components() {
    switch (png_color_type) {
        case PNG_LUM: return 1;
        case PNG_RGB: return 3;
        case PNG_LUMA: return 2;
        case PNG_RGBA: return 4;
        case PNG_PAL: return 1;
        default: return 0;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_DSI::png_get_bitdepth() {
    return png_color_depth;
}
//_______________________________________________________________________________________________________________________________
uint16_t TFT_DSI::png_get_pixelsize() {
    uint16_t bits = png_get_bitdepth() * png_get_components();
    bits += bits % 8;
    return bits;
}

TFT_DSI::png_format_t TFT_DSI::png_get_format() {
    return png_format;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::png_GetPixel(void* pixel, int x, int y) {
    uint32_t bpp = png_get_bpp();
    //    Serial.printf("\nbbp=%i\n",(int)bpp);
    uint32_t Bpp = ((bpp + 7) / 8);
    uint32_t position = (png_width * y + x) * Bpp;
    //    Serial.printf("\nposition in file=%li\n",(long)position);
    memcpy(pixel, png_buffer + position, Bpp);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Initializing color variables */

TFT_DSI::png_s_rgb16b* TFT_DSI::InitColorR5G6B5() {
    png_s_rgb16b* color = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    if (color != 0) { ResetColor(color); }
    return color;
}
TFT_DSI::png_s_rgb18b* TFT_DSI::InitColorR6G6B6() {
    png_s_rgb18b* color = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    if (color != 0) { ResetColor(color); }
    return color;
}
TFT_DSI::png_s_rgb24b* TFT_DSI::InitColorR8G8B8() {
    png_s_rgb24b* color = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    if (color != 0) { ResetColor(color); }
    return color;
}

void TFT_DSI::InitColor(png_s_rgb16b** dst) {
    *dst = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    ResetColor(*dst);
}
void TFT_DSI::InitColor(png_s_rgb18b** dst) {
    *dst = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    ResetColor(*dst);
}
void TFT_DSI::InitColor(png_s_rgb24b** dst) {
    *dst = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    ResetColor(*dst);
}

void TFT_DSI::ResetColor(png_s_rgb16b* dst) {
    *dst = (png_s_rgb16b){0, 0, 0, 0};
}
void TFT_DSI::ResetColor(png_s_rgb18b* dst) {
    *dst = (png_s_rgb18b){0, 0, 0, 0};
}
void TFT_DSI::ResetColor(png_s_rgb24b* dst) {
    *dst = (png_s_rgb24b){0, 0, 0, 0};
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Converting between colors*/

void TFT_DSI::png_rgb24bto18b(png_s_rgb18b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 2; // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 2; // 3;//2;
}

void TFT_DSI::png_rgb24bto16b(png_s_rgb16b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 3; // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 3; // 3;//2;
}
void TFT_DSI::png_rgb18btouint32(uint32_t* dst, png_s_rgb18b* src) {
    memcpy(dst, src, sizeof(png_s_rgb18b));
}
void TFT_DSI::png_rgb16btouint32(uint32_t* dst, png_s_rgb16b* src) {
    memcpy(dst, src, sizeof(png_s_rgb16b));
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_DSI::png_draw_into_Framebuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char* rgbaBuffer, uint32_t, uint8_t) {
    if (!rgbaBuffer || w == 0 || h == 0) return;

    uint16_t* rgbBuffer = (uint16_t*)ps_malloc(w * h * sizeof(uint16_t));

    uint8_t* alphaBuffer = (uint8_t*)ps_malloc(w * h);

    if (!rgbBuffer || !alphaBuffer) return;

    for (uint32_t i = 0; i < w * h; i++) {
        uint8_t r = rgbaBuffer[i * 4 + 0];
        uint8_t g = rgbaBuffer[i * 4 + 1];
        uint8_t b = rgbaBuffer[i * 4 + 2];
        uint8_t a = rgbaBuffer[i * 4 + 3];

        rgbBuffer[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);

        alphaBuffer[i] = a;
    }

    renderRGB565(x, y, w, h, rgbBuffer, alphaBuffer);

    free(rgbBuffer);
    free(alphaBuffer);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#endif // CONFIG_IDF_TARGET_ESP32P4
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
