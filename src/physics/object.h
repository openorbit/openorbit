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

#include "physics/mass.h"
#include "physics/reftypes.h"

#include <vmath/lwcoord.h>

struct PLobject {
  pl_system_t *sys;
  struct PLobject *parent;
  char *name;
  pl_mass_t m;
  //SGdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.

  lwcoord_t p; // Large world coordinates
  quaternion_t q; // Rotation quaternion
  float3 p_offset; // Only for use by subobjects

  float3x3 R; // Rotation wrt frame of reference
  float3x3 I_inv_world; // Inverse of inertia in world coordinates

  float3 v; // Velocity
  float3 angVel; // Angular velocity
  float3 f_ack; // Force accumulator
  float3 t_ack; // Torque accumulator

  float3 g_ack; // Gravitational force accumulator


  float radius; // For simple collission detection
  float airPressure; // In Pa
  float airDensity; // In kg / m^3
  float dragCoef; // Drag coefficent
  float area; // Average area used when calculating drag, we should actually add
              // a callback function to calculate the area
  float temperature; // Temperature of object in K

  obj_array_t psystem;// Optionally attatched particle systems
  obj_array_t children;
};

// Create standard object
PLobject* pl_new_object(pl_world_t *sys, const char *name);

// Create subobject
PLobject* pl_new_sub_object3f(pl_world_t *world, PLobject *parent, const char *name,
                        float x, float y, float z);

// Init standard object (useful if allocated with malloc or explicitly in
// structure).
void pl_object_init(PLobject *obj);

/*! Update and compute all derived properties */
void pl_object_compute_derived(PLobject *obj);

/*! Print object to console */
void pl_object_dump(PLobject *obj);

/*! Detatch object from parent */
void pl_object_detatch(PLobject *obj);

/*! Compute aggregate object mass from children */
void pl_object_update_mass(PLobject *obj);

/*! Assigns an SGdrawable pointer to the object. */
//void plSetDrawableForObject(PLobject *obj, SGdrawable *drawable);

/*! Apply force vector xyz on object */
void pl_object_force3f(PLobject *obj, float x, float y, float z);
/*! Apply force vector f on object */
void pl_object_force3fv(PLobject *obj, float3 f);

void pl_object_force_relative3f(PLobject *obj, float x, float y, float z);
void pl_force_relative3fv(PLobject *obj, float3 f);
void pl_object_force_relative_pos3f(PLobject *obj,
                          float fx, float fy, float fz,
                          float px, float py, float pz);
void pl_object_force_relative_pos3fv(PLobject *obj, float3 f, float3 p);

/*! Apply gravity vector */
void pl_object_set_gravity3f(PLobject *obj, float x, float y, float z);
/*! Apply gravity vector */
void pl_object_set_gravity3fv(PLobject *obj, float3 f);

void pl_object_torque3f(PLobject *obj, float tx, float ty, float tz);
void pl_object_torque3fv(PLobject *obj, float3 t);
void pl_object_torque_relative3f(PLobject *obj, float tx, float ty, float tz,
                        float px, float py, float pz);
void pl_object_torque_relative3fv(PLobject *obj, float3 t, float3 p);


void pl_object_set_pos3d(PLobject *obj, double x, double y, double z);
void pl_object_set_pos_rel3d(PLobject * restrict obj,
                         const PLobject * restrict otherObj,
                         double x, double y, double z);
void pl_object_set_pos_ext3f(PLobject *obj,
                         int32_t i, int32_t j, int32_t k,
                         float x, float y, float z);
void pl_object_set_pos_rel3fv(PLobject * restrict obj,
                          const PLobject * restrict otherObj,
                          float3 rp);

void pl_object_step(PLobject *obj, float dt);
void pl_object_step_child(PLobject *obj, float dt);
void pl_object_normalise(PLobject *obj);
/*! Clear object accumulators */
void pl_object_clear(PLobject *obj);

float3 pl_object_get_vel(PLobject *obj);
void pl_object_set_vel3f(PLobject *obj, float dx, float dy, float dz);
void pl_object_set_vel3fv(PLobject *obj, float3 dp);

float3 pl_object_get_angular_vel(PLobject *obj);
void pl_object_set_angular_vel3f(PLobject *obj, float rx, float ry, float rz);
void pl_object_set_angular_vel3fv(PLobject *obj, float3 r);

quaternion_t pl_object_get_quat(PLobject *obj);

lwcoord_t pl_object_get_lwc(PLobject *obj);

void pl_system_add_object(pl_system_t *sys, PLobject *obj);

/*! Check consistency of object. DO NOT CALL DIRECTLY. */
void pl_object_check(PLobject *obj, const char *file, int line);

void pl_object_set_drag_coef(PLobject *obj, float coef);
void pl_object_set_area(PLobject *obj, float area);


/*! Check consistency of object. When checking, the macro will assert that
    accumulators and other properties are finite, not NaNs and so on. This macro
    is a useful debugging tool.

    In the case that a value is not finite, the program will abort and print an
    error message.
 */
#define PL_CHECK_OBJ(obj) pl_object_check(obj, __FILE__, __LINE__)

#endif /* !PHYSICS_OBJECT_H */
