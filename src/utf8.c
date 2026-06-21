#include "utf8.h"
#include <string.h>

int utf8_seqlen(unsigned char c) {
    if ((c & 0x80) == 0x00) return 1; // 0xxxxxxx
    if ((c & 0xE0) == 0xC0) return 2; // 110xxxxx
    if ((c & 0xF0) == 0xE0) return 3; // 1110xxxx
    if ((c & 0xF8) == 0xF0) return 4; // 11110xxx
    return 1; // fallback, byte invalid/lanjutan
}

int utf8_strlen(const char *s) {
    int count = 0;
    int len = strlen(s);
    int i = 0;
    while (i < len) {
        int seq = utf8_seqlen((unsigned char)s[i]);
        i += seq;
        count++;
    }
    return count;
}
