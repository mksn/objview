#include <model-iqm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <image.h>
#include <vector.h>

typedef float vec3[3];
/* 
 * Types for loading the data from any medium specified 
 *
 */
struct _iqm_header 
{ 
    char magic[16]; // the string "INTERQUAKEMODEL\0", 0 terminated 
    uint version; // must be version 2 
    uint filesize; 
    uint flags; 
    uint num_text, ofs_text; 
    uint num_meshes, ofs_meshes;
    uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
    uint num_triangles, ofs_triangles, ofs_adjacency;
    uint num_joints, ofs_joints;
    uint num_poses, ofs_poses;
    uint num_anims, ofs_anims;
    uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint num_comment, ofs_comment;
    uint num_extensions, ofs_extensions; // these are stored as a
                                         // linked list, not as a
                                         // contiguous array
};

struct _iqm_mesh
{
    uint name;     // unique name for the mesh, if desired
    uint material; // set to a name of a non-unique material or texture
    uint first_vertex, num_vertexes;
    uint first_triangle, num_triangles;
};

struct _iqm_vertexarray
{
    uint type;   // type or custom name
    uint flags;
    uint format; // component format
    uint size;   // number of components
    uint offset; // offset to array of tightly packed components,
                 // with num_vertexes * size total entries
                 // offset must be aligned to max(sizeof(format), 4)
};

struct _iqm_triangle
{
    uint vertex[3];
};

struct _iqm_adjacency
{
    uint triangle[3];
};

struct _iqm_joint
{
    uint name;
    int parent; // parent < 0 means this is a root bone
    float translate[3], rotate[4], scale[3]; 
    // translate is translation <Tx, Ty, Tz>, and rotate is
    // quaternion rotation <Qx, Qy, Qz, Qw>
    // rotation is in relative/parent local space
    // scale is pre-scaling <Sx, Sy, Sz>
    // output = (input*scale)*rotation + translation
};

struct _iqm_pose
{
    int parent; // parent < 0 means this is a root bone
    uint channelmask; // mask of which 10 channels are present for this joint pose
    float channeloffset[10], channelscale[10]; 
    // channels 0..2 are translation <Tx, Ty, Tz> and
    // channels 3..6 are quaternion rotation <Qx, Qy, Qz, Qw>
    // rotation is in relative/parent local space
    // channels 7..9 are scale <Sx, Sy, Sz>
    // output = (input*scale)*rotation + translation
};

//ushort frames[]; // frames is a big unsigned short array where each group of framechannels components is one frame

struct _iqm_anim
{
    uint name;
    uint first_frame, num_frames; 
    float framerate;
    uint flags;
};

enum
{
    ANIM_LOOP = 1<<0
};

struct _iqm_bounds
{
    float bbmins[3], bbmaxs[3]; // the minimum and maximum coordinates of the bounding box for this animation frame
    float xyradius, radius; // the circular radius in the X-Y plane, as well as the spherical radius
};

//char text[]; // big array of all strings,
               // each individual string being 0 terminated,
               //with the first string always being the empty
               //string "" (i.e. text[0] == 0)

//char comment[];

struct _iqm_extension
{
    uint name;
    uint num_data, ofs_data;
    uint ofs_extensions; // pointer to next extension
};

// vertex data is not really interleaved, but this just gives
// examples of standard types of the data arrays
struct _iqm_vertex
{
    float position[3], texcoord[2], normal[3], tangent[4];
    uchar blendindices[4], blendweights[4], color[4];
};

#define IQM_MAGIC "INTERQUAKEMODEL\0"
#define IQM_VERSION 2

int prog; 

/*
 * Private functions
 */

/*
 * Read a 32 bit integer from a stream of unsigned chars
 *
 * @param data - a stream of unsigned chars
 * @return a 32 bit integer value
 *
 */
static inline int
_read32(unsigned char *data)
{
    return data[0] | data[1]<<8 | data[2] << 16 | data[3] << 24;
}

static inline unsigned short
_read16(unsigned char *data)
{
	return data[0] | data[1] << 8;
}

static inline float
_readfloat(unsigned char *data)
{
	union { float f; int i; } u;
	u.i = data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
	return u.f;
}

/*
 * Load an array of floating point numbers from a stream of unsigned chars
 *
 * @param data - stream of unsigned chars
 * @param size - the size of each float
 * @param count - how many floats that should be read
 *
 */
static float *
_load_float_array(unsigned char *data,
                  int size,
                  int count)
{
    float *rc = malloc (size * count * sizeof (float));
    int i, n= count * size;
    union { float f; int i;} u;
    for (i=0; i<n; i++) {
        u.i = data[0] | data[1]<<8 | data[2]<<16 | data[3] << 24;
        rc[i] = u.f;
        data += 4;
    }
    return rc;
}

static unsigned char *
_load_ubyte_array(unsigned char *data,
                  int size,
                  int count)
{
	unsigned char *rc = malloc(size * count);
	memcpy(rc, data, size * count);
	return rc;
}

static void
_load_vertex_arrays(struct iqm_model *m,
                    unsigned char *data,
                    struct _iqm_header *h)
{
    int i;
    struct _iqm_vertexarray *va; //= malloc (sizeof (struct _iqm_vertexarray));
    m->num_vertices = h->num_vertexes;
    for (i=0; i<h->num_vertexarrays; i++) {
        /* memcpy (va, data+h->ofs_vertexarrays+(i*sizeof (struct _iqm_vertexarray)), */
        /*         sizeof (struct _iqm_vertexarray)); */
        va = (struct _iqm_vertexarray *)(data+h->ofs_vertexarrays+(i*sizeof (struct _iqm_vertexarray)));
        switch (va->type) {
            case IQM_VAT_POSITION:
                assert(va->format == IQM_VAF_FLOAT && va->size == 3);
                m->pos = _load_float_array(data + va->offset, va->size, h->num_vertexes);
                break;
            case IQM_VAT_TEXCOORD:
                assert(va->format == IQM_VAF_FLOAT && va->size == 2);
                m->texcoord = _load_float_array(data + va->offset, va->size, h->num_vertexes);
                break;
            case IQM_VAT_NORMAL:
                assert(va->format == IQM_VAF_FLOAT && va->size == 3);
                m->norm = _load_float_array(data + va->offset, va->size, h->num_vertexes);
                break;
            case IQM_VAT_BLENDINDEXES:
                assert(va->format == IQM_VAF_UBYTE && va->size == 4);
                m->blend_index = _load_ubyte_array (data + va->offset, va->size, h->num_vertexes);
                break;
            case IQM_VAT_BLENDWEIGHTS:
                assert(va->format == IQM_VAF_UBYTE && va->size == 4);
                m->blend_weight = _load_ubyte_array (data + va->offset, va->size, h->num_vertexes);
                break;
		}
    }
    //free (va);
}

static void
_load_triangles (struct iqm_model *m,
                 unsigned char *data,
                 struct _iqm_header *h)
{
    int size = h->num_triangles * 3 * (sizeof (int));
    m->num_triangles = h->num_triangles;
    m->triangles = malloc (size);
    memcpy (m->triangles, data + h->ofs_triangles, size);
}

static int
_load_material (struct iqm_model *m,
                char *mat_name)
{
    /*
     * TODO: Improve how the materials are searched for
     */
    char buf[256];
    memset(buf, 0, 256);
    mat_name = strchr(mat_name, '+') + 1;
    strlcpy(buf, m->dir, sizeof buf);
    strlcat(buf, mat_name, sizeof buf);
    strlcat(buf,".png", sizeof buf);
    return load_texture(0, buf);
}

static void
_load_meshes (struct iqm_model *m,
              unsigned char *data,
              struct _iqm_header *h)
{
    int i;
    m->num_meshes = h->num_meshes;
    m->meshes = malloc (h->num_meshes * sizeof (struct iqm_mesh));
    struct _iqm_mesh *q = malloc(h->num_meshes * sizeof(struct _iqm_mesh));
    memcpy(q, data + h->ofs_meshes, h->num_meshes * sizeof(struct _iqm_mesh));
    for (i=0; i<h->num_meshes; i++) {
        char *c = (char *)(data + h->ofs_text + q[i].name);
        m->meshes[i].first = q[i].first_triangle;
        m->meshes[i].count = q[i].num_triangles;
        m->meshes[i].name = malloc(strlen(c) * sizeof(char));
        strlcpy(m->meshes[i].name, c, strlen(c));
        char *material = (char *)(data + h->ofs_text + q[i].material);
        m->meshes[i].material = _load_material (m, material);
    }
}

static void read_pose(struct iqm_pose *pose, struct _iqm_joint *joint)
{
    memcpy(pose->translate, joint->translate, 3 * sizeof(float));
    memcpy(pose->rotate, joint->rotate, 4 * sizeof(float));
    memcpy(pose->scale, joint->scale, 3 * sizeof(float));

	quat_normalize(pose->rotate);
}

static void
_load_bones (struct iqm_model *m,
             unsigned char *data,
             struct _iqm_header *h)
{
    struct _iqm_joint *j = malloc(h->num_joints * sizeof(struct _iqm_joint));
    memcpy(j, data + h->ofs_joints, h->num_joints * sizeof(struct _iqm_joint));
    m->num_bones = h->num_joints;
    m->bones = malloc(h->num_joints * sizeof(struct iqm_bone));
    int i;
    for (i=0; i<h->num_joints; i++) {
        float q[16];
        char *name = (char *)(data + h->ofs_text + j[i].name);
        
        m->bones[i].parent = j[i].parent;
        m->bones[i].name = malloc(strlen(name)*sizeof(char));
        strcpy(m->bones[i].name, name);
        read_pose(&m->bones[i].bind_pose, &j[i]);
        mat_from_pose(q,
                      m->bones[i].bind_pose.translate,
                      m->bones[i].bind_pose.rotate,
                      m->bones[i].bind_pose.scale);
        if (m->bones[i].parent >= 0) {
            struct iqm_bone *parent = &m->bones[m->bones[i].parent];
            mat_mul44(m->bones[i].bind_matrix, parent->bind_matrix, q);
        } else {
            mat_copy(m->bones[i].bind_matrix, q);
        }
        mat_invert(m->bones[i].inv_bind_matrix, m->bones[i].bind_matrix);
    }
    free (j);
}

static void
_load_anims (struct iqm_model *m,
             unsigned char *data,
             struct _iqm_header *h)
{
    int i;
    m->num_anims = h->num_anims;
    m->anims = malloc (h->num_anims * sizeof(struct iqm_anim));
    struct _iqm_anim *a = malloc(h->num_anims * sizeof(struct _iqm_anim));
    memcpy(a, data + h->ofs_anims, h->num_anims * sizeof(struct _iqm_anim));
    for (i=0; i < h->num_anims; i++) {
        char *name = (char *)data + h->ofs_text + a[i].name;
        m->anims[i].name = malloc(strlen(name)*sizeof(char));
        strlcpy(m->anims[i].name, name, strlen(name));
        m->anims[i].first = a[i].first_frame;
        m->anims[i].count = a[i].num_frames;
        m->anims[i].rate  = a[i].framerate;
        m->anims[i].loop  = a[i].flags;
    }
    free (a);
}

struct chan {
    int mask;
    float offset[10];
    float scale[10];
};

static void
_load_frames (struct iqm_model *mdl,
              unsigned char *data,
              struct _iqm_header *h)
{
    int i;
    int k;
    int n;
    struct chan *chans = malloc (h->num_joints * sizeof(struct chan));
    unsigned char *p = data + h->ofs_frames;

    mdl->num_frames = h->num_frames;
    mdl->bounds = malloc(h->num_frames * h->num_joints * sizeof(struct iqm_bounds));
    mdl->poses = malloc(h->num_frames * sizeof (struct iqm_pose *));
    int ofs_poses = h->ofs_poses;
    for (k=0; k<h->num_joints; k++) {
        chans[k].mask = _read32(data + ofs_poses + 4);
        for (n=0; n<10; n++) {
            chans[k].offset[n] = _readfloat(data + ofs_poses + 8 + n * 4);
            chans[k].scale[n] = _readfloat(data + ofs_poses + 8 + 10 * 4 + n * 4);
        }
        ofs_poses += 22*4;
    }

    for (i=0; i<h->num_frames; i++) {
        mdl->poses[i] = malloc (h->num_joints * sizeof(struct iqm_pose));
        for (k=0; k<h->num_joints; k++) {
            for (n=0; n<3; n++) {
                mdl->poses[i][k].translate[n] = chans[k].offset[n];
                mdl->poses[i][k].rotate[n] = chans[k].offset[3+n];
                mdl->poses[i][k].scale[n]     = chans[k].offset[7+n];
            }
            mdl->poses[i][k].rotate[3] = chans[k].offset[6];

			if (chans[k].mask & 0x01) {
                mdl->poses[i][k].translate[0] += _read16(p) * chans[k].scale[0]; p += 2;
            }
			if (chans[k].mask & 0x02) {
                mdl->poses[i][k].translate[1] += _read16(p) * chans[k].scale[1]; p += 2;
            }
			if (chans[k].mask & 0x04) {
                mdl->poses[i][k].translate[2] += _read16(p) * chans[k].scale[2]; p += 2;
            }
			if (chans[k].mask & 0x08) {
                mdl->poses[i][k].rotate[0] += _read16(p) * chans[k].scale[3]; p += 2;
            }
			if (chans[k].mask & 0x10) {
                mdl->poses[i][k].rotate[1] += _read16(p) * chans[k].scale[4]; p += 2;
            }
			if (chans[k].mask & 0x20) {
                mdl->poses[i][k].rotate[2] += _read16(p) * chans[k].scale[5]; p += 2;
            }
			if (chans[k].mask & 0x40) {
                mdl->poses[i][k].rotate[3] += _read16(p) * chans[k].scale[6]; p += 2;
            }
			if (chans[k].mask & 0x80) {
                mdl->poses[i][k].scale[0] += _read16(p) * chans[k].scale[7]; p += 2;
            }
			if (chans[k].mask & 0x100) {
                mdl->poses[i][k].scale[1] += _read16(p) * chans[k].scale[8]; p += 2;
            }
			if (chans[k].mask & 0x200) {
                mdl->poses[i][k].scale[2] += _read16(p) * chans[k].scale[9]; p += 2;
            }
		}
    }
    free (chans);
}

static struct iqm_model *
_load_model (unsigned char *data,
             struct _iqm_header *imodel,
             char *filename)
{
    struct iqm_model *rc = malloc (sizeof (struct iqm_model));
    memset (rc, 0, sizeof (struct iqm_model));
    int i=0;

    char *p = strrchr(filename,'/') + 1;
    rc->dir = malloc((p - filename + 1) * sizeof(char));
    strlcpy(rc->dir, filename, p - filename+1);
  
    if (imodel->num_vertexarrays &&
        imodel->num_vertexes &&
        imodel->num_triangles &&
        imodel->num_meshes) {
        _load_vertex_arrays (rc, data, imodel);
        _load_triangles (rc, data, imodel);
        _load_meshes (rc, data, imodel);
    }
    if (imodel->num_joints) {
        _load_bones (rc, data, imodel);
    }
    if (imodel->num_anims) {
        _load_anims (rc, data, imodel);
    }
    if (imodel->num_frames) {
        _load_frames (rc, data, imodel);
    }

    rc->min[0] = rc->min[1] = rc->min[2] = 1e10;
    rc->max[1] = rc->max[1] = rc->max[2] = -1e10;
  
    for (i = 0; i < rc->num_vertices * 3; i += 3) {
        float x = rc->pos[i];
        float y = rc->pos[i+1];
        float z = rc->pos[i+2];
        float r = x*x + y*y + z*z;
        if (x < rc->min[0]) rc->min[0] = x;
        if (y < rc->min[1]) rc->min[1] = y;
        if (z < rc->min[2]) rc->min[2] = z;
        if (x > rc->max[0]) rc->max[0] = x;
        if (y > rc->max[1]) rc->max[1] = y;
        if (z > rc->max[2]) rc->max[2] = z;
        if (r > rc->radius) rc->radius = r;
    }
  
    rc->radius = sqrtf(rc->radius);
  
    rc->outpose = NULL;
    rc->outbone = NULL;
    rc->outskin = NULL;
    rc->dpos    = NULL;
    rc->dnorm   = NULL;
  
    return rc;  
}

/*
 * Public functions
 */

/*
 * Load IQM model from disc and parse it.
 *
 * @param model_fname - file name of a supposed IQM data file
 * @return NULL if file does not exist, a valid file pointer otherwise
 *
 */
struct iqm_model *
model_iqm_load(char *model_fname)
{
    struct iqm_model *rc = NULL;
    struct _iqm_header *imodel = malloc(sizeof(struct _iqm_header));
    unsigned char *data;
  
    FILE *file;

    file = fopen (model_fname, "rb");
    if (!file) {
        fprintf (stderr, "cannot open model '%s'\n", model_fname);
        return rc;
    } 

    printf ("loading iqm model '%s'\n", model_fname);
    fread (imodel, 1, sizeof (struct _iqm_header), file);
    if (strcmp(imodel->magic, IQM_MAGIC) != 0) {
        fprintf (stderr, "not and IQM file '%s'\n", model_fname);
        fclose (file);
        return rc;
    }
    if (imodel->version != IQM_VERSION) {
        fprintf (stderr, "unknown IQM version '%s'\n", model_fname);
        fclose (file);
        return NULL;
    }

    data = malloc (imodel->filesize);
    fseek(file , 0, 0);
    fread (data, 1, imodel->filesize, file);
    fclose (file);
    rc = _load_model (data, imodel, model_fname);
    free (data);
    return rc;
}

void
_animate_iqm_model (struct iqm_model *model,
                    int anim,
                    int frame,
                    float t)
{
	struct iqm_pose *pose0, *pose1;
	float m[16], q[4], v[3];
	int frame0, frame1;
	int i;

	if (!model->num_bones) {
        fprintf (stderr, "objview: Spineless jelly! No bones here...\n");
        return;
    }
    
    if (!model->num_anims) {
        fprintf (stderr, "objview: No animations to run...\n");
		return;
    }

	if (!model->outbone) {
		model->outbone = malloc(model->num_bones * sizeof(float[16]));
		model->outskin = malloc(model->num_bones * sizeof(float[16]));
	}

	if (anim < 0) anim = 0;
	if (anim >= model->num_anims) anim = model->num_anims - 1;

	frame0 = frame % model->anims[anim].count;
	frame1 = (frame + 1) % model->anims[anim].count;
	pose0 = model->poses[model->anims[anim].first + frame0];
	pose1 = model->poses[model->anims[anim].first + frame1];

	for (i = 0; i < model->num_bones; i++) {
		int parent = model->bones[i].parent;
		quat_lerp_neighbor_normalize(q, pose0[i].rotate, pose1[i].rotate, t);
		vec_lerp(v, pose0[i].translate, pose1[i].translate, t);
		if (parent >= 0) {
			//mat_from_pose(m, q, v);
			mat_mul(model->outbone[i], model->outbone[parent], m);
		} else {
			//mat_from_pose(model->outbone[i], q, v);
		}
		mat_mul(model->outskin[i], model->outbone[i], model->bones[i].inv_bind_matrix);
	}
}

static void make_vbo(struct iqm_model *model)
{
	int norm_ofs = 0;
    int texcoord_ofs = 0;
    //int color_ofs = 0;
    int blend_index_ofs = 0;
    int blend_weight_ofs = 0;
	int n = 12;
  
	if (model->norm) { norm_ofs = n; n += 12; }
	if (model->texcoord) { texcoord_ofs = n; n += 8; }
	//if (model->color) { color_ofs = n; n += 4; }
    /*
	if (model->blend_index && model->blend_weight) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        color_ofs * model->num_vertices,
                        4 * model->num_vertices,
                        model->color);
    }
    */
        
    if (model->blend_index && model->blend_weight) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        blend_index_ofs * model->num_vertices,
                        4 * model->num_vertices,
                        model->blend_index);
        glBufferSubData(GL_ARRAY_BUFFER,
                        blend_weight_ofs * model->num_vertices,
                        4 * model->num_vertices,
                        model->blend_weight);
    }
    
    glGenBuffers(1, &model->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 model->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 model->num_triangles * 3 * sizeof(int),
                 model->triangles, GL_STATIC_DRAW);
}

static void
_draw_iqm_instances(struct iqm_model *m,
                    float *transform,
                    int count)
{
    int i,k,n;
    int prog, loc, bi_loc, bw_loc;

    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
	loc = glGetUniformLocation(prog, "BoneMatrix");
	bi_loc = glGetAttribLocation(prog, "in_BlendIndex");
	bw_loc = glGetAttribLocation(prog, "in_BlendWeight");

	if (!m->vbo)
		make_vbo(m);

	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);

	glVertexPointer(3, GL_FLOAT, 0, 0 );
	glEnableClientState(GL_VERTEX_ARRAY);
	n = 12 * m->num_vertices;

	if (m->norm) {
		glNormalPointer(GL_FLOAT, 0, (char*)n);
        glEnableClientState(GL_NORMAL_ARRAY);
		n += 12 * m->num_vertices;
	}
	if (m->texcoord) {
		glTexCoordPointer(2, GL_FLOAT, 0, (char*)n);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		n += 8 * m->num_vertices;
	}
	/* if (m->color) { */
	/* 	glColorPointer(4, GL_UNSIGNED_BYTE, 0, (char*)n); */
	/* 	glEnableClientState(GL_COLOR_ARRAY); */
	/* 	n += 4 * m->num_vertices; */
	/* } */

	if (1 || (loc >= 0 && bi_loc >= 0 && bw_loc >= 0)) {
		if (m->outskin && m->blend_index && m->blend_weight) {
			glUniformMatrix4fv(loc, m->num_bones, 0, m->outskin[0]);
			glVertexAttribPointer(bi_loc, 4, GL_UNSIGNED_BYTE, 0, 0, (char*)n);
			n += 4 * m->num_vertices;
			glVertexAttribPointer(bw_loc, 4, GL_UNSIGNED_BYTE, 1, 0, (char*)n);
			n += 4 * m->num_vertices;
			glEnableVertexAttribArray(bi_loc);
			glEnableVertexAttribArray(bw_loc);
		}
	}

	for (i = 0; i < m->num_meshes; i++) {
		struct iqm_mesh *mesh = m->meshes + i;
		glBindTexture(GL_TEXTURE_2D, mesh->material);
		for (k = 0; k < count; k++) {
			// dog slow! should use our own uniforms, or instanced array
            //	glPushMatrix();
            //	glMultMatrixf(&transform[k*16]);
			glDrawElements(GL_TRIANGLES, 3 * mesh->count, GL_UNSIGNED_INT, (char*)(mesh->first*12));
            //	glPopMatrix();
		}
	}

	glDisableVertexAttribArray(ATT_POSITION);
	glDisableVertexAttribArray(ATT_NORMAL);
	glDisableVertexAttribArray(ATT_TEXCOORD);
	glDisableVertexAttribArray(ATT_COLOR);
	if (loc >= 0 && bi_loc >= 0 && bw_loc >= 0) {
		glDisableVertexAttribArray(bi_loc);
		glDisableVertexAttribArray(bw_loc);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*
void
draw_iqm_model (struct iqm_model *m)
{
    float transform[16];

    mat_identity(transform);
    _draw_iqm_instances(m, transform, 1);
}
*/

void
model_iqm_draw_static (struct iqm_model *model)
{
    int i;
    int limit = model->num_meshes;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, model->pos); 
    glNormalPointer(GL_FLOAT, 0, model->norm);
    glTexCoordPointer(2, GL_FLOAT, 0, model->texcoord);

    if (model->dpos) glVertexPointer (3,GL_FLOAT, 0, model->dpos);
    if (model->dnorm) glNormalPointer (GL_FLOAT, 0, model->dnorm);

    for (i = 0; i < limit; i++) {
        struct iqm_mesh *mesh = model->meshes + i;
        glBindTexture(GL_TEXTURE_2D, mesh->material);
        glDrawElements(GL_TRIANGLES,
                       mesh->count * 3,
                       GL_UNSIGNED_INT,
                       model->triangles + mesh->first * 3);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
}

void
_draw_string(void *font, char *string)
{
    while (*string)
        glutBitmapCharacter(font, *string++);
}

void
model_iqm_draw_bones(struct iqm_model *model)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<model->num_bones; i++)
    {
        struct iqm_bone *b = model->bones+i;
        glBegin(GL_LINES);
        if (b->parent >= 0)
        {
            struct iqm_bone *pb = model->bones + b->parent;
            glColor3f(0,1,0);
            glVertex3f(pb->bind_matrix[12],
                       pb->bind_matrix[13],
                       pb->bind_matrix[14]);
        } else {
            glColor3f(0,1,0.5);
            glVertex3f(0,0,0);
        }
        glVertex3f(b->bind_matrix[12],
                   b->bind_matrix[13],
                   b->bind_matrix[14]);

        glEnd();
        glRasterPos3f(b->bind_matrix[12],
                      b->bind_matrix[13],
                      b->bind_matrix[14]);

        _draw_string(GLUT_BITMAP_HELVETICA_10, b->name);
    }

    glEnable(GL_DEPTH_TEST);
}

void
model_iqm_draw_anim_bones(struct iqm_model *model)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<model->num_bones; i++)
    {
        struct iqm_bone *b = model->bones+i;
        glBegin(GL_LINES);
        if (b->parent >= 0)
        {
            struct iqm_bone *pb = model->bones + b->parent;
            glColor3f(0,0.5,0.5);
            glVertex3f(pb->bind_matrix[12],
                       pb->bind_matrix[13],
                       pb->bind_matrix[14]);
        } else {
            glColor3f(0,0.5,1);
            glVertex3f(0,0,0);
        }

        glVertex3f(b->anim_matrix[12],
                   b->anim_matrix[13],
                   b->anim_matrix[14]);

        glEnd();

        glRasterPos3f(b->anim_matrix[12],
                      b->anim_matrix[13],
                      b->anim_matrix[14]);

        _draw_string(GLUT_BITMAP_HELVETICA_10, b->name);
    }

    glEnable(GL_DEPTH_TEST);
}

void
model_iqm_draw (struct iqm_model *model)
{
    
}

struct iqm_bone *
_get_bone (struct iqm_model *m ,int index) {
    return &m->bones[index];
}

void
_get_delta (struct iqm_model *model,
            int frame)
{
    int i;
    for (i=0; i<model->num_bones; i++) {
        struct iqm_bone *bone = model->bones + i;
        struct iqm_pose *pose = model->poses[frame] + i;
        
        if (bone->parent >= 0) {
            float m_pose[16];
            struct iqm_bone *parent = (struct iqm_bone *)(model->bones + bone->parent);
            mat_from_pose (m_pose, pose->translate, pose->rotate, pose->scale);
            mat_mul44(bone->anim_matrix, parent->anim_matrix, m_pose);
        } else {
            mat_from_pose (bone->anim_matrix, pose->translate, pose->rotate, pose->scale);
        }
        mat_mul44(bone->diff, bone->anim_matrix, bone->inv_bind_matrix);
    }
    
}

void
model_iqm_animate (struct iqm_model *model,
                   int a,
                   int f,
                   float t)
{
    /*
    int i,j,k,l;
    float v[3];
    unsigned char bi[4];
    struct iqm_anim *anim;
    struct iqm_bone *bone;
    
    _get_delta (model, f);
    
    memcpy (v, model->pos+(i*3), 3 * sizeof (float));
    memcpy (bi, model->blend_index + (4 * i), 4 * sizeof (unsigned char));
    bone = model->bones + bi[0];
    anim = model->anims + f;
    fprintf (stderr, "objview: frame %s, first %d, count %d\n",
             anim->name,
             anim->first,
             anim->count);
    */

    int i;
    a %= model->num_anims;
    f %= model->anims[a].count;

    printf ("anim %d frame %d\n", a, f);

    _get_delta (model, model->anims[a].first + f);
    
    if (model->dnorm == NULL) {
        model->dnorm = malloc (model->num_vertices * 3 * sizeof(float));
    }
    if (model->dpos == NULL) {
        model->dpos = malloc (model->num_vertices * 3 * sizeof(float));
    }
    
    for (i=0; i<model->num_vertices; i++) {
        unsigned char *bi = &model->blend_index[i*4];
        unsigned char *bw = &model->blend_weight[i*4];
        
        mat_vec_mul (model->dpos + i*3,
                     model->bones[bi[0]].diff,
                     model->pos + i*3);
        mat_vec_mul_n (model->dnorm + i*3,
                       model->bones[bi[0]].diff,
                       model->norm + i*3);
    }        
}
