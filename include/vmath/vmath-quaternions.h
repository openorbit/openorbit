/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit. Open Orbit is free software: you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

  Some files of Open Orbit have relaxed licensing conditions. This file is
  licenced under the 2-clause BSD licence.

  Redistribution and use of this file in source and binary forms, with or
  without modification, are permitted provided that the following conditions are
  met:
 
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    
 float q_scalar(const quaternion_t q)
    __attribute__ ((__pure__));


float3 q_vector(const quaternion_t q);
float3 v_q_rot(float3 v, quaternion_t q);

#define Q_IDENT vf4_set(0.0, 0.0, 0.0, 1.0)

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

void q_mf3_convert(float3x3 m, quaternion_t q);
void q_mf3_convert_inv(float3x3 m, quaternion_t q);

void q_mf4_convert(float4x4 m, quaternion_t q);
void q_mf4_convert_inv(float4x4 m, quaternion_t q);
quaternion_t q_slerp(quaternion_t q0, quaternion_t q1, float t);

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

quaternion_t mf4_q_convert(float4x4 m) __attribute__ ((__nonnull__));
quaternion_t mf3_q_convert(float3x3 m) __attribute__ ((__nonnull__));

quaternion_t q_add(const quaternion_t a, const quaternion_t b);


quaternion_t
q_mul(const quaternion_t a, const quaternion_t b);
    
quaternion_t q_s_mul(quaternion_t q, float d);

quaternion_t q_s_div(const quaternion_t q, float d);


 float q_dot(quaternion_t a, quaternion_t b)
    __attribute__ ((__pure__));

    
 float3 q_cross(const quaternion_t a, const quaternion_t b);

    
 float q_abs(const quaternion_t q);

    
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
quaternion_t q_rotv(float3 axis, float alpha);

quaternion_t
q_rot(float x, float y, float z, float alpha);

#define Q_ROT_X(q, r)                                           \
    do {                                                        \
        float3 _v = {S_CONST(1.0), S_CONST(0.0),        \
                     S_CONST(0.0)};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)

#define Q_ROT_Y(q, r)                                           \
    do {                                                        \
        float3 _v = {S_CONST(0.0), S_CONST(1.0),        \
                    S_CONST(0.0)};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)


#define Q_ROT_Z(q, r)                                           \
    do {                                                        \
        float3 _v = {S_CONST(0.0), S_CONST(0.0),        \
                    S_CONST(1.0)};      \
        (q) = q_rotv(_v, r);                                    \
    } while (0)


quaternion_t q_normalise(quaternion_t q);

static inline quaternion_t
q_vf3_rot(quaternion_t q, float3 v, float dt)
{
  quaternion_t qv = {v.x, v.y, v.z, 0.0f};
  quaternion_t qp = q_add(q, q_s_mul(q_mul(qv, q), dt/2.0));
  return qp;
}

#ifdef __cplusplus
}
#endif 

#endif /* ! __QUATERNIONS_H__ */
