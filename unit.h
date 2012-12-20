#ifndef __MKSN_GRAPHICS_UNIT__
#define __MKSN_GRAPHICS_UNIT__

#include <model-iqm.h>

enum animation_t
{
    ANIM_IDLE,
    AMIN_WALKING,
    ANIM_RUNNING,
    ANIM_DEATH
};

struct ov_unit
{
    struct iqm_model *model;
    struct iqm_animation  *anim;
};

#endif
