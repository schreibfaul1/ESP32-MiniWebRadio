// first release on 01/2025
// updated on Mar 17 2026

#include "Arduino.h"
#ifdef CONFIG_IDF_TARGET_ESP32S3
#include "tft_rgb.h"

#define __malloc_heap_psram(size) heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_RGB::TFT_RGB() { // Constructor
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
void TFT_RGB::loop(){
    if(!m_refresh) handle_vsync_event(m_panel, nullptr);
    GIF_loop();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx) { // static callback function
    if (user_ctx) {
        TFT_RGB* instance = static_cast<TFT_RGB*>(user_ctx);
        return instance->handle_vsync_event(panel, edata);
    }
    return false;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::handle_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *edata) {
 //   if (xSemaphoreTake(m_vsync_semaphore, 0) == pdTRUE) {
        int res = esp_lcd_rgb_panel_refresh(m_panel);
 //       xSemaphoreGive(m_vsync_semaphore);
        if(res == ESP_OK){
            m_refresh = true;
            m_vsyncCounter++;
            return true;
        }
 //   }
    m_refresh = false;
    return false;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::begin(const Pins& newPins, const Timing& newTiming) {

    m_pins = newPins;
    m_timing = newTiming;

    esp_lcd_rgb_panel_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));
    panel_config.clk_src = LCD_CLK_SRC_PLL160M;

    panel_config.timings.pclk_hz = m_timing.pixel_clock_hz;
    panel_config.timings.h_res = m_timing.h_res;
    panel_config.timings.v_res = m_timing.v_res;
    panel_config.timings.hsync_pulse_width = m_timing.hsync_pulse_width;
    panel_config.timings.hsync_back_porch = m_timing.hsync_back_porch;
    panel_config.timings.hsync_front_porch = m_timing.hsync_front_porch;
    panel_config.timings.vsync_pulse_width = m_timing.vsync_pulse_width;
    panel_config.timings.vsync_back_porch = m_timing.vsync_back_porch;
    panel_config.timings.vsync_front_porch = m_timing.vsync_front_porch;
    panel_config.timings.flags.hsync_idle_low = true;
    panel_config.timings.flags.vsync_idle_low = true;
    panel_config.timings.flags.de_idle_high = false;
    panel_config.timings.flags.pclk_active_neg = true;
    panel_config.timings.flags.pclk_idle_high = true;

    panel_config.data_width = 16; // RGB565
 //   panel_config.bits_per_pixel = 16;
    panel_config.num_fbs = 3;
    panel_config.bounce_buffer_size_px = 0;
    panel_config.dma_burst_size = 64;
    panel_config.hsync_gpio_num = (gpio_num_t)m_pins.hsync;
    panel_config.vsync_gpio_num = (gpio_num_t)m_pins.vsync;
    panel_config.de_gpio_num = (gpio_num_t)m_pins.de;
    panel_config.pclk_gpio_num = (gpio_num_t)m_pins.pclk;
    panel_config.disp_gpio_num = (gpio_num_t)m_pins.bl;

    int8_t pinArr[16] = {               // Daten-Pins für RGB565
        m_pins.b0, m_pins.b1, m_pins.b2, m_pins.b3, m_pins.b4,
        m_pins.g0, m_pins.g1, m_pins.g2, m_pins.g3, m_pins.g4, m_pins.g5,
        m_pins.r0, m_pins.r1, m_pins.r2, m_pins.r3, m_pins.r4
    };

    for (int i = 0; i < 16; ++i) {
        //  log_i("i %i. pin %i", i, pinArr[i]);
        panel_config.data_gpio_nums[i] = (gpio_num_t)pinArr[i];
    }
    panel_config.flags.disp_active_low = false;
    panel_config.flags.refresh_on_demand = true;
    panel_config.flags.fb_in_psram = true;
    panel_config.flags.double_fb = false;
    panel_config.flags.no_fb = false;
    panel_config.flags.bb_invalidate_cache = false;

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &m_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(m_panel));
    if(tft_info) tft_info("Display initialisiert.");

    // Hintergrundbeleuchtung einschalten
    if(m_pins.bl >= 0){
        gpio_set_direction((gpio_num_t)m_pins.bl, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)m_pins.bl, 1); // Hintergrundbeleuchtung aktivieren
    }
    m_h_res = m_timing.h_res;
    m_v_res = m_timing.v_res;

    void *fb0, *fb1, *fb2;
    esp_lcd_rgb_panel_get_frame_buffer(m_panel, 3, &fb0, &fb1, &fb2);
    m_framebuffer[0] = (uint16_t*)fb0;
    m_framebuffer[1] = (uint16_t*)fb1;
    m_framebuffer[2] = (uint16_t*)fb2;

    // log_w("m_h_res: %d, m_v_res: %d, m_framebuffer[0] %i", m_h_res, m_v_res, m_framebuffer[0]);
    memset(m_framebuffer[0], 0xFF, m_h_res * m_v_res * 2);
    // log_w("m_h_res: %d, m_v_res: %d, m_framebuffer[1] %i", m_h_res, m_v_res, m_framebuffer[1]);
    memset(m_framebuffer[1], 0xFF, m_h_res * m_v_res * 2);
    // log_w("m_h_res: %d, m_v_res: %d, m_framebuffer[2] %i", m_h_res, m_v_res, m_framebuffer[2]);
    memset(m_framebuffer[2], 0xFF, m_h_res * m_v_res * 2);

    esp_lcd_rgb_panel_event_callbacks_t cbs = { // if not used, set to nullptr
        .on_color_trans_done = nullptr,
        .on_vsync = TFT_RGB::on_vsync_event,
        .on_bounce_empty = nullptr,           /*!< Bounce buffer empty callback. */
        .on_frame_buf_complete = nullptr,      /*!< A whole frame buffer was just sent to the LCD DMA */
    };
    esp_lcd_rgb_panel_register_event_callbacks(m_panel, &cbs, this);
    esp_lcd_rgb_panel_refresh(m_panel);
    m_rowBuffer = (uint8_t*)ps_malloc(m_rowBufferSize);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::reset() {
    ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panel));
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void *bitmap) {
    bool res = false;
    if(x0 >= x1 || y0 >= y1) {log_w("%s %i: x0 %i, y0 %i, x1 %i, y1 %i", __FILE__, __LINE__, x0, y0, x1, y1); return false;}
    xSemaphoreTake(m_vsync_semaphore, 0.3 * configTICK_RATE_HZ);
    res = esp_lcd_panel_draw_bitmap(m_panel, x0, y0, x1, y1, (const uint16_t*)bitmap);
    xSemaphoreGive(m_vsync_semaphore);
    return res;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::setDisplayInversion(bool invert) {
    esp_lcd_panel_invert_color(m_panel, invert);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::setRotation(uint8_t r) {
    if (r >= 4) return;
    m_rotation = r;
    uint16_t deg = r * 90;
    char     buff[256];
    sprintf(buff, "Rotation: " ANSI_ESC_CYAN "%d° CW" ANSI_ESC_RESET " logical width: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET " logical height: " ANSI_ESC_CYAN "%d" ANSI_ESC_RESET, deg, logicalWidth(),
            logicalHeight());
    if (tft_info) tft_info(buff);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data) {
    // Check whether parameters are within the valid range
    if (x < 0 || y < 0 || w <= 0 || h <= 0) return;
    if (x + w > logicalWidth() || y + h > logicalHeight()) return; // logicalWidth() = vertical resolution
    if (!data || !m_framebuffer[0]) return;

    uint16_t* dst = data;
    uint16_t* src = m_framebuffer[0] + y * logicalWidth() + x;

    for (int32_t row = 0; row < h; row++) {
        memcpy(dst, src, w * sizeof(uint16_t));
        src += logicalWidth();
        dst += w;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#endif // CONFIG_IDF_TARGET_ESP32S3
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
