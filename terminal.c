#include "objview.h"
#include "terminal.h"
#include "ctype.h"

#include <stdarg.h>

#define CLHISTORY 20
#define NOLINES 20
#define LASTLINE (NOLINES-1)

static char *terminal_buf[NOLINES];
static char *terminal_command_lines[CLHISTORY];
static int cursor_pos = 0;
static int current_command_line = 0;

void terminal_init()
{
  memset(terminal_buf,0,sizeof(char*) * NOLINES);
  memset(terminal_command_lines,0,sizeof(char*) * CLHISTORY);
  terminal_command_lines[current_command_line] = malloc(1);
  terminal_command_lines[current_command_line][0] = '\0';
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
  glColor3f(1,1,1);
  for(i=0; i<NOLINES; i++) {
    if (terminal_buf[i] != NULL) {
      draw_string(20, 20*i+16, terminal_buf[i]);
    }
  }
  glColor3f(.8,.9,1);
  if (terminal_command_lines[current_command_line])
    draw_string(20, 20*i+16, terminal_command_lines[current_command_line]);
}

int terminal_special(const char special) 
{
  int last_pos = strlen(terminal_command_lines[current_command_line]);
  switch(special) {
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
    case GLUT_KEY_DOWN:
      do {
        current_command_line = (current_command_line - 1 < 0)? (CLHISTORY-1):current_command_line-1;
      } while (terminal_command_lines[current_command_line] == NULL);
      cursor_pos = strlen(terminal_command_lines[current_command_line]);
      break;
    case GLUT_KEY_UP:
      do {
        current_command_line = (current_command_line + 1 > (CLHISTORY-1)) ? (CLHISTORY-1):current_command_line + 1;
      } while (terminal_command_lines[current_command_line] == NULL);
      cursor_pos = strlen(terminal_command_lines[current_command_line]);
      break;
    default:
      return 0;
      break;
  }
  return 1;
}

int terminal_keyboard(const char key)
{
  fprintf(stderr, "terminal_command_lines = %s\n", terminal_command_lines[current_command_line]);

  if (key == 0x1b || key == '\r')
  {
    // return 0 and print string;
    fprintf(stderr, "Time to bail!");
    terminal_puts(terminal_command_lines[current_command_line]);
    free(terminal_command_lines[(CLHISTORY-1)]);
    memmove(terminal_command_lines+1, terminal_command_lines,(CLHISTORY-1));
    current_command_line = 0;
    terminal_command_lines[current_command_line] = malloc(1);
    terminal_command_lines[current_command_line][0] = '\0';
    cursor_pos = 0;
    return 0;
  }

  int last_pos = strlen(terminal_command_lines[current_command_line]);

  if (isprint(key)) {
    terminal_command_lines[current_command_line] = realloc(terminal_command_lines[current_command_line], last_pos+2);
    if (cursor_pos == last_pos) {
      terminal_command_lines[current_command_line][cursor_pos++] = key;
      terminal_command_lines[current_command_line][cursor_pos] = 0;
    } else {
      memmove (terminal_command_lines[current_command_line]+cursor_pos+1,
          terminal_command_lines[current_command_line]+cursor_pos, 
          last_pos-cursor_pos+1);
      terminal_command_lines[current_command_line][cursor_pos++] = key;
    }
    return 1;
  }
  return 0;
}
