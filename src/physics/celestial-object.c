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

