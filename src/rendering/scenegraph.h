/*
  Copyright 2008,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SCENEGRAPH_H_
#define SCENEGRAPH_H_

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "geo/geo.h"

//#include <openorbit/openorbit.h>
//#include "physics/orbit.h"
#include "sim/simtime.h"
#include "common/lwcoord.h"
#include "rendering/reftypes.h"
#include "rendering/drawable.h"

#include "camera.h"
#include "particles.h"
#include "material.h"
#include "light.h"
#include "drawable.h"

void sgSetScenePos3f(SGscene *sc, float x, float y, float z);

void sgSetScenePosLW(SGscene *sc, const OOlwcoord *lwc);

void sgDrawOverlay(SGoverlay *overlay);

SGscenegraph* sgNewSceneGraph();
void sgPaint(SGscenegraph *sg);

/*!
 * Sets the current camera in the scenegraph to cam. The camera should be in the
 * list of known cameras kept by the scenegraph, but this property is not
 * checked unless -DDEBUG is specifyed when building.
 * \param sg The scenegraph in question
 * \param cam A camera pointer to a camera known by the scenegraph
 * \pre sg != NULL
 * \pre cam != NULL
 */
void sgSetCam(SGscenegraph *sg, SGcam *cam);

/*!
 * Create a new scene. If parent is NULL, the scene is inserted in the parent's
 * scene vector. In case of null a root scene is created. Note that
 * sgNewSceneGraph creates a root scene by default so you should normally pass
 * in a valid pointer to it.
 */
SGscene* sgNewScene(SGscenegraph *sg, const char *name);
void sgSetSceneAmb4f(SGscene *sc, float r, float g, float b, float a);

void sgSetSky(SGscenegraph *sg, SGdrawable *obj);

SGcam* sgGetCam(SGscenegraph *sg);

SGscene* sgSceneGetParent(SGscene *sc);

SGscene* sgGetScene(SGscenegraph *sg, const char *sceneName);
void sgSceneAddChild(SGscene *parent, SGscene *child);
void sgSceneAddObj(SGscene *sc, SGdrawable *object);

void sgSceneAddLight(SGscene *sc, SGlight *light);

void sgAssertNoGLError(void);
void sgCheckGLError(const char *file, int line);
void sgClearGLError(void);

#define SG_CHECK_ERROR sgCheckGLError(__FILE__, __LINE__)

#endif /* SCENEGRAPH_H_ */
