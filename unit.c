#include "objview.h"
#include "unit.h"

void
ov_unit_draw(struct ov_unit *unit)
{
  ov_model_draw(unit->model);
}

void
ov_unit_animate(struct ov_unit *unit,
                int             anim,
                float           time)
{
  ov_model_animate(unit->model,
                   unit->animations[anim],
                   time);
}
