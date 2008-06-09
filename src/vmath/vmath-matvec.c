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

 
#include <tgmath.h>
#include <string.h>

#include <vmath/vmath-matvec.h>
#include <vmath/vmath-constants.h>

/* standard non vectorised routines */
vector_t
m_v_mul(matrix_t *a, const vector_t v) {
	vector_t res;
	for (int i = 0 ; i < 4 ; i ++) {
        res.a[i] = a->a[i][0] * v.a[0] + a->a[i][1] * v.a[1]
                 + a->a[i][2] * v.a[2] + a->a[i][3] * v.a[3];
    }
	return res;
}

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


void
m_add(matrix_t *res, matrix_t *a, matrix_t *b)
{
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0 ; j < 4 ; j ++) {
            res->a[i][j] = a->a[i][j] + b->a[i][j];
        }
    }
}

vector_t
v_s_add(vector_t *a, scalar_t b)
{
	vector_t c = {.s.x = b, .s.y = b, .s.z = b, .s.w = b};
	vector_t r;
	r = v_add(*a, c);
	return r;
}

vector_t
v_add(vector_t a, const vector_t b)
{
	vector_t res;
    for (int i = 0 ; i < 4 ; i ++) {
        res.a[i] = a.a[i] + b.a[i];
    }
	return res;
}

vector_t
v_sub(vector_t a, const vector_t b)
{
	vector_t res;
    for (int i = 0 ; i < 4 ; i ++) {
        res.a[i] = a.a[i] - b.a[i];
    }
	return res;
}


vector_t
v_cross(vector_t a, vector_t b)
{
    vector_t res;
    res.a[0] = a.a[1]*b.a[2]-a.a[2]*b.a[1];
    res.a[1] = a.a[2]*b.a[0]-a.a[0]*b.a[2];
    res.a[2] = a.a[0]*b.a[1]-a.a[1]*b.a[0];
    return res;
}

scalar_t
v_dot(const vector_t a, const vector_t b)
{
    return a.a[0]*b.a[0] + a.a[1]*b.a[1] + a.a[2]*b.a[2] + a.a[3]*b.a[3];
}

vector_t
v_s_mul(vector_t a, scalar_t s)
{
	vector_t res;
    res.a[0] = a.a[0] * s;
    res.a[1] = a.a[1] * s;
    res.a[2] = a.a[2] * s;
    res.a[3] = a.a[3] * s;
}

vector_t
v_s_div(vector_t a, scalar_t s)
{
    vector_t res;
    scalar_t d = S_CONST(1.0) / s;
    res.a[0] = a.a[0] * d;
    res.a[1] = a.a[1] * d;
    res.a[2] = a.a[2] * d;
    res.a[3] = a.a[3] * d;
    return res;
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

vector_t
v_normalise(vector_t v)
{
    scalar_t norm = v_abs(v);
    return v_s_mul(v, S_ONE/norm);
}

scalar_t
m_det(const matrix_t *m)
{
    // compute the four subdeterminants (Saurrus)
    // note, this version has been written to work, not to be efficient in any
    // way
    scalar_t sub_det[4];
    
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
    
    scalar_t det = MAT_ELEM(*m, 0, 0) * sub_det[0]
                 - MAT_ELEM(*m, 0, 1) * sub_det[1]
                 + MAT_ELEM(*m, 0, 2) * sub_det[2]
                 - MAT_ELEM(*m, 0, 3) * sub_det[3];
    
    return det;
}

scalar_t
m_subdet3(const matrix_t *m, int k, int l)
{
    scalar_t acc = S_CONST(0.0);
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
    scalar_t sign = S_CONST(1.0);
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
    scalar_t det = m_det(M);
    matrix_t M_adj = m_adj(M);
    matrix_t M_inv;
    
    if (det != S_CONST(0.0)) {
        scalar_t sign = S_CONST(1.0);
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


scalar_t
v_abs(const vector_t v)
{
    return sqrt(v.a[0]*v.a[0] + v.a[1]*v.a[1] + v.a[2]*v.a[2] + v.a[3]*v.a[3]);
}

void
m_transpose(matrix_t *mt, matrix_t *m)
{
    for ( unsigned int i = 0; i < 4; i += 1 ) {
        for ( unsigned int j = 0; j < 4; j += 1 ) {
            mt->a[j][i] = m->a[i][j];
        }
    }
}

void
m_axis_rot_x(matrix_t *m, const angle_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = S_ONE;
    m->a[1][1] = cos_a; m->a[1][1] = sin_a; 
    m->a[2][1] = -sin_a; m->a[2][1] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_axis_rot_y(matrix_t *m, scalar_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][2] = -sin_a;
    m->a[1][1] = S_ONE;
    m->a[2][0] = sin_a; m->a[2][2] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_axis_rot_z(matrix_t *m, scalar_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][1] = sin_a;
    m->a[1][0] = -sin_a; m->a[1][1] = cos_a;
    m->a[2][2] = S_ONE;
    m->a[3][3] = S_ONE;
}


void
m_vec_rot_x(matrix_t *m, scalar_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = S_ONE;
    m->a[1][1] = cos_a; m->a[1][1] = -sin_a; 
    m->a[2][1] = sin_a; m->a[2][1] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_vec_rot_y(matrix_t *m, scalar_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
    memset(m, 0, sizeof(matrix_t));
    m->a[0][0] = cos_a; m->a[0][2] = sin_a;
    m->a[1][1] = S_ONE;
    m->a[2][0] = -sin_a; m->a[2][2] = cos_a;
    m->a[3][3] = S_ONE;
}

void
m_vec_rot_z(matrix_t *m, scalar_t a)
{
    scalar_t sin_a = sin(a);
    scalar_t cos_a = cos(a);
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
m_cpy(matrix_t * restrict dst, matrix_t * restrict src)
{
    for (int i = 0 ; i < 4 ; i ++) {
        dst->a[i][0] = src->a[i][0];
        dst->a[i][1] = src->a[i][1];
        dst->a[i][2] = src->a[i][2];
        dst->a[i][3] = src->a[i][3];
    }
}


vector_t
v_set(scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
{
	vector_t v = {.s.x = v0, .s.y = v1, .s.z = v2, .s.w = v3};
	return v;
}

bool
v_eq(vector_t a, vector_t  b, scalar_t tol)
{
    for (int i = 0 ; i < 4 ; i ++) {
        if (!((a.a[i] <= b.a[i]+tol) && (a.a[i] >= b.a[i]-tol))) {
            return false;
        }
    }
    
    return true;
}

bool
m_eq(const matrix_t *a, const matrix_t *b, scalar_t tol)
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
m_translate(matrix_t *m, scalar_t x, scalar_t y, scalar_t z, scalar_t w)
{
    memset(m, 0, sizeof(mat_arr_t));    
    
    m->a[0][0] = x;
    m->a[1][1] = y;
    m->a[2][2] = z;
    m->a[3][3] = w;
}
