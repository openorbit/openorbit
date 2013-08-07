/*
 Copyright 2013 Mattias Holm <lorrden(at)openorbit.org>

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

#include "physics/celestial-object.h"
#include "physics/world.h"
#include "physics/reftypes.h"
#include "physics/octtree.h"

#include <celmek/celmek.h>

void
pl_celinit(pl_world_t *world)
{
  cm_init();

  const cm_orbits_t *orbits = cm_get_orbits();

  for (int i = 0 ; i < orbits->len ; i ++) {
    pl_new_celobject(world, orbits->orbits[i]);
  }
}


pl_celobject_t*
pl_new_celobject(pl_world_t *world, cm_orbit_t *cm_orbit)
{
  pl_celobject_t *celobj = smalloc(sizeof(pl_celobject_t));
  celobj->cm_orbit = cm_orbit;
  pl_octtree_insert_celbody(world->octtree, celobj);
  pl_world_add_celobject(world, celobj);
  return celobj;
}

quatd_t
pl_celobject_get_body_quat(pl_celobject_t *celobj)
{
  return cm_orbit_get_bodyq(celobj->cm_orbit);
}

quatd_t
pl_celobject_get_orbit_quat(pl_celobject_t *celobj)
{
  return cm_orbit_get_orbitq(celobj->cm_orbit);
}

float3
pl_celobject_get_vel(pl_celobject_t *celobj)
{
  return vf3_set(celobj->cm_orbit->v.x,
                 celobj->cm_orbit->v.y,
                 celobj->cm_orbit->v.z);
}

lwcoord_t
pl_celobject_get_lwc(pl_celobject_t *celobj)
{
  lwcoord_t lwc;
  lwc_set(&lwc,
          celobj->cm_orbit->p.x,
          celobj->cm_orbit->p.y,
          celobj->cm_orbit->p.z);

  return lwc;
}

double3
pl_celobject_compute_airspeed_for_object(pl_celobject_t *celobj, pl_object_t *obj)
{
  double rot_vel = celobj->cm_orbit->W_prime / CM_SEC_PER_JD;
  // rot vel is in radians per day, this means that the time it takes to rotate
  // one revolution is 2 pi / rot_vel (or 1/rot_vel = days per radian)
  // Now, the base speed of the air should be the current tangent of the
  // rotation plus the velocity of the planet.

  double3 base = celobj->cm_orbit->v; // m/s

  // Compute local body relative coordinate of the object
  double3 local_p = lwc_globald(&obj->p) - celobj->cm_orbit->p;
  double3x3 rot; // Create the reverse rotation matrix, i.e. we are rotating the
                 // world around the planet instead of the planet itself
  md3_zxz_rotmatrix(rot, -celobj->cm_orbit->W,
                    -(M_PI_4 - celobj->cm_orbit->r.y),
                    -(M_PI_4 + celobj->cm_orbit->r.x));
  double3 local_p_rot = md3_v_mul(rot, local_p);
  // Local pos is now in planet centered rectangular coordinates

  // We now want to compute the tangental airspeed due to the rotation of the
  // planet.
  // As z is up and tangental airspeed is positive around it, we can compute it
  // as follows:
  double local_dist_xy = sqrt(local_p_rot.x * local_p_rot.x + local_p_rot.y * local_p_rot.y);
  double xy_circ = 2.0 * M_PI * local_dist_xy; // Circumference of xy circle

  // Now we have the circumference, we can set the airspeed from the rotation
  double speed = xy_circ / rot_vel;
  // direction of air mass is the global rotated z of the planet crossed with
  // the local pos
  md3_zxz_rotmatrix(rot, (M_PI_4 + celobj->cm_orbit->r.x),
                    (M_PI_4 - celobj->cm_orbit->r.y),
                    celobj->cm_orbit->W);


  double3 planet_z = md3_v_mul(rot, vd3_set(0, 0, 1));
  double3 air_velocity = vd3_cross(planet_z, vd3_normalise(local_p));
  air_velocity = vd3_normalise(air_velocity);
  air_velocity = vd3_s_mul(air_velocity, speed);

  double3 airspeed = vd3_add(air_velocity, base); // Airspeed without object
                                                  // movement
  return vd3_sub(vd3_set(airspeed.x, airspeed.y, airspeed.z), obj->v);
}
