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

#include <ode/ode.h>

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "geo/geo.h"

//#include <openorbit/openorbit.h>
//#include "physics/orbit.h"
#include "sim/simtime.h"
#include "common/lwcoord.h"

#define SG_MAX_LIGHTS GL_MAX_LIGHTS

typedef struct SGdrawable SGdrawable;
typedef struct OOscene OOscene;
typedef struct OOoverlay OOoverlay;
typedef struct OOscenegraph OOscenegraph;
typedef struct SGmaterial SGmaterial;

typedef struct SGlight SGlight;
typedef struct SGspotlight SGspotlight;
typedef struct SGpointlight SGpointlight;

typedef struct OOsphere OOsphere;
typedef struct SGcylinder SGcylinder;

struct SGmaterial {
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float emission[4];
  float shininess;
};


#include "camera.h"

typedef void (*SGdrawfunc)(SGdrawable*);

struct SGdrawable {
  const char *name;
  OOscene *scene;
  bool enabled;
  float3 p;  // Position
  float3 dp; // delta pos per time step
  float3 dr; // delta rot per time step
  quaternion_t q; // Quaternion
                  //float s; // Scale

  SGdrawfunc draw; // Draw function for this object
};

#include "particles.h"


SGdrawable* ooSgNewDrawable(SGdrawable *drawable, const char *name, SGdrawfunc df);

void ooSgSetObjectQuat(SGdrawable *obj, float x, float y, float z, float w);
void sgSetObjectQuatv(SGdrawable *obj, quaternion_t q);

void ooSgSetObjectPosLW(SGdrawable *obj, const OOlwcoord *lw);
void ooSgSetObjectPos(SGdrawable *obj, float x, float y, float z);
//void ooSgSetObjectScale(OOdrawable *obj, float s);
void ooSgSetObjectSpeed(SGdrawable *obj, float dx, float dy, float dz);
void ooSgSetObjectAngularSpeed(SGdrawable *obj, float drx, float dry, float drz);

void ooSgSetScenePos(OOscene *sc, float x, float y, float z);
void ooSgSetSceneScale(OOscene *sc, float scale);

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

SGmaterial* sgSphereGetMaterial(OOsphere *sphere);


SGdrawable* ooSgNewSphere(const char *name, float radius, const char *tex);
SGdrawable* sgNewEllipsis(const char *name,
                          double semiMajor, double semiMinor,
                          double longAsc, double inc, double argPeri,
                          float r, float g, float b,
                          size_t vertCount);

SGdrawable* sgLoadModel(const char *file);
void sgSceneAddLight(OOscene *sc, SGlight *light);
SGlight* sgNewPointlight(OOscene *sc, float3 p);
SGlight* sgNewPointlight3f(OOscene *sc, float x, float y, float z);
void sgSetLightPos3f(SGlight *light, float x, float y, float z);
void sgSetLightPosv(SGlight *light, float3 v);
void sgSetLightPosLW(SGlight *light, OOlwcoord *lwc);



void sgSetMaterialAmb4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialDiff4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialSpec4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialEmiss4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialShininess(SGmaterial *mat, float s);


#endif /* SCENEGRAPH_H_ */
