#include "objview.h"
#include "unit.h"
#include "vector.h"

int     root_motion_comp;
typedef float mat4[16];

void
ov_model_draw(struct ov_model *model)
{
  int i;

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  if (model->anivertices) {
    glVertexPointer(3, GL_FLOAT, sizeof(struct ov_anivertex),
        &model->anivertices->position[0]);
    glNormalPointer(GL_FLOAT, sizeof(struct ov_anivertex),
        &model->anivertices->normal[0]);
  } else {
    glVertexPointer(3, GL_FLOAT, sizeof(struct ov_vertex),
        &model->vertices->position[0]);
    glNormalPointer(GL_FLOAT, sizeof(struct ov_vertex),
        &model->vertices->normal[0]);
  }

  glTexCoordPointer(2, GL_FLOAT, sizeof(struct ov_vertex),
      &model->vertices->texcoord[0]);

  for (i = 0; i < model->num_meshes; i++) {
    glBindTexture(GL_TEXTURE_2D, model->meshes[i].texture);
    glDrawElements(GL_TRIANGLES, model->meshes[i].count,
        GL_UNSIGNED_INT, &model->triangles[model->meshes[i].first]);
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

static void pose_lerp(struct ov_pose *r,
    struct ov_pose *a,
    struct ov_pose *b,
    float t)
{
  vec_lerp(r->position, a->position, b->position, t);
  quat_lerp_neighbor_normalize(r->rotate, a->rotate, b->rotate, t);
  vec_lerp(r->scale, a->scale, b->scale, t);
}

static void
calc_root_motion(float root_motion[16], struct ov_animation *anim, float frame_time)
{
  struct ov_pose *start = anim->frames[0];
  struct ov_pose *stop  = anim->frames[anim->num_frames-1];
  struct ov_pose still;
  float  m0[16];
  float  m[16];
  float  t = frame_time / (anim->num_frames-1);

  vec_lerp (still.position, start->position, stop->position, t);
  quat_lerp (still.rotate, start->rotate, stop->rotate, t);
  vec_lerp (still.scale, start->scale, stop->scale, t);

  mat_from_pose (m0, start->position, start->rotate, start->scale);
  mat_from_pose (m, still.position, still.rotate, still.scale);

  mat_invert (m, m);
  mat_mul (root_motion, m0, m);
}

void
ov_skeleton_animate(struct ov_skeleton *skeleton,
                    struct ov_action *action,
                    float frame_time)
{
  struct ov_animation *animation = action->animation;
  int *bonemap = action->bonemap;

  int frame0 = floor(frame_time);
  int frame1 = floor(frame_time + 1);
  float t = frame_time - floor(frame_time);
  int i;

  if (frame0 < 0 || frame0 >= animation->num_frames || frame1 < 0 || frame1 >= animation->num_frames) {
    fprintf(stderr, "animation overflow! (%d, %d / %d)\n", frame0, frame1, animation->num_frames);
    return;
  }

  struct ov_pose *anim_frame_0 = animation->frames[frame0];
  struct ov_pose *anim_frame_1 = animation->frames[frame1];
  struct ov_pose pose[MAXBONES];

  for (i = 0; i < skeleton->num_bones; i++) {
    int a = bonemap[i];
    if (a >= 0)
      pose_lerp(&pose[i], &anim_frame_0[a], &anim_frame_1[a], t);
    else
      pose[i] = skeleton->bones[i].bind_pose;
  }

  mat4 root_motion;
  if (root_motion_comp) {
    calc_root_motion(root_motion, animation, frame_time);
  }

  for (i = 0; i < skeleton->num_bones; i++) {
    float m[16];
    mat_from_pose(m,
        pose[i].position,
        pose[i].rotate,
        pose[i].scale);
    if (skeleton->bones[i].parent != -1)
      mat_mul44(skeleton->bones[i].pose_matrix,
          skeleton->bones[skeleton->bones[i].parent].pose_matrix,
          m);

    else
    {
      if (root_motion_comp)
        mat_mul44(skeleton->bones[i].pose_matrix, root_motion, m);
      else
        mat_copy(skeleton->bones[i].pose_matrix, m);
    }
  }
}

void
ov_skin_component_draw(struct ov_skin_component *component,
    struct ov_skeleton *skeleton)
{
  struct ov_model *model = component->model;
  int *bonemap = component->bonemap;

  float skin_matrix[MAXBONES][16];
  int i, k;

  for (i = 0; i < model->skeleton->num_bones; i++) {
    int a = bonemap[i];
    assert(a != -1);
    mat_mul44(skin_matrix[i],
        skeleton->bones[a].pose_matrix,
        model->skeleton->bones[i].inv_bind_matrix);
  }

  if (!model->anivertices)
    model->anivertices = malloc(sizeof (struct ov_anivertex) * model->num_vertices);

  for (i = 0; i < model->num_vertices; i++) {
    float *position = model->vertices[i].position;
    float *normal = model->vertices[i].normal;
    float *aposition = model->anivertices[i].position;
    float *anormal = model->anivertices[i].normal;
    float tposition[3];
    float tnormal[3];

    aposition[0] = aposition[1] = aposition[2] = 0;
    anormal[0] = anormal[1] = anormal[2] = 0;

    for (k = 0; k < 4; k++) {
      int b = model->vertices[i].blend_index[k];
      float w = model->vertices[i].blend_weight[k];

      mat_vec_mul(tposition, skin_matrix[b], position);
      mat_vec_mul_n(tnormal, skin_matrix[b], normal);
      vec_scale(tposition, tposition, w);
      vec_scale(tnormal, tnormal, w);
      vec_add(aposition, aposition, tposition);
      vec_add(anormal, anormal, tnormal);
    }
  }

  ov_model_draw(model);
}

void ov_bone_component_draw(struct ov_bone_component *component,
    struct ov_skeleton *skeleton)
{
  /*
   * Concatenate the transform of the bone that the
   * component is attached to:
   *
   */
  glPushMatrix();
  glMultMatrixf(skeleton->bones[component->bone].pose_matrix);
  ov_model_draw(component->model);
  glPopMatrix();
}
