/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef __SKY_H__
#define __SKY_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <vmath/vmath.h>

#include "rendering/types.h"

#include "scenegraph.h"
#include "shader-manager.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#define STAR_CNT 5000

/*!
    @function   equ_cart_convert
    @abstract   Converts equatorial coordinates to cartesian unit coordinates
    @discussion The function converts an equatorial coordinates given in radians to cartesian unit coordinates that
        lies on the unit sphere
    @param      cart The resulting cartesian vector.
    @param      ra Right ascension in radians
    @param      dec Declination in radians
*/
float3 ooEquToCart(float ra, float dec);

void sgDrawBackground(sg_background_t *bg);
sg_background_t* sgCreateBackgroundFromFile(const char *file);

#ifdef __cplusplus
}
#endif

#endif
