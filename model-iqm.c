#include <model-iqm.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <image.h>
#include <vector.h>
#include <unit.h>

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
_load_vertex_arrays(struct ov_model *m,
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
                m->pos = _load_float_array(data + va->offset,
                                           va->size,
                                           h->num_vertexes);
                break;
            case IQM_VAT_TEXCOORD:
                assert(va->format == IQM_VAF_FLOAT && va->size == 2);
                m->texcoord = _load_float_array(data + va->offset,
                                                va->size,
                                                h->num_vertexes);
                break;
            case IQM_VAT_NORMAL:
                assert(va->format == IQM_VAF_FLOAT && va->size == 3);
                m->norm = _load_float_array(data + va->offset,
                                            va->size,
                                            h->num_vertexes);
                break;
            case IQM_VAT_BLENDINDEXES:
                assert(va->format == IQM_VAF_UBYTE && va->size == 4);
                m->blend_index = _load_ubyte_array (data + va->offset,
                                                    va->size,
                                                    h->num_vertexes);
                break;
            case IQM_VAT_BLENDWEIGHTS:
                assert(va->format == IQM_VAF_UBYTE && va->size == 4);
                m->blend_weight = _load_ubyte_array (data + va->offset,
                                                     va->size, h->num_vertexes);
                break;
		}
    }
    //free (va);
}

static void
_load_triangles (struct ov_model *m,
                 unsigned char *data,
                 struct _iqm_header *h)
{
    int size = h->num_triangles * 3 * (sizeof (int));
    m->num_triangles = h->num_triangles;
    m->triangles = malloc (size);
    memcpy (m->triangles, data + h->ofs_triangles, size);
}

static int
_load_material (struct ov_model *m,
                char             *mat_name)
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
_load_meshes (struct ov_model   *m,
              unsigned char      *data,
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

static void read_pose(struct ov_pose   *pose,
                      struct _iqm_joint *joint)
{
    memcpy(pose->translate, joint->translate, 3 * sizeof(float));
    memcpy(pose->rotate, joint->rotate, 4 * sizeof(float));
    memcpy(pose->scale, joint->scale, 3 * sizeof(float));

	quat_normalize(pose->rotate);
}

static void
_load_bones (struct ov_skeleton *s,
             unsigned char       *data,
             struct _iqm_header  *h)
{
    struct _iqm_joint *j = malloc(h->num_joints * sizeof(struct _iqm_joint));
    memcpy(j, data + h->ofs_joints, h->num_joints * sizeof(struct _iqm_joint));
    s->num_bones = h->num_joints;
    s->bones = malloc(h->num_joints * sizeof(struct iqm_bone));
    int i;
    for (i=0; i<h->num_joints; i++) {
        float q[16];
        char *name = (char *)(data + h->ofs_text + j[i].name);
        
        s->bones[i].parent = j[i].parent;
        s->bones[i].name = malloc(strlen(name)*sizeof(char)+1);
        strcpy(s->bones[i].name, name);
        read_pose(&s->bones[i].bind_pose, &j[i]);
        mat_from_pose(q,
                      s->bones[i].bind_pose.translate,
                      s->bones[i].bind_pose.rotate,
                      s->bones[i].bind_pose.scale);
        if (s->bones[i].parent >= 0) {
            struct ov_bone *parent = &s->bones[s->bones[i].parent];
            mat_mul44(s->bones[i].bind_matrix, parent->bind_matrix, q);
        } else {
            mat_copy(s->bones[i].bind_matrix, q);
        }
        mat_invert(s->bones[i].inv_bind_matrix, s->bones[i].bind_matrix);
    }
    free (j);
}

static void
_load_anims (struct ov_animation *a,
             unsigned char        *data,
             struct _iqm_header   *h)
{
    int i;
    struct _iqm_anim *n = malloc(h->num_anims * sizeof(struct _iqm_anim));
    memcpy(n, data + h->ofs_anims, h->num_anims * sizeof(struct _iqm_anim));
    //for (i=0; i < h->num_anims; i++) {
    i=0;
    char *name = (char *)data + h->ofs_text + n[i].name;
    a->name = malloc(strlen(name)*sizeof(char));
    strlcpy(a->name, name, strlen(name));
    a->first = n[i].first_frame;
    a->count = n[i].num_frames;
    a->rate  = n[i].framerate;
    a->loop  = n[i].flags;
    //}
    //free (a);
}

struct chan {
    int mask;
    float offset[10];
    float scale[10];
};

static void
_load_frames (struct ov_animation *anim,
              unsigned char *data,
              struct _iqm_header *h)
{
    int i;
    int k;
    int n;
    struct chan *chans = malloc (h->num_joints * sizeof(struct chan));
    unsigned char *p = data + h->ofs_frames;

    anim->num_frames = h->num_frames;
    anim->poses = malloc(h->num_frames * sizeof (struct iqm_pose *));
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
        anim->poses[i] = malloc (h->num_joints * sizeof(struct iqm_pose));
        for (k=0; k<h->num_joints; k++) {
            for (n=0; n<3; n++) {
                anim->poses[i][k].translate[n] = chans[k].offset[n];
                anim->poses[i][k].rotate[n] = chans[k].offset[3+n];
                anim->poses[i][k].scale[n]     = chans[k].offset[7+n];
            }
            anim->poses[i][k].rotate[3] = chans[k].offset[6];

			if (chans[k].mask & 0x01) {
                anim->poses[i][k].translate[0] += _read16(p) * chans[k].scale[0]; p += 2;
            }
			if (chans[k].mask & 0x02) {
                anim->poses[i][k].translate[1] += _read16(p) * chans[k].scale[1]; p += 2;
            }
			if (chans[k].mask & 0x04) {
                anim->poses[i][k].translate[2] += _read16(p) * chans[k].scale[2]; p += 2;
            }
			if (chans[k].mask & 0x08) {
                anim->poses[i][k].rotate[0] += _read16(p) * chans[k].scale[3]; p += 2;
            }
			if (chans[k].mask & 0x10) {
                anim->poses[i][k].rotate[1] += _read16(p) * chans[k].scale[4]; p += 2;
            }
			if (chans[k].mask & 0x20) {
                anim->poses[i][k].rotate[2] += _read16(p) * chans[k].scale[5]; p += 2;
            }
			if (chans[k].mask & 0x40) {
                anim->poses[i][k].rotate[3] += _read16(p) * chans[k].scale[6]; p += 2;
            }
			if (chans[k].mask & 0x80) {
                anim->poses[i][k].scale[0] += _read16(p) * chans[k].scale[7]; p += 2;
            }
			if (chans[k].mask & 0x100) {
                anim->poses[i][k].scale[1] += _read16(p) * chans[k].scale[8]; p += 2;
            }
			if (chans[k].mask & 0x200) {
                anim->poses[i][k].scale[2] += _read16(p) * chans[k].scale[9]; p += 2;
            }
		}
    }
    free (chans);
}

static struct ov_model *
_load_model (unsigned char *data,
             struct _iqm_header *imodel,
             char *filename)
{
    struct ov_model *rc = malloc (sizeof *rc);
    memset (rc, 0, sizeof *rc);
    rc->skeleton = malloc (sizeof (struct ov_skeleton));
    memset (rc->skeleton, 0, sizeof (struct ov_skeleton));
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
        _load_bones (rc->skeleton, data, imodel);
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

static struct ov_animation *
_load_animation (unsigned char *data,
                 struct _iqm_header *imodel,
                 char *filename)
{
    struct ov_animation *rc = malloc (sizeof (struct iqm_animation));
    memset (rc, 0, sizeof (struct iqm_animation));
    rc->skeleton = malloc (sizeof (struct iqm_skeleton));
    memset (rc->skeleton, 0, sizeof (struct iqm_skeleton));
    
    char *p = strrchr(filename,'/') + 1;
    rc->dir = malloc((p - filename + 1) * sizeof(char));
    strlcpy(rc->dir, filename, p - filename+1);
  

    if (imodel->num_joints) {
        _load_bones (rc->skeleton, data, imodel);
    }
    if (imodel->num_anims) {
        _load_anims (rc, data, imodel);
    }

    if (imodel->num_frames) {
        _load_frames (rc, data, imodel);
    }
  
    return rc;  
}

static void
_draw_string(void *font, char *string)
{
    while (*string)
        glutBitmapCharacter(font, *string++);
}

static void
_get_delta (struct iqm_model     *model,
            struct iqm_animation *animation,
            int                  *table,
            int                   frame)
{
    int i;
    for (i=0; i<model->skeleton->num_bones; i++) {
        struct iqm_bone *bone = model->skeleton->bones + i;
        struct iqm_pose *pose = animation->poses[frame] + i;
        
        if (bone->parent >= 0) {
            float m_pose[16];
            struct iqm_bone *parent = model->skeleton->bones + bone->parent;
            mat_from_pose (m_pose, pose->translate, pose->rotate, pose->scale);
            mat_mul44(bone->anim_matrix, parent->anim_matrix, m_pose);
        } else {
            mat_from_pose (bone->anim_matrix, pose->translate, pose->rotate, pose->scale);
        }
        mat_mul44(bone->diff, bone->anim_matrix, bone->inv_bind_matrix);
    }
    
}

/*
 * Public functions
 */

/*
 * Load IQM model from disc and parse it.
 *
 * @param model_fname - file name of a supposed IQM data file
 * @return NULL if file does not exist, a valid pointer to a model otherwise
 *
 */
struct ov_model *
model_iqm_load_model(char *model_fname)
{
    struct ov_model *rc = NULL;
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

/*
 * Load animation data in the IQM format.
 *
 * @param animation_fname - name of the file containing the animation
 *                          data
 * @return NULL if file doesn't exist, a valid pointer to an animation otherwise
 *
 */
struct ov_animation *
model_iqm_load_animation(char *animation_fname)
{
    struct ov_animation *rc = NULL;
    struct _iqm_header *imodel = malloc(sizeof(struct _iqm_header));
    unsigned char *data;
  
    FILE *file;

    file = fopen (animation_fname, "rb");
    if (!file) {
        fprintf (stderr, "cannot open model '%s'\n", animation_fname);
        return rc;
    } 

    printf ("loading iqm animation '%s'\n", animation_fname);
    fread (imodel, 1, sizeof (struct _iqm_header), file);
    if (strcmp(imodel->magic, IQM_MAGIC) != 0) {
        fprintf (stderr, "not and IQM file '%s'\n", animation_fname);
        fclose (file);
        return rc;
    }
    if (imodel->version != IQM_VERSION) {
        fprintf (stderr, "unknown IQM version '%s'\n", animation_fname);
        fclose (file);
        return NULL;
    }

    data = malloc (imodel->filesize);
    fseek(file , 0, 0);
    fread (data, 1, imodel->filesize, file);
    fclose (file);
    rc = _load_animation (data, imodel, animation_fname);
    free (data);
    return rc;
}

/*
 * Draw the static model, no animations, bells or whistles
 *
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

/*
 * Draw the bones of the static image as a wireframe
 *
 * @param model - pointer to an instance of struct iqm_model
 *
 */
void
model_iqm_draw_bones(struct iqm_model *model)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<model->skeleton->num_bones; i++)
    {
        struct iqm_bone *b = model->skeleton->bones+i;
        glBegin(GL_LINES);
        if (b->parent >= 0)
        {
            struct iqm_bone *pb = model->skeleton->bones + b->parent;
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

/*
 * Draw the animated bones of the model
 *
 */
void
model_iqm_draw_anim_bones(struct iqm_model *model)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<model->skeleton->num_bones; i++)
    {
        struct iqm_bone *b = model->skeleton->bones+i;
        glBegin(GL_LINES);
        if (b->parent >= 0)
        {
            struct iqm_bone *pb = model->skeleton->bones + b->parent;
            glColor3f(0,0.5,0.5);
            glVertex3f(pb->anim_matrix[12],
                       pb->anim_matrix[13],
                       pb->anim_matrix[14]);
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

int *
_match_bones (struct iqm_model *m,
              struct iqm_animation *a)
{
    int p = sizeof (int) * a->skeleton->num_bones;
    int *rc = malloc (p);
    int i,j;
    for (i=0; i<a->skeleton->num_bones; i++) {
        struct iqm_bone *ab = a->skeleton->bones + i;
        for (j=0; j<m->skeleton->num_bones; j++) {
            struct iqm_bone *mb = m->skeleton->bones + j;
            if (strcmp(mb->name, ab->name) == 0) {
                rc [i] = j;
                break;
            } else {
                rc [i] = -1;
            }
        }
    }
    return rc;
}

void
model_iqm_animate (struct iqm_model      *model,
                   struct iqm_animations *animations,
                   int                    a,
                   int                    f,
                   float                  t)
{
    int i;
    a %= animations->num_anims;
    f %= animations->anims[a]->count;

    printf ("anim %d frame %d\n", a, f);

    int *table = _match_bones (model, animations->anims[a]);
    
    _get_delta (model, animations->anims[a], table, f);
    
    if (model->dnorm == NULL) {
        model->dnorm = malloc (model->num_vertices * 3 * sizeof(float));
    }
    if (model->dpos == NULL) {
        model->dpos = malloc (model->num_vertices * 3 * sizeof(float));
    }
    
    for (i=0; i<model->num_vertices; i++) {
        unsigned char *bi = &model->blend_index[i*4];
        //unsigned char *bw = &model->blend_weight[i*4];
        
        mat_vec_mul (model->dpos + i*3,
                     model->skeleton->bones[bi[0]].diff,
                     model->pos + i*3);
        mat_vec_mul_n (model->dnorm + i*3,
                       model->skeleton->bones[bi[0]].diff,
                       model->norm + i*3);
    }
}
