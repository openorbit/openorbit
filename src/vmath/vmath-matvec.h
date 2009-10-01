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
#include <vmath/vmath-matvec.inl>

static inline float3
vf3_set(float x, float y, float z)
{
  float3_u uc = {.a = {x,y,z,0.0}};
  return uc.v;
}
static inline float4
vf4_set(float x, float y, float z, float w)
{
  float4_u uc = {.a = {x,y,z,w}};
  return uc.v;
}

static inline float
vf3_get(float3 v, short i)
{
  float3_u uc = {.v = v};
  return uc.a[i];
}

static inline float
vf4_get(float4 v, short i)
{
  float4_u uc = {.v = v};
  return uc.a[i];
}


static inline double
vd3_get(double3 v, short i)
{
  double3_u uc = {.v = v};
  return uc.a[i];
}
static inline double
vd4_get(double4 v, short i)
{
  double4_u uc = {.v = v};
  return uc.a[i];
}


static inline double3
vd3_set(double x, double y, double z)
{
  double3_u uc = {.a = {x,y,z,0.0}};
  return uc.v;
}
static inline double4
vd4_set(double x, double y, double z, double w)
{
  double4_u uc = {.a = {x,y,z,w}};
  return uc.v;
}

static inline float
vf3_abs(float3 v)
{
  float3 res = v * v;
  return sqrt(vf3_get(res, 0) + vf3_get(res, 1) + vf3_get(res, 2));
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
  return vf3_get(res, 0) + vf3_get(res, 1) + vf3_get(res, 2);
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


vector_t v_neg(vector_t v) __attribute__((__pure__));

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




scalar_t v_dot(const vector_t a, const vector_t b)
    __attribute__ ((__pure__));


vector_t m_v_mul(const matrix_t *a, const vector_t v);

    
void m_transpose(matrix_t *mt, const matrix_t *m) __attribute__ ((__nonnull__));


void m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
    __attribute__ ((__nonnull__));    
    
void m_add(matrix_t *res, matrix_t *a, matrix_t *b)
    __attribute__ ((__nonnull__));

vector_t v_s_add(vector_t a, scalar_t b);

vector_t v_add(vector_t a, const vector_t b)
    __attribute__ ((__pure__));


vector_t v_sub(vector_t a, const vector_t b)
    __attribute__ ((__pure__));


vector_t v_s_mul(vector_t a, scalar_t s)
    __attribute__ ((__pure__));


vector_t v_s_div(vector_t a, scalar_t s)
    __attribute__ ((__pure__));


static inline float3
vf3_s_add(float3 a, float b)
{
  float4 bv = vf4_set(b,b,b,0.0f);
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
vf3_s_mul(float3 a, float b)
{
  float4 bv = vf4_set(b,b,b,1.0f);
  return a * bv;
}

static inline double3
vd3_s_mul(double3 a, double b)
{
  double4 bv = vd3_set(b,b,b);
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
  float4 bv = vf4_set(b,b,b,1.0f);
  return a / bv;
}

static inline double3
vd3_s_div(double3 a, double b)
{
  double4 bv = vd4_set(b,b,b,1.0f);
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
    
vector_t v_cross(vector_t a, vector_t b)
    __attribute__ ((__pure__));
    
vector_t v_normalise(vector_t v) __attribute__ ((__pure__));
    
 scalar_t v_abs(const vector_t v)
    __attribute__ ((__pure__));

/*! Compute determinant of 4x4 matrix M */
scalar_t m_det(const matrix_t *M);
/*! Compute subdet with respect to position k, l */
scalar_t m_subdet3(const matrix_t *M, int k, int l);

/*! Compute inverse of 4x4 matrix M */
matrix_t m_inv(const matrix_t *M);


/* creates rotation matrices, these are untested and might not work */
void m_axis_rot_x(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));
void m_axis_rot_y(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));
void m_axis_rot_z(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));

void m_vec_rot_x(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));
void m_vec_rot_y(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));
void m_vec_rot_z(matrix_t *m, scalar_t a) __attribute__ ((__nonnull__));



/* creates unit matrix */

void m_unit(matrix_t *m) __attribute__ ((__nonnull__));

/* creates zero matrix */
    
void m_zero(matrix_t *m) __attribute__ ((__nonnull__));

/* copying functions for duplicating matrices and vectors */    
void m_cpy(matrix_t * restrict dst, matrix_t * restrict src) __attribute__ ((__nonnull__));

vector_t
v_set(scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
    __attribute__ ((__pure__));


/* Comparative functions */
/*! Compares two vectors for elementvise equality, with a given absolute
 *  tolerance */
     
bool v_eq(vector_t a, vector_t b, scalar_t tol)
    __attribute__ ((__pure__));

/*! Compares two matrices for elementvise equality, with a given absolute
 *  tolerance */
bool m_eq(const matrix_t *a, const matrix_t *b, scalar_t tol)
    __attribute__ ((__pure__, __nonnull__));


void m_translate(matrix_t *m, scalar_t x, scalar_t y, scalar_t z, scalar_t w);

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


#ifdef __cplusplus
}
#endif 

#endif
