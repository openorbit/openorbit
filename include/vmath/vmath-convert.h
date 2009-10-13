/*
  Copyright 2007 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef MATH_CONVERT_H__
#define MATH_CONVERT_H__

#include <vmath/vmath-constants.h>
#include <vmath/vmath-integer.h>
#include <vmath/vmath-matvec.h>

#define DEG_TO_RAD(d) ((d) * S_PI/180.0f)
#define RAD_TO_DEG(r) ((r) * 180.0f/S_PI)

static inline float3
v3i_to_v3f(int3 iv)
{
  float3 res = vf3_set((float)v3i_get(iv, 0),
                       (float)v3i_get(iv, 1),
                       (float)v3i_get(iv, 2));
  return res;
}


static inline double3
v3l_to_v3d(long3 lv)
{
  double3 res = vd3_set((double)v3l_get(lv, 0),
                        (double)v3l_get(lv, 1),
                        (double)v3l_get(lv, 2));
  return res;
}


#endif /* ! MATH_CONVERT_H__ */