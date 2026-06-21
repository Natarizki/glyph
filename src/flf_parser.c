#include "flf_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Parser FIGlet (.flf) - support standar ASCII 32-126 + extra char code

int flf_load(const char *path, Font *font) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    font_init(font);

    char header[512];
    if (!fgets(header, sizeof(header), f)) {
        fclose(f);
        return -1;
    }

    // signature harus mulai "flf2a"
    if (strncmp(header, "flf2a", 5) != 0) {
        fclose(f);
        return -1;
    }

    char hardblank;
    int height, baseline, max_len, old_layout, comment_lines;

    // format: flf2a<hardblank> height baseline max_len old_layout comment_lines ...
    if (sscanf(header + 5, "%c %d %d %d %d %d",
               &hardblank, &height, &baseline,
               &max_len, &old_layout, &comment_lines) < 6) {
        fclose(f);
        return -1;
    }

    font->hardblank = hardblank;
    font->height = height;
    strncpy(font->name, path, sizeof(font->name) - 1);

    // skip baris komentar
    char line[512];
    for (int i = 0; i < comment_lines; i++) {
        if (!fgets(line, sizeof(line), f)) {
            fclose(f);
            return -1;
        }
    }

    // baca glyph standar ASCII 32 - 126
    for (int code = 32; code <= 126; code++) {
        int gi = font->glyph_count;
        font->glyphs[gi].codepoint = code;
        font->glyphs[gi].height = height;

        for (int row = 0; row < height && row < MAX_GLYPH_HEIGHT; row++) {
            if (!fgets(line, sizeof(line), f)) {
                fclose(f);
                return -1;
            }
            line[strcspn(line, "\n")] = 0;
            line[strcspn(line, "\r")] = 0;  // jaga-jaga CRLF

            int len = strlen(line);
            while (len > 0 && line[len - 1] == '@') {
                line[len - 1] = '\0';
                len--;
            }

            for (int k = 0; k < len; k++) {
                if (line[k] == hardblank) line[k] = ' ';
            }

            strncpy(font->glyphs[gi].rows[row], line, MAX_GLYPH_WIDTH);
            if (len > font->glyphs[gi].width) {
                font->glyphs[gi].width = len;
            }
        }

        // PENTING: pad semua baris glyph ini biar sama panjang (width)
        // biar nggak misalign pas dirender berurutan
        for (int row = 0; row < height && row < MAX_GLYPH_HEIGHT; row++) {
            int rl = strlen(font->glyphs[gi].rows[row]);
            for (int p = rl; p < font->glyphs[gi].width && p < MAX_GLYPH_WIDTH; p++) {
                font->glyphs[gi].rows[row][p] = ' ';
            }
            font->glyphs[gi].rows[row][font->glyphs[gi].width < MAX_GLYPH_WIDTH ? font->glyphs[gi].width : MAX_GLYPH_WIDTH] = '\0';
        }

        font->glyph_count++;
    }

    fclose(f);
    return 0;
}
