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


#ifndef orbit_octtree_h
#define orbit_octtree_h

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "physics/reftypes.h"
#include "physics/object.h"
#include "physics/celestial-object.h"

// There are two things to keep in mind here. Firstly, the octtrees are used
// for two things. One is for spatial partitioning of objects. This helps with
// collission detection and culling. In this case, bodies may be inserted
// in internal nodes.
//
// The second usecase is n-body simulation. In this case celestial bodies are
// inserted in the leafs or the external nodes.
//
// Can we combine these into one octtree to rule them all?

struct pl_octtree_t {
  double width;
  double3 center;
  double3 cog;
  double GM;

  obj_array_t celestial_bodies;
  obj_array_t rigid_bodies;

  struct pl_octtree_t *parent;
  struct pl_octtree_t *children[8];
};

pl_octtree_t* pl_new_octtree(double3 center, double width);
void pl_octtree_delete(pl_octtree_t *tree);

void pl_octtree_update_gravity(pl_octtree_t *tree);

double3 pl_octtree_compute_gravity(pl_octtree_t *tree, pl_object_t *body);

void pl_octtree_insert_rbody(pl_octtree_t *tree, pl_object_t *body);
void pl_octtree_insert_celbody(pl_octtree_t *tree, pl_celobject_t *body);



void pl_object_update_octtree(pl_object_t *obj);
void pl_celobject_update_octtree(pl_celobject_t *obj);

#endif
