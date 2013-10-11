#ifndef UNIT_H
#define UNIT_H

#define ANIM_FPS 30

/* NOTE: must match list in unit.c */
enum {
  ANIM_IDLE,
  ANIM_WALK,
  ANIM_RUN,
  ANIM_TURN_LEFT,
  ANIM_TURN_RIGHT,
  ANIM_STRAFE_LEFT,
  ANIM_STRAFE_RIGHT,
  ANIM_DEATH,
  MAXANIM
};

extern const char *anim_name_list[];

enum {
  MAXMESHES = 256,
  MAXBONES = 256,
  MAXCOMPONENTS = 16
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
  float pose_matrix[16];
  float inv_bind_matrix[16];
};

struct ov_mesh {
  int texture;
  int first;
  int count;
};

struct ov_skeleton {
  char *name;
  int num_bones;
  struct ov_bone bones[MAXBONES];
};

struct ov_model {
  char *name;
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
  char  *name;
  struct ov_skeleton *skeleton;

  float  duration;
  int    num_frames;
  struct ov_pose **frames;
};

struct ov_bone_component {
    struct ov_model *model;
    int bone;
};

struct ov_skin_component {
  struct ov_model *model;
  char *name;
  int bonemap[MAXBONES];
};

struct ov_action {
  struct ov_animation *animation;
  int bonemap[MAXBONES];
};

struct ov_unit {
  struct ov_skeleton       *skeleton;
  struct ov_action         actions[MAXANIM];
  struct ov_skin_component skin_components[MAXCOMPONENTS];
  int                      num_skin_components;
  struct ov_bone_component bone_components[MAXCOMPONENTS];
  int                      num_bone_components;
  float                    position[3];
  float                    rotation;
};

/*
 * Allocation
 *
 */
struct ov_unit *ov_unit_new(void);

/*
 * Loading
 *
 */
void ov_load_iqe(const char *filename,
        struct ov_skeleton **skeletonp,
        struct ov_model **modelp,
        struct ov_animation **animationp);

struct ov_skeleton *ov_skeleton_load(const char *filename);
struct ov_model *ov_model_load(const char *filename);
struct ov_animation *ov_animation_load(const char *filename);

/*
 * Drawing
 *
 */

void ov_skeleton_animate(struct ov_skeleton *skeleton,
    struct ov_action *action,
    float frame_time);
void ov_skin_component_draw(struct ov_skin_component *component,
    struct ov_skeleton *skeleton);
void ov_bone_component_draw(struct ov_bone_component *component,
    struct ov_skeleton *skeleton);

void ov_unit_draw(struct ov_unit *unit);
void ov_unit_animate(struct ov_unit *unit,
    int anim,
    float frame);

/*
 * Manipulation
 *
 */
void ov_unit_set_skeleton (struct ov_unit *unit,
        struct ov_skeleton *skeleton);
void ov_unit_add_skin_component(struct ov_unit *unit,
        struct ov_model *model);
void ov_unit_add_bone_component(struct ov_unit *unit,
        struct ov_model *model,
        const char *bone);
void ov_unit_add_animation(struct ov_unit *unit,
        struct ov_animation *animation,
        int animation_type);
#endif
