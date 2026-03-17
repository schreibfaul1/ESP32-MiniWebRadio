// first release on 09/2019
// updated on Apr 19 2025

#pragma once

#include "../../src/settings.h"
#include "Arduino.h"
#include "Audio.h"
#include "driver/gpio.h"
#include "fonts/Arial.h"
#include "fonts/BigNumbers.h"
#include "fonts/FreeSerifItalic.h"
#include "fonts/Garamond.h"
#include "fonts/TimesNewRoman.h"
#include "fonts/Z003.h"
#include "fonts/fontsdef.h"
#include "tft_base.h"

using namespace std;

#define MADCTL     0x36 // Memory Data Access Control
#define CASET      0x2A // Column Address Set
#define RASET      0x2B // Row Address Set
#define RAMWR      0x2C // Write_memory_start
#define PASET      0x2B // Page Address Set
#define INVON      0x21 // Display Invert On
#define INVOFF     0x20 // Display Invert Off
#define MADCTL_MV  0x20
#define MADCTL_BGR 0x08
#define SLPOUT     0x11 // Sleep OUT
#define DISPON     0x29 // Display ON

class TFT_SPI : public TFT_Base {
  private:
    SPIClass* spi_TFT; // use in class TP
  public:
    TFT_SPI(SPIClass& spiInstance, int csPin);
    ~TFT_SPI();
    void setTFTcontroller(uint8_t TFTcontroller);
    void setDiaplayInversion(uint8_t dispInv);
    void begin(uint8_t DC);
    void setFrequency(uint32_t f);
    void setRotation(uint8_t r);

    // Recommended Non-Transaction
    void     readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data);
    uint8_t         getRotation(void) const;
    void            loop();

  private:
    bool        panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void* bitmap) override;
    void        displayInversion();

    enum Ctrl { ILI9341 = 0, ILI9486 = 3, ILI9488 = 4, ST7796 = 5 };
    uint8_t     _TFTcontroller = ILI9341;
    SPISettings SPIset; // SPI settings for this slave

    bool      m_framebuffer_index = 0;

    uint32_t m_freq;
    uint8_t  m_displayInversion;
    uint8_t  _TFT_DC = 21; /* Data or Command */
    uint8_t  _TFT_CS = 22; /* SPI Chip select */
    uint8_t  buf[1024];
    char     chbuf[256 * 2];

    inline int32_t minimum(int32_t a, int32_t b) {
        if (a < b)
            return a;
        else
            return b;
    }

    inline void TFT_DC_HIGH() { gpio_set_level((gpio_num_t)_TFT_DC, 1); }
    inline void TFT_DC_LOW() { gpio_set_level((gpio_num_t)_TFT_DC, 0); }
    inline void TFT_CS_HIGH() { gpio_set_level((gpio_num_t)_TFT_CS, 1); }
    inline void TFT_CS_LOW() { gpio_set_level((gpio_num_t)_TFT_CS, 0); }

    void     init();
    void     writeCommand(uint16_t cmd);
    uint16_t readCommand();

    void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void write24BitColor(uint16_t color);
    void writePixels(uint16_t* colors, uint32_t len);
    void writeColor(uint16_t color, uint32_t len);

    void startWrite(void);
    void endWrite(void);
    void writePixel(int16_t x, int16_t y, uint16_t color);

// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   J P E G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    // —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    //  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   P N G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
    // —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

};
