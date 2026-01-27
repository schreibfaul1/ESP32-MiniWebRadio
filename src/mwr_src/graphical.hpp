#include "../common.h"

#pragma once

/*  ╔═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
    ║                                                     G R A P H I C   O B J E C T S                                                         ║
    ╚═══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

extern __attribute__((weak)) void graphicObjects_OnChange(ps_ptr<char> name, int32_t val);
extern __attribute__((weak)) void graphicObjects_OnClick(ps_ptr<char> name, uint8_t val);
extern __attribute__((weak)) void graphicObjects_OnRelease(ps_ptr<char> name, releasedArg ra);

extern SemaphoreHandle_t mutex_display;
extern SD_content        s_SD_content;

struct imgSize {
    uint16_t w = 0;
    uint16_t h = 0;
};

imgSize GetImageSize(ps_ptr<char> picturePath) {
    if(picturePath.strlen() == 0) {
        MWR_LOG_DEBUG("picturePath is empty");
        return imgSize{0, 0};
    }
    imgSize img = {0};
    auto scaledPicPath = scaleImage(picturePath);
    if (!SD_MMC.exists(scaledPicPath.c_get())) { /* log_w("file %s not exists, objName: %s", scaledPicPath, m_name)*/
        MWR_LOG_ERROR("cannot open file '%s'", scaledPicPath.c_get());
        return img;
    }
    File file = SD_MMC.open(scaledPicPath.c_get(), "r", false);
    if (file.size() < 24) {
        MWR_LOG_WARN("file '%s' is too small", scaledPicPath.c_get());
        file.close();
        return img;
    }
    char buf[8];
    file.readBytes(buf, 3);
    if ((buf[0] == 0xFF) && (buf[1] == 0xD8) && (buf[2] == 0xFF)) { // format jpeg
        int16_t c1, c2;
        while (true) {
            c1 = file.read();
            if (c1 == -1) {
                MWR_LOG_WARN("sof marker in %s not found", scaledPicPath.c_get());
                file.close();
                return img;
            } // end of file reached
            if (c1 == 0xFF) {
                c2 = file.read();
                if (c2 == 0xC0) break;
            } // 0xFFC0 Marker found
        }
        file.readBytes(buf, 7);
        img.h = buf[3] * 256 + buf[4];
        img.w = buf[5] * 256 + buf[6];
        //    log_i("w %i, h %i", m_w, m_h);
        return img;
    }
    if ((buf[0] == 'B') && (buf[1] == 'M') && (buf[2] == '6')) { // format bmp
        for (int i = 0; i < 15; i++) file.read();                // read 15 dummys
        img.w = file.read();                                 // pos 18
        img.w += (file.read() << 8);
        img.w += (file.read() << 16);
        img.w += (file.read() << 24);
        img.h = file.read(); // pos 22
        img.h += (file.read() << 8);
        img.h += (file.read() << 16);
        img.h += (file.read() << 24);
        //    log_i("w %i, h %i", m_w, m_h);
        return img;
    }
    if ((buf[0] == 'G') && (buf[1] == 'I') && (buf[2] == 'F')) { // format gif
        for (int i = 0; i < 3; i++) file.read();                 // read 3 dummys
        img.w = file.read();                                 // pos 6
        img.w += (file.read() << 8);
        img.h = file.read(); // pos 8
        img.h += (file.read() << 8);
        //    log_i("w %i, h %i", m_w, m_h);
        return img;
    }
    if ((buf[0] == 0x89) && (buf[1] == 'P') && (buf[2] == 'N')) { // format png
        for (int i = 0; i < 13; i++) file.read();                 // read 13 dummys
        img.w = file.read() << 24;                            // pos 16
        img.w += file.read() << 16;                           // pos 17
        img.w += file.read() << 8;                            // pos 18
        img.w += file.read();                                 // pos 19
        img.h = file.read() << 24;                            // pos 20
        img.h += file.read() << 16;                           // pos 21
        img.h += file.read() << 8;                            // pos 22
        img.h += file.read();                                 // pos 23
        // bitDepth  = header[24];  // Position 24 = Bit-Tiefe
        // colorType = header[25];  // Position 25 = Farbtyp
        // log_w("w %i, h %i", m_w, m_h);
        return img;
    }
    MWR_LOG_ERROR("unknown picture format %s", picturePath);
    return img;
}


class slider : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_val = 0;
    int16_t      m_minVal = 0;
    int16_t      m_maxVal = 0;
    uint16_t     m_leftStop = 0;
    uint16_t     m_rightStop = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_railColor = 0;
    uint32_t     m_spotColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_show = false;
    bool         m_objectInit = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    uint8_t      m_railHigh = 0;
    uint16_t     m_middle_h = 0;
    uint16_t     m_spotPos = 0;
    uint8_t      m_spotRadius = 0;
    uint8_t      m_padding_left = 0;
    uint8_t      m_padding_right = 0;
    uint8_t      m_padding_top = 0;
    uint8_t      m_padding_bottom = 0;
    ps_ptr<char> m_name;
    releasedArg  m_ra;

  public:
    slider(ps_ptr<char> name) {
        register_object(this);
        m_name = name;
        m_railHigh = 6;
        m_spotRadius = 12;
        m_bgColor = TFT_BLACK;
        m_railColor = TFT_BEIGE;
        m_spotColor = TFT_RED;
    }
    ~slider() { m_objectInit = false; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_padding_right = paddig_right;
        m_padding_top = paddig_top;                                                     // unused
        m_padding_bottom = paddig_bottom;                                               // unused
        m_leftStop = m_x + m_padding_left + m_spotRadius + 10;                          // x pos left stop
        m_rightStop = m_x + m_w - m_padding_right - m_spotRadius - 10; // x pos right stop
        m_enabled = false;
        m_middle_h = m_y + (m_h / 2);
        m_spotPos = (m_leftStop + m_rightStop) / 2; // in the middle
        m_objectInit = true;
        m_show = false;
    }
    void setMinMaxVal(int16_t minVal, int16_t maxVal) {
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    bool         positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;

        if (m_enabled) {
            if (x < m_leftStop) x = m_leftStop; // (x, y) is in range
            if (x > m_rightStop) x = m_rightStop;
            m_clicked = true;
            drawNewSpot(x);
        }
        if (!m_clicked) {
            if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        }
        if (!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val) {
        if (!m_objectInit) return;
        if (val < m_minVal) val = m_minVal;
        if (val > m_maxVal) val = m_maxVal;
        m_val = val;
        if (m_clicked) return;
        uint16_t spotPos = map_l(val, m_minVal, m_maxVal, m_leftStop, m_rightStop); // val -> x
        if (m_enabled)
            drawNewSpot(spotPos);
        else
            m_spotPos = spotPos;
    }
    int16_t getValue() { return m_val; }

    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        int x = m_x + m_padding_left;
        int y = m_middle_h - (m_railHigh / 2);
        int w = m_w - m_padding_left - m_padding_right;
        int h = m_railHigh;
        (void)h;
        int r = 2;
        if (m_backgroundTransparency) {
            if (m_saveBackground){
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            } else {
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            }
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        tft.fillRoundRect(x, y, w, m_railHigh, r, m_railColor);
        drawNewSpot(m_spotPos);
        m_show = true;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void hide() {
        if(!m_show) return;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
        m_show = false;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }

  private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if (run == 0) {
            MWR_LOG_ERROR("map(): Invalid input range, %li == %li (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewSpot(uint16_t xPos) {
        if (m_enabled) {
            if (m_backgroundTransparency) {
                if (m_saveBackground) {
                    tft.copyFramebuffer(2, 0, m_spotPos - m_spotRadius - 1, m_middle_h - m_spotRadius - 1, 2 * m_spotRadius + 2, 2 * m_spotRadius + 2);
                } else {
                    tft.copyFramebuffer(1, 0, m_spotPos - m_spotRadius - 1, m_middle_h - m_spotRadius - 1, 2 * m_spotRadius + 2, 2 * m_spotRadius + 2);
                }
            } else {
                tft.fillRect(m_spotPos - m_spotRadius, m_middle_h - m_spotRadius, 2 * m_spotRadius, 2 * m_spotRadius + 1, m_bgColor);
            }
            tft.fillRect(m_spotPos - m_spotRadius - 1, m_middle_h - (m_railHigh / 2), 2 * m_spotRadius + 2, m_railHigh, m_railColor);
            tft.fillCircle(xPos, m_middle_h, m_spotRadius, m_spotColor);
        }
        m_spotPos = xPos;
        int32_t val = map_l(m_spotPos, m_leftStop, m_rightStop, m_minVal, m_maxVal); // xPos -> val
        m_ra.val1 = val;
        if (graphicObjects_OnChange) graphicObjects_OnChange(m_name, val);
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class progressbar : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_val = 0;
    int16_t      m_minVal = 0;
    int16_t      m_maxVal = 0;
    int16_t      m_oldPos = 0;
    uint16_t     m_padding_left = 0;
    uint16_t     m_padding_right = 0;
    uint16_t     m_padding_top = 0;
    uint16_t     m_padding_bottom = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_frameColor = 0;
    uint32_t     m_railColorLeft = 0;
    uint32_t     m_railColorRight = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_objectInit = false;
    bool         m_backgroundTransparency = true;
    bool         m_saveBackground = false;
    uint8_t      m_railHigh = 0;
    ps_ptr<char> m_name;
    releasedArg  m_ra;

  public:
    progressbar(ps_ptr<char> name) {
        register_object(this);
        m_name = name;
        m_railHigh = 6;
        m_bgColor = TFT_BLACK;
        m_frameColor = TFT_WHITE;
        m_railColorLeft = TFT_RED;
        m_railColorRight = TFT_GREEN;
    }
    ~progressbar() { m_objectInit = false; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t padding_left, uint16_t padding_right, uint16_t padding_top, uint16_t padding_bottom, int16_t minVal, int16_t maxVal) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = padding_left;
        m_padding_right = padding_right;
        m_padding_top = padding_top;
        m_padding_bottom = padding_bottom;
        m_minVal = minVal;
        m_maxVal = maxVal;
        m_enabled = false;
        m_objectInit = true;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    bool         positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        m_ra.val1 = map_l(x, m_x + 1, m_x + m_w - 2, m_minVal, m_maxVal);
        m_ra.val2 = m_ra.val1 - m_val; // offset
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    void setValue(int16_t val) {
        if (!m_objectInit) return;
        if (val < m_minVal) val = m_minVal;
        if (val > m_maxVal) val = m_maxVal;
        m_val = val;
        if (m_clicked) return;
        if (m_enabled) drawChanges();
    }
    int16_t getValue() { return m_val; }
    void    setNewMinMaxVal(int16_t minVal, int16_t maxVal) {
        m_minVal = minVal;
        m_maxVal = maxVal;
    }
    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        tft.drawRect(m_x + m_padding_left, m_y, m_w - m_padding_left - m_padding_right, m_h, m_frameColor);
        drawNewValue();
        m_enabled = true;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground) {
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            }
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
    void reset() {
        m_val = m_minVal;
        tft.fillRect(m_x, m_y + 1, m_w - m_h - 1, m_h - 2, m_railColorRight);
    }

  private:
    int32_t map_l(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
        const int32_t run = in_max - in_min;
        if (run == 0) {
            MWR_LOG_WARN("map(): Invalid input range, %li == %li (min == max) in %s", in_min, in_max, m_name);
            return -1;
        }
        const int32_t rise = out_max - out_min;
        const int32_t delta = x - in_min;
        return round((float)(delta * rise) / run + out_min);
    }
    void drawNewValue() {
        int      x = m_x + 1 + m_padding_left;
        int      w = m_w - 1 - m_padding_right;
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, x, x + w);
        tft.fillRect(x, m_y + 1, pos, m_h - 2, m_railColorLeft);
        tft.fillRect(pos, m_y + 1, w - pos, m_h - 2, m_railColorRight);
        m_oldPos = pos;
        if (graphicObjects_OnChange) graphicObjects_OnChange(m_name, m_val);
    }
    void drawChanges() {
        int      x = m_x + 1 + m_padding_left;
        int      w = m_w - 1 - m_padding_right;
        uint16_t pos = map_l(m_val, m_minVal, m_maxVal, x, x + w);
        if (pos > m_oldPos) { tft.fillRect(m_oldPos, m_y + 1, pos - m_oldPos, m_h - 2, m_railColorLeft); }
        if (pos < m_oldPos) { tft.fillRect(pos, m_y + 1, m_oldPos - pos, m_h - 2, m_railColorRight); }
        m_oldPos = pos;
        if (graphicObjects_OnChange) graphicObjects_OnChange(m_name, m_val);
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbox : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_h_align = TFT_ALIGN_RIGHT;
    uint8_t      m_v_align = TFT_ALIGN_TOP;
    uint8_t      m_padding_left = 0;  // left margin
    uint8_t      m_paddig_right = 0;  // right margin
    uint8_t      m_paddig_top = 0;    // top margin
    uint8_t      m_paddig_bottom = 0; // bottom margin
    uint8_t      m_borderWidth = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_borderColor = 0;
    ps_ptr<char> m_text;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_autoSize = false;
    bool         m_narrow = false;
    bool         m_noWrap = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    releasedArg  m_ra;

  public:
    textbox(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~textbox() {}
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        writeText(m_text.c_get());
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = m_padding_left + m_borderWidth;
        m_paddig_right = m_paddig_right + m_borderWidth;
        m_paddig_top = m_paddig_top + m_borderWidth;
        m_paddig_bottom = m_paddig_bottom + m_borderWidth;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
    void setText(ps_ptr<char> txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        m_text = txt;
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    void setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }

    void writeText(ps_ptr<char> txt) {
        m_text = txt;
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            tft.setTextColor(m_fgColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (m_borderWidth > 0) { tft.drawRect(m_x, m_y, m_w, m_h, m_borderColor); }
            if (m_borderWidth > 1) { tft.drawRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_borderColor); }
            tft.writeText(m_text.c_get(), x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, m_autoSize);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class inputbox : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_h_align = TFT_ALIGN_RIGHT;
    uint8_t      m_v_align = TFT_ALIGN_TOP;
    uint8_t      m_padding_left = 0;  // left margin
    uint8_t      m_paddig_right = 0;  // right margin
    uint8_t      m_paddig_top = 0;    // top margin
    uint8_t      m_paddig_bottom = 0; // bottom margin
    uint8_t      m_borderWidth = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_borderColor = 0;
    ps_ptr<char> m_text;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_autoSize = false;
    bool         m_narrow = false;
    bool         m_noWrap = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    releasedArg  m_ra;

  public:
    inputbox(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~inputbox() { }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        writeText(m_text);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = m_padding_left + m_borderWidth;
        m_paddig_right = m_paddig_right + m_borderWidth;
        m_paddig_top = m_paddig_top + m_borderWidth;
        m_paddig_bottom = m_paddig_bottom + m_borderWidth;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
    void setText(ps_ptr<char> txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        m_text = txt;
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    void setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }
    void writeText(ps_ptr<char> txt) {
        m_text = txt;
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            tft.setTextColor(m_fgColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (m_borderWidth > 0) { tft.drawRect(m_x, m_y, m_w, m_h, m_borderColor); }
            if (m_borderWidth > 1) { tft.drawRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_borderColor); }

            uint16_t lineLength = 0;
            uint16_t txtMaxWidth = w - 2 * h;
            uint16_t idx = 0;
            lineLength = tft.getLineLength(m_text.c_get(), m_narrow);
            while (lineLength > txtMaxWidth) {
                lineLength = tft.getLineLength(m_text.get() + idx, m_narrow);
                if (lineLength > txtMaxWidth) {
                    idx++;
                    if (idx > m_text.strlen()) break;
                }
            }
            tft.writeText(m_text.get() + idx, x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, false);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class textbutton : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_r = 0; // radius round rect
    uint8_t      m_fontSize = 0;
    uint8_t      m_h_align = TFT_ALIGN_RIGHT;
    uint8_t      m_v_align = TFT_ALIGN_TOP;
    uint8_t      m_padding_left = 0;  // left margin
    uint8_t      m_paddig_right = 0;  // right margin
    uint8_t      m_paddig_top = 0;    // top margin
    uint8_t      m_paddig_bottom = 0; // bottom margin
    uint8_t      m_borderWidth = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_borderColor = 0;
    uint32_t     m_clickColor = 0;
    char*        m_text = NULL;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_autoSize = false;
    bool         m_narrow = false;
    bool         m_noWrap = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    releasedArg  m_ra;

  public:
    textbutton(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        m_fontSize = 1;
    }
    ~textbutton() { x_ps_free(&m_text); }
    void drawTriangeUp() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_h - m_paddig_bottom;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = x0 + (x1 - x0) / 2;
        int16_t  y2 = m_y + m_paddig_top;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeDown() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_paddig_top;
        int16_t  x2 = x0 + (x1 - x0) / 2;
        int16_t  y2 = m_y + m_h - m_paddig_bottom;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeLeft() {
        int16_t  x0 = m_x + m_w - m_paddig_right;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_w - m_paddig_right;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = m_x + m_padding_left;
        int16_t  y2 = y0 + (y1 - y0) / 2;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void drawTriangeRight() {
        int16_t  x0 = m_x + m_padding_left;
        int16_t  y0 = m_y + m_paddig_top;
        int16_t  x1 = m_x + m_padding_left;
        int16_t  y1 = m_y + m_h - m_paddig_bottom;
        int16_t  x2 = m_x + m_w - m_paddig_right;
        int16_t  y2 = y0 + (y1 - y0) / 2;
        uint32_t color = m_fgColor;
        if (m_clicked) color = m_clickColor;
        tft.fillTriangle(x0, y0, x1, y1, x2, y2, color);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom, uint8_t radius) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_r = radius;
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (!m_text) { m_text = strdup(""); }
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        writeText(m_text);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
    }
    void setTextColor(uint32_t color) { m_fgColor = color; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void setClickColor(uint32_t color) { m_clickColor = color; }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_padding_left = max(m_padding_left, m_borderWidth);
        m_paddig_right = max(m_paddig_right, m_borderWidth);
        m_paddig_top = max(m_paddig_top, m_borderWidth);
        m_paddig_bottom = max(m_paddig_bottom, m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        writeText(m_text);
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;

        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        writeText(m_text);
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
    void setText(const char* txt, bool narrow = false, bool noWrap = false) { // prepare a text, wait of show() to write it
        if (!txt) { txt = strdup(""); }
        x_ps_free(&m_text);
        m_text = x_ps_strdup(txt);
        m_narrow = narrow;
        m_noWrap = noWrap;
    }
    const char* getText() { return m_text; }
    void        setAlign(uint8_t h_align, uint8_t v_align) {
        m_h_align = h_align;
        m_v_align = v_align;
    }
    void writeText(const char* txt) {
        if (!txt) { txt = strdup(""); }
        if (txt != m_text) { // no self copy
            x_ps_free(&m_text);
            m_text = x_ps_strdup(txt);
        }
        if (m_enabled) {
            uint16_t txtColor_tmp = tft.getTextColor();
            uint16_t bgColor_tmp = tft.getBackGroundColor();
            if (!m_clicked)
                tft.setTextColor(m_fgColor);
            else
                tft.setTextColor(m_clickColor);
            tft.setBackGoundColor(m_bgColor);
            if (m_backgroundTransparency) {
                if (m_saveBackground)
                    tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
                else
                    tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            if (m_fontSize != 0) { tft.setFont(m_fontSize); }
            int x = m_x + m_padding_left;
            int y = m_y + m_paddig_top;
            int w = m_w - (m_paddig_right + m_padding_left);
            int h = m_h - (m_paddig_bottom + m_paddig_top);
            if (!m_clicked) {
                if (m_borderWidth > 0) { tft.drawRoundRect(m_x, m_y, m_w, m_h, m_r, m_borderColor); }
                if (m_borderWidth > 1) { tft.drawRoundRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_r, m_borderColor); }
            } else {
                if (m_borderWidth > 0) { tft.drawRoundRect(m_x, m_y, m_w, m_h, m_r, m_clickColor); }
                if (m_borderWidth > 1) { tft.drawRoundRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2, m_r, m_clickColor); }
            }
            if (strcmp(m_text, "/l") == 0) {
                drawTriangeLeft();
            } else if (strcmp(m_text, "/r") == 0) {
                drawTriangeRight();
            } else if (strcmp(m_text, "/u") == 0) {
                drawTriangeUp();
            } else if (strcmp(m_text, "/d") == 0) {
                drawTriangeDown();
            } else
                tft.writeText(m_text, x, y, w, h, m_h_align, m_v_align, m_narrow, m_noWrap, m_autoSize);
            tft.setTextColor(txtColor_tmp);
            tft.setBackGoundColor(bgColor_tmp);
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class selectbox : public RegisterTable {

    /*    —————————————————————————————————————————————————————————————————————————————
         |                   textbox                               |  ⏬  |  ⏫  |idx |
          —————————————————————————————————————————————————————————————————————————————
    */
  private:
    int16_t            m_x = 0;
    int16_t            m_y = 0;
    int16_t            m_w = 0;
    int16_t            m_h = 0;
    uint8_t            m_fontSize = 0;
    uint8_t            m_padding_left = 0;  // left margin
    uint8_t            m_paddig_right = 0;  // right margin
    uint8_t            m_paddig_top = 0;    // top margin
    uint8_t            m_paddig_bottom = 0; // bottom margin
    uint8_t            m_borderWidth = 0;
    int8_t             m_idx = 0;
    uint32_t           m_bgColor = 0;
    uint32_t           m_fgColor = 0;
    uint32_t           m_borderColor = 0;
    ps_ptr<char>       m_name;
    bool               m_enabled = false;
    bool               m_clicked = false;
    bool               m_autoSize = false;
    bool               m_narrow = false;
    bool               m_noWrap = false;
    bool               m_backgroundTransparency = false;
    bool               m_saveBackground = false;
    releasedArg        m_ra;
    textbox*           m_txt_select = new textbox("select_txtbox_ssid");
    textbutton*        m_txt_btn_down = new textbutton("select_txtbtn_down");
    textbutton*        m_txt_btn_up = new textbutton("select_txtbtn_up");
    textbox*           m_txt_btn_idx = new textbox("select_txtbox_idx");
    std::vector<char*> m_selContent;

  public:
    selectbox(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_BLACK;
        setFontSize(fontSize);
    }
    ~selectbox() {
        vector_clear_and_shrink(m_selContent);
        delete m_txt_select;
        delete m_txt_btn_down;
        delete m_txt_btn_up;
        delete m_txt_btn_idx;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        if (m_w < 40) {
            MWR_LOG_WARN("width < 40px");
            return;
        } // width
        m_h = h;
        if (m_h < 10) {
            MWR_LOG_WARN("height < 10px");
            return;
        } // high
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_txt_select->begin(m_x, m_y, m_w - (m_h * 3), m_h, m_padding_left, m_paddig_right, m_paddig_top, m_paddig_bottom);
        m_txt_btn_down->begin(m_x + m_w - (m_h * 3), m_y, m_h, m_h, m_h / 5, m_h / 5, m_h / 5, m_h / 5, 0);
        m_txt_btn_up->begin(m_x + m_w - (m_h * 2), m_y, m_h, m_h, m_h / 5, m_h / 5, m_h / 5, m_h / 5, 0);
        m_txt_btn_idx->begin(m_x + m_w - (m_h * 1), m_y, m_h, m_h, m_padding_left, m_paddig_right, m_paddig_top, m_paddig_bottom);
        m_txt_btn_down->setClickColor(TFT_CYAN);
        m_txt_btn_down->setText("/d");
        m_txt_btn_up->setClickColor(TFT_CYAN);
        m_txt_btn_up->setText("/u");
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void show(bool backgroundTransparency, bool saveBackground) {

        m_txt_select->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_txt_btn_down->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_txt_btn_up->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_txt_btn_idx->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_txt_select->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_down->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_up->show(m_backgroundTransparency, m_saveBackground);
        m_txt_btn_idx->show(m_backgroundTransparency, m_saveBackground);
        m_enabled = true;
        m_clicked = false;

        m_idx = 0;
        if (m_selContent.size() > 0) writeText(m_idx);
    }
    void hide() {
        m_enabled = false;
        m_txt_select->hide();
        m_txt_btn_down->hide();
        m_txt_btn_up->hide();
        m_txt_btn_idx->hide();
    }
    void disable() {
        m_enabled = false;
        m_txt_select->disable();
        m_txt_btn_down->disable();
        m_txt_btn_up->disable();
        m_txt_btn_idx->disable();
    }
    void enable() {
        m_enabled = true;
        m_txt_select->enable();
        m_txt_btn_down->enable();
        m_txt_btn_up->enable();
        m_txt_btn_idx->enable();
    }
    void setFontSize(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
        m_txt_select->setFont(m_fontSize);
        m_txt_btn_down->setFont(m_fontSize);
        m_txt_btn_up->setFont(m_fontSize);
        m_txt_btn_idx->setFont(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        m_txt_select->setTextColor(m_fgColor);
        m_txt_btn_down->setTextColor(m_fgColor);
        m_txt_btn_up->setTextColor(m_fgColor);
        m_txt_btn_idx->setTextColor(m_fgColor);
    }
    void setBGcolor(uint32_t color) {
        m_bgColor = color;
        m_txt_select->setBGcolor(m_bgColor);
        m_txt_btn_down->setBGcolor(m_bgColor);
        m_txt_btn_up->setBGcolor(m_bgColor);
        m_txt_btn_idx->setBGcolor(m_bgColor);
    }
    void setBorderColor(uint32_t color) {
        m_borderColor = color;
        m_txt_select->setBorderColor(m_borderColor);
        m_txt_btn_down->setBorderColor(m_borderColor);
        m_txt_btn_up->setBorderColor(m_borderColor);
        m_txt_btn_idx->setBorderColor(m_borderColor);
    }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_txt_select->setBorderWidth(m_borderWidth);
        m_txt_btn_down->setBorderWidth(m_borderWidth);
        m_txt_btn_up->setBorderWidth(m_borderWidth);
        m_txt_btn_idx->setBorderWidth(m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (m_txt_select->positionXY(x, y)) { ; }
        if (m_txt_btn_down->positionXY(x, y)) { ; }
        if (m_txt_btn_up->positionXY(x, y)) { ; }
        if (m_txt_btn_idx->positionXY(x, y)) { ; }
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        bool ret = false;
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        m_txt_select->released();
        if (m_txt_btn_down->released())
            if (m_idx < m_selContent.size() - 1) {
                m_idx++; /* MWR_LOG_DEBUG("btn_down %i/%i", m_idx, m_selContent.size()); */
                writeText(m_idx);
                ret = true;
            }
        if (m_txt_btn_up->released())
            if (m_idx > 0) {
                m_idx--; /* MWR_LOG_DEBUG("btn_up %i/%i",   m_idx, m_selContent.size()); */
                writeText(m_idx);
                ret = true;
            }
        m_txt_btn_idx->released();
        m_clicked = false;
        return ret;
    }
    void addText(const char* txt) {
        if (!txt) { return; }
        if (m_selContent.size() > 0) {
            for (uint8_t i = 0; i < m_selContent.size(); i++) {
                if (strcmp(txt, m_selContent[i]) == 0) {
                    //    MWR_LOG_WARN("addText: %s already in list", txt);
                    return;
                }
            }
        }
        m_selContent.push_back(x_ps_strdup(txt));
    }
    void clearText() { vector_clear_and_shrink(m_selContent); }
    void writeText(uint8_t idx) {
        char* txt = NULL;
        if (idx >= m_selContent.size()) {
            txt = strdup("");
        } else
            txt = m_selContent[idx];
        if (m_enabled) {
            MWR_LOG_DEBUG("writeText: %s", txt);
            m_txt_select->setText(txt, m_narrow, m_noWrap);
            m_txt_select->show(m_backgroundTransparency, m_saveBackground);
            char c_idx[5] = {0};
            itoa(idx + 1, c_idx, 10);
            m_txt_btn_idx->setText(c_idx, m_narrow, m_noWrap);
            m_txt_btn_idx->show(m_backgroundTransparency, m_saveBackground);
        }
    }
    char* getSelectedText() {
        if (m_selContent.size() > 0) { return m_selContent[m_idx]; }
        return NULL;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class keyBoard : public RegisterTable { // show time "hh:mm:ss" e.g. in header
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_r = 0;
    uint8_t      m_padding_left = 0;
    uint8_t      m_paddig_right = 0;
    uint8_t      m_paddig_top = 0;
    uint8_t      m_paddig_bottom = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_val = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    ps_ptr<char> m_name;
    const char*  m_txt = NULL;
    float        m_row1[12] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float        m_row2[11] = {1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.6};
    float        m_row3[11] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.1};
    const char   m_alpha1[12][4] = {"1..", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "BS"};
    const char   m_alpha2[11][4] = {"A..", "a", "s", "d", "f", "g", "h", "j", "k", "l", "RET"};
    const char   m_alpha3[11][6] = {"#..", ".", "z", "x", "c", "v", "b", "n", "m", "_", "   "};
    const char   m_Alpha1[12][4] = {"1..", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "BS"};
    const char   m_Alpha2[11][4] = {"a..", "A", "S", "D", "F", "G", "H", "J", "K", "L", "RET"};
    const char   m_Alpha3[11][6] = {"#..", ".", "Z", "X", "C", "V", "B", "N", "M", "_", "   "};
    const char   m_special1[12][4] = {"1..", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BS"};
    const char   m_special2[11][4] = {"a..", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "RET"};
    const char   m_special3[11][6] = {"#..", "*", "+", ",", "-", "*", "-", ".", "/", ":", "   "};
    const char   m_Special1[12][4] = {"1..", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "BS"};
    const char   m_Special2[11][4] = {"a..", ";", "<", "=", ">", "?", "@", "[", "\\", "]", "RET"};
    const char   m_Special3[11][6] = {"#..", "^", "_", "`", "{", "|", "}", "~", "#", "$", "   "};
    uint32_t     m_color1[12] = {TFT_YELLOW,    TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY,
                                 TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_YELLOW};
    uint32_t     m_color2[11] = {TFT_YELLOW, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_RED};
    uint32_t     m_color3[11] = {TFT_YELLOW, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_LIGHTGREY};
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_clickColor = TFT_CYAN;
    textbutton*  txt_btn_array = new textbutton[34]{textbutton("txt_btn0"),  textbutton("txt_btn1"),  textbutton("txt_btn2"),  textbutton("txt_btn3"),  textbutton("txt_btn4"),  textbutton("txt_btn5"),
                                                    textbutton("txt_btn6"),  textbutton("txt_btn7"),  textbutton("txt_btn8"),  textbutton("txt_btn9"),  textbutton("txt_btn10"), textbutton("txt_btn11"),
                                                    textbutton("txt_btn12"), textbutton("txt_btn13"), textbutton("txt_btn14"), textbutton("txt_btn15"), textbutton("txt_btn16"), textbutton("txt_btn17"),
                                                    textbutton("txt_btn18"), textbutton("txt_btn19"), textbutton("txt_btn20"), textbutton("txt_btn21"), textbutton("txt_btn22"), textbutton("txt_btn23"),
                                                    textbutton("txt_btn24"), textbutton("txt_btn25"), textbutton("txt_btn26"), textbutton("txt_btn27"), textbutton("txt_btn28"), textbutton("txt_btn29"),
                                                    textbutton("txt_btn30"), textbutton("txt_btn31"), textbutton("txt_btn32"), textbutton("txt_btn33")};

  public:
    keyBoard(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_fontSize = fontSize;
    }
    ~keyBoard() { delete[] txt_btn_array; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        uint8_t btnW = (m_w - (paddig_left + paddig_right)) / 12;
        uint8_t btnH = (m_h - (paddig_top + paddig_bottom)) / 3;
        uint8_t margin = btnW / 17;
        btnW -= margin;
        btnH -= margin;
        uint8_t  radius = btnW / 10;
        uint16_t posX = m_x + paddig_left;
        uint16_t posY = m_y + m_paddig_top;
        m_padding_left = paddig_left;
        m_paddig_right = paddig_right;
        m_paddig_top = paddig_top;
        m_paddig_bottom = paddig_bottom;
        m_w = 12 * btnW + 11 * margin + paddig_left + paddig_right; // recalculate width
        m_h = 3 * btnH + 2 * margin + paddig_top + paddig_bottom;   // recalculate high
        for (int i = 0; i < 12; i++) {                              // row 1
            txt_btn_array[i].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row1[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i].setBGcolor(m_bgColor);
            txt_btn_array[i].setTextColor(m_color1[i]);
            txt_btn_array[i].setBorderColor(m_color1[i]);
            txt_btn_array[i].setClickColor(m_clickColor);
            txt_btn_array[i].setBorderWidth(1);
            txt_btn_array[i].setFont(m_fontSize);
            txt_btn_array[i].setText(m_alpha1[i]);
            txt_btn_array[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row1[i] * btnW + margin;
        }
        posY += btnH + margin;
        posX = m_x;
        for (int i = 0; i < 11; i++) { // row 2
            txt_btn_array[i + 12].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row2[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i + 12].setBGcolor(m_bgColor);
            txt_btn_array[i + 12].setTextColor(m_color2[i]);
            txt_btn_array[i + 12].setBorderColor(m_color2[i]);
            txt_btn_array[i + 12].setClickColor(m_clickColor);
            txt_btn_array[i + 12].setBorderWidth(1);
            txt_btn_array[i + 12].setFont(m_fontSize);
            txt_btn_array[i + 12].setText(m_alpha2[i]);
            txt_btn_array[i + 12].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row2[i] * btnW + margin;
        }
        posY += btnH + margin;
        posX = m_x;
        for (int i = 0; i < 11; i++) { // row 3
            txt_btn_array[i + 23].begin(posX + m_padding_left, posY + m_paddig_top, btnW * m_row3[i], btnH, 0, 0, 0, 0, radius);
            txt_btn_array[i + 23].setBGcolor(m_bgColor);
            txt_btn_array[i + 23].setTextColor(m_color3[i]);
            txt_btn_array[i + 23].setBorderColor(m_color3[i]);
            txt_btn_array[i + 23].setClickColor(m_clickColor);
            txt_btn_array[i + 23].setBorderWidth(1);
            txt_btn_array[i + 23].setFont(m_fontSize);
            txt_btn_array[i + 23].setText(m_alpha3[i]);
            txt_btn_array[i + 23].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            posX += m_row3[i] * btnW + margin;
        }
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        m_clicked = false;
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        if (!m_backgroundTransparency) tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        for (int i = 0; i < 34; i++) { txt_btn_array[i].show(m_backgroundTransparency, m_saveBackground); }
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void    disable() { m_enabled = false; }
    void    enable() { m_enabled = true; }
    uint8_t getVal() { return m_val; }
    bool    positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        for (int i = 0; i < 34; i++) {
            if (txt_btn_array[i].positionXY(x, y)) m_txt = txt_btn_array[i].getText();
        }
        if (m_txt) {
            if (strcmp(m_txt, "BS") == 0) {
                m_val = 8;
            } // BS
            else if (strcmp(m_txt, "RET") == 0) {
                m_val = 13;
            } // CR
            else if (strcmp(m_txt, "   ") == 0) {
                m_val = 32;
            } // space
            else if (strcmp(m_txt, "1..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "a..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "A..") == 0) {
                m_val = 0;
            } else if (strcmp(m_txt, "#..") == 0) {
                m_val = 0;
            } else
                m_val = m_txt[0];
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_val);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        // if(graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        for (int i = 0; i < 34; i++) {
            if (txt_btn_array[i].released()) {
                if (strcmp(txt_btn_array[i].getText(), "A..") == 0) { // upcase
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_Alpha1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_Alpha2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_Alpha3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "a..") == 0) { // lowcase
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_alpha1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_alpha2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_alpha3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "1..") == 0) { // special
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_special1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_special2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_special3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
                if (strcmp(txt_btn_array[i].getText(), "#..") == 0) { // Special
                    for (int j = 0; j < 12; j++) { txt_btn_array[j].setText(m_Special1[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 12].setText(m_Special2[j]); }
                    for (int j = 0; j < 11; j++) { txt_btn_array[j + 23].setText(m_Special3[j]); }
                    for (int j = 0; j < 34; j++) { txt_btn_array[j].show(m_backgroundTransparency, m_saveBackground); }
                    break;
                }
            }
        }
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class wifiSettings : public RegisterTable {
    /*                —————————————————————————————————————————————————————————————————————————————
                     |                             selectbox (SSID)            |  ⏬  |  ⏫  |idx |
                      —————————————————————————————————————————————————————————————————————————————
                      —————————————————————————————————————————————————————————————————————————————
                     |                             inputbox (Password)                            |
                      —————————————————————————————————————————————————————————————————————————————
                      —————————————————————————————————————————————————————————————————————————————
                     |                                                                            |
                     |                                                                            |
                     |                                 keyBoard                                   |
                     |                                                                            |
                     |                                                                            |
                      —————————————————————————————————————————————————————————————————————————————
    */
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_padding_left = 0;  // left margin
    uint8_t      m_padding_right = 0;  // right margin
    uint8_t      m_padding_top = 0;    // top margin
    uint8_t      m_padding_bottom = 0; // bottom margin
    uint8_t      m_credentials_idx = 0;
    uint8_t      m_borderWidth = 0;
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_borderColor = TFT_BLACK;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_autoSize = false;
    bool         m_narrow = false;
    bool         m_noWrap = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    releasedArg  m_ra;
    selectbox*   m_sel_ssid = new selectbox("wifiSettings_selectbox_ssid", 0);
    inputbox*    m_in_password = new inputbox("wifiSettings_txtbox_pwd");
    keyBoard*    m_keyboard = new keyBoard("wifiSettings_keyBoard", 0);

    struct credentials {
        ps_ptr<char> ssid;
        ps_ptr<char> password;

        credentials(const char* s, const char* p) : ssid(s), password(p) {}
    };
    deque<credentials> m_credentials;

    struct w_se {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winSelect;
    struct w_pwd {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winPWD;
    struct w_k {
        uint16_t x = 0;
        uint16_t y = 0;
        uint16_t w = 0;
        uint16_t h = 0;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_winKeybrd;

  public:
    wifiSettings(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_borderColor = TFT_LIGHTGREY;
        setFontSize(fontSize);
        m_in_password->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_in_password->setTextColor(m_fgColor);
        m_in_password->setBGcolor(m_bgColor);
        m_in_password->setBorderColor(m_borderColor);
        m_in_password->setBorderWidth(m_borderWidth);
        m_in_password->setFont(0); // auto size
    }
    ~wifiSettings() {
        m_credentials.clear();
        delete m_sel_ssid;
        delete m_in_password;
        delete m_keyboard;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t padding_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        m_padding_left = padding_left;
        m_padding_right = paddig_right;
        m_padding_top = paddig_top;
        m_padding_bottom = paddig_bottom;

        if (w == 320) { // s 320x240
            m_winSelect.x = 10;
            m_winSelect.y = 40;
            m_winSelect.w = 300;
            m_winSelect.h = 28;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 10;
            m_winPWD.y = 80;
            m_winPWD.w = 300;
            m_winPWD.h = 28;
            m_winPWD.pl = 1;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 10;
            m_winKeybrd.y = 120;
            m_winKeybrd.w = 300;
            m_winKeybrd.h = 75;
            m_winKeybrd.pl = 1;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        } else if (w == 480) {  // m 480x320
            m_winSelect.x = 12;
            m_winSelect.y = 50;
            m_winSelect.w = 456;
            m_winSelect.h = 30;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 12;
            m_winPWD.y = 90;
            m_winPWD.w = 456;
            m_winPWD.h = 30;
            m_winPWD.pl = 1;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 12;
            m_winKeybrd.y = 160;
            m_winKeybrd.w = 456;
            m_winKeybrd.h = 114;
            m_winKeybrd.pl = 1;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        } else if (w == 800) {  // l 800x480
            m_winSelect.x = 82;
            m_winSelect.y = 70;
            m_winSelect.w = 636;
            m_winSelect.h = 50;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = 82;
            m_winPWD.y = 130;
            m_winPWD.w = 636;
            m_winPWD.h = 50;
            m_winPWD.pl = 3;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = 82;
            m_winKeybrd.y = 240;
            m_winKeybrd.w = 636;
            m_winKeybrd.h = 160;
            m_winKeybrd.pl = 3;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        } else if (w == 1024) {  // l 1024x600
            m_winSelect.x = m_x + m_w / 30;
            m_winSelect.y = m_y + 70;
            m_winSelect.w = m_w - (2 * m_w / 30);
            m_winSelect.h = 50;
            m_winSelect.pl = 1;
            m_winSelect.pr = 1;
            m_winSelect.pt = 1;
            m_winSelect.pb = 1; // selectbox
            m_winPWD.x = m_x + m_w / 30;
            m_winPWD.y = m_winSelect.y + m_winSelect.h +10;
            m_winPWD.w = m_w - (2 * m_w / 30);
            m_winPWD.h = 50;
            m_winPWD.pl = 3;
            m_winPWD.pr = 1;
            m_winPWD.pt = 1;
            m_winPWD.pb = 1; // password
            m_winKeybrd.x = m_x + m_w / 30;
            m_winKeybrd.y = m_winPWD.y + m_winPWD.h + 10;
            m_winKeybrd.w = m_w - (2 * m_w / 30);
            m_winKeybrd.h = m_h - (m_winKeybrd.y + 10);
            m_winKeybrd.pl = 3;
            m_winKeybrd.pr = 1;
            m_winKeybrd.pt = 1;
            m_winKeybrd.pb = 1; // keyboard
        }
        else {
            MWR_LOG_WARN("unsupported resolution width %i px", w);
            return;
        }

        m_sel_ssid->begin(m_winSelect.x, m_winSelect.y, m_winSelect.w, m_winSelect.h, m_winSelect.pl, m_winSelect.pr, m_winSelect.pt, m_winSelect.pb);
        m_in_password->begin(m_winPWD.x, m_winPWD.y, m_winPWD.w, m_winPWD.h, m_winPWD.pl, m_winPWD.pr, m_winPWD.pt, m_winPWD.pb);
        m_keyboard->begin(m_winKeybrd.x, m_winKeybrd.y, m_winKeybrd.w, m_winKeybrd.h, m_winKeybrd.pl, m_winKeybrd.pr, m_winKeybrd.pt, m_winKeybrd.pb);
    }
    ps_ptr<char> getName() { return m_name; }

    bool isEnabled() { return m_enabled; }

    void show(bool backgroundTransparency, bool saveBackground) {
        m_in_password->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_sel_ssid->show(m_backgroundTransparency, m_saveBackground);
        m_in_password->setText(m_credentials[0].password.c_get());
        m_in_password->show(m_backgroundTransparency, m_saveBackground);
        m_keyboard->show(m_backgroundTransparency, m_saveBackground);
        m_enabled = true;
        m_clicked = false;
    }
    void hide() {
        m_enabled = false;
        m_sel_ssid->hide();
        m_in_password->hide();
        m_keyboard->hide();
    }
    void disable() {
        m_enabled = false;
        m_sel_ssid->disable();
        m_in_password->disable();
        m_keyboard->disable();
    }
    void enable() {
        m_enabled = true;
        m_sel_ssid->enable();
        m_in_password->enable();
        m_keyboard->enable();
    }
    void setFontSize(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = 0;
        if (size != 0) {
            m_fontSize = size;
            tft.setFont(m_fontSize);
        } else {
            m_autoSize = true;
        }
        m_sel_ssid->setFontSize(m_fontSize);
        m_in_password->setFont(m_fontSize);
        //    m_keyboard->setFontSize(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        m_sel_ssid->setTextColor(m_fgColor);
        m_in_password->setTextColor(m_fgColor);
        // m_keyboard->setTextColor(m_fgColor);
    }
    void setBGcolor(uint32_t color) {
        m_bgColor = color;
        m_sel_ssid->setBGcolor(m_bgColor);
        m_in_password->setBGcolor(m_bgColor);
        //    m_keyboard->setBGcolor(m_bgColor);
    }
    void setBorderColor(uint32_t color) {
        m_borderColor = color;
        m_sel_ssid->setBorderColor(m_borderColor);
        m_in_password->setBorderColor(m_borderColor);
    }
    void setBorderWidth(uint8_t width) { // 0 = no border
        m_borderWidth = width;
        if (m_borderWidth > 2) m_borderWidth = 2;
        m_sel_ssid->setBorderWidth(m_borderWidth);
        m_in_password->setBorderWidth(m_borderWidth);
        //    m_keyboard->setBorderWidth(m_borderWidth);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name.c_get(), m_enabled);
        if (m_sel_ssid->positionXY(x, y)) { ; }
        if (m_in_password->positionXY(x, y)) { ; }
        if (m_keyboard->positionXY(x, y)) {
            MWR_LOG_INFO("key pressed %i", m_keyboard->getVal());
            changePassword(m_keyboard->getVal(), m_credentials_idx);
            m_in_password->setText(m_credentials[m_credentials_idx].password.c_get());
            m_in_password->show(m_backgroundTransparency, m_saveBackground);
        }
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        bool ret = false;
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (m_sel_ssid->released()) {
            const char* selTxt = m_sel_ssid->getSelectedText();
            if (selTxt) {
                for (int i = 0; i < m_credentials.size(); i++) {
                    if (m_credentials[i].ssid.equals(selTxt)) {
                        m_in_password->setText(m_credentials[i].password.c_get());
                        m_in_password->show(m_backgroundTransparency, m_saveBackground);
                        m_credentials_idx = i;
                    }
                }
            }
            ret = true;
        }
        if (m_in_password->released()) { /*log_e("m_txt_password released")*/
            ;
        }
        if (m_keyboard->released()) { /*log_e("keyboard released")*/
            ;
        }
        if (m_keyboard->getVal() == 0x0D) {                              // enter
            m_ra.arg1 = m_credentials[m_credentials_idx].ssid;           // ssid
            m_ra.arg2 = m_credentials[m_credentials_idx].password;      // password
            // log_w("enter pressed ssid %s, password %s", m_ssid[m_pwd_idx], m_password[m_pwd_idx]);
            if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name.c_get(), m_ra);
        }
        m_clicked = false;
        return ret;
    }
    void add_WiFi_Items(ps_ptr<char> ssid, ps_ptr<char> pw) {
        if (ssid.strlen() == 0) { ssid = ""; }
        m_credentials.emplace_back(ssid.c_get(), pw.c_get());
        m_sel_ssid->addText(ssid.c_get());
    }
    void clearText() {
        m_sel_ssid->clearText();
        m_in_password->setText("");
    }
    char* getSelectedText() {
        //    if(m_selContent.size() > 0){return m_selContent[m_idx];}
        return NULL;
    }

  private:
    void changePassword(char ch, uint8_t idx) {
        int len = m_credentials[idx].password.strlen();
        if (ch == 0x08) { // backspace
            if (len == 0) return;
            m_credentials[idx].password = m_credentials[idx].password.substr(0, len - 1);
        } else if (ch == 0x0D) { // enter
            //    log_w("enter pressed");
        } else {
            if (len < 63) {
                char c[2] = {0};
                c[0] = ch;
                m_credentials[idx].password.append(c);
            }
        }
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class timeString : public RegisterTable { // show time "hh:mm:ss" e.g. in header
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_h_align = TFT_ALIGN_CENTER;
    uint8_t      m_v_align = TFT_ALIGN_CENTER;
    uint32_t     m_bgColor = 0;
    uint32_t     m_fgColor = 0;
    uint32_t     m_borderColor = 0;
    ps_ptr<char> m_name;
    char         m_time[10] = "00:00:00";
    bool         m_enabled = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    bool         m_clicked = false;
    releasedArg  m_ra;
    textbox*     txt_time = new textbox[8]{textbox("txt_timeH10"), textbox("txt_timeH01"), textbox("txt_timeC1"),  textbox("txt_timeM10"),
                                           textbox("txt_timeM01"), textbox("txt_timeC2"),  textbox("txt_timeS10"), textbox("txt_timeS01")}; // time of the day
  public:
    timeString(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fgColor = TFT_LIGHTGREY;
        m_fontSize = fontSize;
    }
    ~timeString() { delete[] txt_time; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t pl, uint16_t pr, uint16_t pt, uint16_t pb) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        uint8_t  w_digits = m_w / 7;
        uint8_t  w_colon = w_digits / 2;
        uint16_t xPos[8] = {
            static_cast<uint16_t>(m_x + pl + 0 * w_digits + 0 * w_colon), /* H10 */
            static_cast<uint16_t>(m_x + pl + 1 * w_digits + 0 * w_colon), /* H01 */
            static_cast<uint16_t>(m_x + pl + 2 * w_digits + 0 * w_colon), /* C1 */
            static_cast<uint16_t>(m_x + pl + 2 * w_digits + 1 * w_colon), /* M10 */
            static_cast<uint16_t>(m_x + pl + 3 * w_digits + 1 * w_colon), /* M01 */
            static_cast<uint16_t>(m_x + pl + 4 * w_digits + 1 * w_colon), /* C2 */
            static_cast<uint16_t>(m_x + pl + 4 * w_digits + 2 * w_colon), /* S10 */
            static_cast<uint16_t>(m_x + pl + 5 * w_digits + 2 * w_colon)  /* S01 */
        };
        uint8_t width[8] = {w_digits, w_digits, w_colon, w_digits, w_digits, w_colon, w_digits, w_digits};
        for (uint8_t i = 0; i < 8; i++) {
            txt_time[i].begin(xPos[i], m_y + pt, width[i], h, 0, 0, 0, 0);
            txt_time[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_time[i].setTextColor(m_fgColor);
            txt_time[i].setFont(m_fontSize);
        }
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        m_enabled = true;
        if (m_saveBackground) tft.copyFramebuffer(0, 2, m_x, m_y, m_w, m_h);
        updateTime(m_time, true);
    }
    void hide() {
        if (m_backgroundTransparency) {
            if (m_saveBackground)
                tft.copyFramebuffer(2, 0, m_x, m_y, m_w, m_h);
            else
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    void setFont(uint8_t size) { // size 0 -> auto, choose besr font size
        m_fontSize = size;
        m_fontSize = size;
        tft.setFont(m_fontSize);
    }
    void setTextColor(uint32_t color) {
        m_fgColor = color;
        for (uint8_t i = 0; i < 8; i++) { txt_time[i].setTextColor(m_fgColor); }
    }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void setBorderColor(uint32_t color) { m_borderColor = color; }
    void updateTime(const char* hl_time, bool complete = true) {
        if (!hl_time) return;
        if (strlen(hl_time) != 8) return;
        if (!m_enabled) return;
        memcpy(m_time, hl_time, 8);     // hhmmss
        static char oldtime[8] = {255}; // hhmmss
        tft.setFont(m_fontSize);
        tft.setTextColor(m_fgColor);
        if (complete == true) {
            if (m_backgroundTransparency) {
                tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
            } else {
                tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
            }
            for (uint8_t i = 0; i < 8; i++) { oldtime[i] = 255; }
        }
        for (uint8_t i = 0; i < 8; i++) {
            if (oldtime[i] != m_time[i]) {
                char ch[2] = {0, 0};
                ch[0] = m_time[i];
                txt_time[i].setText(ch, true);
                txt_time[i].show(m_backgroundTransparency, m_saveBackground);
                oldtime[i] = m_time[i];
            }
        }
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button1state : public RegisterTable { // click button
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint32_t     m_bgColor = 0;
    char*        m_defaultPicturePath = NULL;
    char*        m_clickedPicturePath = NULL;
    char*        m_inactivePicturePath = NULL;
    char*        m_alternativePicturePath = NULL; // e.g. IR select
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_backgroundTransparency = false;
    ps_ptr<char> m_name;
    releasedArg  m_ra;

  public:
    button1state(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        setDefaultPicturePath(NULL);
        setClickedPicturePath(NULL);
        setInactivePicturePath(NULL);
        setAlternativePicturePath(NULL);
    }
    ~button1state() {
        x_ps_free(&m_defaultPicturePath);
        x_ps_free(&m_clickedPicturePath);
        x_ps_free(&m_inactivePicturePath);
        x_ps_free(&m_alternativePicturePath);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool backgroundTransparency = false) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_backgroundTransparency = backgroundTransparency;
    }
    ps_ptr<char> getName() { return m_name; }
    void enable() { m_enabled = true; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            setInactive();
            return;
        }
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_enabled = true;
    }
    void hide() {
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void setInactive() {
        drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        m_enabled = false;
    }
    void showAlternativePic(bool inactive = false) {
        m_clicked = false;
        m_enabled = true;
        if (inactive) {
            setInactive();
            return;
        }
        drawImage(m_alternativePicturePath, m_x, m_y, m_w, m_h);
    }
    void showClickedPic() { drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h); }
    void setDefaultPicturePath(const char* path) {
        x_ps_free(&m_defaultPicturePath);
        if (path)
            m_defaultPicturePath = x_ps_strdup(path);
        else
            m_defaultPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedPicturePath(const char* path) {
        x_ps_free(&m_clickedPicturePath);
        if (path)
            m_clickedPicturePath = x_ps_strdup(path);
        else
            m_clickedPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path) {
        x_ps_free(&m_inactivePicturePath);
        if (path)
            m_inactivePicturePath = x_ps_strdup(path);
        else
            m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }
    void setAlternativePicturePath(const char* path) {
        x_ps_free(&m_alternativePicturePath);
        if (path)
            m_alternativePicturePath = x_ps_strdup(path);
        else
            m_alternativePicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    bool click() { // e.g. from IR
        if (!m_enabled) { return false; }
        drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        return true;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) {
            drawImage(m_clickedPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        drawImage(m_defaultPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class button2state : public RegisterTable { // on off switch
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint32_t     m_bgColor = 0;
    char*        m_offPicturePath = NULL;
    char*        m_onPicturePath = NULL;
    char*        m_clickedOffPicturePath = NULL;
    char*        m_clickedOnPicturePath = NULL;
    char*        m_inactivePicturePath = NULL;
    char*        m_alternativeOnPicturePath = NULL;
    char*        m_alternativeOffPicturePath = NULL;
    bool         m_enabled = false;
    bool         m_active = true;
    bool         m_clicked = false;
    bool         m_state = false;
    ps_ptr<char> m_name;
    releasedArg  m_ra;

  public:
    button2state(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        setOffPicturePath(NULL);
        setOnPicturePath(NULL);
        setClickedOffPicturePath(NULL);
        setClickedOnPicturePath(NULL);
        setInactivePicturePath(NULL);
    }
    ~button2state() {
        x_ps_free(&m_offPicturePath);
        x_ps_free(&m_onPicturePath);
        x_ps_free(&m_clickedOffPicturePath);
        x_ps_free(&m_clickedOnPicturePath);
        x_ps_free(&m_inactivePicturePath);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_enabled = false;
        m_active = true;
    }
    ps_ptr<char> getName() { return m_name; }
    void enable() { m_enabled = true; }
    bool         isEnabled() { return m_enabled; }
    void         show() {
        m_clicked = false;
        if (m_active) {
            if (m_state)
                drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
            m_enabled = true;
        } else {
            drawImage(m_inactivePicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void showClickedPic() {
        if (m_state) {
            drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
        } else {
            drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void showAlternativePic() {
        if (m_state) {
            drawImage(m_alternativeOnPicturePath, m_x, m_y, m_w, m_h);
        } else {
            drawImage(m_alternativeOffPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    void setAlternativeOnPicturePath(const char* path) {
        x_ps_free(&m_alternativeOnPicturePath);
        if (path)
            m_alternativeOnPicturePath = x_ps_strdup(path);
        else
            m_alternativeOnPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    void setAlternativeOffPicturePath(const char* path) {
        x_ps_free(&m_alternativeOffPicturePath);
        if (path)
            m_alternativeOffPicturePath = x_ps_strdup(path);
        else
            m_alternativeOffPicturePath = x_ps_strdup("alternativePicturePath is not set");
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void setValue(bool val) {
        m_state = val;
        if (m_enabled) {
            if (m_state)
                drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        }
    }
    bool getValue() { return m_state; }
    void setOn() { m_state = true; }
    void setOff() { m_state = false; }
    void setActive(bool act) { m_active = act; }
    bool getActive() { return m_active; }

    void setOffPicturePath(const char* path) {
        x_ps_free(&m_offPicturePath);
        if (path)
            m_offPicturePath = x_ps_strdup(path);
        else
            m_offPicturePath = x_ps_strdup("defaultPicturePath is not set");
    }
    void setClickedOffPicturePath(const char* path) {
        x_ps_free(&m_clickedOffPicturePath);
        if (path)
            m_clickedOffPicturePath = x_ps_strdup(path);
        else
            m_clickedOffPicturePath = x_ps_strdup("clickedOffPicturePath is not set");
    }
    void setClickedOnPicturePath(const char* path) {
        x_ps_free(&m_clickedOnPicturePath);
        if (path)
            m_clickedOnPicturePath = x_ps_strdup(path);
        else
            m_clickedOnPicturePath = x_ps_strdup("clickedOnPicturePath is not set");
    }
    void setOnPicturePath(const char* path) {
        x_ps_free(&m_onPicturePath);
        if (path)
            m_onPicturePath = x_ps_strdup(path);
        else
            m_onPicturePath = x_ps_strdup("clickedPicturePath is not set");
    }
    void setInactivePicturePath(const char* path) {
        x_ps_free(&m_inactivePicturePath);
        if (path)
            m_inactivePicturePath = x_ps_strdup(path);
        else
            m_inactivePicturePath = x_ps_strdup("inactivePicturePath is not set");
    }

    bool click() {
        if (!m_enabled) return false;
        if (m_state)
            drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
        else
            drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = true;
        m_state = !m_state;

        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        return true;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) {
            if (m_state)
                drawImage(m_clickedOnPicturePath, m_x, m_y, m_w, m_h);
            else
                drawImage(m_clickedOffPicturePath, m_x, m_y, m_w, m_h);
            m_clicked = true;
            m_state = !m_state;
        }
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (m_state)
            drawImage(m_onPicturePath, m_x, m_y, m_w, m_h);
        else
            drawImage(m_offPicturePath, m_x, m_y, m_w, m_h);
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class numbersBox : public RegisterTable { // range 000...999
  private:
    bool         m_enabled = false;
    uint16_t     m_segmWidth = 0;
    uint16_t     m_segmentHigh = 0;
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_box_x = 0;
    int16_t      m_box_y = 0;
    int16_t      m_box_w = 0;
    int16_t      m_box_h = 0;
    uint32_t     m_bgColor = TFT_BLACK;
    bool         m_clicked = false;
    releasedArg  m_ra;
    ps_ptr<char> m_name;
    const char*  m_color = "blue";
    char         m_root[20] = "/digits/s/";
    char         m_numbers[4] = "000";

  public:
    numbersBox(const char* name) {
        register_object(this);
        m_name = name;
    }
    ~numbersBox() { ; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        placingDigits(w, h);
        m_enabled = false;
    }
    ps_ptr<char> getName() { return m_name; }

    bool show(uint16_t color) {
        if     (color == TFT_BLUE) m_color = "blue";
        else if(color == TFT_ORANGE) m_color = "orange";
        else if(color == TFT_GREEN) m_color = "green";
        else if(color == TFT_RED) m_color = "red";
        else m_color = "orange";
        ps_ptr<char> path;
        for (uint8_t i = 0; i < 3; i++) {
            path.assignf("%s%c%s.jpg", m_root, m_numbers[i], m_color);
            if (!drawImage(path.c_get(), m_x + m_box_x + i * m_segmWidth, m_y + m_box_y)) return false;
        }
        m_enabled = true;
        return true;
    }
    void hide() {
        tft.fillRect(m_x + m_box_x, m_y + m_box_y, m_box_w, m_box_h, m_bgColor);
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    bool isEnabled() { return m_enabled; }
    void setNumbers(uint16_t numbers) {
        if (numbers > 999) return;
        snprintf(m_numbers, sizeof(m_numbers), "%03u", numbers);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
private:
    void placingDigits(uint16_t w, uint16_t h){

       imgSize img = GetImageSize("/digits/s/0green.jpg"); // get size of digit '0'
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get digit size");
            return;
        }
        MWR_LOG_DEBUG("digits w = %i, h = %i", img.w, img.h);
        m_segmWidth = img.w;
        m_segmentHigh = img.h;

        m_box_w = 3 * m_segmWidth;
        m_box_h = m_segmentHigh;
        m_box_x = (w - m_box_w) / 2;
        m_box_y = (h - m_box_h) / 2;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class offTimerBox : public RegisterTable { // range 000...999
  private:
    bool         m_enabled = false;
    uint16_t     m_digitsWidth = 0;
    uint16_t     m_colonWidth = 0;
    uint16_t     m_digitsHigh = 0;
    uint16_t     m_digitsXpos[4] = {0};
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_box_x = 0;
    int16_t      m_box_y = 0;
    int16_t      m_box_w = 0;
    int16_t      m_box_h = 0;
    uint32_t     m_bgColor = 0;
    const char*  m_color = "green";
    uint16_t     m_offColor = TFT_RED;
    uint16_t     m_onColor = TFT_GREEN;
    bool         m_clicked = false;
    releasedArg  m_ra;
    ps_ptr<char> m_name;
    ps_ptr<char> m_path;
    char         m_numbers[10] = "000";

  public:
    offTimerBox(const char* name) {
        register_object(this);
        m_name = name;
    }
    ~offTimerBox() { ; }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        placingDigits(m_w, m_h);
        m_enabled = false;
    }
    ps_ptr<char> getName() { return m_name; }

    bool show(uint16_t time) {
        if(!time) m_color = "red";
        else      m_color = "green";
        ps_ptr<char> numbers;
        numbers.assignf("%dc%02d", time / 60, time % 60);
        m_path.assignf("/digits/s/x%s.jpg", m_color);

        m_path[10] = numbers[0];
        drawImage(m_path.c_get(), m_x + m_digitsXpos[0], m_y + m_box_y);
        m_path[10] = numbers[1];
        drawImage(m_path.c_get(), m_x + m_digitsXpos[1], m_y + m_box_y);
        m_path[10] = numbers[2];
        drawImage(m_path.c_get(), m_x + m_digitsXpos[2], m_y + m_box_y);
        m_path[10] = numbers[3];
        drawImage(m_path.c_get(), m_x + m_digitsXpos[3], m_y + m_box_y);
        m_enabled = true;
        return true;
    }
    void hide() {
        tft.fillRect(m_x + m_box_x, m_y + m_box_y, m_box_w, m_box_h, m_bgColor);
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable() { m_enabled = true; }
    bool isEnabled() { return m_enabled; }
    // void setTime(uint16_t time) {
    //     snprintf(m_numbers, sizeof(m_numbers), "%03u", time);
    // }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
private:
    void placingDigits(uint16_t w, uint16_t h){

       imgSize img = GetImageSize("/digits/s/0green.jpg"); // get size of digit '0'
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get digit size");
            return;
        }
        MWR_LOG_DEBUG("digits w = %i, h = %i", img.w, img.h);
        m_digitsWidth = img.w;
        m_digitsHigh = img.h;

        img = GetImageSize("/digits/s/cgreen.jpg"); // get size of colon
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get colon size");
            return;
        }
        MWR_LOG_DEBUG("colon w = %i, h = %i", img.w, img.h);
        m_colonWidth = img.w;

        m_box_w = 3 * m_digitsWidth + m_colonWidth;
        m_box_h = m_digitsHigh;
        m_box_x = (w - m_box_w) / 2;
        m_box_y = (h - m_box_h) / 2;
        m_digitsXpos[0] = m_box_x;
        m_digitsXpos[1] = m_digitsXpos[0] + m_digitsWidth;
        m_digitsXpos[2] = m_digitsXpos[1] + m_colonWidth;
        m_digitsXpos[3] = m_digitsXpos[2] + m_digitsWidth;
        MWR_LOG_DEBUG("box w=%i, h=%i, x=%i, y=%i, x0=%i, x1=%i, x2=%i, x3=%i", m_box_w, m_box_h, m_box_x, m_box_y, m_digitsXpos[0], m_digitsXpos[1], m_digitsXpos[2], m_digitsXpos[3]);
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class pictureBox : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint16_t     m_image_w = 0;
    uint16_t     m_image_h = 0;
    uint16_t     m_image_x = 0;
    uint16_t     m_image_y = 0;
    uint8_t      m_padding_left = 0;  // left margin
    uint8_t      m_padding_right = 0;  // right margin
    uint8_t      m_padding_top = 0;    // top margin
    uint8_t      m_padding_bottom = 0; // bottom margin
    uint32_t     m_bgColor = 0;
    ps_ptr<char> m_PicturePath;
    ps_ptr<char> m_altPicturePath;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false; // is used and to draw further objects on this box
    releasedArg  m_ra;

  public:
    pictureBox(const char* name) {
        register_object(this);
        m_name = name;
        setPicturePath(NULL);
        setAlternativPicturePath(NULL);
    }
    ~pictureBox() { }

    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t padding_left, uint8_t padding_right, uint8_t padding_top, uint8_t padding_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_padding_left = padding_left;
        m_padding_right = padding_right;
        m_padding_top = padding_top;
        m_padding_bottom = padding_bottom;
        m_enabled = false;
    }
    ps_ptr<char> getName() { return m_name; }

    bool isEnabled() { return m_enabled; }

    bool show(bool backgroundTransparency, bool saveBackground) {
        m_backgroundTransparency = backgroundTransparency;
        m_saveBackground = saveBackground;
        int x = m_x + m_padding_left + m_image_x;
        int y = m_y + m_padding_top + m_image_y;
        int w = m_w - (m_padding_right + m_padding_left);
        int h = m_h - (m_padding_bottom + m_padding_top);
        if (m_image_w == 0 || m_image_h == 0) {
            if (m_saveBackground) { tft.copyFramebuffer(1, 2, m_x, m_y, m_w, m_h); }
            if (m_backgroundTransparency) { tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h); }
            m_enabled = drawImage(m_altPicturePath.c_get(), x, y, w, h);
            if (m_saveBackground) { tft.copyFramebuffer(0, 1, m_x, m_y, m_w, m_h); }
            return m_enabled;
        } else {
            if (m_saveBackground) { tft.copyFramebuffer(1, 2, m_x, m_y, m_w, m_h); }
            if (m_backgroundTransparency) { tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h); }
            m_enabled = drawImage(m_PicturePath.c_get(), x, y, w, h);
            if (m_saveBackground) { tft.copyFramebuffer(0, 1, m_x, m_y, m_w, m_h); }
            return m_enabled;
        }
    }
    void hide() {
        if (m_saveBackground) {
            tft.copyFramebuffer(2, 1, m_x, m_y, m_w, m_h); // restore background
        }
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() {
        if (m_saveBackground) {
            tft.copyFramebuffer(2, 1, m_x, m_y, m_w, m_h); // restore background
        }
        m_enabled = false;
    }
    void enable() { m_enabled = true; }

    void setPicturePath(ps_ptr<char> path) {
        m_PicturePath = path;
        imgSize img = GetImageSize(path);
        m_image_w = img.w;
        m_image_h = img.h;
    }
    void setAlternativPicturePath(ps_ptr<char> path) {
        m_altPicturePath = path;
    }

    void align(bool h, bool v){
        if(h){ m_padding_left = 0; m_padding_right = 0; m_image_x = (m_w - m_image_w) / 2; }
        else m_image_x = 0;
        if(v){ m_padding_top = 0; m_padding_bottom = 0; m_image_y = (m_h - m_image_h) / 2; }
        else m_image_y = 0;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock24 : public RegisterTable { // draw a clock in 24h format
  private:
    pictureBox* pic_clock24_digitsH10 = new pictureBox("clock24_digitsH10");     // digits hour   * 10
    pictureBox* pic_clock24_digitsH01 = new pictureBox("clock24_digitsH01");     // digits hour   * 01
    pictureBox* pic_clock24_digitsM10 = new pictureBox("clock24_digitsM10");     // digits minute * 10
    pictureBox* pic_clock24_digitsM01 = new pictureBox("clock24_digitsM01");     // digits minute * 01
    pictureBox* pic_clock24_digitsColon = new pictureBox("clock24_digitsColon"); // digits colon
    int16_t     m_x = 0;
    int16_t     m_y = 0;
    int16_t     m_w = 0;
    int16_t     m_h = 0;
    struct pos {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_h10, m_h01, m_c, m_m10, m_m01;

    uint32_t     m_bgColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_state = false;
    bool         m_backgroundTransparency = false;
    bool         m_showAll = false;
    ps_ptr<char> m_name;
    char*        m_pathBuff = NULL;
    uint8_t      m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg  m_ra;

  public:
    imgClock24(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock24() {
        x_ps_free(&m_pathBuff);
        delete pic_clock24_digitsH10;
        delete pic_clock24_digitsH01;
        delete pic_clock24_digitsColon;
        delete pic_clock24_digitsM10;
        delete pic_clock24_digitsM01;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        placingDigits(m_w, m_h);
        m_enabled = false;
        pic_clock24_digitsH10->begin(m_x + m_h10.x, m_y + m_h10.y, m_h10.w, m_h10.h, m_h10.pl, m_h10.pr, m_h10.pt, m_h10.pb);
        pic_clock24_digitsH01->begin(m_x + m_h01.x, m_y + m_h01.y, m_h01.w, m_h01.h, m_h01.pl, m_h01.pr, m_h01.pt, m_h01.pb);
        pic_clock24_digitsColon->begin(m_x + m_c.x, m_y + m_c.y,   m_c.w,   m_c.h,   m_c.pl,   m_c.pr,   m_c.pt,   m_c.pb);
        pic_clock24_digitsM10->begin(m_x + m_m10.x, m_y + m_m10.y, m_m10.w, m_m10.h, m_m10.pl, m_m10.pr, m_m10.pt, m_m10.pb);
        pic_clock24_digitsM01->begin(m_x + m_m01.x, m_y + m_m01.y, m_m01.w, m_m10.h, m_m01.pl, m_m01.pr, m_m01.pt, m_m01.pb);
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() {
        m_enabled = false;
        m_showAll = false;
    }
    bool isDisabled() { return !m_enabled; }
    bool enable() { return m_enabled = true; }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday) {
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_hour = minuteOfTheDay / 60;
        m_min = minuteOfTheDay % 60;
        m_weekday = weekday;
        if (m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t m_min) {
        static uint8_t oldTime[4];
        static bool    k = false;
        uint8_t        time[5];
        time[0] = m_hour / 10;
        time[1] = m_hour % 10;
        time[2] = m_min / 10;
        time[3] = m_min % 10;

        for (uint8_t i = 0; i < 4; i++) {
            if ((time[i] != oldTime[i]) || m_showAll) {
                sprintf(m_pathBuff, "/digits/l/%igreen.jpg", time[i]);
                if (i == 0) {
                    pic_clock24_digitsH10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_clock24_digitsH01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_clock24_digitsM10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_clock24_digitsM01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            oldTime[i] = time[i];
        }

        k = !k;
        if (k) {
            pic_clock24_digitsColon->setPicturePath("/digits/l/cgreen.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        } else {
            pic_clock24_digitsColon->setPicturePath("/digits/l/cgreen_dk.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        }
        m_showAll = false;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        m_clicked = false;
        return true;
    }
private:
    void placingDigits(uint16_t w, uint16_t h){
        uint16_t digits_y = 0, digits_w = 0, colon_w = 0, digits_h = 0, paddig_l = 0;

        imgSize img = GetImageSize("/digits/l/0green.jpg"); // get size of digit '0'
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get digit size");
            return;
        }
        MWR_LOG_DEBUG("digits w = %i, h = %i", img.w, img.h);
        digits_w = img.w;
        digits_h = img.h;

        img = GetImageSize("/digits/l/cgreen.jpg"); // get size of colon
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get colon size");
            return;
        }
        MWR_LOG_DEBUG("colon w = %i, h = %i", img.w, img.h);
        colon_w = img.w;
        digits_y = (h - digits_h) / 2;
        paddig_l = (w - (4 * digits_w + colon_w)) / 2;
        m_h10.x = paddig_l;
        m_h10.y = digits_y;
        m_h10.w = digits_w;
        m_h10.h = digits_h;
        m_h01.x = m_h10.x + digits_w;
        m_h01.y = digits_y;
        m_h01.w = digits_w;
        m_h01.h = digits_h;
        m_c.x   = m_h01.x + digits_w;
        m_c.y   = digits_y;
        m_c.w   = colon_w;
        m_c.h   = digits_h;
        m_m10.x = m_c.x + colon_w;
        m_m10.y = digits_y;
        m_m10.w = digits_w;
        m_m10.h = digits_h;
        m_m01.x = m_m10.x + digits_w;
        m_m01.y = digits_y;
        m_m01.w = digits_w;
        m_m01.h = digits_h;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class imgClock24small : public RegisterTable { // draw a clock in 24h format
  private:
    pictureBox*  pic_clock24_digitsH10 = new pictureBox("clock24_digitsH10");     // digits hour   * 10
    pictureBox*  pic_clock24_digitsH01 = new pictureBox("clock24_digitsH01");     // digits hour   * 01
    pictureBox*  pic_clock24_digitsM10 = new pictureBox("clock24_digitsM10");     // digits minute * 10
    pictureBox*  pic_clock24_digitsM01 = new pictureBox("clock24_digitsM01");     // digits minute * 01
    pictureBox*  pic_clock24_digitsColon = new pictureBox("clock24_digitsColon"); // digits colon
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint32_t     m_bgColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_state = false;
    bool         m_backgroundTransparency = false;
    bool         m_showAll = false;
    ps_ptr<char> m_name = NULL;
    char*        m_pathBuff = NULL;
    uint8_t      m_min = 0, m_hour = 0, m_weekday = 0;
    releasedArg  m_ra;

    struct pos {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } m_h10, m_h01, m_c, m_m10, m_m01;

  public:
    imgClock24small(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
    }
    ~imgClock24small() {
        x_ps_free(&m_pathBuff);
        delete pic_clock24_digitsH10;
        delete pic_clock24_digitsH01;
        delete pic_clock24_digitsColon;
        delete pic_clock24_digitsM10;
        delete pic_clock24_digitsM01;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        placingDigits(m_w, m_h);
        m_enabled = false;
        pic_clock24_digitsH10->begin(m_x + m_h10.x, m_y + m_h10.y, m_h10.w, m_h10.h, m_h10.pl, m_h10.pr, m_h10.pt, m_h10.pb);
        pic_clock24_digitsH01->begin(m_x + m_h01.x, m_y + m_h01.y, m_h01.w, m_h01.h, m_h01.pl, m_h01.pr, m_h01.pt, m_h01.pb);
        pic_clock24_digitsColon->begin(m_x + m_c.x, m_y + m_c.y,   m_c.w,   m_c.h,   m_c.pl,   m_c.pr,   m_c.pt,   m_c.pb);
        pic_clock24_digitsM10->begin(m_x + m_m10.x, m_y + m_m10.y, m_m10.w, m_m10.h, m_m10.pl, m_m10.pr, m_m10.pt, m_m10.pb);
        pic_clock24_digitsM01->begin(m_x + m_m01.x, m_y + m_m01.y, m_m01.w, m_m10.h, m_m01.pl, m_m01.pr, m_m01.pt, m_m01.pb);
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        writeTime(m_hour, m_min);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() {
        m_enabled = false;
        m_showAll = false;
    }
    bool isDisabled() { return !m_enabled; }
    bool enable() { return m_enabled = true; }
    void updateTime(uint16_t minuteOfTheDay, uint8_t weekday) {
        // minuteOfTheDay counts at 00:00, from 0...23*60+59
        // weekDay So - 0, Mo - 1 ... Sa - 6
        m_hour = minuteOfTheDay / 60;
        m_min = minuteOfTheDay % 60;
        m_weekday = weekday;
        if (m_enabled) writeTime(m_hour, m_min);
    }
    void writeTime(uint8_t m_hour, uint8_t m_min) {
        static uint8_t oldTime[4];
        static bool    k = false;
        uint8_t        time[5];
        time[0] = m_hour / 10;
        time[1] = m_hour % 10;
        time[2] = m_min / 10;
        time[3] = m_min % 10;

        for (uint8_t i = 0; i < 4; i++) {
            if ((time[i] != oldTime[i]) || m_showAll) {
                sprintf(m_pathBuff, "/digits/s/%ired.jpg", time[i]);
                if (i == 0) {
                    pic_clock24_digitsH10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_clock24_digitsH01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_clock24_digitsM10->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_clock24_digitsM01->setPicturePath(m_pathBuff);
                    pic_clock24_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            oldTime[i] = time[i];
        }

        k = !k;
        if (k) {
            pic_clock24_digitsColon->setPicturePath("/digits/s/cred.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        } else {
            pic_clock24_digitsColon->setPicturePath("/digits/s/cred_dk.jpg");
            pic_clock24_digitsColon->show(m_backgroundTransparency, false);
        }
        m_showAll = false;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        //    if(!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        m_clicked = false;
        return true;
    }
private:
    void placingDigits(uint16_t w, uint16_t h){
        uint16_t digits_y = 0, digits_w = 0, colon_w = 0, digits_h = 0, paddig_l = 0;

        imgSize img = GetImageSize("/digits/s/0green.jpg"); // get size of digit '0'
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get digit size");
            return;
        }
        MWR_LOG_DEBUG("digits w = %i, h = %i", img.w, img.h);
        digits_w = img.w;
        digits_h = img.h;

        img = GetImageSize("/digits/s/cgreen.jpg"); // get size of colon
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get colon size");
            return;
        }
        MWR_LOG_DEBUG("colon w = %i, h = %i", img.w, img.h);
        colon_w = img.w;
        digits_y = (h - digits_h) / 2;
        paddig_l = (w - (4 * digits_w + colon_w)) / 2;
        m_h10.x = paddig_l;
        m_h10.y = digits_y;
        m_h10.w = digits_w;
        m_h10.h = digits_h;
        m_h01.x = m_h10.x + digits_w;
        m_h01.y = digits_y;
        m_h01.w = digits_w;
        m_h01.h = digits_h;
        m_c.x   = m_h01.x + digits_w;
        m_c.y   = digits_y;
        m_c.w   = colon_w;
        m_c.h   = digits_h;
        m_m10.x = m_c.x + colon_w;
        m_m10.y = digits_y;
        m_m10.w = digits_w;
        m_m10.h = digits_h;
        m_m01.x = m_m10.x + digits_w;
        m_m01.y = digits_y;
        m_m01.w = digits_w;
        m_m01.h = digits_h;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class alarmClock : public RegisterTable { // draw a clock in 12 or 24h format
  private:
    pictureBox* pic_alarm_digitsH10 = new pictureBox("alarm_digitsH10");     // digits hour   * 10
    pictureBox* pic_alarm_digitsH01 = new pictureBox("alarm_digitsH01");     // digits hour   * 01
    pictureBox* pic_alarm_digitsM10 = new pictureBox("alarm_digitsM10");     // digits minute * 10
    pictureBox* pic_alarm_digitsM01 = new pictureBox("alarm_digitsM01");     // digits minute * 01
    pictureBox* pic_alarm_digitsColon = new pictureBox("alarm_digitsColon"); // digits colon
    textbox*    txt_alarm_days = new textbox[7]{textbox("txt_alarm_days0"), textbox("txt_alarm_days1"), textbox("txt_alarm_days2"), textbox("txt_alarm_days3"),
                                                textbox("txt_alarm_days4"), textbox("txt_alarm_days5"), textbox("txt_alarm_days6")}; // days of the week
    textbox*    txt_alarm_time = new textbox[7]{textbox("txt_alarm_time0"), textbox("txt_alarm_time1"), textbox("txt_alarm_time2"), textbox("txt_alarm_time3"),
                                                textbox("txt_alarm_time4"), textbox("txt_alarm_time5"), textbox("txt_alarm_time6")}; // time of the day

    int16_t  m_x = 0;
    int16_t  m_y = 0;
    int16_t  m_w = 0;
    int16_t  m_h = 0;
    struct pos {
        uint16_t x;
        uint16_t y;
        uint16_t w;
        uint16_t h;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } s_h10, s_h01, s_c, s_m10, s_m01;

    uint16_t     m_alarmdaysXPos[7] = {0};
    uint16_t     m_alarmdaysYPos = 0;
    uint16_t     m_alarmtimeYPos = 0;
    uint16_t     m_alarmdaysYoffset = 0;
    uint16_t     m_alarmdaysW = 0;
    uint16_t     m_alarmdaysH = 0;
    uint16_t     m_fontSize   = 0; // auto
    uint32_t     m_bgColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_state = false;
    bool         m_showAll = false;
    bool         m_backgroundTransparency = false;
    ps_ptr<char> m_name;
    ps_ptr<char> m_pathBuff;
    uint8_t*     m_alarmDays = NULL;
    int16_t*     m_alarmTime = NULL;
    uint8_t      m_min = 0, m_hour = 0, m_weekday = 0;
    int8_t       m_btnAlarmDay = -1;
    int8_t       m_btnAlarmTime = -1;
    int8_t       m_idx = 0;
    uint8_t      m_alarmDigits[4] = {0};
    const char*  m_p1 = "/digits/m/"; // path
    uint8_t      m_p1Len = 21;
    const char   m_WD[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    releasedArg  m_ra;

  public:
    alarmClock(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
    }
    ~alarmClock() {
        delete pic_alarm_digitsH10;
        delete pic_alarm_digitsH01;
        delete pic_alarm_digitsColon;
        delete pic_alarm_digitsM10;
        delete pic_alarm_digitsM01;
        delete[] txt_alarm_days;
        delete[] txt_alarm_time;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        placingDigits(w, h);
        m_enabled = false;
        m_alarmdaysYPos = m_y + m_alarmdaysYoffset; // m_y;
        m_alarmtimeYPos = m_alarmdaysYPos + m_alarmdaysH + 1;
        pic_alarm_digitsH10->begin(m_x + s_h10.x, m_y + s_h10.y, s_h10.w, s_h10.h, s_h10.pl, s_h10.pr, s_h10.pt, s_h10.pb);
        pic_alarm_digitsH01->begin(m_x + s_h01.x, m_y + s_h01.y, s_h01.w, s_h01.h, s_h01.pl, s_h01.pr, s_h01.pt, s_h01.pb);
        pic_alarm_digitsColon->begin(m_x + s_c.x, m_y + s_c.y,   s_c.w, s_c.h, s_c.pl, s_c.pr, s_c.pt, s_c.pb);
        pic_alarm_digitsM10->begin(m_x + s_m10.x, m_y + s_m10.y, s_m10.w, s_m10.h, s_m10.pl, s_m10.pr, s_m10.pt, s_m10.pb);
        pic_alarm_digitsM01->begin(m_x + s_m01.x, m_y + s_m01.y, s_m01.w, s_m10.h, s_m01.pl, s_m01.pr, s_m01.pt, s_m01.pb);


        for (uint8_t i = 0; i < 7; i++) {
            txt_alarm_days[i].begin(m_alarmdaysXPos[i], m_alarmdaysYPos, m_alarmdaysW, m_alarmdaysH, 0, 0, 0, 0);
            txt_alarm_days[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_alarm_days[i].setBorderWidth(1);
            txt_alarm_days[i].setFont(m_fontSize);
            txt_alarm_days[i].setText(m_WD[i]);
            txt_alarm_time[i].begin(m_alarmdaysXPos[i], m_alarmtimeYPos, m_alarmdaysW, m_alarmdaysH, 0, 0, 0, 0);
            txt_alarm_time[i].setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
            txt_alarm_time[i].setBorderWidth(1);
            txt_alarm_time[i].setFont(m_fontSize);
        }
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool inactive = false) {
        m_clicked = false;
        if (inactive) {
            //    setInactive();
            return;
        }
        m_enabled = true;
        m_showAll = true;
        updateDigits();
        updateAlarmDaysAndTime();
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    void shiftRight() {
        m_idx++;
        if (m_idx == 4) m_idx = 0;
        m_showAll = true;
        updateDigits();
    }
    void shiftLeft() {
        m_idx--;
        if (m_idx == -1) m_idx = 0;
        m_showAll = true;
        updateDigits();
    }
    void digitUp() {
        if (m_idx == 0) { // 10h
            if (m_alarmDigits[0] == 2) return;
            if (m_alarmDigits[0] == 1 && m_alarmDigits[1] > 3) return;
            m_alarmDigits[0]++;
        }
        if (m_idx == 1) { // 1h
            if (m_alarmDigits[0] == 2 && m_alarmDigits[1] == 3) return;
            if (m_alarmDigits[1] == 9) return;
            m_alarmDigits[1]++;
        }
        if (m_idx == 2) { // 10m
            if (m_alarmDigits[2] == 5) return;
            m_alarmDigits[2]++;
        }
        if (m_idx == 3) { // 1m
            if (m_alarmDigits[3] == 9) return;
            m_alarmDigits[3]++;
        }
        m_showAll = true;
        updateDigits();
    }
    void digitDown() {
        if (m_idx == 0) { // 10h
            if (m_alarmDigits[0] == 0) return;
            m_alarmDigits[0]--;
        }
        if (m_idx == 1) { // 1h
            if (m_alarmDigits[1] == 0) return;
            m_alarmDigits[1]--;
        }
        if (m_idx == 2) { // 10m
            if (m_alarmDigits[2] == 0) return;
            m_alarmDigits[2]--;
        }
        if (m_idx == 3) { // 1m
            if (m_alarmDigits[3] == 0) return;
            m_alarmDigits[3]--;
        }
        m_showAll = true;
        updateDigits();
    }
    void alarm_time_and_days(uint8_t* alarmDays, int16_t alarmTime[7]) {
        m_alarmTime = alarmTime;
        m_alarmDays = alarmDays;
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        for (int i = 0; i < 7; i++) {
            if (txt_alarm_days[i].positionXY(x, y)) m_btnAlarmDay = i;
            if (txt_alarm_time[i].positionXY(x, y)) m_btnAlarmTime = i;
        }
        if (m_btnAlarmDay >= 0) alarmDaysPressed(m_btnAlarmDay);
        if (m_btnAlarmTime >= 0) alarmTimePressed(m_btnAlarmTime);
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        char hhmm[10] = "00:00";
        if (m_btnAlarmDay >= 0) { sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmDay] / 60, m_alarmTime[m_btnAlarmDay] % 60); }
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);

        if (m_btnAlarmDay >= 0) {
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmDay;
            *m_alarmDays ^= mask;      // toggle the bit
            if (*m_alarmDays & mask) { // is set
                txt_alarm_days[m_btnAlarmDay].setBorderColor(TFT_RED);
                txt_alarm_days[m_btnAlarmDay].setTextColor(TFT_RED);
                txt_alarm_days[m_btnAlarmDay].setText(m_WD[m_btnAlarmDay]);
                txt_alarm_days[m_btnAlarmDay].show(m_backgroundTransparency, false);
                txt_alarm_time[m_btnAlarmDay].setBorderColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmDay].setTextColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmDay].setText(hhmm);
                txt_alarm_time[m_btnAlarmDay].show(m_backgroundTransparency, false);
            } else { // bit is not set
                txt_alarm_days[m_btnAlarmDay].setBorderColor(TFT_DARKGREY);
                txt_alarm_days[m_btnAlarmDay].setTextColor(TFT_DARKGREY);
                txt_alarm_days[m_btnAlarmDay].setText(m_WD[m_btnAlarmDay]);
                txt_alarm_days[m_btnAlarmDay].show(m_backgroundTransparency, false);
                txt_alarm_time[m_btnAlarmDay].setBorderColor(TFT_DARKGREY);
                txt_alarm_time[m_btnAlarmDay].setTextColor(TFT_DARKGREY);
                txt_alarm_time[m_btnAlarmDay].setText("");
                txt_alarm_time[m_btnAlarmDay].show(m_backgroundTransparency, false);
            }
            m_btnAlarmDay = -1;
        }
        if (m_btnAlarmTime >= 0) {
            uint8_t mask = 0b00000001;
            mask <<= m_btnAlarmTime;
            if (mask & *m_alarmDays) { // bit is set -> alarm is active for that day
                txt_alarm_time[m_btnAlarmTime].setBorderColor(TFT_GREEN);
                txt_alarm_time[m_btnAlarmTime].setTextColor(TFT_GREEN);
                m_alarmTime[m_btnAlarmTime] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60 + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
                char hhmm[10] = "00:00";
                sprintf(hhmm, "%02d:%02d", m_alarmTime[m_btnAlarmTime] / 60, m_alarmTime[m_btnAlarmTime] % 60);
                txt_alarm_time[m_btnAlarmTime].setText(hhmm);
                txt_alarm_time[m_btnAlarmTime].show(m_backgroundTransparency, false);
            }
            m_btnAlarmTime = -1;
        }
        m_clicked = false;
        return true;
    }

  private:
    void updateDigits() {
        static uint8_t m_oldAlarmDigits[4] = {0};
        for (uint8_t i = 0; i < 4; i++) {
            if (m_oldAlarmDigits[i] != m_alarmDigits[i] || m_showAll) {
                m_pathBuff.assignf("%s%i", m_p1, m_alarmDigits[i]);

                if (i == m_idx) {
                    m_pathBuff.append("orange.jpg");
                } else {
                    m_pathBuff.append("red.jpg");
                }

                if (i == 0) {
                    pic_alarm_digitsH10->setPicturePath(m_pathBuff.c_get());
                    pic_alarm_digitsH10->show(m_backgroundTransparency, false);
                }
                if (i == 1) {
                    pic_alarm_digitsH01->setPicturePath(m_pathBuff.c_get());
                    pic_alarm_digitsH01->show(m_backgroundTransparency, false);
                }
                if (i == 2) {
                    pic_alarm_digitsM10->setPicturePath(m_pathBuff.c_get());
                    pic_alarm_digitsM10->show(m_backgroundTransparency, false);
                }
                if (i == 3) {
                    pic_alarm_digitsM01->setPicturePath(m_pathBuff.c_get());
                    pic_alarm_digitsM01->show(m_backgroundTransparency, false);
                }
            }
            m_oldAlarmDigits[i] = m_alarmDigits[i];
        }
        if (m_showAll) {
            pic_alarm_digitsColon->setPicturePath("/digits/m/cred.jpg");
            pic_alarm_digitsColon->show(m_backgroundTransparency, false);
        }
    }
    void updateAlarmDaysAndTime() {
        uint8_t  mask = 0b00000001;
        uint16_t color = TFT_BLACK;

        for (int i = 0; i < 7; i++) {
            // alarmDays
            if (*m_alarmDays & mask)
                color = TFT_RED;
            else
                color = TFT_DARKGREY;
            txt_alarm_days[i].setBorderColor(color);
            txt_alarm_days[i].setTextColor(color);
            txt_alarm_days[i].setText(m_WD[i]);
            txt_alarm_days[i].show(m_backgroundTransparency, false);
            char hhmm[10] = "00:00";
            sprintf(hhmm, "%02d:%02d", m_alarmTime[i] / 60, m_alarmTime[i] % 60);
            if (*m_alarmDays & mask) {
                txt_alarm_time[i].setBorderColor(TFT_GREEN);
                txt_alarm_time[i].setTextColor(TFT_GREEN);
                txt_alarm_time[i].setText(hhmm);
            } else {
                txt_alarm_time[i].setBorderColor(TFT_DARKGREY);
                txt_alarm_time[i].setTextColor(TFT_DARKGREY);
                txt_alarm_time[i].setText("");
            }
            txt_alarm_time[i].show(m_backgroundTransparency, false);
            mask <<= 1;
        }
    }

    void alarmDaysPressed(uint8_t idx) {
        txt_alarm_days[idx].setBorderColor(TFT_YELLOW);
        txt_alarm_days[idx].setTextColor(TFT_YELLOW);
        txt_alarm_days[idx].setText(m_WD[idx]);
        txt_alarm_days[idx].show(m_backgroundTransparency, false);
    }
    void alarmTimePressed(uint8_t idx) {
        uint8_t mask = 0b00000001;
        mask <<= idx;
        if (mask & *m_alarmDays) { // bit is set -> active
            m_alarmTime[idx] = (m_alarmDigits[0] * 10 + m_alarmDigits[1]) * 60 + (m_alarmDigits[2] * 10 + m_alarmDigits[3]);
            char hhmm[10] = {0};
            sprintf(hhmm, "%02d:%02d", m_alarmTime[idx] / 60, m_alarmTime[idx] % 60);
            txt_alarm_time[idx].setBorderColor(TFT_YELLOW);
            txt_alarm_time[idx].setTextColor(TFT_YELLOW);
            txt_alarm_time[idx].setText(hhmm);
            tft.setTextColor(TFT_YELLOW);
            txt_alarm_time[idx].show(m_backgroundTransparency, false);
        }
    }
    void placingDigits(uint16_t w, uint16_t h){
        uint16_t digits_y = 0, digits_w = 0, colon_w = 0, digits_h = 0, digits_paddig_l = 0, alarmdays_padding_l = 0;
        uint16_t h4 = h / 4; // [1/4 days, time + 3/4 digits]

       imgSize img = GetImageSize("/digits/m/0green.jpg"); // get size of digit '0'
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get digit size");
            return;
        }
        MWR_LOG_DEBUG("digits w = %i, h = %i", img.w, img.h);
        digits_w = img.w;
        digits_h = img.h;

        img = GetImageSize("/digits/m/cred.jpg"); // get size of colon
        if(img.w == 0 || img.h == 0){
            MWR_LOG_ERROR("cannot get colon size");
            return;
        }
        colon_w = img.w;

        digits_y = (3 * h4 - digits_h) / 2 + h4;
        digits_paddig_l = (w - (4 * digits_w + colon_w)) / 2;

        m_alarmdaysW = w / 8;
        alarmdays_padding_l = m_alarmdaysW / 2;
        for(int i = 0; i < 7; i++) {m_alarmdaysXPos[i] = alarmdays_padding_l + i * m_alarmdaysW;}
        m_alarmdaysYoffset = 2;
        m_alarmdaysH = h4 / 2;

        s_h10.x = digits_paddig_l;
        s_h10.y = digits_y;
        s_h10.w = digits_w;
        s_h10.h = digits_h;
        s_h01.x = s_h10.x + digits_w;
        s_h01.y = digits_y;
        s_h01.w = digits_w;
        s_h01.h = digits_h;
        s_c.x   = s_h01.x + digits_w;
        s_c.y   = digits_y;
        s_c.w   = digits_w;
        s_c.h   = digits_h;
        s_m10.x = s_c.x + colon_w;
        s_m10.y = digits_y;
        s_m10.w = digits_w;
        s_m10.h = digits_h;
        s_m01.x = s_m10.x + digits_w;
        s_m01.y = digits_y;
        s_m01.w = digits_w;
        s_m01.h = digits_h;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class uniList {

  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int32_t      m_nr[10] = {0};
    uint8_t      m_fontSize = 0;
    uint8_t      m_tftSize = 0;
    uint8_t      m_lineHight = 0;
    uint8_t      m_mode = 0;
    uint32_t     m_bgColor = 0;
    uint8_t      m_indentContent = 0;
    uint8_t      m_indentDirectory = 0;
    ps_ptr<char> m_name;
    char*        m_buff = NULL;
    char*        m_txt[10] = {0};
    char*        m_ext1[10] = {0};
    char*        m_ext2[10] = {0};
    bool         m_enabled = false;

  public:
    uniList(const char* name) {
        m_name = name;
        m_bgColor = TFT_BLACK;
    }
    ~uniList() {
        x_ps_free(&m_buff);
        for (int i = 0; i < 10; i++) {
            x_ps_free(&m_txt[i]);
            x_ps_free(&m_ext1[i]);
            x_ps_free(&m_ext2[i]);
        }
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t fontSize) {
        m_x = x;               // x pos
        m_y = y;               // y pos
        m_w = w;               // width
        m_h = h;               // high
        m_fontSize = fontSize; // default font size
        m_enabled = false;
        m_lineHight = m_h / 10;
        m_buff = x_ps_malloc(1024);
    }
    void setMode(uint8_t mode, const char*  tftSize, uint8_t fontSize) {
        if (mode == RADIO) { m_mode = RADIO; }
        if (mode == PLAYER) { m_mode = PLAYER; }
        if (mode == DLNA) { m_mode = DLNA; }
        m_fontSize = fontSize;
        if (strcmp(tftSize, "s") == 0) m_tftSize = 1;
        if (strcmp(tftSize, "m") == 0) m_tftSize = 2;
        if (strcmp(tftSize, "l") == 0) m_tftSize = 3;
        if (strcmp(tftSize, "xl") == 0) m_tftSize = 4;
        switch (m_tftSize) {
            case 1:
                if (m_mode == RADIO) {
                    m_indentDirectory = 15;
                    m_indentContent = 15;
                } // 320x240
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 15;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 15;
                }
                break;
            case 2:
                if (m_mode == RADIO) {
                    m_indentDirectory = 20;
                    m_indentContent = 20;
                } // 480x320
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 20;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 20;
                }
                break;
            case 3:
                if (m_mode == RADIO) {
                    m_indentDirectory = 30;
                    m_indentContent = 30;
                } // 800x480
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                break;
            case 4:
                if (m_mode == RADIO) {
                    m_indentDirectory = 30;
                    m_indentContent = 30;
                } // 1024x600
                if (m_mode == DLNA) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                if (m_mode == PLAYER) {
                    m_indentDirectory = 10;
                    m_indentContent = 30;
                }
                break;
        }
    }
    void clearList() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        for (int i = 0; i < 10; i++) {
            x_ps_free(&m_txt[i]);
            x_ps_free(&m_ext1[i]);
            x_ps_free(&m_ext2[i]);
            m_nr[i] = -1;
        }
    }
    void drawLine(uint8_t pos, const char* txt, const char* ext1 = NULL, const char* ext2 = NULL, const char* color = ANSI_ESC_WHITE, int32_t nr = -1) {
        if (pos > 9) return;
        if (!txt) return;
        tft.setFont(m_fontSize);
        if (m_mode == RADIO) {
            sprintf(m_buff, ANSI_ESC_YELLOW "%03li %s%s", nr, color, txt);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
            }
            if (ext1) {
                x_ps_free(&m_ext1[pos]);
                m_ext1[pos] = strdup(ext1);
            }
            if (ext2) {
                x_ps_free(&m_ext2[pos]);
                m_ext2[pos] = strdup(ext2);
            }
            m_nr[pos] = nr;
        }
        if (m_mode == DLNA) {
            if (!txt) {
                MWR_LOG_WARN("txt is NULL");
                return;
            }
            if (!ext1)
                sprintf(m_buff, "%s%s", color, txt);
            else if (ext1[0] == '\0')
                sprintf(m_buff, "%s%s", color, txt);
            else
                sprintf(m_buff, "%s%s " ANSI_ESC_CYAN "(%s)", color, txt, ext1);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
                m_nr[pos] = 1;
            }
            if (ext1) {
                x_ps_free(&m_ext1[pos]);
                m_ext1[pos] = strdup(ext1);
            }
            if (ext2) {
                x_ps_free(&m_ext2[pos]);
                m_ext2[pos] = strdup(ext2);
            }
        }
        if (m_mode == PLAYER) {
            if (!txt) {
                MWR_LOG_WARN("txt is NULL");
                return;
            }
            if (nr <= 0)
                sprintf(m_buff, "%s%s", color, txt);
            else
                sprintf(m_buff, "%s%s" ANSI_ESC_YELLOW " %li", color, txt, nr);
            if (txt) {
                x_ps_free(&m_txt[pos]);
                m_txt[pos] = strdup(txt);
                m_nr[pos] = nr;
            }
        }
        tft.writeText(m_buff, pos ? m_indentContent : m_indentDirectory, m_y + pos * m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
    }
    void drawPosInfo(int16_t firstVal, int16_t secondVal, int16_t total, const char* color) { // e.g. 1-9/65
        sprintf(m_buff, "%s%i-%i-%i", color, firstVal, secondVal, total);
        tft.writeText(m_buff, 0, m_y, m_w, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true, false);
    }
    void colourLine(uint8_t pos, const char* color = ANSI_ESC_WHITE) {
        if (pos > 9) return;
        tft.setFont(m_fontSize);
        if (m_mode == RADIO) { sprintf(m_buff, ANSI_ESC_YELLOW "%03li %s%s", m_nr[pos], color, m_txt[pos]); }
        if (m_mode == PLAYER) {
            if (m_nr[pos])
                sprintf(m_buff, "%s%s" ANSI_ESC_YELLOW " %li", color, m_txt[pos], m_nr[pos]); // file
            else
                sprintf(m_buff, "%s%s", color, m_txt[pos]); // directory
        }
        tft.writeText(m_buff, pos ? m_indentContent : m_indentDirectory, m_y + pos * m_lineHight, m_w - 10, m_lineHight, TFT_ALIGN_LEFT, TFT_ALIGN_CENTER, true, true);
    }
    const char* getTxtByPos(uint8_t pos) { return m_txt[pos]; }
    int16_t     getNumberByPos(uint8_t pos) { return m_nr[pos]; }
    void        drawTriangeUp() {
        auto triangleUp = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + s, y + 0, x + 0, y + 2 * s, x + 2 * s, y + 2 * s, TFT_RED); };
        int  line = 1;
        if (m_mode == RADIO) line = 0;
        triangleUp(0, m_y + (line * m_lineHight), m_lineHight / 3.5);
    }
    void drawTriangeDown() {
        auto triangleDown = [&](int16_t x, int16_t y, uint8_t s) { tft.fillTriangle(x + 0, y + 0, x + 2 * s, y + 0, x + s, y + 2 * s, TFT_RED); };
        triangleDown(0, m_y + (9 * m_lineHight), m_lineHight / 3.5);
    }
};
extern uniList myList;
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
  ———————————————————————————————————————————————————————
  | DLNA List                       Vol3    01:10:32    |           m_itemsListPos
  | Musik                                      1-9/6    |           <-- 0
  |   Videos(7)                                         |           <-- 1
  |   Interpreten(2)                                    |           <-- 2
  |   Alben                                             |           <-- 3
  |   Alle Titel(7)                                     |           <-- 4
  |   Genres                                            |           <-- 5
  |   Ordner(1)                                         |           <-- 6
  |   Wiedergabelisten                                  |           <-- 7
  |   Filme(23)                                         |           <-- 8
  |                                                     |           <-- 9
  | 003   0:00    128K              IP:192.168.178.24   |
  ———————————————————————————————————————————————————————
*/

class dlnaList : public RegisterTable {

  private:
    int16_t                                    m_x = 0;
    int16_t                                    m_y = 0;
    int16_t                                    m_w = 0;
    int16_t                                    m_h = 0;
    int16_t                                    m_oldX = 0;
    int16_t                                    m_oldY = 0;
    uint8_t*                                   m_dlnaLevel;
    uint8_t                                    m_fontSize = 0;
    uint8_t                                    m_lineHight = 0;
    uint8_t                                    m_browseOnRelease = 0;
    uint8_t                                    m_itemListPos = 0;
    uint8_t                                    m_itemListPos_last = 0;
    int8_t                                     m_currDLNAsrvNr = -1;
    int16_t                                    m_currItemNr[10] = {0};
    int16_t                                    m_viewPoint = 0;
    uint16_t                                   m_dlnaMaxItems = 0;
    uint32_t                                   m_bgColor = 0;
    bool                                       m_enabled = false;
    bool                                       m_clicked = false;
    bool                                       m_state = false;
    bool                                       m_isAudio = false;
    bool                                       m_isURL = false;
    char*                                      m_name = NULL;
    char*                                      m_pathBuff = NULL;
    const char*                                m_chptr = NULL;
    char*                                      m_buff = NULL;
    const char*                                m_tftSize = "";
    const std::deque<DLNA_Client::dlnaServer>* m_dlnaServer;
    const std::deque<DLNA_Client ::srvItem>*   m_srvContent;
    DLNA_Client*                               m_dlna;
    dlnaHistory_s*                             m_dlnaHistory = NULL;
    releasedArg                                m_ra;
    enum DLNA_Action { DLNA_NONE = 0, DLNA_FILE = 1, DLNA_SERVERLIST = 2, DLNA_PREV_LEVEL = 3, DLNA_NEXT_LEVEL = 4, DLNA_WIPE = 5};

  public:
    dlnaList(const char* name) {
        register_object(this);
        if (name)
            m_name = x_ps_strdup(name);
        else
            m_name = x_ps_strdup("dlnaList");
        m_buff = x_ps_malloc(512);
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        m_ra.arg1 = "";
        m_ra.arg2 = "";
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~dlnaList() {
        x_ps_free(&m_name);
        x_ps_free(&m_buff);
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_fontSize = fontSize;
        m_enabled = false;
        m_lineHight = m_h / 10;
        m_tftSize = tftSize;

    }
    void client_and_history(DLNA_Client* dlna, dlnaHistory_s* dh, uint8_t historySize) {
        m_dlna = dlna;
        m_dlnaHistory = dh;
        for (uint8_t i = 0; i < historySize; i++) {
            m_currItemNr[i] = 0;
            m_dlnaHistory[i].objId = "0";
            m_dlnaHistory[i].name = "";
            m_dlnaHistory[i].maxItems = 9;
            m_dlnaHistory[i].childCount = 0;
        }
        m_dlnaHistory[0].name = "Media Server";
    }
    ps_ptr<char> getName() { return m_name; }

    bool isEnabled() { return m_enabled; }

    void show(int8_t number, const std::deque<DLNA_Client::dlnaServer>& dlnaServer, const std::deque<DLNA_Client::srvItem>& srvContent, uint8_t* dlnaLevel, uint16_t maxItems) {
        m_browseOnRelease = DLNA_NONE;
        m_dlnaServer = &dlnaServer;
        m_srvContent = &srvContent;
        m_dlnaLevel = dlnaLevel;
        if (m_dlnaLevel == 0) m_currDLNAsrvNr = number;
        m_clicked = false;
        m_enabled = true;
        m_dlnaMaxItems = maxItems;
        m_dlnaHistory[0].maxItems = m_dlna->getNrOfServers();
        dlnaItemsList();
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }

    bool positionXY(uint16_t x, uint16_t y) { // called every tine if x or y has changed
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_clicked == false) {
            m_oldX = x;
            m_oldY = y;
        }
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick((const char*)m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        hasReleased(x - m_x, y - m_y);
        m_clicked = false;

        if (m_chptr || (m_itemListPos == 0 && (*m_dlnaLevel) > 0)) {
            if(m_itemListPos_last != m_itemListPos) drawItem(m_itemListPos_last, false); // redraw old line, make default color
            bool res = drawItem(m_itemListPos, true); // make cyan or magenta
        //    m_itemListPos_last = m_itemListPos;
            m_chptr = NULL;
            if(res == false) return false;
            vTaskDelay(300);
        }

        if (m_browseOnRelease == DLNA_NONE) { ; } // nothing todo
        if (m_browseOnRelease == DLNA_FILE) { ; } // file
        if (m_browseOnRelease == DLNA_SERVERLIST) { // get serverlist
            (*m_dlnaLevel)++;
            m_dlna->browseServer(m_currDLNAsrvNr, "0", 0, 9);
        }
        if (m_browseOnRelease == DLNA_PREV_LEVEL) { // previous level
            (*m_dlnaLevel)--;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), 0, 9);
        }
        if (m_browseOnRelease == DLNA_NEXT_LEVEL) { // folder, next level
            (*m_dlnaLevel)++;
            if( m_dlnaHistory[*m_dlnaLevel].childCount == 0) return false;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), 0, 9);
        }
        if (m_browseOnRelease == DLNA_WIPE) { m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9); } // scroll up / down

        m_browseOnRelease = DLNA_NONE;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name, m_ra);
        m_ra.val1 = 0;
        return true;
    }

  private:
    void dlnaItemsList() {
        uint8_t pos = 0;
        myList.setMode(DLNA, m_tftSize, m_fontSize);
        myList.clearList();
        myList.drawLine(0, m_dlnaHistory[*m_dlnaLevel].name.c_get(), NULL, ANSI_ESC_ORANGE);
        tft.setTextColor(TFT_WHITE);
        for (pos = 1; pos < 10; pos++) {
            if (pos == 1 && m_viewPoint > 0) { myList.drawTriangeUp(); }
            if (pos == 9 && m_viewPoint + 9 < m_dlnaMaxItems - 1) { myList.drawTriangeDown(); }
            if (*m_dlnaLevel == 0 && pos > m_dlnaServer->size()) { /* log_e("pos too high %i", pos);*/
                break;
            } // guard
            if (*m_dlnaLevel > 0 && pos > m_srvContent->size()) { /* log_e("pos too high %i", pos);*/
                break;
            } // guard
            drawItem(pos);
        }
        sprintf(m_buff, "%i-%i/%i", m_viewPoint + 1, m_viewPoint + (pos - 1), m_dlnaMaxItems); // shows the current items pos e.g. "30-39/210"
        tft.setTextColor(TFT_ORANGE);
        tft.writeText(m_buff, 10, m_y, m_w - 10, m_lineHight, TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER, true, true);
        return;
    }

    bool drawItem(int8_t pos, bool selectedLine = false) { // pos 0 is parent, pos 1...9 are itens, selectedLine means released (ok)

        if (pos < 0 || pos > 9) { MWR_LOG_WARN("pos oor %i", pos); return false; } // guard
        if (*m_dlnaLevel == 0 && pos > m_dlnaServer->size()) { /* log_e("pos too high %i", pos);*/ return false; } // guard
        if (*m_dlnaLevel > 0 && pos > m_srvContent->size()) { /* log_e("pos too high %i", pos);*/  return false; } // guard

        char         extension[15] = {0};
        char         dummy[] = "";
        bool         isAudio = false;
        bool         isURL = false;
        bool         isServer = false;
        bool         res = false;
        const char *item = dummy, *itemURL = dummy; (void)itemURL;
        ps_ptr<char> color = ANSI_ESC_WHITE;
        ps_ptr<char> duration = "?";
        int32_t      itemSize = 0;
        int16_t      childCount = 0;

        if (pos == 0) {
            if (pos + m_viewPoint == m_currItemNr[*m_dlnaLevel] + 1) { color = ANSI_ESC_MAGENTA; }
            else                                                     { color = ANSI_ESC_ORANGE; }
            if (selectedLine)                                        { color = ANSI_ESC_CYAN; res = true; }
            myList.drawLine(pos, m_dlnaHistory[*m_dlnaLevel].name.c_get(), "", "", color.c_get(), 1);
            if(color == ANSI_ESC_MAGENTA) m_itemListPos_last = pos;
            return res;
        }
        myList.drawLine(0, m_dlnaHistory[*m_dlnaLevel].name.c_get(), "", "", ANSI_ESC_ORANGE, 1);
        if (*m_dlnaLevel == 0) { // is list of server
            if (m_dlnaServer->at(pos - 1).friendlyName.c_get()) {
                item = m_dlnaServer->at(pos - 1).friendlyName.c_get();
                isServer = true;
            }
        } else { // is list of folder or file
            if (m_srvContent->at(pos - 1).title.c_get()) {
                item = m_srvContent->at(pos - 1).title.c_get();
                itemSize = m_srvContent->at(pos - 1).itemSize;
                childCount = m_srvContent->at(pos - 1).childCount;
                duration = m_srvContent->at(pos - 1).duration;
            }
            if (startsWith(m_srvContent->at(pos - 1).itemURL.c_get(), "http")) {
                isAudio = m_srvContent->at(pos - 1).isAudio;
                isURL = true;
                itemURL = m_srvContent->at(pos - 1).itemURL.c_get();
            }
        }

        if (isURL && isAudio)                                           { color = ANSI_ESC_YELLOW; } // is audiofile
        else if ((pos - 1) + m_viewPoint == m_currItemNr[*m_dlnaLevel]) { color = ANSI_ESC_MAGENTA;} // is current item
        else                                                            { color = ANSI_ESC_WHITE; }  // all other
        if (selectedLine && childCount > 0)                             { color = ANSI_ESC_CYAN; res = true; }    // is folder with childs
        if (selectedLine && isServer)                                   { color = ANSI_ESC_CYAN; res = true; }    // is server
        if (selectedLine && isURL && isAudio)                           { color = ANSI_ESC_CYAN; res = true; }    // is file
        if (childCount)                                                 { sprintf(extension, "%i", childCount); } // only folders have childCount
        if (itemSize)                                                   { sprintf(extension, "%li", itemSize);  } // only files have itemsize
        if (!duration.equals("?"))                                      { sprintf(extension, "%s", duration.c_get()); } // must be a audiofile
        if(color == ANSI_ESC_MAGENTA) { m_itemListPos_last = pos; }
        myList.drawLine(pos, item, extension, itemURL, color.c_get(), 1);
        return res;
    }

    void hasReleased(uint16_t x, uint16_t y) {
        m_itemListPos = y / (m_h / 10);
        bool     guard1 = false;
        bool     guard2 = false;
        uint16_t itemURLsize = 0;
        uint16_t titleSize = 0;
        for (int i = 0; i < m_srvContent->size(); i++) {
            if (strcmp(m_srvContent->at(i).itemURL.c_get(), "?") != 0) itemURLsize++;
            if (strcmp(m_srvContent->at(i).title.c_get(), "?") != 0) titleSize++;
        }
        if (itemURLsize > (m_itemListPos - 1)) guard2 = true;
        bool guard3 = false;
        if (titleSize > (m_itemListPos - 1)) guard3 = true;
        uint16_t friendlyName = 0;
        for (int i = 0; i < m_dlnaServer->size(); i++) {
            if (strcmp(m_dlnaServer->at(i).friendlyName.c_get(), "?") != 0) friendlyName++;
        }
        if (friendlyName > (m_itemListPos - 1)) guard1 = true;

        if (m_oldY && (m_oldY + 7 * m_lineHight < y)) { // fast wipe down
            m_ra.val1 = 0;
            if (m_viewPoint == 0) goto exit;
            if (m_viewPoint > 36)
                m_viewPoint -= 36;
            else
                m_viewPoint = 0;
            m_browseOnRelease = DLNA_WIPE;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY + 1 * m_lineHight < y)) { // normal wipe down
            m_ra.val1 = 0;
            if (m_viewPoint == 0) goto exit;
            if (m_viewPoint > 9)
                m_viewPoint -= 9;
            else
                m_viewPoint = 0;
            m_browseOnRelease = DLNA_WIPE;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY - 8 * m_lineHight > y)) { // fast wipe up
            m_ra.val1 = 0;
            if (m_viewPoint + 9 >= m_dlnaMaxItems - 1) goto exit;
            int16_t diff = (m_dlnaMaxItems - 1) - (m_viewPoint + 9);
            if (diff >= 36)
                m_viewPoint += 36;
            else
                m_viewPoint += diff;
            m_browseOnRelease = DLNA_WIPE;
            m_chptr = NULL;
            goto exit;
        }

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // normal wipe up
            m_ra.val1 = 0;
            if (m_viewPoint + 9 >= m_dlnaMaxItems - 1) goto exit;
            m_viewPoint += 9;
            m_browseOnRelease = DLNA_WIPE;
            m_chptr = NULL;
            goto exit;
        }

        if (m_itemListPos == 0) { // previous level, content list
            if (*m_dlnaLevel == 0) { goto exit; }
            m_viewPoint = 0;
            m_browseOnRelease = DLNA_PREV_LEVEL;
            goto exit;
        }

        if (guard1) { // server list
            if (*m_dlnaLevel == 0) {
                m_chptr = m_dlnaServer->at(m_itemListPos - 1).friendlyName.c_get();
                m_currDLNAsrvNr = m_itemListPos - 1;
                m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
                if (m_dlnaServer->at(m_itemListPos - 1).friendlyName.c_get() == NULL) {
                    MWR_LOG_WARN("invalid pointer in dlna history");
                    m_dlnaHistory[(*m_dlnaLevel) + 1].name = "dummy";
                    goto exit;
                }
                m_dlnaHistory[(*m_dlnaLevel) + 1].childCount = 0;
                m_dlnaHistory[(*m_dlnaLevel) + 1].name = m_dlnaServer->at(m_itemListPos - 1).friendlyName;
                m_browseOnRelease = DLNA_SERVERLIST;
                goto exit;
            }
        }

        if (guard2) { // is file
            if (startsWith(m_srvContent->at(m_itemListPos - 1).itemURL.c_get(), "http")) {
                m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
                if (m_srvContent->at(m_itemListPos - 1).isAudio) {
                    sprintf(m_buff, "%s", m_srvContent->at(m_itemListPos - 1).title.c_get());
                    m_chptr = m_buff;
                    m_ra.arg1 = m_srvContent->at(m_itemListPos - 1).itemURL; // url --> connecttohost()
                    m_ra.arg2 = m_srvContent->at(m_itemListPos - 1).title;   // filename --> showFileName()
                    if (m_ra.arg1.strlen() > 0 && m_ra.arg2.strlen() > 0) m_ra.val1 = 1;
                    m_browseOnRelease = DLNA_FILE;
                    goto exit;
                }
            }
        }

        if (guard3) { // is folder
            m_viewPoint = 0;
            sprintf(m_buff, "%s (%d)", m_srvContent->at(m_itemListPos - 1).title.c_get(), m_srvContent->at(m_itemListPos - 1).childCount);
            m_currItemNr[*m_dlnaLevel] = m_itemListPos - 1;
            m_chptr = m_buff;
            m_dlnaHistory[(*m_dlnaLevel) + 1].objId = m_srvContent->at(m_itemListPos - 1).objectId;
            m_dlnaHistory[(*m_dlnaLevel) + 1].name = m_srvContent->at(m_itemListPos - 1).title;
            m_dlnaHistory[(*m_dlnaLevel) + 1].childCount = m_srvContent->at(m_itemListPos - 1).childCount;
            m_browseOnRelease = DLNA_NEXT_LEVEL;
            goto exit;
        }
        // log_i("at this position is nothing to do");
    exit:
        return;
    }

  public: // -------------------------- IR actions --------------------------------------------------------------
    void prevPage() { // from IR control
        if (m_viewPoint == 0) return;
        if (m_viewPoint > 9)
            m_viewPoint -= 9;
        else
            m_viewPoint = 0;
        if (m_currItemNr[*m_dlnaLevel] > 9) {
            m_currItemNr[*m_dlnaLevel] -= 9;
        } else {
            m_currItemNr[*m_dlnaLevel] = 0;
        }
        m_chptr = NULL;
        m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9);
        m_dlna->loop();
        while (m_dlna->getState() != m_dlna->IDLE) {
            m_dlna->loop();
            vTaskDelay(10);
        } // wait of browse rady
        m_srvContent = &m_dlna->getBrowseResult();
        dlnaItemsList();
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        return;
    }
    void nextPage() { // from IR control
        if (m_dlnaMaxItems <= m_viewPoint + 9) return;
        if (m_dlnaMaxItems - 9 > m_viewPoint) {
            m_viewPoint += 9;
        } else {
            m_viewPoint = m_dlnaMaxItems - 9;
        }
        if (m_dlnaMaxItems - 9 > m_currItemNr[*m_dlnaLevel]) {
            m_currItemNr[*m_dlnaLevel] += 9;
        } else {
            m_currItemNr[*m_dlnaLevel] = m_dlnaMaxItems - 1;
        }
        m_chptr = NULL;
        m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9);
        m_dlna->loop();
        while (m_dlna->getState() != m_dlna->IDLE) {
            m_dlna->loop();
            vTaskDelay(10);
        } // wait of browse rady
        m_srvContent = &m_dlna->getBrowseResult();
        dlnaItemsList();
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        return;
    }
    void prevItem() { // from IR control
        if (*m_dlnaLevel == 0 && m_currItemNr[*m_dlnaLevel] == 0) return;
        if (m_currItemNr[*m_dlnaLevel] < 0) return;
        if (m_currItemNr[*m_dlnaLevel] < m_viewPoint) {
            m_viewPoint -= 9;
            if (m_viewPoint < 0) m_viewPoint = 0;
            m_chptr = NULL;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse ready
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return;
        }
        m_currItemNr[*m_dlnaLevel]--;
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        drawItem(m_currItemNr[*m_dlnaLevel] + 1 - m_viewPoint + 1); // std colour
    }
    void nextItem() { // from IR control
        if (m_dlnaMaxItems == m_currItemNr[*m_dlnaLevel] - 1) return;
        if (m_currItemNr[*m_dlnaLevel] == m_dlnaMaxItems - 1) return;
        m_currItemNr[*m_dlnaLevel]++;
        if (m_currItemNr[*m_dlnaLevel] >= m_viewPoint + 9) {
            m_viewPoint += 9;
            m_chptr = NULL;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return;
        }
        drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
        drawItem(m_currItemNr[*m_dlnaLevel] - 1 - m_viewPoint + 1); // std colour
    }

    const char* getSelectedURL() { // ok from IR
        if (*m_dlnaLevel == 0) {   //------------------------------------------------------------------------------------------------------- choose server
            // log_e("server %s", m_dlnaServer.friendlyName[m_currItemNr[0]]);
            m_chptr = m_dlnaServer->at(m_currItemNr[0]).friendlyName.c_get();
            m_currDLNAsrvNr = m_currItemNr[0];
            m_currItemNr[*m_dlnaLevel] = m_currItemNr[0];
            drawItem(m_currItemNr[*m_dlnaLevel] + m_viewPoint + 1, true); // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)++;
            if (m_dlnaServer->at(m_currItemNr[0]).friendlyName.c_get() == NULL) {
                MWR_LOG_ERROR("invalid pointer in dlna history");
                m_dlnaHistory[*m_dlnaLevel].name = "dummy";
                return NULL;
            }
            m_dlnaHistory[*m_dlnaLevel].name = m_dlnaServer->at(m_currItemNr[0]).friendlyName;
            m_dlna->browseServer(m_currDLNAsrvNr, "0", 0, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            m_dlnaMaxItems = m_dlna->getTotalMatches();
            m_dlnaHistory[*m_dlnaLevel].maxItems = m_dlnaMaxItems; // level 1
            // log_e("m_dlnaMaxItems %i, level %i", m_dlnaMaxItems, (*m_dlnaLevel));
            dlnaItemsList();
            return NULL;
        }
        if (m_currItemNr[*m_dlnaLevel] + 1 == m_viewPoint) { // DLNA history, parent item ---------------------------------------------- back to parent
            // log_e("%s", m_dlnaHistory[*m_dlnaLevel].name);
            drawItem(0, true); // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)--;
            m_dlnaMaxItems = m_dlnaHistory[*m_dlnaLevel].maxItems;
            m_viewPoint = 0;
            m_currItemNr[*m_dlnaLevel] = 0;
            if(*m_dlnaLevel == 0)  m_dlna->browseServer(m_currDLNAsrvNr, "0", 0, 9);
            else                   m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), m_viewPoint, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            dlnaItemsList();
            return NULL;
        }
        if (strcmp(m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get(), "?") == 0) { // --------------------------------------- choose folder
            drawItem(m_currItemNr[*m_dlnaLevel] - m_viewPoint + 1, true);                                   // make cyan
            vTaskDelay(300);
            (*m_dlnaLevel)++;
            m_currItemNr[*m_dlnaLevel] = 0;
            m_dlnaHistory[*m_dlnaLevel].objId = m_srvContent->at(m_currItemNr[(*m_dlnaLevel) - 1] - m_viewPoint).objectId;
            m_dlnaHistory[*m_dlnaLevel].name = m_srvContent->at(m_currItemNr[(*m_dlnaLevel) - 1] - m_viewPoint).title;
            m_dlnaHistory[*m_dlnaLevel].childCount = m_srvContent->at(m_currItemNr[(*m_dlnaLevel) - 1] - m_viewPoint).childCount;
            m_viewPoint = 0;
            m_dlna->browseServer(m_currDLNAsrvNr, m_dlnaHistory[*m_dlnaLevel].objId.c_get(), 0, 9);
            m_dlna->loop();
            while (m_dlna->getState() != m_dlna->IDLE) {
                m_dlna->loop();
                vTaskDelay(10);
            } // wait of browse rady
            m_srvContent = &m_dlna->getBrowseResult();
            m_dlnaMaxItems = m_dlna->getTotalMatches();
            m_dlnaHistory[*m_dlnaLevel].maxItems = m_dlnaMaxItems;
            // log_e("m_dlnaMaxItems %i, level %i", m_dlnaMaxItems, (*m_dlnaLevel)); // level 2, 3, 4...
            dlnaItemsList();
            if (!m_dlnaMaxItems) { // folder is empty
                m_currItemNr[*m_dlnaLevel]--;
                drawItem(m_currItemNr[*m_dlnaLevel] + 0 - m_viewPoint + 1); // make magenta
            }
            return NULL;
        }
        if (startsWith(m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get(), "http") != 0) { // ---------------------------------- choose file
            drawItem(m_currItemNr[*m_dlnaLevel] - m_viewPoint + 1, true);                                          // make cyan
            vTaskDelay(300);
            return m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).itemURL.c_get();
        }
        return NULL;
    }
    const char* getSelectedTitle() { return m_srvContent->at(m_currItemNr[*m_dlnaLevel] - m_viewPoint).title.c_get(); }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
  ———————————————————————————————————————————————————————
  | Audio Files                     Vol11    15:32:18   |
  | /audiofiles/myPlaylist/                    1-9/11   |           <- parent folder  m_curAudioFileNr = -1
  |   320_test.mpr 9610227                              |           <--               m_curAudioFileNr - m_viewpos = 0
  |   If_I_Had_a_Chicken_mono_mp3 1591510               |           <--
  |   If_I_Had_a_Chicken_mono_16bit.wav 13257580        |           <--
  |   If_I_Had_a_Chicken_mono_8bit.wav 6628972          |           <--
  |   If_I_Had_a_Chicken_stereo_mp3 6012554             |           <--
  |   If_I_Had_a_Chicken_stereo_16bit.wav 26514608      |           <--
  |   If_I_Had_a_Chicken_stereo_8bit.wav 1327260        |           <--
  |   beep.mp3 75302                                    |           <--
  |     click.mp3 3360                                  |           <--               m_curAudioFileNr - m_viewpos = 8
  | 003   0:00    128K              IP:192.168.178.24   |
  ———————————————————————————————————————————————————————
*/

class fileList : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_oldX = 0;
    int16_t      m_oldY = 0;
    uint8_t      m_lineHight = 0;
    uint8_t      m_browseOnRelease = 0;
    uint16_t     m_fileListPos = 0;
    int16_t      m_curAudioFileNr = 0;
    uint16_t     m_viewPos = 0;
    uint8_t      m_fontSize = 0;
    uint32_t     m_bgColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_state = false;
    ps_ptr<char> m_name;
    ps_ptr<char> m_curAudioFolder;
    char*        m_curAudioPath = NULL;
    char*        m_curAudioName = NULL;
    char*        m_fileItemsPos = NULL;
    const char*  m_tftSize = "";
    const char*  m_rootColor = ANSI_ESC_LIGHTBROWN;
    const char*  m_folderColor = ANSI_ESC_ORANGE;
    const char*  m_fileColor = ANSI_ESC_WHITE;
    const char*  m_selectColor = ANSI_ESC_CYAN;
    const char*  m_irColor = ANSI_ESC_MAGENTA;
    const char*  m_currentColor = ANSI_ESC_MAGENTA;
    releasedArg  m_ra;

  public:
    fileList(const char* name) {
        register_object(this);
        m_name = name;
        m_fileItemsPos = x_ps_malloc(30);
        m_curAudioPath = x_ps_malloc(1024);
        m_curAudioName = x_ps_malloc(1024);
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_ra.arg1 = "";
        m_ra.arg2 = "";
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~fileList() {
        x_ps_free(&m_fileItemsPos);
        x_ps_free(&m_curAudioName); //   song.mp3
        x_ps_free(&m_curAudioPath); //   /audiofiles/folder1/song.mp3
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_enabled = false;
        m_tftSize = tftSize;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(ps_ptr<char> cur_AudioFolder, uint16_t curAudioFileNr) {
        m_browseOnRelease = 0;
        m_clicked = false;
        m_enabled = true;
        if (!cur_AudioFolder.valid()) {
            MWR_LOG_DEBUG("cur_AudioFolder set to /audiofiles/");
            m_curAudioFolder = "/audiofiles/";
        } else if (!cur_AudioFolder.equals(m_curAudioFolder))
            m_curAudioFolder = cur_AudioFolder; // cur_AudioFolder can have the same address as m_curAudioFolder
        m_curAudioFileNr = curAudioFileNr;
        s_SD_content.listFilesInDir(m_curAudioFolder.c_get(), true, false);
        if (m_curAudioFileNr >= s_SD_content.getSize()) m_curAudioFileNr = s_SD_content.getSize(); // guard
        m_viewPos = calculateDisplayStartPosition(s_SD_content.getSize(), m_curAudioFileNr);       // calculate viewPos
        audioFileslist(m_viewPos);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_clicked == false) {
            m_oldX = x;
            m_oldY = y;
        }
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released(uint16_t x, uint16_t y) {
        if (!m_enabled) return false;
        if (!m_clicked) return false;

        hasReleased(x - m_x, y - m_y);
        m_clicked = false;
        int pos = (y - m_y) / m_lineHight;

        char* fileName = NULL;

        if (m_browseOnRelease == 1) {
            if (m_viewPos + 9 >= s_SD_content.getSize()) {
                goto exit;
            } // wipe up
            else {
                m_viewPos += 9;
                m_ra.val1 = 1;
            }
            audioFileslist(m_viewPos);
        }
        if (m_browseOnRelease == 2) {
            if (m_viewPos == 0) { goto exit; } // wipe down
            if (m_viewPos > 9) {
                m_viewPos -= 9;
                m_ra.val1 = 1;
            } else {
                m_viewPos = 0;
                m_ra.val1 = 1;
            }
            audioFileslist(m_viewPos);
        }
        if (m_browseOnRelease == 3) {                               // log_e("m_curAudioFolder = %s", m_curAudioFolder);                                         // previous folder
            if (m_curAudioFolder.equals("/audiofiles/")) goto exit; // is already the root
            myList.drawLine(pos, m_curAudioFolder.c_get(), "", "", ANSI_ESC_CYAN, -1);
            int lastSlash = m_curAudioFolder.last_index_of('/');
            if (lastSlash != -1) { // Look for the penultimate '/' before the position of the last
                int secondLastSlash = m_curAudioFolder.last_index_of('/', lastSlash - 1);
                if (secondLastSlash != -1) m_curAudioFolder[secondLastSlash + 1] = '\0';
            }
            MWR_LOG_DEBUG("m_curAudioFolder = %s", m_curAudioFolder);
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            s_SD_content.listFilesInDir(m_curAudioFolder.c_get(), true, false);
            m_ra.val1 = 2;
            m_ra.val2 = m_curAudioFileNr;
            m_ra.arg1 = m_curAudioFolder;
        }
        if (m_browseOnRelease == 4) {
            m_viewPos += m_fileListPos; // next folder
            int16_t idx = m_viewPos - 1;
            myList.drawLine(pos, s_SD_content.getColouredSStringByIndex(idx), "", "", ANSI_ESC_CYAN, -1);
            m_curAudioFolder = s_SD_content.getFilePathByIndex(idx);
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            s_SD_content.listFilesInDir(m_curAudioFolder.c_get(), true, false);
            m_ra.val1 = 2; // isfolder
            m_ra.val2 = m_viewPos;
            m_ra.arg1 = m_curAudioFolder;
            m_ra.arg2 = "";
            m_ra.arg3 = "";
        }
        if (m_browseOnRelease == 5) {
            m_viewPos += m_fileListPos; // play file
            myList.drawLine(pos, m_curAudioName, "", "", ANSI_ESC_CYAN, -1);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            m_ra.arg1 = m_curAudioFolder; // fileFolder
            m_ra.arg2 = m_curAudioName;   // fileName
            m_ra.arg3 = m_curAudioPath;   // filePath
            m_ra.val1 = 3;                // isfile
            m_ra.val2 = m_viewPos - 1;    // fileNr (is curAudioFileNr)
        }
    exit:
        m_browseOnRelease = 0;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        x_ps_free(&fileName);
        m_ra.val1 = 0;
        m_ra.arg1 = "";
        m_ra.arg2 = "";
        m_ra.arg3 = "";
        return true;
    }

    void prevPage() { // from IR control
        if (m_viewPos == 0) return;
        if (m_viewPos > 9) {
            m_viewPos -= 9;
            m_curAudioFileNr -= 9;
        } else {
            m_curAudioFileNr -= m_viewPos;
            m_viewPos = 0;
        }
        audioFileslist(m_viewPos);
        return;
    }
    void nextPage() { // from IR control
        if (m_viewPos + 9 <= s_SD_content.getSize() - 1) {
            m_viewPos += 9;
            m_curAudioFileNr += 9;
            if (m_curAudioFileNr > s_SD_content.getSize() - 1) m_curAudioFileNr = s_SD_content.getSize() - 1;
            audioFileslist(m_viewPos);
            return;
        }
    }

    void prevFile() { // from IR control
        if (m_curAudioFileNr < 0) return;
        if (m_curAudioFileNr && m_curAudioFileNr - m_viewPos == 0) {
            if (m_viewPos >= 9)
                m_viewPos -= 9;
            else
                m_viewPos = 0;
            m_curAudioFileNr--;
            audioFileslist(m_viewPos);
            return;
        }
        int         pos = m_curAudioFileNr - m_viewPos + 1;
        const char* color = m_fileColor;                                 // assume is file
        if (s_SD_content.isDir(m_curAudioFileNr)) color = m_folderColor; // is folder
        myList.colourLine(pos, color);
        m_curAudioFileNr--;
        myList.colourLine(pos - 1, m_irColor);
    }
    void nextFile() { // from IR control
        if (m_curAudioFileNr == s_SD_content.getSize() - 1) return;
        if (m_curAudioFileNr - m_viewPos == 8) {
            if (m_viewPos + 9 < s_SD_content.getSize())
                m_viewPos += 9;
            else
                m_viewPos = s_SD_content.getSize() - 1;
            m_curAudioFileNr++;
            audioFileslist(m_viewPos);
            return;
        }
        const char* color;
        int         pos = m_curAudioFileNr - m_viewPos + 1;
        if (m_curAudioFileNr == -1) {
            color = m_rootColor;
        } // is root dir
        else {
            if (s_SD_content.isDir(m_curAudioFileNr))
                color = m_folderColor; // is folder
            else
                color = m_fileColor; // is file
        }
        myList.colourLine(pos, color);
        m_curAudioFileNr++;
        myList.colourLine(pos + 1, m_irColor);
    }
    const char* getSelectedFile() {
        if (m_curAudioFileNr == -1) {                                 // get parent folder
            if (m_curAudioFolder.equals("/audiofiles/")) return NULL; // is already the root
            myList.colourLine(m_y, m_selectColor);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            int lastSlash = m_curAudioFolder.last_index_of('/');
            if (lastSlash != -1) { // Look for the penultimate '/' before the position of the last
                int secondLastSlash = m_curAudioFolder.last_index_of('/', lastSlash - 1);
                if (secondLastSlash != -1) m_curAudioFolder[secondLastSlash + 1] = '\0'; // previous folder
            }
            m_curAudioFileNr = 0;
            m_viewPos = 0;
            s_SD_content.listFilesInDir(m_curAudioFolder.c_get(), true, false);
            show(m_curAudioFolder.c_get(), 0);
            return NULL;
        }
        if (s_SD_content.isDir(m_curAudioFileNr)) { // is child folder
            myList.colourLine(m_y, m_selectColor);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            strcpy(m_curAudioPath, s_SD_content.getFilePathByIndex(m_curAudioFileNr));
            show(m_curAudioPath, 0);
            return NULL;
        }
        myList.colourLine(m_y, m_selectColor);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return s_SD_content.getFilePathByIndex(m_curAudioFileNr);
    }
    const char* getSelectedFileName() { return s_SD_content.getFileNameByIndex(m_curAudioFileNr); }
    const char* getSelectedFilePath() {
        myList.colourLine(m_curAudioFileNr - m_viewPos + 1, m_selectColor);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return s_SD_content.getFilePathByIndex(m_curAudioFileNr);
    }
    uint16_t getSelectedFileNr() { return m_curAudioFileNr; }

  private:
    void audioFileslist(uint16_t viewPos) {
        // guard -------------------------------------------------------------------------------------------------------------------------------------
        if (s_SD_content.getSize() == 0) { ; }                                           // folder empty
        if (viewPos >= s_SD_content.getSize()) { viewPos = s_SD_content.getSize() - 1; } // viewPos too high
        //--------------------------------------------------------------------------------------------------------------------------------------------

        tft.setFont(m_fontSize);
        myList.setMode(PLAYER, m_tftSize, m_fontSize);
        myList.clearList();
        const char* color;

        color = m_folderColor;
        if (m_curAudioFolder.equals("/audiofiles/")) color = m_rootColor; // is root
        myList.drawLine(0, m_curAudioFolder.c_get(), "", "", color, 0);
        color = m_fileColor;
        for (uint8_t pos = 1; pos < 10; pos++) {
            int idx = pos + viewPos - 1;
            if (pos == 1 && viewPos > 0 && s_SD_content.getSize()) { myList.drawTriangeUp(); }
            if (pos == 9 && viewPos + 9 < s_SD_content.getSize()) { myList.drawTriangeDown(); }
            if (viewPos + pos > s_SD_content.getSize()) break;
            if (s_SD_content.isDir(idx)) {
                if (idx == m_curAudioFileNr) {
                    color = m_currentColor;
                } // is current folder
                else {
                    color = m_folderColor;
                } // is folder
            } else {
                if (idx == m_curAudioFileNr) {
                    color = m_currentColor;
                } // current file
                else {
                    color = m_fileColor;
                } // is file
            }
            if (s_SD_content.isDir(idx))
                myList.drawLine(pos, s_SD_content.getFileNameByIndex(idx), "", "", color, 0);
            else
                myList.drawLine(pos, s_SD_content.getFileNameByIndex(idx), "", "", color, s_SD_content.getFileSizeByIndex(idx));
        }
        uint16_t firstVal = viewPos + 1;
        uint16_t secondVal = firstVal + 8;
        if (secondVal > s_SD_content.getSize()) secondVal = s_SD_content.getSize();
        myList.drawPosInfo(firstVal, secondVal, s_SD_content.getSize(), ANSI_ESC_ORANGE); // shows the current items pos e.g. "30-39/210"
        return;
    }

    int calculateDisplayStartPosition(int list_size, int current_position) {
        // Calculate the theoretical starting position to get current_position to the middle
        int start_position = current_position - 4;
        // Make sure the starting position is not negative
        start_position = std::max(0, start_position);
        // Make sure the starting position doesn't go beyond the end of the list
        if (start_position + 9 > list_size) { start_position = std::max(0, list_size - 9); }
        return start_position;
    }

    void hasReleased(uint16_t x, uint16_t y) {
        m_fileListPos = y / (m_h / 10);

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // -------------------------------------- normal wipe up
            m_browseOnRelease = 1;
            goto exit;
        }

        if (m_oldY && (m_oldY + 2 * m_lineHight < y)) { // -------------------------------------- normal wipe down
            m_browseOnRelease = 2;
            goto exit;
        }

        if (m_fileListPos == 0) {                          //  ----------------------------------------------------------- previous folder
            if (m_curAudioFolder.last_index_of('/') > 0) { // not the first '/'
                m_browseOnRelease = 3;
            }
        } else {
            if (m_fileListPos + m_viewPos > s_SD_content.getSize()) goto exit; // ----------------- next folder
            int idx = m_viewPos + m_fileListPos - 1;
            if (s_SD_content.isDir(idx)) {
                strcpy(m_curAudioName, "");
                m_curAudioFolder = s_SD_content.getFileFolderByIndex(idx);
                m_browseOnRelease = 4;
            } else { // -------------------------------------------------------------------------- playfile
                strcpy(m_curAudioName, s_SD_content.getFileNameByIndex(idx));
                m_curAudioFolder = s_SD_content.getFileFolderByIndex(idx);
                strcpy(m_curAudioPath, s_SD_content.getFilePathByIndex(idx));
                m_browseOnRelease = 5;
            }
        }
    exit:
        return;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
extern stationManagement staMgnt; /*
———————————————————————————————————————————————————————
| Stations List                   Vol3    01:16:32    |           m_stationListPos
| 017 BGRADIOk                                        |           <-- 0
| 018 knixx.fm                                        |           <-- 1
| 019 -0N-Chrismas on Radio                           |           <-- 2
| 020 BBC 6music                                      |           <-- 3
| 021 -0N-Movues on Radio                             |           <-- 4
| 022 -0N-Top40 on Radio                              |           <-- 5
| 023 Rockantenne Alternative (mp3)                   |           <-- 6
| 024 Gra Wroclaw                                     |           <-- 7
| 025 Classic EuroDisco                               |           <-- 8
| 026 Hit Radio FFH - Soundtrack (AAC+)               |           <-- 9
| 003   0:00    128K              IP:192.168.178.24   |
———————————————————————————————————————————————————————
*/
class stationsList : public RegisterTable {
  private:
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int16_t      m_oldX = 0;
    int16_t      m_oldY = 0;
    uint8_t      m_lineHight = 0;
    uint16_t     m_firstStationsLineNr = 0;
    uint16_t*    m_curSstationNr = NULL;
    uint16_t     m_curStaNrCpy = 0;
    uint8_t      m_browseOnRelease = 0;
    uint8_t      m_fontSize = 0;
    uint8_t      m_stationListPos = 0;
    uint32_t     m_bgColor = 0;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_state = false;
    ps_ptr<char> m_name;
    char*        m_pathBuff = NULL;
    char*        m_buff = NULL;
    releasedArg  m_ra;
    const char*  m_colorToDraw = NULL;
    const char*  m_staNameToDraw = NULL;
    const char*  m_tftSize = "";
    uint16_t     m_staNrToDraw = 0;

  public:
    stationsList(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_enabled = false;
        m_clicked = false;
        m_state = false;
        m_pathBuff = x_ps_malloc(50);
        m_ra.arg1 = "";
        m_ra.arg2 = "";
        m_ra.val1 = 0;
        m_ra.val2 = 0;
    }
    ~stationsList() { x_ps_free(&m_buff); }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char* tftSize, uint8_t fontSize) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w; // width
        m_h = h; // high
        m_lineHight = m_h / 10;
        m_fontSize = fontSize;
        m_enabled = false;
        m_tftSize = tftSize;
    }
    void         currentStationNr(uint16_t* curStationNr) { m_curSstationNr = curStationNr; }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show() {
        m_clicked = false;
        m_enabled = true;
        m_browseOnRelease = 0;
        stationslist(true);
    }
    void hide() {
        m_enabled = false;
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
    }
    void disable() { m_enabled = false; }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        hasClicked(x - m_x, y - m_y);
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;

        if (m_browseOnRelease == 1) {
            stationslist(false); // wipe up
        }
        if (m_browseOnRelease == 2) {
            stationslist(false); // wipe down
        }
        if (m_browseOnRelease == 3) {
            myList.getTxtByPos(m_stationListPos); // click
            myList.colourLine(m_stationListPos, ANSI_ESC_CYAN);
            vTaskDelay(300 / portTICK_PERIOD_MS);
            m_ra.val1 = myList.getNumberByPos(m_stationListPos);
        }
        m_browseOnRelease = 0;
        m_oldX = 0;
        m_oldY = 0;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        x_ps_free(&m_buff);
        m_ra.val1 = 0;
        m_ra.arg1 = "";
        return true;
    }

  private:
    void stationslist(bool first) {
        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if (first) {
            if (staMgnt.getSumStations() <= 10)
                m_firstStationsLineNr = 0;
            else if (*m_curSstationNr < 5)
                m_firstStationsLineNr = 0;
            else if (*m_curSstationNr + 5 <= staMgnt.getSumStations())
                m_firstStationsLineNr = *m_curSstationNr - 5;
            else
                m_firstStationsLineNr = staMgnt.getSumStations() - 10;
            m_curStaNrCpy = *m_curSstationNr;
            if (m_curStaNrCpy == 0) m_curStaNrCpy = 1;
        }
        myList.clearList();
        myList.setMode(RADIO, m_tftSize, m_fontSize);

        for (uint8_t pos = 0; pos < 10; pos++) {
            if (pos + m_firstStationsLineNr + 1 > staMgnt.getSumStations()) break;
            if (staMgnt.getStationFav(pos + m_firstStationsLineNr + 1) == '*')
                m_colorToDraw = ANSI_ESC_WHITE; // is fav station
            else
                m_colorToDraw = ANSI_ESC_GREY;                                                        // is not a fav station
            if ((pos + m_firstStationsLineNr + 1) == m_curStaNrCpy) m_colorToDraw = ANSI_ESC_MAGENTA; // is the current station

            m_staNameToDraw = staMgnt.getStationName(pos + m_firstStationsLineNr + 1); // the station name
            m_staNrToDraw = pos + m_firstStationsLineNr + 1;                           // the station number
            myList.drawLine(pos, m_staNameToDraw, NULL, NULL, m_colorToDraw, m_staNrToDraw);
            if (pos == 1 && m_firstStationsLineNr > 0 && staMgnt.getSumStations()) { myList.drawTriangeUp(); }
            if (pos == 9 && m_firstStationsLineNr + 10 < staMgnt.getSumStations()) { myList.drawTriangeDown(); }
        }
        xSemaphoreGive(mutex_display);
    }
    void hasClicked(uint16_t x, uint16_t y) {
        m_stationListPos = y / (m_h / 10);
        if (m_oldY && (m_oldY + 2 * m_lineHight < y)) { // wipe up
            if (m_browseOnRelease != 1) {
                m_browseOnRelease = 1;
                if (m_firstStationsLineNr == 0) {
                    m_browseOnRelease = 0;
                    return;
                } // nothing to do
                else if (m_firstStationsLineNr < 10)
                    m_firstStationsLineNr = 0;
                else
                    m_firstStationsLineNr -= 9;
            }
            return;
        }

        if (m_oldY && (m_oldY - 2 * m_lineHight > y)) { // wipe down
            if (m_browseOnRelease != 2) {
                m_browseOnRelease = 2;
                if (m_firstStationsLineNr + 10 >= staMgnt.getSumStations()) {
                    m_browseOnRelease = 0;
                    return;
                } // nothing to do
                else
                    m_firstStationsLineNr += 9;
            }
            return;
        }
        if (myList.getNumberByPos(m_stationListPos) == -1) return;
        if (m_oldX || m_oldY) return;
        m_oldX = x;
        m_oldY = y;
        m_browseOnRelease = 3; // pos has clicked
        return;
    }

  public:
    void prevPage() { // from IR control
        if (m_firstStationsLineNr == 0) {
            return;
        } // nothing to do
        else if (m_firstStationsLineNr < 10) {
            m_curStaNrCpy -= m_firstStationsLineNr;
            m_firstStationsLineNr = 0;
        } else {
            m_firstStationsLineNr -= 9;
            m_curStaNrCpy -= 9;
        }
        stationslist(false);
    }
    void nextPage() { // from IR control
        if (m_firstStationsLineNr + 10 >= staMgnt.getSumStations()) {
            m_browseOnRelease = 0;
            return;
        } // nothing to do
        else {
            m_firstStationsLineNr += 9;
            m_curStaNrCpy += 9;
            if (m_curStaNrCpy > staMgnt.getSumStations()) m_curStaNrCpy = staMgnt.getSumStations();
        }
        stationslist(false);
    }
    void prevStation() { // from IR control
        if (m_curStaNrCpy < 2) return;
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr - 1;
        if (pos < 0) return;
        if (pos == 0) { // prev page
            if (m_firstStationsLineNr > 8)
                m_firstStationsLineNr -= 9;
            else
                m_firstStationsLineNr = 0;
            m_curStaNrCpy--;
            stationslist(false);
            return;
        }
        myList.colourLine(pos, staMgnt.getStationFav(m_curStaNrCpy) == '*' ? ANSI_ESC_WHITE : ANSI_ESC_GREY);
        myList.colourLine(pos - 1, ANSI_ESC_MAGENTA);
        m_curStaNrCpy--;
    }
    void nextStation() { // from IR control
        if (m_curStaNrCpy >= staMgnt.getSumStations()) return;
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr - 1;
        if (pos > 9) return;
        if (pos == 9) { // next Page
            m_firstStationsLineNr += 9;
            m_curStaNrCpy++;
            stationslist(false);
            return;
        }
        myList.colourLine(pos, staMgnt.getStationFav(m_curStaNrCpy) == '*' ? ANSI_ESC_WHITE : ANSI_ESC_GREY);
        myList.colourLine(pos + 1, ANSI_ESC_MAGENTA);
        m_curStaNrCpy++;
    }
    uint16_t getSelectedStation() { // from IR control
        int8_t pos = m_curStaNrCpy - m_firstStationsLineNr;
        myList.colourLine(pos - 1, ANSI_ESC_CYAN);
        vTaskDelay(300 / portTICK_PERIOD_MS);
        return m_curStaNrCpy;
    }
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class vuMeter : public RegisterTable {
  private:
    uint16_t     m_x = 0;
    uint16_t     m_y = 0;
    uint16_t     m_w = 0;
    uint16_t     m_h = 0;
    uint32_t     m_bgColor = TFT_BLACK;
    uint32_t     m_frameColor = TFT_DARKGREY;
    ps_ptr<char> m_name;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_backgroundTransparency = false;
    uint8_t      m_VUleftCh = 0;  // VU meter left channel
    uint8_t      m_VUrightCh = 0; // VU meter right channel
    releasedArg  m_ra;
    uint8_t      m_segm_w = 0;
    uint8_t      m_segm_h = 0;
    uint8_t      m_frameSize = 1;
    uint16_t     m_frame_x = 0;
    uint16_t     m_frame_y = 0;
    uint16_t     m_frame_w = 0;
    uint16_t     m_frame_h = 0;

  public:
    vuMeter(const char* name) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
    }
    ~vuMeter() {}
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t paddig_left, uint8_t paddig_right, uint8_t paddig_top, uint8_t paddig_bottom) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        m_frame_x = x + paddig_left;
        m_frame_y = y + paddig_top;
        uint16_t frame_w = m_w - paddig_left - paddig_right;
        uint16_t frame_h = m_h - paddig_top - paddig_bottom;
        m_segm_w = ((frame_w - 3 * m_frameSize) / 2) - m_frameSize;  // 2 columns + 3 frameSizes
        m_segm_h = ((frame_h - 2 * m_frameSize) / 12) - m_frameSize; // 12 rows + 2 frameSizes
        m_frame_w = 2  * m_segm_w + 3 * m_frameSize;
        m_frame_h = 12 * m_segm_h + 13 * m_frameSize;
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         show(bool backgroundTransparency = false) {
        m_backgroundTransparency = backgroundTransparency;
        m_enabled = true;
        m_clicked = false;
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        tft.drawRect(m_frame_x, m_frame_y, m_frame_w, m_frame_h, m_frameColor);
        for (uint8_t i = 0; i < 12; i++) {
            drawRect(i, 0, 0);
            drawRect(i, 1, 0);
        }
        m_VUleftCh = 0;
        m_VUrightCh = 0;
    }
    void hide() {
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }
    void disable() { m_enabled = false; }
    void enable()  { m_enabled = true; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void update(uint16_t vum) {
        if (!m_enabled) return;
        uint8_t left = map_l(vum >> 8, 0, 127, 0, 11);
        uint8_t right = map_l(vum & 0x00FF, 0, 127, 0, 11);

        xSemaphoreTake(mutex_display, portMAX_DELAY);
        if (left > m_VUleftCh) {
            for (int32_t i = m_VUleftCh; i < left; i++) { drawRect(i, 1, 1); }
        }
        if (left < m_VUleftCh) {
            for (int32_t i = left; i < m_VUleftCh; i++) { drawRect(i, 1, 0); }
        }
        m_VUleftCh = left;

        if (right > m_VUrightCh) {
            for (int32_t i = m_VUrightCh; i < right; i++) { drawRect(i, 0, 1); }
        }
        if (right < m_VUrightCh) {
            for (int32_t i = right; i < m_VUrightCh; i++) { drawRect(i, 0, 0); }
        }
        m_VUrightCh = right;
        xSemaphoreGive(mutex_display);
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }

  private:
    void drawRect(uint8_t row, uint8_t col, bool br) {
        uint16_t color = 0;
        uint16_t y_end = m_frame_y + m_frame_h - m_frameSize - m_segm_h;
        uint16_t xPos = m_frame_x + m_frameSize + col * (m_segm_w + m_frameSize);
        uint16_t yPos = y_end - row * (m_frameSize + m_segm_h);
        if (row > 11) return;
        switch (row) {
            case 0 ... 6: // green
                br ? color = TFT_GREEN : color = TFT_DARKGREEN;
                break;
            case 7 ... 9: // yellow
                br ? color = TFT_YELLOW : color = TFT_DARKYELLOW;
                break;
            case 10 ... 11: // red
                br ? color = TFT_RED : color = TFT_DARKRED;
                break;
        }
        tft.fillRect(xPos, yPos, m_segm_w, m_segm_h, color);
    };
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayHeader : public RegisterTable {
  private:
    textbox*     txt_Item = new textbox("header_Item");          // Radio, Player, Clock....
    pictureBox*  pic_Speaker = new pictureBox("header_Speaker"); // loudspeaker symbol
    textbox*     txt_Volume = new textbox("header_Volume");      // volume
    pictureBox*  pic_RSSID = new pictureBox("header_RSSID");     // RSSID symbol
    timeString*  m_timeStringObject = NULL;
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    int8_t       m_rssi = 0;
    int8_t       m_old_rssi = -1;
    uint8_t      m_fontSize = 0;
    uint8_t      m_volume = 0;
    uint32_t     m_bgColor = TFT_BLACK;
    ps_ptr<char> m_name;
    ps_ptr<char> m_item;
    char         m_time[10] = "00:00:00";
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_speakerOn = false;
    bool         m_backgroundTransparency = false;
    const char   m_rssiSymbol[5][18] = {"/common/RSSI0.png", "/common/RSSI1.png", "/common/RSSI2.png", "/common/RSSI3.png", "/common/RSSI4.png"};
    const char   m_speakerSymbol[2][25] = {"/common/Speaker_off.png", "/common/Speaker_on.png"};
    releasedArg  m_ra;
    uint16_t     m_itemColor = TFT_GREENYELLOW;
    uint16_t     m_volumeColor = TFT_DEEPSKYBLUE;
    uint16_t     m_timeColor = TFT_GREENYELLOW;
    //------------------------------------------------------------------------------------------------------------------------------------------------
#if TFT_CONTROLLER < 2 // 320 x 240px
    //------------------------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 165;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 165;
        uint16_t w = 30;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 25 x 20 px
    struct w_v {
        uint16_t x = 195;
        uint16_t w = 30;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 225;
        uint16_t w = 35;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 27 x 20 px
    struct w_t {
        uint16_t x = 260;
        uint16_t w = 60;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time;      // time object
    //------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7 // 480 x 320px
    //------------------------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 240;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 240;
        uint16_t w = 45;
        uint8_t  pl = 3;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 38 x 30 px
    struct w_v {
        uint16_t x = 285;
        uint16_t w = 50;
        uint8_t  pl = 10;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 335;
        uint16_t w = 45;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 39 x 30 px
    struct w_t {
        uint16_t x = 380;
        uint16_t w = 100;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time; // time object
    //------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER == 7                 // 800 x 480px
    //------------------------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 400;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 400;
        uint16_t w = 60;
        uint8_t  pl = 1;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 57 x 46 px
    struct w_v {
        uint16_t x = 460;
        uint16_t w = 100;
        uint8_t  pl = 10;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 560;
        uint16_t w = 80;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 64 x 48 px
    struct w_t {
        uint16_t x = 640;
        uint16_t w = 160;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time; // time object
    //------------------------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER == 8                 // 1024 x 600px
    //------------------------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_i {
        uint16_t x = 0;
        uint16_t w = 400;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Item; // Radio, Player, Clock...
    struct w_l {
        uint16_t x = 400;
        uint16_t w = 75;
        uint8_t  pl = 1;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Speaker; // loudspeaker symbol 69 x 56 px
    struct w_v {
        uint16_t x = 475;
        uint16_t w = 100;
        uint8_t  pl = 10;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Volume; // volume
    struct w_r {
        uint16_t x = 740;
        uint16_t w = 80;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_RSSID; // RSSID symbol 75 x 56 px
    struct w_t {
        uint16_t x = 840;
        uint16_t w = 160;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_time; // time object
#endif
  public:
    displayHeader(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
        m_timeStringObject = new timeString("timeString", m_fontSize);
    }
    ~displayHeader() {
        delete txt_Item;
        delete pic_Speaker;
        delete txt_Volume;
        delete pic_RSSID;
        delete m_timeStringObject;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        txt_Item->begin(s_Item.x, m_y, s_Item.w, m_h, s_Item.pl, s_Item.pr, s_Item.pt, s_Item.pb);
        pic_Speaker->begin(s_Speaker.x, m_y, s_Speaker.w, m_h, s_Speaker.pl, s_Speaker.pr, s_Speaker.pt, s_Speaker.pb);
        txt_Volume->begin(s_Volume.x, m_y, s_Volume.w, m_h, s_Volume.pl, s_Volume.pr, s_Volume.pt, s_Volume.pb);
        pic_RSSID->begin(s_RSSID.x, m_y, s_RSSID.w, m_h, s_RSSID.pl, s_RSSID.pr, s_RSSID.pt, s_RSSID.pb);
        m_timeStringObject->begin(s_time.x, m_y, s_time.w, m_h, s_time.pl, s_time.pr, s_time.pt, s_time.pb);

        txt_Item->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_Item->setTextColor(m_itemColor);
        txt_Item->setFont(m_fontSize); // 0 -> auto
        pic_Speaker->setPicturePath(m_speakerSymbol[0]);
        txt_Volume->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_Volume->setFont(m_fontSize); // 0 -> auto
        pic_RSSID->setPicturePath(m_rssiSymbol[0]);
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void show(bool transparency = false) {
        m_backgroundTransparency = transparency;
        if(m_backgroundTransparency) tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        else                         tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_timeStringObject->show(m_backgroundTransparency, false);
        m_enabled = true;
        m_clicked = false;
        m_old_rssi = -1;
        updateItem(m_item);
        speakerOnOff(m_speakerOn);
        updateVolume(m_volume);
        updateRSSI(m_rssi);
        updateTime(m_time, true);
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void enable() { m_enabled = true; m_timeStringObject->enable(); }
    void disable() { m_enabled = false; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void updateItem(ps_ptr<char> hl_item) { // radio, clock, audioplayer...
        if (!m_enabled) return;
        m_item = hl_item;
        txt_Item->setText(hl_item.c_get());
        txt_Item->show(m_backgroundTransparency, false);
    }
    void setItemColor(uint16_t itemColor) {
        m_itemColor = itemColor;
        txt_Item->setTextColor(m_itemColor);
    }

    void speakerOnOff(bool on) {
        m_speakerOn = on;
        if (!m_enabled) return;
        pic_Speaker->setPicturePath(m_speakerSymbol[m_speakerOn]);
        pic_Speaker->show(m_backgroundTransparency, false);
    }
    void updateVolume(uint8_t vol) {
        m_volume = vol;
        if (!m_enabled) return;
        char buff[15];
        itoa(m_volume, buff, 10);
        txt_Volume->setTextColor(m_volumeColor);
        txt_Volume->setText(buff);
        txt_Volume->show(m_backgroundTransparency, false);
    }

    void updateRSSI(int8_t rssi, bool show = false) {
        static int32_t old_rssi = -1;
        int8_t         new_rssi = -1;
        if (rssi >= 0) return;
        m_rssi = rssi;
        if (m_rssi < -1) new_rssi = 4;
        if (m_rssi < -50) new_rssi = 3;
        if (m_rssi < -65) new_rssi = 2;
        if (m_rssi < -75) new_rssi = 1;
        if (m_rssi < -85) new_rssi = 0;

        if (new_rssi != old_rssi) {
            old_rssi = new_rssi; // no need to draw a rssi icon if rssiRange has not changed
            if (ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO) {
                static int32_t tmp_rssi = 0;
                if ((abs(rssi - tmp_rssi) > 4)) { SerialPrintfln("WiFI_info:   RSSI is " ANSI_ESC_CYAN "%d" ANSI_ESC_WHITE " dB", rssi); }
                tmp_rssi = rssi;
            }
            if (m_enabled) show = true;
        }
        if (show) {
            pic_RSSID->setPicturePath(m_rssiSymbol[new_rssi]);
            pic_RSSID->show(m_backgroundTransparency, false);
        }
    }
    void updateTime(const char* hl_time, bool complete = true) {
        if (!m_enabled) return;
        memcpy(m_time, hl_time, 8); // hhmmss
        m_timeStringObject->updateTime(m_time, false);
    }
    void setTimeColor(uint16_t timeColor) {
        m_timeColor = timeColor;
        m_timeStringObject->setTextColor(m_timeColor);
        updateTime(m_time, true);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, m_enabled);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }
};
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class displayFooter : public RegisterTable {
  private:
    pictureBox*  pic_Antenna = new pictureBox("footer_Antenna");     // antenna symbol
    textbox*     txt_StaNr = new textbox("footer_StaNr");            // station number
    pictureBox*  pic_Flag = new pictureBox("footer_Flag");           // flag symbol
    pictureBox*  pic_Hourglass = new pictureBox("footer_Hourglass"); // hourglass symbol
    textbox*     txt_OffTimer = new textbox("footer_OffTimer");      // off timer
    textbox*     txt_BitRate = new textbox("footer_BitRate");        // bit rate
    textbox*     txt_IpAddr = new textbox("footer_IPaddr");          // ip address
    int16_t      m_x = 0;
    int16_t      m_y = 0;
    int16_t      m_w = 0;
    int16_t      m_h = 0;
    uint8_t      m_fontSize = 0;
    int8_t       m_timeCounter = 0;
    uint8_t      m_volume = 0;
    uint16_t     m_staNr = 0;
    uint16_t     m_offTime = 0;
    uint32_t     m_bitRate = 0;
    uint16_t     m_bgColor = TFT_BLACK;
    uint16_t     m_stationColor = TFT_LAVENDER;
    uint16_t     m_bitRateColor = TFT_LAVENDER;
    uint16_t     m_ipAddrColor = TFT_GREENYELLOW;
    ps_ptr<char> m_name;
    char*        m_ipAddr = NULL;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_backgroundTransparency = false;
    releasedArg  m_ra;
    const char   m_stationSymbol[22] = "/common/Antenna.png";
    const char   m_hourGlassymbol[2][27] = {"/common/Hourglass_blue.png", "/common/Hourglass_red.png"};
    //-----------------------------------------------------------------------------------------------------------------------------------
#if TFT_CONTROLLER < 2 // 320 x 240px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 0;
        uint16_t w = 25;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 19 x 20 px
    struct w_s {
        uint16_t x = 25;
        uint16_t w = 32;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 57;
        uint16_t w = 40;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  33...40 x 20 px
    struct w_h {
        uint16_t x = 100;
        uint16_t w = 20;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   16 x 20 px
    struct w_o {
        uint16_t x = 122;
        uint16_t w = 35;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 158;
        uint16_t w = 42;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 200;
        uint16_t w = 120;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#elif TFT_CONTROLLER < 7 // 480 x 320px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 1;
        uint16_t w = 30;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 29 x 30 px
    struct w_s {
        uint16_t x = 30;
        uint16_t w = 50;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 80;
        uint16_t w = 48;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 3;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  40...48 x 24 px
    struct w_h {
        uint16_t x = 132;
        uint16_t w = 24;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   23 x 30 px
    struct w_o {
        uint16_t x = 160;
        uint16_t w = 54;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 214;
        uint16_t w = 66;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 280;
        uint16_t w = 200;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#elif  TFT_CONTROLLER == 7                  // 800 x 480px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a {
        uint16_t x = 0;
        uint16_t w = 51;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 47 x 48 px
    struct w_s {
        uint16_t x = 51;
        uint16_t w = 84;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f {
        uint16_t x = 135;
        uint16_t w = 80;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 5;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  60...80 x 40 px
    struct w_h {
        uint16_t x = 225;
        uint16_t w = 40;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 3;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   35 x 44 px
    struct w_o {
        uint16_t x = 265;
        uint16_t w = 75;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 340;
        uint16_t w = 110;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 450;
        uint16_t w = 350;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#elif  TFT_CONTROLLER == 8                  // 1024 x 600px
    //-----------------------------------------------------------padding-left-right-top-bottom-------------------------------------------
    struct w_a { // antenna
        uint16_t x = 0;
        uint16_t w = 60;
        uint8_t  pl = 15;
        uint8_t  pr = 0;
        uint8_t  pt = 1;
        uint8_t  pb = 0;
    } const s_Antenna; // Antenna.png: 55 x 56 px
    struct w_s { // station number
        uint16_t x = 80;
        uint16_t w = 100;
        uint8_t  pl = 5;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_StaNr;
    struct w_f { // flags
        uint16_t x = 180;
        uint16_t w = 110;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 5;
        uint8_t  pb = 0;
    } const s_Flag; // Flags:  max 100 x 50 px
    struct w_h {
        uint16_t x = 300;
        uint16_t w = 40;
        uint8_t  pl = 2;
        uint8_t  pr = 0;
        uint8_t  pt = 3;
        uint8_t  pb = 0;
    } const s_Hourglass; // Hourglass:   45 x 56 px
    struct w_o {
        uint16_t x = 360;
        uint16_t w = 80;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_OffTimer;
    struct w_b {
        uint16_t x = 450;
        uint16_t w = 110;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 2;
    } const s_BitRate;
    struct w_i {
        uint16_t x = 700;
        uint16_t w = 324;
        uint8_t  pl = 0;
        uint8_t  pr = 0;
        uint8_t  pt = 0;
        uint8_t  pb = 0;
    } const s_IPaddr;
    //-----------------------------------------------------------------------------------------------------------------------------------
#endif
  public:
    displayFooter(const char* name, uint8_t fontSize) {
        register_object(this);
        m_name = name;
        m_bgColor = TFT_BLACK;
        m_fontSize = fontSize;
    }
    ~displayFooter() {
        x_ps_free(&m_ipAddr);
        delete pic_Antenna;
        delete txt_StaNr;
        delete pic_Flag;
        delete pic_Hourglass;
        delete txt_OffTimer;
        delete txt_BitRate;
        delete txt_IpAddr;
    }
    void begin(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
        m_x = x; // x pos
        m_y = y; // y pos
        m_w = w;
        m_h = h;
        pic_Antenna->begin(s_Antenna.x, m_y, s_Antenna.w, m_h, s_Antenna.pl, s_Antenna.pr, s_Antenna.pt, s_Antenna.pb);
        txt_StaNr->begin(s_StaNr.x, m_y, s_StaNr.w, m_h, s_StaNr.pl, s_StaNr.pr, s_StaNr.pt, s_StaNr.pb);
        pic_Flag->begin(s_Flag.x, m_y, s_Flag.w, m_h, s_Flag.pl, s_Flag.pr, s_Flag.pt, s_Flag.pb);
        pic_Hourglass->begin(s_Hourglass.x, m_y, s_Hourglass.w, m_h, s_Hourglass.pl, s_Hourglass.pr, s_Hourglass.pt, s_Hourglass.pb);
        txt_OffTimer->begin(s_OffTimer.x, m_y, s_OffTimer.w, m_h, s_OffTimer.pl, s_OffTimer.pr, s_OffTimer.pt, s_OffTimer.pb);
        txt_BitRate->begin(s_BitRate.x, m_y, s_BitRate.w, m_h, s_BitRate.pl, s_BitRate.pr, s_BitRate.pt, s_BitRate.pb);
        txt_IpAddr->begin(s_IPaddr.x, m_y, s_IPaddr.w, m_h, s_IPaddr.pl, s_IPaddr.pr, s_IPaddr.pt, s_IPaddr.pb);

        txt_StaNr->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_StaNr->setTextColor(m_stationColor);
        txt_StaNr->setFont(m_fontSize); // 0 -> auto
        txt_OffTimer->setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
        txt_OffTimer->setFont(m_fontSize); // 0 -> auto
        txt_BitRate->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        txt_BitRate->setTextColor(m_bitRateColor);
        txt_BitRate->setBorderColor(m_bitRateColor);
        txt_BitRate->setBorderWidth(1);
        txt_BitRate->setFont(m_fontSize); // 0 -> auto
        txt_IpAddr->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        txt_IpAddr->setTextColor(m_ipAddrColor);
        txt_IpAddr->setFont(m_fontSize); // 0 -> auto
        pic_Antenna->setPicturePath(m_stationSymbol);
    }
    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }

    void         show(bool transparency = false) {
        m_backgroundTransparency = transparency;
        if(m_backgroundTransparency) tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        else                         tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = true;
        m_clicked = false;
        pic_Antenna->show(m_backgroundTransparency, false);
        updateStation(m_staNr);
        updateOffTime(m_offTime);
        updateBitRate(m_bitRate);
        if (m_ipAddr)
            writeIpAddr(m_ipAddr);
        else
            writeIpAddr("");
    }
    void hide() {
        tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        m_enabled = false;
    }
    void enable() { m_enabled = true; }
    void disable() { m_enabled = false; }
    void setBGcolor(uint32_t color) { m_bgColor = color; }
    void updateStation(uint16_t staNr) {
        m_staNr = staNr;
        char buff[10];
        sprintf(buff, "%03d", m_staNr);
        txt_StaNr->setText(buff);
        txt_StaNr->show(m_backgroundTransparency, false);
    }
    void setStationNrColor(uint16_t stationColor) { m_stationColor = stationColor; }
    void updateFlag(const char* flag) {
        if (flag) {
            pic_Flag->setAlternativPicturePath("/flags/unknown.jpg");
            pic_Flag->setPicturePath(flag);
            pic_Flag->show(m_backgroundTransparency, false);
        } else {
            pic_Flag->hide();
        }
    }
    void updateOffTime(uint16_t offTime) {
        m_offTime = offTime;
        if (!m_enabled) return;
        char buff[15];
        sprintf(buff, "%d:%02d", m_offTime / 60, m_offTime % 60);
        if (m_offTime) {
            txt_OffTimer->setTextColor(TFT_RED);
            txt_OffTimer->setText(buff);
            txt_OffTimer->show(m_backgroundTransparency, false);
            pic_Hourglass->setPicturePath(m_hourGlassymbol[1]);
            pic_Hourglass->show(m_backgroundTransparency, false);
        } else {
            txt_OffTimer->setTextColor(TFT_DEEPSKYBLUE);
            txt_OffTimer->setText(buff);
            txt_OffTimer->show(m_backgroundTransparency, false);
            pic_Hourglass->setPicturePath(m_hourGlassymbol[0]);
            pic_Hourglass->show(m_backgroundTransparency, false);
        }
    }
    void updateTC(uint8_t timeCounter) {
        m_timeCounter = timeCounter;
        if (!m_enabled) return;
        if (!m_timeCounter) {
            updateBitRate(m_bitRate);
        } else {
            uint16_t x0 = s_BitRate.x;
            uint16_t x1x2 = round(s_BitRate.x + ((float)((s_BitRate.w) / 10) * timeCounter)) - 1;
            uint16_t y0y1 = m_y + m_h - 5;
            uint16_t y2 = round((m_y + m_h - 5) - ((float)(m_h - 6) / 10) * timeCounter);
            if (m_backgroundTransparency) {
                tft.copyFramebuffer(1, 0, s_BitRate.x, m_y, s_BitRate.w, m_h);
            } else {
                tft.fillRect(s_BitRate.x, m_y, s_BitRate.w, m_h, m_bgColor);
            }
            tft.fillTriangle(x0, y0y1, x1x2, y0y1, x1x2, y2, TFT_RED);
        }
    }

    void updateBitRate(uint32_t bitRate) {
        m_bitRate = bitRate / 1000; // KBit/s
        if (!m_enabled) return;
        char sbr[10];
        itoa(m_bitRate, sbr, 10);
        if (m_bitRate < 1000) {
            strcat(sbr, "K");
        } else {
            sbr[2] = sbr[1];
            sbr[1] = '.';
            sbr[3] = 'M';
            sbr[4] = '\0';
        }
        txt_BitRate->setText(sbr);
        txt_BitRate->show(m_backgroundTransparency, false);
    }
    void setBitRateColor(uint16_t bitRateColor) {
        m_bitRateColor = bitRateColor;
        txt_BitRate->setBorderColor(m_bitRateColor);
        txt_BitRate->setTextColor(m_bitRateColor);
    }
    void setIpAddr(const char* ipAddr) {
        if (!ipAddr) return;
        x_ps_free(&m_ipAddr);
        m_ipAddr = strdup(ipAddr);
    }
    void writeIpAddr(const char* ipAddr) {
        char myIP[30] = "IP:";
        strcat(myIP, ipAddr);
        txt_IpAddr->setText(myIP, true, true);
        txt_IpAddr->show(m_backgroundTransparency, false);
    }
    void setIpAddrColor(uint16_t ipAddrColor) {
        m_ipAddrColor = ipAddrColor;
        txt_IpAddr->setTextColor(m_ipAddrColor);
    }
    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        uint8_t pos = 0;
        if (graphicObjects_OnClick) graphicObjects_OnClick(m_name, pos);
        pic_Antenna->positionXY(x, y); // transfer the position to the graphic objects
        txt_StaNr->positionXY(x, y);
        pic_Flag->positionXY(x, y);
        pic_Hourglass->positionXY(x, y);
        txt_OffTimer->positionXY(x, y);
        txt_BitRate->positionXY(x, y);
        txt_IpAddr->positionXY(x, y);
        if (!m_enabled) return false;
        return true;
    }
    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease(m_name, m_ra);
        return true;
    }

  private:
}; // displayFooter
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
class messageBox : public RegisterTable {
  private:
    ps_ptr<char> m_name;
    ps_ptr<char> m_text;
    bool         m_enabled = false;
    bool         m_clicked = false;
    bool         m_narrow = false;
    bool         m_noWrap = false;
    uint16_t     m_x = 0;
    uint16_t     m_y = 0;
    uint16_t     m_w = 0;
    uint16_t     m_h = 0;
    uint16_t     m_pl = 0;
    uint16_t     m_pr = 0;
    uint16_t     m_pt = 0;
    uint16_t     m_pb = 0;
    releasedArg  m_ra;
    uint16_t     m_bgColor = TFT_YELLOW;
    uint16_t     m_textColor = TFT_DARKRED;
    bool         m_backgroundTransparency = false;
    bool         m_saveBackground = false;
    textbox*     txt_msgBox = new textbox("msgBox txt");

#if TFT_CONTROLLER < 2 // 320 x 240px

    struct p {
        uint16_t x = 320 / 4;
        uint16_t y = 240 / 4;
        uint16_t w = 320 / 2;
        uint16_t h = 240 / 2;
        uint8_t  pl = 5;
        uint8_t  pr = 5;
        uint8_t  pt = 5;
        uint8_t  pb = 5;
    } const m_win;

#elif TFT_CONTROLLER < 7 // 480 x 320px
    struct p {
        uint16_t x = 480 / 4;
        uint16_t y = 320 / 4;
        uint16_t w = 480 / 2;
        uint16_t h = 320 / 2;
        uint8_t  pl = 10;
        uint8_t  pr = 10;
        uint8_t  pt = 10;
        uint8_t  pb = 10;
    } const m_win;
#else                    // 800 x 480px
    struct p {
        uint16_t x = 800 / 4;
        uint16_t y = 480 / 4;
        uint16_t w = 800 / 2;
        uint16_t h = 480 / 2;
        uint8_t  pl = 20;
        uint8_t  pr = 20;
        uint8_t  pt = 20;
        uint8_t  pb = 20;
    } const m_win;
#endif

  public:
    messageBox(const char* name) {
        m_name = name;
        m_x = m_win.w;
        m_y = m_win.y;
    }
    ~messageBox() { delete txt_msgBox; }

    // clang-format off
    void begin(int16_t x, int16_t y, int16_t w, int16_t h) {
        if (x > -1) m_x = x; else m_x = m_win.x;
        if (y > -1) m_y = y; else m_y = m_win.y;
        if (w > -1) m_w = w; else m_w = m_win.w;
        if (h > -1) m_h = h; else m_h = m_win.h;
        txt_msgBox->begin(m_x, m_y, m_w, m_h, m_pl, m_pr, m_pt, m_pb);
        txt_msgBox->setTextColor(m_textColor);
        txt_msgBox->setBGcolor(m_bgColor);
    }
    // clang-format on

    ps_ptr<char> getName() { return m_name; }
    bool         isEnabled() { return m_enabled; }
    void         disable() { m_enabled = false; }
    void         setBGcolor(uint32_t color) { m_bgColor = color; }

    void setText(const char* txt, bool narrow = false, bool noWrap = true) { // prepare a text, wait of show() to write it
        m_text = txt;
        m_narrow = narrow;
        m_noWrap = noWrap;
        txt_msgBox->setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
        txt_msgBox->setFont(0); // auto
        txt_msgBox->setText(m_text.c_get(), m_narrow, m_noWrap);
    }

    void show() { txt_msgBox->show(m_backgroundTransparency, m_saveBackground); }

    void hide() {
        if (m_saveBackground) {
            tft.copyFramebuffer(2, 1, m_x, m_y, m_w, m_h); // restore background
        }
        if (m_backgroundTransparency) {
            tft.copyFramebuffer(1, 0, m_x, m_y, m_w, m_h);
        } else {
            tft.fillRect(m_x, m_y, m_w, m_h, m_bgColor);
        }
        m_enabled = false;
    }

    bool positionXY(uint16_t x, uint16_t y) {
        if (x < m_x) return false;
        if (y < m_y) return false;
        if (x > m_x + m_w) return false;
        if (y > m_y + m_h) return false;
        if (m_enabled) m_clicked = true;
        if (!m_enabled) return false;
        return true;
    }

    bool released() {
        if (!m_enabled) return false;
        if (!m_clicked) return false;
        m_clicked = false;
        if (graphicObjects_OnRelease) graphicObjects_OnRelease((const char*)m_name.c_get(), m_ra);
        return true;
    }
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
