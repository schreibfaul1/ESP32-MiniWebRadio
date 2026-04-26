// first release on 09/2019
// updated on Mar 17 2026

// #include "common.h"

#include "tft_spi.h"
#include "Arduino.h"

SPIClass*   SPItransfer;

#define __malloc_heap_psram(size) heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL)


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
TFT_SPI::TFT_SPI(SPIClass& spiInstance, int csPin){
    m_freq = 20000000;
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

    if(_TFTcontroller == ILI9341)   { m_h_res = 320; m_v_res = 240; m_rotation = 0;}
    if(_TFTcontroller == ILI9486)  { m_h_res = 480; m_v_res = 320; m_rotation = 0;}
    if(_TFTcontroller == ILI9488)   { m_h_res = 480; m_v_res = 320; m_rotation = 0;}
    if(_TFTcontroller == ST7796 )   { m_h_res = 480; m_v_res = 320; m_rotation = 0;}

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
void TFT_SPI::setDiaplayInversion(uint8_t i) {
    m_displayInversion = i;
    startWrite();
    if(_TFTcontroller == ILI9341) { writeCommand(i ? INVON : INVOFF); }
    if(_TFTcontroller == ILI9486) { writeCommand(i ? INVON : INVOFF); }
    if(_TFTcontroller == ILI9488) { writeCommand(i ? INVON : INVOFF); }
    if(_TFTcontroller == ST7796)  { writeCommand(i ? INVON : INVOFF); }
    endWrite();

}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::displayInversion() {
    if(_TFTcontroller == ILI9341) { writeCommand(m_displayInversion ? INVON : INVOFF); }
    if(_TFTcontroller == ILI9486) { writeCommand(m_displayInversion ? INVON : INVOFF); }
    if(_TFTcontroller == ILI9488) { writeCommand(m_displayInversion ? INVON : INVOFF); }
    if(_TFTcontroller == ST7796)  { writeCommand(m_displayInversion ? INVON : INVOFF); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setFrequency(uint32_t f) {
    if(f > 80000000) f = 80000000;
    m_freq = f; // overwrite default
    spi_TFT->setFrequency(m_freq);
    SPIset = SPISettings(m_freq, MSBFIRST, SPI_MODE0);
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
// int16_t TFT_SPI::width(void) const { return m_h_res; }
// int16_t TFT_SPI::height(void) const { return m_v_res; }
uint8_t TFT_SPI::getRotation(void) const { return m_rotation; }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::panelDrawBitmap(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const void* bitmap) {
    bool res = false;
    if(x0 >= x1 || y0 >= y1) {log_w("%s %i: x0 %i, y0 %i, x1 %i, y1 %i", __FILE__, __LINE__, x0, y0, x1, y1); return false;}

    int16_t w = abs(x1 - x0);
    int16_t h = abs(y1 - y0);
    uint16_t* pixels = const_cast<uint16_t*>(static_cast<const uint16_t*>(bitmap));

    startWrite();
    setAddrWindow(x0, y0, w, h);
    for(int16_t j = y0; j < y0 + h; j++) {
        writePixels(pixels + j * m_h_res + x0, w);
    }
    endWrite();

    return res;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::writeCommand(uint16_t cmd) {
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 || _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) spi_TFT->write(cmd);

    if(_TFTcontroller == ILI9486) spi_TFT->write16(cmd);
    TFT_DC_HIGH();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT_SPI::readCommand() {
    uint16_t ret = 0;
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 ||  _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) ret = spi_TFT->transfer(0);

    if(_TFTcontroller == ILI9486) ret = spi_TFT->transfer16(0);
    TFT_DC_HIGH();
    return ret;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::begin(uint8_t DC) {
    SPIset = SPISettings(m_freq, MSBFIRST, SPI_MODE0);
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
        case ILI9486:
            writeCommand(RAMWR); spi_TFT->write16(color);
            break;
        case ILI9488:
            writeCommand(RAMWR); write24BitColor(color);
            break;
        case ST7796:
            writeCommand(RAMWR); write24BitColor(color);
            break;
        default:
            if(tft_info) tft_info("unknown tft controller");
            break;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data) {
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
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
        writeCommand(SLPOUT); // Sleep out
        delay(120);
        writeCommand(RAMWR);
        displayInversion();
        writeCommand(DISPON); // Display on

        writeCommand(RAMWR);
        writeCommand(MADCTL);
        spi_TFT->write(MADCTL_MV | MADCTL_BGR);
    }
    if(_TFTcontroller == ILI9486) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9486");

        writeCommand(0x11); // Sleep out, also SW reset
        delay(120);

        writeCommand(0x3A); // Interface Pixel Format
        spi_TFT->write16(0x55);

        writeCommand(0xC2); // Power Control 3 (For Normal Mode)
        spi_TFT->write16(0x44);

        writeCommand(0xC5); // VCOM Control
        spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00);

        if(_TFTcontroller == ILI9486) {
            writeCommand(0xE0); // PGAMCTRL(alternative Positive Gamma Control)
            spi_TFT->write16(0x0F); spi_TFT->write16(0x1F); spi_TFT->write16(0x1C); spi_TFT->write16(0x0C); spi_TFT->write16(0x0F); spi_TFT->write16(0x08);
            spi_TFT->write16(0x48); spi_TFT->write16(0x98); spi_TFT->write16(0x37); spi_TFT->write16(0x0A); spi_TFT->write16(0x13); spi_TFT->write16(0x04);
            spi_TFT->write16(0x11); spi_TFT->write16(0x0D); spi_TFT->write16(0x00);

            writeCommand(0xE1); // NGAMCTRL (alternative Negative Gamma Correction)
            spi_TFT->write16(0x0F); spi_TFT->write16(0x32); spi_TFT->write16(0x2E); spi_TFT->write16(0x0B); spi_TFT->write16(0x0D); spi_TFT->write16(0x05);
            spi_TFT->write16(0x47); spi_TFT->write16(0x75); spi_TFT->write16(0x37); spi_TFT->write16(0x06); spi_TFT->write16(0x10); spi_TFT->write16(0x03);
            spi_TFT->write16(0x24); spi_TFT->write16(0x20); spi_TFT->write16(0x00);
        }
        writeCommand(MADCTL); // Memory Access Control
        spi_TFT->write16(0x48);

        displayInversion();

        writeCommand(DISPON); // Display ON
        delay(150);

        writeCommand(MADCTL);
        spi_TFT->write16(MADCTL_MV | MADCTL_BGR);
    }
    if(_TFTcontroller == ILI9488) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9488");
        writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0x00); spi_TFT->write(0x03); spi_TFT->write(0x09); spi_TFT->write(0x08); spi_TFT->write(0x16); spi_TFT->write(0x0A);
        spi_TFT->write(0x3F); spi_TFT->write(0x78); spi_TFT->write(0x4C); spi_TFT->write(0x09); spi_TFT->write(0x0A); spi_TFT->write(0x08);
        spi_TFT->write(0x16); spi_TFT->write(0x1A); spi_TFT->write(0x0F);
        writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x00); spi_TFT->write(0x16); spi_TFT->write(0x19); spi_TFT->write(0x03); spi_TFT->write(0x0F); spi_TFT->write(0x05);
        spi_TFT->write(0x32); spi_TFT->write(0x45); spi_TFT->write(0x46); spi_TFT->write(0x04); spi_TFT->write(0x0E); spi_TFT->write(0x0D);
        spi_TFT->write(0x35); spi_TFT->write(0x37); spi_TFT->write(0x0F);

        writeCommand(0xC0); // Power Control 1
        spi_TFT->write(0x17); spi_TFT->write(0x15);
        writeCommand(0xC1); // Power Control 2
        spi_TFT->write(0x41);
        writeCommand(0xC5); // VCOM Control
        spi_TFT->write(0x00); spi_TFT->write(0x12); spi_TFT->write(0x80);
        writeCommand(MADCTL); // Memory Access Control
        spi_TFT->write(0x48);
        writeCommand(0x3A); // Pixel Interface Format
        spi_TFT->write(0x66);
        writeCommand(0xB0); // Interface Mode Control
        spi_TFT->write(0x00);
        writeCommand(0xB1); // Frame Rate Control
        spi_TFT->write(0xA0);
        writeCommand(0xB4); // Display Inversion Control
        spi_TFT->write(0x02);
        writeCommand(0xB6); // Display Function Control
        spi_TFT->write(0x02); spi_TFT->write(0x02); spi_TFT->write(0x3B);
        writeCommand(0xB7); // Entry Mode Set
        spi_TFT->write(0xC6);
        writeCommand(0xF7); // Adjust Control 3
        spi_TFT->write(0xA9); spi_TFT->write(0x51); spi_TFT->write(0x2C); spi_TFT->write(0x82);
        writeCommand(SLPOUT); // Exit Sleep
        displayInversion();
        delay(120);
        writeCommand(DISPON); // Display on
        delay(25);
        writeCommand(MADCTL);
        spi_TFT->write(MADCTL_MV | MADCTL_BGR);
    }
    if(_TFTcontroller == ST7796) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796");
        writeCommand(0x01);
        delay(120);
        writeCommand(SLPOUT); // Sleep Out
        delay(120);
        writeCommand(MADCTL); // Memory Data Access Control
        spi_TFT->write(0x40);
        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0xC3);
        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x96);
        writeCommand(0xB4); // Display Inversion Control
        spi_TFT->write(0x00);
        writeCommand(0xB0); // RAM control
        spi_TFT->write(0x00);
        writeCommand(0xB5); // Blanking Porch Control
        spi_TFT->write(0x08); spi_TFT->write(0x08); spi_TFT->write(0x00); spi_TFT->write(0x64);
        writeCommand(0xC0); // Power Control 1
        spi_TFT->write(0xF0); spi_TFT->write(0x17);
        writeCommand(0xC0); // Power Control 2
        spi_TFT->write(0x14);
        writeCommand(0xC2); // Power Control 3
        spi_TFT->write(0xA7);
        writeCommand(0xC5); // VCOM Control
        spi_TFT->write(0x20);
        writeCommand(0xE8); // Display Output Ctrl Adjust
        spi_TFT->write(0x40); spi_TFT->write(0x8A); spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x29); spi_TFT->write(0x01); spi_TFT->write(0xBF); spi_TFT->write(0x33);

        writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0xF0); spi_TFT->write(0x0B); spi_TFT->write(0x11); spi_TFT->write(0x0B); spi_TFT->write(0x0A); spi_TFT->write(0x27);
        spi_TFT->write(0x3C); spi_TFT->write(0x55); spi_TFT->write(0x51); spi_TFT->write(0x37); spi_TFT->write(0x15); spi_TFT->write(0x17);
        spi_TFT->write(0x31); spi_TFT->write(0x35);

        writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x4E); spi_TFT->write(0x15); spi_TFT->write(0x19); spi_TFT->write(0x0B); spi_TFT->write(0x09); spi_TFT->write(0x27);
        spi_TFT->write(0x34); spi_TFT->write(0x32); spi_TFT->write(0x46); spi_TFT->write(0x38); spi_TFT->write(0x14); spi_TFT->write(0x16);
        spi_TFT->write(0x26); spi_TFT->write(0x2A);

        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x3C);

        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x69);
        displayInversion();
        writeCommand(DISPON); // Display on
        delay(25);

        writeCommand(MADCTL);
        spi_TFT->write(MADCTL_MV | MADCTL_BGR);
     }

    endWrite();
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setRotation(uint8_t m) {
    m_rotation = m % 4; // can't be higher than 3
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341) { // ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
        uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
        writeCommand(CASET);
        spi_TFT->write32(xa);
        writeCommand(RASET);
        spi_TFT->write32(ya);
        writeCommand(RAMWR);
    }
    if(_TFTcontroller == ILI9486) {
        writeCommand(CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);  // XEND
        writeCommand(PASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(RAMWR);
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);    // XEND
        writeCommand(PASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(RAMWR);
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);   // XEND
        writeCommand(RASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(RAMWR);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
