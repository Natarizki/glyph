#include "font.h"
#include <string.h>

void font_init(Font *font) {
    memset(font, 0, sizeof(Font));
    font->hardblank = '$';
}

Glyph *font_get_glyph(Font *font, int codepoint) {
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}
