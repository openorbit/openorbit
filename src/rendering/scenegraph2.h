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
#include "rendering/material.h"
#include "rendering/light.h"
#include "rendering/scenegraph.h"


struct SGgeometry {
  SGobject *obj;
  int vertexCount;

  GLuint vba;
  GLuint vbo;

  bool hasNormals;
  bool hasTexCoords;
  GLsizei normalOffset;
  GLsizei texCoordOffset;
};

struct SGobject {
  SGobject *parent;
  SGscene2 *scene;
  float3 pos;
  float3 dp; // delta pos per time step
  float3 dr; // delta rot per time step
  quaternion_t q; // Quaternion

  float4x4 R;
  float4x4 modelViewMatrix;

  size_t lightCount;
  SGlight *lights[SG_OBJ_MAX_LIGHTS];

  size_t texCount;
  GLuint textures[SG_OBJ_MAX_TEXTURES];

  SGmaterial material;

  SGshader *shader;
  SGgeometry *geometry;
  obj_array_t subObjects;
};


SGobject* sgCreateObject(SGscene2 *scene);
SGobject* sgCreateSubObject(SGobject *parent);

void sgObjectSetPos(SGobject *obj, float4 pos);
void sgObjectLoadShader(SGobject *obj, const char *name);

SGgeometry*
sgCreateGeometry(SGobject *obj, size_t vertexCount,
                 float *vertices, float *normals, float *texCoords);
void sgAnimateObject(SGobject *obj, float dt);
void sgRecomputeModelViewMatrix(SGobject *obj);
void sgDrawObject(SGobject *obj);
void sgDrawGeometry(SGgeometry *geo);


#endif
