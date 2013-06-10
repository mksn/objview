#include "objview.h"
#include "unit.h"
#include "terminal.h"

#define ANIM_FPS 30

static int leftstate = 0, rightstate = 0, middlestate = 0;
static int lastx;
static int lasty;
static float pitch = 38;
static float yaw   = 45;
static float dist = 5;
static float time = 0;
static int anim = ANIM_IDLE;

static float swidth = 1;
static float sheight = 1;
static int terminal_input = 0;

static float light_position[4] = { -1, 2, 2, 0 };

static struct ov_unit *humon = NULL;

void keyboardFunc(unsigned char key, int x, int y)
{
  if (!terminal_input) {
    switch (key) {
      case 'i':
        anim = ANIM_IDLE;
        break;

      case 'w':
        anim = ANIM_WALK;
        break;

      case 'r':
        anim = ANIM_RUN;
        break;

      case 'a':
        anim = ANIM_TURN_LEFT;
        break;

      case 'd':
        anim = ANIM_TURN_RIGHT;
        break;

      case 'q':
        anim = ANIM_STRAFE_LEFT;
        break;

      case 'e':
        anim = ANIM_STRAFE_RIGHT;
        break;

      case 'z':
        anim = ANIM_DEATH;
        break;

      case 0x1b:
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
  int time_in_millis = glutGet(GLUT_ELAPSED_TIME);

  time = time_in_millis * ANIM_FPS * 0.001;

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
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glTranslatef(0, 0, -dist);
  glRotatef(pitch, 1.0, 0.0, 0.0);
  glRotatef(yaw, 0.0, 1.0, 0.0);
  glRotatef(-90, 1.0, 0.0, 0.0);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);

  //ov_unit_animate(dog, anim, time);
  //ov_unit_draw(dog);
  ov_unit_animate(humon, anim, time);
  ov_unit_draw(humon);

  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, swidth, sheight, 0, -1, 1);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  terminal_display();

  glutSwapBuffers();
  glutPostRedisplay();

  GLenum e = glGetError();
  if (e != GL_NO_ERROR) fprintf (stderr, "display: e: %s\n", gluErrorString(e));
}

void init(int argc, char **argv)
{
  /*
    dog = ov_unit_new();
    dog->model = ov_model_load("dog/tr_mo_chorani.iqe");
    dog->animations[ANIM_IDLE] = ov_animation_load("dog/tr_mo_chien_idle.iqe");
    dog->animations[ANIM_WALK] = ov_animation_load("dog/tr_mo_chien_marche.iqe");
  */

  humon = ov_unit_new();
  ov_unit_set_skeleton(humon, ov_skeleton_load("human/ge_hom_skel.iqe"));
  ov_unit_add_animation(humon, ov_animation_load("human/fy_hom_ab_marche.iqe"),
                        ANIM_WALK);
  ov_unit_add_animation(humon, ov_animation_load("human/fy_hom_ab_idle.iqe"),
                        ANIM_IDLE);
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_armor00_armpad.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_armor00_bottes.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_armor00_gilet.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_armor00_hand.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_armor00_pantabottes.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_cheveux_basic01.iqe"));
  ov_unit_add_skin_component(humon, ov_model_load("human/fy_hom_visage.iqe"));
  ov_unit_add_bone_component(humon, ov_model_load("human/fy_wea_dague.iqe"), "box_arme");
  ov_unit_add_bone_component(humon, ov_model_load("human/fy_wea_grand_bouclier.iqe"), "box_bouclier");
}

int main (int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(MKSN_GLUT_INIT);
  glutInitWindowSize(1024, 768);
  glutCreateWindow("ObjView");

 glAlphaFunc(GL_GREATER, 0.2);

  terminal_init();

  /*
   * Initiate the model with the argument, otherwise
   * just go with the default selection
   */
  init (argc, argv);

  terminal_puts("Mika was here!");
  terminal_puts("Kilroy... go home!");
  terminal_puts("Veni vidi vici!");

  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboardFunc);
  glutSpecialFunc(specialKeyboardFunc);
  glutMouseFunc(mouseFunc);
  glutMotionFunc(motionFunc);
  glutMainLoop();

  return 0;
}
