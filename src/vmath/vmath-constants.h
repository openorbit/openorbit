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



#ifndef __MATH_CONSTANTS_H__
#define __MATH_CONSTANTS_H__

#ifdef __cplusplus
extern "C" {
#endif 

#include <vmath/vmath-types.h>

#define S_CONST(a) (a ## f)
    

#define S_PI            S_CONST(3.14159265)
#define S_PI_HALF       S_CONST(1.57079633)
#define S_ZERO          S_CONST(0.0)
#define S_POINT_FIVE    S_CONST(0.5)
#define S_ONE           S_CONST(1.0)
#define S_TWO           S_CONST(2.0)


#ifdef __cplusplus
}
#endif 

#endif /* !__MATH_CONSTANTS_H__ */
