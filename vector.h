#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdio.h>

void mat_print       (FILE *, float m[16]);
void mat_identity    (float m[16]);
void mat_copy        (float p[16], const float q[16]);
void mat_mul44       (float m[16], const float n[16], const float o[16]);
void mat_mul         (float m[16], const float a[16], const float b[16]);
void mat_frustum     (float m[16],
                      float left, float right,
                      float bottom, float top,
                      float nearval, float farval);
void mat_perspective (float m[16],float fov, float aspect, float near, float far);
void mat_ortho       (float m[16],
                      float left, float right,
                      float bottom, float top,
                      float nearval, float farval);
void mat_scale       (float m[16], float x, float y, float z);
void mat_translate   (float m[16], float x, float y, float z);
void mat_rotate_x    (float p[16], float angle);
void mat_transpose   (float to[16], const float from[16]);
void mat_invert      (float out[16], const float m[16]);
void mat_vec_mul     (float p[3], const float m[16], const float v[3]);
void mat_vec_mul_n   (float p[3], const float m[16], const float v[3]);
void mat_vec_mul_t   (float p[3], const float m[16], const float v[3]);

void vec_scale       (float p[3], const float v[3], float s);
void vec_add         (float p[3], const float a[3], const float b[3]);
void vec_lerp        (float p[3], const float a[3], const float b[3], float t);
void vec_average     (float p[3], const float a[3], const float b[3]);
void vec_cross       (float p[3], const float a[3], const float b[3]);

float vec_dot        (const float a[3], const float b[3]);
float vec_dist2      (const float a[3], const float b[3]);
float vec_dist       (const float a[3], const float b[3]);

void vec_normalize   (float v[3]);
void vec_face_normal (float n[3], const float *p0, const float *p1, const float *p2);
void vec_yup_to_zup  (float v[3]);

void quat_normalize      (float q[4]);
void quat_lerp           (float p[4], const float a[4], const float b[4], float t);
void quat_lerp_normalize (float p[4], const float a[4], const float b[4], float t);

void quat_lerp_neighbor_normalize(float p[4], const float a[4], const float b[4], float t);

//void mat_from_quat_vec(float m[16], const float q[4], const float v[3]);
void mat_from_pose(float m[16], const float q[3], const float v[4], const float s[3]);

#endif
