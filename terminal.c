#include "objview.h"
#include "terminal.h"
#include "ctype.h"

#include <stdarg.h>

#define NOLINES 20
#define LASTLINE (NOLINES-1)

static char *terminal_buf[NOLINES];
static char *terminal_command_line;

void terminal_init()
{
  memset(terminal_buf,0,sizeof(char*) * NOLINES);
  terminal_command_line = malloc(1);
  terminal_command_line[0] = '\0';
}

void terminal_puts(const char *s)
{
  //todo: DEM DEre newlines... remember
  free(terminal_buf[0]);
  memmove(terminal_buf, terminal_buf+1, sizeof(char*) * (NOLINES-1));
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
    if (terminal_buf[i] != NULL) {
      draw_string(20, 20*i+16, terminal_buf[i]);
    }
  }
  if (terminal_command_line)
    draw_string(20, 20*i+16, terminal_command_line);
}

int terminal_keyboard(const char key)
{
  static int cursor_pos = 0;

  fprintf(stderr, "terminal_command_line = %s\n", terminal_command_line);

  if (key == 0x1b || key == '\r')
  {
    // return 0 and print string;
    fprintf(stderr, "Time to bail!");
    terminal_puts(terminal_command_line);
    free(terminal_command_line);
    terminal_command_line = malloc(1);
    terminal_command_line[0] = '\0';
    cursor_pos = 0;
    return 0;
  }

  int last_pos = strlen(terminal_command_line);

  switch(key) {
    case GLUT_KEY_LEFT:
      cursor_pos = cursor_pos - 1 < 0 ? 0 : cursor_pos - 1;
      break;
    case GLUT_KEY_RIGHT:
      cursor_pos = cursor_pos + 1 > last_pos ? last_pos : cursor_pos + 1;
      break;
    case GLUT_KEY_HOME:
      cursor_pos = 0;
      break;
    case GLUT_KEY_END:
      cursor_pos = last_pos;
      break;
    default:
      break;
  }

  if (isalnum(key)) {
    terminal_command_line = realloc(terminal_command_line, last_pos+1);
    if (cursor_pos == strlen(terminal_command_line)) {
      terminal_command_line[cursor_pos++] = key;
    } else {
      memmove (terminal_command_line+cursor_pos+1,
          terminal_command_line+cursor_pos, 
          last_pos-cursor_pos+1);
      terminal_command_line[cursor_pos++] = key;
    }
    return 1;
  }
  return 0;
}
