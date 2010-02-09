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

typedef struct PLobject PLobject;

#include <vmath/vmath.h>

#include "physics/orbit.h"
#include "physics/mass.h"
#include "common/lwcoord.h"

struct PLobject {
  PLsystem *sys;
  struct PLobject *parent;
  char *name;
  dBodyID id; // Using ODE at the moment, but this is not really necisary
  OOlwcoord p; // Large world coordinates
  PLmass m;
  SGdrawable *drawable; //!< Link to scenegraph drawable object representing this
                        //!< object.
};

PLobject* plObject3f(PLsystem *sys, float x, float y, float z);
PLobject* plSubObject3f(PLobject *obj, float x, float y, float z);
void plDetachObject(PLobject *obj);

void plSetDrawableForObject(PLobject *obj, SGdrawable *drawable);

void plForce3f(PLobject *obj, float x, float y, float z);
void plForce3d(PLobject *obj, double x, double y, double z);
void plForce3dv(PLobject *obj, double3 f);

void plForceRelative3f(PLobject *obj, float x, float y, float z);
void plForceRelative3d(PLobject *obj, double x, double y, double z);
void plForceRelative3dv(PLobject *obj, double3 f);

void plStepObjectf(PLobject *obj, float dt);
void plStepObjectd(PLobject *obj, double dt);
void plSetObjectPos3d(PLobject *obj, double x, double y, double z);
void plNormaliseObject(PLobject *obj);
void plClearObject(PLobject *obj);


#endif /* !PHYSICS_OBJECT_H */
