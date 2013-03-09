/*
  Copyright 2008,2009,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SCENEGRAPH_H_
#define SCENEGRAPH_H_

void sgAssertNoGLError(void);
void sgCheckGLError(const char *file, int line);
void sgClearGLError(void);

#define SG_CHECK_ERROR sgCheckGLError(__FILE__, __LINE__)

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "geo/geo.h"

//#include <openorbit/openorbit.h>
//#include "physics/orbit.h"
//#include "sim/simtime.h"
#include <vmath/lwcoord.h>

#include "rendering/types.h"
#include "rendering/camera.h"
#include "rendering/particles.h"
#include "rendering/material.h"
#include "rendering/light.h"
#include "rendering/sky.h"
#include "rendering/window.h"
#include "rendering/viewport.h"
#include "rendering/scene.h"
#include "rendering/overlay.h"
#include "rendering/object.h"

#endif /* SCENEGRAPH_H_ */
