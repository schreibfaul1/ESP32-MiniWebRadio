
#include "fontsdef.h"

FontInfo fontList[MAX_FONTS];
uint8_t  fontCount = 0;

void registerFont(const lv_font_t* font) {
    if (fontCount >= MAX_FONTS) { return; }

    const auto* dsc = static_cast<const lv_font_fmt_txt_dsc_t*>(font->dsc);

    uint8_t pos = 0;

    while (pos < fontCount && fontList[pos].line_height < font->line_height) { ++pos; }

    for (uint8_t i = fontCount; i > pos; --i) { fontList[i] = fontList[i - 1]; }

    fontList[pos] = {font->name, font, font->line_height, dsc->glyph_bitmap, dsc->glyph_dsc, dsc->cmaps, dsc->cmap_num};

    ++fontCount;
}

void listFonts() {
    for (int i = 0; i < fontCount; i++) { printf("name: %s, line-height %li, adv_w %i\n", fontList[i].name, fontList[i].line_height, fontList[i].glyph_dsc[6].adv_w); }
}