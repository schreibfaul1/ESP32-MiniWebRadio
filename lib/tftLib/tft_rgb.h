// first release on 09/2019
// updated on Jan 17 2024


#pragma once

#include "Arduino.h"
#include "FS.h"
#include "SPI.h"
#include "SD.h"
#include "SD_MMC.h"
#include "Wire.h"
#include "vector"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lcd_panel_interface.h"
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

#define ANSI_ESC_RESET          "\033[0m"

#define ANSI_ESC_BLACK          "\033[30m"
#define ANSI_ESC_RED            "\033[31m"
#define ANSI_ESC_GREEN          "\033[32m"
#define ANSI_ESC_YELLOW         "\033[33m"
#define ANSI_ESC_BLUE           "\033[34m"
#define ANSI_ESC_MAGENTA        "\033[35m"
#define ANSI_ESC_CYAN           "\033[36m"
#define ANSI_ESC_WHITE          "\033[37m"

#define ANSI_ESC_GREY           "\033[90m"
#define ANSI_ESC_LIGHTRED       "\033[91m"
#define ANSI_ESC_LIGHTGREEN     "\033[92m"
#define ANSI_ESC_LIGHTYELLOW    "\033[93m"
#define ANSI_ESC_LIGHTBLUE      "\033[94m"
#define ANSI_ESC_LIGHTMAGENTA   "\033[95m"
#define ANSI_ESC_LIGHTCYAN      "\033[96m"
#define ANSI_ESC_LIGHTGREY      "\033[97m"

#define ANSI_ESC_DARKRED        "\033[38;5;52m"
#define ANSI_ESC_DARKGREEN      "\033[38;5;22m"
#define ANSI_ESC_DARKYELLOW     "\033[38;5;136m"
#define ANSI_ESC_DARKBLUE       "\033[38;5;17m"
#define ANSI_ESC_DARKMAGENTA    "\033[38;5;53m"
#define ANSI_ESC_DARKCYAN       "\033[38;5;23m"
#define ANSI_ESC_DARKGREY       "\033[38;5;240m"

#define ANSI_ESC_BROWN          "\033[38;5;130m"
#define ANSI_ESC_ORANGE         "\033[38;5;214m"
#define ANSI_ESC_DARKORANGE     "\033[38;5;166m"
#define ANSI_ESC_LIGHTORANGE    "\033[38;5;215m"
#define ANSI_ESC_PURPLE         "\033[38;5;129m"
#define ANSI_ESC_PINK           "\033[38;5;213m"
#define ANSI_ESC_LIME           "\033[38;5;190m"
#define ANSI_ESC_NAVY           "\033[38;5;25m"
#define ANSI_ESC_AQUAMARINE     "\033[38;5;51m"
#define ANSI_ESC_LAVENDER       "\033[38;5;189m"


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

// RGB565 Color definitions            R    G    B
#define TFT_RED             0xF800 // 255,   0,   0
#define TFT_DARKRED         0x8000 // 128,   0,   0
#define TFT_LIGHTRED        0xFBEF // 255, 127, 127
#define TFT_GREEN           0x07E0 //   0, 255,   0
#define TFT_DARKGREEN       0x0400 //   0, 128,   0
#define TFT_LIGHTGREEN      0x7FE0 // 127, 255, 127
#define TFT_BLUE            0x001F //   0,   0, 255
#define TFT_DARKBLUE        0x0010 //   0,   0, 128
#define TFT_LIGHTBLUE       0x7BFF // 127, 127, 255
#define TFT_CYAN            0x07FF //   0, 255, 255
#define TFT_DARKCYAN        0x0410 //   0, 128, 128
#define TFT_LIGHTCYAN       0x7FFF // 127, 255, 255
#define TFT_MAGENTA         0xF81F // 255,   0, 255
#define TFT_DARKMAGENTA     0x8010 // 128,   0, 128
#define TFT_LIGHTMAGENTA    0xF97F // 255, 127, 255
#define TFT_YELLOW          0xFFE0 // 255, 255,   0
#define TFT_DARKYELLOW      0x8400 // 128, 128,   0
#define TFT_LIGHTYELLOW     0xFFF7 // 255, 255, 127
#define TFT_WHITE           0xFFFF // 255, 255, 255
#define TFT_BLACK           0x0000 //   0,   0,   0
#define TFT_GREY            0x8410 // 128, 128, 128
#define TFT_LIGHTGREY       0xC618 // 192, 192, 192
#define TFT_DARKGREY        0xAD55 //  64,  64,  64
#define TFT_BROWN           0xA145 // 165,  42,  42
#define TFT_DARKBROWN       0x8200 // 128,  64,   0
#define TFT_LIGHTBROWN      0xFDB2 // 254, 198, 125

#define TFT_AQUAMARINE      0x7FFA // 127, 255, 212
#define TFT_BEIGE           0xF7BB // 245, 245, 220
#define TFT_CHOCOLATE       0xD342 // 210, 105,  30
#define TFT_CORNSILK        0xFFDB // 255, 248, 220
#define TFT_DEEPSKYBLUE     0x05FF //   0, 191, 255
#define TFT_GREENYELLOW     0xAFE5 // 173, 255,  47
#define TFT_GOLD            0xFEA0 // 255, 215,   0
#define TFT_HOTPINK         0xFB56 // 255, 105, 180
#define TFT_LAVENDER        0xE73F // 230, 230, 250
#define TFT_LAWNGREEN       0x7FE0 // 124, 252,   0
#define TFT_LIME            0x07E0 //   0. 255,   0
#define TFT_MAROON          0x7800 // 128,   0,   0
#define TFT_MEDIUMVIOLETRED 0xC0B0 // 199,  21, 133
#define TFT_NAVY            0x000F //   0,   0, 128
#define TFT_OLIVE           0x7BE0 // 128, 128,   0
#define TFT_ORANGE          0xFD20 // 255, 165,   0
#define TFT_LIGHTORANGE     0xFDB8 // 255, 200, 124
#define TFT_DARKORANGE      0xFC00 // 255, 140,   0
#define TFT_PINK            0xFE19 // 255, 192, 203
#define TFT_PURPLE          0x780F // 128,   0, 128
#define TFT_SANDYBROWN      0xF52C // 244, 164,  96
#define TFT_TURQUOISE       0x471A //  64, 224, 208
#define TFT_VIOLET          0x801F // 128,   0, 255




#if TFT_FONT == 1
#define TFT_TIMES_NEW_ROMAN
#elif TFT_FONT == 2
#define TFT_FREE_SERIF_ITALIC
#elif TFT_FONT == 3
#define TFT_ARIAL
#elif TFT_FONT == 4
#define TFT_Z300
#else
#define TFT_GARAMOND // if nothing is chosen
#endif

#define TFT_ALIGN_RIGHT          (1)
#define TFT_ALIGN_LEFT           (2)
#define TFT_ALIGN_CENTER         (3)
#define TFT_ALIGN_TOP            (4)
#define TFT_ALIGN_DOWN           (5)

class TFT_RGB {
  public:
    enum Rotate { _0, _90, _180, _270 };
    struct Pins {
        int8_t b0;
        int8_t b1;
        int8_t b2;
        int8_t b3;
        int8_t b4;
        int8_t g0;
        int8_t g1;
        int8_t g2;
        int8_t g3;
        int8_t g4;
        int8_t g5;
        int8_t r0;
        int8_t r1;
        int8_t r2;
        int8_t r3;
        int8_t r4;
        int8_t hsync;
        int8_t vsync;
        int8_t de;
        int8_t pclk;
        int8_t bl;
    };
    struct Timing {
        uint16_t h_res;
        uint16_t v_res;
        uint32_t pixel_clock_hz;
        uint8_t  hsync_pulse_width;
        uint8_t  hsync_back_porch;
        uint8_t  hsync_front_porch;
        uint8_t  vsync_pulse_width;
        uint8_t  vsync_back_porch;
        uint8_t  vsync_front_porch;
    };

    TFT_RGB();
    ~TFT_RGB() { ; }
    void begin(const Pins& newPins, const Timing& newTiming);
    void setDisplayInversion(bool i);
    // Recommended Non-Transaction
    void            drawLine(int16_t Xpos0, int16_t Ypos0, int16_t Xpos1, int16_t Ypos1, uint16_t color);
    void            drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color);
    void            readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data);
    void            fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void            drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void            fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
    void            fillScreen(uint16_t color);
    void            drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void            fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void            drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void            fillCircle(int16_t Xm, int16_t Ym, uint16_t r, uint16_t color);
    bool            drawBmpFile(fs::FS& fs, const char* path, uint16_t x = 0, uint16_t y = 0, uint16_t maxWidth = 0, uint16_t maxHeight = 0, float scale = 1.0);
    bool            drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat);
    bool            drawJpgFile(fs::FS& fs, const char* path, uint16_t x = 0, uint16_t y = 0, uint16_t maxWidth = 0, uint16_t maxHeight = 0);
    inline void     setBackGoundColor(uint16_t BGcolor) { m_backGroundColor = BGcolor; }
    inline uint16_t getBackGroundColor() { return m_backGroundColor; }
    inline void     setTextColor(uint16_t FGcolor) { m_textColor = FGcolor; }
    inline uint16_t getTextColor() { return m_textColor; }
    void            setFont(uint16_t font);
    inline void     setTextOrientation(uint16_t orientation = 0) { m_textorientation = orientation; } // 0 h other v
    size_t          writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align = TFT_ALIGN_LEFT, uint8_t v_align = TFT_ALIGN_CENTER, bool narrow = false,
                       bool noWrap = false, bool autoSize = false);
  private:
    Pins                   m_pins;
    Timing                 m_timing;
    esp_lcd_panel_handle_t m_panel;
    uint16_t               m_h_res = 0;
    uint16_t               m_v_res = 0;
    uint16_t*              m_framebuffer;

  private:
    File gif_file;

    void     writeToFramebuffer(const uint8_t* bmi, uint16_t posX, uint16_t posY, uint16_t width, uint16_t height);
    uint16_t validCharsInString(const char* str, uint16_t* chArr, int8_t* ansiArr);
    uint16_t fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap);
    uint8_t  fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap);

  private:
    uint8_t fontSizes[11] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66};

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
    fonts_t _current_font;
    uint8_t _font;


    uint16_t m_backGroundColor = TFT_WHITE;
    uint16_t m_textColor = TFT_BLACK;
    uint8_t  m_textorientation = 0;

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   J P E G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

    bool debug = false;

    vector<unsigned short> gif_next;
    vector<uint8_t>        gif_vals;
    vector<uint8_t>        gif_stack;
    vector<uint16_t>       gif_GlobalColorTable;
    vector<uint16_t>       gif_LocalColorTable;

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
    uint8_t GIF_readCommentExtension(char* buf);
    uint8_t GIF_readDataSubBlock(char* buf);
    bool    GIF_readExtension(char Label);
    int32_t GIF_GetCode(int32_t code_size, int32_t flag);
    int32_t GIF_LZWReadByte(bool init);
    bool    GIF_ReadImage(uint16_t x, uint16_t y);

    inline int32_t minimum(int32_t a, int32_t b) {
        if (a < b)
            return a;
        else
            return b;
    }

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   J P E G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#define LDB_WORD(ptr) (uint16_t)(((uint16_t)*((uint8_t*)(ptr)) << 8) | (uint16_t)*(uint8_t*)((ptr) + 1))
#define JD_SZBUF      512 /* Specifies size of stream input buffer */
#define JD_FORMAT     1   /* Specifies output pixel format. 0: RGB888 (24-bit/pix) 1: RGB565 (16-bit/pix) 2: Grayscale (8-bit/pix) */
#define JD_USE_SCALE  1   /* Switches output descaling feature. 0: Disable 1: Enable */
#define JD_TBLCLIP    0   /* Use table conversion for saturation arithmetic. A bit faster, but increases 1 KB of code size. 0: Disable 1: Enable */
#define JD_FASTDECODE 1   /* Optimization level  0: Basic optimization. Suitable for 8/16-bit MCUs. Workspace of 3100 bytes needed. */
                          /*                                1: + 32-bit barrel shifter. Suitable for 32-bit MCUs. Workspace of 3480 bytes needed.*/
                          /*                              2: + Table conversion for huffman decoding (wants 6 << HUFF_BIT bytes of RAM). Workspace of 9644 bytes needed. */
// Do not change this, it is the minimum size in bytes of the workspace needed by the decoder
#if JD_FASTDECODE == 0
    #define TJPGD_WORKSPACE_SIZE 3100
#endif
#if JD_FASTDECODE == 1
    #define TJPGD_WORKSPACE_SIZE 3500
#endif
#if JD_FASTDECODE == 2
    #define TJPGD_WORKSPACE_SIZE (3500 + 6144)
#endif

  private:
    enum { TJPG_ARRAY = 0, TJPG_FS_FILE, TJPG_SD_FILE };

    enum JDR {             /* Error code */
               JDR_OK = 0, /* 0: Succeeded */
               JDR_INTR,   /* 1: Interrupted by output function */
               JDR_INP,    /* 2: Device error or wrong termination of input stream */
               JDR_MEM1,   /* 3: Insufficient memory pool for the image */
               JDR_MEM2,   /* 4: Insufficient stream input buffer */
               JDR_PAR,    /* 5: Parameter error */
               JDR_FMT1,   /* 6: Data format error (may be broken data) */
               JDR_FMT2,   /* 7: Right format but not supported */
               JDR_FMT3    /* 8: Not supported JPEG standard */
    };
#if JD_FASTDECODE >= 1
    typedef int16_t jd_yuv_t;
#else
    typedef uint8_t jd_yuv_t;
#endif

    typedef struct {     /* Rectangular region in the output image */
        uint16_t left;   /* Left end */
        uint16_t right;  /* Right end */
        uint16_t top;    /* Top end */
        uint16_t bottom; /* Bottom end */
    } JRECT;

    typedef struct _JDEC {        /* Decompressor object structure */
        size_t    dctr;           /* Number of bytes available in the input buffer */
        uint8_t*  dptr;           /* Current data read ptr */
        uint8_t*  inbuf;          /* Bit stream input buffer */
        uint8_t   dbit;           /* Number of bits availavble in wreg or reading bit mask */
        uint8_t   scale;          /* Output scaling ratio */
        uint8_t   msx, msy;       /* MCU size in unit of block (width, height) */
        uint8_t   qtid[3];        /* Quantization table ID of each component, Y, Cb, Cr */
        uint8_t   ncomp;          /* Number of color components 1:grayscale, 3:color */
        int16_t   dcv[3];         /* Previous DC element of each component */
        uint16_t  nrst;           /* Restart inverval */
        uint16_t  width, height;  /* Size of the input image (pixel) */
        uint8_t*  huffbits[2][2]; /* Huffman bit distribution tables [id][dcac] */
        uint16_t* huffcode[2][2]; /* Huffman code word tables [id][dcac] */
        uint8_t*  huffdata[2][2]; /* Huffman decoded data tables [id][dcac] */
        int32_t*  qttbl[4];       /* Dequantizer tables [id] */
#if JD_FASTDECODE >= 1
        uint32_t wreg;   /* Working shift register */
        uint8_t  marker; /* Detected marker (0:None) */
    #if JD_FASTDECODE == 2
        uint8_t   longofs[2][2]; /* Table offset of long code [id][dcac] */
        uint16_t* hufflut_ac[2]; /* Fast huffman decode tables for AC short code [id] */
        uint8_t*  hufflut_dc[2]; /* Fast huffman decode tables for DC short code [id] */
    #endif
#endif
        void*     workbuf; /* Working buffer for IDCT and RGB output */
        jd_yuv_t* mcubuf;  /* Working buffer for the MCU */
        void*     pool;    /* Pointer to available memory pool */
        size_t    sz_pool; /* Size of momory pool (bytes available) */
        void*     device;  /* Pointer to I/O device identifiler for the session */
        uint8_t   swap;    /* control byte swapping */
    } JDEC;

  private:
    File           m_jpgSdFile;
    bool           m_swap = false;
    const uint8_t* m_array_data = nullptr;
    uint32_t       m_array_index = 0;
    uint32_t       m_array_size = 0;
    uint8_t        m_workspace[TJPGD_WORKSPACE_SIZE] __attribute__((aligned(4))); // Must align workspace to a 32 bit boundary
    uint8_t        m_jpg_source = 0;
    int16_t        m_jpeg_x = 0;
    int16_t        m_jpeg_y = 0;
    uint8_t        m_jpgScale = 0;
    uint16_t       m_jpgWidth = 0;
    uint16_t       m_jpgHeight = 0;
    uint16_t       m_jpgWidthMax = 0;
    uint16_t       m_jpgHeightMax = 0;

  public:
    void    JPEG_setJpgScale(uint8_t scale);
    uint8_t JPEG_drawSdJpg(int32_t x, int32_t y);
    uint8_t JPEG_getSdJpgSize(uint16_t* w, uint16_t* h);
    void    JPEG_setSwapBytes(bool swap);

  private:
    int          JPEG_jd_output(JDEC* jdec, void* bitmap, JRECT* jrect);
    bool         JPEG_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
    unsigned int JPEG_jd_input(JDEC* jdec, uint8_t* buf, unsigned int len);
    void*        JPEG_alloc_pool(JDEC* jd, size_t ndata);
    uint8_t      JPEG_create_qt_tbl(JDEC* jd, const uint8_t* data, size_t ndata);
    uint8_t      JPEG_create_huffman_tbl(JDEC* jd, const uint8_t* data, size_t ndata);
    int          JPEG_huffext(JDEC* jd, unsigned int id, unsigned int cls);
    int          JPEG_bitext(JDEC* jd, unsigned int nbit);
    uint8_t      JPEG_restart(JDEC* jd, uint16_t rstn);
    void         JPEG_block_idct(int32_t* src, jd_yuv_t* dst);
    uint8_t      JPEG_mcu_load(JDEC* jd);
    uint8_t      JPEG_mcu_output(JDEC* jd, unsigned int x, unsigned int y);
    uint8_t      JPEG_jd_prepare(JDEC* jd, uint8_t* pool, size_t sz_pool, void* dev);
    uint8_t      JPEG_jd_decomp(JDEC* jd, uint8_t scale);

  private:
#if JD_TBLCLIP == 0 /* JD_TBLCLIP */
    uint8_t JPEG_BYTECLIP(int val);
#endif
};
