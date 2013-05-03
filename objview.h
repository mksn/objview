#ifndef objview_h
#define objview_h

#ifdef __APPLE__
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_3_2_CORE_PROFILE
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_CORE_PROFILE
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

extern char *xstrsep (char **strinp, const char *delim);
extern int xstrlcpy (char *dst, char *src, int sz);
extern int xstrlcat (char *dst, char *src, int sz);

#ifndef __APPLE__
#define strlcpy xstrlcpy
#define strlcat xstrlcat
#define strlsep xstrlsep
#endif

#endif
