/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/


/*!
 * \file vmath-matvec.h
 * \brief Declarations of the linear algebra routines.
 *
 * In Open Orbit, there is a lot of linear algebra going on. Especially in the
 * graphics and physict engines. This file stores the declarations of common
 * useful linear algebra routines.
 * */

#ifndef __MATH_VEC_H__
#define __MATH_VEC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <math.h>
#include <vmath/vmath-types.h>
  //#include <vmath/vmath-matvec.inl>

void vf3_outprod(float3x3 m, float3 a, float3 b);

static inline float
vf3_x(float3 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.x;
#else
  float3_u vu = {.v = v};
  return vu.s.x;
#endif
}

static inline float
vf3_y(float3 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.y;
#else
  float3_u vu = {.v = v};
  return vu.s.y;
#endif
}

static inline float
vf3_z(float3 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.z;
#else
  float3_u vu = {.v = v};
  return vu.s.z;
#endif
}


static inline float
vf4_x(float4 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.x;
#else
  float4_u vu = {.v = v};
  return vu.s.x;
#endif
}

static inline float
vf4_y(float4 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.y;
#else
  float4_u vu = {.v = v};
  return vu.s.y;
#endif
}

static inline float
vf4_z(float4 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.z;
#else
  float4_u vu = {.v = v};
  return vu.s.z;
#endif
}


static inline float
vf4_w(float4 v)
{
#if __has_feature(attribute_ext_vector_type)
  return v.w;
#else
  float4_u vu = {.v = v};
  return vu.s.w;
#endif
}


static inline void
vf3_set_x(float3 *v, float x)
{
  float *fp = (float*)v;
  fp[0] = x;
}

static inline void
vf3_set_y(float3 *v, float y)
{
  float *fp = (float*)v;
  fp[1] = y;
}

static inline void
vf3_set_z(float3 *v, float z)
{
  float *fp = (float*)v;
  fp[2] = z;
}



static inline float3
vf3_set(float x, float y, float z)
{
#if __has_feature(attribute_ext_vector_type)
  float3 v = {x, y, z};
  return v;
#else
  float3_u uc = {.a = {x,y,z,0.0}};
  return uc.v;
#endif
}

static inline void
vf3_seti(float3 *v, int i, float val)
{
#if __has_feature(attribute_ext_vector_type)
  v[i] = val;
#else
  ((float*)v)[i] = val;
#endif
}


static inline float4
vf4_set(float x, float y, float z, float w)
{
#if __has_feature(attribute_ext_vector_type)
  float4 v = {x, y, z, w};
  return v;
#else
  float4_u uc = {.a = {x,y,z,w}};
  return uc.v;
#endif
}

static inline float
vf3_get(float3 v, short i)
{
#if __has_feature(attribute_ext_vector_type)
  return v[i];
#else
  float3_u uc = {.v = v};
  return uc.a[i];
#endif
}

static inline float
vf4_get(float4 v, short i)
{
#if __has_feature(attribute_ext_vector_type)
  return v[i];
#else
  float4_u uc = {.v = v};
  return uc.a[i];
#endif
}


static inline double
vd3_get(double3 v, short i)
{
#if __has_feature(attribute_ext_vector_type)
  return v[i];
#else
  double3_u uc = {.v = v};
  return uc.a[i];
#endif
}
static inline double
vd4_get(double4 v, short i)
{
#if __has_feature(attribute_ext_vector_type)
  return v[i];
#else
  double4_u uc = {.v = v};
  return uc.a[i];
#endif
}


static inline double3
vd3_set(double x, double y, double z)
{
#if __has_feature(attribute_ext_vector_type)
  double3 v = {x, y, z};
  return v;
#else
  double3_u uc = {.a = {x,y,z,0.0}};
  return uc.v;
#endif
}
static inline double4
vd4_set(double x, double y, double z, double w)
{
#if __has_feature(attribute_ext_vector_type)
  double4 v = {x, y, z, w};
  return v;
#else
  double4_u uc = {.a = {x,y,z,w}};
  return uc.v;
#endif
}

static inline float
vf3_abs(float3 v)
{
  float3 res = v * v;
  return sqrt(vf3_get(res, 0) + vf3_get(res, 1) + vf3_get(res, 2));
}

static inline float
vf4_abs(float4 v)
{
  float4 res = v * v;
  return sqrt(vf4_get(res, 0) + vf4_get(res, 1) + vf4_get(res, 2) + vf4_get(res, 3));
}


static inline double
vd3_abs(double3 v)
{
  double3 res = v * v;
  return sqrt(vd3_get(res, 0) + vd3_get(res, 1) + vd3_get(res, 2));
}

static inline float
vf3_abs_square(float3 v)
{
  float3 res = v * v;
  return vf3_x(res) + vf3_y(res) + vf3_z(res);
}

static inline double
vd3_abs_square(double3 v)
{
  double3 res = v * v;
  return vd3_get(res, 0) + vd3_get(res, 1) + vd3_get(res, 2);
}

static inline float3
vf3_neg(float3 v)
{
  return -v;
}

static inline float4
vf4_neg(float4 v)
{
  return -v;
}

static inline double3
vd3_neg(double3 v)
{
  return -v;
}

static inline double4
vd4_neg(double4 v)
{
  return -v;
}


float4 v_neg(float4 v) __attribute__((__pure__));

static inline float
vf3_dot(float3 a, float3 b)
{
  float3 c = a * b;
  float3_u uc = {.v = c};

  return uc.a[0] + uc.a[1] + uc.a[2];
}

static inline float
vf4_dot(float4 a, float4 b)
{
  float4 c = a * b;
  float4_u uc = {.v = c};

  return uc.a[0] + uc.a[1] + uc.a[2] + uc.a[3];
}


static inline double
vd3_dot(double3 a, double3 b)
{
  double3 c = a * b;
  double3_u uc = {.v = c};

  return uc.a[0] + uc.a[1] + uc.a[2];
}

static inline float
vd4_dot(double3 a, double3 b)
{
  double3 c = a * b;
  double3_u uc = {.v = c};

  return uc.a[0] + uc.a[1] + uc.a[2] + uc.a[3];
}




float vf3_dot(float3 a, float3 b)
    __attribute__ ((__pure__));


float4 m_v_mul(const matrix_t *a, float4 v);
float3 m_v3_mulf(const matrix_t *a, float3 v);
float3 mf3_v_mul(const float3x3 a, float3 v);

void m_transpose(matrix_t *mt, const matrix_t *m) __attribute__ ((__nonnull__));


void m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
    __attribute__ ((__nonnull__));

void m_add(matrix_t *res, matrix_t *a, matrix_t *b)
    __attribute__ ((__nonnull__));

void m_sub(matrix_t *res, matrix_t *a, matrix_t *b)
    __attribute__ ((__nonnull__));

float4 v_s_add(float4 a, float b);

float4 v_add(float4 a, float4 b)
    __attribute__ ((__pure__));


float4 v_sub(float4 a, float4 b)
    __attribute__ ((__pure__));


float4 v_s_mul(float4 a, float s)
    __attribute__ ((__pure__));


float4 v_s_div(float4 a, float s)
    __attribute__ ((__pure__));


static inline float3
vf3_s_add(float3 a, float b)
{
  float3 bv = vf3_set(b,b,b);
  return a + bv;
}

static inline float4
vf4_s_add(float4 a, float b)
{
  float4 bv = vf4_set(b,b,b,b);
  return a + bv;
}
static inline float3
vf3_add(float3 a, float3 b)
{
  return a + b;
}

static inline float4
vf4_add(float4 a, float4 b)
{
  return a + b;
}

static inline double3
vd3_add(double3 a, double3 b)
{
  return a + b;
}

static inline double3
vd3_s_mod(double3 a, double b)
{
  double3_u uc = {.v = a};
  double3_u res;

  res.a[0] = fmod(uc.a[0], b);
  res.a[1] = fmod(uc.a[1], b);
  res.a[2] = fmod(uc.a[2], b);

  return res.v;
}

static inline double3
vd3_copysign(double3 a, double3 b)
{
  double3_u uc = {.v = a};
  double3_u ub = {.v = b};

  double3_u res;

  res.a[0] = copysign(uc.a[0], ub.a[0]);
  res.a[1] = copysign(uc.a[1], ub.a[1]);
  res.a[2] = copysign(uc.a[2], ub.a[2]);

  return res.v;
}

static inline float3
vf3_sub(float3 a, float3 b)
{
  return a - b;
}

static inline float3
vf3_repr(float3 a)
{
  float3 ones = vf3_set(1.0f, 1.0f, 1.0f);
  return ones / a;
}

static inline float4
vf4_sub(float4 a, float4 b)
{
  return a - b;
}

static inline double3
vd3_sub(double3 a, double3 b)
{
  return a - b;
}

static inline float3
vf3_mul(float3 a, float3 b)
{
  return a * b;
}


static inline float3
vf3_s_mul(float3 a, float b)
{
  float3 bv = vf3_set(b,b,b);
  return a * bv;
}

static inline double3
vd3_s_mul(double3 a, double b)
{
  double3 bv = vd3_set(b,b,b);
  return a * bv;
}


static inline float4
vf4_s_mul(float4 a, float b)
{
  float4 bv = vf4_set(b,b,b,b);
  return a * bv;
}

static inline float3
vf3_s_div(float3 a, float b)
{
  float3 bv = vf3_set(b,b,b);
  return a / bv;
}

static inline double3
vd3_s_div(double3 a, double b)
{
  double3 bv = vd3_set(b,b,b);
  return a / bv;
}


static inline float4
vf4_s_div(float4 a, float b)
{
  float4 bv = vf4_set(b,b,b,b);
  return a / bv;
}

static inline double3
vd3_abs_c(double3 a)
{
  double3_u ua = {.v = a};

  double3 absa = vd3_set(fabs(ua.a[0]), fabs(ua.a[1]), fabs(ua.a[2]));
  return absa;
}

/* This is really a 3d x product */

float3 vf3_cross(float3 a, float3 b)
    __attribute__ ((__pure__));

float4 v_normalise(float4 v) __attribute__ ((__pure__));

 float v_abs(const float4 v)
    __attribute__ ((__pure__));

/*! Compute determinant of 4x4 matrix M */
float m_det(const matrix_t *M);
/*! Compute subdet with respect to position k, l */
float m_subdet3(const matrix_t *M, int k, int l);

/*! Compute inverse of 4x4 matrix M */
matrix_t m_inv(const matrix_t *M);


/* creates rotation matrices, these are untested and might not work */
void m_axis_rot_x(matrix_t *m, float a) __attribute__ ((__nonnull__));
void m_axis_rot_y(matrix_t *m, float a) __attribute__ ((__nonnull__));
void m_axis_rot_z(matrix_t *m, float a) __attribute__ ((__nonnull__));

void m_vec_rot_x(matrix_t *m, float a) __attribute__ ((__nonnull__));
void m_vec_rot_y(matrix_t *m, float a) __attribute__ ((__nonnull__));
void m_vec_rot_z(matrix_t *m, float a) __attribute__ ((__nonnull__));

void m_rot(matrix_t *m, float x, float y, float z, float alpha);


/* creates unit matrix */

void m_unit(matrix_t *m) __attribute__ ((__nonnull__));

/* creates zero matrix */

void m_zero(matrix_t *m) __attribute__ ((__nonnull__));

/* copying functions for duplicating matrices and vectors */
void m_cpy(matrix_t * restrict dst, const matrix_t * restrict src)
  __attribute__ ((__nonnull__));


/* Comparative functions */
/*! Compares two vectors for elementvise equality, with a given absolute
 *  tolerance */

bool v_eq(float4 a, float4 b, float tol)
    __attribute__ ((__pure__));

static inline bool
vf3_lt(float3 a, float3 b)
{
  return vf3_abs_square(a) < vf3_abs_square(b);
}

static inline bool
vf3_lte(float3 a, float3 b)
{
  return vf3_abs_square(a) <= vf3_abs_square(b);
}

static inline bool
vf3_gt(float3 a, float3 b)
{
  return vf3_abs_square(a) > vf3_abs_square(b);
}


/*! Compares two matrices for elementvise equality, with a given absolute
 *  tolerance */
bool m_eq(const matrix_t *a, const matrix_t *b, float tol)
    __attribute__ ((__pure__, __nonnull__));


void m_translate(matrix_t *m, float x, float y, float z, float w);

static inline float3
vf3_normalise(float3 v)
{
  float norm = vf3_abs(v);
  return vf3_s_mul(v, 1.0f/norm);
}

static inline double3
vd3_normalise(double3 v)
{
  double norm = vd3_abs(v);
  return vd3_s_mul(v, 1.0/norm);
}

static inline void
m_s_mul(matrix_t *res, const matrix_t *a, float s)
{
  float4 vs = vf4_set(s, s, s, s);
  res->v[0] = vf3_mul(a->v[0], vs);
  res->v[1] = vf3_mul(a->v[1], vs);
  res->v[2] = vf3_mul(a->v[2], vs);
  res->v[3] = vf3_mul(a->v[3], vs);
}

void mf3_ident(float3x3 m);
void mf3_add2(float3x3 a, const float3x3 b);
void mf3_add(float3x3 a, const float3x3 b, const float3x3 c);

void mf3_cpy(float3x3 dst, const float3x3 src);
void mf3_rot(float3x3 m, float x, float y, float z, float alpha);
void md3_rot(double3x3 m, double x, double y, double z, double alpha);
void mf3_transpose2(float3x3 a, const float3x3 b);
void mf3_mul2(float3x3 a, const float3x3 b);
void mf3_mul3(float3x3 a, const float3x3 b, const float3x3 c);
void mf3_inv2(float3x3 invmat, const float3x3 mat);
void mf3_inv1(float3x3 mat);
void mf3_sub(float3x3 a, const float3x3 b, const float3x3 c);
void mf3_s_mul(float3x3 res, const float3x3 m, float s);

void mf3_basis(float3x3 res, const float3x3 m, const float3x3 b);


#ifdef __cplusplus
}
#endif

#endif
