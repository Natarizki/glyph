#include "term.h"
#include "utf8.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>

int term_width(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
}

static void enable_windows_ansi(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | 0x0004);
}
#else
#include <sys/ioctl.h>
#include <unistd.h>

int term_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80;
}

static void enable_windows_ansi(void) {}
#endif

static RGB hsv_to_rgb(float h, float s, float v) {
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

    RGB col;
    col.r = (uint8_t)((rf + m) * 255);
    col.g = (uint8_t)((gf + m) * 255);
    col.b = (uint8_t)((bf + m) * 255);
    return col;
}

static RGB lerp_rgb(RGB a, RGB b, float t) {
    RGB out;
    out.r = (uint8_t)(a.r + t * (b.r - a.r));
    out.g = (uint8_t)(a.g + t * (b.g - a.g));
    out.b = (uint8_t)(a.b + t * (b.b - a.b));
    return out;
}

static void set_fg_rgb(RGB c) {
    printf("\033[38;2;%d;%d;%dm", c.r, c.g, c.b);
}

static void reset_color(void) {
    printf("\033[0m");
}

static RGB style_color(RenderOptions *opts, int char_index, int total_chars) {
    switch (opts->style) {
        case STYLE_RAINBOW: {
            float hue = fmodf((360.0f * char_index) / (total_chars > 1 ? total_chars : 1) + opts->hue_offset, 360.0f);
            return hsv_to_rgb(hue, 1.0f, 1.0f);
        }
        case STYLE_GAY: {
            float hue = (float)(rand() % 360);
            return hsv_to_rgb(hue, 1.0f, 1.0f);
        }
        case STYLE_GRADIENT: {
            // gradasi dari fg_color ke putih
            float t = (total_chars > 1) ? (float)char_index / (total_chars - 1) : 0;
            RGB white = {255, 255, 255};
            return lerp_rgb(opts->fg_color, white, t);
        }
        case STYLE_CUSTOM_GRADIENT: {
            float t = (total_chars > 1) ? (float)char_index / (total_chars - 1) : 0;
            return lerp_rgb(opts->grad_from, opts->grad_to, t);
        }
        default:
            return opts->fg_color;
    }
}

int render_text(Font *font, const char *text, RenderOptions *opts) {
    enable_windows_ansi();

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
                if (seq > 64) seq = 1;

                if (!(seq == 1 && rowstr[byte_i] == ' ')) {
                    RGB col = style_color(opts, i, len);
                    set_fg_rgb(col);

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
