#include "anim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

static volatile int keep_running = 1;

static void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
}

static void clear_screen(void) { printf("\033[2J\033[H"); }
static void hide_cursor(void) { printf("\033[?25l"); }
static void show_cursor(void) { printf("\033[?25h"); }

void anim_typewriter(Font *font, const char *text, RenderOptions *opts, int delay_ms, FILE *output) {
    int len = strlen(text);
    char buf[1024];

    if (output) {
        for (int i = 1; i <= len; i++) {
            strncpy(buf, text, i);
            buf[i] = '\0';
            fprintf(output, "--- Frame %d ---\n", i);
            render_text_to_stream(font, buf, opts, output);
            fprintf(output, "\n");
        }
        return;
    }

    hide_cursor();
    for (int i = 1; i <= len; i++) {
        clear_screen();
        strncpy(buf, text, i);
        buf[i] = '\0';
        render_text(font, buf, opts);
        fflush(stdout);
        sleep_ms(delay_ms);
    }
    show_cursor();
}

void anim_wave(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops, FILE *output) {
    RenderOptions wave_opts = *opts;
    wave_opts.style = STYLE_RAINBOW;

    if (output) {
        if (loops <= 0) {
            fprintf(stderr, "error: --output butuh --loops > 0 buat animasi wave\n");
            return;
        }
        for (int frame = 0; frame < loops; frame++) {
            wave_opts.hue_offset = fmodf((float)(frame * 12), 360.0f);
            fprintf(output, "--- Frame %d ---\n", frame + 1);
            render_text_to_stream(font, text, &wave_opts, output);
            fprintf(output, "\n");
        }
        return;
    }

    signal(SIGINT, handle_sigint);
    hide_cursor();
    int frame = 0;

    while (keep_running) {
        clear_screen();
        wave_opts.hue_offset = fmodf((float)(frame * 12), 360.0f);
        render_text(font, text, &wave_opts);
        fflush(stdout);
        sleep_ms(speed_ms);

        frame++;
        if (loops > 0 && frame >= loops) break;
    }
    show_cursor();
}

void anim_fade(Font *font, const char *text, RenderOptions *opts, int speed_ms, int steps, FILE *output) {
    RenderOptions fade_opts = *opts;
    RGB target = opts->fg_color;
    if (steps <= 0) steps = 20;

    if (output) {
        for (int s = 1; s <= steps; s++) {
            float t = (float)s / steps;
            RGB faded;
            faded.r = (uint8_t)(target.r * t);
            faded.g = (uint8_t)(target.g * t);
            faded.b = (uint8_t)(target.b * t);
            fade_opts.style = STYLE_PLAIN;
            fade_opts.fg_color = faded;

            fprintf(output, "--- Frame %d ---\n", s);
            render_text_to_stream(font, text, &fade_opts, output);
            fprintf(output, "\n");
        }
        return;
    }

    hide_cursor();
    for (int s = 1; s <= steps; s++) {
        float t = (float)s / steps;
        RGB faded;
        faded.r = (uint8_t)(target.r * t);
        faded.g = (uint8_t)(target.g * t);
        faded.b = (uint8_t)(target.b * t);
        fade_opts.style = STYLE_PLAIN;
        fade_opts.fg_color = faded;

        clear_screen();
        render_text(font, text, &fade_opts);
        fflush(stdout);
        sleep_ms(speed_ms);
    }
    show_cursor();
}

void anim_bounce(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops, FILE *output) {
    int amplitude = 3;

    if (output) {
        if (loops <= 0) {
            fprintf(stderr, "error: --output butuh --loops > 0 buat animasi bounce\n");
            return;
        }
        for (int frame = 0; frame < loops; frame++) {
            int offset = (int)(amplitude * fabs(sin(frame * 0.3)));
            fprintf(output, "--- Frame %d ---\n", frame + 1);
            for (int i = 0; i < offset; i++) fprintf(output, "\n");
            render_text_to_stream(font, text, opts, output);
            fprintf(output, "\n");
        }
        return;
    }

    signal(SIGINT, handle_sigint);
    hide_cursor();
    int frame = 0;

    while (keep_running) {
        clear_screen();
        int offset = (int)(amplitude * fabs(sin(frame * 0.3)));
        for (int i = 0; i < offset; i++) printf("\n");

        render_text(font, text, opts);
        fflush(stdout);
        sleep_ms(speed_ms);

        frame++;
        if (loops > 0 && frame >= loops) break;
    }
    show_cursor();
}
