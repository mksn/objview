#include "objview.h"
#include "unit.h"

static int leftstate = 0, rightstate = 0, middlestate = 0;
static int lastx;
static int lasty;
static float pitch = 38;
static float yaw   = 45;
static float dist = 5;
static float time = 0;
static int anim = ANIM_IDLE;

static float light_position[4] = { -1, 2, 2, 0 };

static struct ov_unit *dog = NULL;

void keyboardFunc(unsigned char key, int x, int y)
{
  if (key == 'i') anim = ANIM_IDLE;
  if (key == 'w') anim = ANIM_WALK;
  if (key == 0x1b) exit(1);
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
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60, (GLfloat)w / (GLfloat)h, 0.05, 7000.0);
}

void display()
{
  glClearColor (0.3, 0.3, 0.4, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable (GL_DEPTH_TEST);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glTranslatef(0, 0, -dist);
  glRotatef(pitch, 1.0, 0.0, 0.0);
  glRotatef(yaw, 0.0, 1.0, 0.0);
  glRotatef(-90, 1.0, 0.0, 0.0);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);

  ov_animate_model(dog->model, dog->animations[anim], time+=0.5);
  ov_draw_model(dog->model);

  glutSwapBuffers();
  glutPostRedisplay();

  GLenum e = glGetError();
  if (e != GL_NO_ERROR) fprintf (stderr, "display: e: %s\n", gluErrorString(e));
}

void init(int argc, char **argv)
{
  dog = ov_create_unit();
  dog->model = ov_load_model_iqe("tr_mo_chorani.iqe");
  dog->animations[ANIM_IDLE] = ov_load_animation_iqe("tr_mo_chien_idle.iqe");
  dog->animations[ANIM_WALK] = ov_load_animation_iqe("tr_mo_chien_marche.iqe");
}

int main (int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(MKSN_GLUT_INIT);
  glutInitWindowSize(1024, 768);
  glutCreateWindow("ObjView");

  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glAlphaFunc(GL_GREATER, 0.2);

  /*
   * Initiate the model with the argument, otherwise
   * just go with the default selection
   */
  init (argc, argv);

  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboardFunc);
  glutMouseFunc(mouseFunc);
  glutMotionFunc(motionFunc);
  glutMainLoop();

  return 0;
}
