#include "color.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    const char *name;
    RGB color;
} NamedColor;

static NamedColor presets[] = {
    {"red",     {255, 0, 0}},
    {"green",   {0, 255, 0}},
    {"blue",    {0, 0, 255}},
    {"yellow",  {255, 255, 0}},
    {"cyan",    {0, 255, 255}},
    {"magenta", {255, 0, 255}},
    {"white",   {255, 255, 255}},
    {"black",   {0, 0, 0}},
    {"orange",  {255, 165, 0}},
    {"purple",  {160, 32, 240}},
    {"pink",    {255, 105, 180}},
    {"lime",    {50, 205, 50}},
    {"teal",    {0, 128, 128}},
    {"gold",    {255, 215, 0}},
};
#define NUM_PRESETS (sizeof(presets) / sizeof(NamedColor))

static int hex_to_byte(const char *s) {
    int val = 0;
    for (int i = 0; i < 2; i++) {
        char c = tolower(s[i]);
        val *= 16;
        if (c >= '0' && c <= '9') val += c - '0';
        else if (c >= 'a' && c <= 'f') val += c - 'a' + 10;
        else return -1;
    }
    return val;
}

int color_parse(const char *str, RGB *out) {
    if (str[0] == '#') {
        if (strlen(str) != 7) return 0; // format harus #rrggbb
        int r = hex_to_byte(str + 1);
        int g = hex_to_byte(str + 3);
        int b = hex_to_byte(str + 5);
        if (r < 0 || g < 0 || b < 0) return 0;
        out->r = r; out->g = g; out->b = b;
        return 1;
    }

    for (size_t i = 0; i < NUM_PRESETS; i++) {
        if (strcmp(str, presets[i].name) == 0) {
            *out = presets[i].color;
            return 1;
        }
    }
    return 0;
}
