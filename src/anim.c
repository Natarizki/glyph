#include "anim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
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

static void clear_screen(void) {
    printf("\033[2J\033[H");
}

static void hide_cursor(void) {
    printf("\033[?25l");
}

static void show_cursor(void) {
    printf("\033[?25h");
}

void anim_typewriter(Font *font, const char *text, RenderOptions *opts, int delay_ms) {
    int len = strlen(text);
    char buf[1024];

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

void anim_wave(Font *font, const char *text, RenderOptions *opts, int speed_ms, int loops) {
    signal(SIGINT, handle_sigint);
    hide_cursor();

    RenderOptions wave_opts = *opts;
    wave_opts.style = STYLE_RAINBOW;

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
