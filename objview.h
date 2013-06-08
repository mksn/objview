#ifndef OBJVIEW_H
#define OBJVIEW_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE
#define BACKSPACE 0x7f
#define DELETE    0x8
#else
#include <GL/freeglut.h>
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE
#define BACKSPACE 0x8
#define DELETE    0x7f
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

extern char *xstrsep  (char **strinp, const char *delim);
extern int   xstrlcpy (char *dst,     char       *src,  int sz);
extern int   xstrlcat (char *dst,     char       *src,  int sz);

#ifndef __APPLE__
#define strlcpy xstrlcpy
#define strlcat xstrlcat
#define strlsep xstrlsep
#endif

#endif
