#ifndef TERM_H
#define TERM_H

#include "font.h"
#include <stdint.h>
#include <stdio.h>

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
    float hue_offset;
} RenderOptions;

int term_width(void);
int render_text(Font *font, const char *text, RenderOptions *opts);
int render_text_to_stream(Font *font, const char *text, RenderOptions *opts, FILE *stream);

#endif
