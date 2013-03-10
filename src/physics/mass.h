/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>
 
 This file is part of Open Orbit.
 
 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PL_MASS_H_
#define PL_MASS_H_

#include <vmath/vmath.h>

struct pl_mass_t {
  float m; //!< Mass in kg
  float minMass; //!< Minimum mass
  float3x3 I; //!< Moment of inertia
  float3x3 I_inv; //!< Inverse of moment of inertia
  float3 cog; //!< Center of gravity
};

typedef struct pl_mass_t pl_mass_t;

void pl_mass_set(pl_mass_t *mo, float m,
               float cox, float coy, float coz,
               float ixx, float iyy, float izz,
               float ixy, float ixz, float iyz);
void pl_mass_translate(pl_mass_t *m, float dx, float dy, float dz);

/*! Adds mass mb to ma
 */
void pl_mass_add(pl_mass_t * restrict ma, const pl_mass_t * restrict mb);
/*! Modify mass object by setting a new mass, do not do this manually as the
    function will take care of updating the inertia tensor as well as the mass. 
 */
void pl_mass_mod(pl_mass_t *m, float newMass);

/*! Reduce mass with delta mass

 The MassRed function offers a quick way to reduce the mass of an object. It
 will respect the minMass property for the mass object.

 \param m mass object
 \param deltaMass the mass in kg to subtract
 \result The amount of mass removed (may be less than the delta)
 */
float pl_mass_red(pl_mass_t *m, float deltaMass);

/*!
  Set the minimum mass
 */
void pl_mass_set_min(pl_mass_t *m, float minMass);


/*! Rotate mass m by rotation matrix rm
 */
void pl_mass_rotate_m(pl_mass_t *m, const float3x3 rm);

/*! Rotate mass m by quaternion q
 */
void pl_mass_rotate_q(pl_mass_t *m, quaternion_t q);


void pl_mass_hollow_cylinder(pl_mass_t *mo, float m, float r);
void pl_mass_solid_cylinder(pl_mass_t *mo, float m, float r, float h);
void pl_mass_walled_cylinder(pl_mass_t *mo, float m, float out_r, float in_r, float h);
void pl_mass_solid_sphere(pl_mass_t *mo, float m, float r);
void pl_mass_hollow_sphere(pl_mass_t *mo, float m, float r);
void pl_mass_solid_cone(pl_mass_t *mo, float m, float r, float h);

#endif /* ! PL_MASS_H_ */
