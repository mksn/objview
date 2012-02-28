#ifndef __MODEL_IQM_H__
#define __MODEL_IQM_H__
  
#define noFLIP
#define NAMELEN 80

struct iqm_material {
	char name[NAMELEN];
	int texture;
};

struct iqm_pose {
	float translate[3];
	float rotate[4];
	float scale[3];
};

struct iqm_bone {
	char name[NAMELEN];
	int parent;
	struct iqm_pose bind_pose;
	float bind_matrix[16];
	float inv_bind_matrix[16];
};

struct iqm_bounds {
	float min[3], max[3];
	float xyradius, radius;
};

struct iqm_anim {
	char name[NAMELEN];
	int first, count;
	float rate;
	int loop;
};

struct iqm_mesh {
	char name[NAMELEN];
	int material; // struct iqm_material *material;
	int first, count;
};

struct iqm_model {
	char dir[NAMELEN];
	int num_verts, num_tris, num_meshes, num_bones, num_frames, num_anims;
	float *pos, *norm, *texcoord;
	unsigned char *blend_index, *blend_weight, *color;
	int *tris;
	struct iqm_mesh *meshes;
	struct iqm_bone *bones;
	struct iqm_bounds *bounds; // bound for each frame
	struct iqm_pose **poses; // poses for each frame
	struct iqm_anim *anims;

	float min[3], max[3], radius;

	float (*outbone)[16];
	float (*outskin)[16];
	struct iqm_pose *outpose;

	unsigned int vbo, ibo;
};

#define IQM_MAGIC "INTERQUAKEMODEL\0"
#define IQM_VERSION 2

enum {
	IQM_POSITION = 0,
	IQM_TEXCOORD = 1,
	IQM_NORMAL = 2,
	IQM_TANGENT = 3,
	IQM_BLENDINDEXES = 4,
	IQM_BLENDWEIGHTS = 5,
	IQM_COLOR = 6,
	IQM_CUSTOM = 0x10
};

enum {
	IQM_BYTE = 0,
	IQM_UBYTE = 1,
	IQM_SHORT = 2,
	IQM_USHORT = 3,
	IQM_INT = 4,
	IQM_UINT = 5,
	IQM_HALF = 6,
	IQM_FLOAT = 7,
	IQM_DOUBLE = 8,
};

struct iqm_model *load_iqm_model_from_memory(unsigned char *data, char *filename);
struct iqm_model *load_iqm_model(char *filename);
float measure_iqm_radius(struct iqm_model *model);
char *get_iqm_animation_name(struct iqm_model *model, int anim);
void animate_iqm_model(struct iqm_model *model, int anim, int frame, float t);
void draw_iqm_instances(struct iqm_model *model, float *trafo, int count);
void draw_iqm_model(struct iqm_model *model);
void draw_iqm_bones(struct iqm_model *model);

#endif
