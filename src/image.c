#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>

static const char *ramp = " .:-=+*#%@";
#define RAMP_LEN 10

int image_to_ascii(const char *path, int out_width, int out_height, int use_color) {
    int w, h, channels;
    unsigned char *data = stbi_load(path, &w, &h, &channels, 3); // force RGB
    if (!data) {
        fprintf(stderr, "gagal load gambar: %s\n", path);
        return -1;
    }

    if (out_width <= 0) out_width = term_width();
    if (out_height <= 0) {
        // jaga aspect ratio (1 baris terminal ~2x tinggi karakter)
        out_height = (h * out_width) / (w * 2);
    }
    if (out_height <= 0) out_height = 1;

    for (int oy = 0; oy < out_height; oy++) {
        int sy0 = (oy * h) / out_height;
        int sy1 = ((oy + 1) * h) / out_height;
        if (sy1 <= sy0) sy1 = sy0 + 1;

        for (int ox = 0; ox < out_width; ox++) {
            int sx0 = (ox * w) / out_width;
            int sx1 = ((ox + 1) * w) / out_width;
            if (sx1 <= sx0) sx1 = sx0 + 1;

            long sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;

            for (int sy = sy0; sy < sy1 && sy < h; sy++) {
                for (int sx = sx0; sx < sx1 && sx < w; sx++) {
                    unsigned char *px = &data[(sy * w + sx) * 3];
                    sum_r += px[0];
                    sum_g += px[1];
                    sum_b += px[2];
                    count++;
                }
            }
            if (count == 0) count = 1;

            int r = sum_r / count;
            int g = sum_g / count;
            int b = sum_b / count;
            int lum = (r * 299 + g * 587 + b * 114) / 1000;

            int idx = (lum * (RAMP_LEN - 1)) / 255;
            char c = ramp[idx];

            if (use_color) {
                printf("\033[38;2;%d;%d;%dm%c\033[0m", r, g, b, c);
            } else {
                putchar(c);
            }
        }
        putchar('\n');
    }

    stbi_image_free(data);
    return 0;
}
