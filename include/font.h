#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define MAX_GLYPH_HEIGHT 16
#define MAX_GLYPH_WIDTH  64   // dinaikkan biar muat baris UTF-8 multi-byte
#define MAX_GLYPHS       256

typedef struct {
    int codepoint;
    int width;  // lebar dalam KOLOM TAMPILAN (codepoint), bukan byte
    int height;
    char rows[MAX_GLYPH_HEIGHT][MAX_GLYPH_WIDTH + 1];
} Glyph;

typedef struct {
    char name[64];
    int height;
    char hardblank;
    int glyph_count;
    Glyph glyphs[MAX_GLYPHS];
} Font;

Glyph *font_get_glyph(Font *font, int codepoint);
void font_init(Font *font);

#endif
