/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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
#if defined(USE_SINGLE_FP_MATH)
typedef float scalar_t;

typedef float vec_arr_t[4];

#ifdef ENABLE_VECTORISE
typedef scalar_t __attribute__((vector_size (16))) scalar_vec_t;
#endif

typedef struct {
    float x, y, z, w;
} vec_str_t;

typedef union {
    vec_arr_t a;
    vec_str_t s;
#ifdef ENABLE_VECTORISE
    scalar_vec_t v;
#endif
} vector_t;

typedef float mat_arr_t[4][4];

typedef union {
    mat_arr_t a;
#ifdef ENABLE_VECTORISE
    scalar_vec_t v[4];
#endif
} matrix_t;

#elif defined(USE_DOUBLE_FP_MATH) 
typedef double scalar_t;

typedef double vec_arr_t[4];
typedef struct {
    double x, y, z, w;
} vec_str_t;

typedef union {
    vec_arr_t a;
    vec_str_t s;
} vector_t;

typedef double matrix_t[4][4];
#else
#error "Floating point precision must be defined"
#endif


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

typedef vec_arr_t quat_arr_t;
typedef vec_str_t quat_str_t;
typedef vector_t quaternion_t;

typedef vec_arr_t axis_arr_t;
typedef vector_t axis_t;
typedef scalar_t angle_t;

#ifdef __cplusplus
}
#endif 

#endif /* ! __MATH_TYPES_H__ */
