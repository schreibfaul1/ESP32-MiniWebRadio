#pragma once

#include <stdint.h>
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
    uint32_t        range_start;        /** First Unicode character for this range*/
    uint16_t        range_length;       /** Number of Unicode characters related to this range.* Last Unicode character = range_start + range_length - 1*/
    uint16_t        font_height;
    uint16_t        line_height;        /** The maximum line height required by the font*/
    uint8_t         base_line;          /** Baseline measured from the bottom of the line*/
    uint16_t        glyph_id_start;     /** First glyph ID (array index of `glyph_dsc`) for this range*/
    const uint16_t *unicode_list;
    const void     *glyph_id_ofs_list;  /** if LV_FONT_FMT_TXT_CMAP_FORMAT0_ it's `uint8_t *`, if LV_FONT_FMT_TXT_CMAP_SPARSE_ it's `uint16_t *` */
    uint16_t        list_length;        /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t        type;               /** Type of this character map*/
    uint16_t       *lookup_table;       /** used lookup */
} lv_font_fmt_txt_cmap_t;

/** Format of font character map.*/
enum _lv_font_fmt_txt_cmap_type_t
{
    LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    LV_FONT_FMT_TXT_CMAP_SPARSE_FULL,
    LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    LV_FONT_FMT_TXT_CMAP_SPARSE_TINY,
};