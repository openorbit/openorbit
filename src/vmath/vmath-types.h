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
 * \file types
 * \brief The header declares types used in the math functions.
 */

#ifndef __MATH_TYPES_H__
#define __MATH_TYPES_H__
#ifdef __cplusplus
extern "C" {
#endif 


/* SIMD-friendly 4 element-vectors */
/* all functions take restricted pointers as arguments */

/*************** Basic types ****************/
typedef float scalar_t;

typedef float vec_arr_t[4];

typedef float __attribute__((vector_size (16))) v4f_t;
typedef float __attribute__((vector_size (64))) m4f_t;

typedef float ma4_t[4][4];



typedef union {
    vec_arr_t a;
    struct {
        scalar_t x, y, z, w;
    };
    v4f_t v;
} vector_t;

typedef float mat_arr_t[4][4];

typedef union {
    mat_arr_t a;
    v4f_t v[4];
} matrix_t;



#define MAT_ELEM(M, i, j) ((M).a[i][j])

/************** Derived types ****************/
/*!
 * \brief A quaternion
 * 
 * The quaternion_t type is an overlay of the vector type. It is to be
 * compatible with the vector_t type in most situations. The layout of the
 * quaternion_t type is an array of either floats or doubles (depending on
 * build settings), named x, y, z and w where [x, y, z] is the vectorial part
 * and w the scalar part of the quaternion.
 */

typedef vector_t quaternion_t;

typedef vec_arr_t axis_arr_t;
typedef vector_t axis_t;
typedef scalar_t angle_t;

static inline v4f_t
v4f_make(float x, float y, float z, float w)
{
  vector_t v;
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = w;
  return v.v;
}

static inline v4f_t
v3f_make(float x, float y, float z)
{
  vector_t v;
  v.x = x;
  v.y = y;
  v.z = z;
  v.w = 0.0;
  return v.v;
}


#ifdef __cplusplus
}
#endif 

#endif /* ! __MATH_TYPES_H__ */
