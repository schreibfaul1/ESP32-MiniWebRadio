// first release on 01/2025
// updated on Feb 12 2025


#include "Arduino.h"
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
    xSemaphoreGive(m_vsync_semaphore);
}
void TFT_RGB::loop(){
    if(!m_refresh) handle_vsync_event(m_panel, nullptr);
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
    if (xSemaphoreTake(m_vsync_semaphore, 0) == pdTRUE) {
        esp_lcd_rgb_panel_refresh(m_panel);
        xSemaphoreGive(m_vsync_semaphore);
        m_refresh = true;
        return true;
    }
    m_refresh = false;
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::begin(const Pins& newPins, const Timing& newTiming) {

    m_pins = newPins;
    m_timing = newTiming;

    esp_lcd_rgb_panel_config_t panel_config;
    panel_config.clk_src = LCD_CLK_SRC_PLL240M;

    panel_config.timings.pclk_hz = m_timing.pixel_clock_hz;
    panel_config.timings.h_res = m_timing.h_res;
    panel_config.timings.v_res = m_timing.v_res;
    panel_config.timings.hsync_pulse_width = m_timing.hsync_pulse_width;
    panel_config.timings.hsync_back_porch = m_timing.hsync_back_porch;
    panel_config.timings.hsync_front_porch = m_timing.hsync_front_porch;
    panel_config.timings.vsync_pulse_width = m_timing.vsync_pulse_width;
    panel_config.timings.vsync_back_porch = m_timing.vsync_back_porch;
    panel_config.timings.vsync_front_porch = m_timing.vsync_front_porch;
    panel_config.timings.flags.hsync_idle_low = false;
    panel_config.timings.flags.vsync_idle_low = false;
    panel_config.timings.flags.de_idle_high = false;
    panel_config.timings.flags.pclk_active_neg = true;
    panel_config.timings.flags.pclk_idle_high = false;

    panel_config.data_width = 16; // RGB565
    panel_config.bits_per_pixel = 16;
    panel_config.num_fbs = 3;
    panel_config.bounce_buffer_size_px = 0;
    panel_config.dma_burst_size = 64;
    panel_config.hsync_gpio_num = m_pins.hsync;
    panel_config.vsync_gpio_num = m_pins.vsync;
    panel_config.de_gpio_num = m_pins.de;
    panel_config.pclk_gpio_num = m_pins.pclk;
    panel_config.disp_gpio_num = m_pins.bl;

    int8_t pinArr[16] = {               // Daten-Pins für RGB565
        m_pins.b0, m_pins.b1, m_pins.b2, m_pins.b3, m_pins.b4,
        m_pins.g0, m_pins.g1, m_pins.g2, m_pins.g3, m_pins.g4, m_pins.g5,
        m_pins.r0, m_pins.r1, m_pins.r2, m_pins.r3, m_pins.r4
    };

    for (int i = 0; i < 16; ++i) {
        //  log_i("i %i. pin %i", i, pinArr[i]);
        panel_config.data_gpio_nums[i] = pinArr[i];
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
    gpio_set_direction((gpio_num_t)m_pins.bl, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)m_pins.bl, 1); // Hintergrundbeleuchtung aktivieren

    m_h_res = m_timing.h_res;
    m_v_res = m_timing.v_res;

    void *fb0, *fb1, *fb2;
    esp_lcd_rgb_panel_get_frame_buffer(m_panel, 3, &fb0, &fb1, &fb2);
    m_framebuffer[0] = (uint16_t*)fb0;
    m_framebuffer[1] = (uint16_t*)fb1;
    m_framebuffer[2] = (uint16_t*)fb2;

    log_e("m_h_res: %d, m_v_res: %d, m_framebuffer[0] %i", m_h_res, m_v_res, m_framebuffer[0]);
    memset(m_framebuffer[0], 0xFF, m_h_res * m_v_res * 2);
    log_e("m_h_res: %d, m_v_res: %d, m_framebuffer[1] %i", m_h_res, m_v_res, m_framebuffer[1]);
    memset(m_framebuffer[1], 0xFF, m_h_res * m_v_res * 2);
    log_e("m_h_res: %d, m_v_res: %d, m_framebuffer[2] %i", m_h_res, m_v_res, m_framebuffer[2]);
    memset(m_framebuffer[2], 0xFF, m_h_res * m_v_res * 2);

    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_color_trans_done = nullptr,  // if not used, set to nullptr
        .on_vsync = TFT_RGB::on_vsync_event,
        .on_bounce_empty = nullptr,
        .on_frame_buf_complete = nullptr,
    };
    esp_lcd_rgb_panel_register_event_callbacks(m_panel, &cbs, this);
    esp_lcd_rgb_panel_refresh(m_panel);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void *bitmap) {
    bool res = false;
    if(x0 >= x1 || y0 >= y1) {log_w("x0 %i, y0 %i, x1 %i, y1 %i", x0, y0, x1, y1); return false;}
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

void TFT_RGB::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    // Calculate differences
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);

    // Determine directions
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;

    int16_t err = dx - dy; // Fehlerterm

    while (true) {
        // Set point in framebuffer if within bounds
        if (x0 >= 0 && x0 < m_h_res && y0 >= 0 && y0 < m_v_res) {
            m_framebuffer[0][y0 * m_h_res + x0] = color;
        }

        // Wenn Endpunkt erreicht, beenden
        if (x0 == x1 && y0 == y1) break;

        // Add error term twice and correct errors
        int16_t e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    // Update on the display (only the drawn area)
    int16_t update_x0 = std::min(x0, x1);
    int16_t update_y0 = std::min(y0, y1);
    int16_t update_x1 = std::max(x0, x1) + 1;
    int16_t update_y1 = std::max(y0, y1) + 1;
    panelDrawBitmap(update_x0, update_y0, update_x1, update_y1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Clipping: Rechteck-Koordinaten auf den Framebuffer-Bereich beschränken
    int16_t x0 = max((int16_t)0, x);
    int16_t y0 = max((int16_t)0, y);
    int16_t x1 = min((int)m_h_res, x + w); // Rechte Grenze
    int16_t y1 = min((int)m_v_res, y + h); // Untere Grenze
    // Zeichnen des Rechtecks nur im gültigen Bereich
    for (int16_t j = y0; j < y1; ++j) { // Zeilen iterieren
        for (int16_t i = x0; i < x1; ++i) { // Spalten iterieren
            m_framebuffer[0][j * m_h_res + i] = color;
        }
    }
    panelDrawBitmap(x0, y0, x1, y1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::fillScreen(uint16_t color) {
    fillRect(0, 0, m_h_res, m_v_res, color);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {

    auto drawLine = [](int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t* m_framebuffer[0], uint16_t m_h_res) {
        // Bresenham-Algorithmus für Linien
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2; // Fehlerwert

        while (true) {
            m_framebuffer[0][y0 * m_h_res + x0] = color; // Pixel setzen
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };


    // Zeichne die drei Linien des Dreiecks
    drawLine(x0, y0, x1, y1, color, &m_framebuffer[0], m_h_res); // Linie von Punkt 0 nach Punkt 1
    drawLine(x1, y1, x2, y2, color, &m_framebuffer[0], m_h_res); // Linie von Punkt 1 nach Punkt 2
    drawLine(x2, y2, x0, y0, color, &m_framebuffer[0], m_h_res); // Linie von Punkt 2 nach Punkt 0

    // Aktualisierung des gezeichneten Bereichs
    int16_t x_min = std::min({x0, x1, x2});
    int16_t y_min = std::min({y0, y1, y2});
    int16_t x_max = std::max({x0, x1, x2});
    int16_t y_max = std::max({y0, y1, y2});
    panelDrawBitmap(x_min, y_min, x_max + 1, y_max + 1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
 // Helferfunktion zum Zeichnen einer horizontalen Linie
    auto drawHorizontalLine = [&](int16_t x_start, int16_t x_end, int16_t y) {
        if (y >= 0 && y < m_v_res) { // Clipping in y-Richtung
            if (x_start > x_end) std::swap(x_start, x_end);
            x_start = std::max((int16_t)0, x_start); // Clipping in x-Richtung
            x_end = std::min((int16_t)(m_h_res - 1), x_end);
            for (int16_t x = x_start; x <= x_end; ++x) {
                m_framebuffer[0][y * m_h_res + x] = color;
            }
        }
    };

    // Punkte nach ihrer y-Koordinate sortieren
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

    // Variablen zur Begrenzung des aktualisierten Bereichs
    int16_t x_min = std::min({x0, x1, x2});
    int16_t x_max = std::max({x0, x1, x2});
    int16_t y_min = std::min({y0, y1, y2});
    int16_t y_max = std::max({y0, y1, y2});

    // Clipping auf Framebuffer-Grenzen
    x_min = std::max((int16_t)0, x_min);
    x_max = std::min((int16_t)(m_h_res - 1), x_max);
    y_min = std::max((int16_t)0, y_min);
    y_max = std::min((int16_t)(m_v_res - 1), y_max);

    // Dreieck in zwei Teile zerlegen (oben und unten)
    if (y1 == y2) { // Sonderfall: flaches unteres Dreieck
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t x_start = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
    } else if (y0 == y1) { // Sonderfall: flaches oberes Dreieck
        for (int16_t y = y0; y <= y2; ++y) {
            int16_t x_start = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            int16_t x_end = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            drawHorizontalLine(x_start, x_end, y);
        }
    } else { // Allgemeiner Fall: Dreieck wird in zwei Teile aufgeteilt
        for (int16_t y = y0; y <= y1; ++y) { // Unterer Teil
            int16_t x_start = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
        for (int16_t y = y1; y <= y2; ++y) { // Oberer Teil
            int16_t x_start = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
    }

    // Aktualisierung nur des geänderten Bereichs
    panelDrawBitmap(x_min, y_min, x_max + 1, y_max + 1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color) {
    if(Xpos < 0 || Xpos >= m_h_res || Ypos < 0 || Ypos >= m_v_res) return;
    if(Width == 0 || Height == 0) return;
    if(Width > m_h_res - Xpos) Width = m_h_res - Xpos;
    if(Height > m_v_res - Ypos) Height = m_v_res - Ypos;

    auto drawLine = [](int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t* fb, uint16_t m_h_res) {
        // Bresenham-Algorithmus für Linien
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2; // Fehlerwert

        while (true) {
            fb[y0 * m_h_res + x0] = color; // Pixel setzen
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };

    // Zeichne die vier Linien des Rechtecks
    drawLine(Xpos, Ypos, Xpos + Width - 1, Ypos, Color, m_framebuffer[0], m_h_res); // Oben
    drawLine(Xpos + Width - 1, Ypos, Xpos + Width - 1, Ypos + Height - 1, Color, m_framebuffer[0], m_h_res); // Rechts
    drawLine(Xpos, Ypos + Height - 1, Xpos + Width - 1, Ypos + Height - 1, Color, m_framebuffer[0], m_h_res); // Unten
    drawLine(Xpos, Ypos + Height - 1, Xpos, Ypos, Color, m_framebuffer[0], m_h_res); // Links

    // Aktualisierung des gezeichneten Bereichs
    int16_t x0 = std::min((int)Xpos, Xpos + Width);
    int16_t y0 = std::min((int)Ypos, Ypos + Height);
    int16_t x1 = std::max((int)Xpos, Xpos + Width);
    int16_t y1 = std::max((int)Ypos, Ypos + Height);

    panelDrawBitmap(x0, y0, x1, y1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Helferfunktion: Kreislinie für die Ecken berechnen
    auto drawCircleQuadrant = [&](int16_t cx, int16_t cy, int16_t r, uint8_t quadrant) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            if (quadrant & 0x1) m_framebuffer[0][(cy - y) * m_h_res + (cx + x)] = color; // oben rechts
            if (quadrant & 0x2) m_framebuffer[0][(cy + y) * m_h_res + (cx + x)] = color; // unten rechts
            if (quadrant & 0x4) m_framebuffer[0][(cy + y) * m_h_res + (cx - x)] = color; // unten links
            if (quadrant & 0x8) m_framebuffer[0][(cy - y) * m_h_res + (cx - x)] = color; // oben links

            if (quadrant & 0x10) m_framebuffer[0][(cy - x) * m_h_res + (cx + y)] = color; // oben rechts (90° gedreht)
            if (quadrant & 0x20) m_framebuffer[0][(cy + x) * m_h_res + (cx + y)] = color; // unten rechts (90° gedreht)
            if (quadrant & 0x40) m_framebuffer[0][(cy + x) * m_h_res + (cx - y)] = color; // unten links (90° gedreht)
            if (quadrant & 0x80) m_framebuffer[0][(cy - x) * m_h_res + (cx - y)] = color; // oben links (90° gedreht)

            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
        }
    };

    // Rechteckseiten zeichnen (ohne die abgerundeten Ecken)
    for (int16_t i = x + r; i < x + w - r; i++) { // Obere und untere horizontale Linien
        m_framebuffer[0][y * m_h_res + i] = color; // Oben
        m_framebuffer[0][(y + h - 1) * m_h_res + i] = color; // Unten
    }
    for (int16_t i = y + r; i < y + h - r; i++) { // Linke und rechte vertikale Linien
        m_framebuffer[0][i * m_h_res + x] = color; // Links
        m_framebuffer[0][i * m_h_res + (x + w - 1)] = color; // Rechts
    }

    // Abgerundete Ecken zeichnen
    drawCircleQuadrant(x + w - r - 1, y + r, r, 0x1 | 0x10); // Oben rechts
    drawCircleQuadrant(x + w - r - 1, y + h - r - 1, r, 0x2 | 0x20); // Unten rechts
    drawCircleQuadrant(x + r, y + h - r - 1, r, 0x4 | 0x40); // Unten links
    drawCircleQuadrant(x + r, y + r, r, 0x8 | 0x80); // Oben links

    // Aktualisierung des gezeichneten Bereichs
    panelDrawBitmap(x, y, x + w, y + h, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Helferfunktion: Kreisfüllung für die Ecken berechnen
    auto fillCircleQuadrant = [&](int16_t cx, int16_t cy, int16_t r, uint8_t quadrant) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            for (int16_t i = 0; i <= x; i++) {
                if (quadrant & 0x1) m_framebuffer[0][(cy - y) * m_h_res + (cx + i)] = color; // oben rechts
                if (quadrant & 0x2) m_framebuffer[0][(cy + y) * m_h_res + (cx + i)] = color; // unten rechts
                if (quadrant & 0x4) m_framebuffer[0][(cy + y) * m_h_res + (cx - i)] = color; // unten links
                if (quadrant & 0x8) m_framebuffer[0][(cy - y) * m_h_res + (cx - i)] = color; // oben links
            }
            for (int16_t i = 0; i <= y; i++) {
                if (quadrant & 0x10) m_framebuffer[0][(cy - x) * m_h_res + (cx + i)] = color; // oben rechts (gedreht)
                if (quadrant & 0x20) m_framebuffer[0][(cy + x) * m_h_res + (cx + i)] = color; // unten rechts (gedreht)
                if (quadrant & 0x40) m_framebuffer[0][(cy + x) * m_h_res + (cx - i)] = color; // unten links (gedreht)
                if (quadrant & 0x80) m_framebuffer[0][(cy - x) * m_h_res + (cx - i)] = color; // oben links (gedreht)
            }

            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
        }
    };

    // Horizontale Bereiche zwischen den oberen und unteren Viertelkreisen füllen
    for (int16_t i = y; i < y + r; i++) { // Bereich oberhalb der Viertelkreise
        for (int16_t j = x + r; j < x + w - r; j++) {
            m_framebuffer[0][i * m_h_res + j] = color;
        }
    }
    for (int16_t i = y + h - r; i < y + h; i++) { // Bereich unterhalb der Viertelkreise
        for (int16_t j = x + r; j < x + w - r; j++) {
            m_framebuffer[0][i * m_h_res + j] = color;
        }
    }

    // Vertikaler Bereich zwischen den Viertelkreisen füllen
    for (int16_t i = y + r; i < y + h - r; i++) { // Vertikaler Bereich
        for (int16_t j = x; j < x + w; j++) { // Horizontaler Bereich
            m_framebuffer[0][i * m_h_res + j] = color;
        }
    }

    // Viertelkreise in den Ecken füllen
    fillCircleQuadrant(x + w - r - 1, y + r, r, 0x1 | 0x10); // Oben rechts
    fillCircleQuadrant(x + w - r - 1, y + h - r - 1, r, 0x2 | 0x20); // Unten rechts
    fillCircleQuadrant(x + r, y + h - r - 1, r, 0x4 | 0x40); // Unten links
    fillCircleQuadrant(x + r, y + r, r, 0x8 | 0x80); // Oben links

    // Aktualisierung des gezeichneten Bereichs
    panelDrawBitmap(x, y, x + w, y + h, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    // Bresenham-Algorithmus für Kreise
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    // Setze die Anfangspunkte (Symmetrieachsen)
    m_framebuffer[0][(cy + r) * m_h_res + cx] = color; // Oben
    m_framebuffer[0][(cy - r) * m_h_res + cx] = color; // Unten
    m_framebuffer[0][cy * m_h_res + (cx + r)] = color; // Rechts
    m_framebuffer[0][cy * m_h_res + (cx - r)] = color; // Links

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // Punkte in den acht Symmetrieachsen zeichnen
        m_framebuffer[0][(cy + y) * m_h_res + (cx + x)] = color; // Quadrant 1
        m_framebuffer[0][(cy + y) * m_h_res + (cx - x)] = color; // Quadrant 2
        m_framebuffer[0][(cy - y) * m_h_res + (cx + x)] = color; // Quadrant 3
        m_framebuffer[0][(cy - y) * m_h_res + (cx - x)] = color; // Quadrant 4
        m_framebuffer[0][(cy + x) * m_h_res + (cx + y)] = color; // Quadrant 5
        m_framebuffer[0][(cy + x) * m_h_res + (cx - y)] = color; // Quadrant 6
        m_framebuffer[0][(cy - x) * m_h_res + (cx + y)] = color; // Quadrant 7
        m_framebuffer[0][(cy - x) * m_h_res + (cx - y)] = color; // Quadrant 8
    }

    // Aktualisierung des gezeichneten Bereichs
    panelDrawBitmap(cx - r, cy - r, cx + r + 1, cy + r + 1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::fillCircle(int16_t Xm, int16_t Ym, uint16_t r, uint16_t color){
//void TFT_RGB::drawFilledCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    // Bresenham-Algorithmus für Kreise
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    // Fülle die erste vertikale Linie durch den Mittelpunkt
    for (int16_t i = Ym - r; i <= Ym + r; i++) {
        m_framebuffer[0][i * m_h_res + Xm] = color;
    }

    while (x <= y) {
        // Fülle horizontale Linien für alle acht Symmetrieachsen
        for (int16_t i = Xm - x; i <= Xm + x; i++) {
            m_framebuffer[0][(Ym + y) * m_h_res + i] = color; // Unten +y
            m_framebuffer[0][(Ym - y) * m_h_res + i] = color; // Oben -y
        }
        for (int16_t i = Xm - y; i <= Xm + y; i++) {
            m_framebuffer[0][(Ym + x) * m_h_res + i] = color; // Rechts +x
            m_framebuffer[0][(Ym - x) * m_h_res + i] = color; // Links -x
        }

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
    }

    // Aktualisierung des gezeichneten Bereichs
    panelDrawBitmap(Xm - r, Ym - r, Xm + r + 1, Ym + r + 1, m_framebuffer[0]);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::copyFramebuffer(uint8_t source, uint8_t destination, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for(uint16_t j = y; j < y + h; j++) {
        memcpy(m_framebuffer[destination] + j * m_h_res + x, m_framebuffer[source] + j * m_h_res + x, w * 2);
    }
    if(destination == 0) panelDrawBitmap(x, y, x + w, y + h, m_framebuffer[0]); // just draw when copied in fb0
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data) {
    memcpy(data, m_framebuffer[0] + y * m_h_res + x, w * sizeof(uint16_t));
    return;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::setFont(uint16_t font) {

#ifdef TFT_TIMES_NEW_ROMAN
    switch(font) {
        case 15:
            m_current_font.cmaps = cmaps_Times15;
            m_current_font.glyph_bitmap = glyph_bitmap_Times15;
            m_current_font.glyph_dsc = glyph_dsc_Times15;
            m_current_font.range_start = cmaps_Times15->range_start;
            m_current_font.range_length = cmaps_Times15->range_length;
            m_current_font.line_height = cmaps_Times15->line_height;
            m_current_font.font_height = cmaps_Times15->font_height;
            m_current_font.base_line = cmaps_Times15->base_line;
            m_current_font.lookup_table = cmaps_Times15->lookup_table;
            break;
        case 16:
            m_current_font.cmaps = cmaps_Times16;
            m_current_font.glyph_bitmap = glyph_bitmap_Times16;
            m_current_font.glyph_dsc = glyph_dsc_Times16;
            m_current_font.range_start = cmaps_Times16->range_start;
            m_current_font.range_length = cmaps_Times16->range_length;
            m_current_font.line_height = cmaps_Times16->line_height;
            m_current_font.font_height = cmaps_Times16->font_height;
            m_current_font.base_line = cmaps_Times16->base_line;
            m_current_font.lookup_table = cmaps_Times16->lookup_table;
            break;
        case 18:
            m_current_font.cmaps = cmaps_Times18;
            m_current_font.glyph_bitmap = glyph_bitmap_Times18;
            m_current_font.glyph_dsc = glyph_dsc_Times18;
            m_current_font.range_start = cmaps_Times18->range_start;
            m_current_font.range_length = cmaps_Times18->range_length;
            m_current_font.line_height = cmaps_Times18->line_height;
            m_current_font.font_height = cmaps_Times18->font_height;
            m_current_font.base_line = cmaps_Times18->base_line;
            m_current_font.lookup_table = cmaps_Times18->lookup_table;
            break;
        case 21:
            m_current_font.cmaps = cmaps_Times21;
            m_current_font.glyph_bitmap = glyph_bitmap_Times21;
            m_current_font.glyph_dsc = glyph_dsc_Times21;
            m_current_font.range_start = cmaps_Times21->range_start;
            m_current_font.range_length = cmaps_Times21->range_length;
            m_current_font.line_height = cmaps_Times21->line_height;
            m_current_font.font_height = cmaps_Times21->font_height;
            m_current_font.base_line = cmaps_Times21->base_line;
            m_current_font.lookup_table = cmaps_Times21->lookup_table;
            break;
        case 25:
            m_current_font.cmaps = cmaps_Times25;
            m_current_font.glyph_bitmap = glyph_bitmap_Times25;
            m_current_font.glyph_dsc = glyph_dsc_Times25;
            m_current_font.range_start = cmaps_Times25->range_start;
            m_current_font.range_length = cmaps_Times25->range_length;
            m_current_font.line_height = cmaps_Times25->line_height;
            m_current_font.font_height = cmaps_Times25->font_height;
            m_current_font.base_line = cmaps_Times25->base_line;
            m_current_font.lookup_table = cmaps_Times15->lookup_table;
            break;
        case 27:
            m_current_font.cmaps = cmaps_Times27;
            m_current_font.glyph_bitmap = glyph_bitmap_Times27;
            m_current_font.glyph_dsc = glyph_dsc_Times27;
            m_current_font.range_start = cmaps_Times27->range_start;
            m_current_font.range_length = cmaps_Times27->range_length;
            m_current_font.line_height = cmaps_Times27->line_height;
            m_current_font.font_height = cmaps_Times27->font_height;
            m_current_font.base_line = cmaps_Times27->base_line;
            m_current_font.lookup_table = cmaps_Times27->lookup_table;
            break;
        case 34:
            m_current_font.cmaps = cmaps_Times34;
            m_current_font.glyph_bitmap = glyph_bitmap_Times34;
            m_current_font.glyph_dsc = glyph_dsc_Times34;
            m_current_font.range_start = cmaps_Times34->range_start;
            m_current_font.range_length = cmaps_Times34->range_length;
            m_current_font.line_height = cmaps_Times34->line_height;
            m_current_font.font_height = cmaps_Times34->font_height;
            m_current_font.base_line = cmaps_Times34->base_line;
            m_current_font.lookup_table = cmaps_Times34->lookup_table;
            break;
        case 38:
            m_current_font.cmaps = cmaps_Times38;
            m_current_font.glyph_bitmap = glyph_bitmap_Times38;
            m_current_font.glyph_dsc = glyph_dsc_Times38;
            m_current_font.range_start = cmaps_Times38->range_start;
            m_current_font.range_length = cmaps_Times38->range_length;
            m_current_font.line_height = cmaps_Times38->line_height;
            m_current_font.font_height = cmaps_Times38->font_height;
            m_current_font.base_line = cmaps_Times38->base_line;
            m_current_font.lookup_table = cmaps_Times38->lookup_table;
            break;
        case 43:
            m_current_font.cmaps = cmaps_Times43;
            m_current_font.glyph_bitmap = glyph_bitmap_Times43;
            m_current_font.glyph_dsc = glyph_dsc_Times43;
            m_current_font.range_start = cmaps_Times43->range_start;
            m_current_font.range_length = cmaps_Times43->range_length;
            m_current_font.line_height = cmaps_Times43->line_height;
            m_current_font.font_height = cmaps_Times43->font_height;
            m_current_font.base_line = cmaps_Times43->base_line;
            m_current_font.lookup_table = cmaps_Times43->lookup_table;
            break;
        case 56:
            m_current_font.cmaps = cmaps_Times56;
            m_current_font.glyph_bitmap = glyph_bitmap_Times56;
            m_current_font.glyph_dsc = glyph_dsc_Times56;
            m_current_font.range_start = cmaps_Times56->range_start;
            m_current_font.range_length = cmaps_Times56->range_length;
            m_current_font.line_height = cmaps_Times56->line_height;
            m_current_font.font_height = cmaps_Times56->font_height;
            m_current_font.base_line = cmaps_Times56->base_line;
            m_current_font.lookup_table = cmaps_Times56->lookup_table;
            break;
        case 66:
            m_current_font.cmaps = cmaps_Times66;
            m_current_font.glyph_bitmap = glyph_bitmap_Times66;
            m_current_font.glyph_dsc = glyph_dsc_Times66;
            m_current_font.range_start = cmaps_Times66->range_start;
            m_current_font.range_length = cmaps_Times66->range_length;
            m_current_font.line_height = cmaps_Times66->line_height;
            m_current_font.font_height = cmaps_Times66->font_height;
            m_current_font.base_line = cmaps_Times66->base_line;
            m_current_font.lookup_table = cmaps_Times66->lookup_table;
            break;
        case 81:
            m_current_font.cmaps = cmaps_Times81;
            m_current_font.glyph_bitmap = glyph_bitmap_Times81;
            m_current_font.glyph_dsc = glyph_dsc_Times81;
            m_current_font.range_start = cmaps_Times81->range_start;
            m_current_font.range_length = cmaps_Times81->range_length;
            m_current_font.line_height = cmaps_Times81->line_height;
            m_current_font.font_height = cmaps_Times81->font_height;
            m_current_font.base_line = cmaps_Times81->base_line;
            m_current_font.lookup_table = cmaps_Times81->lookup_table;
            break;
        case 96:
            m_current_font.cmaps = cmaps_Times96;
            m_current_font.glyph_bitmap = glyph_bitmap_Times96;
            m_current_font.glyph_dsc = glyph_dsc_Times96;
            m_current_font.range_start = cmaps_Times96->range_start;
            m_current_font.range_length = cmaps_Times96->range_length;
            m_current_font.line_height = cmaps_Times96->line_height;
            m_current_font.font_height = cmaps_Times96->font_height;
            m_current_font.base_line = cmaps_Times96->base_line;
            m_current_font.lookup_table = cmaps_Times96->lookup_table;
            break;
        case 156:
            m_current_font.cmaps = cmaps_BigNumbers;
            m_current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            m_current_font.glyph_dsc = glyph_dsc_BigNumbers;
            m_current_font.range_start = cmaps_BigNumbers->range_start;
            m_current_font.range_length = cmaps_BigNumbers->range_length;
            m_current_font.line_height = cmaps_BigNumbers->line_height;
            m_current_font.font_height = cmaps_BigNumbers->font_height;
            m_current_font.base_line = cmaps_BigNumbers->base_line;
            m_current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: log_e("unknown font size for Times New Roman, size is %i", font); break;
    }
#endif

#ifdef TFT_GARAMOND
    switch(font) {
        case 15:
            m_current_font.cmaps = cmaps_Garamond15;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond15;
            m_current_font.glyph_dsc = glyph_dsc_Garamond15;
            m_current_font.range_start = cmaps_Garamond15->range_start;
            m_current_font.range_length = cmaps_Garamond15->range_length;
            m_current_font.line_height = cmaps_Garamond15->line_height;
            m_current_font.font_height = cmaps_Garamond15->font_height;
            m_current_font.base_line = cmaps_Garamond15->base_line;
            m_current_font.lookup_table = cmaps_Garamond15->lookup_table;
            break;
        case 16:
            m_current_font.cmaps = cmaps_Garamond16;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond16;
            m_current_font.glyph_dsc = glyph_dsc_Garamond16;
            m_current_font.range_start = cmaps_Garamond16->range_start;
            m_current_font.range_length = cmaps_Garamond16->range_length;
            m_current_font.line_height = cmaps_Garamond16->line_height;
            m_current_font.font_height = cmaps_Garamond16->font_height;
            m_current_font.base_line = cmaps_Garamond16->base_line;
            m_current_font.lookup_table = cmaps_Garamond16->lookup_table;
            break;
        case 18:
            m_current_font.cmaps = cmaps_Garamond18;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond18;
            m_current_font.glyph_dsc = glyph_dsc_Garamond18;
            m_current_font.range_start = cmaps_Garamond18->range_start;
            m_current_font.range_length = cmaps_Garamond18->range_length;
            m_current_font.line_height = cmaps_Garamond18->line_height;
            m_current_font.font_height = cmaps_Garamond18->font_height;
            m_current_font.base_line = cmaps_Garamond18->base_line;
            m_current_font.lookup_table = cmaps_Garamond18->lookup_table;
            break;
        case 21:
            m_current_font.cmaps = cmaps_Garamond21;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond21;
            m_current_font.glyph_dsc = glyph_dsc_Garamond21;
            m_current_font.range_start = cmaps_Garamond21->range_start;
            m_current_font.range_length = cmaps_Garamond21->range_length;
            m_current_font.line_height = cmaps_Garamond21->line_height;
            m_current_font.font_height = cmaps_Garamond21->font_height;
            m_current_font.base_line = cmaps_Garamond21->base_line;
            m_current_font.lookup_table = cmaps_Garamond21->lookup_table;
            break;
        case 25:
            m_current_font.cmaps = cmaps_Garamond25;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond25;
            m_current_font.glyph_dsc = glyph_dsc_Garamond25;
            m_current_font.range_start = cmaps_Garamond25->range_start;
            m_current_font.range_length = cmaps_Garamond25->range_length;
            m_current_font.line_height = cmaps_Garamond25->line_height;
            m_current_font.font_height = cmaps_Garamond25->font_height;
            m_current_font.base_line = cmaps_Garamond25->base_line;
            m_current_font.lookup_table = cmaps_Garamond25->lookup_table;
            break;
        case 27:
            m_current_font.cmaps = cmaps_Garamond27;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond27;
            m_current_font.glyph_dsc = glyph_dsc_Garamond27;
            m_current_font.range_start = cmaps_Garamond27->range_start;
            m_current_font.range_length = cmaps_Garamond27->range_length;
            m_current_font.line_height = cmaps_Garamond27->line_height;
            m_current_font.font_height = cmaps_Garamond27->font_height;
            m_current_font.base_line = cmaps_Garamond27->base_line;
            m_current_font.lookup_table = cmaps_Garamond27->lookup_table;
            break;
        case 34:
            m_current_font.cmaps = cmaps_Garamond34;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond34;
            m_current_font.glyph_dsc = glyph_dsc_Garamond34;
            m_current_font.range_start = cmaps_Garamond34->range_start;
            m_current_font.range_length = cmaps_Garamond34->range_length;
            m_current_font.line_height = cmaps_Garamond34->line_height;
            m_current_font.font_height = cmaps_Garamond34->font_height;
            m_current_font.base_line = cmaps_Garamond34->base_line;
            m_current_font.lookup_table = cmaps_Garamond34->lookup_table;
            break;
        case 38:
            m_current_font.cmaps = cmaps_Garamond38;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond38;
            m_current_font.glyph_dsc = glyph_dsc_Garamond38;
            m_current_font.range_start = cmaps_Garamond38->range_start;
            m_current_font.range_length = cmaps_Garamond38->range_length;
            m_current_font.line_height = cmaps_Garamond38->line_height;
            m_current_font.font_height = cmaps_Garamond38->font_height;
            m_current_font.base_line = cmaps_Garamond38->base_line;
            m_current_font.lookup_table = cmaps_Garamond38->lookup_table;
            break;
        case 43:
            m_current_font.cmaps = cmaps_Garamond43;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond43;
            m_current_font.glyph_dsc = glyph_dsc_Garamond43;
            m_current_font.range_start = cmaps_Garamond43->range_start;
            m_current_font.range_length = cmaps_Garamond43->range_length;
            m_current_font.line_height = cmaps_Garamond43->line_height;
            m_current_font.font_height = cmaps_Garamond43->font_height;
            m_current_font.base_line = cmaps_Garamond43->base_line;
            m_current_font.lookup_table = cmaps_Garamond43->lookup_table;
            break;
        case 56:
            m_current_font.cmaps = cmaps_Garamond56;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond56;
            m_current_font.glyph_dsc = glyph_dsc_Garamond56;
            m_current_font.range_start = cmaps_Garamond56->range_start;
            m_current_font.range_length = cmaps_Garamond56->range_length;
            m_current_font.line_height = cmaps_Garamond56->line_height;
            m_current_font.font_height = cmaps_Garamond56->font_height;
            m_current_font.base_line = cmaps_Garamond56->base_line;
            m_current_font.lookup_table = cmaps_Garamond56->lookup_table;
            break;
        case 66:
            m_current_font.cmaps = cmaps_Garamond66;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond66;
            m_current_font.glyph_dsc = glyph_dsc_Garamond66;
            m_current_font.range_start = cmaps_Garamond66->range_start;
            m_current_font.range_length = cmaps_Garamond66->range_length;
            m_current_font.line_height = cmaps_Garamond66->line_height;
            m_current_font.font_height = cmaps_Garamond66->font_height;
            m_current_font.base_line = cmaps_Garamond66->base_line;
            m_current_font.lookup_table = cmaps_Garamond66->lookup_table;
            break;
        case 81:
            m_current_font.cmaps = cmaps_Garamond81;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond81;
            m_current_font.glyph_dsc = glyph_dsc_Garamond81;
            m_current_font.range_start = cmaps_Garamond81->range_start;
            m_current_font.range_length = cmaps_Garamond81->range_length;
            m_current_font.line_height = cmaps_Garamond81->line_height;
            m_current_font.font_height = cmaps_Garamond81->font_height;
            m_current_font.base_line = cmaps_Garamond81->base_line;
            m_current_font.lookup_table = cmaps_Garamond81->lookup_table;
            break;
        case 96:
            m_current_font.cmaps = cmaps_Garamond96;
            m_current_font.glyph_bitmap = glyph_bitmap_Garamond96;
            m_current_font.glyph_dsc = glyph_dsc_Garamond96;
            m_current_font.range_start = cmaps_Garamond96->range_start;
            m_current_font.range_length = cmaps_Garamond96->range_length;
            m_current_font.line_height = cmaps_Garamond96->line_height;
            m_current_font.font_height = cmaps_Garamond96->font_height;
            m_current_font.base_line = cmaps_Garamond96->base_line;
            m_current_font.lookup_table = cmaps_Garamond96->lookup_table;
            break;
        case 156:
            m_current_font.cmaps = cmaps_BigNumbers;
            m_current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            m_current_font.glyph_dsc = glyph_dsc_BigNumbers;
            m_current_font.range_start = cmaps_BigNumbers->range_start;
            m_current_font.range_length = cmaps_BigNumbers->range_length;
            m_current_font.line_height = cmaps_BigNumbers->line_height;
            m_current_font.font_height = cmaps_BigNumbers->font_height;
            m_current_font.base_line = cmaps_BigNumbers->base_line;
            m_current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif

#ifdef TFT_FREE_SERIF_ITALIC
    switch(font) {
        case 15:
            m_current_font.cmaps = cmaps_FreeSerifItalic15;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic15;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic15;
            m_current_font.range_start = cmaps_FreeSerifItalic15->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic15->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic15->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic15->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic15->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic15->lookup_table;
            break;
        case 16:
            m_current_font.cmaps = cmaps_FreeSerifItalic16;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic16;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic16;
            m_current_font.range_start = cmaps_FreeSerifItalic16->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic16->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic16->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic16->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic16->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic16->lookup_table;
            break;
        case 18:
            m_current_font.cmaps = cmaps_FreeSerifItalic18;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic18;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic18;
            m_current_font.range_start = cmaps_FreeSerifItalic18->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic18->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic18->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic18->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic18->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic18->lookup_table;
            break;
        case 21:
            m_current_font.cmaps = cmaps_FreeSerifItalic21;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic21;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic21;
            m_current_font.range_start = cmaps_FreeSerifItalic21->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic21->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic21->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic21->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic21->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic21->lookup_table;
            break;
        case 25:
            m_current_font.cmaps = cmaps_FreeSerifItalic25;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic25;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic25;
            m_current_font.range_start = cmaps_FreeSerifItalic25->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic25->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic25->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic25->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic25->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic25->lookup_table;
            break;
        case 27:
            m_current_font.cmaps = cmaps_FreeSerifItalic27;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic27;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic27;
            m_current_font.range_start = cmaps_FreeSerifItalic27->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic27->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic27->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic27->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic27->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic27->lookup_table;
            break;
        case 34:
            m_current_font.cmaps = cmaps_FreeSerifItalic34;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic34;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic34;
            m_current_font.range_start = cmaps_FreeSerifItalic34->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic34->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic34->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic34->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic34->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic34->lookup_table;
            break;
        case 38:
            m_current_font.cmaps = cmaps_FreeSerifItalic38;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic38;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic38;
            m_current_font.range_start = cmaps_FreeSerifItalic38->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic38->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic38->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic38->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic38->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic38->lookup_table;
            break;
        case 43:
            m_current_font.cmaps = cmaps_FreeSerifItalic43;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic43;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic43;
            m_current_font.range_start = cmaps_FreeSerifItalic43->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic43->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic43->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic43->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic43->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic43->lookup_table;
            break;
        case 56:
            m_current_font.cmaps = cmaps_FreeSerifItalic56;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic56;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic56;
            m_current_font.range_start = cmaps_FreeSerifItalic56->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic56->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic56->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic56->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic56->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic56->lookup_table;
            break;
        case 66:
            m_current_font.cmaps = cmaps_FreeSerifItalic66;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic66;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic66;
            m_current_font.range_start = cmaps_FreeSerifItalic66->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic66->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic66->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic66->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic66->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic66->lookup_table;
            break;
        case 81:
            m_current_font.cmaps = cmaps_FreeSerifItalic81;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic81;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic81;
            m_current_font.range_start = cmaps_FreeSerifItalic81->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic81->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic81->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic81->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic81->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic81->lookup_table;
            break;
        case 96:
            m_current_font.cmaps = cmaps_FreeSerifItalic96;
            m_current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic96;
            m_current_font.glyph_dsc = glyph_dsc_FreeSerifItalic96;
            m_current_font.range_start = cmaps_FreeSerifItalic96->range_start;
            m_current_font.range_length = cmaps_FreeSerifItalic96->range_length;
            m_current_font.line_height = cmaps_FreeSerifItalic96->line_height;
            m_current_font.font_height = cmaps_FreeSerifItalic96->font_height;
            m_current_font.base_line = cmaps_FreeSerifItalic96->base_line;
            m_current_font.lookup_table = cmaps_FreeSerifItalic96->lookup_table;
            break;
        case 156:
            m_current_font.cmaps = cmaps_BigNumbers;
            m_current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            m_current_font.glyph_dsc = glyph_dsc_BigNumbers;
            m_current_font.range_start = cmaps_BigNumbers->range_start;
            m_current_font.range_length = cmaps_BigNumbers->range_length;
            m_current_font.line_height = cmaps_BigNumbers->line_height;
            m_current_font.font_height = cmaps_BigNumbers->font_height;
            m_current_font.base_line = cmaps_BigNumbers->base_line;
            m_current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }

#endif

#ifdef TFT_ARIAL
    switch(font) {
        case 15:
            m_current_font.cmaps = cmaps_Arial15;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial15;
            m_current_font.glyph_dsc = glyph_dsc_Arial15;
            m_current_font.range_start = cmaps_Arial15->range_start;
            m_current_font.range_length = cmaps_Arial15->range_length;
            m_current_font.line_height = cmaps_Arial15->line_height;
            m_current_font.font_height = cmaps_Arial15->font_height;
            m_current_font.base_line = cmaps_Arial15->base_line;
            m_current_font.lookup_table = cmaps_Arial15->lookup_table;
            break;
        case 16:
            m_current_font.cmaps = cmaps_Arial16;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial16;
            m_current_font.glyph_dsc = glyph_dsc_Arial16;
            m_current_font.range_start = cmaps_Arial16->range_start;
            m_current_font.range_length = cmaps_Arial16->range_length;
            m_current_font.line_height = cmaps_Arial16->line_height;
            m_current_font.font_height = cmaps_Arial16->font_height;
            m_current_font.base_line = cmaps_Arial16->base_line;
            m_current_font.lookup_table = cmaps_Arial16->lookup_table;
            break;
        case 18:
            m_current_font.cmaps = cmaps_Arial18;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial18;
            m_current_font.glyph_dsc = glyph_dsc_Arial18;
            m_current_font.range_start = cmaps_Arial18->range_start;
            m_current_font.range_length = cmaps_Arial18->range_length;
            m_current_font.line_height = cmaps_Arial18->line_height;
            m_current_font.font_height = cmaps_Arial18->font_height;
            m_current_font.base_line = cmaps_Arial18->base_line;
            m_current_font.lookup_table = cmaps_Arial18->lookup_table;
            break;
        case 21:
            m_current_font.cmaps = cmaps_Arial21;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial21;
            m_current_font.glyph_dsc = glyph_dsc_Arial21;
            m_current_font.range_start = cmaps_Arial21->range_start;
            m_current_font.range_length = cmaps_Arial21->range_length;
            m_current_font.line_height = cmaps_Arial21->line_height;
            m_current_font.font_height = cmaps_Arial21->font_height;
            m_current_font.base_line = cmaps_Arial21->base_line;
            m_current_font.lookup_table = cmaps_Arial21->lookup_table;
            break;
        case 25:
            m_current_font.cmaps = cmaps_Arial25;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial25;
            m_current_font.glyph_dsc = glyph_dsc_Arial25;
            m_current_font.range_start = cmaps_Arial25->range_start;
            m_current_font.range_length = cmaps_Arial25->range_length;
            m_current_font.line_height = cmaps_Arial25->line_height;
            m_current_font.font_height = cmaps_Arial25->font_height;
            m_current_font.base_line = cmaps_Arial25->base_line;
            m_current_font.lookup_table = cmaps_Arial25->lookup_table;
            break;
        case 27:
            m_current_font.cmaps = cmaps_Arial27;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial27;
            m_current_font.glyph_dsc = glyph_dsc_Arial27;
            m_current_font.range_start = cmaps_Arial27->range_start;
            m_current_font.range_length = cmaps_Arial27->range_length;
            m_current_font.line_height = cmaps_Arial27->line_height;
            m_current_font.font_height = cmaps_Arial27->font_height;
            m_current_font.base_line = cmaps_Arial27->base_line;
            m_current_font.lookup_table = cmaps_Arial27->lookup_table;
            break;
        case 34:
            m_current_font.cmaps = cmaps_Arial34;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial34;
            m_current_font.glyph_dsc = glyph_dsc_Arial34;
            m_current_font.range_start = cmaps_Arial34->range_start;
            m_current_font.range_length = cmaps_Arial34->range_length;
            m_current_font.line_height = cmaps_Arial34->line_height;
            m_current_font.font_height = cmaps_Arial34->font_height;
            m_current_font.base_line = cmaps_Arial34->base_line;
            m_current_font.lookup_table = cmaps_Arial34->lookup_table;
            break;
        case 38:
            m_current_font.cmaps = cmaps_Arial38;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial38;
            m_current_font.glyph_dsc = glyph_dsc_Arial38;
            m_current_font.range_start = cmaps_Arial38->range_start;
            m_current_font.range_length = cmaps_Arial38->range_length;
            m_current_font.line_height = cmaps_Arial38->line_height;
            m_current_font.font_height = cmaps_Arial38->font_height;
            m_current_font.base_line = cmaps_Arial38->base_line;
            m_current_font.lookup_table = cmaps_Arial38->lookup_table;
            break;
        case 43:
            m_current_font.cmaps = cmaps_Arial43;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial43;
            m_current_font.glyph_dsc = glyph_dsc_Arial43;
            m_current_font.range_start = cmaps_Arial43->range_start;
            m_current_font.range_length = cmaps_Arial43->range_length;
            m_current_font.line_height = cmaps_Arial43->line_height;
            m_current_font.font_height = cmaps_Arial43->font_height;
            m_current_font.base_line = cmaps_Arial43->base_line;
            m_current_font.lookup_table = cmaps_Arial43->lookup_table;
            break;
        case 56:
            m_current_font.cmaps = cmaps_Arial56;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial56;
            m_current_font.glyph_dsc = glyph_dsc_Arial56;
            m_current_font.range_start = cmaps_Arial56->range_start;
            m_current_font.range_length = cmaps_Arial56->range_length;
            m_current_font.line_height = cmaps_Arial56->line_height;
            m_current_font.font_height = cmaps_Arial56->font_height;
            m_current_font.base_line = cmaps_Arial56->base_line;
            m_current_font.lookup_table = cmaps_Arial56->lookup_table;
            break;
        case 66:
            m_current_font.cmaps = cmaps_Arial66;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial66;
            m_current_font.glyph_dsc = glyph_dsc_Arial66;
            m_current_font.range_start = cmaps_Arial66->range_start;
            m_current_font.range_length = cmaps_Arial66->range_length;
            m_current_font.line_height = cmaps_Arial66->line_height;
            m_current_font.font_height = cmaps_Arial66->font_height;
            m_current_font.base_line = cmaps_Arial66->base_line;
            m_current_font.lookup_table = cmaps_Arial66->lookup_table;
            break;
        case 81:
            m_current_font.cmaps = cmaps_Arial81;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial81;
            m_current_font.glyph_dsc = glyph_dsc_Arial81;
            m_current_font.range_start = cmaps_Arial81->range_start;
            m_current_font.range_length = cmaps_Arial81->range_length;
            m_current_font.line_height = cmaps_Arial81->line_height;
            m_current_font.font_height = cmaps_Arial81->font_height;
            m_current_font.base_line = cmaps_Arial81->base_line;
            m_current_font.lookup_table = cmaps_Arial81->lookup_table;
            break;
        case 96:
            m_current_font.cmaps = cmaps_Arial96;
            m_current_font.glyph_bitmap = glyph_bitmap_Arial96;
            m_current_font.glyph_dsc = glyph_dsc_Arial96;
            m_current_font.range_start = cmaps_Arial96->range_start;
            m_current_font.range_length = cmaps_Arial96->range_length;
            m_current_font.line_height = cmaps_Arial96->line_height;
            m_current_font.font_height = cmaps_Arial96->font_height;
            m_current_font.base_line = cmaps_Arial96->base_line;
            m_current_font.lookup_table = cmaps_Arial96->lookup_table;
            break;
        case 156:
            m_current_font.cmaps = cmaps_BigNumbers;
            m_current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            m_current_font.glyph_dsc = glyph_dsc_BigNumbers;
            m_current_font.range_start = cmaps_BigNumbers->range_start;
            m_current_font.range_length = cmaps_BigNumbers->range_length;
            m_current_font.line_height = cmaps_BigNumbers->line_height;
            m_current_font.font_height = cmaps_BigNumbers->font_height;
            m_current_font.base_line = cmaps_BigNumbers->base_line;
            m_current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif

#ifdef TFT_Z003
    switch(font) {
        case 15:
            m_current_font.cmaps = cmaps_Z003_15;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_15;
            m_current_font.glyph_dsc = glyph_dsc_Z003_15;
            m_current_font.range_start = cmaps_Z003_15->range_start;
            m_current_font.range_length = cmaps_Z003_15->range_length;
            m_current_font.line_height = cmaps_Z003_15->line_height;
            m_current_font.font_height = cmaps_Z003_15->font_height;
            m_current_font.base_line = cmaps_Z003_15->base_line;
            m_current_font.lookup_table = cmaps_Z003_15->lookup_table;
            break;
        case 16:
            m_current_font.cmaps = cmaps_Z003_16;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_16;
            m_current_font.glyph_dsc = glyph_dsc_Z003_16;
            m_current_font.range_start = cmaps_Z003_16->range_start;
            m_current_font.range_length = cmaps_Z003_16->range_length;
            m_current_font.line_height = cmaps_Z003_16->line_height;
            m_current_font.font_height = cmaps_Z003_16->font_height;
            m_current_font.base_line = cmaps_Z003_16->base_line;
            m_current_font.lookup_table = cmaps_Z003_16->lookup_table;
            break;
        case 18:
            m_current_font.cmaps = cmaps_Z003_18;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_18;
            m_current_font.glyph_dsc = glyph_dsc_Z003_18;
            m_current_font.range_start = cmaps_Z003_18->range_start;
            m_current_font.range_length = cmaps_Z003_18->range_length;
            m_current_font.line_height = cmaps_Z003_18->line_height;
            m_current_font.font_height = cmaps_Z003_18->font_height;
            m_current_font.base_line = cmaps_Z003_18->base_line;
            m_current_font.lookup_table = cmaps_Z003_18->lookup_table;
            break;
        case 21:
            m_current_font.cmaps = cmaps_Z003_21;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_21;
            m_current_font.glyph_dsc = glyph_dsc_Z003_21;
            m_current_font.range_start = cmaps_Z003_21->range_start;
            m_current_font.range_length = cmaps_Z003_21->range_length;
            m_current_font.line_height = cmaps_Z003_21->line_height;
            m_current_font.font_height = cmaps_Z003_21->font_height;
            m_current_font.base_line = cmaps_Z003_21->base_line;
            m_current_font.lookup_table = cmaps_Z003_21->lookup_table;
            break;
        case 25:
            m_current_font.cmaps = cmaps_Z003_25;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_25;
            m_current_font.glyph_dsc = glyph_dsc_Z003_25;
            m_current_font.range_start = cmaps_Z003_25->range_start;
            m_current_font.range_length = cmaps_Z003_25->range_length;
            m_current_font.line_height = cmaps_Z003_25->line_height;
            m_current_font.font_height = cmaps_Z003_25->font_height;
            m_current_font.base_line = cmaps_Z003_25->base_line;
            m_current_font.lookup_table = cmaps_Z003_25->lookup_table;
            break;
        case 27:
            m_current_font.cmaps = cmaps_Z003_27;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_27;
            m_current_font.glyph_dsc = glyph_dsc_Z003_27;
            m_current_font.range_start = cmaps_Z003_27->range_start;
            m_current_font.range_length = cmaps_Z003_27->range_length;
            m_current_font.line_height = cmaps_Z003_27->line_height;
            m_current_font.font_height = cmaps_Z003_27->font_height;
            m_current_font.base_line = cmaps_Z003_27->base_line;
            m_current_font.lookup_table = cmaps_Z003_27->lookup_table;
            break;
        case 34:
            m_current_font.cmaps = cmaps_Z003_34;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_34;
            m_current_font.glyph_dsc = glyph_dsc_Z003_34;
            m_current_font.range_start = cmaps_Z003_34->range_start;
            m_current_font.range_length = cmaps_Z003_34->range_length;
            m_current_font.line_height = cmaps_Z003_34->line_height;
            m_current_font.font_height = cmaps_Z003_34->font_height;
            m_current_font.base_line = cmaps_Z003_34->base_line;
            m_current_font.lookup_table = cmaps_Z003_34->lookup_table;
            break;
        case 38:
            m_current_font.cmaps = cmaps_Z003_38;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_38;
            m_current_font.glyph_dsc = glyph_dsc_Z003_38;
            m_current_font.range_start = cmaps_Z003_38->range_start;
            m_current_font.range_length = cmaps_Z003_38->range_length;
            m_current_font.line_height = cmaps_Z003_38->line_height;
            m_current_font.font_height = cmaps_Z003_38->font_height;
            m_current_font.base_line = cmaps_Z003_38->base_line;
            m_current_font.lookup_table = cmaps_Z003_38->lookup_table;
            break;
        case 43:
            m_current_font.cmaps = cmaps_Z003_43;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_43;
            m_current_font.glyph_dsc = glyph_dsc_Z003_43;
            m_current_font.range_start = cmaps_Z003_43->range_start;
            m_current_font.range_length = cmaps_Z003_43->range_length;
            m_current_font.line_height = cmaps_Z003_43->line_height;
            m_current_font.font_height = cmaps_Z003_43->font_height;
            m_current_font.base_line = cmaps_Z003_43->base_line;
            m_current_font.lookup_table = cmaps_Z003_43->lookup_table;
            break;
        case 56:
            m_current_font.cmaps = cmaps_Z003_56;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_56;
            m_current_font.glyph_dsc = glyph_dsc_Z003_56;
            m_current_font.range_start = cmaps_Z003_56->range_start;
            m_current_font.range_length = cmaps_Z003_56->range_length;
            m_current_font.line_height = cmaps_Z003_56->line_height;
            m_current_font.font_height = cmaps_Z003_56->font_height;
            m_current_font.base_line = cmaps_Z003_56->base_line;
            m_current_font.lookup_table = cmaps_Z003_56->lookup_table;
            break;
        case 66:
            m_current_font.cmaps = cmaps_Z003_66;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_66;
            m_current_font.glyph_dsc = glyph_dsc_Z003_66;
            m_current_font.range_start = cmaps_Z003_66->range_start;
            m_current_font.range_length = cmaps_Z003_66->range_length;
            m_current_font.line_height = cmaps_Z003_66->line_height;
            m_current_font.font_height = cmaps_Z003_66->font_height;
            m_current_font.base_line = cmaps_Z003_66->base_line;
            m_current_font.lookup_table = cmaps_Z003_66->lookup_table;
            break;
        case 81:
            m_current_font.cmaps = cmaps_Z003_81;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_81;
            m_current_font.glyph_dsc = glyph_dsc_Z003_81;
            m_current_font.range_start = cmaps_Z003_81->range_start;
            m_current_font.range_length = cmaps_Z003_81->range_length;
            m_current_font.line_height = cmaps_Z003_81->line_height;
            m_current_font.font_height = cmaps_Z003_81->font_height;
            m_current_font.base_line = cmaps_Z003_81->base_line;
            m_current_font.lookup_table = cmaps_Z003_81->lookup_table;
            break;
        case 96:
            m_current_font.cmaps = cmaps_Z003_96;
            m_current_font.glyph_bitmap = glyph_bitmap_Z003_96;
            m_current_font.glyph_dsc = glyph_dsc_Z003_96;
            m_current_font.range_start = cmaps_Z003_96->range_start;
            m_current_font.range_length = cmaps_Z003_96->range_length;
            m_current_font.line_height = cmaps_Z003_96->line_height;
            m_current_font.font_height = cmaps_Z003_96->font_height;
            m_current_font.base_line = cmaps_Z003_96->base_line;
            m_current_font.lookup_table = cmaps_Z003_96->lookup_table;
            break;
        case 156:
            m_current_font.cmaps = cmaps_BigNumbers;
            m_current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            m_current_font.glyph_dsc = glyph_dsc_BigNumbers;
            m_current_font.range_start = cmaps_BigNumbers->range_start;
            m_current_font.range_length = cmaps_BigNumbers->range_length;
            m_current_font.line_height = cmaps_BigNumbers->line_height;
            m_current_font.font_height = cmaps_BigNumbers->font_height;
            m_current_font.base_line = cmaps_BigNumbers->base_line;
            m_current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫    T E X T    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫          *
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::writeTheFramebuffer(const uint8_t* bmi, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) {

    auto bitreader = [&](const uint8_t* bm) { // lambda
        static uint16_t       bmi = 0;
        static uint8_t        idx = 0;
        static const uint8_t* bitmap = NULL;
        if(bm) {
            bitmap = bm;
            idx = 0x80;
            bmi = 0;
            return (int32_t)0;
        }
        bool bit = *(bitmap + bmi) & idx;
        idx >>= 1;
        if(idx == 0) {
            bmi++;
            idx = 0x80;
        }
        if(bit) { return (int32_t) m_textColor;}
        return (int32_t)-1;  // _backColor, -1 is transparent
    };

    bitreader(bmi);

    for(int16_t j = posY; j < posY + height; j++) {
        for(int16_t i = posX; i < posX + width; i++) {
            int32_t color = bitreader(0);
            if(color == -1) {
                continue;
            }
            m_framebuffer[0][j * m_h_res + i] = color;
        }
    }
    if(width == 0 || height == 0) return; // nothing to draw
    panelDrawBitmap(posX, posY, posX + width, posY + height, m_framebuffer[0]);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// The function is passed a string and two arrays of length strlen(str + 1). This is definitely enough, since ANSI sequences or non-ASCII UTF-8 characters are always greater than 1.
// For each printable character found in the LookUp table, the codepoint is written to the next position in the charr. The number of printable characters is increased by one.
// If an ANSI sequence is found, the color found is written into ansiArr at the position of the current character. The return value is the number of printable character.
uint16_t TFT_RGB::validCharsInString(const char* str, uint16_t* chArr, int8_t* ansiArr) {
    int16_t  codePoint = -1;
    uint16_t idx = 0;
    uint16_t chLen = 0;
    while((uint8_t)str[idx] != 0) {
        switch((uint8_t)str[idx]) {
            case '\033': // ANSI sequence
                if(strncmp(str + idx, "\033[30m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 1;  break;} // ANSI_ESC_BLACK
                if(strncmp(str + idx, "\033[31m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 2;  break;} // ANSI_ESC_RED
                if(strncmp(str + idx, "\033[32m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 3;  break;} // ANSI_ESC_GREEN
                if(strncmp(str + idx, "\033[33m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 4;  break;} // ANSI_ESC_YELLOW
                if(strncmp(str + idx, "\033[34m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 5;  break;} // ANSI_ESC_BLUE
                if(strncmp(str + idx, "\033[35m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 6;  break;} // ANSI_ESC_MAGENTA
                if(strncmp(str + idx, "\033[36m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 7;  break;} // ANSI_ESC_CYAN
                if(strncmp(str + idx, "\033[37m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 8;  break;} // ANSI_ESC_WHITE
                if(strncmp(str + idx, "\033[38;5;130m", 11) == 0)           {idx += 11; ansiArr[chLen] = 9;  break;} // ANSI_ESC_BROWN
                if(strncmp(str + idx, "\033[38;5;214m", 11) == 0)           {idx += 11; ansiArr[chLen] = 10; break;} // ANSI_ESC_ORANGE
                if(strncmp(str + idx, "\033[90m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 11; break;} // ANSI_ESC_GREY
                if(strncmp(str + idx, "\033[91m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 12; break;} // ANSI_ESC_LIGHTRED
                if(strncmp(str + idx, "\033[92m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 13; break;} // ANSI_ESC_LIGHTGREEN
                if(strncmp(str + idx, "\033[93m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 14; break;} // ANSI_ESC_LIGHTYELLOW
                if(strncmp(str + idx, "\033[94m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 15; break;} // ANSI_ESC_LIGHTBLUE
                if(strncmp(str + idx, "\033[95m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 16; break;} // ANSI_ESC_LIGHTMAGENTA
                if(strncmp(str + idx, "\033[96m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 17; break;} // ANSI_ESC_LIGHTCYAN
                if(strncmp(str + idx, "\033[97m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 18; break;} // ANSI_ESC_LIGHTGREY
                if(strncmp(str + idx, "\033[38;5;52m", 10) == 0)            {idx += 10; ansiArr[chLen] = 19; break;} // ANSI_ESC_DARKRED
                if(strncmp(str + idx, "\033[38;5;22m", 10) == 0)            {idx += 10; ansiArr[chLen] = 20; break;} // ANSI_ESC_DARKGREEN
                if(strncmp(str + idx, "\033[38;5;136m", 11) == 0)           {idx += 11; ansiArr[chLen] = 21; break;} // ANSI_ESC_DARKYELLOW
                if(strncmp(str + idx, "\033[38;5;17m", 10) == 0)            {idx += 10; ansiArr[chLen] = 22; break;} // ANSI_ESC_DARKBLUE
                if(strncmp(str + idx, "\033[38;5;53m", 10) == 0)            {idx += 10; ansiArr[chLen] = 23; break;} // ANSI_ESC_DARKMAGENTA
                if(strncmp(str + idx, "\033[38;5;23m", 10) == 0)            {idx += 10; ansiArr[chLen] = 24; break;} // ANSI_ESC_DARKCYAN
                if(strncmp(str + idx, "\033[38;5;240m", 11) == 0)           {idx += 11; ansiArr[chLen] = 25; break;} // ANSI_ESC_DARKGREY
                if(strncmp(str + idx, "\033[38;5;166m", 11) == 0)           {idx += 11; ansiArr[chLen] = 26; break;} // ANSI_ESC_DARKORANGE
                if(strncmp(str + idx, "\033[38;5;215m", 11) == 0)           {idx += 11; ansiArr[chLen] = 27; break;} // ANSI_ESC_LIGHTORANGE
                if(strncmp(str + idx, "\033[38;5;129m", 11) == 0)           {idx += 11; ansiArr[chLen] = 28; break;} // ANSI_ESC_PURPLE
                if(strncmp(str + idx, "\033[38;5;213m", 11) == 0)           {idx += 11; ansiArr[chLen] = 29; break;} // ANSI_ESC_PINK
                if(strncmp(str + idx, "\033[38;5;190m", 11) == 0)           {idx += 11; ansiArr[chLen] = 30; break;} // ANSI_ESC_LIME
                if(strncmp(str + idx, "\033[38;5;25m", 10) == 0)            {idx += 10; ansiArr[chLen] = 31; break;} // ANSI_ESC_NAVY
                if(strncmp(str + idx, "\033[38;5;51m", 10) == 0)            {idx += 10; ansiArr[chLen] = 32; break;} // ANSI_ESC_AQUAMARINE
                if(strncmp(str + idx, "\033[38;5;189m", 11) == 0)           {idx += 11; ansiArr[chLen] = 33; break;} // ANSI_ESC_LAVENDER
                if(strncmp(str + idx, "\033[38;2;210;180;140m", 19) == 0)   {idx += 19; ansiArr[chLen] = 34; break;} // ANSI_ESC_LIGHTBROWN
                if(strncmp(str + idx, "\033[0m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_RESET       unused
                if(strncmp(str + idx, "\033[1m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_BOLD        unused
                if(strncmp(str + idx, "\033[2m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_FAINT       unused
                if(strncmp(str + idx, "\033[3m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_ITALIC      unused
                if(strncmp(str + idx, "\033[4m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_UNDERLINE   unused
                log_w("unknown ANSI ESC SEQUENCE");                         {idx += 4;  ansiArr[chLen] = -1; break;} // unknown
                break;
            case 0x20 ... 0x7F:                   // is ASCII
                chArr[chLen] = (uint8_t)str[idx]; // codepoint
                idx += 1;
                chLen += 1;
                break;
            case 0xC2 ... 0xD1:
                codePoint = ((uint8_t)str[idx] - 0xC2) * 0x40 + (uint8_t)str[idx + 1]; // codepoint
                if(m_current_font.lookup_table[codePoint] != 0) {                       // is invalid UTF8 char
                    chArr[chLen] = codePoint;
                    chLen += 1;
                }
                else { log_w("character 0x%02X%02X is not in table", str[idx], str[idx + 1]); }
                idx += 2;
                break;
            case 0xD2 ... 0xDF:
                log_w("character 0x%02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 2;
                break;
            case 0xE0:
                if((uint8_t)str[idx + 1] == 0x80 && (uint8_t)str[idx + 2] == 0x99) {
                    codePoint = 0xA4;
                    chLen += 1;
                } // special sign 0xe28099 (general punctuation)
                else log_w("character %02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 3;
                break;
            case 0xE1 ... 0xEF: idx += 3; break;
            case 0xF0 ... 0xFF: idx += 4; break;
            case 0x0A: idx+=1; break; // is '/n'
            default: log_w("char is not printable 0x%02X", (uint8_t)str[idx]); idx += 1;
        }
    }
    return chLen;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t TFT_RGB::fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap){
    // cpArr contains the CodePoints of all printable characters in the string. chLength is the length of cpArr. From a starting position, the characters that fit into a width of win_W are determined.
    // The ending of a word is defined by a space or an \n. The number of characters that can be written is returned. For later alignment of the characters, the length is passed in usedPxLength.
    // narrow: no x offsets are taken into account
    // noWrap: The last possible character is written, spaces and line ends are not taken into account.
    uint16_t idx = begin;
    uint16_t pxLength = 0;
    uint16_t lastSpacePos = 0;
    uint16_t drawableChars = 0;
    uint16_t lastUsedPxLength = 0;
    uint16_t glyphPos = 0;
    while(cpArr[idx] != 0) {
        *usedPxLength = pxLength;
        if(cpArr[idx] == 0x20 || cpArr[idx - 1] == '-') {
            lastSpacePos = drawableChars;
            lastUsedPxLength = pxLength;
        }
        glyphPos = m_current_font.lookup_table[cpArr[idx]];
        pxLength += m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        int ofsX = m_current_font.glyph_dsc[glyphPos].ofs_x;
        if(ofsX < 0) ofsX = 0;
        if(!narrow) pxLength += ofsX;
        if(pxLength > win_W || cpArr[idx] == '\n') { // force wrap
            if(noWrap) { return drawableChars; }
            if(lastSpacePos) {
                *usedPxLength = lastUsedPxLength;
                return lastSpacePos;
            }
            else return drawableChars;
        }
        idx++;
        drawableChars++;
        *usedPxLength = pxLength;
    }
    return drawableChars;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TFT_RGB::fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap){
    // First, the largest character set is used to check whether a given string str fits into a window of size win_W - winH.
    // If this is not the case, the next smaller character set is selected and checked again.
    // The largest possible character set (in px) is used; if nothing fits, the smallest character set is used. Then parts of the string will not be able to be written.
    // cpArr contains the codepoints of the str, chLength determines th Length of cpArr, returns the number of lines used
    uint8_t nrOfFonts = sizeof(fontSizes);
    uint8_t currentFontSize = 0;
    uint16_t usedPxLength = 0;
    uint16_t drawableCharsTotal = 0;
    uint16_t drawableCharsinline = 0;
    uint16_t startPos = 0;
    uint8_t  nrOfLines = 0;
    while(true){
        currentFontSize = fontSizes[nrOfFonts - 1];
        if(currentFontSize == 0) break;
        setFont(currentFontSize);
        drawableCharsTotal = 0;
        startPos = 0;
        nrOfLines = 1;
        int16_t win_H_remain = win_H;
        while(true){
            if(win_H_remain < m_current_font.line_height) {break;}
            drawableCharsinline = fitinline(cpArr, chLength, startPos, win_W, &usedPxLength, narrow, noWrap);
            win_H_remain -= m_current_font.line_height;
        //    log_i("drawableCharsinline  %i,chLength  %i, currentFontSize %i", drawableCharsinline, chLength, currentFontSize);
            drawableCharsTotal += drawableCharsinline;
            startPos += drawableCharsinline;
            if(drawableCharsinline == 0) break;
            if(drawableCharsTotal == chLength) goto exit;
            nrOfLines++;
        }
        if(drawableCharsTotal == chLength) goto exit;
        if(nrOfFonts == 0) break;
        nrOfFonts--;
    }
exit:
//  log_w("nrOfLines  %i", nrOfLines);
    return nrOfLines;
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
size_t TFT_RGB::writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align, uint8_t v_align, bool narrow, bool noWrap, bool autoSize) {
    // autoSize choose the biggest possible font
    uint16_t idx = 0;
    uint16_t utfPosArr[strlen(str) + 1] = {0};
    int8_t   ansiArr[strlen(str) + 1] = {0};
    uint16_t strChLength = 0; // nr. of chars
    uint8_t  nrOfLines = 1;

    //-------------------------------------------------------------------------------------------------------------------
    auto drawChar = [&](uint16_t idx, uint16_t x, uint16_t y) { // lambda
        uint16_t glyphPos = m_current_font.lookup_table[utfPosArr[idx]];
        uint16_t adv_w = m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        uint32_t bitmap_index = m_current_font.glyph_dsc[glyphPos].bitmap_index;
        uint16_t box_w = m_current_font.glyph_dsc[glyphPos].box_w;
        uint16_t box_h = m_current_font.glyph_dsc[glyphPos].box_h;
        int16_t  ofs_x = m_current_font.glyph_dsc[glyphPos].ofs_x;
        int16_t  ofs_y = m_current_font.glyph_dsc[glyphPos].ofs_y;
        if(ofs_x < 0) ofs_x = 0;
        x += ofs_x;
        y = y + (m_current_font.line_height - m_current_font.base_line - 1) - box_h - ofs_y;
        writeTheFramebuffer(m_current_font.glyph_bitmap + bitmap_index, x, y, box_w, box_h);
        if(!narrow) adv_w += ofs_x;
        return adv_w;
    };
    //-------------------------------------------------------------------------------------------------------------------

    strChLength = validCharsInString(str, utfPosArr, ansiArr); // fill utfPosArr,
    if(autoSize) {nrOfLines = fitInAddrWindow(utfPosArr, strChLength, win_W, win_H, narrow, noWrap);}  // choose perfect fontsize
    if(!strChLength) return 0;
    //----------------------------------------------------------------------
    if((win_X + win_W) > m_h_res) { win_W = m_v_res - win_X; }   // Limit, right edge of the display
    if((win_Y + win_H) > m_v_res) { win_H = m_h_res - win_Y; } // Limit, bottom of the display

    idx = 0;
    uint16_t pX = win_X;
    uint16_t pY = win_Y;
    int16_t  pH = win_H;
    int16_t  pW = win_W;

    if(v_align == TFT_ALIGN_TOP){
        ; // nothing to do, is default
    }
    if(v_align == TFT_ALIGN_CENTER){
        int offset = (win_H - (nrOfLines * m_current_font.line_height)) / 2;
        pY = pY + offset;
    }
    if(v_align == TFT_ALIGN_DOWN){
        int offset = (win_H - (nrOfLines * m_current_font.line_height));
        pY = pY + offset;
    }

    uint16_t charsToDraw = 0;
    uint16_t usedPxLength = 0;
    uint16_t charsDrawn = 0;
    while(true) { // outer while
        if(noWrap && idx) goto exit;
        if(pH < m_current_font.line_height) { goto exit; }
        //charsToDraw = fitinline(idx, pW, &usedPxLength);
        charsToDraw = fitinline(utfPosArr, strChLength, idx, pW, &usedPxLength, narrow, noWrap);

        if(h_align == TFT_ALIGN_RIGHT)  { pX += win_W - (usedPxLength + 1); }
        if(h_align == TFT_ALIGN_CENTER) { pX += (win_W - usedPxLength) / 2; }
        uint16_t cnt = 0;
        while(true) {               // inner while
            if(ansiArr[idx] != 0) { //
                if(ansiArr[idx] ==  1) setTextColor(TFT_BLACK);
                if(ansiArr[idx] ==  2) setTextColor(TFT_RED);
                if(ansiArr[idx] ==  3) setTextColor(TFT_GREEN);
                if(ansiArr[idx] ==  4) setTextColor(TFT_YELLOW);
                if(ansiArr[idx] ==  5) setTextColor(TFT_BLUE);
                if(ansiArr[idx] ==  6) setTextColor(TFT_MAGENTA);
                if(ansiArr[idx] ==  7) setTextColor(TFT_CYAN);
                if(ansiArr[idx] ==  8) setTextColor(TFT_WHITE);
                if(ansiArr[idx] ==  9) setTextColor(TFT_BROWN);
                if(ansiArr[idx] == 10) setTextColor(TFT_ORANGE);
                if(ansiArr[idx] == 11) setTextColor(TFT_GREY);
                if(ansiArr[idx] == 12) setTextColor(TFT_LIGHTRED);
                if(ansiArr[idx] == 13) setTextColor(TFT_LIGHTGREEN);
                if(ansiArr[idx] == 14) setTextColor(TFT_LIGHTYELLOW);
                if(ansiArr[idx] == 15) setTextColor(TFT_LIGHTBLUE);
                if(ansiArr[idx] == 16) setTextColor(TFT_LIGHTMAGENTA);
                if(ansiArr[idx] == 17) setTextColor(TFT_LIGHTCYAN);
                if(ansiArr[idx] == 18) setTextColor(TFT_LIGHTGREY);
                if(ansiArr[idx] == 19) setTextColor(TFT_DARKRED);
                if(ansiArr[idx] == 20) setTextColor(TFT_DARKGREEN);
                if(ansiArr[idx] == 21) setTextColor(TFT_DARKYELLOW);
                if(ansiArr[idx] == 22) setTextColor(TFT_DARKBLUE);
                if(ansiArr[idx] == 23) setTextColor(TFT_DARKMAGENTA);
                if(ansiArr[idx] == 24) setTextColor(TFT_DARKCYAN);
                if(ansiArr[idx] == 25) setTextColor(TFT_DARKGREY);
                if(ansiArr[idx] == 26) setTextColor(TFT_DARKORANGE);
                if(ansiArr[idx] == 27) setTextColor(TFT_LIGHTORANGE);
                if(ansiArr[idx] == 28) setTextColor(TFT_PURPLE);
                if(ansiArr[idx] == 29) setTextColor(TFT_PINK);
                if(ansiArr[idx] == 30) setTextColor(TFT_LIME);
                if(ansiArr[idx] == 31) setTextColor(TFT_NAVY);
                if(ansiArr[idx] == 32) setTextColor(TFT_AQUAMARINE);
                if(ansiArr[idx] == 33) setTextColor(TFT_LAVENDER);
                if(ansiArr[idx] == 34) setTextColor(TFT_LIGHTBROWN);
            }
            if(cnt == 0 && utfPosArr[idx] == 0x20) {
                idx++;
                charsDrawn++;
                continue;
            } // skip leading spaces
            uint16_t res = drawChar(idx, pX, pY);
            pX += res;
            pW -= res;
            idx++;
            cnt++;
            charsDrawn++;
            if(idx == strChLength) goto exit;
            if(cnt == charsToDraw) break;
        } // inner while
        pH -= m_current_font.line_height;
        pY += m_current_font.line_height;
        pX = win_X;
        pW = win_W;
    } // outer while
exit:
    panelDrawBitmap(win_X, win_Y, win_X + win_W, win_Y + win_H, m_framebuffer[0]);
    return charsDrawn;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  B I T M A P  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫              *
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#define bmpRead32(d, o) (d[o] | (uint16_t)(d[(o) + 1]) << 8 | (uint32_t)(d[(o) + 2]) << 16 | (uint32_t)(d[(o) + 3]) << 24)
#define bmpRead16(d, o) (d[o] | (uint16_t)(d[(o) + 1]) << 8)

#define bmpColor8(c)  (((uint16_t)(((uint8_t*)(c))[0] & 0xE0) << 8) | ((uint16_t)(((uint8_t*)(c))[0] & 0x1C) << 6) | ((((uint8_t*)(c))[0] & 0x3) << 3))
#define bmpColor16(c) ((((uint8_t*)(c))[0] | ((uint16_t)((uint8_t*)(c))[1]) << 8))
#define bmpColor24(c) (((uint16_t)(((uint8_t*)(c))[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[0] & 0xF8) >> 3))
#define bmpColor32(c) (((uint16_t)(((uint8_t*)(c))[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[2] & 0xFC) << 3) | ((((uint8_t*)(c))[1] & 0xF8) >> 3))

bool TFT_RGB::drawBmpFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, float scale) {
    if (scale <= 0) {
        log_e("Invalid scale value: %f", scale);
        return false;
    }

    if (!fs.exists(path)) {
        log_e("file %s does not exist", path);
        return false;
    }

    File bmp_file = fs.open(path);
    if (!bmp_file) {
        log_e("Failed to open file for reading: %s", path);
        return false;
    }

    constexpr size_t headerLen = 0x36; // BMP-Header-Größe
    uint8_t headerBuf[headerLen];

    if (bmp_file.size() < headerLen || bmp_file.read(headerBuf, headerLen) < headerLen) {
        log_e("Failed to read the file's header");
        bmp_file.close();
        return false;
    }

    if (headerBuf[0] != 'B' || headerBuf[1] != 'M') {
        log_e("Invalid BMP file format");
        bmp_file.close();
        return false;
    }

    const uint32_t dataOffset = bmpRead32(headerBuf, 0x0A);
    const int32_t bmpWidthI = bmpRead32(headerBuf, 0x12);
    const int32_t bmpHeightI = bmpRead32(headerBuf, 0x16);
    const uint16_t bitsPerPixel = bmpRead16(headerBuf, 0x1C);
    const uint32_t compression = bmpRead32(headerBuf, 0x1E);

    if (compression != 0) {
        log_e("Compressed BMP files are not supported");
        bmp_file.close();
        return false;
    }

    const size_t bmpWidth = abs(bmpWidthI);
    const size_t bmpHeight = abs(bmpHeightI);

    const size_t scaledWidth = bmpWidth * scale;
    const size_t scaledHeight = bmpHeight * scale;

    // Wenn maxWidth oder maxHeight 0 ist, wird der Wert ignoriert
    const size_t effectiveMaxWidth = (maxWidth == 0) ? scaledWidth : maxWidth;
    const size_t effectiveMaxHeight = (maxHeight == 0) ? scaledHeight : maxHeight;

    // Begrenzen der tatsächlichen Darstellungsgröße auf den verfügbaren Ausschnitt
    const size_t displayWidth = std::min(effectiveMaxWidth, scaledWidth);
    const size_t displayHeight = std::min(effectiveMaxHeight, scaledHeight);

    const size_t rowSize = ((bmpWidth * bitsPerPixel / 8 + 3) & ~3);
    uint8_t* rowBuffer = new uint8_t[rowSize];

    for (size_t i_y = 0; i_y < displayHeight; ++i_y) {
        const float srcY = i_y / scale;
        const size_t srcRow = bmpHeight - 1 - (size_t)srcY;

        if (srcRow >= bmpHeight) continue;

        bmp_file.seek(dataOffset + srcRow * rowSize);
        if (bmp_file.read(rowBuffer, rowSize) != rowSize) {
            log_e("Failed to read BMP row data");
            delete[] rowBuffer;
            bmp_file.close();
            return false;
        }

        for (size_t i_x = 0; i_x < displayWidth; ++i_x) {
            const float srcX = i_x / scale;
            const size_t srcCol = (size_t)srcX;

            if (srcCol >= bmpWidth) continue;

            const uint8_t* pixelPtr = rowBuffer + srcCol * (bitsPerPixel / 8);
            uint16_t color;

            switch (bitsPerPixel) {
                case 16:
                    color = bmpColor16(pixelPtr);
                    break;
                case 24:
                    color = bmpColor24(pixelPtr);
                    break;
                case 32:
                    color = bmpColor32(pixelPtr);
                    break;
                default:
                    log_e("Unsupported bitsPerPixel: %d", bitsPerPixel);
                    delete[] rowBuffer;
                    bmp_file.close();
                    return false;
            }

            const size_t xPos = x + i_x;
            const size_t yPos = y + i_y;

            if (xPos < m_h_res && yPos < m_v_res) {
                m_framebuffer[0][yPos * m_h_res + xPos] = color;
            }
        }
    }

    delete[] rowBuffer;
    bmp_file.close();

    // Nur den betroffenen Bereich auf dem Display aktualisieren
    panelDrawBitmap(x, y, x + displayWidth, y + displayHeight, m_framebuffer[0]);
    return true;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  G I F ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat) {
    // debug=true;
    int32_t iterations = repeat;

    gif_next.clear();
    gif_next.shrink_to_fit();
    gif_vals.clear();
    gif_vals.shrink_to_fit();
    gif_stack.clear();
    gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();
    gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();

    gif_decodeSdFile_firstread = false;
    gif_GlobalColorTableFlag = false;
    gif_LocalColorTableFlag = false;
    gif_SortFlag = false;
    gif_TransparentColorFlag = false;
    gif_UserInputFlag = false;
    gif_ZeroDataBlock = 0;
    gif_InterlaceFlag = false;

    do { // repeat this gif
        gif_file = fs.open(path);
        if(!gif_file) {
            if(tft_info) tft_info("Failed to open file for reading");
            return false;
        }
        GIF_readGifItems();

        // check it's a gif
        if(!gif_GifHeader.startsWith("GIF")) {
            if(tft_info) tft_info("File is not a gif");
            return false;
        }
        // check dimensions
        // { log_w("Width: %i, Height: %i,", gif_LogicalScreenWidth, gif_LogicalScreenHeight); }
        if(gif_LogicalScreenWidth * gif_LogicalScreenHeight > 155000) {
            if(tft_info) tft_info("!Image is too big!!");
            return false;
        }

        while(GIF_decodeGif(x, y) == true) {
            if(iterations == 0) break;
        } // endwhile
        iterations--;
        gif_file.close();
    } while(iterations > 0);

    GIF_freeMemory();
    // log_i("freeHeap= %i", ESP.getFreeHeap());
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TFT_RGB::GIF_readHeader() {

    //      7 6 5 4 3 2 1 0        Field Name                    Type
    //     +---------------+
    //   0 |               |       Signature                     3 Bytes
    //     +-             -+
    //   1 |               |
    //     +-             -+
    //   2 |               |
    //     +---------------+
    //   3 |               |       Version                       3 Bytes
    //     +-             -+
    //   4 |               |
    //     +-             -+
    //   5 |               |
    //     +---------------+
    //
    //  i) Signature - Identifies the GIF Data Stream. This field contains
    //     the fixed value 'GIF'.
    //
    // ii) Version - Version number used to format the data stream.
    //     Identifies the minimum set of capabilities necessary to a decoder
    //     to fully process the contents of the Data Stream.
    //
    //     Version Numbers as of 10 July 1990 :       "87a" - May 1987
    //                                                "89a" - July 1989
    //
    gif_file.readBytes(gif_buffer, 6); // Header
    gif_buffer[6] = 0;
    gif_GifHeader = gif_buffer;
    if(debug) { log_i("GifHeader: %s", gif_GifHeader.c_str()); }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_readLogicalScreenDescriptor() {

    //    Logical Screen Descriptor
    //
    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Logical Screen Width          Unsigned
    //    +-             -+
    // 1  |               |
    //    +---------------+
    // 2  |               |       Logical Screen Height         Unsigned
    //    +-             -+
    // 3  |               |
    //    +---------------+
    // 4  | |     | |     |       <Packed Fields>               See below
    //    +---------------+
    // 5  |               |       Background Color Index        Byte
    //    +---------------+
    // 6  |               |       Pixel Aspect Ratio            Byte
    //    +---------------+
    //
    gif_file.readBytes(gif_buffer, 7); // Logical Screen Descriptor
    gif_LogicalScreenWidth = gif_buffer[0] + 256 * gif_buffer[1];
    gif_LogicalScreenHeight = gif_buffer[2] + 256 * gif_buffer[3];
    gif_PackedFields = gif_buffer[4];
    gif_GlobalColorTableFlag = (gif_PackedFields & 0x80);
    gif_ColorResulution = ((gif_PackedFields & 0x70) >> 3) + 1;
    gif_SortFlag = (gif_PackedFields & 0x08);
    gif_SizeOfGlobalColorTable = (gif_PackedFields & 0x07);
    gif_SizeOfGlobalColorTable = (1 << (gif_SizeOfGlobalColorTable + 1));
    gif_BackgroundColorIndex = gif_buffer[5];
    gif_PixelAspectRatio = gif_buffer[6];

    if(debug) {
        log_i("LogicalScreenWidth=%i", gif_LogicalScreenWidth);
        log_i("LogicalScreenHeight=%i", gif_LogicalScreenHeight);
        log_i("PackedFields=%i", gif_PackedFields);
        log_i("SortFlag=%i", gif_SortFlag);
        log_i("GlobalColorTableFlag=%i", gif_GlobalColorTableFlag);
        log_i("ColorResulution=%i", gif_ColorResulution);
        log_i("SizeOfGlobalColorTable=%i", gif_SizeOfGlobalColorTable);
        log_i("BackgroundColorIndex=%i", gif_BackgroundColorIndex);
        log_i("PixelAspectRatio=%i", gif_PixelAspectRatio);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_readImageDescriptor() {

    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Image Separator               Byte, fixed value 0x2C, always read before
    //    +---------------+
    // 1  |               |       Image Left Position           Unsigned
    //    +-             -+
    // 2  |               |
    //    +---------------+
    // 3  |               |       Image Top Position            Unsigned
    //    +-             -+
    // 4  |               |
    //    +---------------+
    // 5  |               |       Image Width                   Unsigned
    //    +-             -+
    // 6  |               |
    //    +---------------+
    // 7  |               |       Image Height                  Unsigned
    //    +-             -+
    // 8  |               |
    //    +---------------+
    // 9  | | | |   |     |       <Packed Fields>               See below
    //    +---------------+
    //
    //    <Packed Fields>  =      Local Color Table Flag        1 Bit
    //                            Interlace Flag                1 Bit
    //                            Sort Flag                     1 Bit
    //                            Reserved                      2 Bits
    //                            Size of Local Color Table     3 Bits

    gif_file.readBytes(gif_buffer, 9); // Image Descriptor
    gif_ImageLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif_ImageTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif_ImageWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif_ImageHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif_PackedFields = gif_buffer[8];
    gif_LocalColorTableFlag = ((gif_PackedFields & 0x80) >> 7);
    gif_InterlaceFlag = ((gif_PackedFields & 0x40) >> 6);
    gif_SortFlag = ((gif_PackedFields & 0x20)) >> 5;
    gif_SizeOfLocalColorTable = (gif_PackedFields & 0x07);
    gif_SizeOfLocalColorTable = (1 << (gif_SizeOfLocalColorTable + 1));

    if(debug) {
        log_i("ImageLeftPosition=%i", gif_ImageLeftPosition);
        log_i("ImageTopPosition=%i", gif_ImageTopPosition);
        log_i("ImageWidth=%i", gif_ImageWidth);
        log_i("ImageHeight=%i", gif_ImageHeight);
        log_i("PackedFields=%i", gif_PackedFields);
        log_i("LocalColorTableFlag=%i", gif_LocalColorTableFlag);
        log_i("InterlaceFlag=%i", gif_InterlaceFlag);
        log_i("SortFlag=%i", gif_SortFlag);
        log_i("SizeOfLocalColorTable=%i", gif_SizeOfLocalColorTable);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_readLocalColorTable() {
    // The size of the local color table can be calculated by the value given in the image descriptor.
    // Just like with the global color table, if the image descriptor specifies a size of N,
    // the color table will contain 2^(N+1) colors and will take up 3*2^(N+1) bytes.
    // The colors are specified in RGB value triplets.
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    gif_LocalColorTable.reserve(gif_SizeOfLocalColorTable);
    if(gif_LocalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif_SizeOfLocalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill LocalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_LocalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        // for(i=0;i<SizeOfLocalColorTable; i++) log_i("LocalColorTable %i= %i", i, LocalColorTable[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_readGlobalColorTable() {
    // Each GIF has its own color palette. That is, it has a list of all the colors that can be in the image
    // and cannot contain colors that are not in that list.
    // The global color table is where that list of colors is stored. Each color is stored in three bytes.
    // Each of the bytes represents an RGB color value. The first byte is the value for red (0-255), next green, then
    // blue. The size of the global color table is determined by the value in the packed byte of the logical screen
    // descriptor. If the value from that byte is N, then the actual number of colors stored is 2^(N+1). This means that
    // the global color table will take up 3*2^(N+1) bytes in the stream.

    //    Value In <Packed Fields>    Number Of Colors    Byte Length
    //        0                           2                   6
    //        1                           4                   12
    //        2                           8                   24
    //        3                           16                  48
    //        4                           32                  96
    //        5                           64                  192
    //        6                           128                 384
    //        7                           256                 768
    gif_GlobalColorTable.clear();
    if(gif_GlobalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif_SizeOfGlobalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill GlobalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_GlobalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        //    for(i=0;i<gif_SizeOfGlobalColorTable;i++) log_i("GlobalColorTable %i= %i", i, gif_GlobalColorTable[i]);
        //    log_i("Read GlobalColorTable Size=%i", gif_SizeOfGlobalColorTable);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_readGraphicControlExtension() {

    //      +---------------+
    //   0  |               |       Block Size                    Byte
    //      +---------------+
    //   1  |     |     | | |       <Packed Fields>               See below
    //      +---------------+
    //   2  |               |       Delay Time                    Unsigned
    //      +-             -+
    //   3  |               |
    //      +---------------+
    //   4  |               |       Transparent Color Index       Byte
    //      +---------------+
    //
    //      +---------------+
    //   0  |               |       Block Terminator              Byte
    //      +---------------+

    uint8_t BlockSize = 0;
    gif_file.readBytes(gif_buffer, 1);
    BlockSize = gif_buffer[0]; // Number of bytes in the block, not including the Block Terminator

    if(BlockSize == 0) return;
    gif_file.readBytes(gif_buffer, BlockSize);
    gif_PackedFields = gif_buffer[0];
    gif_DisposalMethod = (gif_PackedFields & 0x1C) >> 2;
    gif_UserInputFlag = (gif_PackedFields & 0x02);
    gif_TransparentColorFlag = gif_PackedFields & 0x01;
    gif_DelayTime = gif_buffer[1] + 256 * gif_buffer[2];
    gif_TransparentColorIndex = gif_buffer[3];

    if(debug) {
        log_i("BlockSize GraphicontrolExtension=%i", BlockSize);
        log_i("DisposalMethod=%i", gif_DisposalMethod);
        log_i("UserInputFlag=%i", gif_UserInputFlag);
        log_i("TransparentColorFlag=%i", gif_TransparentColorFlag);
        log_i("DelayTime=%i", gif_DelayTime);
        log_i("TransparentColorIndex=%i", gif_TransparentColorIndex);
    }
    gif_file.readBytes(gif_buffer, 1); // marks the end of the Graphic Control Extension
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_RGB::GIF_readPlainTextExtension(char* buf) {

    //      7 6 5 4 3 2 1 0        Field Name                    Type
    //     +---------------+
    //  0  |               |       Block Size                    Byte
    //     +---------------+
    //  1  |               |       Text Grid Left Position       Unsigned
    //     +-             -+
    //  2  |               |
    //     +---------------+
    //  3  |               |       Text Grid Top Position        Unsigned
    //     +-             -+
    //  4  |               |
    //     +---------------+
    //  5  |               |       Text Grid Width               Unsigned
    //     +-             -+
    //  6  |               |
    //     +---------------+
    //  7  |               |       Text Grid Height              Unsigned
    //     +-             -+
    //  8  |               |
    //     +---------------+
    //  9  |               |       Character Cell Width          Byte
    //     +---------------+
    // 10  |               |       Character Cell Height         Byte
    //     +---------------+
    // 11  |               |       Text Foreground Color Index   Byte
    //     +---------------+
    // 12  |               |       Text Background Color Index   Byte
    //     +---------------+
    //
    //     +===============+
    //     |               |
    //  N  |               |       Plain Text Data               Data Sub-blocks
    //     |               |
    //     +===============+
    //
    //     +---------------+
    //  0  |               |       Block Terminator              Byte
    //     +---------------+
    //
    uint8_t BlockSize = 0, numBytes = 0;
    BlockSize = gif_file.read();
    // log_i("BlockSize=%i", BlockSize);
    if(BlockSize > 0) {
        gif_file.readBytes(gif_buffer, BlockSize);
        // log_i("%s", buffer);
    }

    gif_TextGridLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif_TextGridTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif_TextGridWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif_TextGridHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif_CharacterCellWidth = gif_buffer[8];
    gif_CharacterCellHeight = gif_buffer[9];
    gif_TextForegroundColorIndex = gif_buffer[10];
    gif_TextBackgroundColorIndex = gif_buffer[11];

    if(debug) {
        log_i("TextGridLeftPosition=%i", gif_TextGridLeftPosition);
        log_i("TextGridTopPosition=%i", gif_TextGridTopPosition);
        log_i("TextGridWidth=%i", gif_TextGridWidth);
        log_i("TextGridHeight=%i", gif_TextGridHeight);
        log_i("CharacterCellWidth=%i", gif_CharacterCellWidth);
        log_i("CharacterCellHeight=%i", gif_CharacterCellHeight);
        log_i("TextForegroundColorIndex=%i", gif_TextForegroundColorIndex);
        log_i("TextBackgroundColorIndex=%i", gif_TextBackgroundColorIndex);
    }

    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_RGB::GIF_readApplicationExtension(char* buf) {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Block Size                    Byte
    //    +---------------+
    // 1  |               |
    //    +-             -+
    // 2  |               |
    //    +-             -+
    // 3  |               |       Application Identifier        8 Bytes
    //    +-             -+
    // 4  |               |
    //    +-             -+
    // 5  |               |
    //    +-             -+
    // 6  |               |
    //    +-             -+
    // 7  |               |
    //    +-             -+
    // 8  |               |
    //    +---------------+
    // 9  |               |
    //    +-             -+
    // 10  |               |       Appl. Authentication Code     3 Bytes
    //    +-             -+
    // 11  |               |
    //    +---------------+
    //
    //    +===============+
    //    |               |
    //    |               |       Application Data              Data Sub-blocks
    //    |               |
    //    |               |
    //    +===============+
    //
    //    +---------------+
    // 0  |               |       Block Terminator              Byte
    //    +---------------+

    uint8_t BlockSize = 0, numBytes = 0;
    BlockSize = gif_file.read();
    if(debug) { log_i("BlockSize=%i", BlockSize); }
    if(BlockSize > 0) { gif_file.readBytes(gif_buffer, BlockSize); }
    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_RGB::GIF_readCommentExtension(char* buf) {

    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //  +===============+
    //  |               |
    // N |               |       Comment Data                  Data Sub-blocks
    //  |               |
    //  +===============+
    //
    //  +---------------+
    // 0 |               |       Block Terminator              Byte
    //  +---------------+

    uint8_t numBytes = 0;
    numBytes = GIF_readDataSubBlock(buf);
    //sprintf(chbuf, "GIF: Comment %s", buf);
    // if(tft_info) tft_info(chbuf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_RGB::GIF_readDataSubBlock(char* buf) {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Block Size                    Byte
    //    +---------------+
    // 1  |               |
    //    +-             -+
    // 2  |               |
    //    +-             -+
    // 3  |               |
    //    +-             -+
    //    |               |       Data Values                   Byte
    //    +-             -+
    // up |               |
    //    +-   . . . .   -+
    // to |               |
    //    +-             -+
    //    |               |
    //    +-             -+
    // 255|               |
    //    +---------------+
    //

    uint8_t BlockSize = 0;
    BlockSize = gif_file.read();
    if(BlockSize > 0) {
        gif_ZeroDataBlock = false;
        gif_file.readBytes(buf, BlockSize);
    }
    else gif_ZeroDataBlock = true;
    if(debug) { log_i("Blocksize = %i", BlockSize); }
    return BlockSize;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_RGB::GIF_readExtension(char Label) {
    char buf[256];
    switch(Label) {
        case 0x01:
            if(debug) { log_i("PlainTextExtension"); }
            GIF_readPlainTextExtension(buf);
            break;
        case 0xff:
            if(debug) { log_i("ApplicationExtension"); }
            GIF_readApplicationExtension(buf);
            break;
        case 0xfe:
            if(debug) { log_i("CommentExtension"); }
            GIF_readCommentExtension(buf);
            break;
        case 0xF9:
            if(debug) { log_i("GraphicControlExtension"); }
            GIF_readGraphicControlExtension();
            break;
        default: return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_RGB::GIF_GetCode(int32_t code_size, int32_t flag) {
    //    Assuming a character array of 8 bits per character and using 5 bit codes to be
    //    packed, an example layout would be similar to:
    //
    //         +---------------+
    //      0  |               |    bbbaaaaa
    //         +---------------+
    //      1  |               |    dcccccbb
    //         +---------------+
    //      2  |               |    eeeedddd
    //         +---------------+
    //      3  |               |    ggfffffe
    //         +---------------+
    //      4  |               |    hhhhhggg
    //         +---------------+
    //               . . .
    //         +---------------+
    //      N  |               |
    //         +---------------+

    static char    DSBbuffer[300];
    static int32_t curbit, lastbit, done, last_byte;
    int32_t        i, j, ret;
    uint8_t        count;

    if(flag) {
        curbit = 0;
        lastbit = 0;
        done = false;
        return 0;
    }

    if((curbit + code_size) >= lastbit) {
        if(done) {
            // log_i("done");
            if(curbit >= lastbit) { return 0; }
            return -1;
        }
        DSBbuffer[0] = DSBbuffer[last_byte - 2];
        DSBbuffer[1] = DSBbuffer[last_byte - 1];

        // The rest of the Image Block represent data sub-blocks. Data sub-blocks are are groups of 1 - 256 bytes.
        // The first byte in the sub-block tells you how many bytes of actual data follow. This can be a value from 0
        // (00) it 255 (FF). After you've read those bytes, the next byte you read will tell you now many more bytes of
        // data follow that one. You continue to read until you reach a sub-block that says that zero bytes follow.
        //    endWrite();
        count = GIF_readDataSubBlock(&DSBbuffer[2]);
        //    startWrite();
        // log_i("Dtatblocksize %i", count);
        if(count == 0) done = true;

        last_byte = 2 + count;

        curbit = (curbit - lastbit) + 16;

        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for(i = curbit, j = 0; j < code_size; ++i, ++j) ret |= ((DSBbuffer[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_RGB::GIF_LZWReadByte(bool init) {
    static int32_t fresh = false;
    int32_t        code, incode;
    static int32_t firstcode, oldcode;

    if(gif_next.capacity() < (1 << gif_MaxLzwBits)) gif_next.reserve((1 << gif_MaxLzwBits) - gif_next.capacity());
    if(gif_vals.capacity() < (1 << gif_MaxLzwBits)) gif_vals.reserve((1 << gif_MaxLzwBits) - gif_vals.capacity());
    if(gif_stack.capacity() < (1 << (gif_MaxLzwBits + 1))) gif_stack.reserve((1 << (gif_MaxLzwBits + 1)) - gif_stack.capacity());
    gif_next.clear();
    gif_vals.clear();
    gif_stack.clear();

    static uint8_t* sp;

    int32_t i;

    if(init) {
        //    LWZMinCodeSize      ColorCodes      ClearCode       EOICode
        //    2                   #0-#3           #4              #5
        //    3                   #0-#7           #8              #9
        //    4                   #0-#15          #16             #17
        //    5                   #0-#31          #32             #33
        //    6                   #0-#63          #64             #65
        //    7                   #0-#127         #128            #129
        //    8                   #0-#255         #256            #257

        gif_CodeSize = gif_LZWMinimumCodeSize + 1;
        gif_ClearCode = (1 << gif_LZWMinimumCodeSize);
        gif_EOIcode = gif_ClearCode + 1;
        gif_MaxCode = gif_ClearCode + 2;
        gif_MaxCodeSize = 2 * gif_ClearCode;

        if(debug) {
            log_i("ClearCode=%i", gif_ClearCode);
            log_i("EOIcode=%i", gif_EOIcode);
            log_i("CodeSize=%i", gif_CodeSize);
            log_i("MaxCode=%i", gif_MaxCode);
            log_i("MaxCodeSize=%i", gif_MaxCodeSize);
        }

        fresh = false;

        GIF_GetCode(0, true);

        fresh = true;

        for(i = 0; i < gif_ClearCode; i++) {
            gif_next[i] = 0;
            gif_vals[i] = i;
        }
        for(; i < (1 << gif_MaxLzwBits); i++) gif_next[i] = gif_vals[0] = 0;

        sp = &gif_stack[0];

        return 0;
    }
    else if(fresh) {
        fresh = false;
        do { firstcode = oldcode = GIF_GetCode(gif_CodeSize, false); } while(firstcode == gif_ClearCode);

        return firstcode;
    }

    if(sp > &gif_stack[0]) return *--sp;

    while((code = GIF_GetCode(gif_CodeSize, false)) >= 0) {
        if(code == gif_ClearCode) {
            for(i = 0; i < gif_ClearCode; ++i) {
                gif_next[i] = 0;
                gif_vals[i] = i;
            }
            for(; i < (1 << gif_MaxLzwBits); ++i) gif_next[i] = gif_vals[i] = 0;

            gif_CodeSize = gif_LZWMinimumCodeSize + 1;
            gif_MaxCodeSize = 2 * gif_ClearCode;
            gif_MaxCode = gif_ClearCode + 2;
            sp = &gif_stack[0];

            firstcode = oldcode = GIF_GetCode(gif_CodeSize, false);
            return firstcode;
        }
        else if(code == gif_EOIcode) {
            if(debug) { log_i("read EOI Code"); }
            int32_t count;
            char    buf[260];

            if(gif_ZeroDataBlock) return -2;
            while((count = GIF_readDataSubBlock(buf)) > 0);

            if(count != 0) return -2;
        }

        incode = code;

        if(code >= gif_MaxCode) {
            *sp++ = firstcode;
            code = oldcode;
        }

        while(code >= gif_ClearCode) {
            *sp++ = gif_vals[code];
            if(code == (int32_t)gif_next[code]) { return -1; }
            code = gif_next[code];
        }
        *sp++ = firstcode = gif_vals[code];

        if((code = gif_MaxCode) < (1 << gif_MaxLzwBits)) {
            gif_next[code] = oldcode;
            gif_vals[code] = firstcode;
            ++gif_MaxCode;
            if((gif_MaxCode >= gif_MaxCodeSize) && (gif_MaxCodeSize < (1 << gif_MaxLzwBits))) {
                gif_MaxCodeSize *= 2;
                ++gif_CodeSize;
            }
        }
        oldcode = incode;

        if(sp > &gif_stack[0]) return *--sp;
    }
    return code;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TFT_RGB::GIF_ReadImage(uint16_t x, uint16_t y) {
    int32_t  color;
    int32_t  xpos = x + gif_ImageLeftPosition;
    int32_t  ypos = y + gif_ImageTopPosition;
    int32_t  max = gif_ImageHeight * gif_ImageWidth;
    uint32_t i = 0;
    // LZW Mindest-Codierungsgröße lesen
    gif_LZWMinimumCodeSize = gif_file.read();
    if (debug) {
        log_i("LZWMinimumCodeSize=%i", gif_LZWMinimumCodeSize);
    }

    // Initialisiere LZW-Dekodierung
    if (GIF_LZWReadByte(true) < 0) return false;

    // Speicher für die Bilddaten anfordern
    uint16_t* buf = (uint16_t*)__malloc_heap_psram(max * sizeof(uint16_t));
    if (!buf) return false;

    // Lese die Bilddaten und dekodiere sie
    while (i < max) {
        color = GIF_LZWReadByte(false);
        if ((color == gif_TransparentColorIndex) && gif_TransparentColorFlag) {
            // Transparente Pixel ignorieren
            buf[i] = m_backGroundColor; // Fülle Hintergrundfarbe für Transparenz
        } else {
            // Nutze die richtige Farbpalette (lokal oder global)
            if (gif_LocalColorTableFlag) {
                buf[i] = gif_LocalColorTable[color];
            } else {
                buf[i] = gif_GlobalColorTable[color];
            }
        }
        i++;
    }

    // Schreibe die Bilddaten in den Framebuffer
    for (uint16_t row = 0; row < gif_ImageHeight; row++) {
        for (uint16_t col = 0; col < gif_ImageWidth; col++) {
            uint16_t fb_x = xpos + col; // Berechne X-Position im Framebuffer
            uint16_t fb_y = ypos + row; // Berechne Y-Position im Framebuffer

            // Stelle sicher, dass die Koordinaten innerhalb der Grenzen liegen
            if (fb_x < m_h_res && fb_y < m_v_res) {
                m_framebuffer[0][fb_y * m_h_res + fb_x] = buf[row * gif_ImageWidth + col];
            }
        }
    }
    panelDrawBitmap(xpos, ypos, xpos + gif_ImageWidth, ypos + gif_ImageHeight, m_framebuffer[0]);

    // Speicher freigeben
    if (buf) {
        free(buf);
        buf = NULL;
    }

    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_RGB::GIF_readGifItems() {
    GIF_readHeader();
    GIF_readLogicalScreenDescriptor();
    gif_decodeSdFile_firstread = true;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_RGB::GIF_decodeGif(uint16_t x, uint16_t y) {
    char           c = 0;
    static int32_t test = 1;
    char           Label = 0;
    if(gif_decodeSdFile_firstread == true) GIF_readGlobalColorTable(); // If exists
    gif_decodeSdFile_firstread = false;

    while(c != ';') { // Trailer found
        c = gif_file.read();
        // log_w("c= %c",c);
        if(c == '!') {               // it is a Extension
            Label = gif_file.read(); // Label
            GIF_readExtension(Label);
        }
        if(c == ',') {
            GIF_readImageDescriptor(); // ImgageDescriptor
            GIF_readLocalColorTable(); // can follow the ImagrDescriptor
            GIF_ReadImage(x, y);       // read Image Data
            test++;
            return true; // more images can follow
        }
    }
    // for(int32_t i=0; i<bigbuf.size(); i++)  log_i("bigbuf %i=%i", i, bigbuf[i]);
    // if(tft_info) tft_info("GIF: found Trailer");
    return false; // no more images to decode
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_RGB::GIF_freeMemory() {
    gif_next.clear();
    gif_next.shrink_to_fit();
    gif_vals.clear();
    gif_vals.shrink_to_fit();
    gif_stack.clear();
    gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();
    gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ J P E G ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::drawJpgFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
    if(!fs.exists(path)) {log_e("file %s not exists", path); return false; }
    if(maxWidth) m_jpgWidthMax = maxWidth; else m_jpgWidthMax = m_h_res;
    if(maxHeight) m_jpgHeightMax = maxHeight; else m_jpgHeightMax = m_v_res;

    m_jpgSdFile = fs.open(path, FILE_READ);
    if(!m_jpgSdFile) {log_e("Failed to open file for reading"); JPEG_setJpgScale(1); return false;}
    JPEG_getSdJpgSize(&m_jpgWidth, &m_jpgHeight);
    int res = JPEG_drawSdJpg(x, y); (void) res;
    // log_w("path %s, res %i, x %i, y %i, m_jpgWidth %i, m_jpgHeight %i", path, res, x, y, m_jpgWidth, m_jpgHeight);
    m_jpgSdFile.close();
    panelDrawBitmap(x, y, x + m_jpgWidth, y + m_jpgHeight, m_framebuffer[0]);
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::JPEG_setJpgScale(uint8_t scaleFactor) {
    switch (scaleFactor) {
        case 1:  m_jpgScale = 0; break;
        case 2:  m_jpgScale = 1; break;
        case 4:  m_jpgScale = 2; break;
        case 8:  m_jpgScale = 3; break;
        default: m_jpgScale = 0;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::JPEG_setSwapBytes(bool swapBytes){
  m_swap = swapBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
unsigned int TFT_RGB::JPEG_jd_input(JDEC* jdec, uint8_t* buf, unsigned int len){
    uint32_t bytesLeft = 0;

    if (m_jpg_source == TJPG_ARRAY) {  // Handle an array input
        if (m_array_index + len > m_array_size) { len = m_array_size - m_array_index; } // Avoid running off end of array
        if (buf) memcpy_P(buf, (const uint8_t*)(m_array_data + m_array_index), len); // If buf is valid then copy len bytes to buffer
        m_array_index += len;  // Move pointer
    }
    else if (m_jpg_source == TJPG_SD_FILE) {  // Handle SD library input
        bytesLeft = m_jpgSdFile.available();  // Check how many bytes are available
        if (bytesLeft < len) len = bytesLeft;
        if (buf) {
            m_jpgSdFile.read(buf, len); // Read into buffer, pointer moved as well
        } else {
            m_jpgSdFile.seek(m_jpgSdFile.position() + len); // Buffer is null, so skip data by moving pointer
        }
    }
    return len;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Pass image block back to the sketch for rendering, may be a complete or partial MCU
int TFT_RGB::JPEG_jd_output(JDEC* jdec, void* bitmap, JRECT* jrect) {
    jdec = jdec; // Supress warning as ID is not used

    int16_t  x = jrect->left + m_jpeg_x;  // Retrieve rendering parameters and add any offset
    int16_t  y = jrect->top + m_jpeg_y;
    uint16_t w = jrect->right + 1 - jrect->left;
    uint16_t h = jrect->bottom + 1 - jrect->top;
//    if(x > m_jpgWidthMax) return true;  // Clip width and height to the maximum allowed dimensions
//    if(y > m_jpgHeightMax) return true;
    bool r = JPEG_tft_output(x, y, w, h, (uint16_t*)bitmap);
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_RGB::JPEG_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
      if (!bitmap || w <= 0 || h <= 0) {  // Check for valid parameters
        log_e("Invalid parameters: bitmap is null or width/height is zero.");
        return false;
    }
    // Clip the rectangle to ensure it doesn't exceed framebuffer boundaries
    int16_t x_end = std::min((int16_t)(x + w), (int16_t)m_h_res); // End of rectangle in x-direction
    int16_t y_end = std::min((int16_t)(y + h), (int16_t)(m_v_res)); // End of rectangle in y-direction

    if (x >= m_h_res || y >= m_v_res || x_end <= 0 || y_end <= 0) {
        log_e("Rectangle is completely outside the framebuffer boundaries.");
        return false;
    }

    // Adjust start coordinates if they are out of bounds
    int16_t start_x = max((int16_t)0, x);        // Sichtbarer Startpunkt in x-Richtung
    int16_t start_y = max((int16_t)0, y);        // Sichtbarer Startpunkt in y-Richtung
    int16_t clip_x_offset = start_x - x;         // Offset im Bitmap in x-Richtung
    int16_t clip_y_offset = start_y - y;         // Offset im Bitmap in y-Richtung

    // Berechnung der sichtbaren Breite und Höhe
    int16_t visible_w = x_end - start_x;         // Sichtbare Breite
    int16_t visible_h = y_end - start_y;         // Sichtbare Höhe

    // Zeilenweises Kopieren mit Clipping
    for (int16_t j = 0; j < visible_h ; ++j) {
        // Quelle im Bitmap: Berechne die richtige Zeilenposition
        uint16_t* src_ptr = bitmap + (clip_y_offset + j) * w + clip_x_offset;

        // Ziel im Framebuffer: Berechne die richtige Zeilenposition
        uint16_t* dest_ptr = m_framebuffer[0] + (start_y + j) * m_h_res + start_x;

        // Kopiere nur die sichtbare Breite
        memcpy(dest_ptr, src_ptr, visible_w * sizeof(uint16_t));
    }
    // log_w("Bitmap erfolgreich mit Clipping gezeichnet bei x: %d, y: %d, sichtbare Breite: %d, sichtbare Höhe: %d", start_x, start_y, visible_w, visible_h);
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_drawSdJpg(int32_t x, int32_t y) {
    JDEC    jdec;
    uint8_t r = JDR_OK;

    m_jpg_source = TJPG_SD_FILE;
    m_jpeg_x = x;
    m_jpeg_y = y;
    jdec.swap = m_swap;
    r = JPEG_jd_prepare(&jdec, m_workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (r == JDR_OK) { r = JPEG_jd_decomp(&jdec, m_jpgScale); } // Extract image and render
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_getSdJpgSize(uint16_t* w, uint16_t* h) {

    JDEC    jdec;
    uint8_t r = JDR_OK;
    *w = 0;
    *h = 0;

    m_jpg_source = TJPG_SD_FILE;
    r = JPEG_jd_prepare(&jdec, m_workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (r == JDR_OK) {
        *w = jdec.width;
        *h = jdec.height;
    }
    m_jpgSdFile.seek(0);
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if JD_FASTDECODE == 2
    #define HUFF_BIT  10 /* Bit length to apply fast huffman decode */
    #define HUFF_LEN  (1 << HUFF_BIT)
    #define HUFF_MASK (HUFF_LEN - 1)
#endif

const uint8_t Zig[64] = {/* Zigzag-order to raster-order conversion table */
                                0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
                                41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
                                30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

const uint16_t Ipsf[64] = {/* See also aa_idct.png */
     (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192),
     (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
     (uint16_t)(1.38704 * 8192), (uint16_t)(1.92388 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.63099 * 8192),
     (uint16_t)(1.38704 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.38268 * 8192),
     (uint16_t)(1.30656 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.70711 * 8192), (uint16_t)(1.53636 * 8192),
     (uint16_t)(1.30656 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.36048 * 8192),
     (uint16_t)(1.17588 * 8192), (uint16_t)(1.63099 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.38268 * 8192),
     (uint16_t)(1.17588 * 8192), (uint16_t)(0.92388 * 8192), (uint16_t)(0.63638 * 8192), (uint16_t)(0.32442 * 8192),
     (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192),
     (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
     (uint16_t)(0.78570 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.92388 * 8192),
     (uint16_t)(0.78570 * 8192), (uint16_t)(0.61732 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.21677 * 8192),
     (uint16_t)(0.54120 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.63638 * 8192),
     (uint16_t)(0.54120 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.29290 * 8192), (uint16_t)(0.14932 * 8192),
     (uint16_t)(0.27590 * 8192), (uint16_t)(0.38268 * 8192), (uint16_t)(0.36048 * 8192), (uint16_t)(0.32442 * 8192),
     (uint16_t)(0.27590 * 8192), (uint16_t)(0.21678 * 8192), (uint16_t)(0.14932 * 8192), (uint16_t)(0.07612 * 8192)};

#if JD_TBLCLIP
    #define BYTECLIP(v) Clip8[(unsigned int)(v) & 0x3FF]
const uint8_t Clip8[1024] = {    /* 0..255 */
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
    137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
    206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
    229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
    252, 253, 254, 255,
    /* 256..511 */
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255,
    /* -512..-257 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* -256..-1 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#if JD_TBLCLIP == 0 /* JD_TBLCLIP */
uint8_t TFT_RGB::JPEG_BYTECLIP(int val) {
    if(val < 0) return 0;
    else if(val > 255) return 255;
    return (uint8_t)val;
}
#endif
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void* TFT_RGB::JPEG_alloc_pool(JDEC  *jd,size_t ndata) {
    char *rp = 0;

    ndata = (ndata + 3) & ~3; /* Align block size to the word boundary */

    if(jd->sz_pool >= ndata) {
        jd->sz_pool -= ndata;
        rp = (char *)jd->pool;           /* Get start of available memory pool */
        jd->pool = (void *)(rp + ndata); /* Allocate requierd bytes */
    }

    return (void *)rp; /* Return allocated memory block (NULL:no memory to allocate) */
}//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_create_qt_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
    unsigned int i, zi;
    uint8_t      d;
    int32_t*     pb;

    while (ndata) {                      /* Process all tables in the segment */
        if (ndata < 65) return JDR_FMT1; /* Err: table size is unaligned */
        ndata -= 65;
        d = *data++;                                              /* Get table property */
        if (d & 0xF0) return JDR_FMT1;                            /* Err: not 8-bit resolution */
        i = d & 3;                                                /* Get table ID */
        pb = (int32_t*)JPEG_alloc_pool(jd, 64 * sizeof(int32_t)); /* Allocate a memory block for the table */
        if (!pb) return JDR_MEM1;                                 /* Err: not enough memory */
        jd->qttbl[i] = pb;                                        /* Register the table */
        for (i = 0; i < 64; i++) {                                /* Load the table */
            zi = Zig[i];                                          /* Zigzag-order to raster-order conversion */
            pb[zi] = (int32_t)((uint32_t)*data++ * Ipsf[zi]);     /* Apply scale factor of Arai algorithm to the de-quantizers */
        }
    }

    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_create_huffman_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
    unsigned int i, j, b, cls, num;
    size_t       np;
    uint8_t      d, *pb, *pd;
    uint16_t     hc, *ph;

    while (ndata) {                      /* Process all tables in the segment */
        if (ndata < 17) return JDR_FMT1; /* Err: wrong data size */
        ndata -= 17;
        d = *data++;                   /* Get table number and class */
        if (d & 0xEE) return JDR_FMT1; /* Err: invalid class/number */
        cls = d >> 4;
        num = d & 0x0F;                         /* class = dc(0)/ac(1), table number = 0/1 */
        pb = (uint8_t*)JPEG_alloc_pool(jd, 16); /* Allocate a memory block for the bit distribution table */
        if (!pb) return JDR_MEM1;               /* Err: not enough memory */
        jd->huffbits[num][cls] = pb;
        for (np = i = 0; i < 16; i++) { /* Load number of patterns for 1 to 16-bit code */
            np += (pb[i] = *data++);    /* Get sum of code words for each code */
        }
        ph = (uint16_t*)JPEG_alloc_pool(jd, np * sizeof(uint16_t)); /* Allocate a memory block for the code word table */
        if (!ph) return JDR_MEM1;                                   /* Err: not enough memory */
        jd->huffcode[num][cls] = ph;
        hc = 0;
        for (j = i = 0; i < 16; i++) { /* Re-build huffman code word table */
            b = pb[i];
            while (b--) ph[j++] = hc++;
            hc <<= 1;
        }

        if (ndata < np) return JDR_FMT1; /* Err: wrong data size */
        ndata -= np;
        pd = (uint8_t*)JPEG_alloc_pool(jd, np); /* Allocate a memory block for the decoded data */
        if (!pd) return JDR_MEM1;               /* Err: not enough memory */
        jd->huffdata[num][cls] = pd;
        for (i = 0; i < np; i++) { /* Load decoded data corresponds to each code word */
            d = *data++;
            if (!cls && d > 11) return JDR_FMT1;
            pd[i] = d;
        }
#if JD_FASTDECODE == 2
        { /* Create fast huffman decode table */
            unsigned int span, td, ti;
            uint16_t*    tbl_ac = 0;
            uint8_t*     tbl_dc = 0;

            if (cls) {
                tbl_ac = alloc_pool(jd, HUFF_LEN * sizeof(uint16_t)); /* LUT for AC elements */
                if (!tbl_ac) return JDR_MEM1;                         /* Err: not enough memory */
                jd->hufflut_ac[num] = tbl_ac;
                memset(tbl_ac, 0xFF, HUFF_LEN * sizeof(uint16_t)); /* Default value (0xFFFF: may be long code) */
            } else {
                tbl_dc = alloc_pool(jd, HUFF_LEN * sizeof(uint8_t)); /* LUT for AC elements */
                if (!tbl_dc) return JDR_MEM1;                        /* Err: not enough memory */
                jd->hufflut_dc[num] = tbl_dc;
                memset(tbl_dc, 0xFF, HUFF_LEN * sizeof(uint8_t)); /* Default value (0xFF: may be long code) */
            }
            for (i = b = 0; b < HUFF_BIT; b++) { /* Create LUT */
                for (j = pb[b]; j; j--) {
                    ti = ph[i] << (HUFF_BIT - 1 - b) & HUFF_MASK; /* Index of input pattern for the code */
                    if (cls) {
                        td = pd[i++] | ((b + 1) << 8); /* b15..b8: code length, b7..b0: zero run and data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_ac[ti++] = (uint16_t)td);
                    } else {
                        td = pd[i++] | ((b + 1) << 4); /* b7..b4: code length, b3..b0: data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_dc[ti++] = (uint8_t)td);
                    }
                }
            }
            jd->longofs[num][cls] = i; /* Code table offset for long code */
        }
#endif
    }

    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int TFT_RGB::JPEG_huffext(JDEC* jd, unsigned int id, unsigned int cls) {
    size_t       dc = jd->dctr;
    uint8_t*     dp = jd->dptr;
    unsigned int d, flg = 0;

#if JD_FASTDECODE == 0
    uint8_t         bm, nd, bl;
    const uint8_t*  hb = jd->huffbits[id][cls]; /* Bit distribution table */
    const uint16_t* hc = jd->huffcode[id][cls]; /* Code word table */
    const uint8_t*  hd = jd->huffdata[id][cls]; /* Data table */

    bm = jd->dbit; /* Bit mask to extract */
    d = 0;
    bl = 16; /* Max code length */
    do {
        if (!bm) {              /* Next byte? */
            if (!dc) {          /* No input data is available, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = jd->infunc(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            } else {
                dp++; /* Next data ptr */
            }
            dc--;                                       /* Decrement number of available bytes */
            if (flg) {                                  /* In flag sequence? */
                flg = 0;                                /* Exit flag sequence */
                if (*dp != 0) return 0 - (int)JDR_FMT1; /* Err: unexpected flag is detected (may be collapted data) */
                *dp = 0xFF;                             /* The flag is a data 0xFF */
            } else {
                if (*dp == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
            bm = 0x80; /* Read from MSB */
        }
        d <<= 1; /* Get a bit */
        if (*dp & bm) d++;
        bm >>= 1;

        for (nd = *hb++; nd; nd--) { /* Search the code word in this bit length */
            if (d == *hc++) {        /* Matched? */
                jd->dbit = bm;
                jd->dctr = dc;
                jd->dptr = dp;
                return *hd; /* Return the decoded data */
            }
            hd++;
        }
        bl--;
    } while (bl);

#else
    const uint8_t * hb, *hd;
    const uint16_t* hc;
    unsigned int    nc, bl, wbit = jd->dbit % 32;
    uint32_t        w = jd->wreg & ((1UL << wbit) - 1);

    while (wbit < 16) { /* Prepare 16 bits into the working register */
        if (jd->marker) {
            d = 0xFF; /* Input stream has stalled for a marker. Generate stuff bits */
        } else {
            if (!dc) {          /* Buffer empty, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            }
            d = *dp++;
            dc--;
            if (flg) {                      /* In flag sequence? */
                flg = 0;                    /* Exit flag sequence */
                if (d != 0) jd->marker = d; /* Not an escape of 0xFF but a marker */
                d = 0xFF;
            } else {
                if (d == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
        }
        w = w << 8 | d; /* Shift 8 bits in the working register */
        wbit += 8;
    }
    jd->dctr = dc;
    jd->dptr = dp;
    jd->wreg = w;

    #if JD_FASTDECODE == 2
    /* Table serch for the short codes */
    d = (unsigned int)(w >> (wbit - HUFF_BIT)); /* Short code as table index */
    if (cls) {                                  /* AC element */
        d = jd->hufflut_ac[id][d];              /* Table decode */
        if (d != 0xFFFF) {                      /* It is done if hit in short code */
            jd->dbit = wbit - (d >> 8);         /* Snip the code length */
            return d & 0xFF;                    /* b7..0: zero run and following data bits */
        }
    } else {                            /* DC element */
        d = jd->hufflut_dc[id][d];      /* Table decode */
        if (d != 0xFF) {                /* It is done if hit in short code */
            jd->dbit = wbit - (d >> 4); /* Snip the code length  */
            return d & 0xF;             /* b3..0: following data bits */
        }
    }

    /* Incremental serch for the codes longer than HUFF_BIT */
    hb = jd->huffbits[id][cls] + HUFF_BIT;             /* Bit distribution table */
    hc = jd->huffcode[id][cls] + jd->longofs[id][cls]; /* Code word table */
    hd = jd->huffdata[id][cls] + jd->longofs[id][cls]; /* Data table */
    bl = HUFF_BIT + 1;
    #else
    /* Incremental serch for all codes */
    hb = jd->huffbits[id][cls]; /* Bit distribution table */
    hc = jd->huffcode[id][cls]; /* Code word table */
    hd = jd->huffdata[id][cls]; /* Data table */
    bl = 1;
    #endif
    for (; bl <= 16; bl++) { /* Incremental search */
        nc = *hb++;
        if (nc) {
            d = w >> (wbit - bl);
            do {                          /* Search the code word in this bit length */
                if (d == *hc++) {         /* Matched? */
                    jd->dbit = wbit - bl; /* Snip the huffman code */
                    return *hd;           /* Return the decoded data */
                }
                hd++;
            } while (--nc);
        }
    }
#endif
    return 0 - (int)JDR_FMT1; /* Err: code not found (may be collapted data) */
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int TFT_RGB::JPEG_bitext(JDEC* jd, unsigned int nbit) {
    size_t       dc = jd->dctr;
    uint8_t*     dp = jd->dptr;
    unsigned int d, flg = 0;

#if JD_FASTDECODE == 0
    uint8_t mbit = jd->dbit;
    d = 0;
    do {
        if (!mbit) {            /* Next byte? */
            if (!dc) {          /* No input data is available, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = jd->infunc(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            } else {
                dp++; /* Next data ptr */
            }
            dc--;                                       /* Decrement number of available bytes */
            if (flg) {                                  /* In flag sequence? */
                flg = 0;                                /* Exit flag sequence */
                if (*dp != 0) return 0 - (int)JDR_FMT1; /* Err: unexpected flag is detected (may be collapted data) */
                *dp = 0xFF;                             /* The flag is a data 0xFF */
            } else {
                if (*dp == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence */
                }
            }
            mbit = 0x80; /* Read from MSB */
        }
        d <<= 1; /* Get a bit */
        if (*dp & mbit) d |= 1;
        mbit >>= 1;
        nbit--;
    } while (nbit);

    jd->dbit = mbit;
    jd->dctr = dc;
    jd->dptr = dp;
    return (int)d;

#else
    unsigned int wbit = jd->dbit % 32;
    uint32_t     w = jd->wreg & ((1UL << wbit) - 1);

    while (wbit < nbit) { /* Prepare nbit bits into the working register */
        if (jd->marker) {
            d = 0xFF; /* Input stream stalled, generate stuff bits */
        } else {
            if (!dc) {          /* Buffer empty, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            }
            d = *dp++;
            dc--;
            if (flg) {                      /* In flag sequence? */
                flg = 0;                    /* Exit flag sequence */
                if (d != 0) jd->marker = d; /* Not an escape of 0xFF but a marker */
                d = 0xFF;
            } else {
                if (d == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
        }
        w = w << 8 | d; /* Get 8 bits into the working register */
        wbit += 8;
    }
    jd->wreg = w;
    jd->dbit = wbit - nbit;
    jd->dctr = dc;
    jd->dptr = dp;

    return (int)(w >> ((wbit - nbit) % 32));
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_restart(JDEC* jd, uint16_t rstn) {
    unsigned int i;
    uint8_t*     dp = jd->dptr;
    size_t       dc = jd->dctr;

#if JD_FASTDECODE == 0
    uint16_t d = 0;

    /* Get two bytes from the input stream */
    for (i = 0; i < 2; i++) {
        if (!dc) { /* No input data is available, re-fill input buffer */
            dp = jd->inbuf;
            dc = jd->infunc(jd, dp, JD_SZBUF);
            if (!dc) return JDR_INP;
        } else {
            dp++;
        }
        dc--;
        d = d << 8 | *dp; /* Get a byte */
    }
    jd->dptr = dp;
    jd->dctr = dc;
    jd->dbit = 0;

    /* Check the marker */
    if ((d & 0xFFD8) != 0xFFD0 || (d & 7) != (rstn & 7)) { return JDR_FMT1; /* Err: expected RSTn marker is not detected (may be collapted data) */ }

#else
    uint16_t marker;

    if (jd->marker) { /* Generate a maker if it has been detected */
        marker = 0xFF00 | jd->marker;
        jd->marker = 0;
    } else {
        marker = 0;
        for (i = 0; i < 2; i++) { /* Get a restart marker */
            if (!dc) {            /* No input data is available, re-fill input buffer */
                dp = jd->inbuf;
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return JDR_INP;
            }
            marker = (marker << 8) | *dp++; /* Get a byte */
            dc--;
        }
        jd->dptr = dp;
        jd->dctr = dc;
    }

    /* Check the marker */
    if ((marker & 0xFFD8) != 0xFFD0 || (marker & 7) != (rstn & 7)) { return JDR_FMT1; /* Err: expected RSTn marker was not detected (may be collapted data) */ }

    jd->dbit = 0; /* Discard stuff bits */
#endif

    jd->dcv[2] = jd->dcv[1] = jd->dcv[0] = 0; /* Reset DC offset */
    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::JPEG_block_idct(int32_t* src, jd_yuv_t* dst) {
    const int32_t M13 = (int32_t)(1.41421 * 4096), M2 = (int32_t)(1.08239 * 4096), M4 = (int32_t)(2.61313 * 4096), M5 = (int32_t)(1.84776 * 4096);
    int32_t       v0, v1, v2, v3, v4, v5, v6, v7;
    int32_t       t10, t11, t12, t13;
    int           i;

    /* Process columns */
    for (i = 0; i < 8; i++) {
        v0 = src[8 * 0]; /* Get even elements */
        v1 = src[8 * 2];
        v2 = src[8 * 4];
        v3 = src[8 * 6];

        t10 = v0 + v2; /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[8 * 7]; /* Get odd elements */
        v5 = src[8 * 1];
        v6 = src[8 * 5];
        v7 = src[8 * 3];

        t10 = v5 - v4; /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        src[8 * 0] = v0 + v7; /* Write-back transformed values */
        src[8 * 7] = v0 - v7;
        src[8 * 1] = v1 + v6;
        src[8 * 6] = v1 - v6;
        src[8 * 2] = v2 + v5;
        src[8 * 5] = v2 - v5;
        src[8 * 3] = v3 + v4;
        src[8 * 4] = v3 - v4;

        src++; /* Next column */
    }

    /* Process rows */
    src -= 8;
    for (i = 0; i < 8; i++) {
        v0 = src[0] + (128L << 8); /* Get even elements (remove DC offset (-128) here) */
        v1 = src[2];
        v2 = src[4];
        v3 = src[6];

        t10 = v0 + v2; /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[7]; /* Get odd elements */
        v5 = src[1];
        v6 = src[5];
        v7 = src[3];

        t10 = v5 - v4; /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        /* Descale the transformed values 8 bits and output a row */
#if JD_FASTDECODE >= 1
        dst[0] = (int16_t)((v0 + v7) >> 8);
        dst[7] = (int16_t)((v0 - v7) >> 8);
        dst[1] = (int16_t)((v1 + v6) >> 8);
        dst[6] = (int16_t)((v1 - v6) >> 8);
        dst[2] = (int16_t)((v2 + v5) >> 8);
        dst[5] = (int16_t)((v2 - v5) >> 8);
        dst[3] = (int16_t)((v3 + v4) >> 8);
        dst[4] = (int16_t)((v3 - v4) >> 8);
#else
        dst[0] = BYTECLIP((v0 + v7) >> 8);
        dst[7] = BYTECLIP((v0 - v7) >> 8);
        dst[1] = BYTECLIP((v1 + v6) >> 8);
        dst[6] = BYTECLIP((v1 - v6) >> 8);
        dst[2] = BYTECLIP((v2 + v5) >> 8);
        dst[5] = BYTECLIP((v2 - v5) >> 8);
        dst[3] = BYTECLIP((v3 + v4) >> 8);
        dst[4] = BYTECLIP((v3 - v4) >> 8);
#endif

        dst += 8;
        src += 8; /* Next row */
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_mcu_load(JDEC* jd) {
    int32_t*       tmp = (int32_t*)jd->workbuf; /* Block working buffer for de-quantize and IDCT */
    int            d, e;
    unsigned int   blk, nby, i, bc, z, id, cmp;
    jd_yuv_t*      bp;
    const int32_t* dqf;

    nby = jd->msx * jd->msy; /* Number of Y blocks (1, 2 or 4) */
    bp = jd->mcubuf;         /* Pointer to the first block of MCU */

    for (blk = 0; blk < nby + 2; blk++) {      /* Get nby Y blocks and two C blocks */
        cmp = (blk < nby) ? 0 : blk - nby + 1; /* Component number 0:Y, 1:Cb, 2:Cr */

        if (cmp && jd->ncomp != 3) { /* Clear C blocks if not exist (monochrome image) */
            for (i = 0; i < 64; bp[i++] = 128);
        } else {              /* Load Y/C blocks from input stream */
            id = cmp ? 1 : 0; /* Huffman table ID of this component */

            /* Extract a DC element from input stream */
            d = JPEG_huffext(jd, id, 0);        /* Extract a huffman coded data (bit length) */
            if (d < 0) return (uint8_t)(0 - d); /* Err: invalid code or input */
            bc = (unsigned int)d;
            d = jd->dcv[cmp];                       /* DC value of previous block */
            if (bc) {                               /* If there is any difference from previous block */
                e = JPEG_bitext(jd, bc);            /* Extract data bits */
                if (e < 0) return (uint8_t)(0 - e); /* Err: input */
                bc = 1 << (bc - 1);                 /* MSB position */
                if (!(e & bc)) e -= (bc << 1) - 1;  /* Restore negative value if needed */
                d += e;                             /* Get current value */
                jd->dcv[cmp] = (int16_t)d;          /* Save current DC value for next block */
            }
            dqf = jd->qttbl[jd->qtid[cmp]]; /* De-quantizer table ID for this component */
            tmp[0] = d * dqf[0] >> 8;       /* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */

            /* Extract following 63 AC elements from input stream */
            memset(&tmp[1], 0, 63 * sizeof(int32_t)); /* Initialize all AC elements */
            z = 1;                                    /* Top of the AC elements (in zigzag-order) */
            do {
                d = JPEG_huffext(jd, id, 1);        /* Extract a huffman coded value (zero runs and bit length) */
                if (d == 0) break;                  /* EOB? */
                if (d < 0) return (uint8_t)(0 - d); /* Err: invalid code or input error */
                bc = (unsigned int)d;
                z += bc >> 4;                           /* Skip leading zero run */
                if (z >= 64) return JDR_FMT1;           /* Too long zero run */
                if (bc &= 0x0F) {                       /* Bit length? */
                    d = JPEG_bitext(jd, bc);            /* Extract data bits */
                    if (d < 0) return (uint8_t)(0 - d); /* Err: input device */
                    bc = 1 << (bc - 1);                 /* MSB position */
                    if (!(d & bc)) d -= (bc << 1) - 1;  /* Restore negative value if needed */
                    i = Zig[z];                         /* Get raster-order index */
                    tmp[i] = d * dqf[i] >> 8;           /* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */
                }
            } while (++z < 64); /* Next AC element */

            if (JD_FORMAT != 2 || !cmp) {                         /* C components may not be processed if in grayscale output */
                if (z == 1 || (JD_USE_SCALE && jd->scale == 3)) { /* If no AC element or scale ratio is 1/8, IDCT can be
                                                                                                                         ommited and the block is filled with DC value */
                    d = (jd_yuv_t)((*tmp / 256) + 128);
                    if (JD_FASTDECODE >= 1) {
                        for (i = 0; i < 64; bp[i++] = d);
                    } else {
                        memset(bp, d, 64);
                    }
                } else {
                    JPEG_block_idct(tmp, bp); /* Apply IDCT and store the block to the MCU buffer */
                }
            }
        }

        bp += 64; /* Next block */
    }

    return JDR_OK; /* All blocks have been loaded successfully */
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_mcu_output(JDEC* jd, unsigned int x, unsigned int y) {
    const int    CVACC = (sizeof(int) > 2) ? 1024 : 128; /* Adaptive accuracy for both 16-/32-bit systems */
    unsigned int ix, iy, mx, my, rx, ry;
    int          yy, cb, cr;
    jd_yuv_t *   py, *pc;
    uint8_t*     pix;
    JRECT        rect;

    mx = jd->msx * 8;
    my = jd->msy * 8;                                /* MCU size (pixel) */
    rx = (x + mx <= jd->width) ? mx : jd->width - x; /* Output rectangular size (it may be clipped at right/bottom end of image) */
    ry = (y + my <= jd->height) ? my : jd->height - y;
    if (JD_USE_SCALE) {
        rx >>= jd->scale;
        ry >>= jd->scale;
        if (!rx || !ry) return JDR_OK; /* Skip this MCU if all pixel is to be rounded off */
        x >>= jd->scale;
        y >>= jd->scale;
    }
    rect.left = x;
    rect.right = x + rx - 1; /* Rectangular area in the frame buffer */
    rect.top = y;
    rect.bottom = y + ry - 1;

    if (!JD_USE_SCALE || jd->scale != 3) { /* Not for 1/8 scaling */
        pix = (uint8_t*)jd->workbuf;

        if (JD_FORMAT != 2) { /* RGB output (build an RGB MCU from Y/C component) */
            for (iy = 0; iy < my; iy++) {
                pc = py = jd->mcubuf;
                if (my == 16) { /* Double block height? */
                    pc += 64 * 4 + (iy >> 1) * 8;
                    if (iy >= 8) py += 64;
                } else { /* Single block height */
                    pc += mx * 8 + iy * 8;
                }
                py += iy * 8;
                for (ix = 0; ix < mx; ix++) {
                    cb = pc[0] - 128; /* Get Cb/Cr component and remove offset */
                    cr = pc[64] - 128;
                    if (mx == 16) {                /* Double block width? */
                        if (ix == 8) py += 64 - 8; /* Jump to next block if double block heigt */
                        pc += ix & 1;              /* Step forward chroma pointer every two pixels */
                    } else {                       /* Single block width */
                        pc++;                      /* Step forward chroma pointer every pixel */
                    }
                    yy = *py++; /* Get Y component */
                    *pix++ = /*R*/ JPEG_BYTECLIP(yy + ((int)(1.402 * CVACC) * cr) / CVACC);
                    *pix++ = /*G*/ JPEG_BYTECLIP(yy - ((int)(0.344 * CVACC) * cb + (int)(0.714 * CVACC) * cr) / CVACC);
                    *pix++ = /*B*/ JPEG_BYTECLIP(yy + ((int)(1.772 * CVACC) * cb) / CVACC);
                }
            }
        } else { /* Monochrome output (build a grayscale MCU from Y comopnent) */
            for (iy = 0; iy < my; iy++) {
                py = jd->mcubuf + iy * 8;
                if (my == 16) { /* Double block height? */
                    if (iy >= 8) py += 64;
                }
                for (ix = 0; ix < mx; ix++) {
                    if (mx == 16) {                /* Double block width? */
                        if (ix == 8) py += 64 - 8; /* Jump to next block if double block height */
                    }
                    if (JD_FASTDECODE >= 1) {
                        *pix++ = JPEG_BYTECLIP(*py++); /* Get and store a Y value as grayscale */
                    } else {
                        *pix++ = *py++; /* Get and store a Y value as grayscale */
                    }
                }
            }
        }

        /* Descale the MCU rectangular if needed */
        if (JD_USE_SCALE && jd->scale) {
            unsigned int x, y, r, g, b, s, w, a;
            uint8_t*     op;

            /* Get averaged RGB value of each square correcponds to a pixel */
            s = jd->scale * 2;                       /* Number of shifts for averaging */
            w = 1 << jd->scale;                      /* Width of square */
            a = (mx - w) * (JD_FORMAT != 2 ? 3 : 1); /* Bytes to skip for next line in the square */
            op = (uint8_t*)jd->workbuf;
            for (iy = 0; iy < my; iy += w) {
                for (ix = 0; ix < mx; ix += w) {
                    pix = (uint8_t*)jd->workbuf + (iy * mx + ix) * (JD_FORMAT != 2 ? 3 : 1);
                    r = g = b = 0;
                    for (y = 0; y < w; y++) { /* Accumulate RGB value in the square */
                        for (x = 0; x < w; x++) {
                            r += *pix++;          /* Accumulate R or Y (monochrome output) */
                            if (JD_FORMAT != 2) { /* RGB output? */
                                g += *pix++;      /* Accumulate G */
                                b += *pix++;      /* Accumulate B */
                            }
                        }
                        pix += a;
                    } /* Put the averaged pixel value */
                    *op++ = (uint8_t)(r >> s);     /* Put R or Y (monochrome output) */
                    if (JD_FORMAT != 2) {          /* RGB output? */
                        *op++ = (uint8_t)(g >> s); /* Put G */
                        *op++ = (uint8_t)(b >> s); /* Put B */
                    }
                }
            }
        }
    } else { /* For only 1/8 scaling (left-top pixel in each block are the DC value of the block) */

        /* Build a 1/8 descaled RGB MCU from discrete comopnents */
        pix = (uint8_t*)jd->workbuf;
        pc = jd->mcubuf + mx * my;
        cb = pc[0] - 128; /* Get Cb/Cr component and restore right level */
        cr = pc[64] - 128;
        for (iy = 0; iy < my; iy += 8) {
            py = jd->mcubuf;
            if (iy == 8) py += 64 * 2;
            for (ix = 0; ix < mx; ix += 8) {
                yy = *py; /* Get Y component */
                py += 64;
                if (JD_FORMAT != 2) {
                    *pix++ = /*R*/ JPEG_BYTECLIP(yy + ((int)(1.402 * CVACC) * cr / CVACC));
                    *pix++ = /*G*/ JPEG_BYTECLIP(yy - ((int)(0.344 * CVACC) * cb + (int)(0.714 * CVACC) * cr) / CVACC);
                    *pix++ = /*B*/ JPEG_BYTECLIP(yy + ((int)(1.772 * CVACC) * cb / CVACC));
                } else {
                    *pix++ = yy;
                }
            }
        }
    }

    /* Squeeze up pixel table if a part of MCU is to be truncated */
    mx >>= jd->scale;
    if (rx < mx) { /* Is the MCU spans rigit edge? */
        uint8_t *    s, *d;
        unsigned int x, y;

        s = d = (uint8_t*)jd->workbuf;
        for (y = 0; y < ry; y++) {
            for (x = 0; x < rx; x++) { /* Copy effective pixels */
                *d++ = *s++;
                if (JD_FORMAT != 2) {
                    *d++ = *s++;
                    *d++ = *s++;
                }
            }
            s += (mx - rx) * (JD_FORMAT != 2 ? 3 : 1); /* Skip truncated pixels */
        }
    }

    /* Convert RGB888 to RGB565 if needed */
    if (JD_FORMAT == 1) {
        uint8_t*     s = (uint8_t*)jd->workbuf;
        uint16_t     w, *d = (uint16_t*)s;
        unsigned int n = rx * ry;

        if (jd->swap) {
            do {
                w = (*s++ & 0xF8) << 8;     // RRRRR-----------
                w |= (*s++ & 0xFC) << 3;    // -----GGGGGG-----
                w |= *s++ >> 3;             // -----------BBBBB
                *d++ = (w << 8) | (w >> 8); // Swap bytes
            } while (--n);
        } else {
            do {
                w = (*s++ & 0xF8) << 8;  // RRRRR-----------
                w |= (*s++ & 0xFC) << 3; // -----GGGGGG-----
                w |= *s++ >> 3;          // -----------BBBBB
                *d++ = w;
            } while (--n);
        }
    }

    /* Output the rectangular */
    bool r = JPEG_jd_output(jd, jd->workbuf, &rect);
    return r ? JDR_OK : JDR_INTR;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_jd_prepare(JDEC* jd, uint8_t* pool, size_t sz_pool, void* dev) {
    uint8_t *    seg, b;
    uint16_t     marker;
    unsigned int n, i, ofs;
    size_t       len;
    uint8_t      rc;
    uint8_t      tmp = jd->swap; // Copy the swap flag

    memset(jd, 0, sizeof(JDEC)); /* Clear decompression object (this might be a problem if machine's null pointer is not all bits zero) */
    jd->pool = pool;             /* Work memory */
    jd->sz_pool = sz_pool;       /* Size of given work memory */
    // jd->infunc = infunc;         /* Stream input function */
    jd->device = dev; /* I/O device identifier */
    jd->swap = tmp;   // Restore the swap flag

    jd->inbuf = seg = (uint8_t*)JPEG_alloc_pool(jd, JD_SZBUF); /* Allocate stream input buffer */
    if (!seg) return JDR_MEM1;

    ofs = marker = 0; /* Find SOI marker */
    do {
        if (JPEG_jd_input(jd, seg, 1) != 1) return JDR_INP; /* Err: SOI was not detected */
        ofs++;
        marker = marker << 8 | seg[0];
    } while (marker != 0xFFD8);

    for (;;) { /* Parse JPEG segments */
        /* Get a JPEG marker */
        if (JPEG_jd_input(jd, seg, 4) != 4) return JDR_INP;
        marker = LDB_WORD(seg);  /* Marker */
        len = LDB_WORD(seg + 2); /* Length field */
        if (len <= 2 || (marker >> 8) != 0xFF) return JDR_FMT1;
        len -= 2;       /* Segent content size */
        ofs += 4 + len; /* Number of bytes loaded */

        switch (marker & 0xFF) {
            case 0xC0: /* SOF0 (baseline JPEG) */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                jd->width = LDB_WORD(&seg[3]);                         /* Image width in unit of pixel */
                jd->height = LDB_WORD(&seg[1]);                        /* Image height in unit of pixel */
                jd->ncomp = seg[5];                                    /* Number of color components */
                if (jd->ncomp != 3 && jd->ncomp != 1) return JDR_FMT3; /* Err: Supports only Grayscale and Y/Cb/Cr */

                /* Check each image component */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[7 + 3 * i];                            /* Get sampling factor */
                    if (i == 0) {                                  /* Y component */
                        if (b != 0x11 && b != 0x22 && b != 0x21) { /* Check sampling factor */
                            return JDR_FMT3;                       /* Err: Supports only 4:4:4, 4:2:0 or 4:2:2 */
                        }
                        jd->msx = b >> 4;
                        jd->msy = b & 15;               /* Size of MCU [blocks] */
                    } else {                            /* Cb/Cr component */
                        if (b != 0x11) return JDR_FMT3; /* Err: Sampling factor of Cb/Cr must be 1 */
                    }
                    jd->qtid[i] = seg[8 + 3 * i];         /* Get dequantizer table ID for this component */
                    if (jd->qtid[i] > 3) return JDR_FMT3; /* Err: Invalid ID */
                }
                break;

            case 0xDD: /* DRI - Define Restart Interval */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                jd->nrst = LDB_WORD(seg); /* Get restart interval (MCUs) */
                break;

            case 0xC4: /* DHT - Define Huffman Tables */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                rc = JPEG_create_huffman_tbl(jd, seg, len); /* Create huffman tables */
                if (rc) return rc;
                break;

            case 0xDB: /* DQT - Define Quaitizer Tables */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                rc = JPEG_create_qt_tbl(jd, seg, len); /* Create de-quantizer tables */
                if (rc) return rc;
                break;

            case 0xDA: /* SOS - Start of Scan */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                if (!jd->width || !jd->height) return JDR_FMT1; /* Err: Invalid image size */
                if (seg[0] != jd->ncomp) return JDR_FMT3;       /* Err: Wrong color components */

                /* Check if all tables corresponding to each components have been loaded */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[2 + 2 * i];                               /* Get huffman table ID */
                    if (b != 0x00 && b != 0x11) return JDR_FMT3;      /* Err: Different table number for DC/AC element */
                    n = i ? 1 : 0;                                    /* Component class */
                    if (!jd->huffbits[n][0] || !jd->huffbits[n][1]) { /* Check huffman table for this component */
                        return JDR_FMT1;                              /* Err: Nnot loaded */
                    }
                    if (!jd->qttbl[jd->qtid[i]]) { /* Check dequantizer table for this component */
                        return JDR_FMT1;           /* Err: Not loaded */
                    }
                }

                /* Allocate working buffer for MCU and pixel output */
                n = jd->msy * jd->msx;                                                        /* Number of Y blocks in the MCU */
                if (!n) return JDR_FMT1;                                                      /* Err: SOF0 has not been loaded */
                len = n * 64 * 2 + 64;                                                        /* Allocate buffer for IDCT and RGB output */
                if (len < 256) len = 256;                                                     /* but at least 256 byte is required for IDCT */
                jd->workbuf = JPEG_alloc_pool(jd, len);                                       /* and it may occupy a part of following MCU working buffer for RGB output */
                if (!jd->workbuf) return JDR_MEM1;                                            /* Err: not enough memory */
                jd->mcubuf = (jd_yuv_t*)JPEG_alloc_pool(jd, (n + 2) * 64 * sizeof(jd_yuv_t)); /* Allocate MCU working buffer */
                if (!jd->mcubuf) return JDR_MEM1;                                             /* Err: not enough memory */

                /* Align stream read offset to JD_SZBUF */
                if (ofs %= JD_SZBUF) { jd->dctr = JPEG_jd_input(jd, seg + ofs, (size_t)(JD_SZBUF - ofs)); }
                jd->dptr = seg + ofs - (JD_FASTDECODE ? 0 : 1);

                return JDR_OK; /* Initialization succeeded. Ready to decompress the JPEG image. */

            case 0xC1:                            /* SOF1 */
            case 0xC2:                            /* SOF2 */
            case 0xC3:                            /* SOF3 */
            case 0xC5:                            /* SOF5 */
            case 0xC6:                            /* SOF6 */
            case 0xC7:                            /* SOF7 */
            case 0xC9:                            /* SOF9 */
            case 0xCA:                            /* SOF10 */
            case 0xCB:                            /* SOF11 */
            case 0xCD:                            /* SOF13 */
            case 0xCE:                            /* SOF14 */
            case 0xCF:                            /* SOF15 */
            case 0xD9: /* EOI */ return JDR_FMT3; /* Unsuppoted JPEG standard (may be progressive JPEG) */

            default: /* Unknown segment (comment, exif or etc..) */
            // log_e("Unknown segment %02X", marker);
                /* Skip segment data (null pointer specifies to remove data from the stream) */
                if (JPEG_jd_input(jd, 0, len) != len) return JDR_INP;
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_RGB::JPEG_jd_decomp(JDEC* jd, uint8_t scale) {
    unsigned int x, y, mx, my;
    uint16_t     rst, rsc;
    uint8_t      rc;

    if (scale > (JD_USE_SCALE ? 3 : 0)) return JDR_PAR;
    jd->scale = scale;
    mx = jd->msx * 8;
    my = jd->msy * 8; /* Size of the MCU (pixel) */
    jd->dcv[2] = jd->dcv[1] = jd->dcv[0] = 0; /* Initialize DC values */
    rst = rsc = 0;
    rc = JDR_OK;
    for (y = 0; y < jd->height; y += my) {       /* Vertical loop of MCUs */
        for (x = 0; x < jd->width; x += mx) {    /* Horizontal loop of MCUs */
            if (jd->nrst && rst++ == jd->nrst) { /* Process restart interval if enabled */
                rc = JPEG_restart(jd, rsc++);
                if (rc != JDR_OK) return rc;
                rst = 1;
            }
            rc = JPEG_mcu_load(jd); /* Load an MCU (decompress huffman coded stream, dequantize and apply IDCT) */
            if (rc != JDR_OK) return rc;
            rc = JPEG_mcu_output(jd, x, y); /* Output the MCU (YCbCr to RGB, scaling and output) */
            if (rc != JDR_OK) return rc;
        }
    }
    return rc;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   P N G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool TFT_RGB::drawPngFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {

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

    if(!fs.exists(path)) {
        log_e("File not found: %s", path);
        return NULL;
    }
    png_file = fs.open(path, "r");
    if(!png_file) {
        log_e("Failed to open file for reading");
        return NULL;
    }
    int file_size = png_file.size(); /* get filesize */
    png_buffer = (char*)ps_malloc(file_size);
    png_size = file_size;
    if(!png_buffer) {
        log_e("Failed to allocate memory for file");
        png_file.close();
        return NULL;
    }
    png_file.readBytes(png_buffer, (size_t)file_size);
    png_file.close();
    int err  = png_decode();
//  log_w("png_decode err=%i",err);
    return err == PNG_EOK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
char TFT_RGB::read_bit(uint32_t *bitpointer, const char* bitstream) {
    char result = (char)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
    (*bitpointer)++;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_RGB::read_bits(uint32_t* bitpointer, const char* bitstream, uint32_t nbits) {
    unsigned result = 0, i;
    for(i = 0; i < nbits; i++) result |= ((uint16_t)read_bit(bitpointer, bitstream)) << i;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/* the buffer must be numcodes*2 in size! */
void TFT_RGB::huffman_tree_init(huffman_tree* tree, uint16_t* buffer, uint16_t numcodes, uint16_t maxbitlen) {
    tree->tree2d = buffer;
    tree->numcodes = numcodes;
    tree->maxbitlen = maxbitlen;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate. maxbitlen is the maximum
 * bits that a code in the tree can have. return value is error.*/
void TFT_RGB::huffman_tree_create_lengths(huffman_tree* tree, const uint16_t* bitlen) {
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
    for(bits = 0; bits < tree->numcodes; bits++) { blcount[bitlen[bits]]++; }

    /*step 2: generate the nextcode values */
    for(bits = 1; bits <= tree->maxbitlen; bits++) { nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1; }

    /*step 3: generate all the codes */
    for(n = 0; n < tree->numcodes; n++) {
        if(bitlen[n] != 0) { tree1d[n] = nextcode[bitlen[n]]++; }
    }

    /*convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means uninited, a value >= numcodes is an address to another bit, a value < numcodes is a code. The 2 rows are the 2 possible
     bit values (0 or 1), there are as many columns as codes - 1 a good huffmann tree has N * 2 - 1 nodes, of which N - 1 are internal nodes. Here, the internal nodes are stored (what their 0 and 1
     option point to). There is only memory for such good tree currently, if there are more nodes (due to too long length codes), error 55 will happen */
    for(n = 0; n < tree->numcodes * 2; n++) {
        tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet */
    }

    for(n = 0; n < tree->numcodes; n++) { /*the codes */
        for(i = 0; i < bitlen[n]; i++) {  /*the bits for this code */
            unsigned char bit = (unsigned char)((tree1d[n] >> (bitlen[n] - i - 1)) & 1);
            /* check if oversubscribed */
            if(treepos > tree->numcodes - 2) {
                log_e("oversubscribed");
                png_error = PNG_EMALFORMED;
                return;
            }

            if(tree->tree2d[2 * treepos + bit] == 32767) { /*not yet filled in */
                if(i + 1 == bitlen[n]) {                   /*last bit */
                    tree->tree2d[2 * treepos + bit] = n;   /*put the current code in it */
                    treepos = 0;
                }
                else { /*put address of the next step in here, first that address has to be found of course (it's just
                          nodefilled + 1)... */
                    nodefilled++;
                    tree->tree2d[2 * treepos + bit] =
                        nodefilled + tree->numcodes; /*addresses encoded with numcodes added to it */
                    treepos = nodefilled;
                }
            }
            else { treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes; }
        }
    }

    for(n = 0; n < tree->numcodes * 2; n++) {
        if(tree->tree2d[n] == 32767) { tree->tree2d[n] = 0; /*remove possible remaining 32767's */ }
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_RGB::huffman_decode_symbol(const char* in, uint32_t  * bp, const huffman_tree* codetree, uint32_t inlength) {
    uint16_t      treepos = 0, ct;
    char bit;
    for(;;) {
        /* error: end of input memory reached without endcode */
        if(((*bp) & 0x07) == 0 && ((*bp) >> 3) > inlength) {
            log_e("end of input memory reached without endcode");
            png_error =  PNG_EMALFORMED;
            return 0;
        }

        bit = read_bit(bp, in);

        ct = codetree->tree2d[(treepos << 1) | bit];
        if(ct < codetree->numcodes) { return ct; }

        treepos = ct - codetree->numcodes;
        if(treepos >= codetree->numcodes) {
            log_e("error, treepos is larger than numcodes");
            png_error =  PNG_EMALFORMED;
            return 0;
        }
    }
    return 0;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/* get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
void TFT_RGB::get_tree_inflate_dynamic(huffman_tree* codetree, huffman_tree* codetreeD, huffman_tree* codelengthcodetree, const char* in, uint32_t *bp, uint32_t inlength) {
    uint16_t codelengthcode[NUM_CODE_LENGTH_CODES];
    uint16_t bitlen[NUM_DEFLATE_CODE_SYMBOLS];
    uint16_t bitlenD[NUM_DISTANCE_SYMBOLS];
    uint16_t n, hlit, hdist, hclen, i;

    /*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated */
    /*C-code note: use no "return" between ctor and dtor of an uivector! */
    if((*bp) >> 3 >= inlength - 2) {
        log_e("error, bit pointer will jump past memory");
        png_error = PNG_EMALFORMED;
        return;
    }

    /* clear bitlen arrays */
    memset(bitlen, 0, sizeof(bitlen));
    memset(bitlenD, 0, sizeof(bitlenD));

    /*the bit pointer is or will go past the memory */
    hlit = read_bits(bp, in, 5) +
           257; /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already */
    hdist = read_bits(bp, in, 5) +
            1; /*number of distance codes. Unlike the spec, the value 1 is added to it here already */
    hclen = read_bits(bp, in, 4) +
            4; /*number of code length codes. Unlike the spec, the value 4 is added to it here already */

    for(i = 0; i < NUM_CODE_LENGTH_CODES; i++) {
        if(i < hclen) { codelengthcode[CLCL[i]] = read_bits(bp, in, 3); }
        else { codelengthcode[CLCL[i]] = 0; /*if not, it must stay 0 */ }
    }

    huffman_tree_create_lengths(codelengthcodetree, codelengthcode);

    /* bail now if we encountered an error earlier */
    if(png_error != PNG_EOK) { return; }

    /*now we can use this tree to read the lengths for the tree that this function will return */
    i = 0;
    while(i < hlit + hdist) { /*i is the current symbol we're reading in the part that contains the code lengths of
                                 lit/len codes and dist codes */
        unsigned code = huffman_decode_symbol(in, bp, codelengthcodetree, inlength);
        if(png_error != PNG_EOK) { break; }

        if(code <= 15) { /*a length code */
            if(i < hlit) { bitlen[i] = code; }
            else { bitlenD[i - hlit] = code; }
            i++;
        }
        else if(code == 16) {       /*repeat previous */
            unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6) */
            unsigned value;         /*set value to the previous code */

            if((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }
            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 2);

            if((i - 1) < hlit) { value = bitlen[i - 1]; }
            else { value = bitlenD[i - hlit - 1]; }

            /*repeat this value in the next lengths */
            for(n = 0; n < replength; n++) {
                /* i is larger than the amount of codes */
                if(i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }

                if(i < hlit) { bitlen[i] = value; }
                else { bitlenD[i - hlit] = value; }
                i++;
            }
        }
        else if(code == 17) {       /*repeat "0" 3-10 times */
            unsigned replength = 3; /*read in the bits that indicate repeat length */
            if((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }

            /*error, bit pointer jumps past memory */
            replength += read_bits(bp, in, 3);

            /*repeat this value in the next lengths */
            for(n = 0; n < replength; n++) {
                /* error: i is larger than the amount of codes */
                if(i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }

                if(i < hlit) { bitlen[i] = 0; }
                else { bitlenD[i - hlit] = 0; }
                i++;
            }
        }
        else if(code == 18) {        /*repeat "0" 11-138 times */
            unsigned replength = 11; /*read in the bits that indicate repeat length */
            /* error, bit pointer jumps past memory */
            if((*bp) >> 3 >= inlength) {
                log_e("error, bit pointer jumps past memory");
                png_error = PNG_EMALFORMED;
                break;
            }

            replength += read_bits(bp, in, 7);

            /*repeat this value in the next lengths */
            for(n = 0; n < replength; n++) {
                /* i is larger than the amount of codes */
                if(i >= hlit + hdist) {
                    log_e("error: i is larger than the amount of codes");
                    png_error = PNG_EMALFORMED;
                    break;
                }
                if(i < hlit) bitlen[i] = 0;
                else
                    bitlenD[i - hlit] = 0;
                i++;
            }
        }
        else {
            /* somehow an unexisting code appeared. This can never happen. */
            log_e("error: unexisting code");
            png_error = PNG_EMALFORMED;
            break;
        }
    }

    if(png_error == PNG_EOK && bitlen[256] == 0) { log_e("image data is not a valid PNG image"); png_error = PNG_EMALFORMED;}

    /*the length of the end code 256 must be larger than 0 */
    /*now we've finally got hlit and hdist, so generate the code trees, and the function is done */
    if(png_error == PNG_EOK) { huffman_tree_create_lengths(codetree, bitlen); }
    if(png_error == PNG_EOK) { huffman_tree_create_lengths(codetreeD, bitlenD); }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*inflate a block with dynamic of fixed Huffman tree*/
void TFT_RGB::inflate_huffman(char* out, uint32_t   outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength, uint16_t btype) {
    uint16_t codetree_buffer[DEFLATE_CODE_BUFFER_SIZE];
    uint16_t codetreeD_buffer[DISTANCE_BUFFER_SIZE];
    uint16_t done = 0;

    huffman_tree codetree;
    huffman_tree codetreeD;

    if(btype == 1) {
        /* fixed trees */
        huffman_tree_init(&codetree, (uint16_t*)FIXED_DEFLATE_CODE_TREE, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
        huffman_tree_init(&codetreeD, (uint16_t*)FIXED_DISTANCE_TREE, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
    }
    else if(btype == 2) {
        /* dynamic trees */
        uint16_t     codelengthcodetree_buffer[CODE_LENGTH_BUFFER_SIZE];
        huffman_tree codelengthcodetree;

        huffman_tree_init(&codetree, codetree_buffer, NUM_DEFLATE_CODE_SYMBOLS, DEFLATE_CODE_BITLEN);
        huffman_tree_init(&codetreeD, codetreeD_buffer, NUM_DISTANCE_SYMBOLS, DISTANCE_BITLEN);
        huffman_tree_init(&codelengthcodetree, codelengthcodetree_buffer, NUM_CODE_LENGTH_CODES, CODE_LENGTH_BITLEN);
        get_tree_inflate_dynamic(&codetree, &codetreeD, &codelengthcodetree, in, bp, inlength);
    }

    while(done == 0) {
        unsigned code = huffman_decode_symbol(in, bp, &codetree, inlength);
        if(png_error != PNG_EOK) { return; }

        if(code == 256) {
            /* end code */
            done = 1;
        }
        else if(code <= 255) {
            /* literal symbol */
            if((*pos) >= outsize) {
                log_e("output buffer is too small");
                png_error = PNG_EMALFORMED;
                return;
            }

            /* store output */
            out[(*pos)++] = (unsigned char)(code);
        }
        else if(code >= FIRST_LENGTH_CODE_INDEX && code <= LAST_LENGTH_CODE_INDEX) { /*length code */
            /* part 1: get length base */
            uint32_t   length = LENGTH_BASE[code - FIRST_LENGTH_CODE_INDEX];
            unsigned      codeD, distance, numextrabitsD;
            uint32_t   start, forward, backward, numextrabits;

            /* part 2: get extra bits and add the value of that to length */
            numextrabits = LENGTH_EXTRA[code - FIRST_LENGTH_CODE_INDEX];

            /* error, bit pointer will jump past memory */
            if(((*bp) >> 3) >= inlength) {
                log_e("bit pointer will jump past memory");
                png_error = PNG_EMALFORMED;
                return;
            }
            length += read_bits(bp, in, numextrabits);

            /*part 3: get distance code */
            codeD = huffman_decode_symbol(in, bp, &codetreeD, inlength);
            if(png_error != PNG_EOK) { return; }

            /* invalid distance code (30-31 are never used) */
            if(codeD > 29) {
                log_e("invalid distance code");
                png_error = PNG_EMALFORMED;
                return;
            }

            distance = DISTANCE_BASE[codeD];

            /*part 4: get extra bits from distance */
            numextrabitsD = DISTANCE_EXTRA[codeD];

            /* error, bit pointer will jump past memory */
            if(((*bp) >> 3) >= inlength) {
                log_e("bit pointer will jump past memory");
                png_error =  PNG_EMALFORMED;
                return;
            }

            distance += read_bits(bp, in, numextrabitsD);

            /*part 5: fill in all the out[n] values based on the length and dist */
            start = (*pos);
            backward = start - distance;

            if((*pos) + length >= outsize) {
                log_e("output buffer is too small");
                png_error = PNG_EMALFORMED;
                return;
            }

            for(forward = 0; forward < length; forward++) {
                out[(*pos)++] = out[backward];
                backward++;

                if(backward >= start) { backward = start - distance; }
            }
        }
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::inflate_uncompressed(char* out, uint32_t outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength) {
    uint32_t   p;
    unsigned      len, nlen, n;

    /* go to first boundary of byte */
    while(((*bp) & 0x7) != 0) { (*bp)++; }
    p = (*bp) / 8; /*byte position */

    /* read len (2 bytes) and nlen (2 bytes) */
    if(p >= inlength - 4) {
        log_e("p >= inlength - 4");
        png_error = PNG_EMALFORMED;
        return;
    }

    len = in[p] + 256 * in[p + 1];
    p += 2;
    nlen = in[p] + 256 * in[p + 1];
    p += 2;

    /* check if 16-bit nlen is really the one's complement of len */
    if(len + nlen != 65535) {
        log_e("nlen is not one's complement of len");
        png_error = PNG_EMALFORMED;
        return;
    }

    if((*pos) + len >= outsize) {
        log_e("output buffer is too small");
        png_error = PNG_EMALFORMED;
        return;
    }

    /* read the literal data: len bytes are now stored in the out buffer */
    if(p + len > inlength) {
        log_e("p + len > inlength");
        png_error = PNG_EMALFORMED;
        return;
    }

    for(n = 0; n < len; n++) { out[(*pos)++] = in[p++]; }

    (*bp) = p * 8;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*inflate the deflated data (cfr. deflate spec); return value is the error*/
int8_t TFT_RGB::uz_inflate_data(char* out, uint32_t outsize, const char* in, uint32_t insize, uint32_t inpos) {
    uint32_t   bp = 0;  /*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte) */
    uint32_t   pos = 0; /*byte position in the out buffer */
    uint16_t done = 0;

    while(done == 0) {
        uint16_t btype;

        /* ensure next bit doesn't point past the end of the buffer */
        if((bp >> 3) >= insize) {
            log_e("bp >> 3 >= insize");
            return PNG_EMALFORMED;
        }

        /* read block control bits */
        done = read_bit(&bp, &in[inpos]);
        btype = read_bit(&bp, &in[inpos]) | (read_bit(&bp, &in[inpos]) << 1);

        /* process control type appropriateyly */
        if(btype == 3) {
            log_e("btype == 3");
            png_error = PNG_EMALFORMED;
            return png_error;
        }
        else if(btype == 0) {
            inflate_uncompressed(out, outsize, &in[inpos], &bp, &pos, insize); /*no compression */
        }
        else {
            inflate_huffman(out, outsize, &in[inpos], &bp, &pos, insize, btype); /*compression, btype 01 or 10 */
        }

        /* stop if an error has occured */
        if(png_error != PNG_EOK) { return png_error; }
    }

    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t TFT_RGB::uz_inflate(char* out, uint32_t outsize, const char* in, uint32_t insize) {
    /* we require two bytes for the zlib data header */
    if(insize < 2) {
        log_e("insize < 2");
        return PNG_EMALFORMED;
    }

    /* 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way */
    if((in[0] * 256 + in[1]) % 31 != 0) {
        log_e("FCHECK value is supposed to be made that way");
        return PNG_EMALFORMED;
    }

    /*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec */
    if((in[0] & 15) != 8 || ((in[0] >> 4) & 15) > 7) {
        log_e("only compression method 8: inflate with sliding window of 32k is supported by the PNG spec");
        return PNG_EMALFORMED;
    }

    /* the specification of PNG says about the zlib stream: "The additional flags shall not specify a preset
     * dictionary." */
    if(((in[1] >> 5) & 1) != 0) {
        log_e("The additional flags shall not specify a preset dictionary.");
        return PNG_EMALFORMED;
    }

    /* create output buffer */
    uz_inflate_data(out, outsize, in, insize, 2);

    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Paeth predicter, used by PNG filter type 4*/
int TFT_RGB::paeth_predictor(int a, int b, int c) {
    int p = a + b - c;
    int pa = p > a ? p - a : a - p;
    int pb = p > b ? p - b : b - p;
    int pc = p > c ? p - c : c - p;

    if(pa <= pb && pa <= pc) return a;
    else if(pb <= pc)
        return b;
    else
        return c;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::unfilter_scanline(char* recon, const char* scanline, const char* precon, uint32_t bytewidth, unsigned char filterType, uint32_t length) {
    /*
       For PNG filter method 0
       unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1) precon is the previous unfiltered scanline, recon the result,
       scanline the current one the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead recon and scanline MAY be the same memory address!
       precon must be disjoint.
     */

    uint32_t   i;
    switch(filterType) {
        case 0:
            for(i = 0; i < length; i++) recon[i] = scanline[i];
            break;
        case 1:
            for(i = 0; i < bytewidth; i++) recon[i] = scanline[i];
            for(i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth];
            break;
        case 2:
            if(precon)
                for(i = 0; i < length; i++) recon[i] = scanline[i] + precon[i];
            else
                for(i = 0; i < length; i++) recon[i] = scanline[i];
            break;
        case 3:
            if(precon) {
                for(i = 0; i < bytewidth; i++) recon[i] = scanline[i] + precon[i] / 2;
                for(i = bytewidth; i < length; i++) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
            }
            else {
                for(i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                for(i = bytewidth; i < length; i++) recon[i] = scanline[i] + recon[i - bytewidth] / 2;
            }
            break;
        case 4:
            if(precon) {
                for(i = 0; i < bytewidth; i++)
                    recon[i] = (unsigned char)(scanline[i] + paeth_predictor(0, precon[i], 0));
                for(i = bytewidth; i < length; i++)
                    recon[i] = (unsigned char)(scanline[i] +
                                               paeth_predictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
            }
            else {
                for(i = 0; i < bytewidth; i++) recon[i] = scanline[i];
                for(i = bytewidth; i < length; i++)
                    recon[i] = (unsigned char)(scanline[i] + paeth_predictor(recon[i - bytewidth], 0, 0));
            }
            break;
        default:
        log_e("recon: %s, scanline: %s, precon: %s, bytewidth: %d, length: %d, filterType: %d", recon, scanline, precon, bytewidth, length, filterType);
            png_error = PNG_EMALFORMED;
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::unfilter(char* out, const char* in, unsigned w, unsigned h, unsigned bpp) {
    /*
       For PNG filter method 0
       this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times) out must have enough bytes allocated already, in must have the
       scanlines + 1 filtertype byte per scanline w and h are image dimensions or dimensions of reduced image, bpp is bpp per pixel in and out are allowed to be the same memory address!
     */

    unsigned y;
    char*    prevline = 0;

    uint32_t   bytewidth =
        (bpp + 7) / 8; /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise */
    uint32_t   linebytes = (w * bpp + 7) / 8;

    for(y = 0; y < h; y++) {
        uint32_t   outindex = linebytes * y;
        uint32_t   inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row */
        unsigned char filterType = in[inindex];

        unfilter_scanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
        if(png_error != PNG_EOK) { return; }

        prevline = &out[outindex];
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::remove_padding_bits(char* out, const char* in, uint32_t olinebits, uint32_t ilinebits, unsigned h) {
    /*
       After filtering there are still padding bpp if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working with pure
       image buffers for the Adam7 code, the color convert code and the output to the user. in and out are allowed to be the same buffer, in may also be higher but still overlapping;
       in must have >= ilinebits*h bpp, out must have >= olinebits*h bpp, olinebits must be <= ilinebits also used to move bpp after earlier such operations happened, e.g. in a sequence of reduced
       images from Adam7 only useful if (ilinebits - olinebits) is a value in the range 1..7
     */
    unsigned      y;
    uint32_t   diff = ilinebits - olinebits;
    uint32_t   obp = 0, ibp = 0; /*bit pointers */
    for(y = 0; y < h; y++) {
        uint32_t   x;
        for(x = 0; x < olinebits; x++) {
            unsigned char bit = (unsigned char)((in[(ibp) >> 3] >> (7 - ((ibp) & 0x7))) & 1);
            ibp++;

            if(bit == 0) out[(obp) >> 3] &= (unsigned char)(~(1 << (7 - ((obp) & 0x7))));
            else
                out[(obp) >> 3] |= (1 << (7 - ((obp) & 0x7)));
            ++obp;
        }
        ibp += diff;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from the IDAT chunks*/
void TFT_RGB::post_process_scanlines(char* out, char* in) {
    unsigned bpp = png_get_bpp();
    unsigned w = png_width;
    unsigned h = png_height;

    if(bpp == 0) {
        log_e("bpp == 0");
        png_error = PNG_EMALFORMED;
        return;
    }

    if(bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
        unfilter(in, in, w, h, bpp);
        if(png_error != PNG_EOK) { return; }
        remove_padding_bits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
    }
    else {
        unfilter(out, in, w, h, bpp); /*we can immediatly filter into the out buffer, no other steps needed */
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
int8_t TFT_RGB::png_decode() {
    const char*    chunk;
    char*          compressed;
    char*          inflated;
    uint32_t       compressed_size = 0, compressed_index = 0;
    uint32_t       inflated_size;
    int8_t         error = 0;

    /* if we have an error state, bail now */
    if(error != PNG_EOK) { return error; }

    /* parse the main header, if necessary */
    png_read_header();
    if(error != PNG_EOK) { return error; }

    /* if the state is not HEADER (meaning we are ready to decode the image), stop now */
    if(png_state != PNG_HEADER) { return error; }
    chunk = png_buffer + 33;

    /* scan through the chunks, finding the size of all IDAT chunks, and also verify general well-formed-ness */
    while(chunk < png_buffer + png_size) {
        uint32_t length;
        const char*   data; /*the data in the chunk */ (void)data;

        /* make sure chunk header is not larger than the total compressed */
        if((uint32_t  )(chunk - png_buffer + 12) > png_size) {
            log_e("png_decode: chunk header is not larger than the total compressed");
            error = PNG_EMALFORMED;
            return error;
        }

        /* get length; sanity check it */
        length = upng_chunk_length(chunk);
        if(length > INT_MAX) {
            log_e("png_decode: chunk length is too large");
            error = PNG_EMALFORMED;
            return error;
        }

        /* make sure chunk header+paylaod is not larger than the total compressed */
        if((uint32_t  )(chunk - png_buffer + length + 12) > png_size) {
            log_e("png_decode: chunk header+paylaod is not larger than the total compressed");
            error = PNG_EMALFORMED;
            return error;
        }

        /* get pointer to payload */
        data = chunk + 8;

        /* parse chunks */
        if(upng_chunk_type(chunk) == CHUNK_IDAT) { compressed_size += length; }
        else if(upng_chunk_type(chunk) == CHUNK_IEND) { break; }
        else if(upng_chunk_critical(chunk)) {
            log_e("png_decode: unsupported critical chunk type");
            error = PNG_EUNSUPPORTED;
            return error;
        }

        chunk += upng_chunk_length(chunk) + 12;
    }

    /* allocate enough space for the (compressed and filtered) image data */
    compressed = (char*)ps_malloc(compressed_size);
    if(compressed == NULL) {
        log_e("png_decode: out of memory");
        error = PNG_ENOMEM;
        return error;
    }

    /* scan through the chunks again, this time copying the values into
     * our compressed buffer.  there's no reason to validate anything a second time. */
    chunk = png_buffer + 33;
    while(chunk < png_buffer + png_size) {
        uint32_t   length;
        const char*   data; /*the data in the chunk */

        length = upng_chunk_length(chunk);
        data = chunk + 8;

        /* parse chunks */
        if(upng_chunk_type(chunk) == CHUNK_IDAT) {
            memcpy(compressed + compressed_index, data, length);
            compressed_index += length;
        }
        else if(upng_chunk_type(chunk) == CHUNK_IEND) { break; }

        chunk += upng_chunk_length(chunk) + 12;
    }
    /* allocate space to store inflated (but still filtered) data */
    inflated_size = ((png_width * (png_height * png_get_bpp() + 7)) / 8) + png_height;
    inflated = (char*)ps_malloc(inflated_size);

    if(inflated == NULL) {
        free(compressed);
        log_e("png_decode: out of memory");
        error = PNG_ENOMEM;
        return error;
    }

    /* decompress image data */
    error = uz_inflate(inflated, inflated_size, compressed, compressed_size);
    if(error != PNG_EOK) {
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

    if(png_error != PNG_EOK) {
        if(png_outbuffer) { free(png_outbuffer); png_outbuffer = NULL;}
        png_size = 0;
    }
    else { png_state = PNG_DECODED; }

    /* we are done with our input buffer; free it */
    if(png_buffer) {
        free(png_buffer);
        png_buffer = NULL;
    }
    png_size = 0;

    png_draw_into_Framebuffer(png_pos_x, png_pos_y, png_width, png_height, png_outbuffer, png_outbuff_size, png_format);

    if(png_outbuffer) {
        free(png_outbuffer);
        png_outbuffer = NULL;
    }
    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_RGB::png_format_t TFT_RGB::png_determine_format() {
    switch(png_color_type) {
        case PNG_LUM:
            switch(png_color_depth) {
                case 1:     return PNG_LUMINANCE1;
                case 2:     return PNG_LUMINANCE2;
                case 4:     return PNG_LUMINANCE4;
                case 8:     return PNG_LUMINANCE8;
                default:    return PNG_BADFORMAT;
            }
            break;
        case PNG_RGB:
            switch(png_color_depth) {
                case 8:     return PNG_RGB8;
                case 16:    return PNG_RGB16;
                default:    return PNG_BADFORMAT;
            }
            break;
        case PNG_PAL:
            switch(png_color_depth) {
                case 1:     return PNG_PALLETTE1;
                case 2:     return PNG_PALLETTE2;
                case 4:     return PNG_PALLETTE4;
                case 8:     return PNG_PALLETTE8;
                default:    return PNG_BADFORMAT;
            }
            break;
        case PNG_LUMA:
            switch(png_color_depth) {
                case 1:     return PNG_LUMINANCE_ALPHA1;
                case 2:     return PNG_LUMINANCE_ALPHA2;
                case 4:     return PNG_LUMINANCE_ALPHA4;
                case 8:     return PNG_LUMINANCE_ALPHA8;
                default:    return PNG_BADFORMAT;
            }
            break;
        case PNG_RGBA:
            switch(png_color_depth) {
                case 8:     return PNG_RGBA8;
                case 16:    return PNG_RGBA16;
                default:    return PNG_BADFORMAT;
            }
            break;
        default:            return PNG_BADFORMAT;
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*read the information from the header and store it in the upng_Info. return value is error*/
bool TFT_RGB::png_read_header() {
    png_state = PNG_HEADER;
    if(png_size < 29) {
        png_error = PNG_ENOTPNG;
        log_e("png_size < 29");
        return false;
    }

    if(png_buffer[0] != 137 || png_buffer[1] != 80 || png_buffer[2] != 78 ||
        png_buffer[3] != 71 || /* check that PNG header matches expected value */
        png_buffer[4] != 13 || png_buffer[5] != 10 || png_buffer[6] != 26 || png_buffer[7] != 10) {
        png_error = PNG_ENOTPNG;
        log_e("image data does not have a PNG header");
        return false;
    }

    /* check that the first chunk is the IHDR chunk */
    if(MAKE_DWORD_PTR(png_buffer + 12) != CHUNK_IHDR) {
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

    if(png_format == PNG_BADFORMAT) {
        log_e("image color format is not supported");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (only allowed value in spec) */
    if(png_buffer[26] != 0) {
        png_error = PNG_EMALFORMED;
        log_e("image data is not a valid PNG image");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (only allowed value in spec) */
    if(png_buffer[27] != 0) {
        png_error = PNG_EMALFORMED;
        log_e("image data is not a valid PNG image");
        return false;
    }

    /* check that the compression method (byte 27) is 0 (spec allows 1, but uPNG does not support it) */
    if(png_buffer[28] != 0) {
        png_error = PNG_EUNINTERLACED;
        log_e("image interlacing is not supported");
        return false;
    }
    return true;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int8_t TFT_RGB::png_get_error() { return png_error; }
uint16_t TFT_RGB::png_get_width() { return png_width; }
uint16_t TFT_RGB::png_get_height() { return png_height; }
uint16_t TFT_RGB::png_get_bpp() { return png_get_bitdepth() * png_get_components(); }
const char* TFT_RGB::png_get_outbuffer(){return png_outbuffer;}
uint32_t TFT_RGB::png_get_size(){return png_outbuff_size;}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_RGB::png_get_components() {
    switch(png_color_type) {
        case PNG_LUM:
            return 1;
        case PNG_RGB:
            return 3;
        case PNG_LUMA:
            return 2;
        case PNG_RGBA:
            return 4;
        case PNG_PAL:
            return 1;
        default:
            return 0;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_RGB::png_get_bitdepth() { return png_color_depth; }
//_______________________________________________________________________________________________________________________________
uint16_t TFT_RGB::png_get_pixelsize() {
    uint16_t bits = png_get_bitdepth() * png_get_components();
    bits += bits % 8;
    return bits;
}

TFT_RGB::png_format_t TFT_RGB::png_get_format() { return png_format; }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::png_GetPixel(void* pixel, int x, int y) {
    uint32_t bpp = png_get_bpp();
    //    Serial.printf("\nbbp=%i\n",(int)bpp);
    uint32_t   Bpp = ((bpp + 7) / 8);
    uint32_t   position = (png_width * y + x) * Bpp;
    //    Serial.printf("\nposition in file=%li\n",(long)position);
    memcpy(pixel, png_buffer + position, Bpp);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Initializing color variables */

TFT_RGB::png_s_rgb16b* TFT_RGB::InitColorR5G6B5() {
png_s_rgb16b* color = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    if(color != 0) { ResetColor(color); }
    return color;
}
TFT_RGB::png_s_rgb18b* TFT_RGB::InitColorR6G6B6() {
    png_s_rgb18b* color = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    if(color != 0) { ResetColor(color); }
    return color;
}
TFT_RGB::png_s_rgb24b* TFT_RGB::InitColorR8G8B8() {
    png_s_rgb24b* color = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    if(color != 0) { ResetColor(color); }
    return color;
}

void TFT_RGB::InitColor(png_s_rgb16b** dst) {
    *dst = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    ResetColor(*dst);
}
void TFT_RGB::InitColor(png_s_rgb18b** dst) {
    *dst = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    ResetColor(*dst);
}
void TFT_RGB::InitColor(png_s_rgb24b** dst) {
    *dst = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    ResetColor(*dst);
}

void TFT_RGB::ResetColor(png_s_rgb16b* dst) { *dst = (png_s_rgb16b){0, 0, 0, 0}; }
void TFT_RGB::ResetColor(png_s_rgb18b* dst) { *dst = (png_s_rgb18b){0, 0, 0, 0}; }
void TFT_RGB::ResetColor(png_s_rgb24b* dst) { *dst = (png_s_rgb24b){0, 0, 0, 0}; }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Converting between colors*/

void TFT_RGB::png_rgb24bto18b(png_s_rgb18b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 2;  // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 2;  // 3;//2;
}

void TFT_RGB::png_rgb24bto16b(png_s_rgb16b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 3;  // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 3;  // 3;//2;
}
void TFT_RGB::png_rgb18btouint32(uint32_t* dst, png_s_rgb18b* src) { memcpy(dst, src, sizeof(png_s_rgb18b)); }
void TFT_RGB::png_rgb16btouint32(uint32_t* dst, png_s_rgb16b* src) { memcpy(dst, src, sizeof(png_s_rgb16b)); }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_RGB::png_draw_into_Framebuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char* rgbaBuffer, uint32_t png_outbuff_size, uint8_t png_format) {

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            // Berechne die Zielposition im Framebuffer
            uint16_t screen_x = x + col;
            uint16_t screen_y = y + row;

            // Prüfe, ob das Pixel innerhalb des Bildschirms liegt
            if (screen_x >= m_h_res || screen_y >= m_v_res) {
                continue;
            }

            // Berechne den Index im RGBA-Buffer (basierend auf Bildbreite w)
            int rgbaIndex = (row * w + col) * 4; // 4 Bytes pro Pixel (RGBA)

            uint8_t r = rgbaBuffer[rgbaIndex];     // Rot (8 Bit)
            uint8_t g = rgbaBuffer[rgbaIndex + 1]; // Grün (8 Bit)
            uint8_t b = rgbaBuffer[rgbaIndex + 2]; // Blau (8 Bit)
            uint8_t a = rgbaBuffer[rgbaIndex + 3]; // Alpha (8 Bit)

            if (a == 0) {
                // Falls Alpha 0 ist → Pixel bleibt unverändert
                continue;
            }

            // Berechne den Framebuffer-Index mit x- und y-Versatz
            int fbIndex = screen_y * m_h_res + screen_x;

            if (a < 255) {
                // **Alpha-Blending mit vorhandenem Framebuffer-Wert**

                // 1. Bestehenden RGB565-Wert auslesen
                uint16_t oldColor = m_framebuffer[0][fbIndex];

                // 2. Bestehendes RGB565-Pixel in 8-Bit RGB umwandeln
                uint8_t oldR = ((oldColor >> 11) & 0x1F) << 3;
                uint8_t oldG = ((oldColor >> 5) & 0x3F) << 2;
                uint8_t oldB = (oldColor & 0x1F) << 3;

                // 3. Alpha-Blending Formel anwenden
                uint8_t newR = ((r * a) + (oldR * (255 - a))) / 255;
                uint8_t newG = ((g * a) + (oldG * (255 - a))) / 255;
                uint8_t newB = ((b * a) + (oldB * (255 - a))) / 255;

                // 4. Neue Farbe wieder in RGB565 umwandeln
                m_framebuffer[0][fbIndex] = ((newR >> 3) << 11) |  // 8->5 Bit (Rot)
                                             ((newG >> 2) << 5)  |  // 8->6 Bit (Grün)
                                             (newB >> 3);         // 8->5 Bit (Blau)
            } else {
                // **Normale RGB565-Konvertierung (kein Blending nötig, volle Deckkraft)**
                m_framebuffer[0][fbIndex] = ((r >> 3) << 11) |  // 8->5 Bit (Rot)
                                             ((g >> 2) << 5)  |  // 8->6 Bit (Grün)
                                             (b >> 3);         // 8->5 Bit (Blau)
            }
        }
    }

    // Nur den veränderten Bereich zeichnen
    panelDrawBitmap(x, y, x + w, y + h, &m_framebuffer[0][y * m_h_res + x]);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
