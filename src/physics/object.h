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

#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "physics/mass.h"
#include "physics/reftypes.h"

#include "common/lwcoord.h"

struct PLobject {
  PLsystem *sys;
  PLcompound_object *parent;
  char *name;
  PLmass m;
  SGdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
  
  OOlwcoord p; // Large world coordinates
  quaternion_t q; // Rotation quaternion
  float3 p_offset; // Only for use by subobjects

  float3x3 R; // Rotation wrt frame of reference
  float3x3 I_inv_world; // Inverse of inertia in world coordinates

  float3 v; // Velocity
  float3 angVel; // Angular velocity
  float3 f_ack; // Force accumulator
  float3 t_ack; // Torque accumulator
};

struct PLcompound_object {
  PLobject super;
  obj_array_t children;
};

// Create standard object
PLobject* plObject3f(PLworld *sys);
// Create standard object
PLcompound_object* plCompoundObject(PLworld *sys);
// Create subobject
PLobject* plSubObject3f(PLworld *world, PLcompound_object *parent,
                        float x, float y, float z);

// Init standard object (useful if allocated with malloc or explicitly in structure)
void plInitObject(PLobject *obj);
void plComputeDerived(PLobject *obj);


void plDumpObject(PLobject *obj);

void plDetachObject(PLobject *obj);
void plUpdateMass(PLcompound_object *obj);

void plSetDrawableForObject(PLobject *obj, SGdrawable *drawable);

void plForce3f(PLobject *obj, float x, float y, float z);
void plForce3fv(PLobject *obj, float3 f);

void plForceRelative3f(PLobject *obj, float x, float y, float z);
void plForceRelative3fv(PLobject *obj, float3 f);
void plForceRelativePos3f(PLobject *obj,
                          float fx, float fy, float fz,
                          float px, float py, float pz);
void plForceRelativePos3fv(PLobject *obj, float3 f, float3 p);


void plSetObjectPos3d(PLobject *obj, double x, double y, double z);
void plSetObjectPosRel3d(PLobject * restrict obj,
                         const PLobject * restrict otherObj,
                         double x, double y, double z);
void plSetObjectPosExt3f(PLobject *obj,
                         int32_t i, int32_t j, int32_t k,
                         float x, float y, float z);
void plSetObjectPosRel3fv(PLobject * restrict obj,
                          const PLobject * restrict otherObj,
                          float3 rp);

void plStepObjectf(PLobject *obj, float dt);
void plStepChildObjectf(PLobject *obj, float dt);
void plNormaliseObject(PLobject *obj);
void plClearObject(PLobject *obj);

void plSetVel3f(PLobject *obj, float dx, float dy, float dz);
void plSetVel3fv(PLobject *obj, float3 dp);

void plSetAngularVel3f(PLobject *obj, float rx, float ry, float rz);
void plSetAngularVel3fv(PLobject *obj, float3 r);

quaternion_t plGetQuat(PLobject *obj);


void plSetSystem(PLsystem *sys, PLobject *obj);

#endif /* !PHYSICS_OBJECT_H */
