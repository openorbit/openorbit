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


#include <stdio.h>
#include "physics/octtree.h"
#include "physics/object.h"
#include "physics/celestial-object.h"

#include "common/palloc.h"

#define PL_OCTTREE_MAX_OBJS 20
#define PL_OCTTREE_MAX_CEL_OBJS 5
#define PL_BHUT_THREASHOLD 0.5

static bool
pl_octtree_can_fit_rbody(const pl_octtree_t *tree, const pl_object_t *obj)
{
  double3 p = lwc_globald(&obj->p);
  double3 dist = p - tree->center;

  if (fabs(dist.x) + obj->radius > tree->width*0.5
      || fabs(dist.y) + obj->radius > tree->width*0.5
      || fabs(dist.z) + obj->radius > tree->width*0.5) {
    return false;
  }
  return true;
}


static bool
pl_octtree_can_fit_celbody(const pl_octtree_t *tree, const pl_celobject_t *obj)
{
  double3 p = obj->cm_orbit->p;
  double3 dist = p - tree->center;

  if (fabs(dist.x) + obj->cm_orbit->radius > tree->width*0.5
      || fabs(dist.y) + obj->cm_orbit->radius > tree->width*0.5
      || fabs(dist.z) + obj->cm_orbit->radius > tree->width*0.5) {
    return false;
  }
  return true;
}


pl_octtree_t*
pl_new_octtree(double3 center, double width)
{
  pl_octtree_t *otree = smalloc(sizeof(pl_octtree_t));

  otree->width = width;
  otree->center = center;

  obj_array_init(&otree->rigid_bodies);
  obj_array_init(&otree->celestial_bodies);

  return otree;
}


void
pl_octtree_delete(pl_octtree_t *tree)
{
  for (int i = 0 ; i < 8 ; i ++) {
    if (tree->children[i]) {
      pl_octtree_delete(tree->children[i]);
    }
  }

  obj_array_dispose(&tree->rigid_bodies);
  obj_array_dispose(&tree->celestial_bodies);
  free(tree);
}


void
pl_octtree_insert_rbody(pl_octtree_t *tree, pl_object_t *body)
{
  double3 body_pos = lwc_globald(&body->p);

  if (ARRAY_LEN(tree->rigid_bodies) >= PL_OCTTREE_MAX_OBJS) {
    int octant = vd3_octant(tree->center, body_pos);

    if (tree->children[octant] == NULL) {
      tree->children[octant] = pl_new_octtree(vd3_octant_split(tree->center,
                                                               tree->width,
                                                               octant),
                                              tree->width/2.0);
    }

    if (pl_octtree_can_fit_rbody(tree->children[octant], body)) {
      pl_octtree_insert_rbody(tree->children[octant], body);
    } else {
      // Does not fit in child, put in this place anyway
      obj_array_push(&tree->rigid_bodies, body);
      body->tree = tree;
    }
  } else {
    obj_array_push(&tree->rigid_bodies, body);
    body->tree = tree;
  }
}

void
pl_octtree_insert_celbody(pl_octtree_t *tree, pl_celobject_t *body)
{
  double3 body_pos = body->cm_orbit->p;

  if (ARRAY_LEN(tree->celestial_bodies) >= PL_OCTTREE_MAX_CEL_OBJS) {
    int octant = vd3_octant(tree->center, body_pos);

    if (tree->children[octant] == NULL) {
      tree->children[octant] = pl_new_octtree(vd3_octant_split(tree->center,
                                                               tree->width,
                                                               octant),
                                              tree->width/2.0);
    }

    if (pl_octtree_can_fit_celbody(tree->children[octant], body)) {
      pl_octtree_insert_celbody(tree->children[octant], body);
    } else {
      // Does not fit in child, put in this place anyway
      obj_array_push(&tree->celestial_bodies, body);
      body->tree = tree;
    }
  } else {
    obj_array_push(&tree->celestial_bodies, body);
    body->tree = tree;
  }
}

void
pl_octtree_update_gravity(pl_octtree_t *tree)
{
  double3 cog = vd3_set(0.0, 0.0, 0.0);
  double GM = 0.0;

  for (int i = 0 ; i < 8 ; i ++) {
    if (tree->children[i]) {
      pl_octtree_update_gravity(tree->children[i]);
      cog += tree->children[i]->cog * tree->children[i]->GM;
      GM += tree->children[i]->GM;
    }
  }

  ARRAY_FOR_EACH(i, tree->celestial_bodies) {
    pl_celobject_t *celobj = ARRAY_ELEM(tree->celestial_bodies, i);
    cog += celobj->cm_orbit->p * celobj->cm_orbit->GM;
    GM += celobj->cm_orbit->GM;
  }

  cog = cog / GM;
  tree->cog = cog;
  tree->GM = GM;
}

double3
pl_octtree_compute_gravity(pl_octtree_t *tree, pl_object_t *body)
{
  double3 g = vd3_set(0, 0, 0);
  double3 body_p = lwc_globald(&body->p);

  // First add the gravity for all the celestial bodies in this cell
  ARRAY_FOR_EACH(i, tree->celestial_bodies) {
    pl_celobject_t *celobj = ARRAY_ELEM(tree->celestial_bodies, i);
    double3 dv = body_p - celobj->cm_orbit->p;
    double d = vd3_abs(dv);
    g += vd3_s_mul(vd3_normalise(dv), -celobj->cm_orbit->GM * body->m.m / (d * d));
  }

  // Secondly, check each subnode and add gravity, either the aggregate for the
  // cell or we recurse down in the cell and add the gravity there. We do not
  // recurse if GM of the child is 0.
  for (int i = 0 ; i < 8 ; i ++) {
    if (tree->children[i] != NULL) {
      double3 dv = body_p - tree->children[i]->cog;
      double d = vd3_abs(dv);

      if (tree->children[i]->GM > 0.0) {
        if (tree->children[i]->width / d < PL_BHUT_THREASHOLD) {
          g += vd3_s_mul(vd3_normalise(dv),
                         -tree->children[i]->GM * body->m.m / (d * d));
        } else {
          g += pl_octtree_compute_gravity(tree->children[i], body);
        }
      }
    }
  }

  return g;
}


// When an object moves, it may have to move into another octtree node
void
pl_object_update_octtree(pl_object_t *obj)
{
  // Did not leave the current tree node, we ignore and do nothing
  if (pl_octtree_can_fit_rbody(obj->tree, obj)) return;

  pl_octtree_t *tree = obj->tree->parent;
  // Remove from current tree node

  // TODO: We probably want a list here instead of an array, on the other hand
  //       a list will conflict with parallelization of collission detection.
  ARRAY_FOR_EACH(i, tree->rigid_bodies) {
    if (ARRAY_ELEM(tree->rigid_bodies, i) == obj) {
      obj_array_remove(&tree->rigid_bodies, i);
      break;
    }
  }

  // Insert in parent where it fits
  // If an object does not fit in the root we crash at the moment
  while (!pl_octtree_can_fit_rbody(tree, obj)) {
    tree = tree->parent;
  }
  pl_octtree_insert_rbody(tree, obj);
}


// When a celestial object moves, it may have to move into another octtree node
void
pl_celobject_update_octtree(pl_celobject_t *obj)
{
  // Did not leave the current tree node, we ignore and do nothing
  if (pl_octtree_can_fit_celbody(obj->tree, obj)) return;

  pl_octtree_t *tree = obj->tree->parent;
  // Remove from current tree node

  // TODO: We probably want a list here instead of an array, on the other hand
  //       a list will conflict with parallelization of collission detection.
  ARRAY_FOR_EACH(i, tree->celestial_bodies) {
    if (ARRAY_ELEM(tree->celestial_bodies, i) == obj) {
      obj_array_remove(&tree->celestial_bodies, i);
      break;
    }
  }

  // Insert in parent where it fits
  // If an object does not fit in the root we crash at the moment
  while (!pl_octtree_can_fit_celbody(tree, obj)) {
    tree = tree->parent;
  }
  pl_octtree_insert_celbody(tree, obj);
}

