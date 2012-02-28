#ifndef __MY_VERY_PERSONAL_OPEN_GL_INCLUDE_FILE__
#define __MY_VERY_PERSONAL_OPEN_GL_INCLUDE_FILE__

#ifdef __APPLE__
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
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
#endif

