#include "objview.h"
#include <stdarg.h>

#define NOLINES 20
#define LASTLINE (NOLINES-1)

static char *terminal_buf[NOLINES];

void terminal_init() 
{
	memset(terminal_buf,0,NOLINES);
}

void terminal_puts(const char *s)
{
	//TODO: dem dere newlines... remember
	free(terminal_buf[0]);
	memmove(terminal_buf, terminal_buf+1, NOLINES-1);
	terminal_buf[LASTLINE] = strdup(s);		
}

void terminal_printf(const char *fmt, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	terminal_puts(buf);
}
	 	
static void draw_string(float x, float y, const char *s) 
{
	glRasterPos2f(x+0.345, y+0.375);
	while(*s)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *s++);
}

void terminal_display()
{
	int i;
	for(i=0; i<NOLINES; i++) {
		if (terminal_buf[i] != NULL)
			draw_string(20, 20*i+16, terminal_buf[i]);
	}
}

void terminal_keyboard(void *keyEvent)
{
}


