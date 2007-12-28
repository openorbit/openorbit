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
 * \file linalg.h
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
#include <math/types.h>

#define M_V_MUL(vr, M, vx) \
    m_v_mul((vr).a, (M).a, (vx).a)

void m_v_mul(vec_arr_t res, mat_arr_t a, const vec_arr_t v)
    __attribute__ ((__nonnull__));

#if ENABLE_VECTORISE
    #undef M_V_MUL
    #define M_V_MUL(vr, M, vx) \
        do { \
            matrix_t p;\
            p.v[0] = M.v[0] * vx.v;\
            p.v[1] = M.v[1] * vx.v;\
            p.v[2] = M.v[2] * vx.v;\
            p.v[3] = M.v[3] * vx.v;\
            \
            vr.s.x = p.a[0][0] + p.a[0][1] + p.a[0][2] + p.a[0][3];\
            vr.s.y = p.a[1][0] + p.a[1][1] + p.a[1][2] + p.a[1][3];\
            vr.s.z = p.a[2][0] + p.a[2][1] + p.a[2][2] + p.a[2][3];\
            vr.s.w = p.a[3][0] + p.a[3][1] + p.a[3][2] + p.a[3][3];\
        } while (0)
#endif


#define M_MUL(A, B, C) \
    m_mul((A).a, (B).a, (C).a)

void m_mul(mat_arr_t res, mat_arr_t a, mat_arr_t b)
    __attribute__ ((__nonnull__));    
    
    
#define M_ADD(A, B, C) \
    m_add((A).a, (B).a, (C).a)
    
void m_add(mat_arr_t res, mat_arr_t a, mat_arr_t b)
    __attribute__ ((__nonnull__));

#if ENABLE_VECTORISE
    #undef M_ADD
    #define M_ADD(vr, va, vb)           \
       do {                             \
           vr.v[0] = va.v[0] + vb.v[0]; \
           vr.v[1] = va.v[1] + vb.v[1]; \
           vr.v[2] = va.v[2] + vb.v[2]; \
           vr.v[3] = va.v[3] + vb.v[3]; \
       } while (0)
#endif


#define V_ADD(vr, va, vb) \
    v_add((vr).a, (va).a, (vb).a)

void v_add(vec_arr_t res, vec_arr_t a, const vec_arr_t b)
    __attribute__ ((__nonnull__));

#if ENABLE_VECTORISE
    #undef V_ADD
    #define V_ADD(vr, va, vb) vr.v = va.v + vb.v
#endif

#define V_SUB(vr, va, vb) \
    v_sub((vr).a, (va).a, (vb).a)

void v_sub(vec_arr_t res, vec_arr_t a, const vec_arr_t b)
    __attribute__ ((__nonnull__));

#if ENABLE_VECTORISE
    #undef V_SUB
    #define V_SUB(vr, va, vb) vr.v = va.v - vb.v
#endif


#define V_S_MUL(vr, va, s) \
    v_s_mul((vr).a, (va).a, (s))

void v_s_mul(vec_arr_t res, vec_arr_t a, scalar_t s)
    __attribute__ ((__nonnull__));

#if ENABLE_VECTORISE
    #undef V_S_MUL
    #define V_S_MUL(vr, va, sc) \
        do {\
            vector_t vs = {.s.x = (sc), .s.y = (sc), .s.z = (sc), .s.w = (sc) };\
            vr.v = va.v * vs.v;\
        } while (0)
#endif



#define V_S_DIV(vr, va, s) \
    v_s_div((vr).a, (va).a, (s))

void v_s_div(vec_arr_t res, vec_arr_t a, scalar_t s)
    __attribute__ ((__nonnull__));


#if ENABLE_VECTORISE
    #undef V_S_DIV
    #define V_S_DIV(vr, va, sc) \
        do {\
            scalar_t repr = 1.0f/(sc);\
            vector_t vs = {.s.x = repr, .s.y = repr, .s.z = repr, .s.w = repr };\
            vr.v = va.v * vs.v;\
        } while (0)
#endif


/* This is really a 3d x product */

#define V_CROSS(vr, va, vb) \
    v_cross((vr).a, (va).a, (vb).a)
    
void v_cross(vec_arr_t res, const vec_arr_t a, const vec_arr_t b)
    __attribute__ ((__nonnull__));

#define V_DOT(s, va, vb) \
    (s) = v_dot((va).a, (vb).a)
    
scalar_t v_dot(const vec_arr_t a, const vec_arr_t b)
    __attribute__ ((__pure__, __nonnull__));

#define V_NORMALISE(va) \
    v_normalise((va).a)
    
void v_normalise(vec_arr_t v) __attribute__ ((__nonnull__));

/* norm / absolute value */
#define V_ABS(s, va) \
    (s) = v_abs((va).a)
    
 scalar_t v_abs(const vec_arr_t v)
    __attribute__ ((__pure__, __nonnull__));

#define M_TRANSPOSE(MT, M) \
    m_transpose((MT).a, (M).a)
    
void m_transpose(mat_arr_t mt, mat_arr_t m) __attribute__ ((__nonnull__));


/* creates rotation matrices, these are untested and might not work */
void m_axis_rot_x(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));
void m_axis_rot_y(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));
void m_axis_rot_z(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));

void m_vec_rot_x(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));
void m_vec_rot_y(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));
void m_vec_rot_z(mat_arr_t m, const angle_t a) __attribute__ ((__nonnull__));



/* creates unit matrix */
#define M_UNIT(M) \
    m_unit((M).a)
    
void m_unit(mat_arr_t m) __attribute__ ((__nonnull__));

/* creates zero matrix */
#define M_ZERO(M) \
    m_zero((M).a)
    
void m_zero(mat_arr_t m) __attribute__ ((__nonnull__));

/* copying functions for duplicating matrices and vectors */
#define V_CPY(dst, src) \
    v_cpy((dst).a, (src).a)
    
void v_cpy(vec_arr_t dst, const vec_arr_t src) __attribute__ ((__nonnull__));

#define M_CPY(dst, src) \
    m_cpy((dst).a, (src).a)

void m_cpy(mat_arr_t dst, mat_arr_t src) __attribute__ ((__nonnull__));

#define V_SET(vr, x, y, z, w) \
    v_set((vr).a, x, y, z, w)
    
void v_set(vec_arr_t v, scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
    __attribute__ ((__nonnull__));


/* Comparative functions */
/*! Compares two vectors for elementvise equality, with a given absolute
 *  tolerance */

#define V_EQ(va, vb, t) \
     v_eq((va).a, (vb).a, t)
     
bool v_eq(const vec_arr_t a, const vec_arr_t b, scalar_t tol)
    __attribute__ ((__pure__, __nonnull__));

/*! Compares two matrices for elementvise equality, with a given absolute
 *  tolerance */
#define M_EQ(A, B, t) \
     m_eq((A).a, (B).a, t)
     
bool m_eq(mat_arr_t a, mat_arr_t b, scalar_t tol)
    __attribute__ ((__pure__, __nonnull__));


#define M_TRANSLATE(M, x, y, z, w) \
    m_translate(&(M), (x), (y), (z), (w))

void m_translate(matrix_t *m, scalar_t x, scalar_t y, scalar_t z, scalar_t w);

#ifdef __cplusplus
}
#endif 

#endif
