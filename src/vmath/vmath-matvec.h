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
#include <vmath/vmath-types.h>
#include <vmath/vmath-matvec.inl>


vector_t v_neg(vector_t v) __attribute__((__pure__));

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


#ifdef __cplusplus
}
#endif 

#endif
