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

#define SG_MAX_LIGHTS GL_MAX_LIGHTS

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

void ooSgSetScenePos(OOscene *sc, float x, float y, float z);
void ooSgSetSceneScale(OOscene *sc, float scale);

void sgSetScenePos(OOscene *sc, const OOlwcoord *lwc);

void ooSgDrawOverlay(OOoverlay *overlay);

OOscenegraph* ooSgNewSceneGraph();
void ooSgPaint(OOscenegraph *sg);

/*!
 * Sets the current camera in the scenegraph to cam. The camera should be in the
 * list of known cameras kept by the scenegraph, but this property is not
 * checked unless -DDEBUG is specifyed when building.
 * \param sg The scenegraph in question
 * \param cam A camera pointer to a camera known by the scenegraph
 * \pre sg != NULL
 * \pre cam != NULL
 */
void ooSgSetCam(OOscenegraph *sg, OOcam *cam);

/*!
 * Create a new scene. If parent is NULL, the scene is inserted in the parent's
 * scene vector. In case of null a root scene is created. Note that
 * ooSgNewSceneGraph creates a root scene by default so you should normally pass
 * in a valid pointer to it.
 */
OOscene* ooSgNewScene(OOscene *parent, const char *name);
void sgSetSceneAmb4f(OOscene *sc, float r, float g, float b, float a);

void ooSgSetSky(OOscenegraph *sg, SGdrawable *obj);

/*!
  Finds the root scene from the a given scene. Note that in most cases you want
  to use the OOscenegraph member root for getting the scene, since member
  access is O(1) and locating the root for a given scene is O(treeHeight).
  \param sc Scene from which to find the root scene
  \result The root of sc
  \pre sc != NULL
*/
OOscene* ooSgSceneGetRoot(OOscene *sc);
void ooSgSetRoot(OOscenegraph *sg, OOscene *sc);
OOscene* ooSgGetRoot(OOscenegraph *sg);
OOcam* ooSgGetCam(OOscenegraph *sg);

OOscene* ooSgSceneGetParent(OOscene *sc);

OOscene* ooSgGetScene(OOscenegraph *sg, const char *sceneName);
void ooSgSceneAddChild(OOscene *parent, OOscene *child);
void ooSgSceneAddObj(OOscene *sc, SGdrawable *object);

void sgSceneAddLight(OOscene *sc, SGlight *light);

#endif /* SCENEGRAPH_H_ */
