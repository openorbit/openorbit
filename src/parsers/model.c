/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "model.h"
#include <string.h>
#include <assert.h>
#include "ac3d.h"
#include "collada.h"
#include <gencds/array.h>

model_object_t*
model_object_new(void)
{
  model_object_t *model = malloc(sizeof(model_object_t));
  memset(model, 0, sizeof(model_object_t));

  model->rot[0][0] = 1.0;
  model->rot[1][1] = 1.0;
  model->rot[2][2] = 1.0;
  model->rot[3][3] = 1.0;

  obj_array_init(&model->children);
  float_array_init(&model->vertices);
  float_array_init(&model->texCoords);
  float_array_init(&model->normals);
  float_array_init(&model->colours);

  return model;
}

material_t*
material_create(void)
{
  // Initialise the default material to what opengl has as default
  material_t *m = malloc(sizeof(material_t));

  m->ambient[0] = 0.2;
  m->ambient[1] = 0.2;
  m->ambient[2] = 0.2;
  m->ambient[3] = 1.0;

  m->diffuse[0] = 0.8;
  m->diffuse[1] = 0.8;
  m->diffuse[2] = 0.8;
  m->diffuse[3] = 1.0;

  m->specular[0] = 0.0;
  m->specular[1] = 0.0;
  m->specular[2] = 0.0;
  m->specular[3] = 1.0;

  m->emission[0] = 0.0;
  m->emission[1] = 0.0;
  m->emission[2] = 0.0;
  m->emission[3] = 1.0;

  m->shininess = 0.0;


  return m;
}
model_t*
model_load(const char * restrict fileName)
{
  assert(fileName && "not null");

  model_t *model = NULL;

  char *dot = strrchr(fileName, '.');
  if (dot) {
    char *endptr = strchr(dot, '\0');
    assert(endptr != NULL);

    // Dispatch based on file type
    if (!strcmp(dot, ".hrml")) {

    } else if (!strcmp(dot, ".3ds")) {

    } else if (!strcmp(dot, ".dae")) {
      model = collada_load(fileName);
    } else if (!strcmp(dot, ".ac")) {
      model = ac3d_load(fileName);
    }
  }
  return model;
}

int
model_dispose(model_t * model)
{
  return -1;
}
