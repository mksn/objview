#ifndef __MKSN_GRAPHICS_MODEL_IQE_H__
#define __MKSN_GRAPHICS_MODEL_IQE_H__

#include <unit.h>

struct ov_model *model_iqe_load_model(char *filename);
struct ov_animation *model_iqe_load_animation(struct ov_unit *du,
                                              char *filename);

#endif
