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
static long long get_time_ms(void) { return (long long)GetTickCount64(); }
#else
#include <unistd.h>
#include <time.h>
static void sleep_ms(int ms) { usleep(ms * 1000); }
static long long get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#endif

static volatile int gif_keep_running = 1;
static void gif_handle_sigint(int sig) { (void)sig; gif_keep_running = 0; }

static const char *ramp = " .:-=+*#%@";
#define RAMP_LEN 10

static void render_frame_ascii(unsigned char *data, int w, int h, int out_width, int out_height, int use_color) {
    size_t buf_size = (size_t)out_width * out_height * 24 + out_height + 64;
    char *out_buf = malloc(buf_size);
    if (!out_buf) return;

    size_t pos = 0;
    int last_r = -1, last_g = -1, last_b = -1;

    for (int oy = 0; oy < out_height; oy++) {
        int sy0 = (oy * h) / out_height;
        int sy1 = ((oy + 1) * h) / out_height;
        if (sy1 <= sy0) sy1 = sy0 + 1;

        last_r = last_g = last_b = -1; // reset tiap baris baru

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
                if (r != last_r || g != last_g || b != last_b) {
                    pos += sprintf(out_buf + pos, "\033[38;2;%d;%d;%dm", r, g, b);
                    last_r = r; last_g = g; last_b = b;
                }
                out_buf[pos++] = c;
            } else {
                out_buf[pos++] = c;
            }
        }
        if (use_color) {
            pos += sprintf(out_buf + pos, "\033[0m"); // reset sekali di akhir baris aja
        }
        out_buf[pos++] = '\n';
    }
    out_buf[pos] = '\0';

    fwrite(out_buf, 1, pos, stdout);
    free(out_buf);
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

int gif_to_ascii(const char *path, int out_width, int out_height, int use_color, int loops, int fps) {
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
    printf("\033[?25l");

    int frame_size = w * h * 3;
    int loop_count = 0;
    int target_interval_ms = (fps > 0) ? (1000 / fps) : 0;

    unsigned char *blend_buf = malloc(frame_size);
    if (!blend_buf) {
        stbi_image_free(data);
        free(delays);
        return -1;
    }

    while (gif_keep_running) {
        for (int fi = 0; fi < frames && gif_keep_running; fi++) {
            int next_fi = (fi + 1) % frames;
            unsigned char *fa = data + (fi * frame_size);
            unsigned char *fb = data + (next_fi * frame_size);

            int base_delay = delays ? delays[fi] * 10 : 100;
            if (base_delay <= 0) base_delay = 100;

            // interpolasi dimatikan total, pakai frame asli + delay asli/target fps
            int steps = 1;

            int sub_delay = (target_interval_ms > 0) ? target_interval_ms : (base_delay / steps);
            if (sub_delay <= 0) sub_delay = 1;

            for (int s = 0; s < steps && gif_keep_running; s++) {
                float alpha = (float)s / steps;

                for (int p = 0; p < frame_size; p++) {
                    blend_buf[p] = (unsigned char)(fa[p] * (1.0f - alpha) + fb[p] * alpha);
                }

                long long t_start = get_time_ms();

                printf("\033[2J\033[H");
                render_frame_ascii(blend_buf, w, h, out_width, out_height, use_color);
                fflush(stdout);

                long long t_end = get_time_ms();
                long long render_time = t_end - t_start;

                long long remaining = sub_delay - render_time;
                if (remaining > 0) {
                    sleep_ms((int)remaining);
                }
            }
        }
        loop_count++;
        if (loops > 0 && loop_count >= loops) break;
    }

    free(blend_buf);
    printf("\033[?25h");
    stbi_image_free(data);
    free(delays);
    return 0;
}
