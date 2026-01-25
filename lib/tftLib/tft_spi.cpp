// first release on 09/2019
// updated on Apr 26 2025

#include "tft_spi.h"
#include "Arduino.h"
#if TFT_CONTROLLER < 7
SPIClass*   SPItransfer;

#define __malloc_heap_psram(size) heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL)


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_SPI::TFT_SPI(SPIClass& spiInstance, int csPin){
    _freq = 20000000;
    _TFT_CS = csPin;
    spi_TFT = &spiInstance;
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
}
TFT_SPI::~TFT_SPI() {
    if(m_framebuffer[0]) {free(m_framebuffer[0]);}
    if(m_framebuffer[1]) {free(m_framebuffer[1]);}
    if(m_framebuffer[2]) {free(m_framebuffer[2]);}
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::loop(){
    GIF_loop();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setTFTcontroller(uint8_t TFTcontroller) {
    _TFTcontroller = TFTcontroller; // 0=ILI9341, 1=HX8347D, 2=ILI9486(a), 3=ILI9486(b), 4= ILI9488, 5=ST7796

    if(_TFTcontroller == ILI9341)   { m_h_res = 320; m_v_res = 240; _rotation = 0;}
    if(_TFTcontroller == HX8347D)   { m_h_res = 320; m_v_res = 240; _rotation = 0;}
    if(_TFTcontroller == ILI9486a)  { m_h_res = 480; m_v_res = 320; _rotation = 0;}
    if(_TFTcontroller == ILI9486b)  { m_h_res = 480; m_v_res = 320; _rotation = 0;}
    if(_TFTcontroller == ILI9488)   { m_h_res = 480; m_v_res = 320; _rotation = 0;}
    if(_TFTcontroller == ST7796 )   { m_h_res = 480; m_v_res = 320; _rotation = 0;}
    if(_TFTcontroller == ST7796RPI) { m_h_res = 480; m_v_res = 320; _rotation = 0;}

    m_framebuffer[0] = (uint16_t*)ps_malloc(m_h_res * m_v_res * 2);
    if(!m_framebuffer[0]) {if(tft_info) tft_info("Error allocating memory framebuffer 0"); return; }
    memset(m_framebuffer[0], 0, m_h_res * m_v_res * 2);

    m_framebuffer[1] = (uint16_t*)ps_malloc(m_h_res * m_v_res * 2);
    if(!m_framebuffer[1]) {if(tft_info) tft_info("Error allocating memory framebuffer 1"); return; }
    memset(m_framebuffer[1], 0, m_h_res * m_v_res * 2);

    m_framebuffer[2] = (uint16_t*)ps_malloc(m_h_res * m_v_res * 2);
    if(!m_framebuffer[2]) {if(tft_info) tft_info("Error allocating memory framebuffer 2"); return; }
    memset(m_framebuffer[2], 0, m_h_res * m_v_res * 2);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setDiaplayInversion(uint8_t dispInv) {
    _displayInversion = dispInv;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setFrequency(uint32_t f) {
    if(f > 80000000) f = 80000000;
    _freq = f; // overwrite default
    spi_TFT->setFrequency(_freq);
    SPIset = SPISettings(_freq, MSBFIRST, SPI_MODE0);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::startWrite(void) {
    spi_TFT->beginTransaction(SPIset);
    TFT_CS_LOW();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::endWrite(void) {
    TFT_CS_HIGH();
    spi_TFT->endTransaction();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Return the size of the display (per current rotation)
int16_t TFT_SPI::width(void) const { return m_h_res; }
int16_t TFT_SPI::height(void) const { return m_v_res; }
uint8_t TFT_SPI::getRotation(void) const { return _rotation; }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::writeCommand(uint16_t cmd) {
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 || _TFTcontroller == HX8347D || _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) spi_TFT->write(cmd);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI) spi_TFT->write16(cmd);
    TFT_DC_HIGH();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::readCommand() {
    uint16_t ret = 0;
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 || _TFTcontroller == HX8347D || _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) ret = spi_TFT->transfer(0);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI) ret = spi_TFT->transfer16(0);
    TFT_DC_HIGH();
    return ret;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::begin(uint8_t DC) {
    SPIset = SPISettings(_freq, MSBFIRST, SPI_MODE0);
    String info = "";

    _TFT_DC = DC;

    pinMode(_TFT_DC, OUTPUT);
    digitalWrite(_TFT_DC, LOW);
    pinMode(_TFT_CS, OUTPUT);
    digitalWrite(_TFT_CS, HIGH);
    init(); //
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::writePixels(uint16_t* colors, uint32_t len) {
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)) {
        uint32_t i = 0;
        while(len) {
            write24BitColor(*(colors + i));
            i++;
            len--;
        }
    }
    else { spi_TFT->writePixels((uint8_t*)colors, len * 2); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::writeColor(uint16_t color, uint32_t len) {
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)) {
        uint8_t r = (color & 0xF800) >> 8;
        uint8_t g = (color & 0x07E0) >> 3;
        uint8_t b = (color & 0x001F) << 3;
        uint8_t c[3] = {r, g, b};
        spi_TFT->writePattern(c, 3, len);
    }
    else {
        uint8_t c[2];
        c[0] = (color & 0xFF00) >> 8;
        c[1] = color & 0x00FF;
        spi_TFT->writePattern(c, 2, len);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::write24BitColor(uint16_t color) {
    spi_TFT->write((color & 0xF800) >> 8); // r
    spi_TFT->write((color & 0x07E0) >> 3); // g
    spi_TFT->write((color & 0x001F) << 3); // b
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::writePixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) || (x >= m_v_res) || (y < 0) || (y >= m_h_res)) return;
    setAddrWindow(x, y, 1, 1);
    switch(_TFTcontroller) {
        case ILI9341: spi_TFT->write16(color); break;
        case HX8347D:
            writeCommand(0x22); spi_TFT->write16(color);
            break;
        case ILI9486a:
            writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);
            break;
        case ILI9486b:
            writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);
            break;
        case ILI9488:
            writeCommand(ILI9488_RAMWR); write24BitColor(color);
            break;
        case ST7796:
            writeCommand(ST7796_RAMWR); write24BitColor(color);
            break;
        case ST7796RPI:
            writeCommand(ST7796_RAMWR); spi_TFT->write16(color);
            break;
        default:
            if(tft_info) tft_info("unknown tft controller");
            break;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::copyFramebuffer(uint8_t source, uint8_t destination, uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for(uint16_t j = y; j < y + h; j++) {
        memcpy(m_framebuffer[destination] + j * m_h_res + x, m_framebuffer[source] + j * m_h_res + x, w * 2);
    }
    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* data) {
    for(uint16_t j = 0; j < h; j++) {
        memcpy(data + j * w * 2, m_framebuffer[0] + j * m_h_res + x + y * m_h_res, w * sizeof(uint16_t));
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
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
    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y0; j < y1; j++) {
        writeColor(color, x1 - x0);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::fillScreen(uint16_t color) {
    fill(m_framebuffer[0], m_framebuffer[0] + (m_h_res * m_v_res), color);

    startWrite();
    setAddrWindow(0, 0, m_h_res, m_v_res);
    writeColor(color, m_h_res * m_v_res);
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if(x0 < 0 || x0 >= m_h_res || x1 < 0 || x1 >= m_h_res || x2 < 0 || x2 >= m_h_res || y0 < 0 || y0 >= m_v_res || y1 < 0 || y1 >= m_v_res || y2 < 0 || y2 >= m_v_res) return;

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
    int16_t x = std::min({x0, x1, x2});
    int16_t y = std::min({y0, y1, y2});
    int16_t w = std::max({x0, x1, x2}) - x + 1;
    int16_t h = std::max({y0, y1, y2}) - y + 1;

    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    if(x0 < 0 || x0 >= m_h_res || x1 < 0 || x1 >= m_h_res || x2 < 0 || x2 >= m_h_res || y0 < 0 || y0 >= m_v_res || y1 < 0 || y1 >= m_v_res || y2 < 0 || y2 >= m_v_res) return;

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
    int16_t x = std::max((int16_t)0, x_min);
    int16_t w = std::min((int16_t)(m_h_res - 1), x_max) - x + 1;
    int16_t y = std::max((int16_t)0, y_min);
    int16_t h = std::min((int16_t)(m_v_res - 1), y_max) - y + 1;


    // Dreieck in zwei Teile zerlegen (oben und unten)
    if (y1 == y2) { // Sonderfall: flaches unteres Dreieck
        for (int16_t i = y0; i <= y1; ++i) {
            int16_t x_start = x0 + (x1 - x0) * (i - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (i - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, i);
        }
    } else if (y0 == y1) { // Sonderfall: flaches oberes Dreieck
        for (int16_t i = y0; i <= y2; ++i) {
            int16_t x_start = x0 + (x2 - x0) * (i - y0) / (y2 - y0);
            int16_t x_end = x1 + (x2 - x1) * (i - y1) / (y2 - y1);
            drawHorizontalLine(x_start, x_end, i);
        }
    } else { // Allgemeiner Fall: Dreieck wird in zwei Teile aufgeteilt
        for (int16_t i = y0; i <= y1; ++i) { // Unterer Teil
            int16_t x_start = x0 + (x1 - x0) * (i - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (i - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, i);
        }
        for (int16_t i = y1; i <= y2; ++i) { // Oberer Teil
            int16_t x_start = x1 + (x2 - x1) * (i - y1) / (y2 - y1);
            int16_t x_end = x0 + (x2 - x0) * (i - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, i);
        }
    }
    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color) {
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
    drawLine(Xpos, Ypos, Xpos + Width, Ypos, Color, m_framebuffer[0], m_h_res); // Oben
    drawLine(Xpos + Width - 1, Ypos, Xpos + Width - 1, Ypos + Height - 1, Color, m_framebuffer[0], m_h_res); // Rechts
    drawLine(Xpos, Ypos + Height - 1, Xpos + Width - 1, Ypos + Height - 1, Color, m_framebuffer[0], m_h_res); // Unten
    drawLine(Xpos, Ypos + Height, Xpos, Ypos, Color, m_framebuffer[0], m_h_res); // Links

    // Aktualisierung des gezeichneten Bereichs
    int16_t x = std::min((int)Xpos, Xpos + Width);
    int16_t y = std::min((int)Ypos, Ypos + Height);
    int16_t w = std::max((int)Xpos, Xpos + Width) - x;
    int16_t h = std::max((int)Ypos, Ypos + Height) - y;

    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // helper function: Calculate circular drawing for the corners
    auto drawCircleQuadrant = [&](int16_t cx, int16_t cy, int16_t r, uint8_t quadrant) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            if (quadrant & 0x1) m_framebuffer[0][(cy - y) * m_h_res + (cx + x)] = color; // up right
            if (quadrant & 0x2) m_framebuffer[0][(cy + y) * m_h_res + (cx + x)] = color; // down right
            if (quadrant & 0x4) m_framebuffer[0][(cy + y) * m_h_res + (cx - x)] = color; // down left
            if (quadrant & 0x8) m_framebuffer[0][(cy - y) * m_h_res + (cx - x)] = color; // up left

            if (quadrant & 0x10) m_framebuffer[0][(cy - x) * m_h_res + (cx + y)] = color; // up right (90° rotated)
            if (quadrant & 0x20) m_framebuffer[0][(cy + x) * m_h_res + (cx + y)] = color; // down right (90° rotated)
            if (quadrant & 0x40) m_framebuffer[0][(cy + x) * m_h_res + (cx - y)] = color; // down left (90° rotated)
            if (quadrant & 0x80) m_framebuffer[0][(cy - x) * m_h_res + (cx - y)] = color; // up left (90° rotated)

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

    // draw horizontal lines above and below the quarter circles
    for (int16_t i = x + r; i < x + w - r; i++) { // upper and lower horizontal lines
        m_framebuffer[0][y * m_h_res + i] = color; // above
        m_framebuffer[0][(y + h - 1) * m_h_res + i] = color; // below
    }
    for (int16_t i = y + r; i < y + h - r; i++) { // vertical lines
        m_framebuffer[0][i * m_h_res + x] = color; // left
        m_framebuffer[0][i * m_h_res + (x + w - 1)] = color; // right
    }

    // fill the area between the quarter circles
    drawCircleQuadrant(x + w - r - 1, y + r, r, 0x1 | 0x10); // Oben rechts
    drawCircleQuadrant(x + w - r - 1, y + h - r - 1, r, 0x2 | 0x20); // Unten rechts
    drawCircleQuadrant(x + r, y + h - r - 1, r, 0x4 | 0x40); // Unten links
    drawCircleQuadrant(x + r, y + r, r, 0x8 | 0x80); // Oben links

    // update the drawn area
    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // Helper function: Calculate circular filling for the corners
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
    startWrite();
    setAddrWindow(x, y, w, h);
    for(int16_t j = y; j < y + h; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, w);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::drawCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    if (cx + r < 0 || cx - r >= m_h_res || cy + r < 0 || cy - r >= m_v_res) {
        return; // Circle is completely outside, so don't draw anything
    }
    // Bresenham-Algorithm for circles
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    auto setPixelSafe = [&](int16_t x, int16_t y, uint16_t color) { // Set pixel if it is within the framebuffer
        if (x >= 0 && x < m_h_res && y >= 0 && y < m_v_res) {
            m_framebuffer[0][y * m_h_res + x] = color;
        }
    };

    // set the initial pixels
    setPixelSafe(cx, cy + r, color); // upper pixel
    setPixelSafe(cx, cy - r, color); // lower pixel
    setPixelSafe(cx + r, cy, color); // right pixel
    setPixelSafe(cx - r, cy, color); // left pixel

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // Draw points in the eight symmetry axes
        setPixelSafe(cx + x, cy + y, color); // Quadrant 1
        setPixelSafe(cx - x, cy + y, color); // Quadrant 2
        setPixelSafe(cx + x, cy - y, color); // Quadrant 3
        setPixelSafe(cx - x, cy - y, color); // Quadrant 4
        setPixelSafe(cx + y, cy + x, color); // Quadrant 5
        setPixelSafe(cx - y, cy + x, color); // Quadrant 6
        setPixelSafe(cx + y, cy - x, color); // Quadrant 7
        setPixelSafe(cx - y, cy - x, color); // Quadrant 8
    }

    int16_t x1 = std::max(cx - r, 1) - 1;
    int16_t y1 = std::max(cy - r, 1) - 1;
    int16_t w1 = std::min(cx + r, m_h_res - 1) - x1 + 1;
    int16_t h1 = std::min(cy + r, m_v_res - 1) - y1 + 1;

    // Update of the drawn area
    startWrite();
    setAddrWindow(x1, y1, w1, h1);
    for(int16_t j = y1; j < y1 + h1; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x1, w1);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::fillCircle(int16_t cx, int16_t cy, uint16_t r, uint16_t color){
    if (cx + r < 0 || cx - r >= m_h_res || cy + r < 0 || cy - r >= m_v_res) {
        return; // Circle is completely outside, so don't draw anything
    }
    // Bresenham-Algorithmus für Kreise
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;


    auto setPixelSafe = [&](int16_t x, int16_t y, uint16_t color) {
        if (x >= 0 && x < m_h_res && y >= 0 && y < m_v_res) {
            m_framebuffer[0][y * m_h_res + x] = color;
        }
    };

    // Fülle die erste vertikale Linie durch den Mittelpunkt
    for (int16_t i = cy - r; i <= cy + r; i++) {
        setPixelSafe(cx, i, color);
    }

    while (x <= y) {
        // Fülle horizontale Linien für alle acht Symmetrieachsen
        for (int16_t i = cx - x; i <= cx + x; i++) {
            setPixelSafe(i, cy + y, color); // Unten +y
            setPixelSafe(i, cy - y, color); // Oben -y
        }
        for (int16_t i = cx - y; i <= cx + y; i++) {
            setPixelSafe(i, cy + x, color); // Rechts +x
            setPixelSafe(i, cy - x, color); // Links -x
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

    // Update of the drawn area
    int16_t x1 = std::max(cx - r, 1) - 1;
    int16_t y1 = std::max(cy - r, 1) - 1;
    int16_t w1 = std::min(cx + r, m_h_res - 1) - x1 + 1;
    int16_t h1 = std::min(cy + r, m_v_res - 1) - y1 + 1;

    // Update of the drawn area
    startWrite();
    setAddrWindow(x1, y1, w1, h1);
    for(int16_t j = y1; j < y1 + h1; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x1, w1);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::readRect(int32_t x, int32_t y, int32_t w, uint16_t* data) {

    memcpy(data, m_framebuffer[0] + y * m_h_res + x, w * sizeof(uint16_t));
    return;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setFont(uint16_t font) {

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
/*******************************************************************************************************************************************************************************************************
 *                                                                                                                                                                                                     *
 *        ⏫⏫⏫⏫⏫⏫                                       W R I T E    T E X T    R E L A T E D    F U N C T I O N S                                                      ⏫⏫⏫⏫⏫⏫             *
 *                                                                                                                                                                                                     *
 * *****************************************************************************************************************************************************************************************************
 */
void TFT_SPI::writeInAddrWindow(const uint8_t* bmi, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) {

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
        if(bit) { return (int32_t) _textColor;}
        return (int32_t)-1;  // _backColor, -1 is transparent
    };

    bitreader(bmi);

    if(posX >= m_h_res) {log_e("%s %i posX %i", __FILE__, __LINE__, posX); return;}
    if(posY >= m_v_res) {log_e("%s %i posY %i", __FILE__, __LINE__, posY); return;}
    if(posX + width >= m_h_res) {log_e("%s %i posX %i, width %i", __FILE__, __LINE__, posX, width); return;}
    if(posY + height >= m_v_res) {log_e("%s %i posY %i, height %i", __FILE__, __LINE__, posY, height); return;}


    for(int16_t j = posY; j < posY + height; j++) {
        for(int16_t i = posX; i < posX + width; i++) {
            int32_t color = bitreader(0);
            if(color == -1) {
                continue;
            }
            m_framebuffer[0][j * m_h_res + i] = color;
        }
    }

    startWrite();
    setAddrWindow(posX, posY, width, height);
    for(int16_t j = posY; j < posY + height; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + posX, width);
    }
    endWrite();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// The function is passed a string and two arrays of length strlen(str + 1). This is definitely enough, since ANSI sequences or non-ASCII UTF-8 characters are always greater than 1.
// For each printable character found in the LookUp table, the codepoint is written to the next position in the charr. The number of printable characters is increased by one.
// If an ANSI sequence is found, the color found is written into colorArr at the position of the current character. The return value is the number of printable character.
uint16_t TFT_SPI::analyzeText(const char* str, uint16_t* chArr, uint16_t* colorArr, uint16_t startColor) {
    uint16_t chLen = 0;
    uint16_t idx = 0;
    int32_t  codePoint = -1;
    colorArr[0] = startColor;

    while((uint8_t)str[idx] != 0) {
        colorArr[chLen + 1] = colorArr[chLen]; // set next color to the last one
        switch((uint8_t)str[idx]) {
            case '\033': // ANSI sequence
                if(strncmp(str + idx, "\033[30m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_BLACK;        break;} // ANSI_ESC_BLACK
                if(strncmp(str + idx, "\033[31m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_RED;          break;} // ANSI_ESC_RED
                if(strncmp(str + idx, "\033[32m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_GREEN;        break;} // ANSI_ESC_GREEN
                if(strncmp(str + idx, "\033[33m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_YELLOW;       break;} // ANSI_ESC_YELLOW
                if(strncmp(str + idx, "\033[34m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_BLUE;         break;} // ANSI_ESC_BLUE
                if(strncmp(str + idx, "\033[35m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_MAGENTA;      break;} // ANSI_ESC_MAGENTA
                if(strncmp(str + idx, "\033[36m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_CYAN;         break;} // ANSI_ESC_CYAN
                if(strncmp(str + idx, "\033[37m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_WHITE;        break;} // ANSI_ESC_WHITE
                if(strncmp(str + idx, "\033[38;5;130m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_BROWN;        break;} // ANSI_ESC_BROWN
                if(strncmp(str + idx, "\033[38;5;214m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_ORANGE;       break;} // ANSI_ESC_ORANGE
                if(strncmp(str + idx, "\033[90m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_GREY;         break;} // ANSI_ESC_GREY
                if(strncmp(str + idx, "\033[91m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTRED;     break;} // ANSI_ESC_LIGHTRED
                if(strncmp(str + idx, "\033[92m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTGREEN;   break;} // ANSI_ESC_LIGHTGREEN
                if(strncmp(str + idx, "\033[93m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTYELLOW;  break;} // ANSI_ESC_LIGHTYELLOW
                if(strncmp(str + idx, "\033[94m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTBLUE;    break;} // ANSI_ESC_LIGHTBLUE
                if(strncmp(str + idx, "\033[95m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTMAGENTA; break;} // ANSI_ESC_LIGHTMAGENTA
                if(strncmp(str + idx, "\033[96m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTCYAN;    break;} // ANSI_ESC_LIGHTCYAN
                if(strncmp(str + idx, "\033[97m", 5) == 0)                  {idx += 5;  colorArr[chLen] = TFT_LIGHTGREY;    break;} // ANSI_ESC_LIGHTGREY
                if(strncmp(str + idx, "\033[38;5;52m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_DARKRED;      break;} // ANSI_ESC_DARKRED
                if(strncmp(str + idx, "\033[38;5;22m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_DARKGREEN;    break;} // ANSI_ESC_DARKGREEN
                if(strncmp(str + idx, "\033[38;5;136m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_DARKYELLOW;   break;} // ANSI_ESC_DARKYELLOW
                if(strncmp(str + idx, "\033[38;5;17m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_DARKBLUE;     break;} // ANSI_ESC_DARKBLUE
                if(strncmp(str + idx, "\033[38;5;53m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_DARKMAGENTA;  break;} // ANSI_ESC_DARKMAGENTA
                if(strncmp(str + idx, "\033[38;5;23m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_DARKCYAN;     break;} // ANSI_ESC_DARKCYAN
                if(strncmp(str + idx, "\033[38;5;240m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_DARKGREY;     break;} // ANSI_ESC_DARKGREY
                if(strncmp(str + idx, "\033[38;5;166m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_DARKORANGE;   break;} // ANSI_ESC_DARKORANGE
                if(strncmp(str + idx, "\033[38;5;215m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_LIGHTORANGE;  break;} // ANSI_ESC_LIGHTORANGE
                if(strncmp(str + idx, "\033[38;5;129m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_PURPLE;       break;} // ANSI_ESC_PURPLE
                if(strncmp(str + idx, "\033[38;5;213m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_PINK;         break;} // ANSI_ESC_PINK
                if(strncmp(str + idx, "\033[38;5;190m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_LIME;         break;} // ANSI_ESC_LIME
                if(strncmp(str + idx, "\033[38;5;25m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_NAVY;         break;} // ANSI_ESC_NAVY
                if(strncmp(str + idx, "\033[38;5;51m", 10) == 0)            {idx += 10; colorArr[chLen] = TFT_AQUAMARINE;   break;} // ANSI_ESC_AQUAMARINE
                if(strncmp(str + idx, "\033[38;5;189m", 11) == 0)           {idx += 11; colorArr[chLen] = TFT_LAVENDER;     break;} // ANSI_ESC_LAVENDER
                if(strncmp(str + idx, "\033[38;2;210;180;140m", 19) == 0)   {idx += 19; colorArr[chLen] = TFT_LIGHTBROWN;   break;} // ANSI_ESC_LIGHTBROWN
                if(strncmp(str + idx, "\033[0m", 4) == 0)                   {idx += 4;                                      break;} // ANSI_ESC_RESET       unused
                if(strncmp(str + idx, "\033[1m", 4) == 0)                   {idx += 4;                                      break;} // ANSI_ESC_BOLD        unused
                if(strncmp(str + idx, "\033[2m", 4) == 0)                   {idx += 4;                                      break;} // ANSI_ESC_FAINT       unused
                if(strncmp(str + idx, "\033[3m", 4) == 0)                   {idx += 4;                                      break;} // ANSI_ESC_ITALIC      unused
                if(strncmp(str + idx, "\033[4m", 4) == 0)                   {idx += 4;                                      break;} // ANSI_ESC_UNDERLINE   unused
                {if(tft_info) tft_info("unknown ANSI ESC SEQUENCE");         idx += 4;                                      break;} // unknown
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
                else log_w("character 0x%02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 3;
                break;
            case 0xE1 ... 0xEF: idx += 3; break;
            case 0xF0 ... 0xFF:
                codePoint = -1;
                if(!strncmp(str + idx, "🟢", 4)) {codePoint = 0xF9A2;}
                if(!strncmp(str + idx, "🟡", 4)) {codePoint = 0xF9A1;}
                if(!strncmp(str + idx, "🔴", 4)) {codePoint = 0xF9B4;}
                if(!strncmp(str + idx, "🔵", 4)) {codePoint = 0xF9B5;}
                if(!strncmp(str + idx, "🟠", 4)) {codePoint = 0xF9A0;}
                if(!strncmp(str + idx, "🟣", 4)) {codePoint = 0xF9A3;}
                if(!strncmp(str + idx, "🟤", 4)) {codePoint = 0xF9A4;}
                if(!strncmp(str + idx, "🟩", 4)) {codePoint = 0xF9A9;}
                if(!strncmp(str + idx, "🟨", 4)) {codePoint = 0xF9A8;}
                if(!strncmp(str + idx, "🟥", 4)) {codePoint = 0xF9A5;}
                if(!strncmp(str + idx, "🟦", 4)) {codePoint = 0xF9A6;}
                if(!strncmp(str + idx, "🟧", 4)) {codePoint = 0xF9A7;}
                if(!strncmp(str + idx, "🟪", 4)) {codePoint = 0xF9AA;}
                if(!strncmp(str + idx, "🟫", 4)) {codePoint = 0xF9AB;}
                if(codePoint != -1) {
                    chArr[chLen] = codePoint;
                    chLen += 1;
                    idx += 4;
                    break;
                }
                else{
                    log_w("character 0x%02X%02X%02X%02X  is not in table", str[idx], str[idx + 1], str[idx + 2], str[idx + 3]);
                    idx += 4;
                }
                break;
            default: idx++; break;
        }
    }
    return chLen;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::getLineLength(const char* txt, bool narrow){
    // returns the length of the string in pixels
    uint16_t pxLength = 0;
    uint16_t idx = 0;
    bool     isEmoji = false;
    while((uint8_t)txt[idx] != 0) {
        isEmoji = false;
        if(txt[idx] == 0xF0) { // UTF8
            if(!strncmp(txt + idx, "🟢", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟡", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🔴", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🔵", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟠", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟣", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟤", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟩", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟨", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟥", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟦", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟧", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟪", 4)) {isEmoji = true;}
            if(!strncmp(txt + idx, "🟫", 4)) {isEmoji = true;}
            if(isEmoji){
                uint16_t fh = m_current_font.font_height;
                pxLength += fh - fh / 3; // high as wide - 1/3
                idx += 4;
                continue;
            }
        }
        uint16_t glyphPos = m_current_font.lookup_table[(uint8_t)txt[idx]];
        pxLength += m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        int ofsX = m_current_font.glyph_dsc[glyphPos].ofs_x;
        if(ofsX < 0) ofsX = 0;
        if(!narrow) pxLength += ofsX;
        idx++;
  }
  return pxLength;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap){
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
    bool     isEmoji = false;
    while(cpArr[idx] != 0) {
        *usedPxLength = pxLength;
        if(cpArr[idx] == 0x20 || cpArr[idx - 1] == '-') {
            lastSpacePos = drawableChars;
            lastUsedPxLength = pxLength;
        }
        isEmoji = false;
        if((cpArr[idx] & 0xFF00) == 0xF900){ // This is a emoji, width is the same as height
            if(cpArr[idx] == 0xF9A2) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A1) {isEmoji = true;}
            if(cpArr[idx] == 0xF9B4) {isEmoji = true;}
            if(cpArr[idx] == 0xF9B5) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A0) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A3) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A4) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A9) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A8) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A5) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A6) {isEmoji = true;}
            if(cpArr[idx] == 0xF9A7) {isEmoji = true;}
            if(cpArr[idx] == 0xF9AA) {isEmoji = true;}
            if(cpArr[idx] == 0xF9AB) {isEmoji = true;}
            if(isEmoji){
                uint16_t fh = m_current_font.font_height;
                pxLength += fh - fh / 3; // high as wide - 1/3
            }
        }
        else{ // This is a valid character, get the width from the fonts table
            glyphPos = m_current_font.lookup_table[cpArr[idx]];
            pxLength += m_current_font.glyph_dsc[glyphPos].adv_w / 16;
            int ofsX = m_current_font.glyph_dsc[glyphPos].ofs_x;
            if(ofsX < 0) ofsX = 0;
            if(!narrow) pxLength += ofsX;
        }
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
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap){
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
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
size_t TFT_SPI::writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align, uint8_t v_align, bool narrow, bool noWrap, bool autoSize) {
    // autoSize choose the biggest possible font
    uint16_t idx = 0;
    uint16_t utfPosArr[strlen(str) + 1] = {0};
    uint16_t colorArr[strlen(str) + 1] = {0};
    uint16_t strChLength = 0; // nr. of chars
    uint8_t  nrOfLines = 1;
    bool     isEmoji = false;

    //-------------------------------------------------------------------------------------------------------------------
    auto drawEmoji= [&](uint16_t idx, uint16_t x, uint16_t y) { // lambda
        uint8_t emoji = (utfPosArr[idx] & 0x00FF);
        uint16_t color = 0;
        char shape = 'x';
        switch(emoji){
            case 0xA2: color = TFT_GREEN;    shape = 'c'; break;     // UTF-8: "🟢"
            case 0xA1: color = TFT_YELLOW;   shape = 'c'; break;     // UTF-8: "🟡"
            case 0xB4: color = TFT_RED;      shape = 'c'; break;     // UTF-8: "🔴"
            case 0xB5: color = TFT_BLUE;     shape = 'c'; break;     // UTF-8: "🔵"
            case 0xA0: color = TFT_ORANGE;   shape = 'c'; break;     // UTF-8: "🟠"
            case 0xA3: color = TFT_VIOLET;   shape = 'c'; break;     // UTF-8: "🟣"
            case 0xA4: color = TFT_BROWN;    shape = 'c'; break;     // UTF-8: "🟤"
            case 0xA9: color = TFT_GREEN;    shape = 's'; break;     // UTF-8: "🟩"
            case 0xA8: color = TFT_YELLOW;   shape = 's'; break;     // UTF-8: "🟨"
            case 0xA5: color = TFT_RED;      shape = 's'; break;     // UTF-8: "🟥"
            case 0xA6: color = TFT_BLUE;     shape = 's'; break;     // UTF-8: "🟦"
            case 0xA7: color = TFT_ORANGE;   shape = 's'; break;     // UTF-8: "🟧"
            case 0xAA: color = TFT_VIOLET;   shape = 's'; break;     // UTF-8: "🟪"
            case 0xAB: color = TFT_BROWN;    shape = 's'; break;     // UTF-8: "🟫
        }
        if(shape == 'c') {
            uint16_t fh = m_current_font.font_height; // font height
            uint16_t fw = fh - fh / 3; // y : x =  3/3 : 2/3
            uint16_t p = fh / 5; // padding
            uint16_t r = (fh - 2 * p) / 2; // radius
            uint16_t corr = fw / 10;
            uint16_t cx = x + fw / 2;
            uint16_t cy = y +  fh / 2 + corr;
            fillCircle(cx, cy, r, color);
            return fw;
        }
        else if(shape == 's') {
            uint16_t fh = m_current_font.font_height; // font height
            uint16_t fw = fh - fh / 3; // font height - 1/3
            uint16_t p = fh / 5; // padding
            uint16_t a = (fh - 2 * p); // side length
            uint16_t corr = fw / 10;
            uint16_t sx = x + fw / 2;
            uint16_t sy = y + fh / 2 + corr;
            fillRect(sx - a / 2, sy - a / 2, a, a, color);
            return fw;
        }
        else {
            log_w("unknown shape %c", shape);
            return (uint16_t)0;
        }
        return m_current_font.font_height;
    };
    //-------------------------------------------------------------------------------------------------------------------
    auto drawChar = [&](uint16_t idx, uint16_t x, uint16_t y) { // lambda
        uint32_t glyphPos = m_current_font.lookup_table[utfPosArr[idx]];
        uint32_t adv_w = m_current_font.glyph_dsc[glyphPos].adv_w / 16;
        uint32_t bitmap_index = m_current_font.glyph_dsc[glyphPos].bitmap_index;
        uint32_t box_w = m_current_font.glyph_dsc[glyphPos].box_w;
        uint32_t box_h = m_current_font.glyph_dsc[glyphPos].box_h;
        int16_t  ofs_x = m_current_font.glyph_dsc[glyphPos].ofs_x;
        int16_t  ofs_y = m_current_font.glyph_dsc[glyphPos].ofs_y;
        if(ofs_x < 0) ofs_x = 0;
        x += ofs_x;
        y = y + (m_current_font.line_height - m_current_font.base_line - 1) - box_h - ofs_y;
        if(y > 479) log_e("%s%i y %i idx %i", __FILE__, __LINE__, y, idx);
        writeInAddrWindow(m_current_font.glyph_bitmap + bitmap_index, x, y, box_w, box_h);
        if(!narrow) adv_w += ofs_x;
        return adv_w;
    };
    //-------------------------------------------------------------------------------------------------------------------
    strChLength =  analyzeText(str, utfPosArr, colorArr, _textColor); // fill utfPosArr, colorArr, ansiArr
    // returns the number of chars
    if(autoSize) {nrOfLines = fitInAddrWindow(utfPosArr, strChLength, win_W, win_H, narrow, noWrap);}  // choose perfect fontsize
    if(!strChLength) return 0;
    //----------------------------------------------------------------------
    if((win_X + win_W) > m_h_res) { win_W = m_h_res - win_X; }   // Limit, right edge of the display
    if((win_Y + win_H) > m_v_res) { win_H = m_v_res - win_Y; }   // Limit, bottom of the display
    if(win_W < 0) { win_X = 0; }                                 // Limit, left edge of the display
    if(win_H < 0) { win_Y = 0; }                                 // Limit, top of the display
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
        if(offset < 0) log_e("%s %i offset %i", __FILE__, __LINE__, offset);
        pY = pY + offset;
    }
    if(v_align == TFT_ALIGN_DOWN){
        int offset = (win_H - (nrOfLines * m_current_font.line_height));
        if(offset < 0) log_e("%s %i offset %i", __FILE__, __LINE__, offset);
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

        if(h_align == TFT_ALIGN_RIGHT)  { pX += win_W - (usedPxLength) - 2; }
        if(h_align == TFT_ALIGN_CENTER) { pX += (win_W - usedPxLength) / 2; }
        uint16_t cnt = 0;
        while(true) {               // inner while
            isEmoji = false;
            setTextColor(colorArr[idx]);
            // if(cnt == 0 && utfPosArr[idx] == 0x20) {
            //     idx++;
            //     charsDrawn++;
            //     continue;
            // } // skip leading spaces
            if((utfPosArr[idx] & 0xFF00) == 0xF900){ // This is a emoji, width is the same as height
                if(utfPosArr[idx] == 0xF9A2) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A1) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9B4) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9B5) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A0) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A3) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A4) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A9) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A8) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A5) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A6) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9A7) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9AA) {isEmoji = true;}
                if(utfPosArr[idx] == 0xF9AB) {isEmoji = true;}
            }
            else{
                ;
            }
            uint16_t res = 0;
            if(isEmoji) {
                res = drawEmoji(idx, pX, pY);
            }
            else {
                res = drawChar(idx, pX, pY);
            }
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

bool TFT_SPI::drawBmpFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, float scale) {
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

    startWrite();
    setAddrWindow(x, y, displayWidth, displayHeight);
    for(int16_t j = y; j < y + displayHeight; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, displayWidth);
    }
    endWrite();

    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  G I F  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat) {

    gif.Iterations = repeat;

    GIF_DecoderReset();

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
    // { log_w("Width: %i, Height: %i,", gif.LogicalScreenWidth, gif.LogicalScreenHeight); }
    if(gif.LogicalScreenWidth * gif.LogicalScreenHeight > 155000) {
        if(tft_info) tft_info("!Image is too big!!");
        return false;
    }

    if(psramFound()){gif_ImageBuffer = (uint16_t*) ps_malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));}
    else            {gif_ImageBuffer = (uint16_t*)    malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));}

    if(psramFound()){gif_RestoreBuffer = (uint16_t*) ps_malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));}
    else            {gif_RestoreBuffer = (uint16_t*)    malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));}

    if(!gif_ImageBuffer)   {if(tft_info) tft_info("!Not enough memory!!"); return false;}
    if(!gif_RestoreBuffer) {if(tft_info) tft_info("!Not enough memory!!"); return false;}

    if(GIF_decodeGif(x, y) == false) {
        GIF_freeMemory();
        gif_file.close();
        gif.drawNextImage = false;
        log_w("GIF file closed");
        return true;
    }
    else {
        gif.drawNextImage = true;
    }
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::GIF_loop(){
    if(!gif.drawNextImage) return false;
    if(gif.TimeStamp > millis()) return false;
    if(!GIF_decodeGif(100, 100)){
        GIF_freeMemory();
        gif_file.close();
        gif.drawNextImage = false;
        log_w("GIF file closed");
        return false;
    }
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::GIF_readHeader() {

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
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_readLogicalScreenDescriptor() {

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
    // 4  |a| bbb |c| ddd |       <Packed Fields>               See below
    //    +---------------+
    // 5  |               |       Background Color Index        Byte
    //    +---------------+
    // 6  |               |       Pixel Aspect Ratio            Byte
    //    +---------------+
    //
    /* The first bit of the packed field is the Global Color Table Flag (a). If it is 1, a global color table will follow
           The next three bits are the Color Resolution (bbb), which is only meaningful if there is a global color table.
           The value of this field, N, determines the number of entries in the global color table as 2^(N+1).
           For example, a value of 001 represents 2 bits per pixel, while 111 would represent 8 bits per pixel.

       The next bit is the Sort Flag (c), which indicates whether the color table is sorted or not.
       The last three bits are the Size of Global Color Table (ddd), which indicates the size of the global color table in powers of 2.
    */
    /* The Background Color Index in a GIF specifies the color used for pixels on the screen that are not covered by an image.
       This index is into the Global Color Table and is used as the background color if the Global Color Table Flag is set.
       However, if the Global Color Table Flag is set to zero, this field is ignored, leading to ambiguity about the actual background color if the
       image data does not cover the entire image area.
       This ambiguity means that different GIF implementations might behave differently in such cases. If you are working with GIFs and need to ensure
       consistency in background color handling, it's important to be aware of these implementation differences.
    */
    /* The Graphics Interchange Format (GIF) uses a pixel aspect ratio where each pixel can have up to 8 bits per pixel, allowing for a palette of up
       to 256 different colors chosen from the 24-bit RGB color space.
    */

    gif_file.readBytes(gif_buffer, 7); // Logical Screen Descriptor
    gif.LogicalScreenWidth = gif_buffer[0] + 256 * gif_buffer[1];
    gif.LogicalScreenHeight = gif_buffer[2] + 256 * gif_buffer[3];
    gif.PackedFields = gif_buffer[4];
    gif.GlobalColorTableFlag = (gif.PackedFields & 0x80);
    gif.ColorResulution = ((gif.PackedFields & 0x70) >> 3) + 1;
    gif.SortFlag = (gif.PackedFields & 0x08);
    gif.SizeOfGlobalColorTable = (gif.PackedFields & 0x07);
    gif.SizeOfGlobalColorTable = (1 << (gif.SizeOfGlobalColorTable + 1));
    gif.BackgroundColorIndex = gif_buffer[5];
    gif.PixelAspectRatio = gif_buffer[6];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_readImageDescriptor() {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
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
    gif.ImageLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif.ImageTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif.ImageWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif.ImageHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif.PackedFields = gif_buffer[8];
    gif.LocalColorTableFlag = ((gif.PackedFields & 0x80) >> 7);
    gif.InterlaceFlag = ((gif.PackedFields & 0x40) >> 6);
    gif.SortFlag = ((gif.PackedFields & 0x20)) >> 5;
    gif.SizeOfLocalColorTable = (gif.PackedFields & 0x07);
    gif.SizeOfLocalColorTable = (1 << (gif.SizeOfLocalColorTable + 1));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_readLocalColorTable() {
    // The size of the local color table can be calculated by the value given in the image descriptor.
    // Just like with the global color table, if the image descriptor specifies a size of N,
    // the color table will contain 2^(N+1) colors and will take up 3*2^(N+1) bytes.
    // The colors are specified in RGB value triplets.
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    gif_LocalColorTable.reserve(gif.SizeOfLocalColorTable);
    if(gif.LocalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif.SizeOfLocalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill LocalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_LocalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        // for(i=0;i<SizeOfLocalColorTable; i++) log_i("LocalColorTable %i= %i", i, LocalColorTable[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_readGlobalColorTable() {
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
    if(gif.GlobalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif.SizeOfGlobalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill GlobalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_GlobalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        //    for(i=0;i<gif.SizeOfGlobalColorTable;i++) log_i("GlobalColorTable %i= %i", i, gif_GlobalColorTable[i]);
        //    log_i("Read GlobalColorTable Size=%i", gif.SizeOfGlobalColorTable);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_readGraphicControlExtension() {

/*     7 6 5 4 3 2 1 0
   0 | 0x21            | Extension Introducer  - Identifies the beginning of an extension block. This field contains the fixed value 0x21.
   1 | 0xF9            | Graphic Control Label - Identifies the type of extension block. For the Graphic Control Extension, this field contains the fixed value 0xF9.
   2 | 0x04            | Block Size - The size of the block, not including the Block Terminator. This field contains the fixed value 0x04.
   3 | x x x d d d u t | Packed Fields: xxx - reserved, ddd - disposal method, u . user input flag, t - transparent color flag.
   4 |                 | Delay Time LSB - The delay time in hundredths of a second before the next image is displayed. This field contains the delay time.
   5 |                 | Delay Time MSB - The delay time in hundredths of a second before the next image is displayed. This field contains the delay time.
   6 |                 | Transparent Color Index - The index of the transparent color in the color table. This field contains the index of the transparent color.
   7 | 0x00            | Block Terminator - Marks the end of the Graphic Control Extension. This field contains the fixed value 0x00.

   Disposal Method - Indicates the way in which the graphic is to be treated after being displayed
                       0 -   No disposal specified. The decoder is not required to take any action.
                       1 -   Do not dispose. The graphic is to be left in place.
                       2 -   Restore to background color. The area used by the graphic must be restored to the background color.
                       3 -   Restore to previous. The decoder is required to restore the area overwritten by the graphic with
   User Input Flag - Indicates whether or not user input is expected before continuing. If the flag is set, processing will continue when user input is entered. The nature of the User input
                     is determined by the application (Carriage Return, Mouse Button Click, etc.).
                       0 -   User input is not expected.
                       1 -   User input is expected.
   Transparency Flag - Indicates whether a transparency index is given in the Transparent Index field. (This field is the least significant bit of the byte.)
                       0 -   Transparent Index is not given.
                       1 -   Transparent Index is given.
*/

    uint8_t BlockSize = 0;
    gif_file.readBytes(gif_buffer, 1);
    BlockSize = gif_buffer[0]; // Number of bytes in the block, not including the Block Terminator

    if(BlockSize == 0) return;
    gif_file.readBytes(gif_buffer, BlockSize);
    gif.PackedFields = gif_buffer[0];
    gif.DisposalMethod = (gif.PackedFields & 0x1C) >> 2;
    gif.UserInputFlag = (gif.PackedFields & 0x02);
    gif.TransparentColorFlag = gif.PackedFields & 0x01;
    gif.DelayTime = gif_buffer[1] + 256 * gif_buffer[2];
    gif.TransparentColorIndex = gif_buffer[3];

    gif_file.readBytes(gif_buffer, 1); // marks the end of the Graphic Control Extension
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_SPI::GIF_readPlainTextExtension(char* buf) {

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

    gif.TextGridLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif.TextGridTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif.TextGridWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif.TextGridHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif.CharacterCellWidth = gif_buffer[8];
    gif.CharacterCellHeight = gif_buffer[9];
    gif.TextForegroundColorIndex = gif_buffer[10];
    gif.TextBackgroundColorIndex = gif_buffer[11];

    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_SPI::GIF_readApplicationExtension(char* buf) {

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
    if(BlockSize > 0) { gif_file.readBytes(gif_buffer, BlockSize); }
    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_SPI::GIF_readCommentExtension(char* buf) {

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

uint8_t TFT_SPI::GIF_readDataSubBlock(char* buf) {

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
        gif.ZeroDataBlock = false;
        gif_file.readBytes(buf, BlockSize);
    }
    else gif.ZeroDataBlock = true;
    return BlockSize;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_SPI::GIF_readExtension(char Label) {
    char buf[256];
    switch(Label) {
        case 0x01:
            // log_w("PlainTextExtension");
            GIF_readPlainTextExtension(buf);
            break;
        case 0xff:
            // log_w("ApplicationExtension");
            GIF_readApplicationExtension(buf);
            break;
        case 0xfe:
            // log_w("CommentExtension");
            GIF_readCommentExtension(buf);
            break;
        case 0xF9:
            // log_w("GraphicControlExtension");
            GIF_readGraphicControlExtension();
            break;
        default: return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_SPI::GIF_GetCode(int32_t code_size, int32_t flag) {
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

int32_t TFT_SPI::GIF_LZWReadByte(bool init) {
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

        gif.CodeSize = gif.LZWMinimumCodeSize + 1;
        gif.ClearCode = (1 << gif.LZWMinimumCodeSize);
        gif.EOIcode = gif.ClearCode + 1;
        gif.MaxCode = gif.ClearCode + 2;
        gif.MaxCodeSize = 2 * gif.ClearCode;

        fresh = false;

        GIF_GetCode(0, true);

        fresh = true;

        for(i = 0; i < gif.ClearCode; i++) {
            gif_next[i] = 0;
            gif_vals[i] = i;
        }
        for(; i < (1 << gif_MaxLzwBits); i++) gif_next[i] = gif_vals[0] = 0;

        sp = &gif_stack[0];

        return 0;
    }
    else if(fresh) {
        fresh = false;
        do { firstcode = oldcode = GIF_GetCode(gif.CodeSize, false); } while(firstcode == gif.ClearCode);

        return firstcode;
    }

    if(sp > &gif_stack[0]) return *--sp;

    while((code = GIF_GetCode(gif.CodeSize, false)) >= 0) {
        if(code == gif.ClearCode) {
            for(i = 0; i < gif.ClearCode; ++i) {
                gif_next[i] = 0;
                gif_vals[i] = i;
            }
            for(; i < (1 << gif_MaxLzwBits); ++i) gif_next[i] = gif_vals[i] = 0;

            gif.CodeSize = gif.LZWMinimumCodeSize + 1;
            gif.MaxCodeSize = 2 * gif.ClearCode;
            gif.MaxCode = gif.ClearCode + 2;
            sp = &gif_stack[0];

            firstcode = oldcode = GIF_GetCode(gif.CodeSize, false);
            return firstcode;
        }
        else if(code == gif.EOIcode) {
            int32_t count;
            char    buf[260];

            if(gif.ZeroDataBlock) return -2;
            while((count = GIF_readDataSubBlock(buf)) > 0);

            if(count != 0) return -2;
        }

        incode = code;

        if(code >= gif.MaxCode) {
            *sp++ = firstcode;
            code = oldcode;
        }

        while(code >= gif.ClearCode) {
            *sp++ = gif_vals[code];
            if(code == (int32_t)gif_next[code]) { return -1; }
            code = gif_next[code];
        }
        *sp++ = firstcode = gif_vals[code];

        if((code = gif.MaxCode) < (1 << gif_MaxLzwBits)) {
            gif_next[code] = oldcode;
            gif_vals[code] = firstcode;
            ++gif.MaxCode;
            if((gif.MaxCode >= gif.MaxCodeSize) && (gif.MaxCodeSize < (1 << gif_MaxLzwBits))) {
                gif.MaxCodeSize *= 2;
                ++gif.CodeSize;
            }
        }
        oldcode = incode;

        if(sp > &gif_stack[0]) return *--sp;
    }
    return code;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TFT_SPI::GIF_ReadImage(uint16_t x, uint16_t y) {
    int32_t  color;
    int32_t  xpos = x + gif.ImageLeftPosition;
    int32_t  ypos = y + gif.ImageTopPosition;
    int32_t  max = gif.ImageHeight * gif.ImageWidth;
    uint32_t i = 0;
    static uint8_t gif_LastDisposalMethod = 0;
    static uint16_t gif_LastImageWidth = 0;
    static uint16_t gif_LastImageHeight = 0;
    static uint16_t gif_LastImageLeftPosition = 0;
    static uint16_t gif_LastImageTopPosition = 0;

    gif.LZWMinimumCodeSize = gif_file.read();
    if (GIF_LZWReadByte(true) < 0) return false;


    if(gif.DisposalMethod < 2){

        while (i < max) {
            color = GIF_LZWReadByte(false);
            uint16_t local_x = i % gif.ImageWidth;
            uint16_t local_y = i / gif.ImageWidth;

            int32_t global_x = gif.ImageLeftPosition + local_x;
            int32_t global_y = gif.ImageTopPosition + local_y;

            if ((color == gif.TransparentColorIndex) && gif.TransparentColorFlag) {
                // Transparent: left existing buf value (no overwriting)
            } else {
                if (gif.LocalColorTableFlag) {
                    gif_ImageBuffer[global_y * gif.LogicalScreenWidth + global_x] = gif_LocalColorTable[color];
                } else {
                    gif_ImageBuffer[global_y * gif.LogicalScreenWidth + global_x] = gif_GlobalColorTable[color];
                }
            }
            i++;
        }
    }

    if (gif.DisposalMethod == 2) {
        for (uint16_t row = 0; row < gif.ImageHeight; row++) {
            for (uint16_t col = 0; col < gif.ImageWidth; col++) {
                uint16_t buf_x = gif.ImageLeftPosition + col;
                uint16_t buf_y = gif.ImageTopPosition + row;

                if (buf_x < gif.LogicalScreenWidth && buf_y < gif.LogicalScreenHeight) {
                    gif_ImageBuffer[buf_y * gif.LogicalScreenWidth + buf_x] = gif_LocalColorTable[gif.BackgroundColorIndex];
                }
            }
        }
    }

    // If the last picture disposal method == 3 hard, reset area
    if (gif_LastDisposalMethod == 3) {
        gif_LastDisposalMethod = gif.DisposalMethod;
        for (uint16_t row = 0; row < gif_LastImageHeight; row++) {
            for (uint16_t col = 0; col < gif_LastImageWidth; col++) {
                uint16_t x_buf = gif_LastImageLeftPosition + col;
                uint16_t y_buf = gif_LastImageTopPosition + row;

                if (x_buf < gif.LogicalScreenWidth && y_buf < gif.LogicalScreenHeight) {
                    gif_ImageBuffer[y_buf * gif.LogicalScreenWidth + x_buf] = gif_RestoreBuffer[y_buf * gif.LogicalScreenWidth + x_buf];
                }
            }
        }
    }

    // If this frame disposal method == 3 has → secure current state
    if (gif.DisposalMethod == 3) {
        gif_LastDisposalMethod = gif.DisposalMethod;
        gif.ImageHeight = gif_LastImageHeight;
        gif.ImageWidth = gif_LastImageWidth;
        gif.ImageLeftPosition = gif_LastImageLeftPosition;
        gif.ImageTopPosition = gif_LastImageTopPosition;
        for (uint16_t row = 0; row < gif.ImageHeight; row++) {
            for (uint16_t col = 0; col < gif.ImageWidth; col++) {
                uint16_t x_buf = gif.ImageLeftPosition + col;
                uint16_t y_buf = gif.ImageTopPosition + row;

                if (x_buf < gif.LogicalScreenWidth && y_buf < gif.LogicalScreenHeight) {
                    gif_RestoreBuffer[y_buf * gif.LogicalScreenWidth + x_buf] = gif_ImageBuffer[y_buf * gif.LogicalScreenWidth + x_buf];
                }
            }
        }
    }

    // Copy from gif_imagebuffer in m framebuffer (only part of the current picture)
    for (uint16_t row = 0; row < gif.ImageHeight; row++) {
        for (uint16_t col = 0; col < gif.ImageWidth; col++) {
            uint16_t fb_x = xpos + col;
            uint16_t fb_y = ypos + row;

            uint16_t buf_x = gif.ImageLeftPosition + col;
            uint16_t buf_y = gif.ImageTopPosition + row;

            if (fb_x < m_h_res && fb_y < m_v_res &&
                buf_x < gif.LogicalScreenWidth && buf_y < gif.LogicalScreenHeight) {
                m_framebuffer[0][fb_y * m_h_res + fb_x] = gif_ImageBuffer[buf_y * gif.LogicalScreenWidth + buf_x];
            }
        }
    }

    //panelDrawBitmap(xpos, ypos, xpos + gif.ImageWidth, ypos + gif.ImageHeight, m_framebuffer[0]);

    startWrite();
    setAddrWindow(xpos, ypos, gif.ImageWidth, gif.ImageHeight);
    for(int16_t j = y; j < ypos + gif.ImageHeight; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + xpos, gif.ImageWidth);
    }
    endWrite();

    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_SPI::GIF_readGifItems() {
    GIF_readHeader();
    GIF_readLogicalScreenDescriptor();
    gif.decodeSdFile_firstread = true;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_SPI::GIF_decodeGif(uint16_t x, uint16_t y) {
    char           c = 0;
    static int32_t test = 1;
    char           Label = 0;
    if(gif.decodeSdFile_firstread == true) GIF_readGlobalColorTable(); // If exists
    gif.decodeSdFile_firstread = false;

    while(c != ';') { // Trailer found
        c = gif_file.read();
        if(c == '!') {               // it is a Extension
            Label = gif_file.read(); // Label
            GIF_readExtension(Label);
        }
        if(c == ',') {
            GIF_readImageDescriptor(); // ImgageDescriptor
            GIF_readLocalColorTable(); // can follow the ImagrDescriptor
            GIF_ReadImage(x, y);       // read Image Data
            test++;
            gif.TimeStamp = millis() + gif.DelayTime;
            return true; // more images can follow
        }
    }
    // for(int32_t i=0; i<bigbuf.size(); i++)  log_i("bigbuf %i=%i", i, bigbuf[i]);
    // if(tft_info) tft_info("GIF: found Trailer");
    return false; // no more images to decode
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_SPI::GIF_freeMemory() {
    gif_next.clear();               gif_next.shrink_to_fit();
    gif_vals.clear();               gif_vals.shrink_to_fit();
    gif_stack.clear();              gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();   gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();    gif_LocalColorTable.shrink_to_fit();
    if(gif_ImageBuffer){free(gif_ImageBuffer); gif_ImageBuffer = NULL;}
    if(gif_RestoreBuffer){free(gif_RestoreBuffer); gif_RestoreBuffer = NULL;}
}

void TFT_SPI::GIF_DecoderReset(){
   GIF_freeMemory();
   gif_file.close();
   gif.decodeSdFile_firstread = false;
   gif.GlobalColorTableFlag = false;
   gif.LocalColorTableFlag = false;
   gif.SortFlag = false;
   gif.TransparentColorFlag = false;
   gif.UserInputFlag = false;
   gif.ZeroDataBlock = 0;
   gif.InterlaceFlag = false;
   gif.drawNextImage = false;
   gif.BackgroundColorIndex = 0;
   gif.BlockTerninator = 0;
   gif.CharacterCellWidth = 0;
   gif.CharacterCellHeight = 0;
   gif.CodeSize = 0;
   gif.ColorResulution = 0;
   gif.DisposalMethod = 0;
   gif.ImageSeparator = 0;
   gif.lenDatablock = 0;
   gif.LZWMinimumCodeSize = 0;
   gif.PackedFields = 0;
   gif.PixelAspectRatio = 0;
   gif.TextBackgroundColorIndex = 0;
   gif.TextForegroundColorIndex = 0;
   gif.TransparentColorIndex = 0;
   gif.ClearCode = 0;
   gif.DelayTime = 0;
   gif.EOIcode = 0; // End Of Information
   gif.ImageHeight = 0;
   gif.ImageWidth = 0;
   gif.ImageLeftPosition = 0;
   gif.ImageTopPosition = 0;
   gif.LogicalScreenWidth = 0;
   gif.LogicalScreenHeight = 0;
   gif.MaxCode = 0;
   gif.MaxCodeSize = 0;
   gif.SizeOfGlobalColorTable = 0;
   gif.SizeOfLocalColorTable = 0;
   gif.TextGridLeftPosition = 0;
   gif.TextGridTopPosition = 0;
   gif.TextGridWidth = 0;
   gif.TextGridHeight = 0;
   gif.TimeStamp = 0;
   gif.Iterations = 0;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ J P E G ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::drawJpgFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
    if(!fs.exists(path)) {log_e("file %s not exists", path); return false; }
    if(maxWidth) m_jpgWidthMax = maxWidth; else m_jpgWidthMax = m_h_res;
    if(maxHeight) m_jpgHeightMax = maxHeight; else m_jpgHeightMax = m_v_res;

    m_jpgFile = fs.open(path, FILE_READ);
    if(!m_jpgFile) {log_e("Failed to open file for reading"); JPEG_setJpgScale(1); return false;}
    JPEG_getJpgSize(&m_jpgWidth, &m_jpgHeight);
    int res = JPEG_drawJpg(x, y); (void) res;
    // log_w("path %s, res %i, x %i, y %i, m_jpgWidth %i, m_jpgHeight %i", path, res, x, y, m_jpgWidth, m_jpgHeight);
    m_jpgFile.close();

    startWrite();
    setAddrWindow(x, y, m_jpgWidth, m_jpgHeight);
    for(int16_t j = y; j < y + m_jpgHeight; j++) {
        writePixels(m_framebuffer[0] + j * m_h_res + x, m_jpgWidth);
    }
    endWrite();

    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::JPEG_setJpgScale(uint8_t scaleFactor) {
    switch (scaleFactor) {
        case 1:  m_jpgScale = 0; break;
        case 2:  m_jpgScale = 1; break;
        case 4:  m_jpgScale = 2; break;
        case 8:  m_jpgScale = 3; break;
        default: m_jpgScale = 0;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::JPEG_setSwapBytes(bool swapBytes){
  m_swap = swapBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
unsigned int TFT_SPI::JPEG_jd_input(JDEC* jdec, uint8_t* buf, unsigned int len){
    uint32_t bytesLeft = 0;

    if (m_jpg_source == TJPG_ARRAY) {  // Handle an array input
        if (m_array_index + len > m_array_size) { len = m_array_size - m_array_index; } // Avoid running off end of array
        if (buf) memcpy_P(buf, (const uint8_t*)(m_array_data + m_array_index), len); // If buf is valid then copy len bytes to buffer
        m_array_index += len;  // Move pointer
    }
    else if (m_jpg_source == TJPG_SD_FILE) {  // Handle SD library input
        bytesLeft = m_jpgFile.available();  // Check how many bytes are available
        if (bytesLeft < len) len = bytesLeft;
        if (buf) {
            m_jpgFile.read(buf, len); // Read into buffer, pointer moved as well
        } else {
            m_jpgFile.seek(m_jpgFile.position() + len); // Buffer is null, so skip data by moving pointer
        }
    }
    return len;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Pass image block back to the sketch for rendering, may be a complete or partial MCU
int TFT_SPI::JPEG_jd_output(JDEC* jdec, void* bitmap, JRECT* jrect) {
    jdec = jdec; // Supress warning as ID is not used

    int16_t  x = jrect->left + m_jpeg_x;  // Retrieve rendering parameters and add any offset
    int16_t  y = jrect->top + m_jpeg_y;
    uint16_t w = jrect->right + 1 - jrect->left;
    uint16_t h = jrect->bottom + 1 - jrect->top;
    if(w > m_jpgWidthMax) return true;  // Clip width and height to the maximum allowed dimensions
    if(h > m_jpgHeightMax) return true;
    bool r = JPEG_tft_output(x, y, w, h, (uint16_t*)bitmap);
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::JPEG_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
      if (!bitmap || w <= 0 || h <= 0) {  // Check for valid parameters
        log_e("Invalid parameters: bitmap is null or width/height is zero.");
        return false;
    }
    // Clip the rectangle to ensure it doesn't exceed framebuffer boundaries
    int16_t x_end = std::min((int16_t)(x + w), (int16_t)m_h_res); // End of rectangle in x-direction
    int16_t y_end = std::min((int16_t)(y + h), (int16_t)(m_v_res)); // End of rectangle in y-direction

    if (x >= m_h_res || y >= m_v_res || x_end <= 0 || y_end <= 0) {
        log_e("Rectangle is completely outside the framebuffer boundaries, x: %d, y: %d, x_end: %d, y_end: %d", x, y, x_end, y_end);
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
uint8_t TFT_SPI::JPEG_drawJpg(int32_t x, int32_t y) {
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
uint8_t TFT_SPI::JPEG_getJpgSize(uint16_t* w, uint16_t* h) {
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
    m_jpgFile.seek(0);
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
const uint8_t Clip8[1024] = {	/* 0..255 */
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
uint8_t TFT_SPI::JPEG_BYTECLIP(int val) {
    if(val < 0) return 0;
    else if(val > 255) return 255;
    return (uint8_t)val;
}
#endif
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void* TFT_SPI::JPEG_alloc_pool(JDEC  *jd,size_t ndata) {
	char *rp = 0;

	ndata = (ndata + 3) & ~3; /* Align block size to the word boundary */

	if(jd->sz_pool >= ndata) {
		jd->sz_pool -= ndata;
		rp = (char *)jd->pool;           /* Get start of available memory pool */
		jd->pool = (void *)(rp + ndata); /* Allocate requierd bytes */
	}

	return (void *)rp; /* Return allocated memory block (NULL:no memory to allocate) */
}//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::JPEG_create_qt_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
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
uint8_t TFT_SPI::JPEG_create_huffman_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
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
int TFT_SPI::JPEG_huffext(JDEC* jd, unsigned int id, unsigned int cls) {
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
int TFT_SPI::JPEG_bitext(JDEC* jd, unsigned int nbit) {
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
uint8_t TFT_SPI::JPEG_restart(JDEC* jd, uint16_t rstn) {
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
void TFT_SPI::JPEG_block_idct(int32_t* src, jd_yuv_t* dst) {
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
uint8_t TFT_SPI::JPEG_mcu_load(JDEC* jd) {
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
uint8_t TFT_SPI::JPEG_mcu_output(JDEC* jd, unsigned int x, unsigned int y) {
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
uint8_t TFT_SPI::JPEG_jd_prepare(JDEC* jd, uint8_t* pool, size_t sz_pool, void* dev) {
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
                /* Skip segment data (null pointer specifies to remove data from the stream) */
                if (JPEG_jd_input(jd, 0, len) != len) return JDR_INP;
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::JPEG_jd_decomp(JDEC* jd, uint8_t scale) {
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

bool TFT_SPI::drawPngFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {

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
    // log_w("png_decode err=%i",err);
    return err == PNG_EOK;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
char TFT_SPI::read_bit(uint32_t *bitpointer, const char* bitstream) {
    char result = (char)((bitstream[(*bitpointer) >> 3] >> ((*bitpointer) & 0x7)) & 1);
    (*bitpointer)++;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::read_bits(uint32_t* bitpointer, const char* bitstream, uint32_t nbits) {
    unsigned result = 0, i;
    for(i = 0; i < nbits; i++) result |= ((uint16_t)read_bit(bitpointer, bitstream)) << i;
    return result;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/* the buffer must be numcodes*2 in size! */
void TFT_SPI::huffman_tree_init(huffman_tree* tree, uint16_t* buffer, uint16_t numcodes, uint16_t maxbitlen) {
    tree->tree2d = buffer;
    tree->numcodes = numcodes;
    tree->maxbitlen = maxbitlen;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*given the code lengths (as stored in the PNG file), generate the tree as defined by Deflate. maxbitlen is the maximum
 * bits that a code in the tree can have. return value is error.*/
void TFT_SPI::huffman_tree_create_lengths(huffman_tree* tree, const uint16_t* bitlen) {
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
uint16_t TFT_SPI::huffman_decode_symbol(const char* in, uint32_t  * bp, const huffman_tree* codetree, uint32_t inlength) {
    int16_t      treepos = 0, ct;
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
void TFT_SPI::get_tree_inflate_dynamic(huffman_tree* codetree, huffman_tree* codetreeD, huffman_tree* codelengthcodetree, const char* in, uint32_t *bp, uint32_t inlength) {
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
void TFT_SPI::inflate_huffman(char* out, uint32_t   outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength, uint16_t btype) {
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
void TFT_SPI::inflate_uncompressed(char* out, uint32_t outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength) {
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
int8_t TFT_SPI::uz_inflate_data(char* out, uint32_t outsize, const char* in, uint32_t insize, uint32_t inpos) {
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
int8_t TFT_SPI::uz_inflate(char* out, uint32_t outsize, const char* in, uint32_t insize) {
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
int TFT_SPI::paeth_predictor(int a, int b, int c) {
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
void TFT_SPI::unfilter_scanline(char* recon, const char* scanline, const char* precon, uint32_t bytewidth, unsigned char filterType, uint32_t length) {
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
        log_e("recon: %s, scanline: %s, precon: %s, bytewidth: %lu, length: %lu, filterType: %d", recon, scanline, precon, bytewidth, length, filterType);
            png_error = PNG_EMALFORMED;
            break;
    }
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::unfilter(char* out, const char* in, unsigned w, unsigned h, unsigned bpp) {
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
void TFT_SPI::remove_padding_bits(char* out, const char* in, uint32_t olinebits, uint32_t ilinebits, unsigned h) {
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
void TFT_SPI::post_process_scanlines(char* out, char* in) {
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
int8_t TFT_SPI::png_decode() {
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
    // log_w("inflated_size=%i", inflated_size);

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

    png_draw_into_AddrWindow(png_pos_x, png_pos_y, png_width, png_height, png_outbuffer, png_outbuff_size, png_format);

    if(png_outbuffer) {
        free(png_outbuffer);
        png_outbuffer = NULL;
    }
    return png_error;
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_SPI::png_format_t TFT_SPI::png_determine_format() {
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
bool TFT_SPI::png_read_header() {
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

    // log_w("png_width=%i, png_height=%i, png_color_depth=%i, png_color_type=%i", png_width, png_height, png_color_depth, png_color_type);

    /* determine our color format */
    png_format = png_determine_format();
    png_error = png_format == PNG_BADFORMAT ? PNG_EUNFORMAT : PNG_EOK;
    // log_w("png_format=%i", png_format);
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
int8_t TFT_SPI::png_get_error() { return png_error; }
uint16_t TFT_SPI::png_get_width() { return png_width; }
uint16_t TFT_SPI::png_get_height() { return png_height; }
uint16_t TFT_SPI::png_get_bpp() { return png_get_bitdepth() * png_get_components(); }
const char* TFT_SPI::png_get_outbuffer(){return png_outbuffer;}
uint32_t TFT_SPI::png_get_size(){return png_outbuff_size;}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::png_get_components() {
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
uint16_t TFT_SPI::png_get_bitdepth() { return png_color_depth; }
//_______________________________________________________________________________________________________________________________
uint16_t TFT_SPI::png_get_pixelsize() {
    uint16_t bits = png_get_bitdepth() * png_get_components();
    bits += bits % 8;
    return bits;
}

TFT_SPI::png_format_t TFT_SPI::png_get_format() { return png_format; }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::png_GetPixel(void* pixel, int x, int y) {
    uint32_t bpp = png_get_bpp();
    //    Serial.printf("\nbbp=%i\n",(int)bpp);
    uint32_t   Bpp = ((bpp + 7) / 8);
    uint32_t   position = (png_width * y + x) * Bpp;
    //    Serial.printf("\nposition in file=%li\n",(long)position);
    memcpy(pixel, png_buffer + position, Bpp);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Initializing color variables */

TFT_SPI::png_s_rgb16b* TFT_SPI::InitColorR5G6B5() {
png_s_rgb16b* color = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    if(color != 0) { ResetColor(color); }
    return color;
}
TFT_SPI::png_s_rgb18b* TFT_SPI::InitColorR6G6B6() {
    png_s_rgb18b* color = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    if(color != 0) { ResetColor(color); }
    return color;
}
TFT_SPI::png_s_rgb24b* TFT_SPI::InitColorR8G8B8() {
    png_s_rgb24b* color = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    if(color != 0) { ResetColor(color); }
    return color;
}

void TFT_SPI::InitColor(png_s_rgb16b** dst) {
    *dst = (png_s_rgb16b*)malloc(sizeof(png_s_rgb16b));
    ResetColor(*dst);
}
void TFT_SPI::InitColor(png_s_rgb18b** dst) {
    *dst = (png_s_rgb18b*)malloc(sizeof(png_s_rgb18b));
    ResetColor(*dst);
}
void TFT_SPI::InitColor(png_s_rgb24b** dst) {
    *dst = (png_s_rgb24b*)malloc(sizeof(png_s_rgb24b));
    ResetColor(*dst);
}

void TFT_SPI::ResetColor(png_s_rgb16b* dst) { *dst = (png_s_rgb16b){0, 0, 0, 0}; }
void TFT_SPI::ResetColor(png_s_rgb18b* dst) { *dst = (png_s_rgb18b){0, 0, 0, 0}; }
void TFT_SPI::ResetColor(png_s_rgb24b* dst) { *dst = (png_s_rgb24b){0, 0, 0, 0}; }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*Converting between colors*/

void TFT_SPI::png_rgb24bto18b(png_s_rgb18b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 2;  // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 2;  // 3;//2;
}

void TFT_SPI::png_rgb24bto16b(png_s_rgb16b* dst, png_s_rgb24b* src) {
    dst->r = src->r >> 3;  // 3;//2;
    dst->g = src->g >> 2;
    dst->b = src->b >> 3;  // 3;//2;
}
void TFT_SPI::png_rgb18btouint32(uint32_t* dst, png_s_rgb18b* src) { memcpy(dst, src, sizeof(png_s_rgb18b)); }
void TFT_SPI::png_rgb16btouint32(uint32_t* dst, png_s_rgb16b* src) { memcpy(dst, src, sizeof(png_s_rgb16b)); }
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::png_draw_into_AddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char* rgbaBuffer, uint32_t png_outbuff_size, uint8_t png_format) {
    if (!rgbaBuffer || png_outbuff_size < w * h * 4) return; // not enough data

    for (uint16_t row = 0; row < h; row++) {
        for (uint16_t col = 0; col < w; col++) {
            uint32_t index = (row * w + col) * 4;
            uint8_t r = rgbaBuffer[index];     // Rot
            uint8_t g = rgbaBuffer[index + 1]; // Grün
            uint8_t b = rgbaBuffer[index + 2]; // Blau
            uint8_t a = rgbaBuffer[index + 3]; // Alpha

            uint16_t px = x + col;
            uint16_t py = y + row;

            if (px >= m_h_res || py >= m_v_res) continue; // outside the screen

            // only alpha blending if alpha is not full
            if (a < 255) {
                // get the old pixel
                uint16_t oldPixel = m_framebuffer[0][py * m_h_res + px];

                // Extrahiere die RGB-Komponenten aus dem alten Pixel (RGB565 → 888)
                uint8_t oldR = ((oldPixel >> 11) & 0x1F) << 3; // 5 Bit Rot → 8 Bit
                uint8_t oldG = ((oldPixel >> 5) & 0x3F) << 2;  // 6 Bit Grün → 8 Bit
                uint8_t oldB = (oldPixel & 0x1F) << 3;         // 5 Bit Blau → 8 Bit

                // calculate the new pixel
                uint8_t newR = (r * a + oldR * (255 - a)) / 255;
                uint8_t newG = (g * a + oldG * (255 - a)) / 255;
                uint8_t newB = (b * a + oldB * (255 - a)) / 255;

                // set the new pixel in the framebuffer
                r = newR >> 3;
                g = newG >> 2;
                b = newB >> 3;
            } else {
                // full alpha, no blending
                r = r >> 3;
                g = g >> 2;
                b = b >> 3;
            }

            // set the new pixel in the framebuffer
            m_framebuffer[0][py * m_h_res + px] = (r << 11) | (g << 5) | b;
        }
    }

    // update the display
    startWrite();
    setAddrWindow(x, y, w, h);
    for(uint16_t row = y; row < y + h; row++) {
        writePixels(m_framebuffer[0] + row * m_h_res + x, w);
    }
    endWrite();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ CONTROLLER SPECIFIC  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::init() {
    startWrite();
    if(_TFTcontroller == ILI9341) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9341");
        writeCommand(0xCB); // POWERA
        spi_TFT->write(0x39); spi_TFT->write(0x2C); spi_TFT->write(0x00); spi_TFT->write(0x34);
        spi_TFT->write(0x02);
        writeCommand(0xCF); // POWERB
        spi_TFT->write(0x00); spi_TFT->write(0xC1); spi_TFT->write(0x30);
        writeCommand(0xE8); // DTCA
        spi_TFT->write(0x85); spi_TFT->write(0x00); spi_TFT->write(0x78);
        writeCommand(0xEA); // DTCB
        spi_TFT->write(0x00); spi_TFT->write(0x00);
        writeCommand(0xED); // POWER_SEQ
        spi_TFT->write(0x64); spi_TFT->write(0x03); spi_TFT->write(0X12); spi_TFT->write(0X81);
        writeCommand(0xF7); // PRC
        spi_TFT->write(0x20);
        writeCommand(0xC0);   // Power control
        spi_TFT->write(0x23); // VRH[5:0]
        writeCommand(0xC1);   // Power control
        spi_TFT->write(0x10); // SAP[2:0];BT[3:0]
        writeCommand(0xC5); // VCM control
        spi_TFT->write(0x3e); spi_TFT->write(0x28);
        writeCommand(0xC7); // VCM control2
        spi_TFT->write(0x86);
        writeCommand(0x36);   // Memory Access Control
        spi_TFT->write(0x48); // 88
        writeCommand(0x3A); // PIXEL_FORMAT
        spi_TFT->write(0x55);
        writeCommand(0xB1); // FRC
        spi_TFT->write(0x00); spi_TFT->write(0x18);
        writeCommand(0xB6); // Display Function Control
        spi_TFT->write(0x08); spi_TFT->write(0x82); spi_TFT->write(0x27);
        writeCommand(0xF2); // 3Gamma Function Disable
        spi_TFT->write(0x00);
        writeCommand(0x2A); // COLUMN_ADDR
        spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0xEF);
        writeCommand(0x2A); // PAGE_ADDR
        spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0x01); spi_TFT->write(0x3F);
        writeCommand(0x26); // Gamma curve selected
        spi_TFT->write(0x01);
        writeCommand(0xE0); // Set Gamma
        spi_TFT->write(0x0F); spi_TFT->write(0x31); spi_TFT->write(0x2B); spi_TFT->write(0x0C); spi_TFT->write(0x0E); spi_TFT->write(0x08);
        spi_TFT->write(0x4E); spi_TFT->write(0xF1); spi_TFT->write(0x37); spi_TFT->write(0x07); spi_TFT->write(0x10); spi_TFT->write(0x03);
        spi_TFT->write(0x0E); spi_TFT->write(0x09); spi_TFT->write(0x00);
        writeCommand(0xE1); // Set Gamma
        spi_TFT->write(0x00); spi_TFT->write(0x0E); spi_TFT->write(0x14); spi_TFT->write(0x03); spi_TFT->write(0x11); spi_TFT->write(0x07);
        spi_TFT->write(0x31); spi_TFT->write(0xC1); spi_TFT->write(0x48); spi_TFT->write(0x08); spi_TFT->write(0x0F); spi_TFT->write(0x0C);
        spi_TFT->write(0x31); spi_TFT->write(0x36); spi_TFT->write(0x0F);
        if(_displayInversion == 0) {
            writeCommand(ILI9341_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ILI9341_INVON); // Display Inversion ON
        }
        writeCommand(0x11); // Sleep out
        delay(120);
        writeCommand(0x2c);
        writeCommand(0x29); // Display on
        writeCommand(0x2c);
    }
    if(_TFTcontroller == HX8347D) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "HX8347D");
        // Driving ability Setting
        writeCommand(0xEA); spi_TFT->write(0x00); // PTBA[15:8]
        writeCommand(0xEB); spi_TFT->write(0x20); // PTBA[7:0]
        writeCommand(0xEC); spi_TFT->write(0x0C); // STBA[15:8]
        writeCommand(0xED); spi_TFT->write(0xC4); // STBA[7:0]
        writeCommand(0xE8); spi_TFT->write(0x40); // OPON[7:0]
        writeCommand(0xE9); spi_TFT->write(0x38); // OPON1[7:0]
        writeCommand(0xF1); spi_TFT->write(0x01); // OTPS1B
        writeCommand(0xF2); spi_TFT->write(0x10); // GEN
        writeCommand(0x27); spi_TFT->write(0xA3); // Display control 2 register

        // Gamma 2.2 Setting
        writeCommand(0x40); spi_TFT->write(0x01); // Gamma control 1 register
        writeCommand(0x41); spi_TFT->write(0x00); // Gamma control 2 register
        writeCommand(0x42); spi_TFT->write(0x00); // Gamma control 3 register
        writeCommand(0x43); spi_TFT->write(0x10); // Gamma control 4 register
        writeCommand(0x44); spi_TFT->write(0x0E); // Gamma control 5 register
        writeCommand(0x45); spi_TFT->write(0x24); // Gamma control 6 register
        writeCommand(0x46); spi_TFT->write(0x04); // Gamma control 7 register
        writeCommand(0x47); spi_TFT->write(0x50); // Gamma control 8 register
        writeCommand(0x48); spi_TFT->write(0x02); // Gamma control 9 register
        writeCommand(0x49); spi_TFT->write(0x13); // Gamma control 10 register
        writeCommand(0x4A); spi_TFT->write(0x19); // Gamma control 11 register
        writeCommand(0x4B); spi_TFT->write(0x19); // Gamma control 12 register
        writeCommand(0x4C); spi_TFT->write(0x16); // Gamma control 13 register
        writeCommand(0x50); spi_TFT->write(0x1B); // Gamma control 14 register
        writeCommand(0x51); spi_TFT->write(0x31); // Gamma control 15 register
        writeCommand(0x52); spi_TFT->write(0x2F); // Gamma control 16 register
        writeCommand(0x53); spi_TFT->write(0x3F); // Gamma control 17 register
        writeCommand(0x54); spi_TFT->write(0x3F); // Gamma control 18 register
        writeCommand(0x55); spi_TFT->write(0x3E); // Gamma control 19 register
        writeCommand(0x56); spi_TFT->write(0x2F); // Gamma control 20 register
        writeCommand(0x57); spi_TFT->write(0x7B); // Gamma control 21 register
        writeCommand(0x58); spi_TFT->write(0x09); // Gamma control 22 register
        writeCommand(0x59); spi_TFT->write(0x06); // Gamma control 23 register
        writeCommand(0x5A); spi_TFT->write(0x06); // Gamma control 24 register
        writeCommand(0x5B); spi_TFT->write(0x0C); // Gamma control 25 register
        writeCommand(0x5C); spi_TFT->write(0x1D); // Gamma control 26 register
        writeCommand(0x5D); spi_TFT->write(0xCC); // Gamma control 27 register

        // Power Voltage Setting
        writeCommand(0x1B); spi_TFT->write(0x1B); // VRH=4.65V
        writeCommand(0x1A); spi_TFT->write(0x01); // BT (VGH~15V,VGL~-10V,DDVDH~5V)
        writeCommand(0x24); spi_TFT->write(0x15); // VMH(VCOM High voltage ~3.2V)
        writeCommand(0x25); spi_TFT->write(0x50); // VML(VCOM Low voltage -1.2V)
        writeCommand(0x23); spi_TFT->write(0x88); // for Flicker adjust //can reload from OTP

        // Power on Setting
        writeCommand(0x18); spi_TFT->write(0x36); // I/P_RADJ,N/P_RADJ, Normal mode 60Hz
        writeCommand(0x19); spi_TFT->write(0x01); // OSC_EN='1', start Osc

        if(_displayInversion == 0) { writeCommand(0x01); spi_TFT->write(0x00);} // DP_STB='0', out deep sleep
        else {                       writeCommand(0x01); spi_TFT->write(0x02);} // DP_STB='0', out deep sleep, invon = 1

        writeCommand(0x1F); spi_TFT->write(0x88); // GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x80); // GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x90); // GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0xD0); // GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
        delay(5);
        // 262k/65k color selection
        writeCommand(0x17); spi_TFT->write(0x05); // default 0x06 262k color // 0x05 65k color
        // SET PANEL
        writeCommand(0x36); spi_TFT->write(0x00); // SS_P, GS_P,REV_P,BGR_P
        // Display ON Setting
        writeCommand(0x28); spi_TFT->write(0x38); // GON=1, DTE=1, D=1000
        delay(40);
        writeCommand(0x28); spi_TFT->write(0x3C); // GON=1, DTE=1, D=1100

        writeCommand(0x16); spi_TFT->write(0x08); // MY=0, MX=0, MV=0, BGR=1
        // Set GRAM Area
        writeCommand(0x02); spi_TFT->write(0x00); // Column address start register upper byte
        writeCommand(0x03); spi_TFT->write(0x00); // Column address start register low byte
        writeCommand(0x04); spi_TFT->write(0x00);
        writeCommand(0x05); spi_TFT->write(0xEF); // Column End
        writeCommand(0x06); spi_TFT->write(0x00);
        writeCommand(0x07); spi_TFT->write(0x00); // Row Start
        writeCommand(0x08); spi_TFT->write(0x01);
        writeCommand(0x09); spi_TFT->write(0x3F); // Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9486");

        // Driving ability Setting
        writeCommand(0x11); // Sleep out, also SW reset
        delay(120);

        writeCommand(0x3A); // Interface Pixel Format
        spi_TFT->write16(0x55);

        writeCommand(0xC2); // Power Control 3 (For Normal Mode)
        spi_TFT->write16(0x44);

        writeCommand(0xC5); // VCOM Control
        spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00);

        if(_TFTcontroller == ILI9486a) {
            writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
            spi_TFT->write16(0x00); spi_TFT->write16(0x04); spi_TFT->write16(0x0E); spi_TFT->write16(0x08); spi_TFT->write16(0x17); spi_TFT->write16(0x0A);
            spi_TFT->write16(0x40); spi_TFT->write16(0x79); spi_TFT->write16(0x4D); spi_TFT->write16(0x07); spi_TFT->write16(0x0E); spi_TFT->write16(0x0A);
            spi_TFT->write16(0x1A); spi_TFT->write16(0x1D); spi_TFT->write16(0x0F);
            writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
            spi_TFT->write16(0x00); spi_TFT->write16(0x1B); spi_TFT->write16(0x1F); spi_TFT->write16(0x02); spi_TFT->write16(0x10); spi_TFT->write16(0x05);
            spi_TFT->write16(0x32); spi_TFT->write16(0x34); spi_TFT->write16(0x43); spi_TFT->write16(0x02); spi_TFT->write16(0x0A); spi_TFT->write16(0x09);
            spi_TFT->write16(0x33); spi_TFT->write16(0x37); spi_TFT->write16(0x0F);
        }

        if(_TFTcontroller == ILI9486b) {
            writeCommand(0xE0); // PGAMCTRL(alternative Positive Gamma Control)
            spi_TFT->write16(0x0F); spi_TFT->write16(0x1F); spi_TFT->write16(0x1C); spi_TFT->write16(0x0C); spi_TFT->write16(0x0F); spi_TFT->write16(0x08);
            spi_TFT->write16(0x48); spi_TFT->write16(0x98); spi_TFT->write16(0x37); spi_TFT->write16(0x0A); spi_TFT->write16(0x13); spi_TFT->write16(0x04);
            spi_TFT->write16(0x11); spi_TFT->write16(0x0D); spi_TFT->write16(0x00);

            writeCommand(0xE1); // NGAMCTRL (alternative Negative Gamma Correction)
            spi_TFT->write16(0x0F); spi_TFT->write16(0x32); spi_TFT->write16(0x2E); spi_TFT->write16(0x0B); spi_TFT->write16(0x0D); spi_TFT->write16(0x05);
            spi_TFT->write16(0x47); spi_TFT->write16(0x75); spi_TFT->write16(0x37); spi_TFT->write16(0x06); spi_TFT->write16(0x10); spi_TFT->write16(0x03);
            spi_TFT->write16(0x24); spi_TFT->write16(0x20); spi_TFT->write16(0x00);
        }
        if(_displayInversion == 0) {
            writeCommand(ILI9486_INVOFF); // Display Inversion OFF, normal mode   RPi LCD (A)
        }
        else {
            writeCommand(ILI9486_INVON); // Display Inversion ON,                RPi LCD (B)
        }

        writeCommand(0x36); // Memory Access Control
        spi_TFT->write16(0x48);

        writeCommand(0x29); // Display ON
        delay(150);
    }
    //==========================================
    if(_TFTcontroller == ILI9488) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9488");
        writeCommand(ILI9488_PGAMCTRL); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0x00); spi_TFT->write(0x03); spi_TFT->write(0x09); spi_TFT->write(0x08); spi_TFT->write(0x16); spi_TFT->write(0x0A);
        spi_TFT->write(0x3F); spi_TFT->write(0x78); spi_TFT->write(0x4C); spi_TFT->write(0x09); spi_TFT->write(0x0A); spi_TFT->write(0x08);
        spi_TFT->write(0x16); spi_TFT->write(0x1A); spi_TFT->write(0x0F);
        writeCommand(ILI9488_NGAMCTRL); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x00); spi_TFT->write(0x16); spi_TFT->write(0x19); spi_TFT->write(0x03); spi_TFT->write(0x0F); spi_TFT->write(0x05);
        spi_TFT->write(0x32); spi_TFT->write(0x45); spi_TFT->write(0x46); spi_TFT->write(0x04); spi_TFT->write(0x0E); spi_TFT->write(0x0D);
        spi_TFT->write(0x35); spi_TFT->write(0x37); spi_TFT->write(0x0F);

        if(_displayInversion == 0) {
            writeCommand(ILI9488_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ILI9488_INVON); // Display Inversion ON
        }

        writeCommand(ILI9488_PWCTR1); // Power Control 1
        spi_TFT->write(0x17); spi_TFT->write(0x15);
        writeCommand(ILI9488_PWCTR2); // Power Control 2
        spi_TFT->write(0x41);
        writeCommand(ILI9488_VMCTR1); // VCOM Control
        spi_TFT->write(0x00); spi_TFT->write(0x12); spi_TFT->write(0x80);
        writeCommand(ILI9488_MADCTL); // Memory Access Control
        spi_TFT->write(0x48);
        writeCommand(ILI9488_COLMOD); // Pixel Interface Format
        spi_TFT->write(0x66);
        writeCommand(ILI9488_IFMODE); // Interface Mode Control
        spi_TFT->write(0x00);
        writeCommand(ILI9488_FRMCTR1); // Frame Rate Control
        spi_TFT->write(0xA0);
        writeCommand(ILI9488_INVTR); // Display Inversion Control
        spi_TFT->write(0x02);
        writeCommand(ILI9488_DISCTRL); // Display Function Control
        spi_TFT->write(0x02); spi_TFT->write(0x02); spi_TFT->write(0x3B);
        writeCommand(ILI9488_ETMOD); // Entry Mode Set
        spi_TFT->write(0xC6);
        writeCommand(0xF7); // Adjust Control 3
        spi_TFT->write(0xA9); spi_TFT->write(0x51); spi_TFT->write(0x2C); spi_TFT->write(0x82);
        writeCommand(ILI9488_SLPOUT); // Exit Sleep
        delay(120);
        writeCommand(ILI9488_DISPON); // Display on
        delay(25);
    }
    //==========================================
    if(_TFTcontroller == ST7796) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796");
        writeCommand(ST7796_SWRESET);
        delay(120);
        writeCommand(ST7796_SLPOUT); // Sleep Out
        delay(120);
        writeCommand(ST7796_MADCTL); // Memory Data Access Control
        spi_TFT->write(0x40);
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0xC3);       // Enable extension command 2 partI
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x96);       // Enable extension command 2 partII
        writeCommand(ST7796_DIC); // Display Inversion Control
        spi_TFT->write(0x00);
        writeCommand(ST7796_IFMODE); // RAM control
        spi_TFT->write(0x00);
        writeCommand(ST7796_BPC); // Blanking Porch Control
        spi_TFT->write(0x08); spi_TFT->write(0x08); spi_TFT->write(0x00); spi_TFT->write(0x64);
        writeCommand(ST7796_PWR1); // Power Control 1
        spi_TFT->write(0xF0); spi_TFT->write(0x17);
        writeCommand(ST7796_PWR2); // Power Control 2
        spi_TFT->write(0x14);      //
        writeCommand(ST7796_PWR3); // Power Control 3
        spi_TFT->write(0xA7);
        writeCommand(ST7796_VCMPCTL); // VCOM Control
        spi_TFT->write(0x20);
        writeCommand(ST7796_DOCA); // Display Output Ctrl Adjust
        spi_TFT->write(0x40); spi_TFT->write(0x8A); spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x29); spi_TFT->write(0x01); spi_TFT->write(0xBF); spi_TFT->write(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(ST7796_PGC); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0xF0); spi_TFT->write(0x0B); spi_TFT->write(0x11); spi_TFT->write(0x0B); spi_TFT->write(0x0A); spi_TFT->write(0x27);
        spi_TFT->write(0x3C); spi_TFT->write(0x55); spi_TFT->write(0x51); spi_TFT->write(0x37); spi_TFT->write(0x15); spi_TFT->write(0x17);
        spi_TFT->write(0x31); spi_TFT->write(0x35);

        writeCommand(ST7796_NGC); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x4E); spi_TFT->write(0x15); spi_TFT->write(0x19); spi_TFT->write(0x0B); spi_TFT->write(0x09); spi_TFT->write(0x27);
        spi_TFT->write(0x34); spi_TFT->write(0x32); spi_TFT->write(0x46); spi_TFT->write(0x38); spi_TFT->write(0x14); spi_TFT->write(0x16);
        spi_TFT->write(0x26); spi_TFT->write(0x2A);

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x3C);       // Enable extension command 2 partI

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x69);       // Enable extension command 2 partII

        if(_displayInversion == 0) {
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ST7796_INVON); // Display Inversion ON
        }

        writeCommand(ST7796_DISPON); // Display on
        delay(25);
    } //===============================================================================

    if(_TFTcontroller == ST7796RPI) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796_RPI");
        writeCommand(ST7796_SWRESET);
        delay(120);

        writeCommand(ST7796_SLPOUT); // Sleep Out
        delay(120);

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0xC3);     // Enable extension command 2 partI

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x96);     // Enable extension command 2 partII

        writeCommand(ST7796_MADCTL); // Memory Data Access Control
        spi_TFT->write16(0x48);

        writeCommand(ST7796_COLMOD); // Memory Data Access Control MX, MY, RGB mode
        spi_TFT->write16(0x55);

        writeCommand(ST7796_DIC); // Display Inversion Control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_IFMODE); // RAM control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_BPC); // Blanking Porch Control
        spi_TFT->write16(0x08); spi_TFT->write16(0x08); spi_TFT->write16(0x00); spi_TFT->write16(0x64);
        writeCommand(ST7796_PWR1); // Power Control 1
        spi_TFT->write16(0xF0); spi_TFT->write16(0x17);
        writeCommand(ST7796_PWR2); // Power Control 2
        spi_TFT->write16(0x14);    //
        writeCommand(ST7796_PWR3); // Power Control 3
        spi_TFT->write16(0xA7);
        writeCommand(ST7796_VCMPCTL); // VCOM Control
        spi_TFT->write16(0x20);
        writeCommand(ST7796_DOCA); // Display Output Ctrl Adjust
        spi_TFT->write16(0x40); spi_TFT->write16(0x8A); spi_TFT->write16(0x00); spi_TFT->write16(0x00);
        spi_TFT->write16(0x29); spi_TFT->write16(0x01); spi_TFT->write16(0xBF); spi_TFT->write16(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(ST7796_PGC); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write16(0xF0); spi_TFT->write16(0x0B); spi_TFT->write16(0x11); spi_TFT->write16(0x0B);
        spi_TFT->write16(0x0A); spi_TFT->write16(0x27); spi_TFT->write16(0x3C); spi_TFT->write16(0x55);
        spi_TFT->write16(0x51); spi_TFT->write16(0x37); spi_TFT->write16(0x15); spi_TFT->write16(0x17);
        spi_TFT->write16(0x31); spi_TFT->write16(0x35);

        writeCommand(ST7796_NGC); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write16(0x4E); spi_TFT->write16(0x15); spi_TFT->write16(0x19); spi_TFT->write16(0x0B);
        spi_TFT->write16(0x09); spi_TFT->write16(0x27); spi_TFT->write16(0x34); spi_TFT->write16(0x32);
        spi_TFT->write16(0x46); spi_TFT->write16(0x38); spi_TFT->write16(0x14); spi_TFT->write16(0x16);
        spi_TFT->write16(0x26); spi_TFT->write16(0x2A);
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x3C);     // Enable extension command 2 partI
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x69);     // Enable extension command 2 partII
        if(_displayInversion == 0) {
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ST7796_INVON); // Display Inversion ON
        }
        writeCommand(ST7796_DISPON); // Display on
        delay(25);
    }
    endWrite();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setRotation(uint8_t m) {
    _rotation = m % 4; // can't be higher than 3

    if(_TFTcontroller == HX8347D) { //"HX8347D"
        startWrite();
        if(_rotation == 0) { // 0
            writeCommand(0x16); spi_TFT->write(0x08); writeCommand(0x04); spi_TFT->write(0x00);
            writeCommand(0x05); spi_TFT->write(0xEF); writeCommand(0x08); spi_TFT->write(0x01);
            writeCommand(0x09); spi_TFT->write(0x3F);
            m_h_res = HX8347D_WIDTH; m_v_res = HX8347D_HEIGHT;
        }
        if(_rotation == 1) { // 90°
            writeCommand(0x16); spi_TFT->write(0x68); writeCommand(0x04); spi_TFT->write(0x01);
            writeCommand(0x05); spi_TFT->write(0x3F); writeCommand(0x08); spi_TFT->write(0x00);
            writeCommand(0x09); spi_TFT->write(0xEF);
            m_v_res = HX8347D_WIDTH; m_h_res = HX8347D_HEIGHT;
        }
        if(_rotation == 2) { // 180
            writeCommand(0x16); spi_TFT->write(0xC8); writeCommand(0x04); spi_TFT->write(0x00);
            writeCommand(0x05); spi_TFT->write(0xEF); writeCommand(0x08); spi_TFT->write(0x01);
            writeCommand(0x09); spi_TFT->write(0x3F);
            m_h_res = HX8347D_WIDTH; m_v_res = HX8347D_HEIGHT;
        }
        if(_rotation == 3) { // 270
            writeCommand(0x16); spi_TFT->write(0xA8); writeCommand(0x04); spi_TFT->write(0x01);
            writeCommand(0x05); spi_TFT->write(0x3F); writeCommand(0x08); spi_TFT->write(0x00);
            writeCommand(0x09); spi_TFT->write(0xEF);
            m_v_res = HX8347D_WIDTH; m_h_res = HX8347D_HEIGHT;
        }
        endWrite();
    }
    if(_TFTcontroller == ILI9341) { // ILI9341
        m = ili9341_rotations[_rotation].madctl;
        m_h_res = ili9341_rotations[_rotation].width;
        m_v_res = ili9341_rotations[_rotation].height;
        startWrite();
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(m);
        endWrite();
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9486_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);
                m_h_res = ILI9486_WIDTH;
                m_v_res = ILI9486_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                m_v_res = ILI9486_WIDTH;
                m_h_res = ILI9486_HEIGHT;
                break;
            case 2:
                spi_TFT->write16(ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);
                m_h_res = ILI9486_WIDTH;
                m_v_res = ILI9486_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                m_v_res = ILI9486_WIDTH;
                m_h_res = ILI9486_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ILI9488) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9488_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);
                m_h_res = ILI9488_WIDTH;
                m_v_res = ILI9488_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                m_v_res = ILI9488_WIDTH;
                m_h_res = ILI9488_HEIGHT;
                break;
            case 2:
                spi_TFT->write(ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR);
                m_h_res = ILI9488_WIDTH;
                m_v_res = ILI9488_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                m_v_res = ILI9488_WIDTH;
                m_h_res = ILI9488_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
            case 2:
                spi_TFT->write(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796RPI) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
            case 2:
                spi_TFT->write16(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
        }
        endWrite();
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::invertDisplay(bool i) {
    startWrite();
    if(_TFTcontroller == ILI9341) { writeCommand(i ? ILI9341_INVON : ILI9341_INVOFF); }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) { writeCommand(i ? ILI9486_INVON : ILI9486_INVOFF); }
    if(_TFTcontroller == ILI9488) { writeCommand(i ? ILI9488_INVON : ILI9488_INVOFF); }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) { writeCommand(i ? ST7796_INVON : ST7796_INVOFF); }
    endWrite();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341) { // ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
        uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
        writeCommand(ILI9341_CASET);
        spi_TFT->write32(xa);
        writeCommand(ILI9341_RASET);
        spi_TFT->write32(ya);
        writeCommand(ILI9341_RAMWR);
    }
    if(_TFTcontroller == HX8347D) { // HX8347D
        writeCommand(0x02);
        spi_TFT->write(x >> 8);
        writeCommand(0x03);
        spi_TFT->write(x & 0xFF); // Column Start
        writeCommand(0x04);
        spi_TFT->write((x + w - 1) >> 8);
        writeCommand(0x05);
        spi_TFT->write((x + w - 1) & 0xFF); // Column End
        writeCommand(0x06);
        spi_TFT->write(y >> 8);
        writeCommand(0x07);
        spi_TFT->write(y & 0xFF); // Row Start
        writeCommand(0x08);
        spi_TFT->write((y + h - 1) >> 8);
        writeCommand(0x09);
        spi_TFT->write((y + h - 1) & 0xFF); // Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);  // XEND
        writeCommand(ILI9486_PASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ILI9486_RAMWR);
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);    // XEND
        writeCommand(ILI9488_PASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ILI9488_RAMWR);
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);   // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ST7796_RAMWR);
    }
    if(_TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF); // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ST7796_RAMWR);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#endif // TFT_CONTROLLER < 7