#include "flf_parser.h"
#include "utf8.h"
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

    if (strncmp(header, "flf2a", 5) != 0) {
        fclose(f);
        return -1;
    }

    char hardblank;
    int height, baseline, max_len, old_layout, comment_lines;

    if (sscanf(header + 5, "%c %d %d %d %d %d",
               &hardblank, &height, &baseline,
               &max_len, &old_layout, &comment_lines) < 6) {
        fclose(f);
        return -1;
    }

    font->hardblank = hardblank;
    font->height = height;
    strncpy(font->name, path, sizeof(font->name) - 1);

    char line[512];
    for (int i = 0; i < comment_lines; i++) {
        if (!fgets(line, sizeof(line), f)) {
            fclose(f);
            return -1;
        }
    }

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
            line[strcspn(line, "\r")] = 0;

            int len = strlen(line);
            while (len > 0 && line[len - 1] == '@') {
                line[len - 1] = '\0';
                len--;
            }

            for (int k = 0; k < len; k++) {
                if (line[k] == hardblank) line[k] = ' ';
            }

            strncpy(font->glyphs[gi].rows[row], line, MAX_GLYPH_WIDTH);

            int col_len = utf8_strlen(line);
            if (col_len > font->glyphs[gi].width) {
                font->glyphs[gi].width = col_len;
            }
        }

        // pad pakai selisih CODEPOINT, tapi index buffer pakai BYTE
        // biar aman buat font yang isinya UTF-8 multi-byte (kayak 3d.flf)
        for (int row = 0; row < height && row < MAX_GLYPH_HEIGHT; row++) {
            char *r = font->glyphs[gi].rows[row];
            int row_codepoints = utf8_strlen(r);
            int pad_needed = font->glyphs[gi].width - row_codepoints;
            int byte_len = strlen(r);

            for (int p = 0; p < pad_needed && (byte_len + p) < MAX_GLYPH_WIDTH; p++) {
                r[byte_len + p] = ' ';
            }
            int final_len = byte_len + pad_needed;
            if (final_len > MAX_GLYPH_WIDTH) final_len = MAX_GLYPH_WIDTH;
            if (final_len < 0) final_len = byte_len;
            r[final_len] = '\0';
        }

        font->glyph_count++;
    }

    fclose(f);
    return 0;
}
