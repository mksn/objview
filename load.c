#include "objview.h"
#include "image.h"
#include "unit.h"
#include "vector.h"

#include <ctype.h>

unsigned int loadmaterial(char *material)
{
  char filename[2000], *s;
  s = strrchr(material, ';');
  if (s) material = s + 1;
  sprintf(filename, "%s/%s.png", "textures", material);
  return load_texture(0, filename);
}

#define IQE_MAGIC "# Inter-Quake Export"

enum {
  MAXVERTEX = 65535,
  MAXELEMENT = 65535
};

static struct ov_vertex vertex_buf[MAXVERTEX];
static int element_buf[MAXELEMENT];

struct ov_unit *
ov_create_unit(void)
{
  struct ov_unit *unit = malloc (sizeof(struct ov_unit));
  memset(unit, 0, sizeof *unit);
  return unit;
}

static char *parsestring(char **stringp)
{
  char *start, *end, *s = *stringp;
  while (isspace(*s)) s++;
  if (*s == '"') {
    s++;
    start = end = s;
    while (*end && *end != '"') end++;
    if (*end) *end++ = 0;
  } else {
    start = end = s;
    while (*end && !isspace(*end)) end++;
    if (*end) *end++ = 0;
  }
  *stringp = end;
  return start;
}

static char *parseword(char **stringp)
{
  char *start, *end, *s = *stringp;
  while (isspace(*s)) s++;
  start = end = s;
  while (*end && !isspace(*end)) end++;
  if (*end) *end++ = 0;
  *stringp = end;
  return start;
}

static inline float parsefloat(char **stringp, float def)
{
  char *s = parseword(stringp);
  return *s ? atof(s) : def;
}

static inline int parseint(char **stringp, int def)
{
  char *s = parseword(stringp);
  return *s ? atoi(s) : def;
}

struct ov_model *
ov_load_model_iqe(const char *filename)
{
  FILE *fp;
  char line[256];
  char *sp;
  char *s;
  int i;

  struct ov_model *model = malloc(sizeof *model);
  struct ov_skeleton *skeleton = malloc(sizeof *skeleton);

  int pq_is_bind_pose = 1;

  int position_count = 0;
  int normal_count = 0;
  int texcoord_count = 0;
  int blend_count = 0;
  int element_count = 0;

  int pose_count = 0;
  int bone_count = 0;
  int mesh_count = 0;

  int mesh_first_element = 0;
  int texture = 0;
  int fm = 0;

  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot load model '%s'\n", filename);
    exit(1);
  }

  if (!fgets(line, sizeof line, fp)) {
    fprintf(stderr, "cannot load %s: read error\n", filename);
    exit(1);
  }

  if (memcmp(line, IQE_MAGIC, strlen(IQE_MAGIC))) {
    fprintf(stderr, "cannot load %s: bad iqe magic\n", filename);
    exit(1);
  }

  while (1) {
    if (!fgets(line, sizeof line, fp))
      break;

    sp = line;

    s = parseword(&sp);
    if (!s)
      continue;

    if (s[0] == 'v' && s[1] != 0 && s[2] == 0) {
      switch (s[1]) {
      case 'p':
        vertex_buf[position_count].position[0] = parsefloat(&sp, 0);
        vertex_buf[position_count].position[1] = parsefloat(&sp, 0);
        vertex_buf[position_count].position[2] = parsefloat(&sp, 0);
        position_count++;
        break;

      case 'n':
        vertex_buf[normal_count].normal[0] = parsefloat(&sp, 0);
        vertex_buf[normal_count].normal[1] = parsefloat(&sp, 0);
        vertex_buf[normal_count].normal[2] = parsefloat(&sp, 0);
        normal_count++;
        break;

      case 't':
        vertex_buf[texcoord_count].texcoord[0] = parsefloat(&sp, 0);
        vertex_buf[texcoord_count].texcoord[1] = parsefloat(&sp, 0);
        texcoord_count++;
        break;

      case 'b':
        vertex_buf[blend_count].blend_index[0] = parseint(&sp, 0);
        vertex_buf[blend_count].blend_weight[0] = parsefloat(&sp, 0);
        vertex_buf[blend_count].blend_index[1] = parseint(&sp, 0);
        vertex_buf[blend_count].blend_weight[1] = parsefloat(&sp, 0);
        vertex_buf[blend_count].blend_index[2] = parseint(&sp, 0);
        vertex_buf[blend_count].blend_weight[2] = parsefloat(&sp, 0);
        vertex_buf[blend_count].blend_index[3] = parseint(&sp, 0);
        vertex_buf[blend_count].blend_weight[3] = parsefloat(&sp, 0);
        blend_count++;
        break;
      }
    }

    else if (s[0] == 'f' && s[1] == 'm' && s[2] == 0) {
      int a = parseint(&sp, 0);
      int b = parseint(&sp, 0);
      int c = parseint(&sp, -1);
      while (c > -1) {
        element_buf[element_count++] = a + fm;
        element_buf[element_count++] = b + fm;
        element_buf[element_count++] = c + fm;
        b = c;
        c = parseint(&sp, -1);
      }
    }

    else if (s[0] == 'p' && s[1] == 'q' && s[2] == 0) {
      if (pq_is_bind_pose) {
        skeleton->bones[pose_count].bind_pose.position[0] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.position[1] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.position[2] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.rotate[0] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.rotate[1] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.rotate[2] = parsefloat(&sp, 0);
        skeleton->bones[pose_count].bind_pose.rotate[3] = parsefloat(&sp, 1);
        skeleton->bones[pose_count].bind_pose.scale[0] = parsefloat(&sp, 1);
        skeleton->bones[pose_count].bind_pose.scale[1] = parsefloat(&sp, 1);
        skeleton->bones[pose_count].bind_pose.scale[2] = parsefloat(&sp, 1);
        pose_count++;
      }
    }

    else if (!strcmp(s, "joint")) {
      skeleton->bones[bone_count].name = strdup(parsestring(&sp));
      skeleton->bones[bone_count].parent = parseint(&sp, -1);
      bone_count++;
    }

    else if (!strcmp(s, "mesh")) {
      model->meshes[mesh_count].texture = texture;
      model->meshes[mesh_count].first = mesh_first_element;
      model->meshes[mesh_count].count = element_count - mesh_first_element;
      mesh_count++;
      mesh_first_element = element_count;
      fm = position_count;
    }

    else if (!strcmp(s, "material")) {
      s = parsestring(&sp);
      texture = loadmaterial(s);
    }

    else if (!strcmp(s, "animation")) {
      pq_is_bind_pose = 0;
    }
  }

  model->meshes[mesh_count].texture = texture;
  model->meshes[mesh_count].first = mesh_first_element;
  model->meshes[mesh_count].count = element_count - mesh_first_element;
  mesh_count++;

  /* Copy scratch data into real buffers */

  model->num_vertices = position_count;
  model->vertices = malloc(sizeof (struct ov_vertex) * position_count);
  memcpy(model->vertices, vertex_buf, sizeof (struct ov_vertex) * position_count);
  model->anivertices = NULL;

  model->num_triangles = element_count / 3;
  model->triangles = malloc(sizeof (int) * element_count);
  memcpy(model->triangles, element_buf, sizeof(int) * element_count);

  model->num_meshes = mesh_count;
  skeleton->num_bones = bone_count;

  assert(bone_count == pose_count);

  /* Compute inverse bind matrix */

  float bind_matrix[MAXBONES][16];

  for (i = 0; i < bone_count; i++) {
    float m[16];
    mat_from_pose(m,
      skeleton->bones[i].bind_pose.position,
      skeleton->bones[i].bind_pose.rotate,
      skeleton->bones[i].bind_pose.scale);
    if (skeleton->bones[i].parent != -1)
      mat_mul44(bind_matrix[i], bind_matrix[skeleton->bones[i].parent], m);
    else
      mat_copy(bind_matrix[i], m);
    mat_invert(skeleton->bones[i].inv_bind_matrix, bind_matrix[i]);
  }

  model->skeleton = skeleton;
  return model;
}

struct ov_animation *
ov_load_animation_iqe(const char *filename)
{
  FILE *fp;
  char line[256];
  char *sp;
  char *s;

  struct ov_animation *anim = malloc(sizeof *anim);
  struct ov_skeleton  *skeleton = malloc(sizeof *skeleton);
  anim->frames = NULL;

  int pq_is_bind_pose = 1;

  int pose_count = 0;
  int bone_count = 0;
  int frame_index = -1;

  fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "error: cannot load model '%s'\n", filename);
    exit(1);
  }

  if (!fgets(line, sizeof line, fp)) {
    fprintf(stderr, "cannot load %s: read error\n", filename);
    exit(1);
  }

  if (memcmp(line, IQE_MAGIC, strlen(IQE_MAGIC))) {
    fprintf(stderr, "cannot load %s: bad iqe magic\n", filename);
    exit(1);
  }

  while (1) {
    if (!fgets(line, sizeof line, fp))
      break;

    sp = line;

    s = parseword(&sp);
    if (!s)
      continue;

    else if (s[0] == 'p' && s[1] == 'q' && s[2] == 0) {
      if (!pq_is_bind_pose) {
        anim->frames[frame_index][pose_count].position[0] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].position[1] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].position[2] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].rotate[0] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].rotate[1] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].rotate[2] = parsefloat(&sp, 0);
        anim->frames[frame_index][pose_count].rotate[3] = parsefloat(&sp, 1);
        anim->frames[frame_index][pose_count].scale[0] = parsefloat(&sp, 1);
        anim->frames[frame_index][pose_count].scale[1] = parsefloat(&sp, 1);
        anim->frames[frame_index][pose_count].scale[2] = parsefloat(&sp, 1);
        pose_count++;
      }
    }

    else if (!strcmp(s, "joint")) {
      skeleton->bones[bone_count].name = strdup(parsestring(&sp));
      skeleton->bones[bone_count].parent = parseint(&sp, -1);
      bone_count++;
    }

    else if (!strcmp(s, "animation")) {
      pq_is_bind_pose = 0;
      anim->name = strdup(parsestring(&sp));
    }

    else if (!strcmp(s, "frame")) {
      pq_is_bind_pose = 0;
      pose_count = 0;
      frame_index++;
      anim->frames = realloc(anim->frames, sizeof(struct ov_pose*) * (frame_index + 1));
      anim->frames[frame_index] = malloc(sizeof(struct ov_pose) * bone_count);
    }
  }

  skeleton->num_bones = bone_count;
  anim->num_frames = frame_index + 1;

  anim->skeleton = skeleton;
  return anim;
}
