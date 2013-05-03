#include "objview.h"
#include "unit.h"
#include "vector.h"

void
ov_draw_model(struct ov_model *model)
{
  int i;

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->position[0]); 
  glNormalPointer(GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->normal[0]); 
  glTexCoordPointer(2, GL_FLOAT, sizeof(struct ov_vertex), &model->vertices->texcoord[0]); 

  for (i = 0; i < model->num_meshes; i++) {
    glBindTexture(GL_TEXTURE_2D, model->meshes[i].texture);
    glDrawElements(GL_TRIANGLES, model->meshes[i].count, GL_UNSIGNED_INT, &model->triangles[model->meshes[i].first]);
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}