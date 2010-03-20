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



OOscene*
ooSgGetRoot(OOscenegraph *sg)
{
  assert(sg != NULL);

  return sg->root;
}

OOcam*
ooSgGetCam(OOscenegraph *sg)
{
  assert(sg != NULL);

  return sg->currentCam;
}

OOscene*
ooSgSceneGetParent(OOscene *sc)
{
  assert(sc != NULL);
  return sc->parent;
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
  sg->root = ooSgNewScene(NULL, "root");
  sg->root->sg = sg;
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

void ooSgSetCam(OOscenegraph *sg, OOcam *cam)
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
ooSgNewScene(OOscene *parent, const char *name)
{
  assert(name != NULL);

  OOscene *sc = malloc(sizeof(OOscene));
  sc->parent = parent;
  sc->name = strdup(name);
  sc->t = vf3_set(0.0, 0.0, 0.0);
  //sc->s = 1.0;
  //sc->si = 1.0;
  sc->amb[0] = 0.2;
  sc->amb[1] = 0.2;
  sc->amb[2] = 0.2;
  sc->amb[3] = 1.0;

  memset(sc->lights, 0, SG_MAX_LIGHTS * sizeof(SGlight*));
  sc->sg = NULL;

  obj_array_init(&sc->scenes);
  obj_array_init(&sc->objs);

  if (parent) {
    sc->sg = parent->sg;
    obj_array_push(&parent->scenes, sc);
  }

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
ooSgSceneAddChild(OOscene * restrict parent, OOscene * restrict child)
{
  assert(parent != NULL);
  assert(child != NULL);
  obj_array_push(&parent->scenes, child);
  child->parent = parent;
  child->sg = parent->sg;
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
ooSgSceneGetRoot(OOscene *sc)
{
  assert(sc != NULL);

  while (sc->parent) sc = sc->parent;
  return sc;
}

OOscene*
ooSgGetScene(OOscenegraph *sg, const char *sceneName)
{
  assert(sg != NULL);

  char str[strlen(sceneName)+1];
  strcpy(str, sceneName); // TODO: We do not trust the user, should probably
                          // check alloca result

  OOscene *scene = sg->root;
  char *strp = str;
  char *strTok = strsep(&strp, "/");
  int idx = 0;
  obj_array_t *vec = NULL;

  while (scene) {
    fprintf(stderr, "%s == %s\n", scene->name, strTok);
    if (!strcmp(scene->name, strTok)) {
      if (strp == NULL) {
        // At the end of the sys path
        return scene;
      }

      // If this is not the lowest level, go one level down
      strTok = strsep(&strp, "/");

      vec = &scene->scenes;
      idx = 0;
      if (vec->length <= 0) {
        fprintf(stderr, "no subscenes found\n");
        return NULL;
      }
      scene = vec->elems[idx];
    } else {
      if (vec == NULL) {
        assert(0 && "not found");
        return NULL;
      }
      idx ++;
      if (vec->length <= idx) {
        assert(0 && "index overflow");
        return NULL;
      }
      scene = vec->elems[idx];
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

//void
//ooSgSceneSetScale(OOscene *sc, float s)
//{
//  sc->s = s;
//  sc->si = 1.0f / s;
//}

int
compareDistances(SGdrawable const **o0, SGdrawable const **o1)
{
  bool gt = vf3_gt((*o0)->p, (*o1)->p);

  if (gt) return -1;
  else return 1;
}

void
sgSceneDraw(OOscene *sc, bool recurse)
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
    glPushMatrix();
    glTranslatef(vf3_x(obj->p), vf3_y(obj->p), vf3_z(obj->p));
    matrix_t m;
    q_m_convert(&m, obj->q);
    matrix_t mt;
    m_transpose(&mt, &m);
    glMultMatrixf((GLfloat*)&mt);

    obj->draw(obj);
    glPopMatrix();
  }

  if (recurse) {
    // Render subscenes
    for (size_t i = 0 ; i < sc->scenes.length ; i ++) {
      OOscene *subScene = sc->scenes.elems[i];

      glPushMatrix();
      //glScalef(subScene->si, subScene->si, subScene->si);
      glTranslatef(vf3_x(subScene->t), vf3_y(subScene->t), vf3_z(subScene->t));

      sgSceneDraw(subScene, true);
      glPopMatrix();
    }
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


  //OOscene *sc = sg->currentCam->scene->parent;

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

  // Now, in order to suport grand scales, we draw the scene with the current
  // camera (this will recursivly draw it's child scenes)
  glPushMatrix();

  ooSgCamRotate(sg->currentCam);
  ooSgCamMove(sg->currentCam);
  sgSceneDraw(sg->currentCam->scene, true);

  // At this point we have not drawn the parent scene, so we start go upwards
  // in the scenegraph now and apply the scales. Note that the SG is a tree
  // with respect to the scenes, so we only keep track of the leaf we came
  // from in order to prevent us from recursing back into scenes that have
  // already been drawn
  OOscene *sc = sg->currentCam->scene->parent;
  OOscene *prev = sg->currentCam->scene;

  while (sc) {
    //glScalef(prev->s, prev->s, prev->s);
     // Invert translation as we are going up in hierarchy
    glTranslatef(-vf3_x(prev->t), -vf3_y(prev->t), -vf3_z(prev->t));

    // Note that for each step we do here we must adjust the scales
    // appropriatelly
    for (size_t i = 0; i < sc->scenes.length ; i ++) {
      if (sc->scenes.elems[i] != prev) { // only draw non drawn scenes
        glPushMatrix();
        OOscene *subScene = sc->scenes.elems[i];
        //glScalef(subScene->si, subScene->si, subScene->si);
        glTranslatef(vf3_x(subScene->t), vf3_y(subScene->t), vf3_z(subScene->t));

        sgSceneDraw(subScene, true);
        glPopMatrix();
      }
    }

    sgSceneDraw(sc, false);
    // go up in the tree
    prev = sc;
    sc = sc->parent;
  }
  glPopMatrix();
}

void
ooSgDrawFuncGnd(SGdrawable *obj)
{

}


static void
setSgRec(OOscenegraph *sg, OOscene *sc)
{
  for (size_t i = 0; i < sc->scenes.length ; i ++) {
    ((OOscene*)sc->scenes.elems[i])->sg = sg;
    setSgRec(sg, sc->scenes.elems[i]);
  }
}
void
ooSgSetRoot(OOscenegraph *sg, OOscene *sc)
{
  // TODO: Delete previous scene
  sg->root = sc;

  // This is really ugly, we should initialise these properly when the objects
  // are created
  // TODO: RAII please
  sc->sg = sg;
  for (size_t i = 0; i < sc->scenes.length ; i ++) {
    ((OOscene*)sc->scenes.elems[i])->sg = sg;
    setSgRec(sg, sc->scenes.elems[i]);
  }
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
