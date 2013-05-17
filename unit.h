#ifndef unit_h
#define unit_h

enum {
  ANIM_IDLE,
  ANIM_WALK,
  MAXANIM
};

enum {
  MAXMESHES = 256,
  MAXBONES = 256
};

struct ov_vertex {
  float position[3];
  float normal[3];
  float texcoord[2];
  float blend_weight[4];
  float blend_index[4];
};

struct ov_anivertex {
  float position[3];
  float normal[3];
};

struct ov_pose {
  float position[3];
  float rotate[4];
  float scale[3];
};

struct ov_bone {
  char *name;
  int parent;
  struct ov_pose bind_pose;
  float inv_bind_matrix[16];
};

struct ov_mesh {
  int texture;
  int first;
  int count;
};

struct ov_skeleton {
  int num_bones;
  struct ov_bone bones[MAXBONES];
};

struct ov_model {
  int num_vertices;
  struct ov_vertex    *vertices;
  struct ov_anivertex *anivertices;
  struct ov_skeleton  *skeleton;

  int num_triangles;
  int *triangles;

  int num_meshes;
  struct ov_mesh meshes[MAXMESHES];
};

struct ov_animation {
  char *name;
  struct ov_skeleton *skeleton;

  int num_frames;
  struct ov_pose **frames;

};

struct ov_unit {
  struct ov_skeleton  *skeleton;
  struct ov_model     *model;
  struct ov_animation *animations[MAXANIM];
};

struct ov_unit *ov_unit_new(void);

void ov_load_iqe(const char *filename, struct ov_skeleton **skeletonp, struct ov_model **modelp, struct ov_animation **animationp);

struct ov_skeleton *ov_skeleton_load(const char *filename);
struct ov_model *ov_model_load(const char *filename);
struct ov_animation *ov_animation_load(const char *filename);

void ov_model_draw(struct ov_model *model);
void ov_model_animate(struct ov_model *model, struct ov_animation *anim, float frame);

#endif
