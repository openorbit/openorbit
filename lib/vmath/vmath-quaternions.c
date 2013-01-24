/*
 Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include <vmath/vmath-types.h>
#include <vmath/vmath-constants.h>
#include <vmath/vmath-matvec.h>
#include <vmath/vmath-quaternions.h>
#include <assert.h>

float
q_scalar(quaternion_t q)
{
#if __has_feature(attribute_ext_vector_type)
  return q.w;
#else
  float4_u qu = {.v = q};
  return qu.s.w;
#endif
}

float3
v_q_rot(float3 v, quaternion_t q)
{
  matrix_t m;
  q_m_convert(&m, q);

  float3 res = m_v3_mulf(&m, v);
  return res;
}

float3
q_vector(quaternion_t q)
{
#if __has_feature(attribute_ext_vector_type)
  float3 r = {q.x, q.y, q.z};
  return r;
#else
  float4_u qu = {.v = q};
  float3_u ru = {.s.x = qu.s.x, .s.y = qu.s.y, .s.z = qu.s.z};
  return ru.v;
#endif

}

void
q_m_convert(matrix_t *m, quaternion_t q)
{
  float n = q_dot(q, q);
  float a = (n > 0.0f) ? S_TWO / n : 0.0f;

#if __has_feature(attribute_ext_vector_type)
  float xa = q.x*a, ya = q.y*a, za = q.z*a;
  float xx = q.x*xa, xy = q.x*ya, xz = q.x*za;
  float yy = q.y*ya, yz = q.y*za, zz = q.z*za;
  float wx = q.w*xa, wy = q.w*ya, wz = q.w*za;
#else
  float4_u qu = { .v = q };
  float xa = qu.s.x*a;
  float ya = qu.s.y*a;
  float za = qu.s.z*a;

  float xy = qu.s.x*ya;
  float xz = qu.s.x*za;
  float yz = qu.s.y*za;

  float wx = qu.s.w*xa;
  float wy = qu.s.w*ya;
  float wz = qu.s.w*za;

  float xx = qu.s.x*xa;
  float yy = qu.s.y*ya;
  float zz = qu.s.z*za;

#endif

  m->a[0][0] = 1.0f-(yy+zz); m->a[0][1] =        xy-wz;
  m->a[0][2] =        xz+wy; m->a[0][3] = 0.0f;

  m->a[1][0] =        xy+wz; m->a[1][1] = 1.0f-(xx+zz);
  m->a[1][2] =        yz-wx; m->a[1][3] = 0.0f;

  m->a[2][0] =        xz-wy; m->a[2][1] =        yz+wx;
  m->a[2][2] = 1.0f-(xx+yy); m->a[2][3] = 0.0f;

  m->a[3][0] = 0.0f;         m->a[3][1] = 0.0f;
  m->a[3][2] = 0.0f;         m->a[3][3] = 1.0f;
}

void
q_mf3_convert(float3x3 m, quaternion_t q)
{
  float n = q_dot(q, q);
  float a = (n > 0.0f) ? 2.0f / n : 0.0f;

#if __has_feature(attribute_ext_vector_type)
  float xa = q.x*a, ya = q.y*a, za = q.z*a;
  float xx = q.x*xa, xy = q.x*ya, xz = q.x*za;
  float yy = q.y*ya, yz = q.y*za, zz = q.z*za;
  float wx = q.w*xa, wy = q.w*ya, wz = q.w*za;
#else
  float4_u qu = { .v = q };
  float xa = qu.s.x*a;
  float ya = qu.s.y*a;
  float za = qu.s.z*a;

  float xy = qu.s.x*ya;
  float xz = qu.s.x*za;
  float yz = qu.s.y*za;

  float wx = qu.s.w*xa;
  float wy = qu.s.w*ya;
  float wz = qu.s.w*za;

  float xx = qu.s.x*xa;
  float yy = qu.s.y*ya;
  float zz = qu.s.z*za;

#endif

  m[0][0] = 1.0f-(yy+zz); m[0][1] =        xy-wz;
  m[0][2] =        xz+wy; m[0][3] = 0.0f;

  m[1][0] =        xy+wz; m[1][1] = 1.0f-(xx+zz);
  m[1][2] =        yz-wx; m[1][3] = 0.0f;

  m[2][0] =        xz-wy; m[2][1] =        yz+wx;
  m[2][2] = 1.0f-(xx+yy); m[2][3] = 0.0f;

  m[3][0] = 0.0f;         m[3][1] = 0.0f;
  m[3][2] = 0.0f;         m[3][3] = 1.0f;
}

void
q_mf4_convert(float4x4 m, quaternion_t q)
{
#if ! __has_feature(attribute_ext_vector_type)
#error "clang extended vector attributes required"
#endif
  
  float n = q_dot(q, q);
  float a = (n > 0.0f) ? 2.0f / n : 0.0f;
  
  float xa = q.x*a, ya = q.y*a, za = q.z*a;
  float xx = q.x*xa, xy = q.x*ya, xz = q.x*za;
  float yy = q.y*ya, yz = q.y*za, zz = q.z*za;
  float wx = q.w*xa, wy = q.w*ya, wz = q.w*za;
  
  m[0] = vf4_set(1.0f-(yy+zz), xy-wz, xz+wy, 0.0f);
  m[1] = vf4_set(xy+wz, 1.0f-(xx+zz), yz-wx, 0.0f);
  m[2] = vf4_set(xz-wy, yz+wx, 1.0f-(xx+yy), 0.0f);
  m[3] = vf4_set(0.0f, 0.0f, 0.0f, 1.0f);
}


void
q_mf4_convert_inv(float4x4 m, quaternion_t q)
{
#if ! __has_feature(attribute_ext_vector_type)
#error "clang extended vector attributes required"
#endif

  float n = q_dot(q, q);
  float a = (n > 0.0f) ? 2.0f / n : 0.0f;

  float xa = q.x*a, ya = q.y*a, za = q.z*a;
  float xx = q.x*xa, xy = q.x*ya, xz = q.x*za;
  float yy = q.y*ya, yz = q.y*za, zz = q.z*za;
  float wx = q.w*xa, wy = q.w*ya, wz = q.w*za;

  m[0] = vf4_set(1.0f-(yy+zz), xy+wz, xz-wy, 0.0f);
  m[1] = vf4_set(xy-wz, 1.0f-(xx+zz), yz+wx, 0.0f);
  m[2] = vf4_set(xz+wy, yz-wx, 1.0f-(xx+yy), 0.0f);
  m[3] = vf4_set(0.0f, 0.0f, 0.0f, 1.0f);

}



quaternion_t
m_q_convert(matrix_t *m)
{
#if __has_feature(attribute_ext_vector_type)
  quaternion_t q;
#define QX(q) q.x
#define QY(q) q.y
#define QZ(q) q.z
#define QW(q) q.w

#define QIDX(q, i) q[i]

#else
  float4_u q;

#define QX(q) q.s.x
#define QY(q) q.s.y
#define QZ(q) q.s.z
#define QW(q) q.s.w

#define QIDX(q, i) q.a[i]

#endif

  float tr, s;
  tr = m->a[0][0] + m->a[1][1] + m->a[2][2];
  if (tr >= 0.0f) {
    s = sqrt(tr+m->a[3][3]);
    QW(q) = s*S_POINT_FIVE;
    s = S_POINT_FIVE / s;
    QX(q) = (m->a[2][1] - m->a[1][2]) * s;
    QY(q) = (m->a[0][2] - m->a[2][0]) * s;
    QZ(q) = (m->a[1][0] - m->a[0][1]) * s;
  } else {
    int h = 0;
    if (m->a[1][1] > m->a[0][0]) h = 1;
    if (m->a[2][2] > m->a[h][h]) h = 2;
    switch (h) {
#define CASE_MACRO(i,j,k,I,J,K)                                     \
    case I:                                                     \
      s = sqrt( (m->a[I][I] - (m->a[J][J]+m->a[K][K])) + m->a[3][3] );  \
      QIDX(q, i) = s*0.5f;                                  \
      QIDX(q, j) = (m->a[I][J] + m->a[J][I]) * s;                         \
      QIDX(q, k) = (m->a[K][I] + m->a[I][K]) * s;                         \
      QW(q) = (m->a[K][J] - m->a[J][K]) * s;                       \
      break
    CASE_MACRO(Q_X, Q_Y, Q_Z, 0, 1, 2);
    CASE_MACRO(Q_Y, Q_Z, Q_X, 1, 2, 0);
    CASE_MACRO(Q_Z, Q_X, Q_Y, 2, 0, 1);
#undef CASE_MACRO
    default:
      assert(0);
    }
  }

  // QUERY: Is the last ref to z correct?
  if (m->a[3][3] != 0.0f) {
    s = 1.0f / sqrt(m->a[3][3]);
    QX(q) *= s; QY(q) *= s; QZ(q) *= s; //QZ(q) *= s;
  }

#undef QX
#undef QY
#undef QZ
#undef QW
#undef QIDX
  return q;
}


quaternion_t
q_add(const quaternion_t a, const quaternion_t b)
{
 	return vf4_add(a, b);
}

quaternion_t
q_mul(const quaternion_t a, const quaternion_t b)
{
#if __has_feature(attribute_ext_vector_type)
	quaternion_t r;
  r.x = a.x*b.w + a.w*b.x	+ a.y*b.z - a.z*b.y;
  r.y = a.y*b.w + a.w*b.y	+ a.z*b.x - a.x*b.z;
  r.z = a.z*b.w + a.w*b.z	+ a.x*b.y - a.y*b.x;
  r.w = a.w*b.w - a.x*b.x	- a.y*b.y - a.z*b.z;
  return r;
#else
  float4_u r;
  float4_u au = {.v = a}, bu = {.v = b};
  r.s.x = au.s.x*bu.s.w + au.s.w*bu.s.x	+ au.s.y*bu.s.z - au.s.z*bu.s.y;
  r.s.y = au.s.y*bu.s.w + au.s.w*bu.s.y	+ au.s.z*bu.s.x - au.s.x*bu.s.z;
  r.s.z = au.s.z*bu.s.w + au.s.w*bu.s.z	+ au.s.x*bu.s.y - au.s.y*bu.s.x;
  r.s.w = au.s.w*bu.s.w - au.s.x*bu.s.x	- au.s.y*bu.s.y - au.s.z*bu.s.z;
  return r.v;
#endif
}

quaternion_t
q_s_div(quaternion_t q, float d)
{
#if __has_feature(attribute_ext_vector_type)
	quaternion_t r;
  r.x = q.x / d;
  r.y = q.y / d;
  r.z = q.z / d;
  r.w = q.w / d;
  return r;
#else
  float di = 1.0f / d;
  float4_u dvi = {.s.x = di, .s.y = di, .s.z = di, .s.w = di};
  quaternion_t r = q * dvi.v;
  return r;
#endif
}

quaternion_t
q_s_mul(quaternion_t q, float d)
{
#if __has_feature(attribute_ext_vector_type)
	quaternion_t r;
  r = q * d;
  return r;
#else
  float4_u dvi = {.s.x = d, .s.y = d, .s.z = d, .s.w = d};
  quaternion_t r = q * dvi.v;
  return r;
#endif
}


float
q_dot(quaternion_t a, quaternion_t b)
{
    return vf4_dot(a, b);
}

float3
q_cross(const quaternion_t a, const quaternion_t b)
{
    return vf3_cross(q_vector(a), q_vector(b));
}

float
q_abs(quaternion_t q)
{
  return v_abs(q);
}

quaternion_t
q_conj(const quaternion_t q)
{
#if __has_feature(attribute_ext_vector_type)
	quaternion_t qp = {-q.x, -q.y, -q.z, q.w};
  return qp;
#else
  float4_u qu = {.v = q};
	float4_u qp = {.s.x = -qu.s.x, .s.y = -qu.s.y, .s.z = -qu.s.z, .s.w = qu.s.w};
  return qp.v;
#endif
}

quaternion_t
q_repr(const quaternion_t q)
{
	quaternion_t res;
  quaternion_t qp;
  qp = q_conj(q);
  float d = q_dot(q, q);
  res = q_s_div(qp, d);
	return res;
}

quaternion_t
q_div(quaternion_t a, quaternion_t b)
{
	quaternion_t res;
  quaternion_t br;
  br = q_repr(b);

	res = q_mul(a, br);
	return res;
}

quaternion_t
q_rotv(float3 axis, float alpha)
{
	quaternion_t q;
  float Omega = alpha * S_POINT_FIVE;
  float sin_Omega = sin(Omega);
#if __has_feature(attribute_ext_vector_type)
  q.x = axis.x * sin_Omega;
  q.y = axis.y * sin_Omega;
  q.z = axis.z * sin_Omega;
  q.w = cos(Omega);
#else
  float4_u qu;
  float3_u axisu = { .v = axis };
  qu.s.x = axisu.s.x * sin_Omega;
  qu.s.y = axisu.s.y * sin_Omega;
  qu.s.z = axisu.s.z * sin_Omega;
  qu.s.w = cos(Omega);
  q = qu.v;
#endif
	return q;
}
quaternion_t
q_rot(float x, float y, float z, float alpha)
{
	float3 axis = {x, y, z};
	return q_rotv(axis, alpha);
}
quaternion_t
q_normalise(quaternion_t q)
{
    return v_normalise(q);
}
