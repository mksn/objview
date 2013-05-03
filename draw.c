#include "objview.h"
#include "unit.h"
#include "vector.h"

static int find_bone(struct ov_animation *anim, char *name)
{
	int i;
	for (i = 0; i < anim->num_bones; i++)
		if (!strcmp(anim->bones[i].name, name))
			return i;
	return -1;
}

void
ov_animate_model(struct ov_model *model, struct ov_animation *anim, int frame_index)
{
	float skin_matrix[MAXBONES][16];
	float pose_matrix[MAXBONES][16];
	int i, a;

	frame_index %= anim->num_frames;

	struct ov_pose *anim_frame = anim->frames[frame_index];
	struct ov_pose pose[MAXBONES];

	for (i = 0; i < model->num_bones; i++) {
		a = find_bone(anim, model->bones[i].name);
		if (a >= 0)
			pose[i] = anim_frame[a];
		else
			pose[i] = model->bones[i].bind_pose;
	}

	for (i = 0; i < model->num_bones; i++) {
		float m[16];
		mat_from_pose(m, pose[i].position, pose[i].rotate, pose[i].scale);
		if (model->bones[i].parent != -1) 
			mat_mul44(pose_matrix[i], pose_matrix[model->bones[i].parent], m);
		else
			mat_copy(pose_matrix[i], m);
		mat_mul44(skin_matrix[i], pose_matrix[i], model->bones[i].inv_bind_matrix);
	}

	if (!model->anivertices)
		model->anivertices = malloc(sizeof (struct ov_anivertex) * model->num_vertices);

	for (i = 0; i < model->num_vertices; i++) {
		float *position = model->vertices[i].position;
		float *normal = model->vertices[i].normal;
		float *aposition = model->anivertices[i].position;
		float *anormal = model->anivertices[i].normal;

		int b = model->vertices[i].blend_index[0];

		mat_vec_mul(aposition, skin_matrix[b], position);
		mat_vec_mul_n(anormal, skin_matrix[b], normal);
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
