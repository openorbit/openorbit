/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef __COLOUR_H__
#define __COLOUR_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*!
    @function
    @abstract   Returns RGB values for a black body colour temperature
    @discussion The function takes the temperature in the range [1000K, 40000K] and returns a pointer to the
		corresponding RGB value. As an RGB value is 24b wide compared to a pointer's 32 or 64, it is advisable
		that any caching is done on the RGB value and not the address. At the moment the granularity is 100K.
    @param      temp The black body colour temperature.
    @result		The address of the best matching RGB-value.
*/
const uint8_t *get_temp_colour(int temp);
double bv_to_temp(double bv);

#ifdef __cplusplus
}
#endif

#endif /* !__COLOUR_H__ */
