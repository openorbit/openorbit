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
#include <vmath/vmath-types.h>


scalar_t v_dot(const vector_t a, const vector_t b)
    __attribute__ ((__pure__, __nonnull__));


vector_t m_v_mul(matrix_t *a, const vector_t v);

    
void m_transpose(matrix_t *mt, matrix_t *m) __attribute__ ((__nonnull__));


void m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
    __attribute__ ((__nonnull__));    

#if ENABLE_VECTORISE
    #undef M_MUL
    #define M_MUL(A, B, C) \
        do { \
            matrix_t tmp_res, tmp_c;\
            M_TRANSPOSE(tmp_c, (C));\
            for (int i = 0 ; i < 4 ; i ++) {\
                for (int j = 0; j < 4 ; j ++) {\
                    V_DOT(tmp_res.a[i][j], (B).v[i], tmp_c.v[j]);\
                }\
            }\
        } while (0)
#endif
    
    
    
void m_add(matrix_t *res, matrix_t *a, matrix_t *b)
    __attribute__ ((__nonnull__));

vector_t v_s_add(vector_t a, scalar_t b);

vector_t v_add(vector_t a, const vector_t b)
    __attribute__ ((__nonnull__));


vector_t v_sub(vector_t a, const vector_t b)
    __attribute__ ((__nonnull__));


vector_t v_s_mul(vector_t a, scalar_t s)
    __attribute__ ((__nonnull__));


vector_t v_s_div(vector_t a, scalar_t s)
    __attribute__ ((__nonnull__));


/* This is really a 3d x product */
    
vector_t v_cross(vector_t a, vector_t b)
    __attribute__ ((__nonnull__));
    
vector_t v_normalise(vector_t v) __attribute__ ((__nonnull__));
    
 scalar_t v_abs(const vector_t v)
    __attribute__ ((__pure__, __nonnull__));

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
    __attribute__ ((__pure__, __nonnull__));

/*! Compares two matrices for elementvise equality, with a given absolute
 *  tolerance */
bool m_eq(const matrix_t *a, const matrix_t *b, scalar_t tol)
    __attribute__ ((__pure__, __nonnull__));


void m_translate(matrix_t *m, scalar_t x, scalar_t y, scalar_t z, scalar_t w);

#ifdef __cplusplus
}
#endif 

#endif
