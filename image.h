#ifndef __IMAGE_H__
#define __IMAGE_H__

unsigned char *load_file(char *filename, int *lenp);
int make_texture(unsigned int texid, unsigned char *data, int w, int h, int n);
int load_texture_from_memory(unsigned int texid, unsigned char *data, int len);
int load_texture(unsigned int texid, char *filename);

#endif
