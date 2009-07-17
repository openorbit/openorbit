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

#ifndef IMAGE_H_NWIG3KEJ
#define IMAGE_H_NWIG3KEJ

#include <stdint.h>

typedef enum image_kind_t {
  IMG_RGB,
  IMG_RGBA,
  IMG_BGR,
  IMG_BGRA,
  IMG_GRAY8,
  IMG_GRAY16
} image_kind_t;

typedef struct image_t {
  image_kind_t kind;
  uint16_t w, h;
  uint8_t *data;
} image_t;

int img_load(image_t * restrict img, const char * restrict fileName);

#endif /* end of include guard: IMAGE_H_NWIG3KEJ */
