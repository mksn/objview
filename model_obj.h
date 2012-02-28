#ifndef _MKSN_LODTEST_MODEL_OBJ_H_
#define _MKSN_LODTEST_MODEL_OBJ_H_

#define SEP " \t\r\n"

struct material {
	char name[80];
	int texture;
	struct material *next;
};

struct triangle {
	int vp[3];
	int vt[3];
	int vn[3];
};

struct array {
	int len, cap;
	float *data;
};

struct mesh {
	struct material *material;
	int len, cap;
	struct triangle *tri;
	struct mesh *next;
	float *vba;
	unsigned int vbo;
};

struct model {
	struct material *material;
	struct array position;
	struct array texcoord;
	struct array normal;
	struct mesh *mesh;
	float min[3], max[3], radius;
};

extern void draw_obj_model (struct model *model, float x, float y, float z);
extern struct model *load_obj_model(char *filename);
extern struct mesh *find_mesh(struct model *model, char *matname);
extern struct material *find_material(struct model *model, char *matname);
//extern aabb_t *get_obj_bbox (struct model *model);
extern void draw_obj_instances(struct model *model, float *t, int count);
#endif
