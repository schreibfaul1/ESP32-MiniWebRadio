#pragma once

#include "Arduino.h"
#include "fonts/fontsdef.h"
#include "tft_common_defs.h"

class TFT_Base {
  public:
    virtual ~TFT_Base() = default;

    uint16_t logicalWidth() const;
    uint16_t logicalHeight() const;

    void drawRectLogicalFromFB(uint8_t fb, int16_t x, int16_t y, uint16_t w, uint16_t h);
    bool copyFramebuffer(uint8_t source, uint8_t destination, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void fillCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color);
    void setBackGoundColor(uint16_t BGcolor) { m_backGroundColor = BGcolor; }
    uint16_t getBackGroundColor() { return m_backGroundColor; }
    void setTextColor(uint16_t FGcolor) { m_textColor = FGcolor; }
    uint16_t getTextColor() { return m_textColor; }
    void setFont(uint16_t font);
    void setTextOrientation(uint16_t orientation = 0) { m_textorientation = orientation; }
    size_t writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align = TFT_ALIGN_LEFT, uint8_t v_align = TFT_ALIGN_CENTER, bool narrow = false,
                     bool noWrap = false, bool autoSize = false);
    uint16_t getLineLength(const char* txt, bool narrow);

  protected:
    typedef struct {
        const uint8_t*                     glyph_bitmap;
        const lv_font_fmt_txt_glyph_dsc_t* glyph_dsc;
        const lv_font_fmt_txt_cmap_t*      cmaps;
        uint32_t                           range_start;
        uint16_t                           range_length;
        uint16_t                           line_height;
        uint16_t                           font_height;
        uint16_t                           base_line;
        uint16_t*                          lookup_table;
    } fonts_t;

    bool renderRGB565(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* rgb, const uint8_t* alpha);
    void mapRotation(uint8_t rot, int32_t srcX, int32_t srcY, int32_t& dstX, int32_t& dstY) const;
    void writeTheFramebuffer(const uint8_t* bmi, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height);
    uint16_t analyzeText(const char* str, uint16_t* chArr, uint16_t* colorArr, uint16_t startColor);
    uint16_t fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap);
    uint8_t fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap);
    virtual void afterTextDraw(uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H);

    virtual bool panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void* bitmap) = 0;

    uint16_t  m_h_res = 0;
    uint16_t  m_v_res = 0;
    uint16_t* m_framebuffer[3] = {nullptr, nullptr, nullptr};
    uint8_t   m_rotation = 0;
    uint8_t   fontSizes[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};
    fonts_t   m_current_font = {};
    uint16_t  m_backGroundColor = TFT_WHITE;
    uint16_t  m_textColor = TFT_BLACK;
    uint8_t   m_textorientation = 0;
};
