#include "../../src/settings.h"
#include "tft_base.h"

#include <utility>

#include "fonts/Arial.h"
#include "fonts/BigNumbers.h"
#include "fonts/FreeSerifItalic.h"
#include "fonts/Garamond.h"
#include "fonts/TimesNewRoman.h"
#include "fonts/Z003.h"
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_Base::logicalWidth() const {
    if (m_rotation & 1) return m_v_res;
    return m_h_res;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_Base::logicalHeight() const {
    if (m_rotation & 1) return m_h_res;
    return m_v_res;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_Base::drawBmpFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, float scale) {
    auto bmpRead32 = [](const uint8_t* data, size_t offset) -> uint32_t {
        return data[offset] | (uint16_t)(data[offset + 1]) << 8 | (uint32_t)(data[offset + 2]) << 16 | (uint32_t)(data[offset + 3]) << 24;
    };
    auto bmpRead16 = [](const uint8_t* data, size_t offset) -> uint16_t {
        return data[offset] | (uint16_t)(data[offset + 1]) << 8;
    };
    auto bmpColor16 = [](const uint8_t* pixel) -> uint16_t {
        return ((uint8_t*)pixel)[0] | ((uint16_t)((uint8_t*)pixel)[1]) << 8;
    };
    auto bmpColor24 = [](const uint8_t* pixel) -> uint16_t {
        return ((uint16_t)(((uint8_t*)pixel)[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)pixel)[1] & 0xFC) << 3) | ((((uint8_t*)pixel)[0] & 0xF8) >> 3);
    };
    auto bmpColor32 = [](const uint8_t* pixel) -> uint16_t {
        return ((uint16_t)(((uint8_t*)pixel)[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)pixel)[2] & 0xFC) << 3) | ((((uint8_t*)pixel)[1] & 0xF8) >> 3);
    };

    if (scale <= 0.0f) return false;
    if (!fs.exists(path)) return false;

    File bmp = fs.open(path);
    if (!bmp) return false;

    constexpr size_t headerLen = 54;
    uint8_t          header[headerLen];

    if (bmp.read(header, headerLen) != headerLen) return false;
    if (header[0] != 'B' || header[1] != 'M') return false;

    const uint32_t dataOffset = bmpRead32(header, 0x0A);
    const int32_t  bmpWidthI = bmpRead32(header, 0x12);
    const int32_t  bmpHeightI = bmpRead32(header, 0x16);
    const uint16_t bpp = bmpRead16(header, 0x1C);
    const uint32_t compression = bmpRead32(header, 0x1E);

    if (compression != 0) return false;
    if (!(bpp == 16 || bpp == 24 || bpp == 32)) return false;

    const bool bottomUp = (bmpHeightI > 0);
    const size_t bmpWidth = abs(bmpWidthI);
    const size_t bmpHeight = abs(bmpHeightI);
    const size_t scaledWidth = bmpWidth * scale;
    const size_t scaledHeight = bmpHeight * scale;
    const size_t drawWidth = (maxWidth == 0) ? scaledWidth : std::min((size_t)maxWidth, scaledWidth);
    const size_t drawHeight = (maxHeight == 0) ? scaledHeight : std::min((size_t)maxHeight, scaledHeight);

    size_t dstWidth = (m_rotation & 1) ? drawHeight : drawWidth;
    size_t dstHeight = (m_rotation & 1) ? drawWidth : drawHeight;

    if (x >= logicalWidth() || y >= logicalHeight()) return false;
    if (x + dstWidth > logicalWidth()) dstWidth = logicalWidth() - x;
    if (y + dstHeight > logicalHeight()) dstHeight = logicalHeight() - y;

    const size_t rowSize = ((bmpWidth * bpp / 8 + 3) & ~3);
    if (rowSize > m_rowBufferSize) return false;

    if (!m_rowBuffer) m_rowBuffer = (uint8_t*)ps_malloc(m_rowBufferSize);
    if (!m_rowBuffer) return false;

    uint16_t* pixelBuffer = (uint16_t*)ps_malloc(drawWidth * drawHeight * sizeof(uint16_t));
    if (!pixelBuffer) return false;

    for (size_t dy = 0; dy < drawHeight; ++dy) {
        const size_t srcYScaled = (dy * bmpHeight) / scaledHeight;
        const size_t srcRow = bottomUp ? (bmpHeight - 1 - srcYScaled) : srcYScaled;

        bmp.seek(dataOffset + srcRow * rowSize);
        bmp.read(m_rowBuffer, rowSize);

        for (size_t dx = 0; dx < drawWidth; ++dx) {
            const size_t srcXScaled = (dx * bmpWidth) / scaledWidth;
            const uint8_t* pixelPtr = m_rowBuffer + srcXScaled * (bpp / 8);

            uint16_t color = 0;
            switch (bpp) {
                case 16: color = bmpColor16(pixelPtr); break;
                case 24: color = bmpColor24(pixelPtr); break;
                case 32: color = bmpColor32(pixelPtr); break;
            }

            pixelBuffer[dy * drawWidth + dx] = color;
        }
    }

    renderRGB565(x, y, drawWidth, drawHeight, pixelBuffer, nullptr);
    bmp.close();
    free(pixelBuffer);
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::setFont(uint16_t font) {
    #define SET_FONT_DATA(CMAP, BITMAP, DSC)    \
        do {                                    \
            m_current_font.cmaps = CMAP;        \
            m_current_font.glyph_bitmap = BITMAP; \
            m_current_font.glyph_dsc = DSC;     \
            m_current_font.range_start = CMAP->range_start; \
            m_current_font.range_length = CMAP->range_length; \
            m_current_font.line_height = CMAP->line_height; \
            m_current_font.font_height = CMAP->font_height; \
            m_current_font.base_line = CMAP->base_line; \
            m_current_font.lookup_table = CMAP->lookup_table; \
        } while (0)

    #ifdef TFT_TIMES_NEW_ROMAN
        switch (font) {
            case 15: SET_FONT_DATA(cmaps_Times15, glyph_bitmap_Times15, glyph_dsc_Times15); break;
            case 16: SET_FONT_DATA(cmaps_Times16, glyph_bitmap_Times16, glyph_dsc_Times16); break;
            case 18: SET_FONT_DATA(cmaps_Times18, glyph_bitmap_Times18, glyph_dsc_Times18); break;
            case 21: SET_FONT_DATA(cmaps_Times21, glyph_bitmap_Times21, glyph_dsc_Times21); break;
            case 25:
                SET_FONT_DATA(cmaps_Times25, glyph_bitmap_Times25, glyph_dsc_Times25);
                m_current_font.lookup_table = cmaps_Times15->lookup_table;
                break;
            case 27: SET_FONT_DATA(cmaps_Times27, glyph_bitmap_Times27, glyph_dsc_Times27); break;
            case 34: SET_FONT_DATA(cmaps_Times34, glyph_bitmap_Times34, glyph_dsc_Times34); break;
            case 38: SET_FONT_DATA(cmaps_Times38, glyph_bitmap_Times38, glyph_dsc_Times38); break;
            case 43: SET_FONT_DATA(cmaps_Times43, glyph_bitmap_Times43, glyph_dsc_Times43); break;
            case 56: SET_FONT_DATA(cmaps_Times56, glyph_bitmap_Times56, glyph_dsc_Times56); break;
            case 66: SET_FONT_DATA(cmaps_Times66, glyph_bitmap_Times66, glyph_dsc_Times66); break;
            case 81: SET_FONT_DATA(cmaps_Times81, glyph_bitmap_Times81, glyph_dsc_Times81); break;
            case 96: SET_FONT_DATA(cmaps_Times96, glyph_bitmap_Times96, glyph_dsc_Times96); break;
            case 156: SET_FONT_DATA(cmaps_BigNumbers, glyph_bitmap_BiGNumbers, glyph_dsc_BigNumbers); break;
            default: log_e("unknown font size for Times New Roman, size is %i", font); break;
        }
    #endif

    #ifdef TFT_GARAMOND
        switch (font) {
            case 15: SET_FONT_DATA(cmaps_Garamond15, glyph_bitmap_Garamond15, glyph_dsc_Garamond15); break;
            case 16: SET_FONT_DATA(cmaps_Garamond16, glyph_bitmap_Garamond16, glyph_dsc_Garamond16); break;
            case 18: SET_FONT_DATA(cmaps_Garamond18, glyph_bitmap_Garamond18, glyph_dsc_Garamond18); break;
            case 21: SET_FONT_DATA(cmaps_Garamond21, glyph_bitmap_Garamond21, glyph_dsc_Garamond21); break;
            case 25: SET_FONT_DATA(cmaps_Garamond25, glyph_bitmap_Garamond25, glyph_dsc_Garamond25); break;
            case 27: SET_FONT_DATA(cmaps_Garamond27, glyph_bitmap_Garamond27, glyph_dsc_Garamond27); break;
            case 34: SET_FONT_DATA(cmaps_Garamond34, glyph_bitmap_Garamond34, glyph_dsc_Garamond34); break;
            case 38: SET_FONT_DATA(cmaps_Garamond38, glyph_bitmap_Garamond38, glyph_dsc_Garamond38); break;
            case 43: SET_FONT_DATA(cmaps_Garamond43, glyph_bitmap_Garamond43, glyph_dsc_Garamond43); break;
            case 56: SET_FONT_DATA(cmaps_Garamond56, glyph_bitmap_Garamond56, glyph_dsc_Garamond56); break;
            case 66: SET_FONT_DATA(cmaps_Garamond66, glyph_bitmap_Garamond66, glyph_dsc_Garamond66); break;
            case 81: SET_FONT_DATA(cmaps_Garamond81, glyph_bitmap_Garamond81, glyph_dsc_Garamond81); break;
            case 96: SET_FONT_DATA(cmaps_Garamond96, glyph_bitmap_Garamond96, glyph_dsc_Garamond96); break;
            case 156: SET_FONT_DATA(cmaps_BigNumbers, glyph_bitmap_BiGNumbers, glyph_dsc_BigNumbers); break;
            default: break;
        }
    #endif

    #ifdef TFT_FREE_SERIF_ITALIC
        switch (font) {
            case 15: SET_FONT_DATA(cmaps_FreeSerifItalic15, glyph_bitmap_FreeSerifItalic15, glyph_dsc_FreeSerifItalic15); break;
            case 16: SET_FONT_DATA(cmaps_FreeSerifItalic16, glyph_bitmap_FreeSerifItalic16, glyph_dsc_FreeSerifItalic16); break;
            case 18: SET_FONT_DATA(cmaps_FreeSerifItalic18, glyph_bitmap_FreeSerifItalic18, glyph_dsc_FreeSerifItalic18); break;
            case 21: SET_FONT_DATA(cmaps_FreeSerifItalic21, glyph_bitmap_FreeSerifItalic21, glyph_dsc_FreeSerifItalic21); break;
            case 25: SET_FONT_DATA(cmaps_FreeSerifItalic25, glyph_bitmap_FreeSerifItalic25, glyph_dsc_FreeSerifItalic25); break;
            case 27: SET_FONT_DATA(cmaps_FreeSerifItalic27, glyph_bitmap_FreeSerifItalic27, glyph_dsc_FreeSerifItalic27); break;
            case 34: SET_FONT_DATA(cmaps_FreeSerifItalic34, glyph_bitmap_FreeSerifItalic34, glyph_dsc_FreeSerifItalic34); break;
            case 38: SET_FONT_DATA(cmaps_FreeSerifItalic38, glyph_bitmap_FreeSerifItalic38, glyph_dsc_FreeSerifItalic38); break;
            case 43: SET_FONT_DATA(cmaps_FreeSerifItalic43, glyph_bitmap_FreeSerifItalic43, glyph_dsc_FreeSerifItalic43); break;
            case 56: SET_FONT_DATA(cmaps_FreeSerifItalic56, glyph_bitmap_FreeSerifItalic56, glyph_dsc_FreeSerifItalic56); break;
            case 66: SET_FONT_DATA(cmaps_FreeSerifItalic66, glyph_bitmap_FreeSerifItalic66, glyph_dsc_FreeSerifItalic66); break;
            case 81: SET_FONT_DATA(cmaps_FreeSerifItalic81, glyph_bitmap_FreeSerifItalic81, glyph_dsc_FreeSerifItalic81); break;
            case 96: SET_FONT_DATA(cmaps_FreeSerifItalic96, glyph_bitmap_FreeSerifItalic96, glyph_dsc_FreeSerifItalic96); break;
            case 156: SET_FONT_DATA(cmaps_BigNumbers, glyph_bitmap_BiGNumbers, glyph_dsc_BigNumbers); break;
            default: break;
        }
    #endif

    #ifdef TFT_ARIAL
        switch (font) {
            case 15: SET_FONT_DATA(cmaps_Arial15, glyph_bitmap_Arial15, glyph_dsc_Arial15); break;
            case 16: SET_FONT_DATA(cmaps_Arial16, glyph_bitmap_Arial16, glyph_dsc_Arial16); break;
            case 18: SET_FONT_DATA(cmaps_Arial18, glyph_bitmap_Arial18, glyph_dsc_Arial18); break;
            case 21: SET_FONT_DATA(cmaps_Arial21, glyph_bitmap_Arial21, glyph_dsc_Arial21); break;
            case 25: SET_FONT_DATA(cmaps_Arial25, glyph_bitmap_Arial25, glyph_dsc_Arial25); break;
            case 27: SET_FONT_DATA(cmaps_Arial27, glyph_bitmap_Arial27, glyph_dsc_Arial27); break;
            case 34: SET_FONT_DATA(cmaps_Arial34, glyph_bitmap_Arial34, glyph_dsc_Arial34); break;
            case 38: SET_FONT_DATA(cmaps_Arial38, glyph_bitmap_Arial38, glyph_dsc_Arial38); break;
            case 43: SET_FONT_DATA(cmaps_Arial43, glyph_bitmap_Arial43, glyph_dsc_Arial43); break;
            case 56: SET_FONT_DATA(cmaps_Arial56, glyph_bitmap_Arial56, glyph_dsc_Arial56); break;
            case 66: SET_FONT_DATA(cmaps_Arial66, glyph_bitmap_Arial66, glyph_dsc_Arial66); break;
            case 81: SET_FONT_DATA(cmaps_Arial81, glyph_bitmap_Arial81, glyph_dsc_Arial81); break;
            case 96: SET_FONT_DATA(cmaps_Arial96, glyph_bitmap_Arial96, glyph_dsc_Arial96); break;
            case 156: SET_FONT_DATA(cmaps_BigNumbers, glyph_bitmap_BiGNumbers, glyph_dsc_BigNumbers); break;
            default: break;
        }
    #endif

    #ifdef TFT_Z003
        switch (font) {
            case 15: SET_FONT_DATA(cmaps_Z003_15, glyph_bitmap_Z003_15, glyph_dsc_Z003_15); break;
            case 16: SET_FONT_DATA(cmaps_Z003_16, glyph_bitmap_Z003_16, glyph_dsc_Z003_16); break;
            case 18: SET_FONT_DATA(cmaps_Z003_18, glyph_bitmap_Z003_18, glyph_dsc_Z003_18); break;
            case 21: SET_FONT_DATA(cmaps_Z003_21, glyph_bitmap_Z003_21, glyph_dsc_Z003_21); break;
            case 25: SET_FONT_DATA(cmaps_Z003_25, glyph_bitmap_Z003_25, glyph_dsc_Z003_25); break;
            case 27: SET_FONT_DATA(cmaps_Z003_27, glyph_bitmap_Z003_27, glyph_dsc_Z003_27); break;
            case 34: SET_FONT_DATA(cmaps_Z003_34, glyph_bitmap_Z003_34, glyph_dsc_Z003_34); break;
            case 38: SET_FONT_DATA(cmaps_Z003_38, glyph_bitmap_Z003_38, glyph_dsc_Z003_38); break;
            case 43: SET_FONT_DATA(cmaps_Z003_43, glyph_bitmap_Z003_43, glyph_dsc_Z003_43); break;
            case 56: SET_FONT_DATA(cmaps_Z003_56, glyph_bitmap_Z003_56, glyph_dsc_Z003_56); break;
            case 66: SET_FONT_DATA(cmaps_Z003_66, glyph_bitmap_Z003_66, glyph_dsc_Z003_66); break;
            case 81: SET_FONT_DATA(cmaps_Z003_81, glyph_bitmap_Z003_81, glyph_dsc_Z003_81); break;
            case 96: SET_FONT_DATA(cmaps_Z003_96, glyph_bitmap_Z003_96, glyph_dsc_Z003_96); break;
            case 156: SET_FONT_DATA(cmaps_BigNumbers, glyph_bitmap_BiGNumbers, glyph_dsc_BigNumbers); break;
            default: break;
        }
    #endif

    #undef SET_FONT_DATA
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_Base::renderRGB565(int16_t x, int16_t y, uint16_t w, uint16_t h, const uint16_t* rgb, const uint8_t* alpha) {
    if (!rgb || w == 0 || h == 0) return false;

    int32_t minX = logicalWidth();
    int32_t minY = logicalHeight();
    int32_t maxX = -1;
    int32_t maxY = -1;

    for (uint16_t row = 0; row < h; ++row) {
        for (uint16_t col = 0; col < w; ++col) {
            int32_t srcX = x + col;
            int32_t srcY = y + row;

            int32_t dstX, dstY;
            mapRotation(m_rotation, srcX, srcY, dstX, dstY);

            if (dstX < 0 || dstY < 0) continue;
            if (dstX >= m_h_res || dstY >= m_v_res) continue;

            const size_t fbIndex = dstY * m_h_res + dstX;
            const size_t srcIndex = row * w + col;

            uint16_t newColor = rgb[srcIndex];

            if (alpha) {
                uint8_t a = alpha[srcIndex];

                if (a == 0) continue;

                if (a < 255) {
                    uint16_t old = m_framebuffer[0][fbIndex];
                    uint16_t invA = 255 - a;

                    uint8_t r = ((newColor >> 11) & 0x1F) << 3;
                    uint8_t g = ((newColor >> 5) & 0x3F) << 2;
                    uint8_t b = (newColor & 0x1F) << 3;

                    uint8_t oldR = ((old >> 11) & 0x1F) << 3;
                    uint8_t oldG = ((old >> 5) & 0x3F) << 2;
                    uint8_t oldB = (old & 0x1F) << 3;

                    r = (r * a + oldR * invA + 128) >> 8;
                    g = (g * a + oldG * invA + 128) >> 8;
                    b = (b * a + oldB * invA + 128) >> 8;

                    newColor = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
                }
            }

            m_framebuffer[0][fbIndex] = newColor;

            if (dstX < minX) minX = dstX;
            if (dstY < minY) minY = dstY;
            if (dstX > maxX) maxX = dstX;
            if (dstY > maxY) maxY = dstY;
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }

    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::mapRotation(uint8_t rot, int32_t srcX, int32_t srcY, int32_t& dstX, int32_t& dstY) const {
    switch (rot & 3) {
        case 0:
            dstX = srcX;
            dstY = srcY;
            break;

        case 1:
            dstX = logicalHeight() - 1 - srcY;
            dstY = srcX;
            break;

        case 2:
            dstX = logicalWidth() - 1 - srcX;
            dstY = logicalHeight() - 1 - srcY;
            break;

        case 3:
            dstX = srcY;
            dstY = logicalWidth() - 1 - srcX;
            break;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawRectLogicalFromFB(uint8_t fb, int16_t x, int16_t y, uint16_t w, uint16_t h) {
    int32_t px[4], py[4];
    int32_t sx[4] = {x, x + w - 1, x, x + w - 1};
    int32_t sy[4] = {y, y, y + h - 1, y + h - 1};

    int32_t minX = m_h_res;
    int32_t minY = m_v_res;
    int32_t maxX = -1;
    int32_t maxY = -1;

    for (int i = 0; i < 4; i++) {
        mapRotation(m_rotation, sx[i], sy[i], px[i], py[i]);

        if (px[i] < minX) minX = px[i];
        if (py[i] < minY) minY = py[i];
        if (px[i] > maxX) maxX = px[i];
        if (py[i] > maxY) maxY = py[i];
    }

    panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[fb]);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_Base::copyFramebuffer(uint8_t source, uint8_t destination, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if (w == 0 || h == 0) return false;

    uint16_t lw = logicalWidth();
    uint16_t lh = logicalHeight();

    if (x >= lw || y >= lh) return false;
    if (x + w > lw) w = lw - x;
    if (y + h > lh) h = lh - y;

    for (uint16_t row = 0; row < h; ++row) {
        for (uint16_t col = 0; col < w; ++col) {
            int32_t physX, physY;
            mapRotation(m_rotation, x + col, y + row, physX, physY);

            if (physX < 0 || physY < 0 || physX >= m_h_res || physY >= m_v_res) continue;

            m_framebuffer[destination][physY * m_h_res + physX] = m_framebuffer[source][physY * m_h_res + physX];
        }
    }

    if (destination == 0) drawRectLogicalFromFB(0, x, y, w, h);

    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    if (x >= logicalWidth() || y >= logicalHeight()) return;
    if (x + w < 0 || y + h < 0) return;

    uint16_t lineBuffer[w];
    for (int16_t i = 0; i < w; ++i) lineBuffer[i] = color;

    for (int16_t row = 0; row < h; ++row) {
        renderRGB565(x, y + row, w, 1, lineBuffer, nullptr);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::fillScreen(uint16_t color) {
    fillRect(0, 0, logicalWidth(), logicalHeight(), color);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    while (true) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, x0, y0, rotX, rotY);

        if (rotX >= 0 && rotX < m_h_res && rotY >= 0 && rotY < m_v_res) {
            m_framebuffer[0][rotY * m_h_res + rotX] = color;

            if (rotX < minX) minX = rotX;
            if (rotY < minY) minY = rotY;
            if (rotX > maxX) maxX = rotX;
            if (rotY > maxY) maxY = rotY;
        }

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t y) {
        if (xs > xe) std::swap(xs, xe);

        for (int16_t x = xs; x <= xe; ++x) {
            int32_t rotX, rotY;
            mapRotation(m_rotation, x, y, rotX, rotY);

            if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) continue;

            m_framebuffer[0][rotY * m_h_res + rotX] = color;

            if (rotX < minX) minX = rotX;
            if (rotY < minY) minY = rotY;
            if (rotX > maxX) maxX = rotX;
            if (rotY > maxY) maxY = rotY;
        }
    };

    if (y1 == y2) {
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }
    } else if (y0 == y1) {
        for (int16_t y = y0; y <= y2; ++y) {
            int16_t xa = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            int16_t xb = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            drawSpan(xa, xb, y);
        }
    } else {
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }

        for (int16_t y = y1; y <= y2; ++y) {
            int16_t xa = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int16_t xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawSpan(xa, xb, y);
        }
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (w == 0 || h == 0) return;

    drawLine(x, y, x + w - 1, y, color);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
    drawLine(x + w - 1, y + h - 1, x, y + h - 1, color);
    drawLine(x, y + h - 1, x, y, color);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (w <= 0 || h <= 0 || r <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    drawLine(x + r, y, x + w - r - 1, y, color);
    drawLine(x + r, y + h - 1, x + w - r - 1, y + h - 1, color);
    drawLine(x, y + r, x, y + h - r - 1, color);
    drawLine(x + w - 1, y + r, x + w - 1, y + h - r - 1, color);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx <= cy) {
        auto plot = [&](int16_t px, int16_t py) {
            int32_t rotX, rotY;
            mapRotation(m_rotation, px, py, rotX, rotY);

            if (rotX >= 0 && rotX < m_h_res && rotY >= 0 && rotY < m_v_res) {
                m_framebuffer[0][rotY * m_h_res + rotX] = color;
            }
        };

        plot(x + w - r - 1 + cx, y + r - cy);
        plot(x + w - r - 1 + cy, y + r - cx);
        plot(x + w - r - 1 + cx, y + h - r - 1 + cy);
        plot(x + w - r - 1 + cy, y + h - r - 1 + cx);
        plot(x + r - cx, y + h - r - 1 + cy);
        plot(x + r - cy, y + h - r - 1 + cx);
        plot(x + r - cx, y + r - cy);
        plot(x + r - cy, y + r - cx);

        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }

        cx++;
        ddF_x += 2;
        f += ddF_x;
    }

    int32_t minX = m_h_res;
    int32_t minY = m_v_res;
    int32_t maxX = -1;
    int32_t maxY = -1;

    auto includeCorner = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    includeCorner(x, y);
    includeCorner(x + w - 1, y);
    includeCorner(x, y + h - 1);
    includeCorner(x + w - 1, y + h - 1);

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    if (w <= 0 || h <= 0) return;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t py) {
        if (xs > xe) std::swap(xs, xe);
        for (int16_t px = xs; px <= xe; ++px) plot(px, py);
    };

    for (int16_t py = y + r; py < y + h - r; ++py) drawSpan(x, x + w - 1, py);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t cx = 0;
    int16_t cy = r;

    while (cx <= cy) {
        drawSpan(x + r - cx, x + w - r - 1 + cx, y + r - cy);
        drawSpan(x + r - cy, x + w - r - 1 + cy, y + r - cx);
        drawSpan(x + r - cx, x + w - r - 1 + cx, y + h - r - 1 + cy);
        drawSpan(x + r - cy, x + w - r - 1 + cy, y + h - r - 1 + cx);

        if (f >= 0) {
            cy--;
            ddF_y += 2;
            f += ddF_y;
        }

        cx++;
        ddF_x += 2;
        f += ddF_x;
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    if (r <= 0) return;

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    plot(cx, cy + r);
    plot(cx, cy - r);
    plot(cx + r, cy);
    plot(cx - r, cy);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        plot(cx + x, cy + y);
        plot(cx - x, cy + y);
        plot(cx + x, cy - y);
        plot(cx - x, cy - y);
        plot(cx + y, cy + x);
        plot(cx - y, cy + x);
        plot(cx + y, cy - x);
        plot(cx - y, cy - x);
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::fillCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color) {
    if (r == 0) return;

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    size_t minX = m_h_res;
    size_t minY = m_v_res;
    size_t maxX = 0;
    size_t maxY = 0;

    auto plot = [&](int16_t px, int16_t py) {
        int32_t rotX, rotY;
        mapRotation(m_rotation, px, py, rotX, rotY);

        if (rotX < 0 || rotY < 0 || rotX >= m_h_res || rotY >= m_v_res) return;

        m_framebuffer[0][rotY * m_h_res + rotX] = color;

        if (rotX < minX) minX = rotX;
        if (rotY < minY) minY = rotY;
        if (rotX > maxX) maxX = rotX;
        if (rotY > maxY) maxY = rotY;
    };

    auto drawSpan = [&](int16_t xs, int16_t xe, int16_t py) {
        if (xs > xe) std::swap(xs, xe);
        for (int16_t px = xs; px <= xe; ++px) plot(px, py);
    };

    drawSpan(cx, cx, cy - r);
    drawSpan(cx, cx, cy + r);

    while (x <= y) {
        drawSpan(cx - x, cx + x, cy + y);
        drawSpan(cx - x, cx + x, cy - y);
        drawSpan(cx - y, cx + y, cy + x);
        drawSpan(cx - y, cx + y, cy - x);

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;
    }

    if (maxX >= minX && maxY >= minY) {
        panelDrawBitmap(minX, minY, maxX + 1, maxY + 1, m_framebuffer[0]);
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::writeTheFramebuffer(const uint8_t* bmi, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) {
    if (!bmi || width == 0 || height == 0) return;

    auto bitreader = [&](const uint8_t* bm) {
        static uint16_t       byteIndex = 0;
        static uint8_t        bitMask = 0;
        static const uint8_t* bitmap = nullptr;

        if (bm) {
            bitmap = bm;
            byteIndex = 0;
            bitMask = 0x80;
            return (int32_t)0;
        }

        bool bit = bitmap[byteIndex] & bitMask;

        bitMask >>= 1;
        if (bitMask == 0) {
            bitMask = 0x80;
            byteIndex++;
        }

        return bit ? (int32_t)m_textColor : (int32_t)-1;
    };

    bitreader(bmi);

    uint16_t* rgbBuffer = (uint16_t*)ps_malloc(width * height * sizeof(uint16_t));
    uint8_t* alphaBuffer = (uint8_t*)ps_malloc(width * height);

    if (!rgbBuffer || !alphaBuffer) return;

    for (uint16_t row = 0; row < height; row++) {
        for (uint16_t col = 0; col < width; col++) {
            int32_t color = bitreader(nullptr);
            size_t idx = row * width + col;

            if (color == -1) {
                rgbBuffer[idx] = 0;
                alphaBuffer[idx] = 0;
            }
            else {
                rgbBuffer[idx] = (uint16_t)color;
                alphaBuffer[idx] = 255;
            }
        }
    }

    renderRGB565(posX, posY, width, height, rgbBuffer, alphaBuffer);

    free(rgbBuffer);
    free(alphaBuffer);
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_Base::analyzeText(const char* str, uint16_t* chArr, uint16_t* colorArr, uint16_t startColor) {
    uint16_t chLen = 0;
    uint16_t idx = 0;
    int32_t  codePoint = -1;
    colorArr[0] = startColor;

    while ((uint8_t)str[idx] != 0) {
        colorArr[chLen + 1] = colorArr[chLen];
        switch ((uint8_t)str[idx]) {
            case '\033':
                if (strncmp(str + idx, "\033[30m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_BLACK; break; }
                if (strncmp(str + idx, "\033[31m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_RED; break; }
                if (strncmp(str + idx, "\033[32m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_GREEN; break; }
                if (strncmp(str + idx, "\033[33m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_YELLOW; break; }
                if (strncmp(str + idx, "\033[34m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_BLUE; break; }
                if (strncmp(str + idx, "\033[35m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_MAGENTA; break; }
                if (strncmp(str + idx, "\033[36m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_CYAN; break; }
                if (strncmp(str + idx, "\033[37m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_WHITE; break; }
                if (strncmp(str + idx, "\033[38;5;130m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_BROWN; break; }
                if (strncmp(str + idx, "\033[38;5;214m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_ORANGE; break; }
                if (strncmp(str + idx, "\033[90m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_GREY; break; }
                if (strncmp(str + idx, "\033[91m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTRED; break; }
                if (strncmp(str + idx, "\033[92m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTGREEN; break; }
                if (strncmp(str + idx, "\033[93m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTYELLOW; break; }
                if (strncmp(str + idx, "\033[94m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTBLUE; break; }
                if (strncmp(str + idx, "\033[95m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTMAGENTA; break; }
                if (strncmp(str + idx, "\033[96m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTCYAN; break; }
                if (strncmp(str + idx, "\033[97m", 5) == 0) { idx += 5; colorArr[chLen] = TFT_LIGHTGREY; break; }
                if (strncmp(str + idx, "\033[38;5;52m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_DARKRED; break; }
                if (strncmp(str + idx, "\033[38;5;22m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_DARKGREEN; break; }
                if (strncmp(str + idx, "\033[38;5;136m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_DARKYELLOW; break; }
                if (strncmp(str + idx, "\033[38;5;17m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_DARKBLUE; break; }
                if (strncmp(str + idx, "\033[38;5;53m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_DARKMAGENTA; break; }
                if (strncmp(str + idx, "\033[38;5;23m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_DARKCYAN; break; }
                if (strncmp(str + idx, "\033[38;5;240m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_DARKGREY; break; }
                if (strncmp(str + idx, "\033[38;5;166m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_DARKORANGE; break; }
                if (strncmp(str + idx, "\033[38;5;215m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_LIGHTORANGE; break; }
                if (strncmp(str + idx, "\033[38;5;129m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_PURPLE; break; }
                if (strncmp(str + idx, "\033[38;5;213m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_PINK; break; }
                if (strncmp(str + idx, "\033[38;5;190m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_LIME; break; }
                if (strncmp(str + idx, "\033[38;5;25m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_NAVY; break; }
                if (strncmp(str + idx, "\033[38;5;51m", 10) == 0) { idx += 10; colorArr[chLen] = TFT_AQUAMARINE; break; }
                if (strncmp(str + idx, "\033[38;5;189m", 11) == 0) { idx += 11; colorArr[chLen] = TFT_LAVENDER; break; }
                if (strncmp(str + idx, "\033[38;2;210;180;140m", 19) == 0) { idx += 19; colorArr[chLen] = TFT_LIGHTBROWN; break; }
                if (strncmp(str + idx, "\033[0m", 4) == 0) { idx += 4; break; }
                if (strncmp(str + idx, "\033[1m", 4) == 0) { idx += 4; break; }
                if (strncmp(str + idx, "\033[2m", 4) == 0) { idx += 4; break; }
                if (strncmp(str + idx, "\033[3m", 4) == 0) { idx += 4; break; }
                if (strncmp(str + idx, "\033[4m", 4) == 0) { idx += 4; break; }
                if (tft_info) tft_info("unknown ANSI ESC SEQUENCE");
                idx += 4;
                break;

            case 0x20 ... 0x7F:
                chArr[chLen] = (uint8_t)str[idx];
                idx += 1;
                chLen += 1;
                break;
            case 0xC2 ... 0xD1:
                codePoint = ((uint8_t)str[idx] - 0xC2) * 0x40 + (uint8_t)str[idx + 1];
                if (m_current_font.lookup_table[codePoint] != 0) {
                    chArr[chLen] = codePoint;
                    chLen += 1;
                }
                else {
                    log_w("character 0x%02X%02X is not in table", str[idx], str[idx + 1]);
                }
                idx += 2;
                break;
            case 0xD2 ... 0xDF:
                log_w("character 0x%02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 2;
                break;
            case 0xE0:
                if ((uint8_t)str[idx + 1] == 0x80 && (uint8_t)str[idx + 2] == 0x99) {
                    codePoint = 0xA4;
                    chArr[chLen] = codePoint;
                    chLen += 1;
                }
                else {
                    log_w("character 0x%02X%02X  is not in table", str[idx], str[idx + 1]);
                }
                idx += 3;
                break;
            case 0xE1 ... 0xEF:
                idx += 3;
                break;
            case 0xF0 ... 0xFF:
                codePoint = -1;
                if (!strncmp(str + idx, "🟢", 4)) { codePoint = 0xF9A2; }
                if (!strncmp(str + idx, "🟡", 4)) { codePoint = 0xF9A1; }
                if (!strncmp(str + idx, "🔴", 4)) { codePoint = 0xF9B4; }
                if (!strncmp(str + idx, "🔵", 4)) { codePoint = 0xF9B5; }
                if (!strncmp(str + idx, "🟠", 4)) { codePoint = 0xF9A0; }
                if (!strncmp(str + idx, "🟣", 4)) { codePoint = 0xF9A3; }
                if (!strncmp(str + idx, "🟤", 4)) { codePoint = 0xF9A4; }
                if (!strncmp(str + idx, "🟩", 4)) { codePoint = 0xF9A9; }
                if (!strncmp(str + idx, "🟨", 4)) { codePoint = 0xF9A8; }
                if (!strncmp(str + idx, "🟥", 4)) { codePoint = 0xF9A5; }
                if (!strncmp(str + idx, "🟦", 4)) { codePoint = 0xF9A6; }
                if (!strncmp(str + idx, "🟧", 4)) { codePoint = 0xF9A7; }
                if (!strncmp(str + idx, "🟪", 4)) { codePoint = 0xF9AA; }
                if (!strncmp(str + idx, "🟫", 4)) { codePoint = 0xF9AB; }
                if (codePoint != -1) {
                    chArr[chLen] = codePoint;
                    chLen += 1;
                }
                else {
                    log_w("character 0x%02X%02X%02X%02X  is not in table", str[idx], str[idx + 1], str[idx + 2], str[idx + 3]);
                }
                idx += 4;
                break;
            default:
                idx++;
                break;
        }
    }
    return chLen;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_Base::getLineLength(const char* txt, bool narrow) {
    uint16_t pxLength = 0;
    uint16_t idx = 0;
    bool     isEmoji = false;
    while ((uint8_t)txt[idx] != 0) {
        isEmoji = false;
        if (txt[idx] == 0xF0) {
            if (!strncmp(txt + idx, "🟢", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟡", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🔴", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🔵", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟠", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟣", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟤", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟩", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟨", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟥", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟦", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟧", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟪", 4)) { isEmoji = true; }
            if (!strncmp(txt + idx, "🟫", 4)) { isEmoji = true; }
            if (isEmoji) {
                uint16_t fh = m_current_font.font_height;
                pxLength += fh - fh / 3;
                idx += 4;
                continue;
            }
        }
        uint16_t glyphPos = m_current_font.lookup_table[(uint8_t)txt[idx]];
        pxLength += m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        int ofsX = m_current_font.glyph_dsc[glyphPos].ofs_x;
        if (ofsX < 0) ofsX = 0;
        if (!narrow) pxLength += ofsX;
        idx++;
    }
    return pxLength;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_Base::fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap) {
    (void)chLength;
    uint16_t idx = begin;
    uint16_t pxLength = 0;
    uint16_t lastSpacePos = 0;
    uint16_t drawableChars = 0;
    uint16_t lastUsedPxLength = 0;
    uint16_t glyphPos = 0;
    bool     isEmoji = false;
    while (cpArr[idx] != 0) {
        *usedPxLength = pxLength;
        if (cpArr[idx] == 0x20 || cpArr[idx - 1] == '-') {
            lastSpacePos = drawableChars;
            lastUsedPxLength = pxLength;
        }
        isEmoji = false;
        if ((cpArr[idx] & 0xFF00) == 0xF900) {
            if (cpArr[idx] == 0xF9A2) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A1) { isEmoji = true; }
            if (cpArr[idx] == 0xF9B4) { isEmoji = true; }
            if (cpArr[idx] == 0xF9B5) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A0) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A3) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A4) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A9) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A8) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A5) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A6) { isEmoji = true; }
            if (cpArr[idx] == 0xF9A7) { isEmoji = true; }
            if (cpArr[idx] == 0xF9AA) { isEmoji = true; }
            if (cpArr[idx] == 0xF9AB) { isEmoji = true; }
            if (isEmoji) {
                uint16_t fh = m_current_font.font_height;
                pxLength += fh - fh / 3;
            }
        }
        else {
            glyphPos = m_current_font.lookup_table[cpArr[idx]];
            pxLength += m_current_font.glyph_dsc[glyphPos].adv_w / 16;
            int ofsX = m_current_font.glyph_dsc[glyphPos].ofs_x;
            if (ofsX < 0) ofsX = 0;
            if (!narrow) pxLength += ofsX;
        }
        if (pxLength > win_W || cpArr[idx] == '\n') {
            if (noWrap) return drawableChars;
            if (lastSpacePos) {
                *usedPxLength = lastUsedPxLength;
                return lastSpacePos;
            }
            return drawableChars;
        }
        idx++;
        drawableChars++;
        *usedPxLength = pxLength;
    }
    return drawableChars;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_Base::fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap) {
    uint8_t  nrOfFonts = sizeof(fontSizes);
    uint8_t  currentFontSize = 0;
    uint16_t usedPxLength = 0;
    uint16_t drawableCharsTotal = 0;
    uint16_t drawableCharsinline = 0;
    uint16_t startPos = 0;
    uint8_t  nrOfLines = 0;
    while (true) {
        currentFontSize = fontSizes[nrOfFonts - 1];
        if (currentFontSize == 0) break;
        setFont(currentFontSize);
        drawableCharsTotal = 0;
        startPos = 0;
        nrOfLines = 1;
        int16_t win_H_remain = win_H;
        while (true) {
            if (win_H_remain < m_current_font.line_height) break;
            drawableCharsinline = fitinline(cpArr, chLength, startPos, win_W, &usedPxLength, narrow, noWrap);
            win_H_remain -= m_current_font.line_height;
            drawableCharsTotal += drawableCharsinline;
            startPos += drawableCharsinline;
            if (drawableCharsinline == 0) break;
            if (drawableCharsTotal == chLength) goto exit;
            nrOfLines++;
        }
        if (drawableCharsTotal == chLength) goto exit;
        if (nrOfFonts == 0) break;
        nrOfFonts--;
    }
exit:
    return nrOfLines;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
size_t TFT_Base::writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align, uint8_t v_align, bool narrow, bool noWrap, bool autoSize) {
    uint16_t idx = 0;
    uint16_t utfPosArr[strlen(str) + 1] = {0};
    uint16_t colorArr[strlen(str) + 1] = {0};
    uint16_t strChLength = 0;
    uint8_t  nrOfLines = 1;
    bool     isEmoji = false;

    auto drawEmoji = [&](uint16_t idxLocal, uint16_t x, uint16_t y) {
        uint8_t  emoji = (utfPosArr[idxLocal] & 0x00FF);
        uint16_t color = 0;
        char     shape = 'x';
        switch (emoji) {
            case 0xA2: color = TFT_GREEN; shape = 'c'; break;
            case 0xA1: color = TFT_YELLOW; shape = 'c'; break;
            case 0xB4: color = TFT_RED; shape = 'c'; break;
            case 0xB5: color = TFT_BLUE; shape = 'c'; break;
            case 0xA0: color = TFT_ORANGE; shape = 'c'; break;
            case 0xA3: color = TFT_VIOLET; shape = 'c'; break;
            case 0xA4: color = TFT_BROWN; shape = 'c'; break;
            case 0xA9: color = TFT_GREEN; shape = 's'; break;
            case 0xA8: color = TFT_YELLOW; shape = 's'; break;
            case 0xA5: color = TFT_RED; shape = 's'; break;
            case 0xA6: color = TFT_BLUE; shape = 's'; break;
            case 0xA7: color = TFT_ORANGE; shape = 's'; break;
            case 0xAA: color = TFT_VIOLET; shape = 's'; break;
            case 0xAB: color = TFT_BROWN; shape = 's'; break;
        }
        if (shape == 'c') {
            uint16_t fh = m_current_font.font_height;
            uint16_t fw = fh - fh / 3;
            uint16_t p = fh / 5;
            uint16_t r = (fh - 2 * p) / 2;
            uint16_t corr = fw / 10;
            uint16_t cx = x + fw / 2;
            uint16_t cy = y + fh / 2 + corr;
            fillCircle(cx, cy, r, color);
            return fw;
        }
        if (shape == 's') {
            uint16_t fh = m_current_font.font_height;
            uint16_t fw = fh - fh / 3;
            uint16_t p = fh / 5;
            uint16_t a = (fh - 2 * p);
            uint16_t corr = fw / 10;
            uint16_t sx = x + fw / 2;
            uint16_t sy = y + fh / 2 + corr;
            fillRect(sx - a / 2, sy - a / 2, a, a, color);
            return fw;
        }
        log_w("unknown shape %c", shape);
        return (uint16_t)0;
    };

    auto drawChar = [&](uint16_t idxLocal, uint16_t x, uint16_t y) {
        uint16_t glyphPos = m_current_font.lookup_table[utfPosArr[idxLocal]];
        uint16_t adv_w = m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        uint32_t bitmap_index = m_current_font.glyph_dsc[glyphPos].bitmap_index;
        uint16_t box_w = m_current_font.glyph_dsc[glyphPos].box_w;
        uint16_t box_h = m_current_font.glyph_dsc[glyphPos].box_h;
        int16_t  ofs_x = m_current_font.glyph_dsc[glyphPos].ofs_x;
        int16_t  ofs_y = m_current_font.glyph_dsc[glyphPos].ofs_y;
        if (ofs_x < 0) ofs_x = 0;
        x += ofs_x;
        y = y + (m_current_font.line_height - m_current_font.base_line - 1) - box_h - ofs_y;
        writeTheFramebuffer(m_current_font.glyph_bitmap + bitmap_index, x, y, box_w, box_h);
        if (!narrow) adv_w += ofs_x;
        return adv_w;
    };

    strChLength = analyzeText(str, utfPosArr, colorArr, m_textColor);
    if (autoSize) nrOfLines = fitInAddrWindow(utfPosArr, strChLength, win_W, win_H, narrow, noWrap);
    if (!strChLength) return 0;

    if ((win_X + win_W) > logicalWidth()) win_W = logicalWidth() - win_X;
    if ((win_Y + win_H) > logicalHeight()) win_H = logicalHeight() - win_Y;

    uint16_t pX = win_X;
    uint16_t pY = win_Y;
    int16_t  pH = win_H;
    int16_t  pW = win_W;

    if (v_align == TFT_ALIGN_CENTER) {
        int offset = (win_H - (nrOfLines * m_current_font.line_height)) / 2;
        pY = pY + offset;
    }
    if (v_align == TFT_ALIGN_DOWN) {
        int offset = (win_H - (nrOfLines * m_current_font.line_height));
        pY = pY + offset;
    }

    uint16_t charsToDraw = 0;
    uint16_t usedPxLength = 0;
    uint16_t charsDrawn = 0;
    while (true) {
        if (noWrap && idx) goto exit;
        if (pH < m_current_font.line_height) goto exit;
        charsToDraw = fitinline(utfPosArr, strChLength, idx, pW, &usedPxLength, narrow, noWrap);

        if (h_align == TFT_ALIGN_RIGHT) pX += win_W - usedPxLength - 2;
        if (h_align == TFT_ALIGN_CENTER) pX += (win_W - usedPxLength) / 2;
        uint16_t cnt = 0;
        while (true) {
            isEmoji = false;
            setTextColor(colorArr[idx]);
            if ((utfPosArr[idx] & 0xFF00) == 0xF900) {
                if (utfPosArr[idx] == 0xF9A2) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A1) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9B4) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9B5) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A0) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A3) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A4) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A9) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A8) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A5) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A6) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9A7) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9AA) { isEmoji = true; }
                if (utfPosArr[idx] == 0xF9AB) { isEmoji = true; }
            }
            uint16_t res = isEmoji ? drawEmoji(idx, pX, pY) : drawChar(idx, pX, pY);
            pX += res;
            pW -= res;
            idx++;
            cnt++;
            charsDrawn++;
            if (idx == strChLength) goto exit;
            if (cnt == charsToDraw) break;
        }
        pH -= m_current_font.line_height;
        pY += m_current_font.line_height;
        pX = win_X;
        pW = win_W;
    }
exit:
    afterTextDraw(win_X, win_Y, win_W, win_H);
    return charsDrawn;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::afterTextDraw(uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H) {
    (void)win_X;
    (void)win_Y;
    (void)win_W;
    (void)win_H;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
