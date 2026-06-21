#ifndef COLOR_H
#define COLOR_H

#include "term.h"

// parse warna dari string: nama preset ("red", "orange", dll) atau hex ("#ff5500")
// return 1 sukses, 0 gagal (invalid format)
int color_parse(const char *str, RGB *out);

#endif
