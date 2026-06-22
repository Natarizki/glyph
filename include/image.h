#ifndef IMAGE_H
#define IMAGE_H

int image_to_ascii(const char *path, int out_width, int out_height, int use_color);
int gif_to_ascii(const char *path, int out_width, int out_height, int use_color, int loops);

#endif
