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


#ifndef __QUATERNIONS_H__
#define __QUATERNIONS_H__
#ifdef __cplusplus
extern "C" {
#endif 


#include <math/types.h>
#include <math/constants.h>
#include <math/linalg.h>

#define Q_X 0
#define Q_Y 1
#define Q_Z 2
#define Q_W 3

#define Q_CPY(dst, src) ((dst).s = (src).s)
    
#define QUAT_X(q) (q)[Q_X]
#define QUAT_Y(q) (q)[Q_Y]
#define QUAT_Z(q) (q)[Q_Z]
#define QUAT_W(q) (q)[Q_W]

#define Q_SCALAR(s, q) \
    (s) = q_scalar((q).a)
    
 scalar_t q_scalar(const quat_arr_t q)
    __attribute__ ((__pure__, __nonnull__));

#define Q_VECTOR(v, q) \
    q_vector((v).a, (q).a)

void q_vector(vec_arr_t r, const quat_arr_t q)
    __attribute__ ((__nonnull__));


/*!
 * \brief Converts a quaternion to a rotation matrix.
 * 
 * The q_m_convert function converts quaternion q into the rotational matrix m.
 * The rotational matrix is in a right handed coordinate system. And thus
 * compatible with toolkits such as OpenGL.
 * 
 * \param m A reference to the resulting matrix.
 * \param q The quaternion.
 */
#define Q_M_CONVERT(M, q) \
    q_m_convert((M).a, (q).a)

void q_m_convert(mat_arr_t m, const quat_arr_t q)
    __attribute__ ((__nonnull__));

/*!
 * \brief Converts a rotation matrix to a quaternion.
 * 
 * The m_q_convert function converts a rotational matrix m into a quaternion q.
 * The rotational matrix shall be specified for a right handed coordinate
 * system. For more information, see Ken Shoemake's paper on quaternion
 * rotation.
 * 
 * \param q A reference to the resulting quaternion.
 * \param m The rotational matrix.
 */

#define M_Q_CONVERT(q, M) \
    m_q_convert((q).a, (M).a)

void m_q_convert(quat_arr_t q, mat_arr_t m) __attribute__ ((__nonnull__));

#define Q_ADD(qa, qb, qc) \
    q_add((qa).a, (qb).a, (qc).a)
    
 void q_add(quat_arr_t r, quat_arr_t a, const quat_arr_t b)
    __attribute__ ((__nonnull__));

#define Q_MUL(qa, qb, qc) \
    q_mul((qa).a, (qb).a, (qc).a)

void q_mul(quat_arr_t r, const quat_arr_t a, const quat_arr_t b)
    __attribute__ ((__nonnull__));
    
#define Q_S_DIV(qr, qa, qb) \
    q_s_div((qr).a, (qa).a, (qb).a)
    
void q_s_div(quat_arr_t r, const quat_arr_t q, const scalar_t d)
    __attribute__ ((__nonnull__));

#define Q_DOT(s, qa, qb) \
    (s) = q_dot((qa).a, (qb).a)

 scalar_t q_dot(const quat_arr_t a, const quat_arr_t b)
    __attribute__ ((__pure__, __nonnull__));

#define Q_CROSS(qr, qa, qb) \
    q_cross((qr).a, (qa).a, (qb).a)
    
 void q_cross(vec_arr_t r, const quat_arr_t a, const quat_arr_t b)
    __attribute__ ((__nonnull__));

#define Q_ABS(s, q) \
    (s) = q_abs((q).a)
    
 scalar_t q_abs(const quat_arr_t q)
    __attribute__ ((__pure__, __nonnull__));

#define Q_CONJ(qr, qa) \
    q_conj((qr).a, (qa).a)
    
 void q_conj(quat_arr_t qp, const quat_arr_t q)
    __attribute__ ((__nonnull__));

#define Q_REPR(qr, qa) \
    q_repr((qr).a, (qa).a)

void q_repr(quat_arr_t res, const quat_arr_t q)
    __attribute__ ((__nonnull__));

#define Q_DIV(qr, qa, qb) \
    q_div((qr).a, (qa).a, (qb).a)

void q_div(quat_arr_t res, const quat_arr_t a, const quat_arr_t b)
    __attribute__ ((__nonnull__));

/*!
 * \brief   Creates a rotation quaternion
 * 
 * The q_rot function creates a rotation quaternion for a right hand rotation
 * of alpha radians around the specified axis.
 * 
 * \param q     A reference to the new quaternion
 * \param axis  A unit vector describing the axis of rotation 
 * \param alpha Rotation in radians.
*/
void q_rot(quat_arr_t q, const axis_arr_t axis, const angle_t alpha)
    __attribute__ ((__nonnull__));

#define Q_ROT(q, ax, ang) \
    q_rot((q).a, (ax).a, ang)

#define Q_ROT_X(q, r)                                            \
    do {                                                         \
        vector_t v = {.s.x = S_CONST(1.0), .s.y = S_CONST(0.0),  \
                      .s.z = S_CONST(0.0), .s.w = S_CONST(0.0)}; \
        q_rot((q).a, v.a, r);                                    \
    } while (0)

#define Q_ROT_Y(q, r)                                            \
    do {                                                         \
        vector_t v = {.s.x = S_CONST(0.0), .s.y = S_CONST(1.0),  \
                      .s.z = S_CONST(0.0), .s.w = S_CONST(0.0)}; \
        q_rot((q).a, v.a, r);                                    \
    } while (0)


#define Q_ROT_Z(q, r)                                            \
    do {                                                         \
        vector_t v = {.s.x = S_CONST(0.0), .s.y = S_CONST(0.0),  \
                      .s.z = S_CONST(1.0), .s.w = S_CONST(0.0)}; \
        q_rot((q).a, v.a, r);                                    \
    } while (0)


void q_normalise(quat_arr_t q)
    __attribute__ ((__nonnull__));

#define Q_NORMALISE(q) \
    q_normalise((q).a)

#ifdef __cplusplus
}
#endif 

#endif /* ! __QUATERNIONS_H__ */
