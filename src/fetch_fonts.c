#include "fetch_fonts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// daftar font FIGlet populer dari repo xero/figlet-fonts (curated list)
static const char *popular_fonts[] = {
    "3d", "Slant", "Shadow", "Block", "Banner",
    "Big", "Standard", "Lean", "Small", "Script",
    "Mini", "Bubble", "Digital", "Term", "Thick"
};
#define NUM_FONTS (sizeof(popular_fonts) / sizeof(char *))

int fetch_fonts(void) {
    const char *prefix = getenv("PREFIX");
    char dest_dir[512];

    if (prefix) {
        snprintf(dest_dir, sizeof(dest_dir), "%s/share/glyph/fonts", prefix);
    } else {
        strcpy(dest_dir, "fonts");
    }

    char mkdir_cmd[600];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", dest_dir);
    system(mkdir_cmd);

    printf("Fetching %zu popular FIGlet fonts to %s ...\n", NUM_FONTS, dest_dir);

    int success = 0;
    for (size_t i = 0; i < NUM_FONTS; i++) {
        char url[512];
        snprintf(url, sizeof(url),
            "https://raw.githubusercontent.com/xero/figlet-fonts/master/%s.flf",
            popular_fonts[i]);

        char dest_path[600];
        snprintf(dest_path, sizeof(dest_path), "%s/%s.flf", dest_dir, popular_fonts[i]);

        char cmd[1200];
        snprintf(cmd, sizeof(cmd),
            "curl -sf -o '%s' '%s'", dest_path, url);

        printf("  - %-12s ", popular_fonts[i]);
        int ret = system(cmd);
        if (ret == 0) {
            printf("OK\n");
            success++;
        } else {
            printf("FAILED\n");
            remove(dest_path); // hapus file kosong/gagal kalau ada
        }
    }

    printf("\nDone: %d/%zu fonts downloaded.\n", success, NUM_FONTS);
    return (success > 0) ? 0 : -1;
}
