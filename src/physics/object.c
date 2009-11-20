/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <ode/ode.h>
#include "physics.h"
#include "common/lwcoord.h"
#include <assert.h>


void
plMassSet(PLmass *mo, float m,
          float ixx, float iyy, float izz,
          float ixy, float ixz, float iyz)
{
  memset(mo, 0, sizeof(PLmass));

  mo->m = m;
  mo->moInert[0][0] = ixx;
  mo->moInert[1][1] = iyy;
  mo->moInert[2][2] = izz;
  mo->moInert[0][1] = ixy;
  mo->moInert[1][0] = ixy;
  mo->moInert[0][2] = ixz;
  mo->moInert[2][0] = ixz;
  mo->moInert[1][2] = iyz;
  mo->moInert[2][1] = iyz;
}

void
plMassTranslate(PLmass *m, float dx, float dy, float dz)
{
  matrix_t re;
  m_unit(&re);
  float3 r = vf3_set(dx, dy, dz);
  float rdot = vf3_dot(r, r);
  re.a[0][0] = rdot;
  re.a[1][1] = rdot;
  re.a[2][2] = rdot;
  matrix_t rout;
  vf3_outprod(&rout, r, r);
  matrix_t madj;
  m_sub(&madj, &re, &rout);
  matrix_t mres;
  m_s_mul(&mres, &madj, m->m);

  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
      m->moInert[i][j] += mres.a[i][j];
    }
  }
}

void
plMassRotateM(PLmass *m, const PLfloat3x3 *mat)
{
  // We want to rotate the inertia tensor with
  // Ir = M * I0 * M^-1
  // Please verify
}

void
plMassRotateQ(PLmass *m, quaternion_t q)
{

}


void
plMassAdd(PLmass * restrict md, const PLmass * restrict ms)
{
  md->m += ms->m;

  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
      md->moInert[i][j] += ms->moInert[i][j];
    }
  }
}

void
plMassMod(PLmass *m, float newMass)
{
  float s = newMass / m->m;
  m->m = newMass;

  for (int i = 0 ; i < 3 ; ++ i) {
    for (int j = 0 ; j < 3 ; ++ j) {
      m->moInert[i][j] *= s;
    }
  }
}



/*
  One of the problems with space is that it is big

  For our physics system, this unfortunatelly means that we need to reset the position
  based on the local segmented position after every step. This is OK in general, but
  will cause big problems for collission detection. Though, since the data needs to be
  converted to a single segment for rendering, it may be possible to use that data for
  colission detection.
*/
/*
void dBodyAddForce            (dBodyID, dReal fx, dReal fy, dReal fz);
void dBodyAddTorque           (dBodyID, dReal fx, dReal fy, dReal fz);
void dBodyAddRelForce         (dBodyID, dReal fx, dReal fy, dReal fz);
void dBodyAddRelTorque        (dBodyID, dReal fx, dReal fy, dReal fz);
void dBodyAddForceAtPos       (dBodyID, dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz);
void dBodyAddForceAtRelPos    (dBodyID, dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz);
void dBodyAddRelForceAtPos    (dBodyID, dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz);
void dBodyAddRelForceAtRelPos (dBodyID, dReal fx, dReal fy, dReal fz, dReal px, dReal py, dReal pz);
typedef struct dMass {
    dReal mass; // total mass of the rigid body
    dVector3 c; // center of gravity position in body frame (x,y,z)
    dMatrix3 I; // 3x3 inertia tensor in body frame, about POR
} dMass;
void dMassSetParameters (dMass *, dReal themass,
                         dReal cgx, dReal cgy, dReal cgz,
                         dReal I11, dReal I22, dReal I33,
                         dReal I12, dReal I13, dReal I23);
void dMassAdd (dMass *a, const dMass *b);
void dMassTranslate (dMass *, dReal x, dReal y, dReal z);

*/
PLobject*
plObject3f(float x, float y, float z)
{
  PLobject *obj = malloc(sizeof(PLobject));
  return obj;
}
void
plForce3f(PLobject *obj, float x, float y, float z)
{
  dBodyAddForce(obj->id, x, y, z);
}
void
plForce3d(PLobject *obj, double x, double y, double z)
{
  dBodyAddForce(obj->id, x, y, z);
}
void
plForce3dv(PLobject *obj, PLdouble3 f)
{
  dBodyAddForce(obj->id, ((double*)&f)[0], ((double*)&f)[1], ((double*)&f)[2]);
}

void
plForceRelative3f(PLobject *obj, float fx, float fy, float fz)
{
  dBodyAddRelForce(obj->id, fx, fy, fz);
}

void
plForceRelative3d(PLobject *obj, double fx, double fy, double fz)
{
  dBodyAddRelForce(obj->id, fx, fy, fz);
}
void plForceRelative3dv(PLobject *obj, PLdouble3 f)
{
  dBodyAddRelForce(obj->id,
                      ((double*)&f)[0], ((double*)&f)[1], ((double*)&f)[2]);
}

void
plForceRelativePos3f(PLobject *obj,
                     float fx, float fy, float fz,
                     float px, float py, float pz)
{
  dBodyAddRelForceAtRelPos(obj->id, fx, fy, fz, px, py, pz);
}

void
plForceRelativePos3d(PLobject *obj,
                     double fx, double fy, double fz,
                     double px, double py, double pz)
{
  dBodyAddRelForceAtRelPos(obj->id, fx, fy, fz, px, py, pz);
}

void
plForceRelativePos3dv(PLobject *obj, PLdouble3 f, PLdouble3 p)
{
  dBodyAddRelForceAtRelPos(obj->id,
                           ((double*)&f)[0], ((double*)&f)[1], ((double*)&f)[2],
                           ((double*)&p)[0], ((double*)&p)[1], ((double*)&p)[2]);
}


void plStepObjectf(PLobject *obj, float dt)
{
  /*
  const dReal * dBodyGetPosition (dBodyID);
  const dReal * dBodyGetRotation (dBodyID);
  const dReal * dBodyGetQuaternion (dBodyID);
  const dReal * dBodyGetLinearVel (dBodyID);
  const dReal * dBodyGetAngularVel (dBodyID);
  */
}

void
plStepObjectd(PLobject *obj, double dt)
{

}

void
plNormaliseObject(PLobject *obj)
{
  // Since we are using non safe casts here, we must ensure this in case someone upgrades
  // to 64 bit positions
  assert(sizeof(obj->p.offs) == sizeof(PLfloat3));

  const dReal *pos = dBodyGetPosition(obj->id);
  ((float*)&obj->p.offs)[0] = pos[0];
  ((float*)&obj->p.offs)[1] = pos[1];
  ((float*)&obj->p.offs)[2] = pos[2];

  ooLwcNormalise(&obj->p);

  dBodySetPosition(obj->id,
                  ((float*)&obj->p.offs)[0],
                  ((float*)&obj->p.offs)[1],
                  ((float*)&obj->p.offs)[2]);
}

void
plClearObject(PLobject *obj)
{
  dBodySetPosition(obj->id,
                  ((float*)&obj->p.offs)[0],
                  ((float*)&obj->p.offs)[1],
                  ((float*)&obj->p.offs)[2]);
  dBodySetForce(obj->id, 0.0, 0.0, 0.0);
  dBodySetTorque (obj->id, 0.0, 0.0, 0.0);
}
