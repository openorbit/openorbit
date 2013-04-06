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

#ifndef orbit_pl_world_h
#define orbit_pl_world_h

#include <gencds/array.h>
#include <gencds/avl-tree.h>
#include "physics/reftypes.h"
#include "physics/barneshut.h"
#include "physics/collision.h"
#include "physics/octtree.h"

struct pl_world_t {
  pl_octtree_t *octtree;
  pl_collisioncontext_t *coll_ctxt;
  obj_array_t rigid_bodies;
  obj_array_t root_bodies;
  obj_array_t celestial_objects;
  obj_array_t particle_systems;

  avl_tree_t *celestial_dict;
};

pl_world_t* pl_new_world(double size);
void pl_world_delete(pl_world_t *world);
void pl_world_step(pl_world_t *world, double jde, double dt);
void pl_world_clear(pl_world_t *world);
pl_celobject_t* pl_world_get_celobject(pl_world_t *world, const char *celobj);
void pl_world_add_celobject(pl_world_t *world, pl_celobject_t *celobj);

void pl_time_set(double jde);

#endif
