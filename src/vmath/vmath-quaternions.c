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
q_scalar(const quat_arr_t q)
{
    return QUAT_W(q);
}

void
q_vector(vec_arr_t r, const quat_arr_t q)
{
    r[0] = QUAT_X(q);
    r[1] = QUAT_Y(q);
    r[2] = QUAT_Z(q);
    r[3] = S_ZERO;
}

void
q_m_convert(mat_arr_t m, const quat_arr_t q)
{
    scalar_t n = q_dot(q, q);
    scalar_t a = (n > S_ZERO) ? S_TWO / n : S_ZERO;
    
    scalar_t xa = QUAT_X(q)*a;
    scalar_t ya = QUAT_Y(q)*a;
    scalar_t za = QUAT_Z(q)*a;
    
    scalar_t xy = QUAT_X(q)*ya;
    scalar_t xz = QUAT_X(q)*za;
    scalar_t yz = QUAT_Y(q)*za;
    
    scalar_t wx = QUAT_W(q)*xa;
    scalar_t wy = QUAT_W(q)*ya;
    scalar_t wz = QUAT_W(q)*za;
    
    scalar_t xx = QUAT_X(q)*xa;
    scalar_t yy = QUAT_Y(q)*ya;
    scalar_t zz = QUAT_Z(q)*za;
    
    m[0][0] = S_ONE-(yy+zz); m[0][1] =        xy-wz;
    m[0][2] =        xz+wy;  m[0][3] = S_ZERO;
    
    m[1][0] =        xy+wz;  m[1][1] = S_ONE-(xx+zz);
    m[1][2] =        yz-wx;  m[1][3] = S_ZERO;
    
    m[2][0] =        xz-wy;  m[2][1] =        yz+wx;
    m[2][2] = S_ONE-(xx+yy); m[2][3] = S_ZERO;
    
    m[3][0] = S_ZERO;        m[3][1] = S_ZERO;
    m[3][2] = S_ZERO;        m[3][3] = S_ONE;
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
m_q_convert(quat_arr_t q, mat_arr_t m)
{
    scalar_t tr, s;
    tr = m[0][0] + m[1][1] + m[2][2];
    if (tr >= S_ZERO) {
        s = sqrt(tr+m[3][3]);
        QUAT_W(q) = s*S_POINT_FIVE;
        s = S_POINT_FIVE / s;
        QUAT_X(q) = (m[2][1] - m[1][2]) * s;
        QUAT_Y(q) = (m[0][2] - m[2][0]) * s;
        QUAT_Z(q) = (m[1][0] - m[0][1]) * s;
    } else {
        int h = 0;
        if (m[1][1] > m[0][0]) h = 1;
        if (m[2][2] > m[h][h]) h = 2;
        switch (h) {
#define CASE_MACRO(i,j,k,I,J,K)                                     \
        case I:                                                     \
            s = sqrt( (m[I][I] - (m[J][J]+m[K][K])) + m[3][3] );  \
            q[i] = s*S_POINT_FIVE;                                  \
            q[j] = (m[I][J] + m[J][I]) * s;                         \
            q[k] = (m[K][I] + m[I][K]) * s;                         \
            QUAT_W(q) = (m[K][J] - m[J][K]) * s;                       \
            break
        CASE_MACRO(Q_X, Q_Y, Q_Z, 0, 1, 2);
        CASE_MACRO(Q_Y, Q_Z, Q_X, 1, 2, 0);
        CASE_MACRO(Q_Z, Q_X, Q_Y, 2, 0, 1);
#undef CASE_MACRO
        default:
            assert(0);
        }
    }
    
    if (m[3][3] != S_ZERO) {
        s = S_ONE / sqrt(m[3][3]);
        QUAT_X(q) *= s; QUAT_Y(q) *= s; QUAT_Z(q) *= s; QUAT_Z(q) *= s;
    }
}


 void
q_add(quat_arr_t r, quat_arr_t a, const quat_arr_t b)
{
    v_add(r, a, b);
}

void
q_mul(quat_arr_t r, const quat_arr_t a, const quat_arr_t b)
{
    QUAT_X(r) = QUAT_X(a)*QUAT_W(b) + QUAT_W(a)*QUAT_X(b) + QUAT_Y(a)*QUAT_Z(b) - QUAT_Z(a)*QUAT_Y(b);
    QUAT_Y(r) = QUAT_Y(a)*QUAT_W(b) + QUAT_W(a)*QUAT_Y(b) + QUAT_Z(a)*QUAT_X(b) - QUAT_X(a)*QUAT_Z(b);
    QUAT_Z(r) = QUAT_Z(a)*QUAT_W(b) + QUAT_W(a)*QUAT_Z(b) + QUAT_X(a)*QUAT_Y(b) - QUAT_Y(a)*QUAT_X(b);
    QUAT_W(r) = QUAT_W(a)*QUAT_W(b) - QUAT_X(a)*QUAT_X(b) - QUAT_Y(a)*QUAT_Y(b) - QUAT_Z(a)*QUAT_Z(b);
}

void
q_s_div(quat_arr_t r, const quat_arr_t q, const scalar_t d)
{
    QUAT_X(r) = QUAT_X(q) / d;
    QUAT_Y(r) = QUAT_Y(q) / d;
    QUAT_Z(r) = QUAT_Z(q) / d;
    QUAT_W(r) = QUAT_W(q) / d;
}

 scalar_t
q_dot(const quat_arr_t a, const quat_arr_t b)
{
    return v_dot(a, b);
}

 void
q_cross(vec_arr_t r, const quat_arr_t a, const quat_arr_t b)
{
    v_cross(r, a, b);
}

 scalar_t
q_abs(const quat_arr_t q)
{
    return v_abs(q);
}

void
q_conj(quat_arr_t qp, const quat_arr_t q)
{
    QUAT_X(qp) = -QUAT_X(q);
    QUAT_Y(qp) = -QUAT_Y(q);
    QUAT_Z(qp) = -QUAT_Z(q);
    QUAT_W(qp) = QUAT_W(q);
}

void
q_repr(quat_arr_t res, const quat_arr_t q)
{
    quat_arr_t qp;
    q_conj(qp, q);
    scalar_t d = q_dot(q, q);
    q_s_div(res, qp, d);
}

void
q_div(quat_arr_t res, const quat_arr_t a, const quat_arr_t b)
{
    quat_arr_t br;
    q_repr(br, b);
    q_mul(res, a, br);
}

void
q_rot(quat_arr_t q, const axis_arr_t axis, const angle_t alpha)
{
    scalar_t Omega = alpha * S_POINT_FIVE;
    scalar_t sin_Omega = sin(Omega);
    QUAT_X(q) = axis[0] * sin_Omega;
    QUAT_Y(q) = axis[1] * sin_Omega;
    QUAT_Z(q) = axis[2] * sin_Omega;
    QUAT_W(q) = cos(Omega);
}

void
q_normalise(quat_arr_t q)
{
    v_normalise(q);
}