#ifndef TERM_H
#define TERM_H

#include "font.h"

typedef enum {
    STYLE_PLAIN,
    STYLE_GRADIENT,
    STYLE_BORDER,
    STYLE_RAINBOW,
    STYLE_GAY
} RenderStyle;

typedef enum {
    COL_BLACK = 0, COL_RED, COL_GREEN, COL_YELLOW,
    COL_BLUE, COL_MAGENTA, COL_CYAN, COL_WHITE
} TermColor;

typedef struct {
    RenderStyle style;
    int fg_color;
    int bg_color;
    int use_border;
} RenderOptions;

// dapatkan lebar terminal (kolom), buat keperluan image render
int term_width(void);

// render teks pakai font ke stdout, format ANSI murni
int render_text(Font *font, const char *text, RenderOptions *opts);

#endif
