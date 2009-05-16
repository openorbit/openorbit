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


#include <vmath/vmath-types.h>
#include <vmath/vmath-constants.h>
#include <vmath/vmath-matvec.h>

#define Q_X 0
#define Q_Y 1
#define Q_Z 2
#define Q_W 3

#define Q_CPY(dst, src) ((dst).s = (src).s)
    
#define QUAT_X(q) (q)[Q_X]
#define QUAT_Y(q) (q)[Q_Y]
#define QUAT_Z(q) (q)[Q_Z]
#define QUAT_W(q) (q)[Q_W]

    
 scalar_t q_scalar(const quaternion_t q)
    __attribute__ ((__pure__));


vector_t q_vector(const quaternion_t q);


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

void q_m_convert(matrix_t *m, const quaternion_t q)
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

void m_q_convert(quaternion_t q, matrix_t *m) __attribute__ ((__nonnull__));

    
 quaternion_t q_add(const quaternion_t a, const quaternion_t b);


quaternion_t
q_mul(const quaternion_t a, const quaternion_t b);
    
    
quaternion_t q_s_div(const quaternion_t q, const scalar_t d);


 scalar_t q_dot(const quaternion_t a, const quaternion_t b)
    __attribute__ ((__pure__));

    
 vector_t q_cross(const quaternion_t a, const quaternion_t b);

    
 scalar_t q_abs(const quaternion_t q);

    
quaternion_t
q_conj(const quaternion_t q);


quaternion_t q_repr(const quaternion_t q);


quaternion_t q_div(const quaternion_t a, const quaternion_t b);

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
quaternion_t q_rotv(const quaternion_t axis, const angle_t alpha);

quaternion_t
q_rot(scalar_t x, scalar_t y, scalar_t z, scalar_t alpha);

#define Q_ROT_X(q, r)                                           \
    do {                                                        \
        vector_t _v = {.a = {S_CONST(1.0), S_CONST(0.0),        \
                             S_CONST(0.0), S_CONST(1.0)}};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)

#define Q_ROT_Y(q, r)                                           \
    do {                                                        \
        vector_t _v = {.a = {S_CONST(0.0), S_CONST(1.0),        \
                             S_CONST(0.0), S_CONST(1.0)}};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)


#define Q_ROT_Z(q, r)                                           \
    do {                                                        \
        vector_t _v = {.a = {S_CONST(0.0), S_CONST(0.0),        \
                             S_CONST(1.0), S_CONST(1.0)}};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)


quaternion_t q_normalise(quaternion_t q);


#ifdef __cplusplus
}
#endif 

#endif /* ! __QUATERNIONS_H__ */
