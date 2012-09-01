/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <stdlib.h>
#include <gencds/array.h>

#include "rendering/material.h"

typedef struct model_t model_t;
typedef sg_material_t material_t;
typedef struct model_object_t model_object_t;

struct model_object_t {
  model_t *model;
  int materialId;
  float trans[3];
  float rot[4][4];

  char *texture;
  obj_array_t children;

  uint32_t vertexCount;
  float_array_t vertices;
  float_array_t texCoords;
  float_array_t normals;
  float_array_t colours;
};

struct model_t {
  obj_array_t objs;
  size_t materialCount;
  material_t **materials;
};


model_t * model_load(const char * restrict fileName);
int model_dispose(model_t * restrict model);
material_t* material_create(void);
model_object_t* model_object_new(void);

#endif /* end of include guard: MODEL_H */

