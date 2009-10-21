/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef __SKY_H__
#define __SKY_H__
#ifdef __cplusplus
extern "C" {
#endif 

#include <vmath/vmath.h>

#include "scenegraph.h"

#include "SDL_opengl.h"

#define STAR_CNT 5000
    
typedef struct {
    unsigned char r, g, b, a;
    float x, y, z;
} OOstar;

typedef struct {
  OOdrawable super;
  size_t n_stars; //!< Number of stars loaded
  size_t a_len; //!< Length of data
  OOstar *data;
} OOstars;


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
OOstars* ooSkyInitStars(int starCount);
OOstars *ooSkyRandomStars(void);
    
void ooSkyAddStar(OOstars *stars, double ra, double dec, double mag, double bv);
    
void ooSkyDrawStars(OOstars *stars);
OOstars* ooSkyLoadStars(const char *file);

OOdrawable *ooSkyNewDrawable(const char *file);


#ifdef __cplusplus
}
#endif 

#endif
