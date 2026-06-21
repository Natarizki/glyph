#ifndef TERM_H
#define TERM_H

#include "font.h"
#include <stdint.h>

typedef enum {
    STYLE_PLAIN,
    STYLE_GRADIENT,
    STYLE_BORDER,
    STYLE_RAINBOW,
    STYLE_GAY,
    STYLE_CUSTOM_GRADIENT
} RenderStyle;

typedef struct {
    uint8_t r, g, b;
} RGB;

typedef struct {
    RenderStyle style;
    RGB fg_color;
    RGB grad_from;
    RGB grad_to;
    int use_border;
    float hue_offset;   // buat animasi wave, geser rainbow tiap frame
} RenderOptions;

int term_width(void);
int render_text(Font *font, const char *text, RenderOptions *opts);

#endif
