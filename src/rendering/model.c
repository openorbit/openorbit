/*
  Copyright 2009,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "model.h"
#include <string.h>
#include <assert.h>
#include "ac3d.h"
#include "collada.h"
#include <gencds/array.h>

#include "rendering/object.h"
#include "rendering/material.h"
#include "rendering/types.h"

sg_object_t*
model_load(const char * restrict fileName)
{
  assert(fileName && "not null");

  sg_object_t *model = NULL;

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
