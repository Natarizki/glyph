#include "nfont_parser.h"
#include "utf8.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int nfont_load(const char *path, Font *font) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    font_init(font);

    char line[256];

    if (!fgets(line, sizeof(line), f) || strncmp(line, "NFONT", 5) != 0) {
        fclose(f);
        return -1;
    }

    int current_glyph = -1;
    int row_idx = 0;

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "name:", 5) == 0) {
            strncpy(font->name, line + 6, sizeof(font->name) - 1);
        } else if (strncmp(line, "height:", 7) == 0) {
            font->height = atoi(line + 8);
        } else if (strncmp(line, "hardblank:", 10) == 0) {
            font->hardblank = line[11];
        } else if (strncmp(line, "glyph:", 6) == 0) {
            current_glyph = font->glyph_count;
            font->glyphs[current_glyph].codepoint = atoi(line + 7);
            font->glyphs[current_glyph].height = font->height;
            row_idx = 0;
            font->glyph_count++;
        } else if (strncmp(line, "end", 3) == 0) {
            current_glyph = -1;
        } else if (current_glyph >= 0 && row_idx < MAX_GLYPH_HEIGHT) {
            strncpy(font->glyphs[current_glyph].rows[row_idx],
                    line, MAX_GLYPH_WIDTH);

            // width dihitung dalam jumlah CODEPOINT, bukan byte
            int col_len = utf8_strlen(line);
            if (col_len > font->glyphs[current_glyph].width) {
                font->glyphs[current_glyph].width = col_len;
            }
            row_idx++;
        }
    }

    fclose(f);
    return 0;
}
