#include <mksn-gl.h>
#include <unit.h>
#include <model-iqm.h>

struct iqm_animations *
ov_create_unit_animations ()
{
    struct iqm_animations *rc = malloc (sizeof(struct iqm_animations));
    rc->num_anims = 0;
    rc->at_table  = NULL;
    rc->anims     = NULL;
    return rc;
}   

struct ov_unit *
ov_create_unit ()
{
    struct ov_unit *rc = malloc (sizeof(struct ov_unit));
    rc->animations = malloc (sizeof(struct iqm_animations));
    rc->animations->at_table = malloc(sizeof(struct ov_anim_txlate )*ANIM_T_SIZE);
    memset(rc->animations->at_table, 0, ANIM_T_SIZE);
    rc->animations->anims = malloc (sizeof (struct iqm_animation *) * ANIM_T_SIZE);
    memset(rc->animations->anims, 0, ANIM_T_SIZE);
    rc->animations->num_anims = 0;
    return rc;
}

void
ov_set_model(struct ov_unit *unit,
             char           *model_fname)
{
    unit->model = model_iqm_load_model (model_fname);
}

    
int
ov_get_number_anims(struct ov_unit *unit)
{
    return unit->animations->num_anims;
}


void
ov_add_animation(struct ov_unit *unit,
                 char           *animation_fname,
                 animation_t     key)
{
    struct iqm_animation *a = model_iqm_load_animation (animation_fname);
    if (a != NULL) {
        if (unit->animations->anims[key] != NULL) {
            free (unit->animations->anims[key]);
        } 
        unit->animations->at_table[key] = malloc(sizeof(struct ov_anim_txlate));
        unit->animations->at_table[key]->key  = key;
        unit->animations->at_table[key]->anim = unit->animations->num_anims;
        unit->animations->anims[unit->animations->num_anims++] = a;
    }
}

