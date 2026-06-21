#ifndef UTF8_H
#define UTF8_H

#define UTF8_MAX_SEQ 64  // buffer aman, walau standar UTF-8 max 4 byte

int utf8_seqlen(unsigned char c);
int utf8_strlen(const char *s);

#endif
