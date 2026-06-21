#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "font.h"
#include "flf_parser.h"
#include "nfont_parser.h"
#include "term.h"
#include "image.h"

#define GLYPH_VERSION "1.0.0"

static void print_usage(const char *prog) {
    printf("GLYPH - Generate Lettering Yielding Pixel-art & Hues\n\n");
    printf("Usage:\n");
    printf("  %s text -f <font> \"text\" [options]\n", prog);
    printf("  %s img <image.png|jpg> [-w width] [-h height] [--no-color]\n", prog);
    printf("\nFont can be:\n");
    printf("  - direct path (./fonts/3d.flf)\n");
    printf("  - name only (3d, standard, slant) - auto-resolved from ./fonts/,\n");
    printf("    $PREFIX/share/glyph/fonts/, $PREFIX/share/figlet/\n");
    printf("\nText options:\n");
    printf("  -F, --style   plain|gradient|rainbow|gay|border\n");
    printf("  --color       red|green|blue|yellow|cyan|magenta|white\n");
    printf("\nOther:\n");
    printf("  -h, --help     show this help message\n");
    printf("  -v, --version  show version\n");
}

static void print_version(void) {
    printf("glyph version %s\n", GLYPH_VERSION);
}

static int parse_color(const char *name) {
    if (strcmp(name, "red") == 0) return COL_RED;
    if (strcmp(name, "green") == 0) return COL_GREEN;
    if (strcmp(name, "blue") == 0) return COL_BLUE;
    if (strcmp(name, "yellow") == 0) return COL_YELLOW;
    if (strcmp(name, "cyan") == 0) return COL_CYAN;
    if (strcmp(name, "magenta") == 0) return COL_MAGENTA;
    return COL_WHITE;
}

static int is_flf(const char *path) {
    int len = strlen(path);
    return (len > 4 && strcmp(path + len - 4, ".flf") == 0);
}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static char resolved_path[512];

static const char *resolve_font_path(const char *name) {
    if (file_exists(name)) {
        return name;
    }

    snprintf(resolved_path, sizeof(resolved_path), "fonts/%s", name);
    if (file_exists(resolved_path)) return resolved_path;

    snprintf(resolved_path, sizeof(resolved_path), "fonts/%s.flf", name);
    if (file_exists(resolved_path)) return resolved_path;

    snprintf(resolved_path, sizeof(resolved_path), "fonts/%s.nfont", name);
    if (file_exists(resolved_path)) return resolved_path;

    const char *prefix = getenv("PREFIX");
    if (prefix) {
        snprintf(resolved_path, sizeof(resolved_path), "%s/share/glyph/fonts/%s.nfont", prefix, name);
        if (file_exists(resolved_path)) return resolved_path;

        snprintf(resolved_path, sizeof(resolved_path), "%s/share/glyph/fonts/%s.flf", prefix, name);
        if (file_exists(resolved_path)) return resolved_path;

        snprintf(resolved_path, sizeof(resolved_path), "%s/share/figlet/%s.flf", prefix, name);
        if (file_exists(resolved_path)) return resolved_path;

        snprintf(resolved_path, sizeof(resolved_path), "%s/share/figlet/%s", prefix, name);
        if (file_exists(resolved_path)) return resolved_path;
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
        print_version();
        return 0;
    }

    if (strcmp(argv[1], "text") == 0) {
        char *font_arg = NULL;
        char *text = NULL;
        RenderOptions opts = { STYLE_PLAIN, COL_WHITE, COL_BLACK, 0 };

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
                font_arg = argv[++i];
            } else if ((strcmp(argv[i], "--style") == 0 || strcmp(argv[i], "-F") == 0) && i + 1 < argc) {
                i++;
                if (strcmp(argv[i], "gradient") == 0) opts.style = STYLE_GRADIENT;
                else if (strcmp(argv[i], "rainbow") == 0) opts.style = STYLE_RAINBOW;
                else if (strcmp(argv[i], "gay") == 0) opts.style = STYLE_GAY;
                else if (strcmp(argv[i], "border") == 0) opts.use_border = 1;
            } else if (strcmp(argv[i], "--color") == 0 && i + 1 < argc) {
                opts.fg_color = parse_color(argv[++i]);
            } else if (!text) {
                text = argv[i];
            }
        }

        if (!font_arg || !text) {
            print_usage(argv[0]);
            return 1;
        }

        const char *real_path = resolve_font_path(font_arg);
        if (!real_path) {
            fprintf(stderr, "font not found: %s\n", font_arg);
            fprintf(stderr, "(searched: direct path, ./fonts/, $PREFIX/share/glyph/fonts/, $PREFIX/share/figlet/)\n");
            return 1;
        }

        Font font;
        int ok = is_flf(real_path)
            ? flf_load(real_path, &font)
            : nfont_load(real_path, &font);

        if (ok != 0) {
            fprintf(stderr, "failed to load font: %s\n", real_path);
            return 1;
        }

        return render_text(&font, text, &opts);

    } else if (strcmp(argv[1], "img") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }
        char *img_path = argv[2];
        int w = 0, h = 0, use_color = 1;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
                w = atoi(argv[++i]);
            } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
                h = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--no-color") == 0) {
                use_color = 0;
            }
        }

        return image_to_ascii(img_path, w, h, use_color);
    }

    print_usage(argv[0]);
    return 1;
}
