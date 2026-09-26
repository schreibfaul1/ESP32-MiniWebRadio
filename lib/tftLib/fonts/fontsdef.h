#pragma once

#include <stdbool.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>

#define LV_ATTRIBUTE_LARGE_CONST

typedef struct _glyph_dsc { /** This describes a glyph.*/
    uint32_t bitmap_index;  /**< Start index of the bitmap. A font can be max 1 MB.*/
    uint16_t adv_w;         /**< Draw the next glyph after this width. 8.4 format (real_value * 16 is stored).*/
    uint16_t box_w;         /**< Width of the glyph's bounding box*/
    uint16_t box_h;         /**< Height of the glyph's bounding box*/
    int8_t   ofs_x;         /**< x offset of the bounding box*/
    int8_t   ofs_y;         /**< y offset of the bounding box. Measured from the top of the line*/
} lv_font_fmt_txt_glyph_dsc_t;

typedef struct {
    uint32_t        range_start;       /** First Unicode character for this range*/
    uint16_t        range_length;      /** Number of Unicode characters related to this range.* Last Unicode character = range_start + range_length - 1*/
    uint16_t        glyph_id_start;    /** First glyph ID (array index of `glyph_dsc`) for this range*/
    const uint16_t* unicode_list;      /**/
    const void*     glyph_id_ofs_list; /** if LV_FONT_FMT_TXT_CMAP_FORMAT0_ it's `uint8_t *`, if LV_FONT_FMT_TXT_CMAP_SPARSE_ it's `uint16_t *` */
    uint16_t        list_length;       /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t        type;              /** Type of this character map*/
} lv_font_fmt_txt_cmap_t;

/** Format of font character map.*/
enum _lv_font_fmt_txt_cmap_type_t {
    LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    LV_FONT_FMT_TXT_CMAP_SPARSE_FULL,
    LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    LV_FONT_FMT_TXT_CMAP_SPARSE_TINY,
};

typedef struct {
    const uint8_t* glyph_bitmap; // The bitmaps of all glyphs or a lv_font_fmt_txt_glyph_loader_t * depending on the state of are_glyphs_dynamic_loaded a uint8_t to preserve backwards compatibility
    const lv_font_fmt_txt_glyph_dsc_t* glyph_dsc;     // Describe the glyphs
    const lv_font_fmt_txt_cmap_t*      cmaps;         // Map the glyphs to Unicode characters. Array of lv_font_cmap_fmt_txt_t variables
    const void*                        kern_dsc;      // Store kerning values. Can be lv_font_fmt_txt_kern_pair_t * or lv_font_fmt_txt_kern_classes_t * depending on kern_classes
    uint16_t                           kern_scale;    // Scale kern values in 12.4 format
    uint16_t                           cmap_num;      // Number of cmap tables
    uint16_t                           bpp;           // 	Bit per pixel: 1, 2, 3, 4, 8
    uint16_t                           kern_classes;  // Type of kern_dsc
    uint16_t                           bitmap_format; // storage format of the bitmap from lv_font_fmt_txt_bitmap_format_t
} lv_font_fmt_txt_dsc_t;

typedef struct {
    const void*   glyph_ids;
    const int8_t* values;
    uint32_t      pair_cnt;
    uint32_t      glyph_ids_size;
} lv_font_fmt_txt_kern_pair_t;

typedef struct {
    const char* name;
    int32_t     line_height;
    int32_t     base_line;
    uint8_t     glyph_bitmap;
    const void* dsc;
} lv_font_t;

struct FontInfo {
    const char*                        name;
    const lv_font_t*                   font;
    int32_t                            line_height;
    const uint8_t*                     glyph_bitmap;
    const lv_font_fmt_txt_glyph_dsc_t* glyph_dsc;
    const lv_font_fmt_txt_cmap_t*      cmaps;
    uint16_t                           cmap_num;
};

const uint8_t MAX_FONTS = 20;

void registerFont(const lv_font_t* font);

#define REGISTER_FONT(font)                      \
    static const bool registered_##font = []() { \
        registerFont(&font);                     \
        return true;                             \
    }();

void registerFont(const lv_font_t* font);

void listFonts();