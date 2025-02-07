// first release on 01/2025
// updated on Feb 02 2025


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
#include "fonts/Z003.h"




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
#define TFT_Z003
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
        uint16_t  hsync_front_porch;
        uint8_t  vsync_pulse_width;
        uint8_t  vsync_back_porch;
        uint8_t  vsync_front_porch;
    };

    TFT_RGB();
    ~TFT_RGB() { ; }
    void refresh();
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
    uint16_t*              m_framebuffer[2];
    bool                   m_framebuffer_index = 0;

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

// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   P N G   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

#define MAKE_BYTE(b) ((b) & 0xFF)
#define MAKE_DWORD(a,b,c,d) ((MAKE_BYTE(a) << 24) | (MAKE_BYTE(b) << 16) | (MAKE_BYTE(c) << 8) | MAKE_BYTE(d))
#define MAKE_DWORD_PTR(p) MAKE_DWORD((p)[0], (p)[1], (p)[2], (p)[3])

#define CHUNK_IHDR MAKE_DWORD('I','H','D','R')
#define CHUNK_IDAT MAKE_DWORD('I','D','A','T')
#define CHUNK_IEND MAKE_DWORD('I','E','N','D')

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285

#define NUM_DEFLATE_CODE_SYMBOLS 288    /*256 literals, the end code, some length codes, and 2 unused codes */
#define NUM_DISTANCE_SYMBOLS 32    /*the distance codes have their own symbols, 30 used, 2 unused */
#define NUM_CODE_LENGTH_CODES 19    /*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros */
#define MAX_SYMBOLS 288 /* largest number of symbols used by any tree type */

#define DEFLATE_CODE_BITLEN 15
#define DISTANCE_BITLEN 15
#define CODE_LENGTH_BITLEN 7
#define MAX_BIT_LENGTH 15 /* largest bitlen used by any tree type */

#define DEFLATE_CODE_BUFFER_SIZE (NUM_DEFLATE_CODE_SYMBOLS * 2)
#define DISTANCE_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)
#define CODE_LENGTH_BUFFER_SIZE (NUM_DISTANCE_SYMBOLS * 2)

// #define SET_ERROR(upng,code) do { (upng)->error = (code); (upng)->error_line = __LINE__; } while (0)

#define upng_chunk_length(chunk) MAKE_DWORD_PTR(chunk)
#define upng_chunk_type(chunk) MAKE_DWORD_PTR((chunk) + 4)
#define upng_chunk_critical(chunk) (((chunk)[4] & 32) == 0)

  private:
    char*    png_buffer = NULL; // contains the input data
    char*    png_outbuffer = NULL;
    uint32_t png_outbuff_size = 0;
    File     png_file;
    int32_t  png_size = 0;
    int32_t  out_size = 0;
    uint16_t png_width = 0;
    uint16_t png_height = 0;
    uint8_t  png_color_depth = 0;
    uint8_t  png_color_type = 0;
    //uint8_t  png_format = 0;
    int8_t   png_error = 0;
    uint8_t  png_state = 0;
    uint16_t png_pos_x = 0;
    uint16_t png_pos_y = 0;
    uint16_t png_max_width = 0;
    uint16_t png_max_height = 0;


    const uint16_t LENGTH_BASE[29] = {/*the base lengths represented by codes 257-285 */
                                      3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
                                      31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};

    const uint8_t LENGTH_EXTRA[29] = {/*the extra bits used by codes 257-285 (added to base length) */
                                       0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                       2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};

    const uint16_t DISTANCE_BASE[30] = {/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree) */
                                        1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
                                        33,   49,   65,   97,   129,  193,  257,  385,   513,   769,
                                        1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

    const uint8_t DISTANCE_EXTRA[30] = {/*the extra bits of backwards distances (added to base) */
                                         0, 0, 0, 0, 1, 1, 2, 2,  3,  3,  4,  4,  5,  5,  6,
                                         6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    const uint8_t CLCL[NUM_CODE_LENGTH_CODES] /*the order in which "code length alphabet code lengths" are stored, out of
                                                  this the huffman tree of the dynamic huffman tree lengths is generated */
        = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    const uint16_t FIXED_DEFLATE_CODE_TREE[NUM_DEFLATE_CODE_SYMBOLS * 2] = {
        289, 370, 290, 307, 546, 291, 561, 292, 293, 300, 294, 297, 295, 296, 0,   1,   2,   3,   298, 299, 4,   5,   6,
        7,   301, 304, 302, 303, 8,   9,   10,  11,  305, 306, 12,  13,  14,  15,  308, 339, 309, 324, 310, 317, 311, 314,
        312, 313, 16,  17,  18,  19,  315, 316, 20,  21,  22,  23,  318, 321, 319, 320, 24,  25,  26,  27,  322, 323, 28,
        29,  30,  31,  325, 332, 326, 329, 327, 328, 32,  33,  34,  35,  330, 331, 36,  37,  38,  39,  333, 336, 334, 335,
        40,  41,  42,  43,  337, 338, 44,  45,  46,  47,  340, 355, 341, 348, 342, 345, 343, 344, 48,  49,  50,  51,  346,
        347, 52,  53,  54,  55,  349, 352, 350, 351, 56,  57,  58,  59,  353, 354, 60,  61,  62,  63,  356, 363, 357, 360,
        358, 359, 64,  65,  66,  67,  361, 362, 68,  69,  70,  71,  364, 367, 365, 366, 72,  73,  74,  75,  368, 369, 76,
        77,  78,  79,  371, 434, 372, 403, 373, 388, 374, 381, 375, 378, 376, 377, 80,  81,  82,  83,  379, 380, 84,  85,
        86,  87,  382, 385, 383, 384, 88,  89,  90,  91,  386, 387, 92,  93,  94,  95,  389, 396, 390, 393, 391, 392, 96,
        97,  98,  99,  394, 395, 100, 101, 102, 103, 397, 400, 398, 399, 104, 105, 106, 107, 401, 402, 108, 109, 110, 111,
        404, 419, 405, 412, 406, 409, 407, 408, 112, 113, 114, 115, 410, 411, 116, 117, 118, 119, 413, 416, 414, 415, 120,
        121, 122, 123, 417, 418, 124, 125, 126, 127, 420, 427, 421, 424, 422, 423, 128, 129, 130, 131, 425, 426, 132, 133,
        134, 135, 428, 431, 429, 430, 136, 137, 138, 139, 432, 433, 140, 141, 142, 143, 435, 483, 436, 452, 568, 437, 438,
        445, 439, 442, 440, 441, 144, 145, 146, 147, 443, 444, 148, 149, 150, 151, 446, 449, 447, 448, 152, 153, 154, 155,
        450, 451, 156, 157, 158, 159, 453, 468, 454, 461, 455, 458, 456, 457, 160, 161, 162, 163, 459, 460, 164, 165, 166,
        167, 462, 465, 463, 464, 168, 169, 170, 171, 466, 467, 172, 173, 174, 175, 469, 476, 470, 473, 471, 472, 176, 177,
        178, 179, 474, 475, 180, 181, 182, 183, 477, 480, 478, 479, 184, 185, 186, 187, 481, 482, 188, 189, 190, 191, 484,
        515, 485, 500, 486, 493, 487, 490, 488, 489, 192, 193, 194, 195, 491, 492, 196, 197, 198, 199, 494, 497, 495, 496,
        200, 201, 202, 203, 498, 499, 204, 205, 206, 207, 501, 508, 502, 505, 503, 504, 208, 209, 210, 211, 506, 507, 212,
        213, 214, 215, 509, 512, 510, 511, 216, 217, 218, 219, 513, 514, 220, 221, 222, 223, 516, 531, 517, 524, 518, 521,
        519, 520, 224, 225, 226, 227, 522, 523, 228, 229, 230, 231, 525, 528, 526, 527, 232, 233, 234, 235, 529, 530, 236,
        237, 238, 239, 532, 539, 533, 536, 534, 535, 240, 241, 242, 243, 537, 538, 244, 245, 246, 247, 540, 543, 541, 542,
        248, 249, 250, 251, 544, 545, 252, 253, 254, 255, 547, 554, 548, 551, 549, 550, 256, 257, 258, 259, 552, 553, 260,
        261, 262, 263, 555, 558, 556, 557, 264, 265, 266, 267, 559, 560, 268, 269, 270, 271, 562, 565, 563, 564, 272, 273,
        274, 275, 566, 567, 276, 277, 278, 279, 569, 572, 570, 571, 280, 281, 282, 283, 573, 574, 284, 285, 286, 287, 0,
        0};

    const uint8_t FIXED_DISTANCE_TREE[NUM_DISTANCE_SYMBOLS * 2] = {
        33, 48, 34, 41, 35, 38, 36, 37, 0,  1,  2,  3,  39, 40, 4,  5,  6,  7,  42, 45, 43, 44,
        8,  9,  10, 11, 46, 47, 12, 13, 14, 15, 49, 56, 50, 53, 51, 52, 16, 17, 18, 19, 54, 55,
        20, 21, 22, 23, 57, 60, 58, 59, 24, 25, 26, 27, 61, 62, 28, 29, 30, 31, 0,  0};

  private:
    struct png_s_rgb16b{
        int r:5;
        int g:6;
        int b:5;
        int null : 16;
    };

    struct png_s_rgb18b{
        int r:6;
        int g:6;
        int b:6;
        int null:14;
    };

    struct png_s_rgb24b{
        int r:8;
        int g:8;
        int b:8;
        int null:8;
    };

    struct png_s_rgba32b{
        png_s_rgb24b rgb;
        uint8_t alpha;
    };

    typedef enum _png_color_type {
        UPNG_CT_PALETTE  = 1,
        UPNG_CT_COLOR    = 2,
        UPNG_CT_ALPHA    = 4
    } png_color_t;

	enum _error	{
		PNG_EOK = 0,           /* success (no error) */
		PNG_ENOMEM = 1,        /* memory allocation failed */
		PNG_ENOTFOUND = 2,     /* resource not found (file missing) */
		PNG_ENOTPNG = 3,       /* image data does not have a PNG header */
		PNG_EMALFORMED = 4,    /* image data is not a valid PNG image */
		PNG_EUNSUPPORTED = 5,  /* critical PNG chunk type is not supported */
		PNG_EUNINTERLACED = 6, /* image interlacing is not supported */
		PNG_EUNFORMAT = 7,     /* image color format is not supported */
		PNG_EPARAM = 8         /* invalid parameter to method call */
	};

	typedef enum _png_format {
        PNG_BADFORMAT,
        PNG_RGB8,
        PNG_RGB16,
        PNG_RGBA8,
        PNG_RGBA16,
        PNG_LUMINANCE1,
        PNG_LUMINANCE2,
        PNG_LUMINANCE4,
        PNG_LUMINANCE8,
        PNG_LUMINANCE_ALPHA1,
        PNG_LUMINANCE_ALPHA2,
        PNG_LUMINANCE_ALPHA4,
        PNG_LUMINANCE_ALPHA8,
        PNG_PALLETTE1,
        PNG_PALLETTE2,
        PNG_PALLETTE4,
        PNG_PALLETTE8
    }png_format_t;
    png_format_t png_format;


    enum _state {
        PNG_ERROR        = -1,
        PNG_DECODED    = 0,
        PNG_HEADER        = 1,
        PNG_NEW        = 2
    };

    enum _color {
        PNG_LUM        = 0,
        PNG_RGB        = 2,
        PNG_PAL        = 3,
        PNG_LUMA        = 4,
        PNG_RGBA        = 6
    };

    typedef struct png_source {
        uint32_t   size;
        char       owning;
    } png_source;

    typedef struct huffman_tree {
        uint16_t* tree2d;
        uint16_t maxbitlen;    /*maximum number of bits a single code can get */
        uint16_t numcodes;    /*number of symbols in the alphabet = number of codes */
    } huffman_tree;

  public:
    bool          drawPngFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth = 0, uint16_t maxHeight = 0);
    int8_t        png_get_error();
    uint16_t      png_get_width();
    uint16_t      png_get_height();
    uint16_t      png_get_bpp();
    const char*   png_get_outbuffer();
    uint32_t      png_get_size();
    uint16_t      png_get_components();
    uint16_t      png_get_bitdepth();
    uint16_t      png_get_pixelsize();
    png_format_t  png_get_format();

  private:
    char          read_bit(uint32_t *bitpointer, const char* bitstream);
    uint16_t      read_bits(uint32_t* bitpointer, const char* bitstream, uint32_t nbits);
    void          huffman_tree_init(huffman_tree* tree, uint16_t* buffer, uint16_t numcodes, uint16_t maxbitlen);
    void          huffman_tree_create_lengths(huffman_tree* tree, const uint16_t* bitlen);
    uint16_t      huffman_decode_symbol(const char* in, uint32_t  * bp, const huffman_tree* codetree, uint32_t inlength);
    void          get_tree_inflate_dynamic(huffman_tree* codetree, huffman_tree* codetreeD, huffman_tree* codelengthcodetree, const char* in, uint32_t *bp, uint32_t inlength);
    void          inflate_huffman(char* out, uint32_t   outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength, uint16_t btype);
    void          inflate_uncompressed(char* out, uint32_t outsize, const char* in, uint32_t *bp, uint32_t *pos, uint32_t inlength);
    int8_t        uz_inflate_data(char* out, uint32_t outsize, const char* in, uint32_t insize, uint32_t inpos);
    int8_t        uz_inflate(char* out, uint32_t outsize, const char* in, uint32_t insize);
    int           paeth_predictor(int a, int b, int c);
    void          unfilter_scanline(char* recon, const char* scanline, const char* precon, uint32_t bytewidth, unsigned char filterType, uint32_t length);
    void          unfilter(char* out, const char* in, unsigned w, unsigned h, unsigned bpp);
    void          remove_padding_bits(char* out, const char* in, uint32_t olinebits, uint32_t ilinebits, unsigned h);
    void          post_process_scanlines(char* out, char* in);
    int8_t        png_decode();
    png_format_t  png_determine_format();
    bool          png_read_header();
    void          png_GetPixel(void* pixel, int x, int y);
    png_s_rgb16b* InitColorR5G6B5();
    png_s_rgb18b* InitColorR6G6B6();
    png_s_rgb24b* InitColorR8G8B8();
	void          InitColor( png_s_rgb16b **dst);
	void          InitColor( png_s_rgb18b **dst);
	void          InitColor( png_s_rgb24b **dst);
	void          ResetColor(png_s_rgb16b *dst);
	void          ResetColor(png_s_rgb18b *dst);
	void          ResetColor(png_s_rgb24b *dst);
    void          png_rgb24bto18b(png_s_rgb18b* dst, png_s_rgb24b* src);
    void          png_rgb24bto16b(png_s_rgb16b* dst, png_s_rgb24b* src);
    void          png_rgb18btouint32(uint32_t* dst, png_s_rgb18b* src);
    void          png_rgb16btouint32(uint32_t* dst, png_s_rgb16b* src);
    void          draw_into_Framebuffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char* rgbaBuffer, uint32_t png_outbuff_size, uint8_t png_format);


};