#include "objview.h"
#include "terminal.h"
#include "ctype.h"

#include <stdarg.h>

#define CLHISTORY 20
#define NOLINES 20
#define LASTLINE (NOLINES-1)

static char *terminal_buf[NOLINES];
static char *command_history[CLHISTORY];
static char *command_line;
static int cursor_pos = 0;
static int history_pos = 0;

void terminal_init()
{
  memset(terminal_buf,0,sizeof(char*) * NOLINES);
  memset(command_history,0,sizeof(char*) * CLHISTORY);
  command_line = malloc(1);
  command_line[0] = '\0';
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
  if (command_line)
    draw_string(20, 20*i+16, command_line);
}

static void debug_command_line_history(void)
{
  int i;
  printf("current history = %d\n", history_pos);
  for (i = 0; i < CLHISTORY; i++) {
    if (command_history[i])
      printf("history %d: '%s'\n", i, command_history[i]);
  }
}

int terminal_special(const char special) 
{
  int last_pos = strlen(command_line);
  int candidate;
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
      debug_command_line_history();
      candidate = history_pos - 1;
      if (candidate == -1)
      {
        history_pos = candidate;
        free(command_line);
        command_line = strdup("");
        cursor_pos = strlen(command_line);
      }
      else if (candidate >= 0 && command_history[candidate] != NULL)
      {
        history_pos = candidate;
        free(command_line);
        command_line = strdup(command_history[history_pos]);
        cursor_pos = strlen(command_line);
      }
      break;
    case GLUT_KEY_UP:
      debug_command_line_history();
      candidate = history_pos + 1;
      if (candidate < CLHISTORY && command_history[candidate] != NULL)
      {
        history_pos = candidate;
        free(command_line);
        command_line = strdup(command_history[history_pos]);
        cursor_pos = strlen(command_line);
      }
      break;
    default:
      return 0;
      break;
  }
  return 1;
}

int terminal_keyboard(const char key)
{
  fprintf(stderr, "command_line = %s\n", command_line);

  if (key == 0x1b || key == '\r')
  {
    // return 0 and print string;
    fprintf(stderr, "Time to bail!\n");
    terminal_puts(command_line);
    free(command_history[(CLHISTORY-1)]);
    memmove(command_history+1, command_history, sizeof(char*)*(CLHISTORY-1));
    command_history[0] = strdup(command_line);
    history_pos = -1;
    command_line = malloc(1);
    command_line[0] = '\0';
    cursor_pos = 0;
    return 0;
  }

  int last_pos = strlen(command_line);

  if (isprint(key)) {
    command_line = realloc(command_line, last_pos+2);
    if (cursor_pos == last_pos) {
      command_line[cursor_pos++] = key;
      command_line[cursor_pos] = 0;
    } else {
      memmove (command_line+cursor_pos+1,
          command_line+cursor_pos, 
          last_pos-cursor_pos+1);
      command_line[cursor_pos++] = key;
    }
    return 1;
  }
  return 0;
}
