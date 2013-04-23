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

#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "physics/world.h"
#include "physics/mass.h"
#include "physics/reftypes.h"

#include <vmath/lwcoord.h>

struct pl_object_t {
  pl_world_t *world;
  pl_octtree_t *tree;
  struct pl_object_t *parent;
  char *name;
  pl_mass_t m;

  lwcoord_t p; // Large world coordinates
  quatd_t q; // Rotation quaternion
  double3 p_offset; // Only for use by subobjects

  double3x3 R; // Rotation wrt frame of reference
  double3x3 I_inv_world; // Inverse of inertia in world coordinates

  double3 v; // Velocity
  double3 angVel; // Angular velocity
  double3 f_ack; // Force accumulator
  double3 t_ack; // Torque accumulator

  double3 g_ack; // Gravitational force accumulator


  double radius; // For simple collission detection
  double airPressure; // In Pa
  double airDensity; // In kg / m^3
  double dragCoef; // Drag coefficent
  double area; // Average area used when calculating drag, we should actually add
              // a callback function to calculate the area
  double temperature; // Temperature of object in K

  obj_array_t psystem;// Optionally attatched particle systems
  obj_array_t children;

  double3 g; // Previous g_ack
  double3 t; // Previous t_ack
  double3 f; // Previous f_ack
};

// Create standard object
pl_object_t* pl_new_object(pl_world_t *sys, const char *name);

// Create subobject
pl_object_t* pl_new_sub_object3f(pl_world_t *world, pl_object_t *parent,
                                 const char *name,
                                 float x, float y, float z);

// Init standard object (useful if allocated with malloc or explicitly in
// structure).
void pl_object_init(pl_object_t *obj);

/*! Update and compute all derived properties */
void pl_object_compute_derived(pl_object_t *obj);

/*! Print object to console */
void pl_object_dump(pl_object_t *obj);

/*! Detatch object from parent */
void pl_object_detatch(pl_object_t *obj);

/*! Compute aggregate object mass from children */
void pl_object_update_mass(pl_object_t *obj);

/*! Assigns an SGdrawable pointer to the object. */
//void plSetDrawableForObject(pl_object_t *obj, SGdrawable *drawable);

/*! Apply force vector xyz on object */
void pl_object_force3f(pl_object_t *obj, float x, float y, float z);
/*! Apply force vector f on object */
void pl_object_force3fv(pl_object_t *obj, float3 f);

void pl_object_force_relative3f(pl_object_t *obj, float x, float y, float z);
void pl_force_relative3fv(pl_object_t *obj, float3 f);
void pl_object_force_relative_pos3f(pl_object_t *obj,
                          float fx, float fy, float fz,
                          float px, float py, float pz);
void pl_object_force_relative_pos3fv(pl_object_t *obj, float3 f, float3 p);

/*! Apply gravity vector */
void pl_object_set_gravity3f(pl_object_t *obj, float x, float y, float z);
/*! Apply gravity vector */
void pl_object_set_gravity3fv(pl_object_t *obj, float3 f);

void pl_object_torque3f(pl_object_t *obj, float tx, float ty, float tz);
void pl_object_torque3fv(pl_object_t *obj, float3 t);
void pl_object_torque_relative3f(pl_object_t *obj, float tx, float ty, float tz,
                        float px, float py, float pz);
void pl_object_torque_relative3fv(pl_object_t *obj, float3 t, float3 p);


void pl_object_set_pos3d(pl_object_t *obj, double x, double y, double z);
void pl_object_set_pos_rel3d(pl_object_t * restrict obj,
                         const pl_object_t * restrict otherObj,
                         double x, double y, double z);
void pl_object_set_pos_ext3f(pl_object_t *obj,
                         int32_t i, int32_t j, int32_t k,
                         float x, float y, float z);
void pl_object_set_pos_rel3fv(pl_object_t * restrict obj,
                          const pl_object_t * restrict otherObj,
                          float3 rp);
void pl_object_set_pos_celobj_rel(pl_object_t * restrict obj,
                                  const pl_celobject_t * restrict otherObj,
                                  float3 rp);

void pl_object_step(pl_object_t *obj, float dt);
void pl_object_step_child(pl_object_t *obj, float dt);
void pl_object_normalise(pl_object_t *obj);
/*! Clear object accumulators */
void pl_object_clear(pl_object_t *obj);

double3 pl_object_get_vel(pl_object_t *obj);
void pl_object_set_vel3f(pl_object_t *obj, float dx, float dy, float dz);
void pl_object_set_vel3fv(pl_object_t *obj, float3 dp);

double3 pl_object_get_angular_vel(pl_object_t *obj);
void pl_object_set_angular_vel3f(pl_object_t *obj, float rx, float ry, float rz);
void pl_object_set_angular_vel3fv(pl_object_t *obj, float3 r);

quatd_t pl_object_get_quat(pl_object_t *obj);

lwcoord_t pl_object_get_lwc(pl_object_t *obj);

void pl_system_add_object(pl_system_t *sys, pl_object_t *obj);

/*! Check consistency of object. DO NOT CALL DIRECTLY. */
void pl_object_check(pl_object_t *obj, const char *file, int line);

void pl_object_set_drag_coef(pl_object_t *obj, float coef);
void pl_object_set_area(pl_object_t *obj, float area);


/*! Check consistency of object. When checking, the macro will assert that
    accumulators and other properties are finite, not NaNs and so on. This macro
    is a useful debugging tool.

    In the case that a value is not finite, the program will abort and print an
    error message.
 */
#define PL_CHECK_OBJ(obj) pl_object_check(obj, __FILE__, __LINE__)

#endif /* !PHYSICS_OBJECT_H */
