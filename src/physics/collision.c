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
#include <openorbit/log.h>

#include "collision.h"
#include <vmath/lwcoord.h>
#include "physics/object.h"
#include "palloc.h"

#define THRESHOLD 10
#define TOLERANCE 0.1

pl_recgrid_t*
pl_new_recgrid(pl_collisioncontext_t *ctxt, double size)
{
  pl_recgrid_t *recgrid = pool_alloc(ctxt->pool);

  recgrid->parent = NULL;

  assert(((uintptr_t)recgrid & 3) == 0);
  recgrid->size = size;
  memset(&recgrid->centre, 0, sizeof(lwcoord_t));
  memset(recgrid->children, 0, sizeof(pl_recgrid_t *[8]));
  obj_array_init(&recgrid->objs);

  return recgrid;
}

void
pl_recgrid_delete(pl_recgrid_t *grid)
{
  obj_array_dispose(&grid->objs);
  pool_free(grid);
}

pl_collisioncontext_t*
pl_new_collision_context(void)
{
  pl_collisioncontext_t *ctxt = smalloc(sizeof(pl_collisioncontext_t));
  ctxt->pool = pool_create(sizeof(pl_recgrid_t));
  obj_array_init(&ctxt->colls);
  ctxt->otree = pl_new_recgrid(ctxt, pl_au_to_metres(100.0)); // Roughly the heliospause
  return ctxt;
}

void pl_collcontext_insert_object(pl_collisioncontext_t *ctxt, pl_recgrid_t *grid, pl_object_t *obj);

static int
getoctant(const lwcoord_t *coord, const pl_object_t *obj)
{
  float3 dist = lwc_dist(&obj->p, coord);
  int oct = signbit(dist.x) * 4 + signbit(dist.y) * 2 + signbit(dist.z);
  return oct;
}
static bool
fits(const pl_recgrid_t *grid, const pl_object_t *obj)
{
  float3 dist = lwc_dist(&obj->p, &grid->centre);

  if (fabsf(dist.x) + obj->radius > grid->size
      || fabsf(dist.y) + obj->radius > grid->size
      || fabsf(dist.z) + obj->radius > grid->size) {
    return false;
  }
  return true;
}

static void
split(pl_collisioncontext_t *ctxt, pl_recgrid_t *grid)
{
  if (grid->children[0] == NULL) {
    for (int i = 0 ; i < 8 ; i++) {
      grid->children[i] = pl_new_recgrid(ctxt, grid->size/2.0);
      grid->children[i]->centre = grid->centre;
    }
    lwc_translate3f(&grid->children[0]->centre,
                      grid->size/4.0,  grid->size/4.0,  grid->size/4.0);
    lwc_translate3f(&grid->children[1]->centre,
                      grid->size/4.0,  grid->size/4.0, -grid->size/4.0);
    lwc_translate3f(&grid->children[2]->centre,
                      grid->size/4.0, -grid->size/4.0,  grid->size/4.0);
    lwc_translate3f(&grid->children[3]->centre,
                      grid->size/4.0, -grid->size/4.0, -grid->size/4.0);
    lwc_translate3f(&grid->children[4]->centre,
                     -grid->size/4.0,  grid->size/4.0,  grid->size/4.0);
    lwc_translate3f(&grid->children[5]->centre,
                     -grid->size/4.0,  grid->size/4.0, -grid->size/4.0);
    lwc_translate3f(&grid->children[6]->centre,
                     -grid->size/4.0, -grid->size/4.0,  grid->size/4.0);
    lwc_translate3f(&grid->children[7]->centre,
                     -grid->size/4.0, -grid->size/4.0, -grid->size/4.0);
  }

  for (int i = 0 ; i < grid->objs.length ; i++) {
    pl_object_t *obj = grid->objs.elems[i];
    int octant = getoctant(&grid->centre, obj);
    if (fits(grid->children[octant], obj)) {
      pl_collcontext_insert_object(ctxt, grid->children[octant], obj);
      obj_array_remove(&grid->objs, i);
      i --;
    }
  }
}

void
pl_collcontext_insert_object(pl_collisioncontext_t *ctxt, pl_recgrid_t *grid, pl_object_t *obj)
{
  int octant = getoctant(&grid->centre, obj);
  if (grid->children[octant] && fits(grid->children[octant], obj)){
    pl_collcontext_insert_object(ctxt, grid->children[octant], obj);
  } else {
    obj_array_push(&grid->objs, obj);
  }

  if (grid->objs.length > THRESHOLD) {
    split(ctxt, grid);
  }
}

void
pl_check_collissions(pl_recgrid_t *grid)
{
  if (grid == NULL) return;
  for (int i = 0 ; i < 8 ; ++i) {
    pl_check_collissions(grid->children[i]);
  }

  for (int i = 0 ; i < grid->objs.length ; ++i) {
    pl_object_t *obj_a = grid->objs.elems[i];
    (void) obj_a; // TODO

    // Check against local objects
    for (int j = i ; j < grid->objs.length ; ++j) {
      pl_object_t *obj_b = grid->objs.elems[j];
      (void) obj_b; // TODO
    }

    // Check against parent objects
    pl_recgrid_t *higher_grid = grid->parent;
    while (higher_grid) {
      // Check against local objects
      for (int j = 0 ; j < higher_grid->objs.length ; ++j) {
        pl_object_t *obj_b = higher_grid->objs.elems[j];
        (void) obj_b; // TODO
      }

      higher_grid = higher_grid->parent;
    }
  }
}

bool
pl_collide_coarse(pl_collisioncontext_t *coll,
                  pl_object_t * restrict obj_a, pl_object_t * restrict obj_b)
{
  float3 dist = lwc_dist(&obj_a->p, &obj_b->p);

  if (vf3_abs(dist) > (obj_a->radius + obj_b->radius)) {
    return false;
  }
  log_warn("collision test succeeded %s : %s", obj_a->name, obj_b->name);

  return true;
}

bool
pl_collide_fine(pl_collisioncontext_t *coll,
              pl_object_t * restrict obj_a, pl_object_t * restrict obj_b)
{
  return true;
}

void
pl_collide_insert_object(pl_collisioncontext_t *ctxt, pl_object_t *obj)
{
  pl_collcontext_insert_object(ctxt, ctxt->otree, obj);
}

static void
pl_collide_promote_step(pl_collisioncontext_t *ctxt, pl_recgrid_t *otree)
{
  for (int i = 0 ; i < 8 ; ++ i) {
    if (otree->children[i]) pl_collide_promote_step(ctxt, otree->children[i]);
  }

  for (int i = 0 ; i < otree->objs.length ; ++i) {
    if (!fits(otree, otree->objs.elems[i])) {
      pl_collcontext_insert_object(ctxt, otree->parent, otree->objs.elems[i]);
      obj_array_remove(&otree->objs, i);
      i --;
    }
  }
}

static void
pl_collide_tree_node(pl_collisioncontext_t *coll, pl_recgrid_t *otree)
{
  if (otree == NULL) return;
  for (int i = 0 ; i < 8 ; ++ i) {
    pl_collide_tree_node(coll, otree->children[i]);
  }

  for (int i = 0 ; i < otree->objs.length ; ++i) {
    for (int j = i+1 ; j < otree->objs.length ; ++j) {
      if (pl_collide_coarse(coll, otree->objs.elems[i], otree->objs.elems[j])) {
        if (pl_collide_fine(coll, otree->objs.elems[i], otree->objs.elems[j])) {
          // TODO: Pair objects
          obj_array_push(&coll->colls, otree->objs.elems[i]);
          obj_array_push(&coll->colls, otree->objs.elems[j]);
        }
      }
    }

    // Check against parent objects
    pl_recgrid_t *higher_grid = otree->parent;
    while (higher_grid) {
      // Check against local objects
      for (int j = 0 ; j < higher_grid->objs.length ; ++j) {
        pl_object_t *obj_b = higher_grid->objs.elems[j];
        if (pl_collide_coarse(coll, otree->objs.elems[i], obj_b)) {
          if (pl_collide_fine(coll, otree->objs.elems[i], obj_b)) {
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
pl_collide_step(pl_collisioncontext_t *coll)
{
  coll->colls.length = 0; // Flush collission array
  pl_collide_promote_step(coll, coll->otree);
  pl_collide_tree_node(coll, coll->otree);

  // Resolve computed collisions
  for (int i = 0 ; i < coll->colls.length ; i += 2) {
    pl_object_t *a = coll->colls.elems[i];
    pl_object_t *b = coll->colls.elems[i+1];

    float3 av = (a->m.m - b->m.m) / (a->m.m + b->m.m) * a->v +
                (2.0f*b->m.m) / (a->m.m + b->m.m) * b->v;
    float3 bv = (b->m.m - a->m.m) / (a->m.m + b->m.m) * b->v +
                (2.0f*a->m.m) / (a->m.m + b->m.m) * a->v;


    a->v = av;
    b->v = bv;

    // Compute post colission momentums
    log_info("collission between '%s' and '%s' (%f, %f)", a->name, b->name, a->radius, b->radius);
    lwc_dump(&a->p);lwc_dump(&b->p);
  }
}

