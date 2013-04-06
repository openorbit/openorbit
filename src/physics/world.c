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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common/palloc.h"
#include "physics/world.h"

pl_world_t*
pl_new_world(double size)
{
  pl_world_t *world = smalloc(sizeof(pl_world_t));

  world->coll_ctxt = pl_new_collision_context(size);
  world->octtree = pl_new_octtree(vd3_set(0, 0, 0), size);

  obj_array_init(&world->celestial_objects);
  obj_array_init(&world->rigid_bodies);
  obj_array_init(&world->root_bodies);
  obj_array_init(&world->particle_systems);

  world->celestial_dict = avl_str_new();

  pl_celinit(world);

  return world;
}

void
pl_world_delete(pl_world_t *world)
{
  // TODO: Delete collission context

  obj_array_dispose(&world->celestial_objects);
  obj_array_dispose(&world->rigid_bodies);
  obj_array_dispose(&world->root_bodies);
  obj_array_dispose(&world->particle_systems);

  pl_octtree_delete(world->octtree);
  avl_delete(world->celestial_dict);

  free(world);
}

void
pl_world_step(pl_world_t *world, double jde, double dt)
{
  cm_orbit_compute(jde);

  ARRAY_FOR_EACH(i, world->celestial_objects) {
    pl_celobject_update_octtree(ARRAY_ELEM(world->celestial_objects, i));
  }
  ARRAY_FOR_EACH(i, world->root_bodies) {
    pl_object_update_octtree(ARRAY_ELEM(world->root_bodies, i));
  }
  pl_octtree_update_gravity(world->octtree);

  // Compute drag and gravity for object
  ARRAY_FOR_EACH(i, world->root_bodies) {
    pl_object_t *obj = ARRAY_ELEM(world->root_bodies, i);
    double3 G = pl_octtree_compute_gravity(world->octtree, obj);
    pl_object_set_gravity3fv(obj, vf3_set(G.x, G.y, G.z));

    // TODO: drag computation
    //float3 drag = pl_object_compute_drag(obj);
    //pl_object_force3fv(obj, drag);

    pl_object_step(obj, dt);
  }

  // Do collissions
  pl_collide_step(world->coll_ctxt);
}

void
pl_world_clear(pl_world_t *world)
{
  ARRAY_FOR_EACH(i, world->rigid_bodies) {
    pl_object_clear(ARRAY_ELEM(world->rigid_bodies, i));
  }
}


pl_celobject_t*
pl_world_get_celobject(pl_world_t *world, const char *celobj)
{
  char celobj_lower[strlen(celobj)+1];
  for (int i = 0 ; i < strlen(celobj) ; i ++) {
    celobj_lower[i] = tolower(celobj[i]);
  }
  celobj_lower[strlen(celobj)] = '\0';

  return avl_find(world->celestial_dict, celobj_lower);
}

void
pl_world_add_celobject(pl_world_t *world, pl_celobject_t *celobj)
{
  avl_insert(world->celestial_dict, celobj->cm_orbit->name, celobj);
}

void
pl_time_set(double jde)
{
  cm_orbit_compute(jde);
}
