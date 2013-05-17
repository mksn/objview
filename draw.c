#include "objview.h"
#include "unit.h"
#include "vector.h"

static void pose_lerp(struct ov_pose *r, struct ov_pose *a, struct ov_pose *b, float t)
{
  vec_lerp(r->position, a->position, b->position, t);
  quat_lerp_neighbor_normalize(r->rotate, a->rotate, b->rotate, t);
  vec_lerp(r->scale, a->scale, b->scale, t);
}

static int find_bone(struct ov_animation *anim, char *name)
{
  int i;
  for (i = 0; i < anim->skeleton->num_bones; i++)
    if (!strcmp(anim->skeleton->bones[i].name, name))
      return i;
  return -1;
}

void
ov_animate_model(struct ov_model *model, struct ov_animation *anim, float frame_time)
{
  float skin_matrix[MAXBONES][16];
  float pose_matrix[MAXBONES][16];
  int frame0 = floor(frame_time);
  int frame1 = floor(frame_time + 1);
  float t = frame_time - floor(frame_time);
  int i, k, a;

  frame0 %= anim->num_frames;
  frame1 %= anim->num_frames;

  struct ov_pose *anim_frame_0 = anim->frames[frame0];
  struct ov_pose *anim_frame_1 = anim->frames[frame1];
  struct ov_pose pose[MAXBONES];

  for (i = 0; i < model->skeleton->num_bones; i++) {
    a = find_bone(anim, model->skeleton->bones[i].name);
    if (a >= 0)
      pose_lerp(&pose[i], &anim_frame_0[a], &anim_frame_1[a], t);
    else
      pose[i] = model->skeleton->bones[i].bind_pose;
  }

  for (i = 0; i < model->skeleton->num_bones; i++) {
    float m[16];
    mat_from_pose(m, pose[i].position, pose[i].rotate, pose[i].scale);
    if (model->skeleton->bones[i].parent != -1)
      mat_mul44(pose_matrix[i], pose_matrix[model->skeleton->bones[i].parent], m);
    else
      mat_copy(pose_matrix[i], m);
    mat_mul44(skin_matrix[i], pose_matrix[i], model->skeleton->bones[i].inv_bind_matrix);
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
}

void
ov_draw_model(struct ov_model *model)
{
  int i;

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  if (model->anivertices) {
    glVertexPointer(3, GL_FLOAT, sizeof(struct ov_anivertex), &model->anivertices->position[0]);
    glNormalPointer(GL_FLOAT, sizeof(struct ov_anivertex), &model->anivertices->normal[0]);
  } else {
    glVertexPointer(3, GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->position[0]);
    glNormalPointer(GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->normal[0]);
  }

  glTexCoordPointer(2, GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->texcoord[0]);

  for (i = 0; i < model->num_meshes; i++) {
    glBindTexture(GL_TEXTURE_2D, model->meshes[i].texture);
    glDrawElements(GL_TRIANGLES, model->meshes[i].count, GL_UNSIGNED_INT, &model->triangles[model->meshes[i].first]);
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}
