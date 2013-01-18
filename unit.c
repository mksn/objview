#include <mksn-gl.h>
#include <unit.h>
#include <model-iqm.h>
#include <vector.h>

/*******************************************************************************
 * Private functions 
 *
 */

/**
 * Draw a given string with a given font
 *
 */
static void _draw_string(void *font, char *string)
{
    while (*string)
        glutBitmapCharacter(font, *string++);
}

/**
 * Compute the delta between the bones of the static model
 * and of the next animation "step".
 *
 * model: the static model
 * animation: the animation with the poses to manipulate the
 *            skeleton in the model
 * table    : table of the bones that match, to alleviate the calculation
 * frame    : which frame in question that will be animated and thus
 *            needs a differential calculation
 *
 */
static void
_get_delta (struct ov_model     *model,
            struct ov_animation *animation,
            int                 *table,
            int                  frame)
{
    int i;
    for (i=0; i<model->skeleton->num_bones; i++) {
        struct ov_bone *bone = model->skeleton->bones + i;
        struct ov_pose *pose = animation->poses[frame] + i;
        
        if (bone->parent >= 0) {
            float m_pose[16];
            struct ov_bone *parent = model->skeleton->bones + bone->parent;
            mat_from_pose (m_pose, pose->translate, pose->rotate, pose->scale);
            mat_mul44(bone->anim_matrix, parent->anim_matrix, m_pose);
        } else {
            mat_from_pose (bone->anim_matrix, pose->translate, pose->rotate, pose->scale);
        }
        mat_mul44(bone->diff, bone->anim_matrix, bone->inv_bind_matrix);
    }
    
}

/**
 * Match the bones between model and animation to find the subset that is
 * present in both.
 *
 * m : pointer to the model instance
 * a : pointer to the animation instance
 *
 * Returns: the table with the offsets for each of the bones in the model
 *          corresponding to each bone in the animation skeleton
 */
static int *
_match_bones (struct ov_model *m,
              struct ov_animation *a)
{
    int p = sizeof (int) * a->skeleton->num_bones;
    int *rc = malloc (p);
    int i,j;
    for (i=0; i<a->skeleton->num_bones; i++) {
        struct ov_bone *ab = a->skeleton->bones + i;
        for (j=0; j<m->skeleton->num_bones; j++) {
            struct ov_bone *mb = m->skeleton->bones + j;
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

/*******************************************************************************
 * Public interface
 *
 */

/**
 * Create an object view unit animation
 *
 */
struct ov_animations *
ov_create_unit_animations ()
{
    struct ov_animations *rc = malloc (sizeof(struct iqm_animations));
    rc->num_anims = 0;
    rc->at_table  = NULL;
    rc->anims     = NULL;
    return rc;
}   

struct ov_unit *
ov_create_unit ()
{
    struct ov_unit *rc = malloc (sizeof(struct ov_unit));
    rc->animations = malloc (sizeof(struct iqm_animations));
    rc->animations->at_table = malloc(sizeof(struct ov_anim_txlate )*ANIM_T_SIZE);
    memset(rc->animations->at_table, 0, ANIM_T_SIZE);
    rc->animations->anims = malloc (sizeof (struct iqm_animation *) * ANIM_T_SIZE);
    memset(rc->animations->anims, 0, ANIM_T_SIZE);
    rc->animations->num_anims = 0;
    return rc;
}

void
ov_set_model(struct ov_unit *unit,
             char           *model_fname)
{
    unit->model = model_iqm_load_model (model_fname);
}

    
int
ov_get_number_anims(struct ov_unit *unit)
{
    return unit->animations->num_anims;
}


void
ov_add_animation(struct ov_unit *unit,
                 char           *animation_fname,
                 animation_t     key)
{
    struct ov_animation *a = model_iqm_load_animation (animation_fname);
    if (a != NULL) {
        if (unit->animations->anims[key] != NULL) {
            free (unit->animations->anims[key]);
        } 
        unit->animations->at_table[key] = malloc(sizeof(struct ov_anim_txlate));
        unit->animations->at_table[key]->key  = key;
        unit->animations->at_table[key]->anim = unit->animations->num_anims;
        unit->animations->anims[unit->animations->num_anims++] = a;
    }
}
/*
 * Draw the static model, no animations, bells or whistles
 *
 */
void
//ov_draw_static (struct ov_model *model)
ov_draw_static (struct ov_unit *du)
{
    int i;
    int limit = du->model->num_meshes;

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY); 
    glEnableClientState(GL_NORMAL_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, du->model->pos); 
    glNormalPointer(GL_FLOAT, 0, du->model->norm);
    glTexCoordPointer(2, GL_FLOAT, 0, du->model->texcoord);

    if (du->model->dpos) glVertexPointer (3,GL_FLOAT, 0, du->model->dpos);
    if (du->model->dnorm) glNormalPointer (GL_FLOAT, 0, du->model->dnorm);

    for (i = 0; i < limit; i++) {
        struct ov_mesh *mesh = du->model->meshes + i;
        glBindTexture(GL_TEXTURE_2D, mesh->material);
        glDrawElements(GL_TRIANGLES,
                       mesh->count * 3,
                       GL_UNSIGNED_INT,
                       du->model->triangles + mesh->first * 3);
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
//ov_draw_bones(struct ov_model *model)
ov_draw_bones(struct ov_unit *du)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<du->model->skeleton->num_bones; i++)
    {
        struct ov_bone *b = du->model->skeleton->bones+i;
        glBegin(GL_LINES);
        if (b->parent >= 0)
        {
            struct ov_bone *pb = du->model->skeleton->bones + b->parent;
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
//ov_draw_anim_bones(struct ov_model *model)
ov_draw_anim_bones(struct ov_unit *du)
{
    int i;

    glDisable(GL_DEPTH_TEST);

    for (i=0; i<du->model->skeleton->num_bones; i++)
    {
        struct ov_bone *b = du->model->skeleton->bones+i;
        glBegin(GL_LINES);

        if (b->parent >= 0)
        {
            struct ov_bone *pb = du->model->skeleton->bones + b->parent;
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


void
ov_animate (struct ov_unit *du,
            int             a,
            int             f,
            float           t)
{
    int i;
    a %= du->animations->num_anims;
    f %= du->animations->anims[a]->count;

    printf ("anim %d frame %d\n", a, f);
    int *table = _match_bones (du->model, du->animations->anims[a]);
    
    _get_delta (du->model, du->animations->anims[a], table, f);
    
    if (du->model->dnorm == NULL) {
        du->model->dnorm = malloc (du->model->num_vertices * 3 * sizeof(float));
    }
    if (du->model->dpos == NULL) {
        du->model->dpos = malloc (du->model->num_vertices * 3 * sizeof(float));
    }
    
    for (i=0; i<du->model->num_vertices; i++) {
        unsigned char *bi = &du->model->blend_index[i*4];
        //unsigned char *bw = &du->model->blend_weight[i*4];
        
        mat_vec_mul (du->model->dpos + i*3,
                     du->model->skeleton->bones[bi[0]].diff,
                     du->model->pos + i*3);
        mat_vec_mul_n (du->model->dnorm + i*3,
                       du->model->skeleton->bones[bi[0]].diff,
                       du->model->norm + i*3);
    }
}

