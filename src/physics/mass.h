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

#ifndef PL_MASS_H_
#define PL_MASS_H_

#include <vmath/vmath.h>

struct PLmass {
  float m; //!< Mass in kg
  float minMass; //!< Minimum mass
  float3x3 I; //!< Moment of inertia
  float3 cog; //!< Center of gravity
};

typedef struct PLmass PLmass;

void plMassSet(PLmass *mo, float m,
               float cox, float coy, float coz,
               float ixx, float iyy, float izz,
               float ixy, float ixz, float iyz);
void plMassTranslate(PLmass *m, float dx, float dy, float dz);

/*! Adds mass mb to ma
 */
void plMassAdd(PLmass * restrict ma, const PLmass * restrict mb);
/*! Modify mass object by setting a new mass, do not do this manually as the
    function will take care of updating the inertia tensor as well as the mass. 
 */
void plMassMod(PLmass *m, float newMass);

/*! Reduce mass with delta mass

 The MassRed function offers a quick way to reduce the mass of an object. It
 will respect the minMass property for the mass object.

 \param m mass object
 \param deltaMass the mass in kg to subtract
 \result The amount of mass removed (may be less than the delta)
 */
float plMassRed(PLmass *m, float deltaMass);

/*!
  Set the minimum mass
 */
void plMassSetMin(PLmass *m, float minMass);


/*! Rotate mass m by rotation matrix rm
 */
void plMassRotateM(PLmass *m, const float3x3 rm);

/*! Rotate mass m by quaternion q
 */
void plMassRotateQ(PLmass *m, quaternion_t q);


void plMassHollowCylinder(PLmass *mo, float m, float r);
void plMassSolidCylinder(PLmass *mo, float m, float r, float h);
void plMassWalledCylinder(PLmass *mo, float m, float out_r, float in_r, float h);
void plMassSolidSphere(PLmass *mo, float m, float r);
void plMassHollowSphere(PLmass *mo, float m, float r);
void plMassSolidCone(PLmass *mo, float m, float r, float h);

#endif /* ! PL_MASS_H_ */
