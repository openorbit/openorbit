/*
  Copyright 2006, 2009, 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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
 * \file types
 * \brief The header declares types used in the math functions.
 */

#ifndef __MATH_TYPES_H__
#define __MATH_TYPES_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifndef __has_feature
#define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif

/* SIMD-friendly 4 element-vectors */
/* all functions take restricted pointers as arguments */

/*************** Basic types ****************/

typedef float vec_arr_t[4];

#if __has_feature(attribute_ext_vector_type)
  typedef float __attribute__((ext_vector_type (3))) float3;
  typedef float __attribute__((ext_vector_type (4))) float4;
  typedef float __attribute__((ext_vector_type (16))) float16;

  typedef float3 float3x3[3];
  typedef float4 float4x4[4];
  typedef double __attribute__((ext_vector_type (3))) double3;
  typedef double __attribute__((ext_vector_type (4))) double4;
  typedef double __attribute__((ext_vector_type (16))) double16;

  typedef int16_t __attribute__((ext_vector_type (4))) short3;
  typedef int16_t __attribute__((ext_vector_type (4))) short4;
  typedef uint16_t __attribute__((ext_vector_type (4))) ushort3;
  typedef uint16_t __attribute__((ext_vector_type (4))) ushort4;
  typedef int32_t __attribute__((ext_vector_type (4))) int3;
  typedef int32_t __attribute__((ext_vector_type (4))) int4;
  typedef uint32_t __attribute__((ext_vector_type (4))) uint3;
  typedef uint32_t __attribute__((ext_vector_type (4))) uint4;
  typedef int64_t __attribute__((ext_vector_type (4))) long3;
  typedef int64_t __attribute__((ext_vector_type (4))) long4;
  typedef uint64_t __attribute__((ext_vector_type (4))) ulong3;
  typedef uint64_t __attribute__((ext_vector_type (4))) ulong4;

  typedef double3 double3x3[3];
  typedef double4 double4x4[4];

  typedef float __attribute__((ext_vector_type (4))) quaternion_t;


#else
  typedef float __attribute__((vector_size (16))) float3;
  typedef float __attribute__((vector_size (16))) float4;
  typedef float __attribute__((vector_size (64))) float16;

  typedef float3 float3x3[3];
  typedef float4 float4x4[4];
  typedef double __attribute__((vector_size (32))) double3;
  typedef double __attribute__((vector_size (32))) double4;
  typedef double __attribute__((vector_size (128))) double16;

  typedef int16_t __attribute__((vector_size (8))) short3;
  typedef int16_t __attribute__((vector_size (8))) short4;
  typedef uint16_t __attribute__((vector_size (8))) ushort3;
  typedef uint16_t __attribute__((vector_size (8))) ushort4;
  typedef int32_t __attribute__((vector_size (16))) int3;
  typedef int32_t __attribute__((vector_size (16))) int4;
  typedef uint32_t __attribute__((vector_size (16))) uint3;
  typedef uint32_t __attribute__((vector_size (16))) uint4;
  typedef int64_t __attribute__((vector_size (32))) long3;
  typedef int64_t __attribute__((vector_size (32))) long4;
  typedef uint64_t __attribute__((vector_size (32))) ulong3;
  typedef uint64_t __attribute__((vector_size (32))) ulong4;

  typedef double3 double3x3[4];
  typedef double4 double4x4[4];

  typedef float __attribute__((vector_size (16))) quaternion_t;

#endif

typedef union float3_u {
  float3 v;
  float a[4];
  struct {
    float x, y, z;
  } s;
} float3_u;

typedef union float4_u {
  float4 v;
  float a[4];
  struct {
    float x, y, z, w;
  } s;
} float4_u;

typedef union float4x4_u {
  float4x4 v;
  float a[4][4];
} float4x4_u;

typedef union float16_u {
  float16 v;
  float a[16];
} float16_u;


typedef union double3_u {
  double3 v;
  double a[4];
} double3_u;

typedef union double4_u {
  double4 v;
  double a[4];
} double4_u;

typedef union double4x4_u {
  double4x4 v;
  double a[4][4];
} double4x4_u;


typedef union double16_u {
  double16 v;
  double a[16];
} double16_u;


typedef float ma4_t[4][4];

typedef float mat_arr_t[4][4];


#define MAT_ELEM(M, i, j) ((M).a[i][j])

static inline float4
v4f_make(float x, float y, float z, float w)
{
#if __has_feature(attribute_ext_vector_type)
  float4 v = { x, y, z, w };
  return v;
#else
  float4_u v;
  v.a[0] = x;
  v.a[1] = y;
  v.a[2] = z;
  v.a[3] = 0.0;
  return v.v;
#endif
}

static inline float3
v3f_make(float x, float y, float z)
{
#if __has_feature(attribute_ext_vector_type)
  float3 v = { x, y, z };
  return v;
#else
  float3_u v;
  v.a[0] = x;
  v.a[1] = y;
  v.a[2] = z;
  return v.v;
#endif
}

static inline int3
v3i_make(int32_t x, int32_t y, int32_t z)
{
  union {
    int32_t a[4];
    int3 p;
  } u;

  u.a[0] = x;
  u.a[1] = y;
  u.a[2] = z;
  u.a[3] = 0;

  return u.p;
}
// Note, this is not the initial idea of the vmath lib, the vmath lib was supposed to
// include real vectors only, but in order to keep all the math stuff at the same place
// we add fixed point numbers here as well.
// Fixed point numbers
typedef struct fix128 {
  uint64_t hi;
  uint64_t lo;
} fix128;

typedef struct fix256 {
  struct {
    uint64_t x;
    uint64_t y;
  } hi;
  struct {
    uint64_t x;
    uint64_t y;
  } lo;
} fix256;


#ifdef __cplusplus
}
#endif

#endif /* ! __MATH_TYPES_H__ */
