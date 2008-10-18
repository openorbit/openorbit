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

#include <vmath/vmath-types.h>
#include <vmath/vmath-constants.h>
#include <vmath/vmath-matvec.h>
#include <vmath/vmath-quaternions.h>
#include <assert.h>

 scalar_t
q_scalar(const quaternion_t q)
{
    return q.w;
}

vector_t
q_vector(const quaternion_t q)
{
    vector_t r = {.a = {q.x, q.y, q.z, 0.0f}};
    return r;
}

void
q_m_convert(matrix_t *m, const quaternion_t q)
{
    scalar_t n = q_dot(q, q);
    scalar_t a = (n > 0.0f) ? S_TWO / n : 0.0f;
    
    scalar_t xa = q.x*a;
    scalar_t ya = q.y*a;
    scalar_t za = q.z*a;
    
    scalar_t xy = q.x*ya;
    scalar_t xz = q.x*za;
    scalar_t yz = q.y*za;

    scalar_t wx = q.w*xa;
    scalar_t wy = q.w*ya;
    scalar_t wz = q.w*za;

    scalar_t xx = q.x*xa;
    scalar_t yy = q.y*ya;
    scalar_t zz = q.z*za;
    
    m->a[0][0] = 1.0f-(yy+zz); m->a[0][1] =        xy-wz;
    m->a[0][2] =        xz+wy;  m->a[0][3] = 0.0f;
    
    m->a[1][0] =        xy+wz;  m->a[1][1] = 1.0f-(xx+zz);
    m->a[1][2] =        yz-wx;  m->a[1][3] = 0.0f;
    
    m->a[2][0] =        xz-wy;  m->a[2][1] =        yz+wx;
    m->a[2][2] = 1.0f-(xx+yy); m->a[2][3] = 0.0f;
    
    m->a[3][0] = 0.0f;        m->a[3][1] = 0.0f;
    m->a[3][2] = 0.0f;        m->a[3][3] = 1.0f;
}
#if 0
void
q_m_convert(mat_arr_t m, const quat_arr_t q)
{
    scalar_t xy = QUAT_X(q)*QUAT_Y(q);
    scalar_t xz = QUAT_X(q)*QUAT_Z(q);
    scalar_t yz = QUAT_Y(q)*QUAT_Z(q);
    
    scalar_t wx = QUAT_W(q)*QUAT_X(q);
    scalar_t wy = QUAT_W(q)*QUAT_Y(q);
    scalar_t wz = QUAT_W(q)*QUAT_Z(q);
    
    scalar_t xx = QUAT_X(q)*QUAT_X(q);
    scalar_t yy = QUAT_Y(q)*QUAT_Y(q);
    scalar_t zz = QUAT_Z(q)*QUAT_Z(q);
    scalar_t ww = QUAT_W(q)*QUAT_W(q);

    
    m[0][0] = ww+xx-yy-zz;  m[0][1] = 2.0*(xy-wz);
    m[0][2] = 2.0*(xz+wy);  m[0][3] = 0.0;
    
    m[1][0] = 2.0*(xy+wz);  m[1][1] = ww-xx+yy-zz;
    m[1][2] = 2.0*(yz-wx);  m[1][3] = 0.0;
    
    m[2][0] = 2.0*(xz-wy);  m[2][1] = 2.0*(yz+wx);
    m[2][2] = ww-xx-yy+zz;  m[2][3] = 0.0;
    
    m[3][0] = 0.0;        m[3][1] = 0.0;
    m[3][2] = 0.0;        m[3][3] = ww+xx+yy+zz;
}
#endif


void
m_q_convert(quaternion_t q, matrix_t *m)
{
    scalar_t tr, s;
    tr = m->a[0][0] + m->a[1][1] + m->a[2][2];
    if (tr >= 0.0f) {
        s = sqrt(tr+m->a[3][3]);
        q.w = s*S_POINT_FIVE;
        s = S_POINT_FIVE / s;
        q.x = (m->a[2][1] - m->a[1][2]) * s;
        q.y = (m->a[0][2] - m->a[2][0]) * s;
        q.z = (m->a[1][0] - m->a[0][1]) * s;
    } else {
        int h = 0;
        if (m->a[1][1] > m->a[0][0]) h = 1;
        if (m->a[2][2] > m->a[h][h]) h = 2;
        switch (h) {
#define CASE_MACRO(i,j,k,I,J,K)                                     \
        case I:                                                     \
            s = sqrt( (m->a[I][I] - (m->a[J][J]+m->a[K][K])) + m->a[3][3] );  \
            q.a[i] = s*0.5f;                                  \
            q.a[j] = (m->a[I][J] + m->a[J][I]) * s;                         \
            q.a[k] = (m->a[K][I] + m->a[I][K]) * s;                         \
            q.w = (m->a[K][J] - m->a[J][K]) * s;                       \
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
        q.x *= s; q.y *= s; q.z *= s; q.z *= s;
    }
}


quaternion_t
q_add(const quaternion_t a, const quaternion_t b)
{
 	return v_add(a, b);
}

quaternion_t
q_mul(const quaternion_t a, const quaternion_t b)
{
	quaternion_t r;
    r.x = a.x*b.w + a.w*b.x	+ a.y*b.z - a.z*b.y;
    r.y = a.y*b.w + a.w*b.y	+ a.z*b.x - a.x*b.z;
    r.z = a.z*b.w + a.w*b.z	+ a.x*b.y - a.y*b.x;
    r.w = a.w*b.w - a.x*b.x	- a.y*b.y - a.z*b.z;
    
    return r;
}

quaternion_t
q_s_div(const quaternion_t q, const scalar_t d)
{
	quaternion_t r;
    r.x = q.x / d;
    r.y = q.y / d;
    r.z = q.z / d;
    r.w = q.w / d;
    return r;
}

 scalar_t
q_dot(const quaternion_t a, const quaternion_t b)
{
    return v_dot(a, b);
}

vector_t
q_cross(const quaternion_t a, const quaternion_t b)
{
    return v_cross(a, b);
}

scalar_t
q_abs(const quaternion_t q)
{
    return v_abs(q);
}

quaternion_t
q_conj(const quaternion_t q)
{
	quaternion_t qp = {.a = {-q.x, -q.y, -q.z, q.w}};
    return qp;
}

quaternion_t
q_repr(const quaternion_t q)
{
	quaternion_t res;
    quaternion_t qp;
    qp = q_conj(q);
    scalar_t d = q_dot(q, q);
    res = q_s_div(qp, d);
	return res;
}

quaternion_t
q_div(const quaternion_t a, const quaternion_t b)
{
	quaternion_t res;
    quaternion_t br;
    br = q_repr(b);
    
	res = q_mul(a, br);
	return res;
}

quaternion_t
q_rotv(const vector_t axis, const angle_t alpha)
{
	quaternion_t q;
    scalar_t Omega = alpha * S_POINT_FIVE;
    scalar_t sin_Omega = sin(Omega);
    q.x = axis.x * sin_Omega;
    q.y = axis.y * sin_Omega;
    q.z = axis.z * sin_Omega;
    q.w = cos(Omega);

	return q;
}
quaternion_t
q_rot(scalar_t x, scalar_t y, scalar_t z, scalar_t alpha)
{
	vector_t axis = {.a = {x, y, z, 1.0f}};
	return q_rotv(axis, alpha);
}
quaternion_t
q_normalise(quaternion_t q)
{
    return v_normalise(q);
}
