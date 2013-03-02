#ifndef __MKSN_GRAPHICS_UNIT__
#define __MKSN_GRAPHICS_UNIT__

#include <model-iqm.h>

typedef enum 
{
  ANIM_IDLE,
  AMIN_WALKING,
  ANIM_RUNNING,
  ANIM_DEATH,
  ANIM_T_SIZE // always always aaalways prepend values to thiso
} animation_t;

struct ov_model;
struct ov_animation;
struct ov_skin;

struct ov_anim_txlate {
  int key;
  int anim;
};

struct ov_pose {
  float translate[3];
  float rotate[4];
  float scale[3];
};

struct ov_bone {
  char *name;
  int   parent;
  struct ov_pose bind_pose;
  float  bind_matrix[16];
  float  diff[16];
  float  inv_bind_matrix[16];
  float  abs_bind_matrix[16];
  float  anim_matrix[16];
};

struct ov_skeleton {
  int num_bones;
  struct ov_bone *bones;
};

struct ov_animation {
  char                *dir;
  char                *name;
  int                  first;
  int                  count;
  int                  cap;
  float                rate;
  int                  loop;
  struct ov_skeleton  *skeleton;
  struct ov_pose     **poses;
  int                  num_frames;
};

struct ov_animations {
  struct ov_anim_txlate **at_table;
  int                     num_anims;
  struct ov_animation   **anims;
};

struct ov_mesh {
  char  *name;
  int   material;
  int   first;
  int   count;
  int   first_vert;
  int   num_vertex;
  float *position;
  float *normal;
  float *texcoord;
  float *color;
  int   element_count;
  int   *element;
  int *blendindex;
  int *blendweight;
  float *aposition;
  float *anormal;
};

struct ov_skin {
  struct ov_model      *model;
  char                 *attachement_bone;
};

struct ov_model {
  char                *dir;
  int                  num_triangles;
  int                  num_vertices;
  int                  num_meshes;
  struct ov_skeleton  *skeleton;
  struct ov_mesh      *meshes;

  float               *pos;
  float               *norm;
  float               *texcoord;

  int                 *triangles;
  unsigned char       *blend_index;
  unsigned char       *blend_weight;
  struct ov_bounds    *bounds;
  float               (*outbone)[16];
  float               (*outskin)[16];
  struct iqm_pose     *outpose;
  float               *dpos;
  float               *dnorm;

  float                min[3];
  float                max[3];
  float                radius;
};

struct ov_unit
{
  struct ov_model       *model;
  struct ov_animations  *animations;
  struct ov_skin        *skins;
};

#if 0
struct ov_animation {
  struct iqm_skeleton   *skeleton;
  struct iqm_pose      **poses;
  char                  *name;
  int                    first;
  int                    count;
  struct ov_anim_txlate **at_table;
};
#endif

struct ov_unit *ov_create_unit      ();
int             ov_get_number_anims (struct ov_unit *unit);
int             ov_add_animation    (struct ov_unit *unit,
                                     char           *animation_fname,
                                     animation_t     animation_key);
int             ov_set_model        (struct ov_unit   *unit,
                                     char             *model_fname);
void            ov_draw_static      (struct ov_unit *du);
void            ov_draw_bones       (struct ov_unit *du);
void            ov_draw_anim_bones  (struct ov_unit *du);
void            ov_animate          (struct ov_unit *du,
                                     int a,
                                     int f,
                                     float t);

#endif
