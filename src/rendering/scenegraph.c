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
//#include <OpenGL/gl.h>
#include "SDL_opengl.h"
#include "scenegraph.h"
#include "sky.h"
#include "log.h"
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"

/*!
    Updates a drawable object with ode-data.
*/
void
ooSgSetScenePos(OOscene *sc, float x, float y, float z)
{
  assert(sc != NULL);

  ooLogTrace("setting scene %s pos to [%f, %f, %f]", sc->name, x, y, z);
  sc->t.x = x;
  sc->t.y = y;
  sc->t.z = z;
  sc->t.w = 0.0;
}

void
ooSgSetSceneQuat(OOscene *sc, float x, float y, float z, float w)
{
  assert(sc != NULL);

  sc->q.x = x;
  sc->q.y = y;
  sc->q.z = z;
  sc->q.w = w;
}

void
ooSgSetSceneScale(OOscene *sc, float scale)
{
  assert(sc != NULL);
  sc->s = scale;
  sc->si = 1.0f/scale;
}

// Conforms to dBodySetMovedCallback registered callbacks
void
ooSgUpdateObject(dBodyID body)
{
  OOdrawable *obj = dBodyGetData(body);
  ooLogTrace("updating body %s", obj->name);

  const dReal *pos = dBodyGetPosition(body);
  //const dReal *rot = dBodyGetRotation(body);
  const dReal *quat = dBodyGetQuaternion(body);
  const dReal *linVel = dBodyGetLinearVel(body);
  const dReal *angVel = dBodyGetAngularVel(body);

  obj->p = v_set(pos[0], pos[1], pos[2], 0.0);
  obj->q = v_set(quat[1], quat[2], quat[3], quat[0]);

  obj->dp = v_set(linVel[0], linVel[1], linVel[2], 0.0);
  obj->dr = v_set(angVel[0], angVel[1], angVel[2], 0.0);
}

void
ooSgSetObjectPos(OOdrawable *obj, float x, float y, float z)
{
  assert(obj != NULL);
  obj->p = v_set(x, y, z, 0.0);
}

void
ooSgSetObjectQuat(OOdrawable *obj, float x, float y, float z, float w)
{
  assert(obj != NULL);
  obj->q = v_set(x, y, z, w);
}

void
ooSgSetObjectScale(OOdrawable *obj, float s)
{
  assert(obj != NULL);
  obj->s = s;
}

void
ooSgSetObjectSpeed(OOdrawable *obj, float dx, float dy, float dz)
{
  assert(obj != NULL);
  obj->dp = v_set(dx, dy, dz, 0.0);
}

void
ooSgSetObjectAngularSpeed(OOdrawable *obj, float drx, float dry, float drz)
{
  assert(obj != NULL);
  obj->dr = v_set(drx, dry, drz, 0.0);
}

OOdrawable*
ooSgNewDrawable(const char *name, OOobject *obj, OOdrawfunc df)
{
  assert(obj != NULL);
  assert(df != NULL);

  OOdrawable *drawable = malloc(sizeof(OOdrawable));
  drawable->name = strdup(name);
  drawable->p = v_set(0.0f, 0.0f, 0.0f, 0.0f);
  drawable->q = q_rot(1.0f, 0.0f, 0.0f, 0.0f);
  drawable->s = 1.0;
  drawable->dr = v_set(0.0f, 0.0f, 0.0f, 0.0f);
  drawable->dp = v_set(0.0f, 0.0f, 0.0f, 0.0f);

  drawable->draw = df;
  drawable->obj = obj;
  return drawable;
}

OOscenegraph*
ooSgNewSceneGraph()
{
  OOscenegraph *sg = malloc(sizeof(OOscenegraph));
  sg->root = ooSgNewScene(NULL, "root");
  ooObjVecInit(&sg->cams);
  ooObjVecInit(&sg->overlays);
  return sg;
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


OOscene*
ooSgNewScene(OOscene *parent, const char *name)
{
  assert(name != NULL);

  OOscene *sc = malloc(sizeof(OOscene));
  sc->parent = parent;
  sc->name = strdup(name);

  sc->t = v_set(0.0, 0.0, 0.0, 0.0);
  sc->q = q_rot(0.0, 1.0, 0.0, 0.0); // Default with 0 deg rot around y
  sc->s = 1.0;
  sc->si = 1.0;

  ooObjVecInit(&sc->scenes);
  ooObjVecInit(&sc->objs);

  if (parent) {
    ooObjVecPush(&parent->scenes, sc);
  }

  return sc;
}

void
ooSgSceneAddChild(OOscene * restrict parent, OOscene * restrict child)
{
  assert(parent != NULL);
  assert(child != NULL);
  ooObjVecPush(&parent->scenes, child);
  child->parent = parent;
}

void
ooSgSceneAddObj(OOscene *sc, OOdrawable *object)
{
  assert(sc != NULL);
  assert(object != NULL);

  ooObjVecPush(&sc->objs, object);
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
  OOobjvector *vec = NULL;

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

  glBindTexture(GL_TEXTURE_2D, overlay->tex->texId);

  glBegin(GL_QUADS);
    glVertex2f(overlay->x, overlay->y);
    glVertex2f(overlay->w, overlay->y);
    glVertex2f(overlay->w, overlay->h);
    glVertex2f(overlay->x, overlay->h);
  glEnd();

  glPopMatrix();
}

void
ooSgSceneSetScale(OOscene *sc, float s)
{
  sc->s = s;
  sc->si = 1.0f / s;
}
void
ooSgSceneDraw(OOscene *sc, bool recurse)
{
  assert(sc != NULL);
  ooLogTrace("drawing scene %s at %vf", sc->name, sc->t.v);

  glDepthFunc(GL_LEQUAL);

  // Apply scene transforms
  glPushMatrix();
  glTranslatef(sc->t.x, sc->t.y, sc->t.z);

  // Render objects
  for (size_t i = 0 ; i < sc->objs.length ; i ++) {
    OOdrawable *obj = sc->objs.elems[i];
    ooLogTrace("drawing object %s", obj->name);
    glPushMatrix();
    glTranslatef(obj->p.x, obj->p.y, obj->p.z);
    matrix_t m;
    q_m_convert(&m, obj->q);
    glMultMatrixf((GLfloat*)&m);
    glScalef(obj->s, obj->s, obj->s);
    obj->draw(obj->obj);
    glPopMatrix();
  }

  if (recurse) {
    // Render subscenes
    for (size_t i = 0 ; i < sc->scenes.length ; i ++) {
      OOscene *subScene = sc->scenes.elems[i];

      glPushMatrix();
      glScalef(subScene->s, subScene->s, subScene->s);
      
      ooSgSceneDraw(subScene, true);
      glPopMatrix();
    }
  }
  // Pop scene transform
  glPopMatrix();
}


void
ooSgSetSky(OOscenegraph *sg, OOdrawable *obj)
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


  // Compute sky rotation
  OOscene *sc = sg->currentCam->scene->parent;

  quaternion_t q, q0;
  ooSgCamRotate(sg->currentCam);
  if (sg->currentCam->kind == OOCam_Free) {
    q0 = ((OOfreecam*)(sg->currentCam->camData))->q;
  } else {
    q0 = q_rot(0.0, 1.0, 0.0, 0.0);
  }

  q = q_mul(q0, sg->currentCam->scene->q);

  while (sc) {
    q = q_mul(q, sc->q);
    sc = sc->parent;
  }

  matrix_t m;
  q_m_convert(&m, q);
  glPushMatrix();
  glMultMatrixf((GLfloat*)&m);
  // Draw the sky
  sg->sky->draw(sg->sky->obj);
  glPopMatrix();

  // Then the overlays
  for (size_t i = 0 ; i < sg->overlays.length ; i ++) {
    ooLogTrace("draw overlay %d", i);
    ooSgDrawOverlay(sg->overlays.elems[i]);
  }

  // Now, in order to suport grand scales, we draw the scene with the current
  // camera (this will recursivly draw it's child scenes)
  q_m_convert(&m, q0);
  glPushMatrix();
  glMultMatrixf((GLfloat*)&m);

  ooSgCamMove(sg->currentCam);
  ooSgSceneDraw(sg->currentCam->scene, true);

  // At this point we have not drawn the parent scene, so we start go upwards
  // in the scenegraph now and apply the scales. Note that the SG is a tree
  // with respect to the scenes, so we only keep track of the leaf we came
  // from in order to prevent us from recursing back into scenes that have
  // already been drawn
  sc = sg->currentCam->scene->parent;
  OOscene *prev = sg->currentCam->scene;

  while (sc) {
    // Note that for each step we do here we must adjust the scales
    // appropriatelly
    glScalef(prev->s, prev->s, prev->s);
    glTranslatef(prev->t.x, prev->t.y, prev->t.z);

    for (size_t i = 0; i < sc->scenes.length ; i ++) {
      if (sc->scenes.elems[i] != prev) { // only draw non drawn scenes
        glPushMatrix();
        OOscene *subScene = sc->scenes.elems[i];
        glScalef(subScene->si, subScene->si, subScene->si);
        ooSgSceneDraw(subScene, true);
        glPopMatrix();
      }
    }

    ooSgSceneDraw(sc, false);
    // go up in the tree
    prev = sc;
    sc = sc->parent;
  }
  glPopMatrix();
}


void
ooObjVecInit(OOobjvector *vec)
{
    vec->asize = 16;
    vec->length = 0;
    vec->elems = calloc(vec->asize, sizeof(OOobject*));
}

void
ooObjVecCompress(OOobjvector *vec)
{
  // Remove last NULL pointers
  for (size_t i = vec->length; i > 0; i --) {
    if (vec->elems[i-1] != NULL) {
      vec->length = i;
      break;
    }
  }

  // Move pointers at end of vector to first free entries
  for (size_t i = 0; i < vec->length ; i ++) {
    if (vec->elems[i] == NULL) {
      vec->elems[i] = vec->elems[vec->length-1];
      vec->length --;
    }

    // Remove any unused space at the end of the vector
    for (size_t j = vec->length; j > 0; j --) {
      if (vec->elems[j-1] != NULL) {
        vec->length = j;
        break;
      }
    }
  }
}

void
ooObjVecPush(OOobjvector *vec, OOobject *obj)
{
    if (vec->length >= vec->asize) {
        OOobject *newVec = realloc(vec->elems,
                                   vec->asize * sizeof(OOobject*) * 2);
        if (newVec == NULL) ooLogFatal("realloc of vector failed");
        vec->asize *= 2;
        vec->elems = newVec;
    }
    vec->elems[vec->length ++] = obj;
}


OOobject*
ooObjVecPop(OOobjvector *vec)
{    
    return vec->elems[vec->length --];
}

OOobject*
ooObjVecGet(OOobjvector *vec, size_t i)
{
  if (vec->length <= i)
    ooLogFatal("vector out of bounds length = %d idx = %d", vec->length, i);
  else
    return vec->elems[i];
}

void
ooObjVecSet(OOobjvector *vec, size_t i, OOobject *obj)
{
  if (vec->length <= i)
    ooLogFatal("vector out of bounds length = %d idx = %d", vec->length, i);
  else
    vec->elems[i] = obj;  
}

void
ooSgDrawFuncGnd(OOobject*obj)
{

}

void
ooSgDrawSphere(OOsphere *sp)
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, sp->texId);    
    
//  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  
  gluSphere(sp->quadratic, sp->radius, 128, 128);

  glMatrixMode(GL_MODELVIEW);

}

OOdrawable*
ooSgNewSphere(const char *name, float radius, const char *tex)
{
  OOsphere *sp = malloc(sizeof(OOsphere));
  sp->radius = radius;
  ooTexLoad(tex, tex);
  sp->texId = ooTexNum(tex);
  sp->quadratic = gluNewQuadric();
  gluQuadricOrientation(sp->quadratic, GLU_OUTSIDE);
  gluQuadricNormals(sp->quadratic, GLU_SMOOTH);
  gluQuadricTexture(sp->quadratic, GL_TRUE);
  gluQuadricDrawStyle(sp->quadratic, GLU_FILL);
  return ooSgNewDrawable(name, sp, (OOdrawfunc) ooSgDrawSphere);
}

void ooSgSetRoot(OOscenegraph *sg, OOscene *sc)
{
  // TODO: Delete previous scene
  sg->root = sc;
}

#if 0


void
ooSgDrawMesh(OOmesh *mesh)
{
    assert(mesh != NULL);
    glBindTexture(GL_TEXTURE_2D, mesh->texId);
    glInterleavedArrays(GL_T2F_C4F_N3F_V3F, 0, mesh->vertices);
    glDrawArrays(GL_TRIANGLES, 0, mesh->vCount);
}

OOnode*
ooSgNewMesh(OOtexture *tex)
{
    OOmesh *mesh = malloc(sizeof(OOmesh));
    if (mesh == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(mesh, (OOdrawfunc)ooSgDrawMesh, NULL);
    mesh->vCount = 0;
    mesh->texId = tex->texId;
    
    mesh->vertices = calloc(32, sizeof(OOvertex));
    mesh->vSize = 32;
    
    return node;
}

void
ooSgMeshPushVert(OOnode *node, const OOvertex *v)
{
    OOmesh *mesh = node->obj;
    
    if (mesh->vCount = mesh->vSize) {
        OOvertex *newVerts = realloc(mesh->vertices,
                                     mesh->vSize * 2 * sizeof(OOvertex));
        
        if (!newVerts) ooLogFatal("vertex buffer expansion failed");
        mesh->vSize = mesh->vSize * 2;
        mesh->vertices = newVerts;
    }
    
    mesh->vertices[mesh->vCount] = *v;
    mesh->vCount ++;
}



#endif