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

#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "log.h"

#include "collision.h"
#include "common/lwcoord.h"
#include "physics/object.h"

#define THRESHOLD 10
#define TOLERANCE 0.1

PLrecgrid*
plNewRecgrid(PLcollisioncontext *ctxt, double size)
{
  PLrecgrid *recgrid = pool_alloc(ctxt->pool);

  recgrid->parent = NULL;

  assert(((uintptr_t)recgrid & 3) == 0);
  recgrid->size = size;
  memset(&recgrid->centre, 0, sizeof(OOlwcoord));
  memset(recgrid->children, 0, sizeof(PLrecgrid *[8]));
  obj_array_init(&recgrid->objs);

  return recgrid;
}

void
plDeleteRecgrid(PLrecgrid *grid)
{
  obj_array_dispose(&grid->objs);
  pool_free(grid);
}

PLcollisioncontext*
plNewCollisionContext(void)
{
  PLcollisioncontext *ctxt = malloc(sizeof(PLcollisioncontext));
  ctxt->pool = pool_create(sizeof(PLrecgrid));
  obj_array_init(&ctxt->colls);
  ctxt->otree = plNewRecgrid(ctxt, plAuToMetres(100.0)); // Roughly the heliospause
  return ctxt;
}

void plInsertObject(PLcollisioncontext *ctxt, PLrecgrid *grid, PLobject *obj);

static int
getoctant(const OOlwcoord *coord, const PLobject *obj)
{
  float3 dist = ooLwcDist(&obj->p, coord);
  int oct = signbit(dist.x) * 4 + signbit(dist.y) * 2 + signbit(dist.z);
  return oct;
}
static bool
fits(const PLrecgrid *grid, const PLobject *obj)
{
  float3 dist = ooLwcDist(&obj->p, &grid->centre);

  if (fabsf(dist.x) + obj->radius > grid->size
      || fabsf(dist.y) + obj->radius > grid->size
      || fabsf(dist.z) + obj->radius > grid->size) {
    return false;
  }
  return true;
}

static void
split(PLcollisioncontext *ctxt, PLrecgrid *grid)
{
  if (grid->children[0] == NULL) {
    for (int i = 0 ; i < 8 ; i++) {
      grid->children[i] = plNewRecgrid(ctxt, grid->size/2.0);
      grid->children[i]->centre = grid->centre;
    }
    ooLwcTranslate3f(&grid->children[0]->centre,
                      grid->size/4.0,  grid->size/4.0,  grid->size/4.0);
    ooLwcTranslate3f(&grid->children[1]->centre,
                      grid->size/4.0,  grid->size/4.0, -grid->size/4.0);
    ooLwcTranslate3f(&grid->children[2]->centre,
                      grid->size/4.0, -grid->size/4.0,  grid->size/4.0);
    ooLwcTranslate3f(&grid->children[3]->centre,
                      grid->size/4.0, -grid->size/4.0, -grid->size/4.0);
    ooLwcTranslate3f(&grid->children[4]->centre,
                     -grid->size/4.0,  grid->size/4.0,  grid->size/4.0);
    ooLwcTranslate3f(&grid->children[5]->centre,
                     -grid->size/4.0,  grid->size/4.0, -grid->size/4.0);
    ooLwcTranslate3f(&grid->children[6]->centre,
                     -grid->size/4.0, -grid->size/4.0,  grid->size/4.0);
    ooLwcTranslate3f(&grid->children[7]->centre,
                     -grid->size/4.0, -grid->size/4.0, -grid->size/4.0);
  }

  for (int i = 0 ; i < grid->objs.length ; i++) {
    PLobject *obj = grid->objs.elems[i];
    int octant = getoctant(&grid->centre, obj);
    if (fits(grid->children[octant], obj)) {
      plInsertObject(ctxt, grid->children[octant], obj);
      obj_array_remove(&grid->objs, i);
      i --;
    }
  }
}

void
plInsertObject(PLcollisioncontext *ctxt, PLrecgrid *grid, PLobject *obj)
{
  int octant = getoctant(&grid->centre, obj);
  if (grid->children[octant] && fits(grid->children[octant], obj)){
    plInsertObject(ctxt, grid->children[octant], obj);
  } else {
    obj_array_push(&grid->objs, obj);
  }

  if (grid->objs.length > THRESHOLD) {
    split(ctxt, grid);
  }
}

void
plCheckCollissions(PLrecgrid *grid)
{
  if (grid == NULL) return;
  for (int i = 0 ; i < 8 ; ++i) {
    plCheckCollissions(grid->children[i]);
  }

  for (int i = 0 ; i < grid->objs.length ; ++i) {
    PLobject *obj_a = grid->objs.elems[i];

    // Check against local objects
    for (int j = i ; j < grid->objs.length ; ++j) {
      PLobject *obj_b = grid->objs.elems[j];

    }

    // Check against parent objects
    PLrecgrid *higher_grid = grid->parent;
    while (higher_grid) {
      // Check against local objects
      for (int j = 0 ; j < higher_grid->objs.length ; ++j) {
        PLobject *obj_b = higher_grid->objs.elems[j];

      }

      higher_grid = higher_grid->parent;
    }
  }
}

bool
plCollideCoarse(PLcollisioncontext *coll,
                PLobject * restrict obj_a, PLobject * restrict obj_b)
{
  float3 dist = ooLwcDist(&obj_a->p, &obj_b->p);

  if (vf3_abs(dist) > (obj_a->radius + obj_b->radius)) {
    return false;
  }
  ooLogWarn("collision test succeeded %s : %s", obj_a->name, obj_b->name);

  return true;
}

bool
plCollideFine(PLcollisioncontext *coll,
              PLobject * restrict obj_a, PLobject * restrict obj_b)
{
  return true;
}

void
plCollideInsertObject(PLcollisioncontext *ctxt, PLobject *obj)
{
  plInsertObject(ctxt, ctxt->otree, obj);
}

static void
plCollidePromoteStep(PLcollisioncontext *ctxt, PLrecgrid *otree)
{
  for (int i = 0 ; i < 8 ; ++ i) {
    if (otree->children[i]) plCollidePromoteStep(ctxt, otree->children[i]);
  }

  for (int i = 0 ; i < otree->objs.length ; ++i) {
    if (!fits(otree, otree->objs.elems[i])) {
      plInsertObject(ctxt, otree->parent, otree->objs.elems[i]);
      obj_array_remove(&otree->objs, i);
      i --;
    }
  }
}

static void
plCollideTreeNode(PLcollisioncontext *coll, PLrecgrid *otree)
{
  if (otree == NULL) return;
  for (int i = 0 ; i < 8 ; ++ i) {
    plCollideTreeNode(coll, otree->children[i]);
  }

  for (int i = 0 ; i < otree->objs.length ; ++i) {
    for (int j = i+1 ; j < otree->objs.length ; ++j) {
      if (plCollideCoarse(coll, otree->objs.elems[i], otree->objs.elems[j])) {
        if (plCollideFine(coll, otree->objs.elems[i], otree->objs.elems[j])) {
          // TODO: Pair objects
          obj_array_push(&coll->colls, otree->objs.elems[i]);
          obj_array_push(&coll->colls, otree->objs.elems[j]);
        }
      }
    }

    // Check against parent objects
    PLrecgrid *higher_grid = otree->parent;
    while (higher_grid) {
      // Check against local objects
      for (int j = 0 ; j < higher_grid->objs.length ; ++j) {
        PLobject *obj_b = higher_grid->objs.elems[j];
        if (plCollideCoarse(coll, otree->objs.elems[i], obj_b)) {
          if (plCollideFine(coll, otree->objs.elems[i], obj_b)) {
            // TODO: Pair objects
            obj_array_push(&coll->colls, otree->objs.elems[i]);
            obj_array_push(&coll->colls, obj_b);
          }
        }
      }

      higher_grid = higher_grid->parent;
    }
  }
}

void
plCollideStep(PLcollisioncontext *coll)
{
  coll->colls.length = 0; // Flush collission array
  plCollidePromoteStep(coll, coll->otree);
  plCollideTreeNode(coll, coll->otree);

  // Resolve computed collisions
  for (int i = 0 ; i < coll->colls.length ; i += 2) {
    PLobject *a = coll->colls.elems[i];
    PLobject *b = coll->colls.elems[i+1];

    float3 av = (a->m.m - b->m.m) / (a->m.m + b->m.m) * a->v +
                (2.0f*b->m.m) / (a->m.m + b->m.m) * b->v;
    float3 bv = (b->m.m - a->m.m) / (a->m.m + b->m.m) * b->v +
                (2.0f*a->m.m) / (a->m.m + b->m.m) * a->v;


    a->v = av;
    b->v = bv;

    // Compute post colission momentums
    ooLogInfo("collission between '%s' and '%s' (%f, %f)", a->name, b->name, a->radius, b->radius);
    ooLwcDump(&a->p);ooLwcDump(&b->p);
  }
}

