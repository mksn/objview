#ifndef __MY_VERY_PERSONAL_OPEN_GL_INCLUDE_FILE__
#define __MY_VERY_PERSONAL_OPEN_GL_INCLUDE_FILE__

#ifdef __APPLE__
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif

enum {
	ATT_POSITION,
	ATT_TEXCOORD,
	ATT_NORMAL,
	ATT_TANGENT,
	ATT_BLEND_INDEX,
	ATT_BLEND_WEIGHT,
	ATT_COLOR
};

typedef unsigned char uchar;

extern char *xstrsep (char **strinp, const char *delim);
extern int xstrlcpy (char *dst, char *src, int sz);
extern int xstrlcat (char *dst, char *src, int sz);

#define strlcpy xstrlcpy
#define strlcat xstrlcat
#define strlsep xstrlsep

#endif

