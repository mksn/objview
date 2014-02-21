#include "objview.h"
#include "unit.h"
#include "prop.h"

struct ov_prop *
ov_prop_new(const char *filename)
{
  struct ov_prop *prop = malloc(sizeof *prop);
  prop->model = ov_model_load(filename);
  prop->position[0] = 0;
  prop->position[0] = 0;
  prop->position[0] = 0;
  prop->rotation = 0;
  return prop;
}

void
ov_prop_draw(struct ov_prop *prop)
{
  glPushMatrix();
  glTranslatef(prop->position[0], prop->position[1], prop->position[2]);
  glRotatef(prop->rotation, 0, 0, 1);
  ov_model_draw(prop->model);
  glPopMatrix();
}
