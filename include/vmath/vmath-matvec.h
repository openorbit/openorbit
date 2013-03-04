/*
 Copyright 2006,2012,2013 Mattias Holm <lorrden(at)openorbit.org>

 This file is part of Open Orbit. Open Orbit is free software: you can
 redistribute it and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

 Some files of Open Orbit have relaxed licensing conditions. This file is
 licenced under the 2-clause BSD licence.

 Redistribution and use of this file in source and binary forms, with or
 without modification, are permitted provided that the following conditions are
 met:

 - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <string.h>
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
  float3_u uc = {.a = {x,y,z}};
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

static inline float4
vf4_setv(float3 vec, float w)
{
#if __has_feature(attribute_ext_vector_type)
  float4 v = {vec.x, vec.y, vec.z, w};
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
  return sqrtf(res.x + res.y + res.z);
}

static inline float
vf4_abs(float4 v)
{
  float4 res = v * v;
  return sqrtf(res.x + res.y + res.z + res.w);
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
  //float4_u uc = {.v = c};

  return c.x + c.y + c.z + c.w;
  //return uc.a[0] + uc.a[1] + uc.a[2] + uc.a[3];
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



float3 mf3_v_mul(const float3x3 a, float3 v);

void mf4_add(float4x4 a, const float4x4 b, const float4x4 c);

void m_sub(float4x4 res, float4x4 a, float4x4 b)
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

static inline float4
vf4_mul(float4 a, float4 b)
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
float mf4_det(const float4x4 M);
/*! Compute subdet with respect to position k, l */
float mf4_subdet3(const float4x4 m, int k, int l);

/*! Compute inverse of 4x4 matrix M */
void mf4_inv(float4x4 M_inv, const float4x4 M);

float mf3_det(const float3x3 m);


/* creates rotation matrices, these are untested and might not work */
void m_axis_rot_x(float4x4 m, float a) __attribute__ ((__nonnull__));
void m_axis_rot_y(float4x4 m, float a) __attribute__ ((__nonnull__));
void m_axis_rot_z(float4x4 m, float a) __attribute__ ((__nonnull__));

void m_vec_rot_x(float4x4 m, float a) __attribute__ ((__nonnull__));
void m_vec_rot_y(float4x4 m, float a) __attribute__ ((__nonnull__));
void m_vec_rot_z(float4x4 m, float a) __attribute__ ((__nonnull__));

void m_rot(float4x4 m, float x, float y, float z, float alpha);


/* creates unit matrix */

void m_unit(float4x4 m) __attribute__ ((__nonnull__));

/* creates zero matrix */

void m_zero(float4x4 m) __attribute__ ((__nonnull__));

/* copying functions for duplicating matrices and vectors */
void m_cpy(float4x4 dst, const float4x4 src)
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
bool m_eq(const float4x4 a, const float4x4 b, float tol)
    __attribute__ ((__pure__, __nonnull__));


void m_translate(float4x4 m, float x, float y, float z, float w);

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
m_s_mul(float4x4 res, const float4x4 a, float s)
{
  float4 vs = vf4_set(s, s, s, s);
  res[0] = vf4_mul(a[0], vs);
  res[1] = vf4_mul(a[1], vs);
  res[2] = vf4_mul(a[2], vs);
  res[3] = vf4_mul(a[3], vs);
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

void mf4_set_colvec(const float4x4 m, int col, float4 v);
float4 mf4_colvec(const float4x4 m, int col);

void mf4_transpose1(float4x4 a);
void mf4_transpose2(float4x4 a, const float4x4 b);

float4 mf4_v_mul(const float4x4 a, float4 v);

void mf4_ident(float4x4 m);
void mf4_ident_z_up(float4x4 m);

void mf4_cpy(float4x4 a, const float4x4 b);
void mf4_mul2(float4x4 a, const float4x4 b);
void mf4_add(float4x4 a, const float4x4 b, const float4x4 c);

static inline void
mf4_zero(float4x4 a)
{
  memset(a, 0, sizeof(float4x4));
}

static inline void
mf4_make_translate(float4x4 a, float3 v)
{
  mf4_ident(a);
  float4 v2 = {v.x, v.y, v.z, 1.0f};
  mf4_set_colvec(a, 3, v2);
}

void mf4_make_rotate(float4x4 m, float rads, float3 v);


static inline void
mf4_translate(float4x4 a, float3 v)
{
  float4x4 tmp;
  mf4_make_translate(tmp, v);
  mf4_mul2(a, tmp);
}

// Common opengl replacements
/*! Create ortho projection matrix */
void mf4_ortho(float4x4 m,
               float left, float right,
               float bottom, float top,
               float nearVal, float farVal);

/*! Create 2d ortho projection matrix */
void mf4_ortho2D(float4x4 m,
                 float left, float right,
                 float bottom, float top);

/*! Create fustum matrix for perspective correction */
void mf4_fustum(float4x4 m,
                float left, float right,
                float bottom, float top,
                float nearVal, float farVal);

/*! Create perspective correction matrix the easy way */
void mf4_perspective(float4x4 m,
                     float fovy, float aspect, float zNear, float zFar);

/*! Create matrix that point the camera at a specific position. */
void mf4_lookat(float4x4 m,
                float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ);

/*!
 * Compute normalised axis and angle for a rotation from a to b.
 * \param a Vector to rotate from
 * \param b Vector to rotate to
 * \result Four element vector, where x,y,z correspond to the axis, and w the
 *         angle.
 */
float4 vf3_axis_angle(float3 a, float3 b);

#ifdef __cplusplus
}
#endif

#endif
