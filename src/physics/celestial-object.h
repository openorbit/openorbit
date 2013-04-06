//
//  celestial-object.h
//  orbit
//
//  Created by Mattias Holm on 2013-03-22.
//
//

#ifndef orbit_celestial_object_h
#define orbit_celestial_object_h

#include <celmek/celmek.h>
#include "physics/reftypes.h"
#include "physics/world.h"

struct pl_celobject_t {
  pl_octtree_t *tree;
  cm_orbit_t *cm_orbit;
};

void pl_celinit(pl_world_t *world);
pl_celobject_t* pl_new_celobject(pl_world_t *world, cm_orbit_t *cm_orbit);

quaternion_t pl_celobject_get_body_quat(pl_celobject_t *celobj);
quaternion_t pl_celobject_get_orbit_quat(pl_celobject_t *celobj);
float3 pl_celobject_get_vel(pl_celobject_t *celobj);
lwcoord_t pl_celobject_get_lwc(pl_celobject_t *celobj);

#endif
