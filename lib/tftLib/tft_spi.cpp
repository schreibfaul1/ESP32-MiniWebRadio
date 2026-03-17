// first release on 09/2019
// updated on Apr 26 2025

#include "common.h"

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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  B I T M A P  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫              *
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  G I F  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::drawJpgFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
    if (!fs.exists(path)) {
        log_e("file %s not exists", path);
        return false;
    }
    if (maxWidth)
        m_jpgWidthMax = maxWidth;
    else
        m_jpgWidthMax = logicalWidth();
    if (maxHeight)
        m_jpgHeightMax = maxHeight;
    else
        m_jpgHeightMax = logicalHeight();

    m_jpgSdFile = fs.open(path, FILE_READ);
    if (!m_jpgSdFile) {
        log_e("Failed to open file for reading");
        JPEG_setJpgScale(1);
        return false;
    }
    JPEG_getSdJpgSize(&m_jpgWidth, &m_jpgHeight);
    m_jpegPixelBuffer = (uint16_t*)ps_calloc(m_jpgWidth * m_jpgHeight, 2);

    int res = JPEG_drawSdJpg(0, 0);
    (void)res;
    // log_w("path %s, res %i, x %i, y %i, m_jpgWidth %i, m_jpgHeight %i", path, res, x, y, m_jpgWidth, m_jpgHeight);
    m_jpgSdFile.close();

    renderRGB565(x, y, m_jpgWidth, m_jpgHeight, m_jpegPixelBuffer, NULL);

    free(m_jpegPixelBuffer);
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::JPEG_setJpgScale(uint8_t scaleFactor) {
    switch (scaleFactor) {
        case 1: m_jpgScale = 0; break;
        case 2: m_jpgScale = 1; break;
        case 4: m_jpgScale = 2; break;
        case 8: m_jpgScale = 3; break;
        default: m_jpgScale = 0;
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_SPI::JPEG_setSwapBytes(bool swapBytes) {
    m_swap = swapBytes;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
unsigned int TFT_SPI::JPEG_jd_input(JDEC* jdec, uint8_t* buf, unsigned int len) {
    uint32_t bytesLeft = 0;

    if (m_jpg_source == TJPG_ARRAY) {                                                   // Handle an array input
        if (m_array_index + len > m_array_size) { len = m_array_size - m_array_index; } // Avoid running off end of array
        if (buf) memcpy_P(buf, (const uint8_t*)(m_array_data + m_array_index), len);    // If buf is valid then copy len bytes to buffer
        m_array_index += len;                                                           // Move pointer
    } else if (m_jpg_source == TJPG_SD_FILE) {                                          // Handle SD library input
        bytesLeft = m_jpgSdFile.available();                                            // Check how many bytes are available
        if (bytesLeft < len) len = bytesLeft;
        if (buf) {
            m_jpgSdFile.read(buf, len); // Read into buffer, pointer moved as well
        } else {
            m_jpgSdFile.seek(m_jpgSdFile.position() + len); // Buffer is null, so skip data by moving pointer
        }
    }
    return len;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  Pass image block back to the sketch for rendering, may be a complete or partial MCU
int TFT_SPI::JPEG_jd_output(JDEC* jdec, void* bitmap, JRECT* jrect) {
    jdec = jdec; // Supress warning as ID is not used

    int16_t  x = jrect->left + m_jpeg_x; // Retrieve rendering parameters and add any offset
    int16_t  y = jrect->top + m_jpeg_y;
    uint16_t w = jrect->right + 1 - jrect->left;
    uint16_t h = jrect->bottom + 1 - jrect->top;
    //    if(x > m_jpgWidthMax) return true;  // Clip width and height to the maximum allowed dimensions
    //    if(y > m_jpgHeightMax) return true;
    bool r = JPEG_tft_output(x, y, w, h, (uint16_t*)bitmap);
    return r;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_SPI::JPEG_tft_output(int16_t blockX, int16_t blockY, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!bitmap || w == 0 || h == 0) return false;

    // Write MCU block into linear JPEG pixel buffer (no rotation!)
    for (uint16_t localY = 0; localY < h; ++localY) {
        for (uint16_t localX = 0; localX < w; ++localX) {

            int32_t dstX = blockX + localX;
            int32_t dstY = blockY + localY;

            if (dstX >= m_jpgWidth || dstY >= m_jpgHeight) continue;

            m_jpegPixelBuffer[dstY * m_jpgWidth + dstX] = bitmap[localY * w + localX];
        }
    }
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::JPEG_drawSdJpg(int32_t x, int32_t y) {
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::JPEG_getSdJpgSize(uint16_t* w, uint16_t* h) {

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
    // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
    #if JD_FASTDECODE == 2
        #define HUFF_BIT  10 /* Bit length to apply fast huffman decode */
        #define HUFF_LEN  (1 << HUFF_BIT)
        #define HUFF_MASK (HUFF_LEN - 1)
    #endif

const uint8_t Zig[64] = {/* Zigzag-order to raster-order conversion table */
                         0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6,  7,  14, 21, 28,
                         35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

const uint16_t Ipsf[64] = {/* See also aa_idct.png */
                           (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192), (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192),
                           (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.92388 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.63099 * 8192),
                           (uint16_t)(1.38704 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.38268 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.81226 * 8192),
                           (uint16_t)(1.70711 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.36048 * 8192),
                           (uint16_t)(1.17588 * 8192), (uint16_t)(1.63099 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.38268 * 8192), (uint16_t)(1.17588 * 8192), (uint16_t)(0.92388 * 8192),
                           (uint16_t)(0.63638 * 8192), (uint16_t)(0.32442 * 8192), (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192),
                           (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(1.08979 * 8192),
                           (uint16_t)(1.02656 * 8192), (uint16_t)(0.92388 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.61732 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.21677 * 8192),
                           (uint16_t)(0.54120 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.63638 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.42522 * 8192),
                           (uint16_t)(0.29290 * 8192), (uint16_t)(0.14932 * 8192), (uint16_t)(0.27590 * 8192), (uint16_t)(0.38268 * 8192), (uint16_t)(0.36048 * 8192), (uint16_t)(0.32442 * 8192),
                           (uint16_t)(0.27590 * 8192), (uint16_t)(0.21678 * 8192), (uint16_t)(0.14932 * 8192), (uint16_t)(0.07612 * 8192)};

    #if JD_TBLCLIP
        #define BYTECLIP(v) Clip8[(unsigned int)(v) & 0x3FF]
const uint8_t Clip8[1024] = {/* 0..255 */
                             0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
                             45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
                             88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
                             124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157,
                             158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
                             192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225,
                             226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
                             /* 256..511 */
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                             /* -512..-257 */
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             /* -256..-1 */
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    #endif

    #if JD_TBLCLIP == 0 /* JD_TBLCLIP */
uint8_t TFT_SPI::JPEG_BYTECLIP(int val) {
    if (val < 0)
        return 0;
    else if (val > 255)
        return 255;
    return (uint8_t)val;
}
    #endif
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void* TFT_SPI::JPEG_alloc_pool(JDEC* jd, size_t ndata) {
    char* rp = 0;

    ndata = (ndata + 3) & ~3; /* Align block size to the word boundary */

    if (jd->sz_pool >= ndata) {
        jd->sz_pool -= ndata;
        rp = (char*)jd->pool;           /* Get start of available memory pool */
        jd->pool = (void*)(rp + ndata); /* Allocate requierd bytes */
    }

    return (void*)rp; /* Return allocated memory block (NULL:no memory to allocate) */
} // ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
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
                     // log_e("Unknown segment %02X", marker);
                /* Skip segment data (null pointer specifies to remove data from the stream) */
                if (JPEG_jd_input(jd, 0, len) != len) return JDR_INP;
        }
    }
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT_SPI::JPEG_jd_decomp(JDEC* jd, uint8_t scale) {
    unsigned int x, y, mx, my;
    uint16_t     rst, rsc;
    uint8_t      rc;

    if (scale > (JD_USE_SCALE ? 3 : 0)) return JDR_PAR;
    jd->scale = scale;
    mx = jd->msx * 8;
    my = jd->msy * 8;                         /* Size of the MCU (pixel) */
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

    // png_draw_into_AddrWindow(png_pos_x, png_pos_y, png_width, png_height, png_outbuffer, png_outbuff_size, png_format);
    png_draw_into_Framebuffer(png_pos_x, png_pos_y, png_width, png_height, png_outbuffer, png_outbuff_size, png_format);

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
void TFT_SPI::png_draw_into_Framebuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char* rgbaBuffer, uint32_t, uint8_t) {
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

        // Driving ability Setting
        writeCommand(0x11); // Sleep out, also SW reset
        delay(120);

        writeCommand(0x3A); // Interface Pixel Format
        spi_TFT->write16(0x55);

        writeCommand(0xC2); // Power Control 3 (For Normal Mode)
        spi_TFT->write16(0x44);

        writeCommand(0xC5); // VCOM Control
        spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00);

        // if(_TFTcontroller == ILI9486) {
        //     writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
        //     spi_TFT->write16(0x00); spi_TFT->write16(0x04); spi_TFT->write16(0x0E); spi_TFT->write16(0x08); spi_TFT->write16(0x17); spi_TFT->write16(0x0A);
        //     spi_TFT->write16(0x40); spi_TFT->write16(0x79); spi_TFT->write16(0x4D); spi_TFT->write16(0x07); spi_TFT->write16(0x0E); spi_TFT->write16(0x0A);
        //     spi_TFT->write16(0x1A); spi_TFT->write16(0x1D); spi_TFT->write16(0x0F);
        //     writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
        //     spi_TFT->write16(0x00); spi_TFT->write16(0x1B); spi_TFT->write16(0x1F); spi_TFT->write16(0x02); spi_TFT->write16(0x10); spi_TFT->write16(0x05);
        //     spi_TFT->write16(0x32); spi_TFT->write16(0x34); spi_TFT->write16(0x43); spi_TFT->write16(0x02); spi_TFT->write16(0x0A); spi_TFT->write16(0x09);
        //     spi_TFT->write16(0x33); spi_TFT->write16(0x37); spi_TFT->write16(0x0F);
        // }

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
    //==========================================
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
    //==========================================
    if(_TFTcontroller == ST7796) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796");
        writeCommand(0x01);
        delay(120);
        writeCommand(SLPOUT); // Sleep Out
        delay(120);
        writeCommand(MADCTL); // Memory Data Access Control
        spi_TFT->write(0x40);
        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0xC3);       // Enable extension command 2 partI
        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x96);       // Enable extension command 2 partII
        writeCommand(0xB4); // Display Inversion Control
        spi_TFT->write(0x00);
        writeCommand(0xB0); // RAM control
        spi_TFT->write(0x00);
        writeCommand(0xB5); // Blanking Porch Control
        spi_TFT->write(0x08); spi_TFT->write(0x08); spi_TFT->write(0x00); spi_TFT->write(0x64);
        writeCommand(0xC0); // Power Control 1
        spi_TFT->write(0xF0); spi_TFT->write(0x17);
        writeCommand(0xC0); // Power Control 2
        spi_TFT->write(0x14);      //
        writeCommand(0xC2); // Power Control 3
        spi_TFT->write(0xA7);
        writeCommand(0xC5); // VCOM Control
        spi_TFT->write(0x20);
        writeCommand(0xE8); // Display Output Ctrl Adjust
        spi_TFT->write(0x40); spi_TFT->write(0x8A); spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x29); spi_TFT->write(0x01); spi_TFT->write(0xBF); spi_TFT->write(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0xF0); spi_TFT->write(0x0B); spi_TFT->write(0x11); spi_TFT->write(0x0B); spi_TFT->write(0x0A); spi_TFT->write(0x27);
        spi_TFT->write(0x3C); spi_TFT->write(0x55); spi_TFT->write(0x51); spi_TFT->write(0x37); spi_TFT->write(0x15); spi_TFT->write(0x17);
        spi_TFT->write(0x31); spi_TFT->write(0x35);

        writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x4E); spi_TFT->write(0x15); spi_TFT->write(0x19); spi_TFT->write(0x0B); spi_TFT->write(0x09); spi_TFT->write(0x27);
        spi_TFT->write(0x34); spi_TFT->write(0x32); spi_TFT->write(0x46); spi_TFT->write(0x38); spi_TFT->write(0x14); spi_TFT->write(0x16);
        spi_TFT->write(0x26); spi_TFT->write(0x2A);

        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x3C);       // Enable extension command 2 partI

        writeCommand(0xF0); // Command Set Control
        spi_TFT->write(0x69);       // Enable extension command 2 partII
        displayInversion();
        writeCommand(DISPON); // Display on
        delay(25);

        writeCommand(MADCTL);
        spi_TFT->write(MADCTL_MV | MADCTL_BGR);
     } //===============================================================================

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
