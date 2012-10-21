#include <stdio.h>
#include <mksn-gl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <model-iqm.h>
#include <assert.h>

#define NUMTREES 2048
extern int compile_shader(char *vfile, char *ffile);

int prog;
struct iqm_model *model;
struct iqm_model *animation;
int draw_bones;

float lastx;
float lasty;

float xpos = 850;
float ypos = 120;
float zpos = 281;

float pitch = 758;
float yaw   = 45;

float dist = 5;
float zrot = 45;

int leftstate = 0, rightstate = 0, middlestate = 0;
int frame = 0;

void move_camera()
{
}

void camera()
{
  glLoadIdentity();
  glTranslatef(0, 0, -dist);;
  glRotatef(pitch, 1.0, 0.0, 0.0);
  glRotatef(yaw, 0.0, 1.0, 0.0);
  //  glTranslated(-xpos, -ypos, -zpos);
}

void init(int argc, char **argv)
{
  prog = compile_shader(("vertex.glsl"),
                        ("fragment.glsl"));
  if (argc>2) {
      animation = model_iqm_load(argv[2]);
      //animation = load_iqm_model(argv[2]);
  }
  if (argc>1) {
      model = model_iqm_load (argv[1]);
  } else {
      model = model_iqm_load ("gfx/tmp/tr_mo_c03_idle1.iqm");
  }
  //if (filename == NULL) model= load_iqm_model("gfx/tmp/tr_mo_c03_idle1.iqm");
  //else  model = load_iqm_model(filename);
  draw_bones = 0;
  assert(model != NULL);
}

void reshape(int w, int h)
{
  glViewport (0, 0, (GLsizei)w, (GLsizei)h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (60, (GLfloat)w / (GLfloat)h, 0.05, 7000.0);
	//gluPerspective (60, (GLfloat)1024 / (GLfloat)768, 0.1, 70000.0);
  glMatrixMode (GL_MODELVIEW);

}

void initExtensions()
{
}

void kbdup (unsigned char key, int x, int y)
{
  /*
    x = x;
    y = y;
    if (key == 'w') motion ^= MOTION_W;
    if (key == 'a') motion ^= MOTION_A;
    if (key == 's') motion ^= MOTION_S;
    if (key == 'd') motion ^= MOTION_D;
  */
}


void keyboard (unsigned char key, int x, int y)
{
    if (key == 'b') draw_bones = draw_bones?0:1;
  /*
    x = x;
    y = y;
    if (key == 'w')
    {
    motion |= MOTION_W;
    }

    if (key == 's')
    {
    motion |= MOTION_S;
    }

    if (key == 'd')
    {
    motion |= MOTION_D;
    }

    if (key == 'a')
    {
    motion |= MOTION_A;
    }

    if (key == 'b')
    {
    bounding_box = bounding_box == 0?1:0;
    }
    
    if (key == 'f') 
    { 
		wireframe = wireframe==0?1:0;

		printf ("lt: wireframe = %d\n", wireframe);
    }
	
    if (key == 'r') {
		if (speed == CAMERA_SPEED) { 
    speed = WALKING_SPEED;
    model_start_frame = 70;
    model_end_frame = 89;
		} else {
    speed = CAMERA_SPEED;
    model_start_frame = 94;
    model_start_frame = 101;
		}
    }
  */
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

void mouseMovement(int x, int y)
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


void display()
{
  int i;
  //animate_iqm_model(model, 0, frame++, 0);

  //GLenum error;
  glClearColor (0.3, 0.3, 0.4, 1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  glEnable(GL_DEPTH_TEST);
  
  camera();
  //glLightfv (GL_LIGHT0, GL_POSITION, lightpos);	

  glPushMatrix();
  glRotatef(-90, 1.0, 0.0, 0.0);

  glColor3f(1, 1, 1);
  glEnable(GL_TEXTURE_2D);
  glUseProgram(prog);
  //draw_iqm_model(model);
  //draw_static_iqm_model(model);
  model_iqm_draw_static (model);
  glUseProgram(0);
  glDisable(GL_TEXTURE_2D);

  if (draw_bones)
      model_iqm_draw_bones(model);

  glPopMatrix();

  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  for (i = -4; i <= 4; i++) {
    glVertex3f(-4, 0, i);
    glVertex3f(4, 0, i);
    glVertex3f(i, 0, -4);
    glVertex3f(i, 0, 4);
  }
  glEnd();
    
  glutSwapBuffers();
  glutPostRedisplay();

  GLenum e = glGetError();
  if (e != GL_NO_ERROR) fprintf (stderr, "display: e: %s\n", gluErrorString(e));
}

void
  cleanup()
{
}

void special (int key, int x , int y) {
	switch (key) {
		case GLUT_KEY_UP: 
			//dist -= 1; 
			//if (dist < 1) dist = 1;
			break;
		case GLUT_KEY_DOWN:
      //			dist += 1;
      //			if (dist > 50) dist = 50;
			break;
	};
	//x = x;
	//y = y;
}

int main (int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA |  GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(1024, 768);
  glutCreateWindow("ObjView");
#ifdef __APPLE__
  /* This is so that you can sync the animation vertically */
  int swap_interval = 1;
  CGLContextObj cgl_context = CGLGetCurrentContext();
  CGLSetParameter(cgl_context, kCGLCPSwapInterval, &swap_interval);
#endif

  /*
   * Initiate the model with the argument, otherwise
   * just go with the default selection
   *
   */
  init (argc, argv);
  glutDisplayFunc(display);
  glutIdleFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIgnoreKeyRepeat(1);
  glutKeyboardUpFunc(kbdup);
  glutMouseFunc(mouseFunc);
  glutSpecialFunc (special);
  glutMotionFunc(mouseMovement);
  glutMainLoop();
  return 0;
}
