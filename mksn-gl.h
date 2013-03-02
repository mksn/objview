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
#include <GL/freeglut.h>
#endif
#include <string.h>
#include <stdlib.h>

enum {
	ATT_POSITION,
	ATT_TEXCOORD,
	ATT_NORMAL,
	ATT_TANGENT,
	ATT_BLEND_INDEX,
	ATT_BLEND_WEIGHT,
	ATT_COLOR
};

#ifdef __APPLE__
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_3_2_CORE_PROFILE
#else
#define MKSN_GLUT_INIT GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE | GLUT_CORE_PROFILE
#endif
typedef unsigned char uchar;

extern char *xstrsep (char **strinp, const char *delim);
extern int xstrlcpy (char *dst, char *src, int sz);
extern int xstrlcat (char *dst, char *src, int sz);

#ifndef __APPLE__
#define strlcpy xstrlcpy
#define strlcat xstrlcat
#define strlsep xstrlsep
#endif

#endif

