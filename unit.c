// vim: sw=2 sts=2 et

#include "objview.h"
#include "unit.h"

void
ov_unit_draw(struct ov_unit *unit)
{
  int i = 0;
  for (; i<unit->num_skin_components; i++) {
    ov_model_draw(unit->skin_components[i].model);
  }
}

void
ov_unit_animate(struct ov_unit *unit,
    int             anim,
    float           time)
{
  int i=0;

  for (;i<unit->num_skin_components;i++) {
    ov_model_animate(unit->skin_components[i].model,
        unit->animations[anim],
        time);
  }
}

void ov_unit_set_skeleton (struct ov_unit *unit,
    struct ov_skeleton *skeleton)
{
  unit->skeleton = skeleton;
}

void ov_unit_add_skin_component(struct ov_unit *unit,
    struct ov_model *model)
{
  unit->skin_components[unit->num_skin_components++].model = model;
}

void ov_unit_add_bone_component(struct ov_unit *unit,
    struct ov_model *model)
{
  return;
}

void ov_unit_add_animation(struct ov_unit *unit,
    struct ov_animation *animation,
    int animation_type)
{
  unit->animations[animation_type] = animation;
}

