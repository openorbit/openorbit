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
#ifndef PNG_H_DU136IQO
#define PNG_H_DU136IQO
#include <stdint.h>
#include <stdio.h>


typedef enum {
  PNG_RGB,
  PNG_RGBA
} png_kind_t;
typedef struct {
  png_kind_t kind;
  uint32_t w, h;
  uint8_t *data;  
} png_image_t;

int png_read_file(png_image_t *img, FILE *file);

#endif /* end of include guard: PNG_H_DU136IQO */
