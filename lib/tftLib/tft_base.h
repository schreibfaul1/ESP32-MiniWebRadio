#pragma once

#include "Arduino.h"
#include "FS.h"
#include <vector>
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
    bool drawBmpFile(fs::FS& fs, const char* path, uint16_t x = 0, uint16_t y = 0, uint16_t maxWidth = 0, uint16_t maxHeight = 0, float scale = 1.0f);
    bool drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat);
    bool drawJpgFile(fs::FS& fs, const char* path, uint16_t x = 0, uint16_t y = 0, uint16_t maxWidth = 0, uint16_t maxHeight = 0);
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
    int32_t GIF_readGifItems();
    bool    GIF_decodeGif(uint16_t x, uint16_t y);
    bool    GIF_loop();
    void    GIF_freeMemory();
    void    GIF_DecoderReset();
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
    File      gif_file;
    struct gif_t {
        bool     decodeSdFile_firstread = false;
        bool     GlobalColorTableFlag = false;
        bool     LocalColorTableFlag = false;
        bool     SortFlag = false;
        bool     TransparentColorFlag = false;
        bool     UserInputFlag = false;
        bool     ZeroDataBlock = 0;
        bool     InterlaceFlag = false;
        bool     drawNextImage = false;
        uint8_t  BackgroundColorIndex = 0;
        uint8_t  BlockTerninator = 0;
        uint8_t  CharacterCellWidth = 0;
        uint8_t  CharacterCellHeight = 0;
        uint8_t  CodeSize = 0;
        uint8_t  ColorResulution = 0;
        uint8_t  DisposalMethod = 0;
        uint8_t  ImageSeparator = 0;
        uint8_t  lenDatablock = 0;
        uint8_t  LZWMinimumCodeSize = 0;
        uint8_t  PackedFields = 0;
        uint8_t  PixelAspectRatio = 0;
        uint8_t  TextBackgroundColorIndex = 0;
        uint8_t  TextForegroundColorIndex = 0;
        uint8_t  TransparentColorIndex = 0;
        uint16_t ClearCode = 0;
        uint16_t DelayTime = 0;
        uint16_t EOIcode = 0;
        uint16_t ImageHeight = 0;
        uint16_t ImageWidth = 0;
        uint16_t ImageLeftPosition = 0;
        uint16_t ImageTopPosition = 0;
        uint16_t LogicalScreenWidth = 0;
        uint16_t LogicalScreenHeight = 0;
        uint16_t MaxCode = 0;
        uint16_t MaxCodeSize = 0;
        uint16_t SizeOfGlobalColorTable = 0;
        uint16_t SizeOfLocalColorTable = 0;
        uint16_t TextGridLeftPosition = 0;
        uint16_t TextGridTopPosition = 0;
        uint16_t TextGridWidth = 0;
        uint16_t TextGridHeight = 0;
        uint32_t TimeStamp = 0;
        uint32_t Iterations = 0;
    } gif;
    std::vector<unsigned short> gif_next;
    std::vector<uint8_t>        gif_vals;
    std::vector<uint8_t>        gif_stack;
    std::vector<uint16_t>       gif_GlobalColorTable;
    std::vector<uint16_t>       gif_LocalColorTable;
    const uint8_t gif_MaxLzwBits = 12;
    uint16_t*     gif_ImageBuffer = nullptr;
    uint16_t*     gif_RestoreBuffer = nullptr;
    char          gif_buffer[15];
    char          gif_DSBbuffer[256];
    String        gif_GifHeader = "";
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
    uint16_t*      m_jpegPixelBuffer = nullptr;

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
  protected:
    const uint16_t m_rowBufferSize = 4096;
    uint8_t*       m_rowBuffer = nullptr;
};
