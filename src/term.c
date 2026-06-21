#include "term.h"
#include "utf8.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

int term_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80;
}

// warna internal 64-bit: 16-bit per channel (R,G,B,A)
typedef struct {
    uint16_t r, g, b, a;
} Color64;

// HSV ke RGB presisi 16-bit per channel
static Color64 hsv_to_rgb64(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - c;
    float rf, gf, bf;

    if (h < 60)       { rf = c; gf = x; bf = 0; }
    else if (h < 120) { rf = x; gf = c; bf = 0; }
    else if (h < 180) { rf = 0; gf = c; bf = x; }
    else if (h < 240) { rf = 0; gf = x; bf = c; }
    else if (h < 300) { rf = x; gf = 0; bf = c; }
    else              { rf = c; gf = 0; bf = x; }

    Color64 col;
    col.r = (uint16_t)((rf + m) * 65535.0f);
    col.g = (uint16_t)((gf + m) * 65535.0f);
    col.b = (uint16_t)((bf + m) * 65535.0f);
    col.a = 65535;
    return col;
}

// downscale 16-bit channel -> 8-bit buat dikirim ke terminal (ANSI 24-bit)
static inline int to8(uint16_t v) {
    return v >> 8;
}

static void set_fg_rgb64(Color64 c) {
    printf("\033[38;2;%d;%d;%dm", to8(c.r), to8(c.g), to8(c.b));
}

static void reset_color(void) {
    printf("\033[0m");
}

static Color64 style_color(RenderOptions *opts, int char_index, int total_chars) {
    Color64 col;
    switch (opts->style) {
        case STYLE_RAINBOW: {
            float hue = (360.0f * char_index) / (total_chars > 1 ? total_chars : 1);
            col = hsv_to_rgb64(hue, 1.0f, 1.0f);
            break;
        }
        case STYLE_GRADIENT: {
            float t = (total_chars > 1) ? (float)char_index / (total_chars - 1) : 0;
            uint16_t base = (uint16_t)(opts->fg_color * 257); // 8-bit -> 16-bit
            col.r = (uint16_t)(base + t * (65535 - base));
            col.g = (uint16_t)(base + t * (65535 - base));
            col.b = (uint16_t)(base + t * (65535 - base));
            col.a = 65535;
            break;
        }
        case STYLE_GAY: {
            float hue = (float)(rand() % 360);
            col = hsv_to_rgb64(hue, 1.0f, 1.0f);
            break;
        }
        default: {
            uint16_t v = (uint16_t)(opts->fg_color * 257);
            col.r = col.g = col.b = v;
            col.a = 65535;
        }
    }
    return col;
}

int render_text(Font *font, const char *text, RenderOptions *opts) {
    int len = strlen(text);
    if (len == 0) return -1;

    int height = font->height;
    int total_width = 0;
    for (int i = 0; i < len; i++) {
        Glyph *g = font_get_glyph(font, (unsigned char)text[i]);
        if (g) total_width += g->width;
    }
    if (opts->use_border) total_width += 4;

    if (opts->style == STYLE_GAY) srand((unsigned int)time(NULL));

    if (opts->use_border) {
        printf("+");
        for (int x = 0; x < total_width - 2; x++) printf("-");
        printf("+\n");
    }

    for (int row = 0; row < height; row++) {
        if (opts->use_border) printf("| ");

        for (int i = 0; i < len; i++) {
            Glyph *g = font_get_glyph(font, (unsigned char)text[i]);
            if (!g) continue;

            const char *rowstr = g->rows[row];
            int rowbytes = strlen(rowstr);
            int byte_i = 0;

            while (byte_i < rowbytes) {
                int seq = utf8_seqlen((unsigned char)rowstr[byte_i]);
                if (seq > 64) seq = 1; // safety, buffer 64 byte

                if (!(seq == 1 && rowstr[byte_i] == ' ')) {
                    Color64 col = style_color(opts, i, len);
                    set_fg_rgb64(col);

                    char cell[64] = {0};
                    memcpy(cell, &rowstr[byte_i], seq);
                    fwrite(cell, 1, seq, stdout);
                    reset_color();
                } else {
                    putchar(' ');
                }
                byte_i += seq;
            }
        }

        if (opts->use_border) printf(" |");
        printf("\n");
    }

    if (opts->use_border) {
        printf("+");
        for (int x = 0; x < total_width - 2; x++) printf("-");
        printf("+\n");
    }
    return 0;
}
