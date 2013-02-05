/*
 Copyright 2006,2009,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <tgmath.h>
#include <string.h>

#include <vmath/vmath-matvec.h>
#include <vmath/vmath-constants.h>

void
vf3_outprod(float3x3 m, float3 a, float3 b) {
  m[0] = vf3_set(vf3_x(a), vf3_x(a), vf3_x(a));
  m[1] = vf3_set(vf3_y(a), vf3_y(a), vf3_y(a));
  m[2] = vf3_set(vf3_z(a), vf3_z(a), vf3_z(a));

  m[0] = vf3_mul(m[0], b);
  m[1] = vf3_mul(m[1], b);
  m[2] = vf3_mul(m[2], b);
}

/* standard non vectorised routines */

#ifndef VM_V4_NEG
#define VM_V4_NEG

float4
v_neg(float4 v)
{
  float4 res = -v;
  return res;
}

#endif /* VM_V4_NEG */

#ifndef VM_M4_V4_MUL
#define VM_M4_V4_MUL

float4
m_v_mul(const matrix_t *a, float4 v) {
	float4 res;
#if __has_feature(attribute_ext_vector_type)
	for (int i = 0 ; i < 4 ; i ++) {
    res[i] = a->a[i][0] * v.x + a->a[i][1] * v.y
           + a->a[i][2] * v.z + a->a[i][3] * v.w;
  }
#else
  float4_u vu = { .v = v };
  float4_u resu;
  for (int i = 0 ; i < 4 ; i ++) {
    resu.a[i] = a->a[i][0] * vu.s.x + a->a[i][1] * vu.s.y
    + a->a[i][2] * vu.s.z + a->a[i][3] * vu.s.w;
  }
  res = resu.v;
#endif
	return res;
}
#endif /*VM_M4_V4_MUL*/

float3
m_v3_mulf(const matrix_t *a, float3 v) {
	float3 res;
#if __has_feature(attribute_ext_vector_type)
	res.x = vf3_dot(a->v[0], v);
	res.y = vf3_dot(a->v[1], v);
	res.z = vf3_dot(a->v[2], v);
#else
  float3_u vu = { .v = v };
  float3_u resu;
  for (int i = 0 ; i < 3 ; i ++) {
    resu.a[i] = a->a[i][0] * vu.s.x + a->a[i][1] * vu.s.y
    + a->a[i][2] * vu.s.z;
  }
  res = resu.v;
#endif
	return res;
}

float3
mf3_v_mul(const float3x3 a, float3 v) {
	float3 res;
#if __has_feature(attribute_ext_vector_type)
	res.x = vf3_dot(a[0], v);
	res.y = vf3_dot(a[1], v);
	res.z = vf3_dot(a[2], v);
#else
  float3_u vu = { .v = v };
  float3_u resu;
  for (int i = 0 ; i < 3 ; i ++) {
    resu.a[i] = a[i].x * vu.s.x + a[i].y * vu.s.y
    + a->a[i].z * vu.s.z;
  }
  res = resu.v;
#endif
	return res;
}

void
mf3_add(float3x3 a, const float3x3 b, const float3x3 c)
{
  a[0] = b[0] + c[0];
  a[1] = b[1] + c[1];
  a[2] = b[2] + c[2];
}

void
mf3_add2(float3x3 a, const float3x3 b)
{
  a[0] += b[0];
  a[1] += b[1];
  a[2] += b[2];
}


void
mf3_sub(float3x3 a, const float3x3 b, const float3x3 c)
{
  a[0] = b[0] - c[0];
  a[1] = b[1] - c[1];
  a[2] = b[2] - c[2];
}

void
mf3_s_mul(float3x3 res, const float3x3 m, float s)
{
  float3 v = vf3_set(s, s, s);
  res[0] = m[0] * v;
  res[1] = m[1] * v;
  res[2] = m[2] * v;
}


#ifndef VM_M4_MUL
#define VM_M4_MUL

void
m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b) {
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            res->a[i][j] = a->a[i][0]*b->a[0][j]
                         + a->a[i][1]*b->a[1][j]
                         + a->a[i][2]*b->a[2][j]
                         + a->a[i][3]*b->a[3][j];
        }
    }
}
#endif

void
m_add(matrix_t *res, matrix_t *a, matrix_t *b)
{
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            res->a[i][j] = a->a[i][j] + b->a[i][j];
        }
    }
}

void
m_sub(matrix_t *res, matrix_t *a, matrix_t *b)
{
  for (int i = 0 ; i < 4 ; i ++) {
    for (int j = 0 ; j < 4 ; j ++) {
      res->a[i][j] = a->a[i][j] - b->a[i][j];
    }
  }
}

float4
v_s_add(float4 a, float b)
{
	float4 c = {b,b,b,b};
	float4 r;
	r = vf4_add(a, c);
	return r;
}

float4
v_add(float4 a, float4 b)
{
	float4 res;
  res = a + b;
	return res;
}

float4
v_sub(float4 a, float4 b)
{
	float4 res;
  res = a - b;
	return res;
}


float3
vf3_cross(float3 a, float3 b)
{
#if __has_feature(attribute_ext_vector_type)
  // TODO: Use shuffle vectors intrinsic
  float3 res;
  res[0] = a[1]*b[2]-a[2]*b[1];
  res[1] = a[2]*b[0]-a[0]*b[2];
  res[2] = a[0]*b[1]-a[1]*b[0];
  return res;
#else
  float3_u au = {.v = a}, bu = {.v = b};
  float3_u res;
  res.s.x = au.s.y*bu.s.z-au.s.z*bu.s.y;
  res.s.y = au.s.z*bu.s.x-au.s.x*bu.s.z;
  res.s.z = au.s.x*bu.s.y-au.s.y*bu.s.x;
  return res.v;
#endif
}

#ifndef VM_V4_DOT
#define VM_V4_DOT

float
v_dot(float4 a, float4 b)
{
#if __has_feature(attribute_ext_vector_type)
  float4 c = a * b;
  return c.x + c.y + c.z + c.w;
#else
  float4_u c = { .v = a + b };
  return c.s.x + c.s.y + c.s.z + c.s.w;
  //return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
#endif
}

#endif /* VM_V4_DOT */

float4
v_s_mul(float4 a, float s)
{
#if __has_feature(attribute_ext_vector_type)
	float4 res;
  res = a * s;
  return res;
#else
  float4_u sv = { .s.x = s, .s.y = s, .s.z = s, .s.w = s };
  return a * sv.v;
#endif
}

float4
v_s_div(float4 a, float s)
{
#if __has_feature(attribute_ext_vector_type)
  float4 res;
  float d = 1.0f / s;
  res[0] = a[0] * d;
  res[1] = a[1] * d;
  res[2] = a[2] * d;
  res[3] = a[3] * d;
  return res;
#else
  float d = 1.0f / s;
  float4_u dv = { .s.x = d, .s.y = d, .s.z = d, .s.w = d };
  return a * dv.v;
#endif
}

void
m_lu(const matrix_t *a, matrix_t *l, matrix_t *u)
{
    // compute L, U in A=LU, where L, U are triangular
//    m_cpy(u, a);
//    m_zero(l);
//    for (int i = 0 ; i < 4 ; i ++) {
//        vector_t v;
//        v_s_div(&v, u.a[i], u.a[i][i]);
//        v_s_mul(&v, &v, u.a[i+1][j]);

//    }
    return;
}

float4
v_normalise(float4 v)
{
    float norm = vf4_abs(v);
    return vf4_s_mul(v, 1.0f/norm);
}

float
m_det(const matrix_t *m)
{
    // compute the four subdeterminants (Saurrus)
    // note, this version has been written to work, not to be efficient in any
    // way
    float sub_det[4];

    sub_det[0] =  MAT_ELEM(*m, 1, 1) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 3, 3);
    sub_det[0] += MAT_ELEM(*m, 1, 2) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 3, 1);
    sub_det[0] += MAT_ELEM(*m, 1, 3) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 3, 2);
    sub_det[0] -= MAT_ELEM(*m, 3, 1) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 1, 3);
    sub_det[0] -= MAT_ELEM(*m, 3, 2) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 1, 1);
    sub_det[0] -= MAT_ELEM(*m, 3, 3) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 1, 2);

    sub_det[1] =  MAT_ELEM(*m, 1, 0) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 3, 3);
    sub_det[1] += MAT_ELEM(*m, 1, 2) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 3, 0);
    sub_det[1] += MAT_ELEM(*m, 1, 3) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 3, 2);
    sub_det[1] -= MAT_ELEM(*m, 3, 0) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 1, 3);
    sub_det[1] -= MAT_ELEM(*m, 3, 2) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 1, 0);
    sub_det[1] -= MAT_ELEM(*m, 3, 3) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 1, 2);

    sub_det[2] =  MAT_ELEM(*m, 1, 0) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 3, 3);
    sub_det[2] += MAT_ELEM(*m, 1, 1) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 3, 0);
    sub_det[2] += MAT_ELEM(*m, 1, 3) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 3, 1);
    sub_det[2] -= MAT_ELEM(*m, 3, 0) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 1, 3);
    sub_det[2] -= MAT_ELEM(*m, 3, 1) * MAT_ELEM(*m, 2, 3) * MAT_ELEM(*m, 1, 0);
    sub_det[2] -= MAT_ELEM(*m, 3, 3) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 1, 1);

    sub_det[3] =  MAT_ELEM(*m, 1, 0) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 3, 2);
    sub_det[3] += MAT_ELEM(*m, 1, 1) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 3, 0);
    sub_det[3] += MAT_ELEM(*m, 1, 2) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 3, 1);
    sub_det[3] -= MAT_ELEM(*m, 3, 0) * MAT_ELEM(*m, 2, 1) * MAT_ELEM(*m, 1, 2);
    sub_det[3] -= MAT_ELEM(*m, 3, 1) * MAT_ELEM(*m, 2, 2) * MAT_ELEM(*m, 1, 0);
    sub_det[3] -= MAT_ELEM(*m, 3, 2) * MAT_ELEM(*m, 2, 0) * MAT_ELEM(*m, 1, 1);

    float det = MAT_ELEM(*m, 0, 0) * sub_det[0]
                 - MAT_ELEM(*m, 0, 1) * sub_det[1]
                 + MAT_ELEM(*m, 0, 2) * sub_det[2]
                 - MAT_ELEM(*m, 0, 3) * sub_det[3];

    return det;
}

float
m_subdet3(const matrix_t *m, int k, int l)
{
    float acc = S_CONST(0.0);
    matrix_t m_prim;
    // copy the relevant matrix elements
    for (int i0 = 0, i1 = 0 ; i0 < 4 ; i0 ++) {
        if (k == i0) continue; // skip row for sub det
        for(int j0 = 0, j1 = 0 ; j0 < 4 ; j0 ++) {
            if (l == j0) continue; // skip col for subdet
            MAT_ELEM(m_prim, i1, j1) = MAT_ELEM(*m, i0, j0);
            j1 ++;
        }
        i1 ++;
    }


    // Apply Sarrus
    for (int i = 0 ; i < 3 ; i ++) {
        acc += MAT_ELEM(m_prim, 0, (0+i) % 3) * MAT_ELEM(m_prim, 1, (1+i) % 3)
             * MAT_ELEM(m_prim, 2, (2+i) % 3);

        acc -= MAT_ELEM(m_prim, 2, (0+i) % 3) * MAT_ELEM(m_prim, 1, (1+i) % 3)
             * MAT_ELEM(m_prim, 0, (2+i) % 3);
    }

    return acc;
}

matrix_t
m_adj(const matrix_t *m)
{
    matrix_t M_adj;
    float sign = S_CONST(1.0);
    for (int i = 0 ; i < 4 ; i ++) {
        for(int j = 0; j < 4 ; j ++) {
            MAT_ELEM(M_adj, j, i) = sign * m_subdet3(m, i, j);
            sign *= S_CONST(-1.0);
        }
        sign *= S_CONST(-1.0);
    }

    return M_adj;
}

matrix_t
m_inv(const matrix_t *M)
{
    // Very brute force, there are more efficient ways to do this
    float det = m_det(M);
    matrix_t M_adj = m_adj(M);
    matrix_t M_inv;

    if (det != S_CONST(0.0)) {
        for (int i = 0 ; i < 4 ; i ++) {
            for (int j = 0 ; j < 4 ; j ++) {
                MAT_ELEM(M_inv, i, j) = MAT_ELEM(M_adj, i, j) / det;
            }
        }
    } else {
        // If the determinant is zero, then the matrix is not invertible
        // thus, return NANs in all positions
        for (int i = 0; i < 4; i ++) {
            for (int j = 0; j < 4; j ++) {
                MAT_ELEM(M_inv, i, j) = NAN;
            }
        }
    }

    return M_inv;
}

float
v_abs(float4 v)
{
#if __has_feature(attribute_ext_vector_type)
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
#else
  float4_u vu = { .v = v };
  return sqrt(vu.s.x*vu.s.x + vu.s.y*vu.s.y + vu.s.z*vu.s.z + vu.s.w*vu.s.w);
#endif
}

void
m_transpose(matrix_t *mt, const matrix_t *m)
{
    for ( unsigned int i = 0; i < 4; i += 1 ) {
        for ( unsigned int j = 0; j < 4; j += 1 ) {
            mt->a[j][i] = m->a[i][j];
        }
    }
}

void
m_axis_rot_x(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = S_ONE;
    m->a[1][1] = cos_a; m->a[1][1] = sin_a;
    m->a[2][1] = -sin_a; m->a[2][1] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_axis_rot_y(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][2] = -sin_a;
    m->a[1][1] = S_ONE;
    m->a[2][0] = sin_a; m->a[2][2] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_axis_rot_z(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][1] = sin_a;
    m->a[1][0] = -sin_a; m->a[1][1] = cos_a;
    m->a[2][2] = S_ONE;
    m->a[3][3] = S_ONE;
}


void
m_vec_rot_x(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = S_ONE;
    m->a[1][1] = cos_a; m->a[1][1] = -sin_a;
    m->a[2][1] = sin_a; m->a[2][1] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_vec_rot_y(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][2] = sin_a;
    m->a[1][1] = 1.0f;
    m->a[2][0] = -sin_a; m->a[2][2] = cos_a;
    m->a[3][3] = 1.0f;
}

void
m_vec_rot_z(matrix_t *m, float a)
{
    float sin_a = sin(a);
    float cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][1] = -sin_a;
    m->a[1][0] = sin_a; m->a[1][1] = cos_a;
    m->a[2][2] = S_ONE;
    m->a[3][3] = S_ONE;
}

void
m_unit(matrix_t *m)
{
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = S_ONE;
    m->a[1][1] = S_ONE;
    m->a[2][2] = S_ONE;
    m->a[3][3] = S_ONE;
}

void
m_zero(matrix_t *m)
{
    memset(m, 0, sizeof(matrix_t));
}


void
m_cpy(matrix_t * restrict dst, const matrix_t * restrict src)
{
    for (int i = 0 ; i < 4 ; i ++) {
        dst->a[i][0] = src->a[i][0];
        dst->a[i][1] = src->a[i][1];
        dst->a[i][2] = src->a[i][2];
        dst->a[i][3] = src->a[i][3];
    }
}

bool
v_eq(float4 a, float4  b, float tol)
{
#if __has_feature(attribute_ext_vector_type)
  for (int i = 0 ; i < 4 ; i ++) {
    if (!((a[i] <= b[i]+tol) && (a[i] >= b[i]-tol))) {
      return false;
    }
  }
#else
  float4_u au = {.v = a}, bu = {.v = b};
  for (int i = 0 ; i < 4 ; i ++) {
    if (!((au.a[i] <= bu.a[i]+tol) && (au.a[i] >= bu.a[i]-tol))) {
      return false;
    }
  }
#endif
    return true;
}

void
m_rot(matrix_t *m, float x, float y, float z, float alpha)
{
  float c = cosf(alpha);
  float s = sinf(alpha);

  m->v[0] = vf4_set(x*x+(1-x*x)*c, x*y*(1-c)-z*s, x*z*(1-c)+y*s, 0.0);
  m->v[1] = vf4_set(x*y*(1-c)+z*s, y*y+(1-y*y)*c, y*z*(1-c)-x*s, 0.0);
  m->v[2] = vf4_set(x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z+(1-z*z)*c, 0.0);
  m->v[3] = vf4_set(0.0, 0.0, 0.0, 1.0);
}

void
mf3_rot(float3x3 m, float x, float y, float z, float alpha)
{
  float c = cosf(alpha);
  float s = sinf(alpha);

  m[0] = vf3_set(x*x+(1-x*x)*c, x*y*(1-c)-z*s, x*z*(1-c)+y*s);
  m[1] = vf3_set(x*y*(1-c)+z*s, y*y+(1-y*y)*c, y*z*(1-c)-x*s);
  m[2] = vf3_set(x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z+(1-z*z)*c);
  m[3] = vf3_set(0.0, 0.0, 0.0);
}

void
md3_rot(double3x3 m, double x, double y, double z, double alpha)
{
  double c = cos(alpha);
  double s = sin(alpha);

  m[0] = vd3_set(x*x+(1-x*x)*c, x*y*(1-c)-z*s, x*z*(1-c)+y*s);
  m[1] = vd3_set(x*y*(1-c)+z*s, y*y+(1-y*y)*c, y*z*(1-c)-x*s);
  m[2] = vd3_set(x*z*(1-c)-y*s, y*z*(1-c)+x*s, z*z+(1-z*z)*c);
  m[3] = vd3_set(0.0, 0.0, 0.0);
}

void
mf3_cpy(float3x3 dst, const float3x3 src)
{
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
}

void
mf3_transpose2(float3x3 a, const float3x3 b)
{
  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
#if __has_feature(attribute_ext_vector_type)
      a[i][j] = b[j][i];
#else
      vf3_seti(&a[i], j, b[j][i]);
#endif
    }
  }
}

float4
mf4_colvec(const float4x4 m, int col)
{
  return vf4_set(m[0][col], m[1][col], m[2][col], m[3][col]);
}

void
mf4_set_colvec(const float4x4 m, int col, float4 v)
{
  m[0][col] = v.x;
  m[1][col] = v.y;
  m[2][col] = v.z;
  m[3][col] = v.w;
}

void
mf4_transpose1(float4x4 a)
{
#if ! __has_feature(attribute_ext_vector_type)
#error "Compiler must support clang ext vectors"
#endif
  float4x4 tmp;
  mf4_cpy(tmp, a);
  for (int i = 0 ; i < 4 ; ++ i) {
    a[i] = mf4_colvec(tmp, i);
  }
}

void
mf4_transpose2(float4x4 a, const float4x4 b)
{
#if ! __has_feature(attribute_ext_vector_type)
#error "Compiler must support clang ext vectors"
#endif
  for (int i = 0 ; i < 4 ; ++ i) {
    a[i] = mf4_colvec(b, i);
  }
}

void
mf3_mul2(float3x3 a, const float3x3 b)
{
  float3x3 atmp;
  float3x3 btransp;

  mf3_cpy(atmp, a);
  mf3_transpose2(btransp, b);

  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
#if __has_feature(attribute_ext_vector_type)
      a[i][j] = vf3_dot(atmp[i], b[j]);
#else
#error "Please fix for non clang compiler"
#endif
    }
  }
}

void
mf3_mul3(float3x3 a, const float3x3 b, const float3x3 c)
{
  float3x3 ctransp;
  mf3_transpose2(ctransp, c);

  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
#if __has_feature(attribute_ext_vector_type)
      a[i][j] = vf3_dot(b[i], ctransp[j]);
#else
#error "Please fix for non clang compiler"
#endif
    }
  }
}

float
mf3_det(const float3x3 m)
{
  return m[0][0]*m[1][1]*m[2][2] +
         m[0][1]*m[1][2]*m[2][0] +
         m[0][2]*m[1][0]*m[2][1] -
         m[2][0]*m[1][1]*m[0][2] -
         m[2][1]*m[1][2]*m[0][0] -
         m[2][2]*m[1][0]*m[0][1];
}


void
mf3_adj(float3x3 adjMat, const float3x3 m)
{
  // TODO: Can we vectorise this?
  adjMat[0][0] = m[1][1]*m[2][2] - m[1][2]*m[2][1];
  adjMat[0][1] = -(m[0][1]*m[2][2] - m[0][2]*m[2][1]);
  adjMat[0][2] = m[0][1]*m[1][2] - m[0][2]*m[1][1];

  adjMat[1][0] = -(m[1][0]*m[2][2] - m[1][2]*m[2][0]);
  adjMat[1][1] = m[0][0]*m[2][2] - m[0][2]*m[2][0];
  adjMat[1][2] = -(m[0][0]*m[1][2] - m[0][2]*m[1][0]);

  adjMat[2][0] = m[1][0]*m[2][1] - m[1][1]*m[2][0];
  adjMat[2][1] = -(m[0][0]*m[2][1] - m[0][1]*m[2][0]);
  adjMat[2][2] = m[0][0]*m[1][1] - m[0][1]*m[1][0];
}

void
mf3_inv2(float3x3 invmat, const float3x3 mat)
{
  // Very brute force, there are more efficient ways to do this
  float det = mf3_det(mat);
  float3 reprDetVec = vf3_repr(vf3_set(det, det, det));
  float3x3 M_adj;
  mf3_adj(M_adj, mat);

  if (det != 0.0f) {
    for (int i = 0 ; i < 3 ; i ++) {
      invmat[i] = vf3_mul(M_adj[i], reprDetVec);
    }
  } else {
    // If the determinant is zero, then the matrix is not invertible
    // thus, return NANs in all positions
    for (int i = 0; i < 3; i ++) {
      invmat[i] = vf3_set(NAN, NAN, NAN);
    }
  }
}

void
mf3_inv1(float3x3 mat)
{
  // Very brute force, there are more efficient ways to do this
  float det = mf3_det(mat);
  float3 reprDetVec = vf3_repr(vf3_set(det, det, det));
  float3x3 M_adj;

  mf3_adj(M_adj, mat);

  if (det != 0.0f) {
    for (int i = 0 ; i < 3 ; i ++) {
      mat[i] = vf3_mul(M_adj[i], reprDetVec);
    }
  } else {
    // If the determinant is zero, then the matrix is not invertible
    // thus, return NANs in all positions
    for (int i = 0; i < 3; i ++) {
      mat[i] = vf3_set(NAN, NAN, NAN);
    }
  }
}

void
mf3_ident(float3x3 m)
{
  memset(m, 0, sizeof(float3x3));
  m[0][0] = 1.0f;
  m[1][1] = 1.0f;
  m[2][2] = 1.0f;
}

void
mf4_ident(float4x4 m)
{
  memset(m, 0, sizeof(float4x4));
  m[0][0] = 1.0f;
  m[1][1] = 1.0f;
  m[2][2] = 1.0f;
  m[3][3] = 1.0f;
}


bool
m_eq(const matrix_t *a, const matrix_t *b, float tol)
{
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            if (!((a->a[i][j] <= b->a[i][j]+tol) && (a->a[i][j] >= b->a[i][j]-tol))) {
                return false;
            }
        }
    }

    return true;
}


void
m_translate(matrix_t *m, float x, float y, float z, float w)
{
    memset(m, 0, sizeof(mat_arr_t));

    m->a[0][0] = x;
    m->a[1][1] = y;
    m->a[2][2] = z;
    m->a[3][3] = w;
}


void
mf3_basis(float3x3 res, const float3x3 m, const float3x3 b)
{
  //res = b * m * inv(b)
  float3x3 binv;
  mf3_transpose2(binv, b); // Inverse is the transpose for a rotation matrix
  mf3_mul3(res, b, m);
  mf3_mul2(res, binv);
}

void
mf4_cpy(float4x4 a, const float4x4 b)
{
  memcpy(a, b, sizeof(float4x4));
}

void
mf4_mul2(float4x4 a, const float4x4 b)
{
  float4x4 atmp;
  float4x4 btransp;

  mf4_cpy(atmp, a);
  mf4_transpose2(btransp, b);

  for (int i = 0 ; i < 4 ; ++ i) {
    for (int j = 0 ; j < 4 ; ++ j) {
#if __has_feature(attribute_ext_vector_type)
      a[i][j] = vf4_dot(atmp[i], b[j]);
#else
#error "Please fix for non clang compiler"
#endif
    }
  }
}


// Replacement routines for OpenGL 3
// Note that this is a row major matrix

void
mf4_lookat(float4x4 m,
           float eyeX, float eyeY, float eyeZ,
           float centerX, float centerY, float centerZ,
           float upX, float upY, float upZ)
{
  float3 f = vf3_sub(vf3_set(centerX, centerY, centerZ),
                     vf3_set(eyeX, eyeY, eyeZ));
  float3 up = vf3_set(upX, upY, upZ);
  f = vf3_normalise(f);
  up = vf3_normalise(up);
  float3 s = vf3_cross(f, up);
  float3 u = vf3_cross(s, f);
  m[0] = s;
  m[1] = u;
  m[2] = vf3_neg(f);
  m[3] = vf4_set(0.0f, 0.0f, 0.0f, 1.0f);

  mf4_translate(m, vf4_set(-eyeX, -eyeY, -eyeZ, 0.0f));
}


void
mf4_perspective(float4x4 m,
                float fovy, float aspect, float zNear, float zFar)
{
  float f = 1.0f/tan(fovy/2.0f); // Cotan
  m[0] = vf4_set(f/aspect, 0.0f, 0.0f, 0.0f);
  m[1] = vf4_set(0.0f, f, 0.0f, 0.0f);
  m[2] = vf4_set(0.0f, 0.0f, (zFar+zNear)/(zNear-zFar),
                 (2.0f*zFar*zNear)/(zNear-zFar));
  m[3] = vf4_set(0.0f, 0.0f, -1.0f, 0.0f);
}

void
mf4_frustum(float4x4 m,
            float left, float right,
            float bottom, float top,
            float nearVal, float farVal)
{
  float a = (right+left)/(right-left);
  float b = (top+bottom)/(top-bottom);
  float c = -(farVal+nearVal)/(farVal-nearVal);
  float d = -(2.0f*farVal*nearVal)/(farVal-nearVal);
  m[0] = vf4_set(2.0*nearVal/(right-left), 0.0, a, 0.0f);
  m[1] = vf4_set(0.0, 2.0*nearVal/(top-bottom), b, 0.0f);
  m[2] = vf4_set(0.0, 0.0, c, d);
  m[3] = vf4_set(0.0, 0.0, -1.0, 0.0f);
}


void
mf4_ortho(float4x4 m,
          float left, float right,
          float bottom, float top,
          float nearVal, float farVal)
{
  float tx = (right+left)/(right-left);
  float ty = (top+bottom)/(top-bottom);
  float tz = (farVal+nearVal)/(farVal-nearVal);
  m[0] = vf4_set(2.0/(right-left), 0.0f, 0.0f, tx);
  m[1] = vf4_set(0.0f, 2.0/(top-bottom), 0.0f, ty);
  m[2] = vf4_set(0.0f, 0.0f, -2.0/(farVal-nearVal), tz);
  m[3] = vf4_set(0.0f, 0.0f, 0.0f, 1.0f);
}


void
mf4_ortho2D(float4x4 m,
            float left, float right,
            float bottom, float top)
{
  mf4_ortho(m, left, right, bottom, top, -1.0f, 1.0f);
}

void
mf4_make_rotate(float4x4 m, float rads, float4 v)
{
  float c = cosf(rads);
  float s = sinf(rads);
  v.w = 0.0f;
  float4 v2 = vf3_normalise(v);
  m[0] = vf4_set(v2.x*v2.x*(1.0f-c)+c,
                 v2.x*v2.y*(1.0f-c)-v2.z*s,
                 v2.x*v2.z*(1.0f-c)+v2.y*s,
                 0.0f);
  m[1] = vf4_set(v2.y*v2.x*(1.0f-c)+v2.z*s,
                 v2.y*v2.y*(1.0f-c)+c,
                 v2.y*v2.z*(1.0f-c)-v2.x*s,
                 0.0f);
  m[2] = vf4_set(v2.x*v2.z*(1.0f-c)-v2.y*s,
                 v2.y*v2.z*(1.0f-c)+v2.x*s,
                 v2.z*v2.z*(1.0f-c)+c,
                 0.0f);
  m[3] = vf4_set(0.0f, 0.0f, 0.0f, 1.0f);
}

