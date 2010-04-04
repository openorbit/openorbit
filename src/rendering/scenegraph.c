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


#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif

#include "scenegraph.h"
#include "scenegraph-private.h"
#include "sky.h"
#include "log.h"
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"
#include "texture.h"
#include "common/lwcoord.h"
#include "parsers/model.h"


OOcam*
ooSgGetCam(OOscenegraph *sg)
{
  assert(sg != NULL);

  return sg->currentCam;
}

void
ooSgSetScenePos(OOscene *sc, float x, float y, float z)
{
  assert(sc != NULL);

  ooLogTrace("setting scene %s pos to [%f, %f, %f]", sc->name, x, y, z);
  sc->t = vf3_set(x, y, z);
}


OOscenegraph*
ooSgNewSceneGraph()
{
  OOscenegraph *sg = malloc(sizeof(OOscenegraph));

  sg->usedLights = 0;
  obj_array_init(&sg->cams);
  obj_array_init(&sg->overlays);
  obj_array_init(&sg->scenes);
  return sg;
}


static int
compareScenes(void *camPos, const void *a, const void *b)
{
  OOlwcoord *pos = camPos;
  const OOscene * const *as = a;
  const OOscene * const *bs = b;
  float3 da = ooLwcDist(pos, &(*as)->p);
  float3 db = ooLwcDist(pos, &(*bs)->p);
  if (vf3_lte(da, db)) {
    return -1;
  }

  return 1;
}

void
sgSortScenes(OOscenegraph *sg)
{
  switch (sg->currentCam->kind) {
  case OOCam_Orbit:
    {
      OOorbitcam *ocam = (OOorbitcam*)sg->currentCam;
      qsort_r(sg->scenes.elems, sg->scenes.length, sizeof(OOscene*), &ocam->body->p, compareScenes);
    }
    break;
  case OOCam_Free:
    {
      OOfreecam *fcam = (OOfreecam*)sg->currentCam;
      qsort_r(sg->scenes.elems, sg->scenes.length, sizeof(OOscene*), &fcam->lwc, compareScenes);

    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam *fcam = (OOfixedcam*)sg->currentCam;
      qsort_r(sg->scenes.elems, sg->scenes.length, sizeof(OOscene*), &fcam->body->p, compareScenes);
    }
    break;
  }
}

void
ooSgSetCam(OOscenegraph *sg, OOcam *cam)
{
  assert(sg != NULL);
  assert(cam != NULL);

  #ifdef DEBUG
  bool found = false;
  for (size_t i = 0 ; i < sg->cams.length ; i ++) {
    if (cam == sg->cams.elems[i]) found = true;
  }
  assert(found == true);
  #endif

  sg->currentCam = cam;
}

void
sgSetScenePos(OOscene *sc, const OOlwcoord *lwc)
{
  sc->p = *lwc;
}



OOscene*
ooSgNewScene(OOscenegraph *sg, const char *name)
{
  assert(name != NULL);

  OOscene *sc = malloc(sizeof(OOscene));
  sc->name = strdup(name);
  sc->t = vf3_set(0.0, 0.0, 0.0);

  sc->amb[0] = 0.2;
  sc->amb[1] = 0.2;
  sc->amb[2] = 0.2;
  sc->amb[3] = 1.0;

  memset(sc->lights, 0, SG_MAX_LIGHTS * sizeof(SGlight*));
  sc->sg = sg;

  obj_array_init(&sc->objs);

  obj_array_push(&sg->scenes, sc);

  return sc;
}


void
sgSetSceneAmb4f(OOscene *sc, float r, float g, float b, float a)
{
  sc->amb[0] = r;
  sc->amb[1] = g;
  sc->amb[2] = b;
  sc->amb[3] = a;
}


void
ooSgSceneAddObj(OOscene *sc, SGdrawable *object)
{
  assert(sc != NULL);
  assert(object != NULL);

  object->scene = sc;
  obj_array_push(&sc->objs, object);
}

OOscene*
ooSgGetScene(OOscenegraph *sg, const char *sceneName)
{
  assert(sg != NULL);

  for (int i = 0 ; i < sg->scenes.length ; ++ i) {
    if (!strcmp(sceneName, ((OOscene*)(sg->scenes.elems[i]))->name)) {
      return (OOscene*)sg->scenes.elems[i];
    }
  }

  assert(0 && "dont ask for non existant things");
  return NULL;
}



void
ooSgDrawOverlay(OOoverlay *overlay)
{
  glMatrixMode(GL_MODELVIEW);
  // TODO: Ensure that depth test is run, but overlay is infinitly close
  glDisable(GL_DEPTH_TEST);//|GL_LIGHTNING);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

//  glBindTexture(GL_TEXTURE_2D, overlay->tex->texId);

  glBegin(GL_QUADS);
    glVertex2f(overlay->x, overlay->y);
    glVertex2f(overlay->w, overlay->y);
    glVertex2f(overlay->w, overlay->h);
    glVertex2f(overlay->x, overlay->h);
  glEnd();

  glPopMatrix();
}

int
compareDistances(SGdrawable const **o0, SGdrawable const **o1)
{
  bool gt = vf3_gt((*o0)->p, (*o1)->p);

  if (gt) return -1;
  else return 1;
}

void
sgSceneDraw(OOscene *sc)
{
  assert(sc != NULL);
  ooLogTrace("drawing scene %s at %vf", sc->name, sc->t);

  // Sort objects based on distance from camera, since we are moving
  // objects and not the camera, this is trivial.
  //  qsort(sc->objs.elems, sc->objs.length, sizeof(OOdrawable*),
  //        (int (*)(void const *, void const *))compareDistances);

  glClear(GL_DEPTH_BUFFER_BIT);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sc->amb);
  glDepthFunc(GL_LEQUAL);

  // Apply scene transforms
  glPushMatrix();
  int localLights = 0;
  for (int i = 0 ; i < SG_MAX_LIGHTS ; ++ i) {
    if (sc->lights[i] != NULL) {
     if (sc->sg->usedLights < SG_MAX_LIGHTS) {
       GLenum lightId = sgLightNumberMap[sc->sg->usedLights];
       SGlight *light = sc->lights[i];

       light->enable(light, lightId);

       sc->sg->usedLights ++;
       localLights ++;
     } else {
       ooLogWarn("to many light sources in scene hierarchy, current scene = '%s'",
                 sc->name);
     }
    }
  }

  // Render objects
  for (size_t i = 0 ; i < sc->objs.length ; i ++) {
    SGdrawable *obj = sc->objs.elems[i];
    ooLogTrace("drawing object %s", obj->name);
    sgPaintDrawable(obj);
  }

  // Pop scene transform
  glPopMatrix();


  for (int i = 0 ; i < localLights ; ++ i) {
    if (sc->lights[i] != NULL) {
      SGlight *light = sc->lights[i];
      light->disable(light);
    } else {
      ooLogWarn("null found in light vector");
    }
  }

  sc->sg->usedLights -= localLights;
}


void
ooSgSetSky(OOscenegraph *sg, SGdrawable *obj)
{
  assert(sg != NULL);
  assert(obj != NULL);
  sg->sky = obj;
}
// Drawing is done as follows:
//   when drawing is commanded with a camera, we get the cameras scene and
//   paint that, after that, we go to the cam scenes parent and inverse
//   any rotations and transforms and paint that
void
ooSgPaint(OOscenegraph *sg)
{
  assert(sg != NULL);
  ooLogTrace("SgPaint");

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();
  ooSgCamRotate(sg->currentCam);
  // Draw the sky
  sg->sky->draw(sg->sky);
  glPopMatrix();

  // Then the overlays
  for (size_t i = 0 ; i < sg->overlays.length ; i ++) {
    ooLogTrace("draw overlay %d", i);
    ooSgDrawOverlay(sg->overlays.elems[i]);
  }

  glPushMatrix();

  ooSgCamRotate(sg->currentCam);
  ooSgCamMove(sg->currentCam);
  sgSceneDraw(sg->currentCam->scene);

  glPopMatrix();
}

void
ooSgDrawFuncGnd(SGdrawable *obj)
{

}



void
sgSceneAddLight(OOscene *sc, SGlight *light)
{
  for (int i = 0 ; i < SG_MAX_LIGHTS ; ++ i) {
    if (sc->lights[i] == NULL) {
      sc->lights[i] = light;
      return;
    }
  }

  ooLogWarn("to many lights added to scenes '%s'", sc->name);
}
