#ifndef __MKSN_OBJVIEW_MODEL_IQM__
#define __MKSN_OBJVIEW_MODEL_IQM__

#include <mksn-gl.h>

// all vertex array entries must ordered as defined below, if present
// i.e. position comes before normal comes before ... comes before custom
// where a format and size is given, this means models intended
// for portable use should use these
// an IQM implementation is not required to honor any other
// format/size than those recommended
// however, it may support other format/size combinations for
// these types if it desires
enum // vertex array type
{
  IQM_VAT_POSITION     = 0,  // float, 3
  IQM_VAT_TEXCOORD     = 1,  // float, 2
  IQM_VAT_NORMAL       = 2,  // float, 3
  IQM_VAT_TANGENT      = 3,  // float, 4
  IQM_VAT_BLENDINDEXES = 4,  // ubyte, 4
  IQM_VAT_BLENDWEIGHTS = 5,  // ubyte, 4
  IQM_VAT_COLOR        = 6,  // ubyte, 4

  // all values up to CUSTOM are reserved for future use
  // any value >= CUSTOM is interpreted as CUSTOM type
  // the value then defines an offset into the string table,
  // where offset = value - CUSTOM
  // this must be a valid string naming the type
  IQM_VAT_CUSTOM       = 0x10
};

enum // vertex array format
{
  IQM_VAF_BYTE   = 0,
  IQM_VAF_UBYTE  = 1,
  IQM_VAF_SHORT  = 2,
  IQM_VAF_USHORT = 3,
  IQM_VAF_INT    = 4,
  IQM_VAF_UINT   = 5,
  IQM_VAF_HALF   = 6,
  IQM_VAF_FLOAT  = 7,
  IQM_VAF_DOUBLE = 8,
};

/*
 * Types for actually representing the data
 *
 */
struct iqm_pose {
    float translate[3];
    float rotate[4];
    float scale[3];
};

struct iqm_bone {
    char *name;
    int parent;
    struct iqm_pose bind_pose;
    float bind_matrix[16];
    float diff[16];
    float inv_bind_matrix[16];
    float anim_matrix[16];
};

struct iqm_material {
    char *name;
    int texture;
};

struct iqm_bounds {
    float min[3];
    float max[3];
    float xyradius;
    float radius;
};

struct iqm_skeleton {
    int num_bones;
    struct iqm_bone *bones;
};

struct iqm_anim {
    char                *name;
    int                  first;
    int                  count;
    float                rate;
    int                  loop;
};

struct iqm_animation
{
    char                *dir;
    int                  num_anims;
    int                  num_frames;
    struct iqm_pose    **poses;
    struct iqm_anim     *anims;
    struct iqm_skeleton *skeleton;
};

struct iqm_mesh {
    char                *name;
    int                  material;
    int                  first;
    int                  count;
};

struct iqm_model {
    char                *dir;
    int                  num_vertices;
    int                  num_triangles;
    int                  num_meshes;
    int                  num_anims;
    
    float               *pos;
    float               *norm;
    float               *texcoord;
    
    unsigned char       *blend_index;
    unsigned char       *blend_weight;
    
    int *triangles;

    struct iqm_skeleton *skeleton;
    struct iqm_mesh     *meshes;
    struct iqm_bounds   *bounds;

    /*
     * The current distorted information
     * for the current animation frame
     *
     */
    float               *dpos;
    float               *dnorm;
    
    float                min[3]; 
    float                max[3];
    float                radius;
    
    float              (*outbone)[16];
    float              (*outskin)[16];
    struct              iqm_pose *outpose;
    
    unsigned int        vbo;
    unsigned int        ibo;
};

/*
 * public functions
 *
 */
struct iqm_model *model_iqm_load        (char   *model_fname);
/* void              animate_iqm_model     (struct iqm_model *m, */
/*                                          int anim, */
/*                                          int frame, */
/*                                          float t); */
//void              draw_iqm_model        (struct iqm_model *m);
void              model_iqm_draw_static (struct iqm_model *m);
void              model_iqm_draw_bones  (struct iqm_model *m);
void              model_iqm_draw_anim_bones  (struct iqm_model *m);
void              model_iqm_animate     (struct iqm_model *m,
                                         struct iqm_animation *a,
                                         int anim,
                                         int frame,
                                         float t);
void              model_iqm_draw        (struct iqm_model *m);
#endif
