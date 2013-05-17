// vim: sw=2 sts=2 et

#include "objview.h"
#include "unit.h"

static int find_bone(struct ov_skeleton *skel, char *name)
{
  int i;
  for (i = 0; i < skel->num_bones; i++)
    if (!strcmp(skel->bones[i].name, name))
      return i;
  return -1;
}

static void make_bone_map(int *bonemap, struct ov_skeleton *src, struct ov_skeleton *dst)
{
  int i;
  for (i = 0; i < src->num_bones; i++)
    bonemap[i] = find_bone(dst, src->bones[i].name);
}

void
ov_unit_draw(struct ov_unit *unit)
{
  int i = 0;
  for (i = 0; i<unit->num_skin_components; i++) {
    ov_model_draw(unit->skin_components[i].model);
  }
  for (i = 0; i<unit->num_bone_components; i++) {
    /*
     * GL magic to render the bone components in their correct
     * places
     *
     */
    glPushMatrix();
    int b = unit->bone_components[i].bone; 
    glMultMatrixf(unit->skeleton->bones[b].pose_matrix);
    /* except for this */
    ov_model_draw(unit->bone_components[i].model);
    glPopMatrix();
  }
}

void
ov_unit_animate(struct ov_unit *unit,
    int             anim,
    float           time)
{
  int i=0;

  ov_skeleton_animate(unit->skeleton, &unit->actions[anim], time);

  for (;i<unit->num_skin_components;i++) {
    ov_model_animate(&unit->skin_components[i], unit->skeleton);
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
  unit->skin_components[unit->num_skin_components].model = model;
  make_bone_map(unit->skin_components[unit->num_skin_components].bonemap,
    model->skeleton,
    unit->skeleton);
  unit->num_skin_components++;
}

void ov_unit_add_bone_component(struct ov_unit *unit,
    struct ov_model *model, char *bone)
{
  unit->bone_components[unit->num_bone_components].model = model;
  unit->bone_components[unit->num_bone_components].bone = find_bone(unit->skeleton, bone);
  unit->num_bone_components++;
  return;
}

void ov_unit_add_animation(struct ov_unit *unit,
    struct ov_animation *animation,
    int animation_type)
{
  unit->actions[animation_type].animation = animation;
  make_bone_map(unit->actions[animation_type].bonemap,
    unit->skeleton,
    animation->skeleton);
}
