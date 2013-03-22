/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef PHYSICS__COLLISION_H
#define PHYSICS__COLLISION_H

#include <stdbool.h>
#include <gencds/array.h>
#include "physics/physics.h"
#include "physics/reftypes.h"
#include <vmath/lwcoord.h>
#include "common/palloc.h"


struct pl_recgrid_t {
  struct pl_recgrid_t *parent;
  lwcoord_t centre;
  double size;
  obj_array_t objs;
  struct pl_recgrid_t *children[8];
};

struct pl_collisioncontext_t {
  pool_t *pool;
  pl_recgrid_t *otree;
  obj_array_t colls;
};

pl_collisioncontext_t *pl_new_collision_context(double size);

bool pl_collide_coarse(pl_collisioncontext_t *coll,
                     pl_object_t * restrict obj_a, pl_object_t * restrict obj_b);

bool pl_collide_fine(pl_collisioncontext_t *coll,
                   pl_object_t * restrict obj_a, pl_object_t * restrict obj_b);

void pl_collide_insert_object(pl_collisioncontext_t *coll, pl_object_t *obj);

void pl_collide_step(pl_collisioncontext_t *coll);


#endif /* !PHYSICS__COLLISION_H */

