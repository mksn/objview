#include "objview.h"
#include "unit.h"
#include "terminal.h"
#include "parser.h"
#include "cache.h"

extern int   root_motion_comp;
static int   leftstate = 0, rightstate = 0, middlestate = 0;
static int   lastx;
static int   lasty;
static float pitch = 38;
static float yaw   = 45;
static float dist = 10;
static float swidth = 1;
static float sheight = 1;
static int   terminal_input = 0;
static float last_time = 0;

void keyboardFunc(unsigned char key, int x, int y)
{
  if (!terminal_input) {
    root_motion_comp = 1;
    switch (key) {
      case ' ':
        parser_main("select_next_unit()");
        break;

      case 'i':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'IDLE'");
        break;

      case 'w':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'WALK'");
        break;

      case 'r':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'RUN'");
        break;

      case 'a':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'TURN_LEFT'");
        break;

      case 'd':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'TURN_RIGHT'");
        break;

      case 'q':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'STRAFE_LEFT'");
        break;

      case 'e':
        parser_main("reset_current_action()");
        parser_main("set_current_action 'STRAFE_RIGHT'");
        break;

      case 'z':
        root_motion_comp = 0;
        parser_main("reset_current_action()");
        parser_main("set_current_action 'DEATH'");
        break;

      case 0x1b:
        parser_finalize();
        exit(1);
        break;

      case 0xd:
        terminal_input = 1;
        terminal_open();

      default:
        break;
    }
  }
  else
  {
    terminal_input = terminal_keyboard(key);
  }
}

void specialKeyboardFunc(int key, int x, int y)
{
  if(terminal_input) {
    terminal_input = terminal_special((const char)key);
  }
}

void mouseFunc(int button, int state, int x, int y)
{
  lastx = x;
  lasty = y;
  if (button == GLUT_LEFT_BUTTON)
    leftstate = state == GLUT_DOWN;
  if (button == GLUT_RIGHT_BUTTON)
    rightstate = state == GLUT_DOWN;
  if (button == GLUT_MIDDLE_BUTTON)
    middlestate = state == GLUT_DOWN;
}

void motionFunc(int x, int y)
{
  int diffx = x-lastx;
  int diffy = y-lasty;
  lastx = x;
  lasty = y;
  if (leftstate) {
    pitch += (float)diffy * 0.2;
    if (pitch > 90) pitch = 90;
    if (pitch < -90) pitch = -90;
    yaw += (float)diffx * 0.3;
  }
  if (middlestate) {
    dist += diffy * 0.01 * dist;
    if (dist < 1) dist = 1;
  }
}

void reshape(int w, int h)
{
  glViewport (0, 0, (GLsizei)w, (GLsizei)h);
  swidth = w;
  sheight = h;
}

void display(void)
{
  int i;
  int time_in_millis = glutGet(GLUT_ELAPSED_TIME);
  float this_time = time_in_millis * 0.001;
  float delta = this_time - last_time;
  last_time = this_time;

  glClearColor (0.3, 0.3, 0.4, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable (GL_DEPTH_TEST);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60, swidth / sheight, 0.05, 7000.0);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);

  glTranslatef(0, 0, -dist);
  glRotatef(pitch, 1.0, 0.0, 0.0);
  glRotatef(yaw, 0.0, 1.0, 0.0);
  glRotatef(-90, 1.0, 0.0, 0.0);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);

  char cmd[200];
  sprintf(cmd, "update(%g)", delta);
  parser_main(cmd);
  parser_main("draw()");

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_LIGHTING);

  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  for (i = -4; i <= 4; i++) {
    glVertex3f(-4, i, 0);
    glVertex3f(4, i, 0);
    glVertex3f(i, -4, 0);
    glVertex3f(i, 4, 0);
  }
  glEnd();
  glColor3f(1,1,1);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, swidth, sheight, 0, -1, 1);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  terminal_display(swidth, sheight);

  glutSwapBuffers();
  glutPostRedisplay();

  glutReportErrors();
}

int main (int argc, char **argv)
{
  root_motion_comp = 1;
  glutInit(&argc, argv);
  glutInitDisplayMode(MKSN_GLUT_INIT);
#ifdef RETINA
  glutInitWindowSize(1600, 1200);
#else
  glutInitWindowSize(1024, 768);
#endif
  glutCreateWindow("ObjView");

  glAlphaFunc(GL_GREATER, 0.2);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  cache_init();
  terminal_init();
  parser_init(argc, argv);

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboardFunc);
  glutSpecialFunc(specialKeyboardFunc);
  glutMouseFunc(mouseFunc);
  glutMotionFunc(motionFunc);
  glutMainLoop();

  return 0;
}
