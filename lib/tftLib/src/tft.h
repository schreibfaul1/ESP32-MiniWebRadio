// first release on 09/2019
// updated on Feb 08 2024


#pragma once

#include "Arduino.h"
#include "FS.h"
#include "SPI.h"
#include "SD.h"
#include "vector"
#include "driver/gpio.h"
#include "fonts/fontsdef.h"
#include "fonts/TimesNewRoman.h"
#include "fonts/Garamond.h"
#include "fonts/FreeSerifItalic.h"
#include "fonts/BigNumbers.h"
#include "fonts/Arial.h"
#include "fonts/Z300.h"

using namespace std;


extern __attribute__((weak)) void tft_info(const char*);
extern __attribute__((weak)) void tp_positionXY(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_pressed(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_released(uint16_t x, uint16_t y);
extern __attribute__((weak)) void tp_long_released();

#define ANSI_ESC_BLACK      "\033[30m"
#define ANSI_ESC_RED        "\033[31m"
#define ANSI_ESC_GREEN      "\033[32m"
#define ANSI_ESC_YELLOW     "\033[33m"
#define ANSI_ESC_BLUE       "\033[34m"
#define ANSI_ESC_MAGENTA    "\033[35m"
#define ANSI_ESC_CYAN       "\033[36m"
#define ANSI_ESC_WHITE      "\033[37m"
#define ANSI_ESC_RESET      "\033[0m"

#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define HX8347D_WIDTH  240
#define HX8347D_HEIGHT 320
#define ILI9486_WIDTH  320
#define ILI9486_HEIGHT 480
#define ILI9488_WIDTH  320
#define ILI9488_HEIGHT 480
#define ST7796_WIDTH   320
#define ST7796_HEIGHT  480

// RGB565 Color definitions
#define TFT_AQUAMARINE      0x7FFA // 127, 255, 212
#define TFT_BEIGE           0xF7BB // 245, 245, 220
#define TFT_BLACK           0x0000 //   0,   0,   0
#define TFT_BLUE            0x001F //   0,   0, 255
#define TFT_BROWN           0xA145 // 165,  42,  42
#define TFT_CHOCOLATE       0xD343 // 210, 105,  30
#define TFT_CORNSILK        0xFFDB // 255, 248, 220
#define TFT_CYAN            0x07FF //   0, 255, 255
#define TFT_DARKGREEN       0x0320 //   0, 101,   0
#define TFT_DARKGREY        0xAD55 // 169, 169, 169
#define TFT_DARKCYAN        0x0451 //   0, 139, 139
#define TFT_DARKRED         0x5800 //  90,   0,   0
#define TFT_DARKYELLOW      0x6B60 // 110, 110,   0
#define TFT_DEEPSKYBLUE     0x05FF //   0, 191, 255
#define TFT_GRAY            0x8410 // 128, 128, 128
#define TFT_GREEN           0x07E0 //   0, 255,   0
#define TFT_GREENYELLOW     0xAFE5 // 173, 255,  47
#define TFT_GOLD            0xFEA0 // 255, 215,   0
#define TFT_HOTPINK         0xFB56 // 255, 105, 180
#define TFT_LAVENDER        0xE73F // 230, 230, 250
#define TFT_LAWNGREEN       0x7FE0 // 124, 252,   0
#define TFT_LIGHTBLUE       0xAEDC // 173, 216, 230
#define TFT_LIGHTCYAN       0xE7FF // 224, 255, 255
#define TFT_LIGHTGREY       0xD69A // 211, 211, 211
#define TFT_LIGHTGREEN      0x9772 // 144, 238, 144
#define TFT_LIGHTYELLOW     0xFFFC // 255, 255, 224
#define TFT_LIME            0x07E0 //   0. 255,   0
#define TFT_MAGENTA         0xF81F // 255,   0, 255
#define TFT_MAROON          0x7800 // 128,   0,   0
#define TFT_MEDIUMVIOLETRED 0xC0B0 // 199,  21, 133
#define TFT_NAVY            0x000F //   0,   0, 128
#define TFT_OLIVE           0x7BE0 // 128, 128,   0
#define TFT_ORANGE          0xFD20 // 255, 165,   0
#define TFT_PINK            0xFE19 // 255, 192, 203
#define TFT_PURPLE          0x780F // 128,   0, 128
#define TFT_RED             0xF800 // 255,   0,   0
#define TFT_SANDYBROWN      0xF52C // 244, 164,  96
#define TFT_TURQUOISE       0x471A //  64, 224, 208
#define TFT_VIOLET          0x801F // 128,   0, 255
#define TFT_WHITE           0xFFFF // 255, 255, 255
#define TFT_YELLOW          0xFFE0 // 255, 255,   0


#if TFT_FONT == 1
#define TFT_TIMES_NEW_ROMAN
#elif TFT_FONT == 2
#define TFT_FREE_SERIF_ITALIC
#elif TFT_FONT == 3
#define TFT_ARIAL
#elif TFT_FONT == 4
#define TFT_Z300
#else
#define TFT_GARAMOND // if nothing is choosen
#endif

#define TFT_ALIGN_RIGHT          (1)
#define TFT_ALIGN_LEFT           (2)
#define TFT_ALIGN_CENTER         (3)


class TFT{
    protected:
    File gif_file;
    public:
        TFT(uint8_t TFT_id = 0, uint8_t dispInv = 0);
        virtual ~TFT(){}
        void      begin(uint8_t CS, uint8_t DC, uint8_t spi, uint8_t mosi, uint8_t miso, uint8_t sclk);
        void      setFrequency(uint32_t f);
        void      setRotation(uint8_t r);
        void      invertDisplay(bool i);
        void      scrollTo(uint16_t y);


        // Recommended Non-Transaction
        void      drawLine(int16_t Xpos0, int16_t Ypos0, int16_t Xpos1, int16_t Ypos1, uint16_t color);
        void      drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void      drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        void      drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color);
        void      readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
        void      fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void      drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
        void      fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
        void      fillScreen(uint16_t color);
        void      drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
        void      fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
        void      drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
        void      fillCircle(int16_t Xm, int16_t Ym, uint16_t r, uint16_t color);
        bool      drawBmpFile(fs::FS &fs, const char * path, uint16_t x=0, uint16_t y=0, uint16_t maxWidth=0, uint16_t maxHeight=0, uint16_t offX=0, uint16_t offY=0);
        bool      drawGifFile(fs::FS &fs, const char * path, uint16_t x, uint16_t y, uint8_t repeat);
        bool      drawJpgFile(fs::FS &fs, const char * path, uint16_t x=0, uint16_t y=0, uint16_t maxWidth=0, uint16_t maxHeight=0, uint16_t offX=0, uint16_t offY=0);
        void      writeInAddrWindow(const uint8_t* bmi, uint16_t posX, uint16_t poxY, uint16_t width, uint16_t height);
        size_t    writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t align = TFT_ALIGN_LEFT, bool narrow = false, bool noWrap = false);

        inline void setBackGoundColor(uint16_t BGcolor){_backGroundColor = BGcolor; fillScreen(BGcolor);}
        inline void setTextColor(uint16_t FGcolor){_textColor = FGcolor;}
        void setFont(uint16_t font);
        inline void setTextOrientation(uint16_t orientation=0){_textorientation=orientation;} //0 h other v
        int16_t height(void) const;
        int16_t width(void) const;
        uint8_t getRotation(void) const;

    private:

        enum Ctrl {ILI9341 = 0, HX8347D = 1, ILI9486a = 2, ILI9486b = 3, ILI9488 = 4, ST7796 = 5, ST7796RPI = 6};
        uint8_t _TFTcontroller = ILI9341;
        SPISettings     TFT_SPI;                     // SPI settings for this slave
        SPIClass*       spi_TFT = NULL;             // use in class TP

        typedef struct{
        	const uint8_t* glyph_bitmap;
        	const lv_font_fmt_txt_glyph_dsc_t* glyph_dsc;
        	const lv_font_fmt_txt_cmap_t* cmaps;
        	      uint32_t  range_start;
        	      uint16_t  range_length;
        		  uint16_t  line_height;
        		  uint16_t  font_height;
        		  uint16_t  base_line;
                  uint16_t* lookup_table;
        } fonts_t;
        fonts_t   _current_font;
        uint8_t   _font;


        uint32_t  _freq;
        uint16_t  _height;
        uint16_t  _width;
        uint8_t   _rotation;
        uint8_t   _displayInversion;
        uint16_t  _backGroundColor = TFT_WHITE;
        uint16_t  _textColor = TFT_BLACK;
        uint8_t   _textorientation=0;
        uint8_t   _TFT_DC  = 21;    /* Data or Command */
        uint8_t   _TFT_CS  = 22;    /* SPI Chip select */
        uint8_t   _TFT_SCK = 18;
        uint8_t   _TFT_MISO= 19;
        uint8_t   _TFT_MOSI= 23;
        uint8_t  buf[1024];
        char     chbuf[256];

    //    ------------GIF-------------------

        bool debug=false;

        vector<unsigned short>  gif_next;
        vector<uint8_t>         gif_vals;
        vector<uint8_t>         gif_stack;
        vector<uint16_t>        gif_GlobalColorTable;
        vector<uint16_t>        gif_LocalColorTable;

        const uint8_t gif_MaxLzwBits = 12;

        bool gif_decodeSdFile_firstread = false;
        bool gif_GlobalColorTableFlag = false;
        bool gif_LocalColorTableFlag = false;
        bool gif_SortFlag = false;
        bool gif_TransparentColorFlag = false;
        bool gif_UserInputFlag = false;
        bool gif_ZeroDataBlock = 0;
        bool gif_InterlaceFlag = false;

        char gif_buffer[15];
        char gif_DSBbuffer[256]; // DataSubBlock

        String gif_GifHeader = "";

        uint8_t gif_BackgroundColorIndex = 0;
        uint8_t gif_BlockTerninator = 0;
        uint8_t gif_CharacterCellWidth = 0;
        uint8_t gif_CharacterCellHeight = 0;
        uint8_t gif_CodeSize = 0;
        uint8_t gif_ColorResulution = 0;
        uint8_t gif_DisposalMethod = 0;
        uint8_t gif_ImageSeparator = 0;
        uint8_t gif_lenDatablock = 0;
        uint8_t gif_LZWMinimumCodeSize = 0;
        uint8_t gif_PackedFields = 0;
        uint8_t gif_PixelAspectRatio = 0;
        uint8_t gif_TextBackgroundColorIndex = 0;
        uint8_t gif_TextForegroundColorIndex = 0;
        uint8_t gif_TransparentColorIndex = 0;

        uint16_t gif_ClearCode = 0;
        uint16_t gif_DelayTime = 0;
        uint16_t gif_EOIcode = 0; // End Of Information

        uint16_t gif_ImageHeight = 0;
        uint16_t gif_ImageWidth = 0;
        uint16_t gif_ImageLeftPosition = 0;
        uint16_t gif_ImageTopPosition = 0;
        uint16_t gif_LogicalScreenWidth = 0;
        uint16_t gif_LogicalScreenHeight = 0;
        uint16_t gif_MaxCode = 0;
        uint16_t gif_MaxCodeSize = 0;
        uint16_t gif_SizeOfGlobalColorTable = 0;
        uint16_t gif_SizeOfLocalColorTable = 0;
        uint16_t gif_TextGridLeftPosition = 0;
        uint16_t gif_TextGridTopPosition = 0;
        uint16_t gif_TextGridWidth = 0;
        uint16_t gif_TextGridHeight = 0;

        int32_t GIF_readGifItems();
        bool    GIF_decodeGif(uint16_t x, uint16_t y);
        void    GIF_freeMemory();
        void    GIF_readHeader();
        void    GIF_readLogicalScreenDescriptor();
        void    GIF_readImageDescriptor();
        void    GIF_readLocalColorTable();
        void    GIF_readGlobalColorTable();
        void    GIF_readGraphicControlExtension();
        uint8_t GIF_readPlainTextExtension(char* buf);
        uint8_t GIF_readApplicationExtension(char* buf);
        uint8_t GIF_readCommentExtension(char *buf);
        uint8_t GIF_readDataSubBlock(char *buf);
        bool    GIF_readExtension(char Label);
        int32_t GIF_GetCode(int32_t code_size, int32_t flag);
        int32_t GIF_LZWReadByte(bool init);
        bool    GIF_ReadImage(uint16_t x, uint16_t y);

        //------------TFT-------------------

        inline int32_t minimum(int32_t a, int32_t b){if(a < b) return a; else return b;}

        inline void TFT_DC_HIGH() {gpio_set_level((gpio_num_t)_TFT_DC, 1);}
        inline void TFT_DC_LOW()  {gpio_set_level((gpio_num_t)_TFT_DC, 0);}
        inline void TFT_CS_HIGH() {gpio_set_level((gpio_num_t)_TFT_CS, 1);}
        inline void TFT_CS_LOW()  {gpio_set_level((gpio_num_t)_TFT_CS, 0);}

        inline void _swap_int16_t(int16_t &a, int16_t &b) { int16_t t = a; a = b; b = t; }
        void        init();
        void        writeCommand(uint16_t cmd);
        uint16_t    readCommand();

        // Transaction API not used by GFX
        void      setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void      readAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void      write24BitColor(uint16_t color);
        void      writePixels(uint16_t * colors, uint32_t len);
        void      writeColor(uint16_t color, uint32_t len);
        uint16_t  color565(uint8_t r, uint8_t g, uint8_t b);

        // Required Non-Transaction
        void      drawPixel(int16_t x, int16_t y, uint16_t color);

        // Transaction API
        void      startWrite(void);
        void      endWrite(void);
        void      writePixel(int16_t x, int16_t y, uint16_t color);
        void      writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void      writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void      writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

        void      fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
        void      drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
        void      startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
        void      endBitmap();
        void      startJpeg();
        void      endJpeg();

        void      bmpSkipPixels(fs::File &file, uint8_t bitsPerPixel, size_t len);
        void      bmpAddPixels(fs::File &file, uint8_t bitsPerPixel, size_t len);
        void      drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);
        void      renderJPEG(int32_t xpos, int32_t ypos, uint16_t maxWidth, uint16_t maxHeight);

        uint8_t   readcommand8(uint8_t reg, uint8_t index = 0);
};

//-----------------------------------------------------------------------------------------------------------------------
class JPEGDecoder {

private:

    typedef enum {  // Scan types
        PJPG_GRAYSCALE,
        PJPG_YH1V1,
        PJPG_YH2V1,
        PJPG_YH1V2,
        PJPG_YH2V2
    } pjpeg_scan_type_t;

    typedef enum
    {
       M_SOF0  = 0xC0, M_SOF1  = 0xC1, M_SOF2  = 0xC2, M_SOF3  = 0xC3, M_SOF5  = 0xC5,
       M_SOF6  = 0xC6, M_SOF7  = 0xC7, M_JPG   = 0xC8, M_SOF9  = 0xC9, M_SOF10 = 0xCA,
       M_SOF11 = 0xCB, M_SOF13 = 0xCD, M_SOF14 = 0xCE, M_SOF15 = 0xCF, M_DHT   = 0xC4,
       M_RST0  = 0xD0, M_RST1  = 0xD1, M_RST2  = 0xD2, M_RST3  = 0xD3, M_RST4  = 0xD4,
       M_RST5  = 0xD5, M_RST6  = 0xD6, M_RST7  = 0xD7, M_SOI   = 0xD8, M_EOI   = 0xD9,
       M_SOS   = 0xDA, M_DQT   = 0xDB, M_DNL   = 0xDC, M_DRI   = 0xDD, M_DHP   = 0xDE,
       M_EXP   = 0xDF, M_APP0  = 0xE0, M_APP15 = 0xEF, M_JPG0  = 0xF0, M_JPG13 = 0xFD,
       M_COM   = 0xFE, M_TEM   = 0x01, M_ERROR = 0x100,RST0    = 0xD0, M_DAC   = 0xCC
    } JPEG_MARKER;

    typedef struct{
        int32_t     m_width;        // Image resolution
        int32_t     m_height;
        int32_t     m_comps;        // Number of components (1 or 3)
        int32_t     m_MCUSPerRow;   // Total number of minimum coded units (MCU's) per row/col.
        int32_t     m_MCUSPerCol;
        pjpeg_scan_type_t m_scanType; // Scan type
        int32_t     m_MCUWidth;     // MCU width/height in pixels (each is either 8 or 16 depending on the scan type)
        int32_t     m_MCUHeight;
        uint8_t *m_pMCUBufR;
        uint8_t *m_pMCUBufG;
        uint8_t *m_pMCUBufB;
    } pjpeg_image_info_t;

    typedef struct HufftableT{
        uint16_t mMinCode[16];
        uint16_t mMaxCode[16];
        uint8_t mValPtr[16];
    }HuffTable;

    typedef uint8_t(*pjpeg_need_bytes_callback_t)
        (uint8_t* pBuf, uint8_t buf_size, uint8_t* pBytes_actually_read, void* pCallback_data);

    pjpeg_scan_type_t  scanType, gScanType;
    pjpeg_image_info_t image_info;
    pjpeg_need_bytes_callback_t g_pNeedBytesCallback;
    HuffTable gHuffTab0, gHuffTab1, gHuffTab2, gHuffTab3;

    File g_pInFileSd;

    const uint8_t  JPEG_ARRAY = 0;
    const uint8_t  JPEG_FS_FILE=1;
    const uint8_t  JPEG_SD_FILE=2;
    const uint16_t PJPG_MAX_WIDTH       = 16384;
    const uint16_t PJPG_MAX_HEIGHT      = 16384;
    const uint8_t  PJPG_MAXCOMPSINSCAN  = 3;
    const uint8_t  PJPG_DCT_SCALE_BITS  = 7;
    const uint8_t  PJPG_WINOGRAD_QUANT_SCALE_BITS = 10;

    static const uint16_t PJPG_MAX_IN_BUF_SIZE    = 256;

    const uint8_t
    PJPG_NO_MORE_BLOCKS  =  1,  PJPG_TOO_MANY_COMPONENTS      = 11,
    PJPG_BAD_DHT_COUNTS  =  2,  PJPG_BAD_VARIABLE_MARKER      = 12,
    PJPG_BAD_DHT_INDEX   =  3,  PJPG_W_EXTRA_BYTES_BEFORE_MARKER = 16,
    PJPG_BAD_DHT_MARKER  =  4,  PJPG_NO_ARITHMITIC_SUPPORT    = 17,
    PJPG_BAD_DQT_MARKER  =  5,  PJPG_UNEXPECTED_MARKER        = 18,
    PJPG_BAD_DQT_TABLE   =  6,  PJPG_UNSUPPORTED_MARKER       = 20,
    PJPG_BAD_PRECISION   =  7,  PJPG_UNDEFINED_QUANT_TABLE    = 23,
    PJPG_BAD_HEIGHT      =  8,  PJPG_UNDEFINED_HUFF_TABLE     = 24,
    PJPG_BAD_WIDTH       =  9,  PJPG_UNSUPPORTED_COLORSPACE   = 26,
    PJPG_BAD_SOF_LENGTH  = 10,  PJPG_UNSUPPORTED_SAMP_FACTORS = 27,
    PJPG_BAD_DRI_LENGTH  = 13,  PJPG_BAD_RESTART_MARKER       = 29,
    PJPG_BAD_SOS_LENGTH  = 14,  PJPG_BAD_SOS_SPECTRAL         = 31,
    PJPG_BAD_SOS_COMP_ID = 15,  PJPG_BAD_SOS_SUCCESSIVE       = 32,
    PJPG_NOT_JPEG        = 19,  PJPG_STREAM_READ_ERROR        = 33,
    PJPG_BAD_DQT_LENGTH  = 21,  PJPG_UNSUPPORTED_COMP_IDENT   = 35,
    PJPG_TOO_MANY_BLOCKS = 22,  PJPG_UNSUPPORTED_QUANT_TABLE  = 36,
    PJPG_NOT_SINGLE_SCAN = 25,
    PJPG_DECODE_ERROR    = 28,
    PJPG_ASSERTION_ERROR = 30,
    PJPG_NOTENOUGHMEM    = 34,
    PJPG_UNSUPPORTED_MODE= 37;  // picojpeg doesn't support progressive JPEG's

    const uint8_t ZAG[64] = {
         0,   1,   8,  16,   9,   2,   3,  10,  17,  24,  32,  25,  18,  11,   4,   5,
        12,  19,  26,  33,  40,  48,  41,  34,  27,  20,  13,   6,   7,  14,  21,  28,
        35,  42,  49,  56,  57,  50,  43,  36,  29,  22,  15,  23,  30,  37,  44,  51,
        58,  59,  52,  45,  38,  31,  39,  46,  53,  60,  61,  54,  47,  55, 62,  63};

    const uint8_t gWinogradQuant[64] = {
       128, 178, 178, 167, 246, 167, 151, 232, 232, 151, 128, 209, 219, 209, 128, 101,
       178, 197, 197, 178, 101,  69, 139, 167, 177, 167, 139,  69,  35,  96, 131, 151,
       151, 131,  96,  35,  49,  91, 118, 128, 118,  91,  49,  46,  81, 101, 101,  81,
        46,  42,  69,  79,  69,  42,  35,  54,  54,  35,  28,  37,  28,  19,  19,  10};

    uint8_t   status=0;
    uint8_t   jpg_source=0;
    uint8_t   gHuffVal0[16],  gHuffVal1[16];
    uint8_t   gHuffVal2[256], gHuffVal3[256];
    uint8_t   gCompsInScan=0;
    uint8_t   gValidHuffTables=0;
    uint8_t   gValidQuantTables=0;
    uint8_t   gTemFlag=0;
    uint8_t   gInBuf[PJPG_MAX_IN_BUF_SIZE];
    uint8_t   gInBufOfs=0;
    uint8_t   gInBufLeft=0;
    uint8_t   gBitsLeft=0;
    uint8_t   gCompsInFrame=0;
    uint8_t   gMaxBlocksPerMCU=0;
    uint8_t   gMaxMCUXSize=0;
    uint8_t   gMaxMCUYSize=0;
    uint8_t   gCompList [3];
    uint8_t   gCompDCTab[3]; // 0,1
    uint8_t   gCompACTab[3]; // 0,1
    uint8_t   gCompIdent[3];
    uint8_t   gCompHSamp[3];
    uint8_t   gCompVSamp[3];
    uint8_t   gCompQuant[3];
    uint8_t   gCallbackStatus=0;
    uint8_t   gReduce=0;
    uint8_t   gMCUOrg[6];
    uint8_t   gMCUBufR[256];
    uint8_t   gMCUBufG[256];
    uint8_t   gMCUBufB[256];
    int16_t   is_available=0;
    int16_t   mcu_x;
    int16_t   mcu_y;
    int16_t   gCoeffBuf[8*8];
    int16_t   gQuant0[8*8];
    int16_t   gQuant1[8*8];
    int16_t   gLastDC[3];
    uint16_t  g_nInFileSize=0;
    uint16_t  g_nInFileOfs=0;
    uint16_t  row_pitch=0;
    uint16_t  decoded_width=0, decoded_height=0;
    uint16_t  row_blocks_per_mcu=0, col_blocks_per_mcu=0;
    uint16_t  gBitBuf=0;
    uint16_t  gImageXSize=0, gImageYSize=0;
    uint16_t  gRestartInterval=0;
    uint16_t  gNextRestartNum=0;
    uint16_t  gRestartsLeft=0;
    uint8_t*  jpg_data=0;
    uint16_t  gMaxMCUSPerRow=0;
    uint16_t  gMaxMCUSPerCol=0;
    uint16_t  gNumMCUSRemaining=0;

    void *g_pCallback_data;

    JPEGDecoder *thisPtr;
    int32_t comps=0;
    int32_t MCUSPerRow=0;
    int32_t MCUSPerCol=0;
    char chbuf[256];

public:
    uint16_t *pImage=0;
    int32_t width=0;
    int32_t height=0;
    int32_t MCUWidth=0;
    int32_t MCUHeight=0;
    int32_t MCUx=0;
    int32_t MCUy=0;

    JPEGDecoder();
    ~JPEGDecoder();
    int32_t read(void);
    int32_t decodeSdFile (File g_pInFile);
    void abort(void);
        // Initializes the decompressor. Returns 0 on success, or one of the above error codes on failure. pNeed_bytes_callback will be called
        // to fill the decompressor's internal input buffer. If reduce is 1, only the first pixel of each block will be decoded. This mode is
        // much faster because it skips the AC dequantization, IDCT and chroma upsampling of every image pixel.Not thread safe.
    uint8_t pjpeg_decode_init(pjpeg_image_info_t *pInfo, pjpeg_need_bytes_callback_t pNeed_bytes_callback, void *pCallback_data, uint8_t reduce);
        // Decompresses the file's next MCU. Returns 0 on success, PJPG_NO_MORE_BLOCKS if no more blocks are available, or an error code.
        // Must be called a total of m_MCUSPerRow*m_MCUSPerCol times to completely decompress the image. Not thread safe.
    uint8_t pjpeg_decode_mcu();

private:
    int32_t available(void);
    int32_t readSwappedBytes(void);
    int32_t decodeArray(const uint8_t array[], uint32_t  array_size);
    int32_t decode_mcu(void);
    int32_t decodeCommon(void);
    static uint8_t pjpeg_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t* pBytes_actually_read, void *pCallback_data);
    uint8_t pjpeg_need_bytes_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t*pBytes_actually_read, void *pCallback_data);

    int16_t replicateSignBit16(int8_t n);
    uint16_t getBits(uint8_t numBits, uint8_t FFCheck);
    uint16_t getExtendTest(uint8_t i);
    int16_t getExtendOffset(uint8_t i);
    HuffTable* getHuffTable(uint8_t index);
    uint8_t* getHuffVal(uint8_t index);
    uint8_t readDHTMarker(void);
    uint8_t readDQTMarker(void);
    uint8_t locateSOIMarker(void);
    uint8_t locateSOFMarker(void);
    uint8_t locateSOSMarker(uint8_t* pFoundEOI);
    uint8_t init(void);
    uint8_t processRestart(void);
    uint8_t findEOI(void);
    uint8_t checkHuffTables(void);
    uint8_t checkQuantTables(void);
    uint8_t initScan(void);
    uint8_t initFrame(void);
    uint8_t readSOFMarker(void);
    uint8_t skipVariableMarker(void);
    uint8_t readDRIMarker(void);
    uint8_t readSOSMarker(void);
    uint8_t nextMarker(void);
    uint8_t processMarkers(uint8_t* pMarker);
    uint8_t huffDecode(const HuffTable* pHuffTable, const uint8_t* pHuffVal);
    uint8_t decodeNextMCU(void);
    void fillInBuf(void);
    void fixInBuffer(void);
    void idctRows(void);
    void idctCols(void);
    void createWinogradQuant(int16_t* pQuant);
    void huffCreate(const uint8_t* pBits, HuffTable* pHuffTable);
    void upsampleCb(uint8_t srcOfs, uint8_t dstOfs);
    void upsampleCbH(uint8_t srcOfs, uint8_t dstOfs);
    void upsampleCbV(uint8_t srcOfs, uint8_t dstOfs);
    void upsampleCr(uint8_t srcOfs, uint8_t dstOfs);
    void upsampleCrH(uint8_t srcOfs, uint8_t dstOfs);
    void upsampleCrV(uint8_t srcOfs, uint8_t dstOfs);
    void copyY(uint8_t dstOfs);
    void convertCb(uint8_t dstOfs);
    void convertCr(uint8_t dstOfs);
    void transformBlock(uint8_t mcuBlock);
    void transformBlockReduce(uint8_t mcuBlock);

    inline int32_t jpg_min(int32_t a, int32_t b){
        if(a < b) return a; else return b;
    }
    inline int16_t arithmeticRightShiftN16(int16_t x, int8_t n){
        int16_t r = (uint16_t)x >> (uint8_t)n;
        if (x < 0) r |= replicateSignBit16(n);
        return r;
    }
     inline int32_t arithmeticRightShift8L(long x){
        int32_t r = (uint32_t)x >> 8U;
        if (x < 0) r |= ~(~(uint32_t)0U >> 8U);
        return r;
     }
     inline uint8_t getOctet(uint8_t FFCheck){
        uint8_t c = getChar();
        if ((FFCheck) && (c == 0xFF)){ uint8_t n = getChar();
           if (n){ stuffChar(n); stuffChar(0xFF);}
        } return c;
     }
     inline uint16_t getBits1(uint8_t numBits){
         return getBits(numBits, 0);
     }
     inline uint16_t getBits2(uint8_t numBits){
        return getBits(numBits, 1);
     }
     inline uint8_t getChar(void){
        if (!gInBufLeft){ fillInBuf(); if (!gInBufLeft){
              gTemFlag = ~gTemFlag; return gTemFlag ? 0xFF : 0xD9;}}
        gInBufLeft--; return gInBuf[gInBufOfs++];
     }
     inline void stuffChar(uint8_t i){
        gInBufOfs--;  gInBuf[gInBufOfs] = i; gInBufLeft++;
     }
     inline uint8_t getBit(void){
        uint8_t ret = 0;
        if (gBitBuf & 0x8000) ret = 1;
        if (!gBitsLeft){gBitBuf |= getOctet(1); gBitsLeft += 8;}
        gBitsLeft--; gBitBuf <<= 1;
        return ret;
     }
     inline int16_t huffExtend(uint16_t x, uint8_t s){
        return ((x < getExtendTest(s)) ? ((int16_t)x + getExtendOffset(s)) : (int16_t)x);
     }
     inline uint16_t getMaxHuffCodes(uint8_t index){
         return (index < 2) ? 12 : 255;
     }
     inline int16_t imul_b1_b3(int16_t w){       // 1/cos(4*pi/16)[362, 256+106]
        long x = (w * 362L); x += 128L; return (int16_t)(arithmeticRightShift8L(x));
     }
     inline int16_t imul_b2(int16_t w){          // 1/cos(6*pi/16)[669, 256+256+157]
        long x = (w * 669L); x += 128L; return (int16_t)(arithmeticRightShift8L(x));
     }
     inline int16_t imul_b4(int16_t w){          // 1/cos(2*pi/16)[277, 256+21]
        long x = (w * 277L); x += 128L; return (int16_t)(arithmeticRightShift8L(x));
     }
     inline int16_t imul_b5(int16_t w){          // 1/(cos(2*pi/16) + cos(6*pi/16))[196, 196]
        long x = (w * 196L); x += 128L; return (int16_t)(arithmeticRightShift8L(x));
     }
     inline uint8_t clamp(int16_t s){
        if ((uint16_t)s > 255U){ if (s < 0) return 0; else if (s > 255) return 255;}
        return (uint8_t)s;
     }
     inline uint8_t addAndClamp(uint8_t a, int16_t b){
        b = a + b; if ((uint16_t)b > 255U){if (b < 0)  return 0;else if (b > 255) return 255;}
        return (uint8_t)b;
     }
     inline uint8_t subAndClamp(uint8_t a, int16_t b){
        b = a - b; if ((uint16_t)b > 255U) {if (b < 0) return 0; else if (b > 255) return 255;}
        return (uint8_t)b;
     }
     inline int16_t PJPG_DESCALE(int16_t x){
         return arithmeticRightShiftN16(x + (1U << (PJPG_DCT_SCALE_BITS - 1)), PJPG_DCT_SCALE_BITS);
     }
};


//-----------------------------------------------------------------------------------------------------------------------

//Calibration
//x,y | Ux,Uy  0  ,0     | 1913,1940
//x,y | Ux,Uy  240,0     |  150,1940
//x,y | Ux,Uy  0  ,320   | 1913, 220
//x,y | Ux,Uy  240,320   |  150, 220
// the outcome of this is   x: (1913-150)/240 = 7,3458mV pixel
//                          y: (1944-220)/320 = 5,3875mV pixel

class TP : public TFT {
    public:

        TP(uint8_t _TP_CS, uint8_t _TP_IRQ);
        void loop();
        void setVersion(uint8_t v);
        void setRotation(uint8_t m);
        void setMirror(bool h, bool v);
    private:
        SPISettings TP_SPI;
        uint8_t     _TP_CS, _TP_IRQ;
        uint16_t    x=0, y=0;
        uint8_t     _rotation;
        bool        f_loop=false;
        bool        m_mirror_h = false;
        bool        m_mirror_v = false;

        //const uint8_t TP_Dummy=0x80; //nur Startbit für XPT2046
        float xFaktor;
        float yFaktor;

        uint16_t Xmax=1913; // default, will be overwritten in constructor
        uint16_t Xmin=150;
        uint16_t Ymax=1944;
        uint16_t Ymin=220;
        uint8_t  TP_vers = 0;

        uint32_t m_pressingTime = 0;
        bool     m_f_isPressing = false;
        bool     m_f_longPressed = false;

        enum tpVers{TP_ILI9341_0 = 0, TP_ILI9341_1 = 1, TP_HX8347D_0 = 2, TP_ILI9486_0 = 3, TP_ILI9488_0 = 4, TP_ST7796_0 = 5};

    public:
        uint16_t TP_Send(uint8_t set_val);
        bool read_TP(uint16_t& x, uint16_t& y);
};


