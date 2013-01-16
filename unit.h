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
struct ov_unit
{
    struct iqm_model       *model;
    struct iqm_animations  *animations;
    /*
     * struct ov_model       *mdl;
     * struct ov_animation  **anims;
     * struct ov_skin        **skins;
     */
};

struct ov_animation {
    struct iqm_skeleton   *skeleton;
    struct iqm_pose      **poses;
    char                  *name;
    int                    first;
    int                    count;
    struct ov_anim_txlate **at_table;
};

struct ov_skin {
    struct iqm_model      *model;
    char                  *attachement_bone;
};

struct ov_model {
    char                *dir;
    int                  num_vertices;
    int                  num_meshes;
    struct iqm_skeleton *skeleton;

    float               *pos;
    float               *norm;
    float               *texcoord;

    struct iqm_mesh     *mesh;
    struct iqm_bounds   *bounds;
};

struct ov_unit *ov_create_unit();
int             ov_get_number_anims(struct ov_unit *unit);
void            ov_add_animation(struct ov_unit *unit,
                                 char           *animation_fname,
                                 animation_t     animation_key);
void            ov_set_model(struct ov_unit   *unit,
                             char             *model_fname);
#endif
