// first release on 01/2025
// updated on Mar 01 2026

#include "Arduino.h"
#ifdef CONFIG_IDF_TARGET_ESP32S3
    #pragma once
    #include "../../src/settings.h"
    #include "FS.h"
    #include "SD.h"
    #include "SD_MMC.h"
    #include "SPI.h"
    #include "Wire.h"
    #include "driver/gpio.h"
    #include "esp_lcd_panel_interface.h"
    #include "esp_lcd_panel_io.h"
    #include "esp_lcd_panel_ops.h"
    #include "esp_lcd_panel_rgb.h"
    #include "esp_lcd_panel_vendor.h"
    #include "esp_log.h"
    #include "fonts/Arial.h"
    #include "fonts/BigNumbers.h"
    #include "fonts/FreeSerifItalic.h"
    #include "fonts/Garamond.h"
    #include "fonts/TimesNewRoman.h"
    #include "fonts/Z003.h"
    #include "tft_base.h"
    #include "fonts/fontsdef.h"
    #include "tft_structures.h"
    #include "vector"

using namespace std;

class TFT_RGB : public TFT_Base {
  public:
    enum Rotate { _0, _90, _180, _270 };
    TFT_RGB();
    ~TFT_RGB() { ; }

  private:
    static bool on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t* edata, void* user_ctx);
    bool        handle_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t* edata);

  public:
    void     loop();
    void     reset();
    uint64_t getVsyncCounter() { return m_vsyncCounter; }
    void     clearVsyncCounter() { m_vsyncCounter = 0; }
    void     begin(const Pins& newPins, const Timing& newTiming);
    void     setDisplayInversion(bool i);
    void     setRotation(uint8_t r);
    // Recommended Non-Transaction
    void            readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data);
  private:
    Pins                   m_pins;
    Timing                 m_timing;
    esp_lcd_panel_handle_t m_panel;
    SemaphoreHandle_t      m_vsync_semaphore;
    TaskHandle_t           m_refresh_task_handle = NULL;
    bool                   m_refresh = false;

  private:
    bool        panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void* bitmap) override;
    void        afterTextDraw(uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H) override;

  private:
    uint64_t       m_vsyncCounter = 0;
};

#endif // CONFIG_IDF_TARGET_ESP32S3
