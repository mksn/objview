#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
#include <unit.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/freeglut.h>
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define CLAMP(x,a,b) MIN(MAX(x,a),b)

/*
 * Some vector and matrix math.
 */

typedef float vec3[3];
typedef float vec4[4];
typedef float mat4[16];

struct pose {
	vec3 position;
	vec4 rotation;
	vec3 scale;
};

#define A(row,col) a[(col<<2)+row]
#define B(row,col) b[(col<<2)+row]
#define M(row,col) m[(col<<2)+row]

static void mat_copy(mat4 p, const mat4 m)
{
	memcpy(p, m, sizeof(mat4));
}

static void mat_mul(mat4 m, const mat4 a, const mat4 b)
{
	int i;
	for (i = 0; i < 4; i++) {
		const float ai0=A(i,0), ai1=A(i,1), ai2=A(i,2), ai3=A(i,3);
		M(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
		M(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
		M(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
		M(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
	}
}

static void mat_invert(mat4 out, const mat4 m)
{
	mat4 inv;
	float det;
	int i;

	inv[0] = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] +
		m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] -
		m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] +
		m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] -
		m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] -
		m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] +
		m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] -
		m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] +
		m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] +
		m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
	inv[6] = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] -
		m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
	inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] +
		m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] -
		m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
	inv[3] = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] -
		m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
	inv[7] = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] +
		m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] -
		m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
	inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] +
		m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
	assert (det != 0);
	det = 1.0 / det;
	for (i = 0; i < 16; i++)
		out[i] = inv[i] * det;
}

static void mat_from_pose(mat4 m, const vec3 t, const vec4 q, const vec3 s)
{
	float x2 = q[0] + q[0];
	float y2 = q[1] + q[1];
	float z2 = q[2] + q[2];
	{
		float xx2 = q[0] * x2;
		float yy2 = q[1] * y2;
		float zz2 = q[2] * z2;
		M(0,0) = 1 - yy2 - zz2;
		M(1,1) = 1 - xx2 - zz2;
		M(2,2) = 1 - xx2 - yy2;
	}
	{
		float yz2 = q[1] * z2;
		float wx2 = q[3] * x2;
		M(2,1) = yz2 + wx2;
		M(1,2) = yz2 - wx2;
	}
	{
		float xy2 = q[0] * y2;
		float wz2 = q[3] * z2;
		M(1,0) = xy2 + wz2;
		M(0,1) = xy2 - wz2;
	}
	{
		float xz2 = q[0] * z2;
		float wy2 = q[3] * y2;
		M(0,2) = xz2 + wy2;
		M(2,0) = xz2 - wy2;
	}

	m[0] *= s[0]; m[4] *= s[1]; m[8] *= s[2];
	m[1] *= s[0]; m[5] *= s[1]; m[9] *= s[2];
	m[2] *= s[0]; m[6] *= s[1]; m[10] *= s[2];

	M(0,3) = t[0];
	M(1,3) = t[1];
	M(2,3) = t[2];

	M(3,0) = 0;
	M(3,1) = 0;
	M(3,2) = 0;
	M(3,3) = 1;
}

#undef A
#undef B
#undef M

static float vec_dist2(const vec3 a, const vec3 b)
{
	float d0, d1, d2;
	d0 = a[0] - b[0];
	d1 = a[1] - b[1];
	d2 = a[2] - b[2];
	return d0 * d0 + d1 * d1 + d2 * d2;
}

static void vec_scale(vec3 p, const vec3 v, float s)
{
	p[0] = v[0] * s;
	p[1] = v[1] * s;
	p[2] = v[2] * s;
}

static void vec_add(vec3 p, const vec3 a, const vec3 b)
{
	p[0] = a[0] + b[0];
	p[1] = a[1] + b[1];
	p[2] = a[2] + b[2];
}

static void mat_vec_mul(vec3 p, const mat4 m, const vec3 v)
{
	assert(p != v);
	p[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2] + m[12];
	p[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2] + m[13];
	p[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2] + m[14];
}

static void mat_vec_mul_n(vec3 p, const mat4 m, const vec3 v)
{
	assert(p != v);
	p[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
	p[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
	p[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
}

static void calc_mul_matrix(mat4 *skin_matrix,
                            mat4 *abs_pose_matrix,
                            mat4 *inv_bind_matrix,
                            int count)
{
	int i;
	for (i = 0; i < count; i++)
		mat_mul(skin_matrix[i],
            abs_pose_matrix[i],
            inv_bind_matrix[i]);
}

static void calc_inv_matrix(mat4 *inv_bind_matrix,
                            mat4 *abs_bind_matrix,
                            int count)
{
	int i;
	for (i = 0; i < count; i++)
		mat_invert(inv_bind_matrix[i], abs_bind_matrix[i]);
}

static void calc_abs_matrix(mat4 *abs_pose_matrix,
                            mat4 *pose_matrix,
                            int *parent,
                            int count)
{
	int i;
	for (i = 0; i < count; i++)
		if (parent[i] >= 0)
			mat_mul(abs_pose_matrix[i],
              abs_pose_matrix[parent[i]],
              pose_matrix[i]);
		else
			mat_copy(abs_pose_matrix[i],
               pose_matrix[i]);
}

static void calc_matrix_from_pose(mat4 *pose_matrix,
                                  struct pose *pose,
                                  int count)
{
	int i;
	for (i = 0; i < count; i++)
		mat_from_pose(pose_matrix[i],
                  pose[i].position,
                  pose[i].rotation,
                  pose[i].scale);
}

char basedir[2000];

static unsigned char checker_data[256*256];
static unsigned int checker_texture = 0;
/*
  static void initchecker(void)
  {
  int x, y, i = 0;
  for (y = 0; y < 256; y++) {
  for (x = 0; x < 256; x++) {
  int k = ((x>>5) & 1) ^ ((y>>5) & 1);
  checker_data[i++] = k ? 255 : 192;
  }
  }
  glGenTextures(1, &checker_texture);
  glBindTexture(GL_TEXTURE_2D, checker_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, 1, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, checker_data);
  }

  static void lowerstring(char *s)
  {
  while (*s) { *s = tolower(*s); s++; }
  }

  unsigned int loadtexture(char *filename)
  {
  unsigned int texture;
  unsigned char *image;
  int w, h, n, intfmt = 0, fmt = 0;

  image = stbi_load(filename, &w, &h, &n, 0);
  if (!image) {
  lowerstring(filename);
  image = stbi_load(filename, &w, &h, &n, 0);
  if (!image) {
  fprintf(stderr, "cannot load texture '%s'\n", filename);
  return 0;
  }
  }

  if (n == 1) { intfmt = fmt = GL_LUMINANCE; }
  if (n == 2) { intfmt = fmt = GL_LUMINANCE_ALPHA; }
  if (n == 3) { intfmt = fmt = GL_RGB; }
  if (n == 4) { intfmt = fmt = GL_RGBA; }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, intfmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, image);
  //glGenerateMipmap(GL_TEXTURE_2D);

  free(image);

  return texture;
  }
  unsigned int loadmaterial(char *material)
  {
  int texture;
  char filename[2000], *s;
  s = strrchr(material, ';');
  if (s) material = s + 1;
  sprintf(filename, "%s/%s.png", basedir, material);
  texture = load_texture(filename);
  if (texture)
  return texture;
  sprintf(filename, "%s/textures/%s.png", basedir, material);
  texture = loadtexture(filename);
  if (texture)
  return texture;
  return checker_texture;
  }
*/
/*
 * IQE loading and drawing
 */

#define IQE_MAGIC "# Inter-Quake Export"
#define MAXBONE 256

/*
  struct model {
  struct skel *skel;
  struct mesh *mesh;
  struct anim *anim;
  };

  struct skel {
  int count;
  int parent[MAXBONE];
  char *name[MAXBONE];
  struct pose pose[MAXBONE];
  };

  struct mesh {
  int vertex_count;
  float *position, *normal, *texcoord, *color;
  float *blendweight;
  int *blendindex;

  int element_count;
  int *element;

  int part_count;
  struct part *part;

  float *aposition, *anormal;

  mat4 abs_bind_matrix[MAXBONE];
  mat4 inv_bind_matrix[MAXBONE];
  };

  struct part {
  unsigned int material;
  int first, count;
  };

  struct anim {
  char *name;
  int len, cap;
  struct pose **data;
  struct anim *prev, *next;
  };
*/
struct part {
	unsigned int material;
	int first, count;
};

struct floatarray {
	int len, cap;
	float *data;
};

struct intarray {
	int len, cap;
	int *data;
};

struct partarray {
	int len, cap;
	struct part *data;
};

/* global scratch buffers */
static struct floatarray position = { 0, 0, NULL };
static struct floatarray normal = { 0, 0, NULL };
static struct floatarray texcoord = { 0, 0, NULL };
static struct floatarray color = { 0, 0, NULL };
static struct intarray blendindex = { 0, 0, NULL };
static struct floatarray blendweight = { 0, 0, NULL };
static struct intarray element = { 0, 0, NULL };
static struct partarray partbuf = { 0, 0, NULL };

static void *duparray(void *data, int count, int size)
{
	if (count == 0)
		return NULL;
	void *p = malloc(count * size);
	memcpy(p, data, count * size);
	return p;
}

static inline void pushfloat(struct floatarray *a, float v)
{
	if (a->len + 1 >= a->cap) {
		a->cap = 600 + a->cap * 2;
		a->data = realloc(a->data, a->cap * sizeof(*a->data));
	}
	a->data[a->len++] = v;
}

static inline void pushint(struct intarray *a, int v)
{
	if (a->len + 1 >= a->cap) {
		a->cap = 600 + a->cap * 2;
		a->data = realloc(a->data, a->cap * sizeof(*a->data));
	}
	a->data[a->len++] = v;
}

static void pushpart(struct partarray *a,
                     int first,
                     int last,
                     int material)
{
	/* merge parts if they share materials */
	if (a->len > 0 && a->data[a->len-1].material == material) {
		a->data[a->len-1].count += last - first;
		return;
	}
	if (a->len + 1 >= a->cap) {
		a->cap = 600 + a->cap * 2;
		a->data = realloc(a->data, a->cap * sizeof(*a->data));
	}
	a->data[a->len].first = first;
	a->data[a->len].count = last - first;
	a->data[a->len].material = material;
	a->len++;
}

static struct ov_animations *pushanim(struct ov_animations *head,
                                      char *name)
{
	struct ov_animation *anim = malloc(sizeof *anim);
	anim->name = strdup(name);
	anim->count = anim->cap = 0;
	anim->poses = NULL;
	return head;
}

static struct ov_pose *pushframe(struct ov_animation *a,
                                 int bone_count)
{
	struct ov_pose *pose = malloc(sizeof(struct ov_pose) * bone_count);;
	if (a->count + 1 >= a->cap) {
		a->cap = 128 + a->cap * 2;
		a->poses = realloc(a->poses, a->cap * sizeof(*a->poses));
	}
	a->poses[a->num_frames++] = pose;
	return pose;
}

static void addposition(float x, float y, float z)
{
	pushfloat(&position, x);
	pushfloat(&position, y);
	pushfloat(&position, z);
}

static void addnormal(float x, float y, float z)
{
	pushfloat(&normal, x);
	pushfloat(&normal, y);
	pushfloat(&normal, z);
}

static void addtexcoord(float u, float v)
{
	pushfloat(&texcoord, u);
	pushfloat(&texcoord, v);
}

static void addcolor(float x, float y, float z, float w)
{
	pushfloat(&color, x);
	pushfloat(&color, y);
	pushfloat(&color, z);
	pushfloat(&color, w);
}

static void addblend(int a, int b, int c, int d, float x, float y, float z, float w)
{
	float total = x + y + z + w;
	pushint(&blendindex, a);
	pushint(&blendindex, b);
	pushint(&blendindex, c);
	pushint(&blendindex, d);
	pushfloat(&blendweight, x / total);
	pushfloat(&blendweight, y / total);
	pushfloat(&blendweight, z / total);
	pushfloat(&blendweight, w / total);
}

static void addtriangle(int a, int b, int c)
{
	// flip triangle winding
	pushint(&element, c);
	pushint(&element, b);
	pushint(&element, a);
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

struct ov_animation *model_iqe_load_animation(char *filename) {
}

struct ov_model *model_iqe_load_model(char *filename)
{
	static mat4 loc_bind_matrix[MAXBONE];

	FILE *fp;
	char line[256];
	int material = 0;
	int first = 0;
	int fm = 0;
	char *s, *sp;
 
  struct ov_unit       *du        = malloc(sizeof *du);
	struct ov_skeleton   *skel      = malloc(sizeof *skel);
	struct ov_mesh       *mesh      = malloc(sizeof *mesh);
	struct ov_animations *anims     = malloc(sizeof *anims);

	int                  pose_count = 0;
	struct ov_pose       *pose;

	fprintf(stderr, "loading iqe model '%s'\n", filename);

	skel->num_bones = 0;
	bone = skel->bones[0];

	position.len = 0;
	texcoord.len = 0;
	normal.len = 0;
	element.len = 0;
	blendindex.len = 0;
	blendweight.len = 0;

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
		float x, y, z, w;
		int a, b, c, d;

		if (!fgets(line, sizeof line, fp))
			break;

		sp = line;

		s = parseword(&sp);
		if (!s)
			continue;

		if (s[0] == 'v' && s[1] != 0 && s[2] == 0) {
			switch (s[1]) {
        case 'p':
          x = parsefloat(&sp, 0);
          y = parsefloat(&sp, 0);
          z = parsefloat(&sp, 0);
          addposition(x, y, z);
          break;

        case 'n':
          x = parsefloat(&sp, 0);
          y = parsefloat(&sp, 0);
          z = parsefloat(&sp, 0);
          addnormal(x, y, z);
          break;

        case 't':
          x = parsefloat(&sp, 0);
          y = parsefloat(&sp, 0);
          addtexcoord(x, y);
          break;

        case 'c':
          x = parsefloat(&sp, 0);
          y = parsefloat(&sp, 0);
          z = parsefloat(&sp, 0);
          w = parsefloat(&sp, 1);
          addcolor(x, y, z, w);
          break;

        case 'b':
          a = parseint(&sp, 0);
          x = parsefloat(&sp, 1);
          b = parseint(&sp, 0);
          y = parsefloat(&sp, 0);
          c = parseint(&sp, 0);
          z = parsefloat(&sp, 0);
          d = parseint(&sp, 0);
          w = parsefloat(&sp, 0);
          addblend(a, b, c, d, x, y, z, w);
          break;
			}
		}

		else if (s[0] == 'f' && s[1] == 'm' && s[2] == 0) {
			a = parseint(&sp, 0);
			b = parseint(&sp, 0);
			c = parseint(&sp, -1);
			while (c > -1) {
				addtriangle(a+fm, b+fm, c+fm);
				b = c;
				c = parseint(&sp, -1);
			}
		}

		else if (s[0] == 'p' && s[1] == 'q' && s[2] == 0) {
			if (pose_count < MAXBONE) {
				pose[pose_count].translate[0] = parsefloat(&sp, 0);
				pose[pose_count].translate[1] = parsefloat(&sp, 0);
				pose[pose_count].translate[2] = parsefloat(&sp, 0);
				pose[pose_count].rotate[0] = parsefloat(&sp, 0);
				pose[pose_count].rotate[1] = parsefloat(&sp, 0);
				pose[pose_count].rotate[2] = parsefloat(&sp, 0);
				pose[pose_count].rotate[3] = parsefloat(&sp, 1);
				pose[pose_count].scale[0] = parsefloat(&sp, 1);
				pose[pose_count].scale[1] = parsefloat(&sp, 1);
				pose[pose_count].scale[2] = parsefloat(&sp, 1);
				pose_count++;
			}
		}

		else if (!strcmp(s, "joint")) {
			if (skel->num_bones < MAXBONE) {
				skel->bones[skel->num_bones].name = strdup(parsestring(&sp));
				skel->bones[skel->num_bones].parent = parseint(&sp, -1);
				skel->num_bones++;
			}
		}

		 else if (!strcmp(s, "animation")) { 
		 	s = parsestring(&sp); 
		 	unit = pushanim(unit, s); 
		 } 

		 else if (!strcmp(s, "frame")) { 
		 	bone = pushframe(unit, skel->count); 
		 	bone_count = 0; 
		 } 

		else if (!strcmp(s, "mesh")) {
			if (element.len > first)
				pushpart(unit, &partbuf, first, element.len, material);
			first = element.len;
			fm = position.len / 3;
		}

		else if (!strcmp(s, "material")) {
			s = parsestring(&sp);
			material = loadmaterial(s);
		}
	}

	if (element.len > first)
		pushpart(&partbuf, first, element.len, material);

	if (skel->num_bones > 0) {
		calc_matrix_from_bone(loc_bind_matrix,
                          skel->bones,
                          skel->num_bones);
		calc_abs_matrix(bone->bind_matrix,
                    loc_bind_matrix,
                    bone->parent,
                    skel->num_bones);
		calc_inv_matrix(bone->inv_bind_matrix,
                    bone->abs_bind_matrix,
                    skel->num_bones);
	}

	mesh->num_vertex = position.len / 3;
	mesh->position = duparray(position.data, position.len, sizeof(float));
	mesh->normal = duparray(normal.data, normal.len, sizeof(float));
	mesh->texcoord = duparray(texcoord.data, texcoord.len, sizeof(float));
	mesh->color = duparray(color.data, color.len, sizeof(float));
	mesh->blendindex = duparray(blendindex.data, blendindex.len, sizeof(int));
	mesh->blendweight = duparray(blendweight.data, blendweight.len, sizeof(float));
	mesh->aposition = NULL;
	mesh->anormal = NULL;

	mesh->element_count = element.len;
	mesh->element = duparray(element.data, element.len, sizeof(int));

	mesh->part_count = partbuf.len;
	mesh->part = duparray(partbuf.data, partbuf.len, sizeof(struct part));

	fprintf(stderr, "\t%d batches; %d vertices; %d triangles; %d bones\n",
          mesh->part_count, mesh->num_vertex mesh->element_count / 3, skel->num_bones);

	struct model *model = malloc(sizeof *model);
	model->skel = skel;
	model->mesh = mesh;
	model->anim = anim;
	return model;
}

static mat4 loc_bone_matrix[MAXBONE];
static mat4 abs_bone_matrix[MAXBONE];
static mat4 skin_matrix[MAXBONE];

void animatemodel(struct ov_model *model, struct ov_animation *anim, int frame)
{
	struct ov_skeleton *skel = model->skeleton;
	struct ov_mesh *mesh = model->mesh;

	frame = CLAMP(frame, 0, anim->len-1);

	calc_matrix_from_bone(loc_bone_matrix, anim->data[frame], skel->num_bones);
	calc_abs_matrix(abs_bone_matrix, loc_bone_matrix, skel->parent, skel->num_bones);
	calc_mul_matrix(skin_matrix, abs_bone_matrix, mesh->inv_bind_matrix, skel->num_bones);

	if (!mesh->aposition) mesh->aposition = malloc(sizeof(float) * mesh->num_vertex * 3);
	if (!mesh->anormal) mesh->anormal = malloc(sizeof(float) * mesh->num_vertex * 3);

	int *bi = mesh->blendindex;
	float *bw = mesh->blendweight;
	float *sp = mesh->position;
	float *sn = mesh->normal;
	float *dp = mesh->aposition;
	float *dn = mesh->anormal;
	int n = mesh->num_vertex;

	while (n--) {
		int i;
		dp[0] = dp[1] = dp[2] = 0;
		dn[0] = dn[1] = dn[2] = 0;
		for (i = 0; i < 4; i++) {
			vec3 tp, tn;
			mat_vec_mul(tp, skin_matrix[bi[i]], sp);
			mat_vec_mul_n(tn, skin_matrix[bi[i]], sn);
			vec_scale(tp, tp, bw[i]);
			vec_scale(tn, tn, bw[i]);
			vec_add(dp, dp, tp);
			vec_add(dn, dn, tn);
		}
		bi += 4; bw += 4;
		sp += 3; sn += 3;
		dp += 3; dn += 3;
	}
}

static int haschildren(int *parent, int count, int x)
{
	int i;
	for (i = x; i < count; i++)
		if (parent[i] == x)
			return 1;
	return 0;
}

void drawskeleton(struct model *model)
{
	struct skel *skel = model->skel;
	vec3 x = { 0, 0.1, 0 };
	int i;
	glBegin(GL_LINES);
	for (i = 0; i < skel->num_bones; i++) {
		float *a = abs_bone_matrix[i];
		if (skel->parent[i] >= 0) {
			float *b = abs_bone_matrix[skel->parent[i]];
			glColor4f(1, 1, 1, 1);
			glVertex3f(a[12], a[13], a[14]);
			glVertex3f(b[12], b[13], b[14]);
		} else {
			glColor4f(1, 1, 1, 1);
			glVertex3f(a[12], a[13], a[14]);
			glColor4f(0, 0, 0, 1);
			glVertex3f(0, 0, 0);
		}
		if (!haschildren(skel->parent, skel->num_bones
    , i)) {
			vec3 b;
			mat_vec_mul(b, abs_bone_matrix[i], x);
			glColor4f(1, 1, 1, 1);
			glVertex3f(a[12], a[13], a[14]);
			glColor4f(0, 0, 0, 1);
			glVertex3f(b[0], b[1], b[2]);
		}
	}
	glEnd();
}

void drawmodel(struct model *model)
{
	struct mesh *mesh = model->mesh;
	int i;

	glEnableClientState(GL_VERTEX_ARRAY);
	if (mesh->normal) glEnableClientState(GL_NORMAL_ARRAY);
	if (mesh->texcoord) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	if (mesh->color) glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, mesh->aposition ? mesh->aposition : mesh->position);
	glNormalPointer(GL_FLOAT, 0, mesh->anormal ? mesh->anormal : mesh->normal);
	glTexCoordPointer(2, GL_FLOAT, 0, mesh->texcoord);
	glColorPointer(3, GL_FLOAT, 0, mesh->texcoord);

	for (i = 0; i < mesh->part_count; i++) {
		glColor4f(1, 1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, mesh->part[i].material);
		glDrawElements(GL_TRIANGLES, mesh->part[i].count, GL_UNSIGNED_INT, mesh->element + mesh->part[i].first);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

float measuremodel(struct model *model, float center[3])
{
	struct skel *skel = model->skel;
	struct mesh *mesh = model->mesh;
	struct anim *anim;
	float dist, maxdist = 1;
	int i, k;

	center[0] = center[1] = center[2] = 0;
	for (i = 0; i < mesh->num_vertex; i++)
		vec_add(center, center, mesh->position + i * 3);
	if (mesh->num_vertex) {
		center[0] /= mesh->num_vertex;
		center[1] /= mesh->num_vertex;
		center[2] /= mesh->num_vertex;
	}

	for (i = 0; i < mesh->num_vertex; i++) {
		dist = vec_dist2(center, mesh->position + i * 3);
		if (dist > maxdist)
			maxdist = dist;
	}

	if (skel->num_bones > 0) {
		for (i = 0; i < skel->num_bones; i++) {
			dist = vec_dist2(center, mesh->abs_bind_matrix[i] + 12);
			if (dist > maxdist)
				maxdist = dist;
		}

		for (anim = model->anim; anim; anim = anim->next) {
			for (k = 0; anim && k < anim->len; k++) {
				calc_matrix_from_bone(loc_bone_matrix, anim->data[k], skel->num_bones);
				calc_abs_matrix(abs_bone_matrix, loc_bone_matrix, skel->parent, skel->num_bones);
				for (i = 0; i < skel->num_bones; i++) {
					dist = vec_dist2(center, abs_bone_matrix[i] + 12);
					if (dist > maxdist)
						maxdist = dist;
				}
			}
		}

		memcpy(abs_bone_matrix, mesh->abs_bind_matrix, sizeof abs_bone_matrix);
	}

	return sqrt(maxdist);
}
