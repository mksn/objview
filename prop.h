#ifndef PROP_H
#define PROP_H

struct ov_prop {
  struct ov_model *model;
  float            position[3];
  float            rotation;
};

struct ov_prop *ov_prop_new(const char *filename);

void ov_prop_draw(struct ov_prop *unit);

#endif
