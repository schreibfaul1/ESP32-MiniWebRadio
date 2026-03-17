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
