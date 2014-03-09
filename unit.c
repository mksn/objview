#include "objview.h"
#include "unit.h"

/* NOTE: must match enums in unit.h */
const char *anim_name_list[MAXANIM+1] = {
  "IDLE",
  "WALK",
  "RUN",
  "TURN_LEFT",
  "TURN_RIGHT",
  "STRAFE_LEFT",
  "STRAFE_RIGHT",
  "DEATH",
  "JUMP",
  NULL
};

static int
find_bone(struct ov_skeleton *skel,
          const char *name)
{
  int i;
  for (i = 0; i < skel->num_bones; i++)
    if (!strcmp(skel->bones[i].name, name))
      return i;
  return -1;
}

static
void make_bone_map(int                *bonemap,
                   struct ov_skeleton *src,
                   struct ov_skeleton *dst)
{
  int i;
  for (i = 0; i < src->num_bones; i++)
    bonemap[i] = find_bone(dst, src->bones[i].name);
}

void
ov_unit_draw(struct ov_unit *unit)
{
  int i;
  /*
   * find out the transform for the parent bone and apply that for the starting
   * position
   *
   */
  glPushMatrix();
  if (unit->parent) {
    glTranslatef(unit->parent->position[0],
                 unit->parent->position[1],
                 unit->parent->position[2]);
    glRotatef(unit->parent->rotation, 0, 0, 1);
    glMultMatrixf(unit->parent->skeleton->bones[unit->parent_bone].pose_matrix);
    /*
    glTranslatef(unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[0*4+0],
                 unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[1*4+0],
                 unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[2*4+0]);

    glRotatef(
        unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[0*4+1],
        unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[1*4+1],
        unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[2*4+1],
        unit->parent->skeleton->bones[unit->parent_bone].pose_matrix[3*4+1]);
        */
  } else {
    glTranslatef(unit->position[0], unit->position[1], unit->position[2]);
    glRotatef(unit->rotation, 0, 0, 1);
  }
  for (i = 0; i<unit->num_skin_components; i++) {
    ov_skin_component_draw(&unit->skin_components[i],
                           unit->skeleton);
  }
  for (i = 0; i<unit->num_bone_components; i++) {
    ov_bone_component_draw(&unit->bone_components[i],
                           unit->skeleton);
  }
  glPopMatrix();
}

void
ov_unit_animate(struct ov_unit *unit,
                int             anim,
                int             next_anim,
                float           time,
                float           next_time,
                float           bfac)
{
  // Fall back to idle
  if (unit->actions[anim].animation == 0)
    anim = ANIM_IDLE;
  ov_skeleton_animate(unit->skeleton,
                      &unit->actions[anim],
                      &unit->actions[next_anim],
                      time * ANIM_FPS,
                      next_time * ANIM_FPS,
                      bfac);
}

void
ov_unit_set_skeleton (struct ov_unit     *unit,
                      struct ov_skeleton *skeleton)
{
  unit->skeleton = malloc(sizeof *skeleton);
  memcpy(unit->skeleton,
         skeleton,
         sizeof *skeleton);
}

void
ov_unit_add_skin_component(struct ov_unit  *unit,
                           struct ov_model *model)
{
  unit->skin_components[unit->num_skin_components].model = model;
  make_bone_map(unit->skin_components[unit->num_skin_components].bonemap,
                model->skeleton,
                unit->skeleton);
  unit->num_skin_components++;
}

void
ov_unit_add_bone_component(struct ov_unit  *unit,
                           struct ov_model *model,
                           const char      *bone)
{
  unit->bone_components[unit->num_bone_components].model = model;
  unit->bone_components[unit->num_bone_components].bone = find_bone(unit->skeleton, bone);
  unit->num_bone_components++;
  return;
}

void
ov_unit_add_animation(struct ov_unit      *unit,
                      struct ov_animation *animation,
                      int                  animation_type)
{
  unit->actions[animation_type].animation = animation;
  make_bone_map(unit->actions[animation_type].bonemap,
                unit->skeleton,
                animation->skeleton);
}

void
ov_unit_attach_model(struct ov_unit *unit,
                     struct ov_unit *parent,
                     const char     *bone)
{
  unit->parent = parent;
  unit->parent_bone = find_bone(parent->skeleton, bone);
}
