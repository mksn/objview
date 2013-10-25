#include "objview.h"
#include "terminal.h"
#include "parser.h"
#include "ctype.h"

#include <stdarg.h>
#include <glob.h>

#define CLHISTORY 20
#define NOLINES 20
#define LASTLINE (NOLINES-1)

#define MARGIN_X 15
#define PADDING_X 10
#define PADDING_Y 10

#define PROMPT "> "

#define TEXT_COLOR  1.0, 1.0, 1.0
#define INPUT_COLOR 1.0, 0.9, 0.8
#define CARET_COLOR 1.0, 1.0, 1.0

#ifdef RETINA
#define LEADING 4
#define FONTSIZE 24
#define FONT GLUT_BITMAP_TIMES_ROMAN_24
#define BASELINE 19
#define CARET_WIDTH 2
#else
#define LEADING 2
#define FONTSIZE 13
#define FONT GLUT_BITMAP_8_BY_13
#define BASELINE 10
#define CARET_WIDTH 1
#endif

#define LINEHEIGHT (FONTSIZE+LEADING)

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

static char *terminal_buf[NOLINES];
static char *command_history[CLHISTORY];
static char *command_line;
static int cursor_pos = 0;
static int tp;
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

void terminal_put_raw(const char *input)
{
  char *s = strdup(input);
  free(terminal_buf[0]);
  memmove(terminal_buf, terminal_buf+1, sizeof(char*) * (NOLINES-1));
  terminal_buf[LASTLINE] = s;
  while (*s)
  {
    if (*s == '\t') *s = ' ';
    s++;
  }
}

void terminal_puts(const char *input)
{
  char *buf = strdup(input);
  char *line = buf;
  line = strtok(buf, "\n");
  while (line)
  {
    terminal_put_raw(line);
    line = strtok(NULL, "\n");
  }
  free(buf);
  puts(input);
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
  glRasterPos2f(x+0.375, y+0.375);
  while(*s)
    glutBitmapCharacter(FONT, *s++);
}

static int measure_string(const char *s, int n)
{
  int i;
  int w = 0;
  if (n < 0)
    n = strlen(s);
  for (i=0; i < n; i++)
    w += glutBitmapWidth(FONT, s[i]);
  return w;
}

void terminal_display(int w, int h)
{
  int i, x, y;
  static int offset = 0;

  if (input_state) {
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    /* Draw background */
    glColor4f(0.2,0.2,0.2,0.5);
    glRectf(MARGIN_X, 0, w - MARGIN_X, PADDING_Y*2 + LINEHEIGHT * (NOLINES+1));

    /* Draw text */
    glColor3f(TEXT_COLOR);
    x = MARGIN_X + PADDING_X;
    y = PADDING_Y + BASELINE;
    for(i=0; i<NOLINES; i++) {
      if (terminal_buf[i] != NULL) {
        draw_string(x, y, terminal_buf[i]);
        y += LINEHEIGHT;
      }
    }

    /* Draw input buffer */
    glColor3f(INPUT_COLOR);
    offset = measure_string(PROMPT, -1);
    draw_string(x, y, PROMPT);
    draw_string(x + offset, y, command_line);

    /* Draw caret */
    offset += measure_string(command_line, cursor_pos);
    glColor3f(CARET_COLOR);
    glRectf(x+offset, y-BASELINE, x+offset+CARET_WIDTH, y+(FONTSIZE-BASELINE));

    visible = !visible;
    glColor3f(1,1,1);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
  }
}

static void debug_command_line_history(void)
{
#if DEBUG
  int i;
  printf("current history = %d\n", history_pos);
  for (i = 0; i < CLHISTORY; i++) {
    if (command_history[i])
      printf("history %d: '%s'\n", i, command_history[i]);
  }
#endif
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
      return (input_state = 1);
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
  D(fprintf(stderr, "command_line = [%s], #command_line = %d, cursor_pos: %d, key: %x\n",
        command_line, (int)strlen(command_line), cursor_pos, key));
  int candidate;
  char *t;
  int delta;

  if (key == 0x9) {
    // find the first previous ws
    int i;
    for (i = cursor_pos-1; i>=0; i--) {
      if (command_line[i] == ' ') {
        tp = i + 1;
        break;
      }
    }

    delta = cursor_pos - tp;
    if (delta > 0) {
      t = malloc (delta + 2);
      memset(t,0, delta + 2);
      strncpy(t, command_line + tp, delta);
      fprintf(stderr, "%s\n", command_line + tp);
      t[delta] = '*';
    } else {
      t = malloc(2);
      t[0] = '*';
      t[1] = 0x0;
    }

    char *o = malloc(80*sizeof(char));
    memset(o, 0, 80);
    glob_t g;
    D(fprintf(stderr,"finding tab completion for: %s\n", t));
    glob(t, GLOB_MARK, NULL, &g);
    if (g.gl_pathc == 1) {
      command_line = realloc(command_line, tp + (int)strlen(g.gl_pathv[0]) + 1);
      strcpy(&command_line[tp], g.gl_pathv[0]);
      cursor_pos = strlen(command_line);
      return (input_state = 1);
    }
    for (i=0; i<(int)g.gl_pathc; i++) {
      int olen = strlen(o);
      int glen = strlen(g.gl_pathv[i]);
      if (olen + glen + 1 <= 80)
        sprintf(o, "%s %s", o, g.gl_pathv[i]);
      else {
        terminal_puts(o);
        o = malloc(80*sizeof(char));
        sprintf(o, "%s", g.gl_pathv[i]);
      }
    }
    if (o && strlen(o))
      terminal_puts(o);
    free(t);
    fprintf(stderr, "tab completion!\n");
    return (input_state = 1);
  }

  if (key == 0x1b) {
    // Hold the command line in it's current state
    // and just make the terminal go away
    return (input_state = 0);
  }

  if (key == '\r')
  {
    // return 0 and print string;
    char *buf = malloc(strlen(PROMPT) + strlen(command_line) + 1);
    strcpy(buf, PROMPT);
    strcat(buf, command_line);
    terminal_puts(buf);
    free(buf);
    parser_main(command_line);
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
    return (input_state = 1);
  }

  int last_pos = strlen(command_line);

  if (glutGetModifiers() == GLUT_ACTIVE_CTRL) {
    D(fprintf(stderr, "%s: Ctrl active, key:%d\n", __func__, key));
    int p;

    switch(key) {
      case 0x1:
        cursor_pos = 0;
        break;

      case 0x2:
        cursor_pos = cursor_pos - 1 < 0 ? 0 : cursor_pos - 1;
        break;

      case 0x4:
        if (cursor_pos < last_pos) {
          memmove(command_line + cursor_pos,
              command_line + cursor_pos + 1,
              last_pos - cursor_pos - 1);
          command_line = realloc(command_line, last_pos);
          command_line[last_pos-1] = 0;
        }
        return (input_state = 1);
        break;

      case 0x5:
        cursor_pos = last_pos;
        break;

      case 0x6:
        cursor_pos = cursor_pos + 1 > last_pos ? last_pos : cursor_pos + 1;
        break;

      case 0xb:
        command_line = realloc(command_line, cursor_pos+1);
        command_line[cursor_pos] = 0;
        break;

      case BACKSPACE:
      case 0x17:
        debug_command_line_history();
        p = cursor_pos - 1;
        D(fprintf(stderr, "%s: last_pos: %d, cursor_pos: %d, command_line: %s\n",
            __func__, last_pos, cursor_pos, command_line));
        while (p >= 0) {
          if (command_line[p] == ' ') {
            break;
          }
          p--;
        }
        p = p<0?0:p;
        memmove(command_line + p,
            command_line + cursor_pos,
            last_pos - cursor_pos);
        command_line = realloc(command_line,
           last_pos - cursor_pos + p + 1);
        command_line[last_pos - cursor_pos + p] = 0;
        cursor_pos = p;
        break;

      case 0xe:
        debug_command_line_history();
        candidate = history_pos - 1;
        if (candidate < CLHISTORY && command_history[candidate] != NULL)
        {
          history_pos = candidate;
          free(command_line);
          command_line = strdup(command_history[history_pos]);
          cursor_pos = strlen(command_line);
        }
        break;

      case 0x10:
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

      case 0x3:
      case 0x15:
        debug_command_line_history();
        free(command_line);
        command_line = strdup("");
        cursor_pos = strlen(command_line);
        break;

      default:
        break;
    };
    return (input_state = 1);
  } else if (glutGetModifiers() == GLUT_ACTIVE_ALT) {
    D(fprintf(stderr, "%s: ALT active, key: %d, cursor_pos: %d\n",
        __func__, key, cursor_pos));
    int p;

    switch(key) {
      case 'b':
        p = cursor_pos - 1;
        D(fprintf(stderr, "%s: cursor_pos: %d, p: %d, last_pos: %d\n",
            __func__, cursor_pos, p, last_pos));
        while (p >= 0) {
          if (command_line[p] == ' ') {
            break;
          }
          p--;
        }
        cursor_pos = p;
        break;

      case 'd':
        p = cursor_pos + 1;
        while (p<=last_pos) {
          if (command_line[p] == ' ')  {
            break;
          }
          p++;
        }
        p = p > last_pos ? last_pos : p;
        memmove(command_line+cursor_pos,
            command_line+p, last_pos - p);
        command_line = realloc(command_line,
            last_pos - p + cursor_pos + 1);
        command_line[last_pos - p + cursor_pos] = 0;
        break;

      case 'f':
        p = cursor_pos + 1;
        while (p<=last_pos) {
          if (command_line[p] == ' ')
            break;
          p++;
        }
        cursor_pos = p;
        break;

      default:
        break;
    };

    return (input_state = 1);
  }

  if (key == BACKSPACE) {
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
  else if (key == DELETE) {
    if (cursor_pos < last_pos) {
      memmove(command_line + cursor_pos,
          command_line + cursor_pos + 1,
          last_pos - cursor_pos - 1);
      command_line = realloc(command_line, last_pos);
      command_line[last_pos-1] = 0;
    }
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
