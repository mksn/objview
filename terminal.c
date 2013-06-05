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
static int visible = 0;
static int input_state = 0;

void terminal_init()
{
  memset(terminal_buf,0,sizeof(char*) * NOLINES);
  memset(command_history,0,sizeof(char*) * CLHISTORY);
  command_line = malloc(1);
  command_line[0] = '\0';
}

void terminal_puts(const char *s)
{
  //TODO: dem dere newlines... remember
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

static int measure_cursor_pos (void) 
{
  int i = 0;
  int rc = 0;

  while (i < cursor_pos && command_line[i]) 
  {
    rc += glutBitmapWidth(GLUT_BITMAP_8_BY_13, command_line[i]);
    i += 1;
  }

  return rc;
}

void terminal_display(void)
{
  int i;
  static int offset = 0;
  
  glColor3f(1,1,1);
  for(i=0; i<NOLINES; i++) {
    if (terminal_buf[i] != NULL) {
      draw_string(20, 20*i+16, terminal_buf[i]);
    }
  }
  glColor3f(1,.9,0.86);
  if (input_state) {
    draw_string(20, 20*i+16, command_line);
    offset = measure_cursor_pos();
    if (visible) { 
      glColor3f(1.0,0.5,0.5);
      glRectf(20+offset, 20*(i-1)+16+7, 20+offset+2, 20*i+16);
    }
    visible = !visible;
  }
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
      
    case 0x6:
      if (cursor_pos < last_pos) {
         memmove(command_line + cursor_pos, 
            command_line + cursor_pos + 1, 
            last_pos - cursor_pos - 1);
        command_line = realloc(command_line, last_pos);
        command_line[last_pos-1] = 0;
      }
      break;
      
   default:
      return (input_state = 0);
      break;
  }
  
  return (input_state = 1);
}

void terminal_open (void) 
{
  input_state = 1;
}

void terminal_close (void) 
{
  input_state = 0;
}

int terminal_keyboard(const char key)
{
  fprintf(stderr, "command_line = %s, key: %x\n", command_line, key);
  int candidate;

  if (key == 0x1b || key == '\r')
  {
    // return 0 and print string;
    fprintf(stderr, "Time to bail!\n");
    terminal_puts(command_line);
    free(command_history[(CLHISTORY-1)]);
    memmove(command_history+1, 
        command_history, 
        sizeof(char*)*(CLHISTORY-1));
    command_history[0] = strdup(command_line);
    history_pos = -1;
    command_line = malloc(1);
    command_line[0] = '\0';
    cursor_pos = 0;
    visible = 0;
    return (input_state = 0);
  }

  int last_pos = strlen(command_line);

  if (key == 0x7f) { // backspace
    if (cursor_pos - 1 >= 0) {
      cursor_pos -= 1;
      memmove(command_line + cursor_pos, 
          command_line + cursor_pos + 1, 
          last_pos - cursor_pos - 1);
      command_line = realloc(command_line, last_pos);
      command_line[last_pos-1] = 0;
    }
    return (input_state = 1);
  }
  else if (key == 8) { // || key == 127) { // delete ... we hope
    memmove(command_line + cursor_pos, 
        command_line + cursor_pos + 1, 
        last_pos - cursor_pos - 1);
    command_line = realloc(command_line, last_pos);
    command_line[last_pos-1] = 0;
    return (input_state = 1);
  }

  if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
    fprintf(stderr, "%s: Ctrl active\n", __func__);
    switch(key) { 
      case 0x1:
        cursor_pos = 0;
        break;

      case 0x2:
        cursor_pos = cursor_pos - 1 < 0 ? 0 : cursor_pos - 1;
        break;

      case 0x5:
        cursor_pos = last_pos;
        break;

      case 0x6:
        cursor_pos = cursor_pos + 1 > last_pos ? last_pos : cursor_pos + 1;
        break;

      case 0x4:
        memmove(command_line + cursor_pos, 
            command_line + cursor_pos + 1, 
            last_pos - cursor_pos - 1);
        command_line = realloc(command_line, last_pos);
        command_line[last_pos-1] = 0;
        return (input_state = 1);
        break;

      case 0xf:
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

      case 0x11:
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
        break;
    };
    return (input_state = 1);
  } else if (glutGetModifiers() == GLUT_ACTIVE_ALT) {
    fprintf(stderr, "%s: alt active\n", __func__);
    switch(key) {
      case 'b':
        break;

      case 'd':
        break;

      case 'f':
        break;

      default:
        break;
    };
    return (input_state = 1);
  }

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
    return (input_state = 1);
  }
  return (input_state = 0);
}
