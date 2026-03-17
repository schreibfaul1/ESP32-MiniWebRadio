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
bool TFT_Base::drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat) {

    gif.Iterations = repeat;

    GIF_DecoderReset();

    gif_file = fs.open(path);
    if (!gif_file) {
        if (tft_info) tft_info("Failed to open file for reading");
        return false;
    }
    GIF_readGifItems();
    // check it's a gif
    if (!gif_GifHeader.startsWith("GIF")) {
        if (tft_info) tft_info("File is not a gif");
        return false;
    }
    // check dimensions
    // { log_w("Width: %i, Height: %i,", gif.LogicalScreenWidth, gif.LogicalScreenHeight); }
    if (gif.LogicalScreenWidth * gif.LogicalScreenHeight > 155000) {
        if (tft_info) tft_info("!Image is too big!!");
        return false;
    }

    if (psramFound()) {
        gif_ImageBuffer = (uint16_t*)ps_malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));
    } else {
        gif_ImageBuffer = (uint16_t*)malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));
    }

    if (psramFound()) {
        gif_RestoreBuffer = (uint16_t*)ps_malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));
    } else {
        gif_RestoreBuffer = (uint16_t*)malloc(gif.LogicalScreenWidth * gif.LogicalScreenHeight * sizeof(uint16_t));
    }

    if (!gif_ImageBuffer) {
        if (tft_info) tft_info("!Not enough memory!!");
        return false;
    }
    if (!gif_RestoreBuffer) {
        if (tft_info) tft_info("!Not enough memory!!");
        return false;
    }

    if (GIF_decodeGif(x, y) == false) {
        GIF_freeMemory();
        gif_file.close();
        gif.drawNextImage = false;
        log_w("GIF file closed");
        return true;
    } else {
        gif.drawNextImage = true;
    }
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT_Base::GIF_loop() {
    if (!gif.drawNextImage) return false;
    if (gif.TimeStamp > millis()) return false;
    if (!GIF_decodeGif(100, 100)) {
        GIF_freeMemory();
        gif_file.close();
        gif.drawNextImage = false;
        log_w("GIF file closed");
        return false;
    }
    return true;
}
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT_Base::GIF_readHeader() {

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

void TFT_Base::GIF_readLogicalScreenDescriptor() {

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

void TFT_Base::GIF_readImageDescriptor() {

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

void TFT_Base::GIF_readLocalColorTable() {
    // The size of the local color table can be calculated by the value given in the image descriptor.
    // Just like with the global color table, if the image descriptor specifies a size of N,
    // the color table will contain 2^(N+1) colors and will take up 3*2^(N+1) bytes.
    // The colors are specified in RGB value triplets.
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    gif_LocalColorTable.reserve(gif.SizeOfLocalColorTable);
    if (gif.LocalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while (i != gif.SizeOfLocalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill LocalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_LocalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        // for(i=0;i<SizeOfLocalColorTable; i++) log_i("LocalColorTable %i= %i", i, LocalColorTable[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT_Base::GIF_readGlobalColorTable() {
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
    if (gif.GlobalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while (i != gif.SizeOfGlobalColorTable) {
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

void TFT_Base::GIF_readGraphicControlExtension() {

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
       User Input Flag - Indicates whether or not user input is expected before continuing. If the flag is set, processing will continue when user input is entered. The nature of the User
       input is determined by the application (Carriage Return, Mouse Button Click, etc.). 0 -   User input is not expected. 1 -   User input is expected. Transparency Flag - Indicates whether
       a transparency index is given in the Transparent Index field. (This field is the least significant bit of the byte.) 0 -   Transparent Index is not given. 1 -   Transparent Index is
       given.
    */

    uint8_t BlockSize = 0;
    gif_file.readBytes(gif_buffer, 1);
    BlockSize = gif_buffer[0]; // Number of bytes in the block, not including the Block Terminator

    if (BlockSize == 0) return;
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

uint8_t TFT_Base::GIF_readPlainTextExtension(char* buf) {

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
    if (BlockSize > 0) {
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

uint8_t TFT_Base::GIF_readApplicationExtension(char* buf) {

    //      7 6 5 4 3 2 1 0        Field Name                    Type
    //     +---------------+
    // 0   |               |       Block Size                    Byte
    //     +---------------+
    // 1   |               |
    //     +-             -+
    // 2   |               |
    //     +-             -+
    // 3   |               |       Application Identifier        8 Bytes
    //     +-             -+
    // 4   |               |
    //     +-             -+
    // 5   |               |
    //     +-             -+
    // 6   |               |
    //     +-             -+
    // 7   |               |
    //     +-             -+
    // 8   |               |
    //     +---------------+
    // 9   |               |
    //     +-             -+
    // 10  |               |       Appl. Authentication Code     3 Bytes
    //     +-             -+
    // 11  |               |
    //     +---------------+
    //
    //     +===============+
    //     |               |
    //     |               |       Application Data              Data Sub-blocks
    //     |               |
    //     |               |
    //     +===============+
    //
    //     +---------------+
    // 0   |               |       Block Terminator              Byte
    //     +---------------+

    uint8_t BlockSize = 0, numBytes = 0;
    BlockSize = gif_file.read();
    if (BlockSize > 0) { gif_file.readBytes(gif_buffer, BlockSize); }
    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_Base::GIF_readCommentExtension(char* buf) {

    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //   +===============+
    //   |               |
    // N |               |       Comment Data                  Data Sub-blocks
    //   |               |
    //   +===============+
    //
    //   +---------------+
    // 0 |               |       Block Terminator              Byte
    //   +---------------+

    uint8_t numBytes = 0;
    numBytes = GIF_readDataSubBlock(buf);
    // sprintf(chbuf, "GIF: Comment %s", buf);
    //  if(tft_info) tft_info(chbuf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT_Base::GIF_readDataSubBlock(char* buf) {

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
    if (BlockSize > 0) {
        gif.ZeroDataBlock = false;
        gif_file.readBytes(buf, BlockSize);
    } else
        gif.ZeroDataBlock = true;
    return BlockSize;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_Base::GIF_readExtension(char Label) {
    char buf[256];
    switch (Label) {
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

int32_t TFT_Base::GIF_GetCode(int32_t code_size, int32_t flag) {
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

    if (flag) {
        curbit = 0;
        lastbit = 0;
        done = false;
        return 0;
    }

    if ((curbit + code_size) >= lastbit) {
        if (done) {
            // log_i("done");
            if (curbit >= lastbit) { return 0; }
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
        if (count == 0) done = true;

        last_byte = 2 + count;

        curbit = (curbit - lastbit) + 16;

        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j) ret |= ((DSBbuffer[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_Base::GIF_LZWReadByte(bool init) {
    static int32_t fresh = false;
    int32_t        code, incode;
    static int32_t firstcode, oldcode;

    if (gif_next.capacity() < (1 << gif_MaxLzwBits)) gif_next.reserve((1 << gif_MaxLzwBits) - gif_next.capacity());
    if (gif_vals.capacity() < (1 << gif_MaxLzwBits)) gif_vals.reserve((1 << gif_MaxLzwBits) - gif_vals.capacity());
    if (gif_stack.capacity() < (1 << (gif_MaxLzwBits + 1))) gif_stack.reserve((1 << (gif_MaxLzwBits + 1)) - gif_stack.capacity());
    gif_next.clear();
    gif_vals.clear();
    gif_stack.clear();

    static uint8_t* sp;

    int32_t i;

    if (init) {
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

        for (i = 0; i < gif.ClearCode; i++) {
            gif_next[i] = 0;
            gif_vals[i] = i;
        }
        for (; i < (1 << gif_MaxLzwBits); i++) gif_next[i] = gif_vals[0] = 0;

        sp = &gif_stack[0];

        return 0;
    } else if (fresh) {
        fresh = false;
        do { firstcode = oldcode = GIF_GetCode(gif.CodeSize, false); } while (firstcode == gif.ClearCode);

        return firstcode;
    }

    if (sp > &gif_stack[0]) return *--sp;

    while ((code = GIF_GetCode(gif.CodeSize, false)) >= 0) {
        if (code == gif.ClearCode) {
            for (i = 0; i < gif.ClearCode; ++i) {
                gif_next[i] = 0;
                gif_vals[i] = i;
            }
            for (; i < (1 << gif_MaxLzwBits); ++i) gif_next[i] = gif_vals[i] = 0;

            gif.CodeSize = gif.LZWMinimumCodeSize + 1;
            gif.MaxCodeSize = 2 * gif.ClearCode;
            gif.MaxCode = gif.ClearCode + 2;
            sp = &gif_stack[0];

            firstcode = oldcode = GIF_GetCode(gif.CodeSize, false);
            return firstcode;
        } else if (code == gif.EOIcode) {
            int32_t count;
            char    buf[260];

            if (gif.ZeroDataBlock) return -2;
            while ((count = GIF_readDataSubBlock(buf)) > 0);

            if (count != 0) return -2;
        }

        incode = code;

        if (code >= gif.MaxCode) {
            *sp++ = firstcode;
            code = oldcode;
        }

        while (code >= gif.ClearCode) {
            *sp++ = gif_vals[code];
            if (code == (int32_t)gif_next[code]) { return -1; }
            code = gif_next[code];
        }
        *sp++ = firstcode = gif_vals[code];

        if ((code = gif.MaxCode) < (1 << gif_MaxLzwBits)) {
            gif_next[code] = oldcode;
            gif_vals[code] = firstcode;
            ++gif.MaxCode;
            if ((gif.MaxCode >= gif.MaxCodeSize) && (gif.MaxCodeSize < (1 << gif_MaxLzwBits))) {
                gif.MaxCodeSize *= 2;
                ++gif.CodeSize;
            }
        }
        oldcode = incode;

        if (sp > &gif_stack[0]) return *--sp;
    }
    return code;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TFT_Base::GIF_ReadImage(uint16_t x, uint16_t y) {
    int32_t         color;
    int32_t         xpos = x + gif.ImageLeftPosition;
    int32_t         ypos = y + gif.ImageTopPosition;
    int32_t         max = gif.ImageHeight * gif.ImageWidth;
    uint32_t        i = 0;
    static uint8_t  gif_LastDisposalMethod = 0;
    static uint16_t gif_LastImageWidth = 0;
    static uint16_t gif_LastImageHeight = 0;
    static uint16_t gif_LastImageLeftPosition = 0;
    static uint16_t gif_LastImageTopPosition = 0;

    gif.LZWMinimumCodeSize = gif_file.read();
    if (GIF_LZWReadByte(true) < 0) return false;

    if (gif.DisposalMethod < 2) {

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

                if (buf_x < gif.LogicalScreenWidth && buf_y < gif.LogicalScreenHeight) { gif_ImageBuffer[buf_y * gif.LogicalScreenWidth + buf_x] = gif_LocalColorTable[gif.BackgroundColorIndex]; }
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

    // --- Build contiguous RGB565 block for this frame ---

    const uint16_t frameW = gif.ImageWidth;
    const uint16_t frameH = gif.ImageHeight;

    uint16_t* pixelBuffer = (uint16_t*)ps_malloc(frameW * frameH * sizeof(uint16_t));

    if (!pixelBuffer) return false;

    for (uint16_t row = 0; row < frameH; row++) {
        for (uint16_t col = 0; col < frameW; col++) {

            uint16_t buf_x = gif.ImageLeftPosition + col;
            uint16_t buf_y = gif.ImageTopPosition + row;

            if (buf_x >= gif.LogicalScreenWidth || buf_y >= gif.LogicalScreenHeight) {
                pixelBuffer[row * frameW + col] = 0x0000;
                continue;
            }

            pixelBuffer[row * frameW + col] = gif_ImageBuffer[buf_y * gif.LogicalScreenWidth + buf_x];
        }
    }

    // --- Unified render path ---
    renderRGB565(xpos, ypos, frameW, frameH, pixelBuffer, NULL);

    free(pixelBuffer);

    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT_Base::GIF_readGifItems() {
    GIF_readHeader();
    GIF_readLogicalScreenDescriptor();
    gif.decodeSdFile_firstread = true;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT_Base::GIF_decodeGif(uint16_t x, uint16_t y) {
    char           c = 0;
    static int32_t test = 1;
    char           Label = 0;
    if (gif.decodeSdFile_firstread == true) GIF_readGlobalColorTable(); // If exists
    gif.decodeSdFile_firstread = false;

    while (c != ';') { // Trailer found
        c = gif_file.read();
        if (c == '!') {              // it is a Extension
            Label = gif_file.read(); // Label
            GIF_readExtension(Label);
        }
        if (c == ',') {
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

void TFT_Base::GIF_freeMemory() {
    gif_next.clear();
    gif_next.shrink_to_fit();
    gif_vals.clear();
    gif_vals.shrink_to_fit();
    gif_stack.clear();
    gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();
    gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    if (gif_ImageBuffer) {
        free(gif_ImageBuffer);
        gif_ImageBuffer = NULL;
    }
    if (gif_RestoreBuffer) {
        free(gif_RestoreBuffer);
        gif_RestoreBuffer = NULL;
    }
}

void TFT_Base::GIF_DecoderReset() {
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
// ——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ J P E G ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
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
