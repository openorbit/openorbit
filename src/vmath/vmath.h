/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*
    This header is the master include file for the vectorised math routines.
	The main layout of the headers and the files are the following:
		vmath/vmath-xxx.c: scalar function and implementations
		vmath/arch/xxx/: Overrided inline versions (architecture speciffic, may use
						 intrinsics for the CPU).
		vmath/vmath-xxx.h: headers with function declarations
		vmath/vmath-xxx.inl: Generic vectorised and inline header (gcc specific,
							 but arch independent)
*/

#ifndef VMATH_H__
#define VMATH_H__

#include <vmath/vmath-constants.h>
#include <vmath/vmath-types.h>
#include <vmath/vmath-matvec.h>
#include <vmath/vmath-quaternions.h>
#include <vmath/vmath-convert.h>


#endif /* ! VMATH_H__ */
