/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef orbit_scenegraph2_h
#define orbit_scenegraph2_h

#include <OpenGL/gl3.h>
#include <gencds/array.h>
#include <vmath/vmath.h>
#include "rendering/types.h"
#include "rendering/material.h"
#include "rendering/light.h"
#include "rendering/scenegraph.h"
#include "physics/reftypes.h"



sg_object_t* sgCreateObject(sg_scene_t *scene);
sg_object_t* sgCreateSubObject(sg_object_t *parent);

void sgObjectSetPos(sg_object_t *obj, float4 pos);
void sgObjectLoadShader(sg_object_t *obj, const char *name);

sg_geometry_t*
sg_geometry_create(sg_object_t *obj, size_t vertexCount,
                   float *vertices, float *normals, float *texCoords);

sg_object_t* sg_create_ellipse(const char *name, float semiMajor,
                                      float semiMinor, float asc,
                                      float inc, float argOfPeriapsis,
                                      float dec, float ra, int segments);

sg_object_t* sg_create_sphere(const char *name, GLuint shader, float radius,
                              GLuint tex, GLuint nightTex, float spec,
                              sg_material_t *mat);
void sgAnimateObject(sg_object_t *obj, float dt); // Linear interpolation between frames
void sgUpdateObject(sg_object_t *obj); // Pull from physis system

void sgRecomputeModelViewMatrix(sg_object_t *obj);
void sgDrawObject(sg_object_t *obj);
void sgDrawGeometry(sg_geometry_t *geo);
void sg_object_set_rigid_body(sg_object_t *obj, PLobject *rigidBody);
sg_object_t* sg_load_object(const char *file);

#endif
