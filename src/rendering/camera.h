/*
  Copyright 2006,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef __CAMERA_H__
#define __CAMERA_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <vmath/vmath.h>
#include <vmath/lwcoord.h>
#include "rendering/types.h"


  void sgAnimateCam(sg_camera_t *cam, float dt);
  void sgMoveCam(sg_camera_t *cam);

#ifdef __cplusplus
}
#endif

#endif
