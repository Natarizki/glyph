#ifndef ANIM_H
#define ANIM_H

#include "font.h"
#include "term.h"

// animasi muncul huruf satu-satu
void anim_typewriter(Font *font, const char *text, RenderOptions *opts, int delay_ms);

// animasi warna rainbow yang "berjalan" terus menerus
// loops = jumlah iterasi (0 = infinite sampai Ctrl+C)
void anim_wave(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops);

#endif
