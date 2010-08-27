/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef PHYSICS__COLLISION_H
#define PHYSICS__COLLISION_H

#include <stdbool.h>
#include <gencds/array.h>
#include "physics/physics.h"
#include "physics/reftypes.h"
#include "common/lwcoord.h"
#include "common/palloc.h"


struct PLrecgrid {
  struct PLrecgrid *parent;
  OOlwcoord centre;
  double size;
  obj_array_t objs;
  struct PLrecgrid *children[8];
};

struct PLcollisioncontext {
  pool_t *pool;
  PLrecgrid *otree;
  obj_array_t colls;
};

PLcollisioncontext *plNewCollisionContext(void);

bool plCollideCoarse(PLcollisioncontext *coll,
                     PLobject * restrict obj_a, PLobject * restrict obj_b);

bool plCollideFine(PLcollisioncontext *coll,
                   PLobject * restrict obj_a, PLobject * restrict obj_b);

void plCollideInsertObject(PLcollisioncontext *coll, PLobject *obj);

void plCollideStep(PLcollisioncontext *coll);


#endif /* !PHYSICS__COLLISION_H */

