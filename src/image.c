#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

static volatile int gif_keep_running = 1;
static void gif_handle_sigint(int sig) { (void)sig; gif_keep_running = 0; }

static const char *ramp = " .:-=+*#%@";
#define RAMP_LEN 10

static void render_frame_ascii(unsigned char *data, int w, int h, int out_width, int out_height, int use_color) {
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
}

int image_to_ascii(const char *path, int out_width, int out_height, int use_color) {
    int w, h, channels;
    unsigned char *data = stbi_load(path, &w, &h, &channels, 3);
    if (!data) {
        fprintf(stderr, "gagal load gambar: %s\n", path);
        return -1;
    }

    if (out_width <= 0) out_width = term_width();
    if (out_height <= 0) {
        out_height = (h * out_width) / (w * 2);
    }
    if (out_height <= 0) out_height = 1;

    render_frame_ascii(data, w, h, out_width, out_height, use_color);

    stbi_image_free(data);
    return 0;
}

int gif_to_ascii(const char *path, int out_width, int out_height, int use_color, int loops) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "gagal buka file: %s\n", path);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *buffer = malloc(filesize);
    if (!buffer) {
        fclose(f);
        return -1;
    }
    fread(buffer, 1, filesize, f);
    fclose(f);

    int w, h, frames, channels;
    int *delays = NULL;

    unsigned char *data = stbi_load_gif_from_memory(
        buffer, (int)filesize, &delays, &w, &h, &frames, &channels, 3);

    free(buffer);

    if (!data) {
        fprintf(stderr, "gagal decode GIF: %s\n", path);
        return -1;
    }

    if (out_width <= 0) out_width = term_width();
    if (out_height <= 0) {
        out_height = (h * out_width) / (w * 2);
    }
    if (out_height <= 0) out_height = 1;

    signal(SIGINT, gif_handle_sigint);
    printf("\033[?25l"); // hide cursor

    int frame_size = w * h * 3;
    int loop_count = 0;

    while (gif_keep_running) {
        for (int fi = 0; fi < frames && gif_keep_running; fi++) {
            printf("\033[2J\033[H"); // clear screen
            unsigned char *frame_data = data + (fi * frame_size);
            render_frame_ascii(frame_data, w, h, out_width, out_height, use_color);
            fflush(stdout);

            int delay_ms = delays ? delays[fi] * 10 : 100; // delay GIF biasanya dalam centiseconds
            if (delay_ms <= 0) delay_ms = 100;
            sleep_ms(delay_ms);
        }
        loop_count++;
        if (loops > 0 && loop_count >= loops) break;
    }

    printf("\033[?25h"); // show cursor
    stbi_image_free(data);
    free(delays);
    return 0;
}
