#ifndef ANIM_H
#define ANIM_H

#include "font.h"
#include "term.h"
#include <stdio.h>

void anim_typewriter(Font *font, const char *text, RenderOptions *opts, int delay_ms, FILE *output);
void anim_wave(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops, FILE *output);
void anim_fade(Font *font, const char *text, RenderOptions *opts, int speed_ms, int steps, FILE *output);
void anim_bounce(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops, FILE *output);

#endif
