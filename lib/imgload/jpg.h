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

#ifndef JPG_H_8G9PU4E5
#define JPG_H_8G9PU4E5

#include <stdint.h>

typedef enum jpg_kind_t {
  JPG_RGB,
  JPG_GRAY
} jpg_kind_t;

typedef struct jpg_image_t {
  jpg_kind_t kind;
  uint32_t w, h;
  uint8_t *data;
} jpg_image_t;

int jpeg_load(jpg_image_t * restrict img, const char * restrict filename);


#endif /* end of include guard: JPG_H_8G9PU4E5 */

