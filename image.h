// vim: sw=2 sts=2 et

#ifndef image_h
#define image_h

unsigned char *load_file(char *filename, int *lenp);
int make_texture(unsigned int texid, unsigned char *data, int w, int h, int n);
int load_texture_from_memory(unsigned int texid, unsigned char *data, int len);
int load_texture(unsigned int texid, char *filename);

#endif
