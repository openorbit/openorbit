/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2008 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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
    Updates a drawable object with ode-data. This conforms to the 
*/

void
ooSgSetScenePos(OOscene *sc, float x, float y, float z)
{
  assert(sc != NULL);
  
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


void
ooSgUpdateObject(dBodyID body)
{
  OOdrawable *obj = dBodyGetData(body);
  const dReal *pos = dBodyGetPosition(body);
  //const dReal *rot = dBodyGetRotation(body);
  const dReal *quat = dBodyGetQuaternion(body);
  //const dReal *linVel = dBodyGetLinearVel(body);
  //const dReal *angVel = dBodyGetAngularVel(body);
  
  obj->p = v_set(pos[0], pos[1], pos[2], 0.0);
  obj->q = v_set(quat[1], quat[2], quat[3], quat[0]);
}

OOdrawable*
ooSgNewDrawable(OOobject *obj, OOdrawfunc df)
{
  OOdrawable *drawable = malloc(sizeof(OOdrawable));
  drawable->p = v_set(0.0f, 0.0f, 0.0f, 0.0f);
  drawable->q = q_rot(1.0f, 0.0f, 0.0f, 0.0f);
  drawable->s = 1.0;
  drawable->draw = df;
  drawable->obj = obj;
  return drawable;
}

OOscenegraph* ooSgNewSceneGraph()
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
    OOscene *sc = malloc(sizeof(OOscene));
    sc->parent = parent;
    sc->name = strdup(name);
    ooObjVecInit(&sc->scenes);
    ooObjVecInit(&sc->objs);

    sc->data = NULL;
    sc->preStepUpdate = NULL;

    if (parent) {
      ooObjVecPush(&parent->scenes, sc);
    }

    return sc;
}

void
updateOde(OOscene *sc)
{
  const dReal *p = dBodyGetPosition(sc->data); 
  const dReal *r = dBodyGetQuaternion(sc->data);
  
  sc->t = v_set(p[0], p[1], p[2], 1.0f);
  sc->q = v_set(r[1], r[2], r[3], r[0]);
}


void
updateEllipse(OOscene *sc)
{
  OOorbsys *os = sc->data;
  
  float ts = ooTimeGetJD();
  
  v4f_t p = ooGeoEllipseSegPoint(os->orbit,
                                 ts / os->phys.param.period * os->orbit->vec.length);
  
  sc->t.v = p;
}

void
ooSgSceneAttachOrbSys(OOscene *sc, OOorbsys *sys)
{
  assert(sc != NULL);
  assert(sys != NULL);
  
  sc->data = sys;
  sc->preStepUpdate = (OOdrawfunc)updateEllipse;
}

void
ooSgSceneAttachOdeObj(OOscene *sc, dBodyID body)
{
  assert(sc != NULL);
  sc->data = body;
  sc->preStepUpdate = (OOdrawfunc)updateOde;
}

void
ooSgSceneAddChild(OOscene *parent, OOscene *child)
{
  assert(parent != NULL);
  assert(child != NULL);
    ooObjVecPush(&parent->scenes, child);
    child->parent = parent;
}

void
ooSgSceneAddObj(OOscene *sc, OOobject *object)
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
ooSgSceneDraw(OOscene *sc)
{
    assert(sc != NULL);
    
    // Apply scene transforms
    glPushMatrix();
    glTranslatef(sc->t.x, sc->t.y, sc->t.z);
    
    // Render objects
    for (size_t i = 0 ; i < sc->objs.length ; i ++) {
      OOdrawable *obj = sc->objs.elems[i];
      glPushMatrix();
      glTranslatef(obj->p.x, obj->p.y, obj->p.z);
      matrix_t m;
      q_m_convert(&m, obj->q);
      glMultMatrixf((GLfloat*)&m);
      glScalef(obj->s, obj->s, obj->s);
      obj->draw(obj->obj);
      glPopMatrix();
    }
    
    
    // Render subscenes
    for (size_t i = 0 ; i < sc->scenes.length ; i ++) {
      OOscene *subScene = sc->scenes.elems[i];
      glPushMatrix();
      glScalef(subScene->s, subScene->s, subScene->s);
      ooSgSceneDraw(subScene);
      glPopMatrix();
    }
    
    // Pop scene transform
    glPopMatrix();
}

void
ooSgPreUpdate(OOscene *sc)
{
  assert(sc != NULL);
  
  if (sc->preStepUpdate != NULL) {
    sc->preStepUpdate(sc);
  }
  
  for (size_t i = 0 ; i < sc->scenes.length ; i ++) {
    ooSgPreUpdate(sc->scenes.elems[i]);
  }
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

    //ooSgPreUpdate(sg->root);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    // Compute sky rotation
    OOscene *sc = sg->currentCam->scene->parent;
    quaternion_t q = sg->currentCam->scene->q;
    
    while (sc) {
      q = q_mul(q, sc->q);
      sc = sc->parent;
    }
    // rotate sg->currentCam
    matrix_t m;
    q_m_convert(&m, q);
    glPushMatrix();
    glMultMatrixf((GLfloat*)&m);
    // Draw the sky
    sg->sky->draw(sg->sky->obj);
    glPopMatrix();

    // Then the overlays
    for (size_t i = 0 ; i < sg->overlays.length ; i ++) {
      ooSgDrawOverlay(sg->overlays.elems[i]);
    }

    // Now, in order to suport grand scales, we draw the scene with the current
    // camera (this will recursivly draw it's child scenes)
    ooSgSceneDraw(sg->currentCam->scene);

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
      
      glScalef(prev->si, prev->si, prev->si);
      
      for (size_t i = 0; i < sc->scenes.length ; i ++) {
        if (sc->scenes.elems[i] != prev) { // only draw non drawn scenes
          ooSgSceneDraw(sc->scenes.elems[i]);
        }
      }
      
      // go up in the tree
      prev = sc;
      sc = sc->parent;
    }
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
ooSgCamPopMove(OOcam *cam)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void
ooSgDrawFuncGnd(OOobject*obj)
{
    
}

void
ooSgDrawSphere(OOsphere *sp)
{
    glBindTexture(GL_TEXTURE_2D, sp->texId);    
    gluSphere(sp->quadratic, sp->radius, 64, 64);
    
    glPopMatrix();
}

OOdrawable*
ooSgNewSphere(float size)
{
  
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


void
ooSgTransform(OOtransform *t)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}

void
ooSgPostTransform(OOtransform *t)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


void
ooSgOdeTransform(OOodetransform *t)
{
    const dReal *pos = dBodyGetPosition(t->body);
    const dReal *rot = dBodyGetRotation(t->body);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glTranslatef(pos[0], pos[1], pos[2]);
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



OOnode*
ooSgNewTransform(float dx, float dy, float dz, float rx, float ry, float rz, float rot)
{
    OOtransform *transf = malloc(sizeof(OOtransform));
    if (transf == NULL) {return NULL;}
    OOnode *node = ooSgNewNode(transf,
                               (OOdrawfunc)ooSgTransform,
                               (OOdrawfunc)ooSgPostTransform);
        
    return node;
}

OOnode*
ooSgNewOdeTransform(dWorldID world, dBodyID body)
{
    OOodetransform *transf = malloc(sizeof(OOodetransform));
    if (transf == NULL) {return NULL;}
    OOnode *node = ooSgNewNode(transf,
                               (OOdrawfunc)ooSgOdeTransform,
                               (OOdrawfunc)ooSgPostTransform);
    
    transf->world = world;
    transf->body = body;
    
    return node;
}

OOnode*
ooSgNewSphere(OOtexture *tex)
{
    OOsphere *sphere = malloc(sizeof(OOsphere));
    if (sphere == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(sphere, (OOdrawfunc)ooSgDrawSphere, NULL);
    sphere->texId = tex->texId;
    
    return node;
}

void
ooSgDrawSphere(OOsphere *sphere)
{
//    matrix_t m;
//    glPushMatrix();
    
    glBindTexture(GL_TEXTURE_2D, sphere->texId);
        
//    matrix_t rot_orig;
//    matrix_t rot_ax;
//    q_m_convert(&rot_orig, planet->rot_orig);
//    q_m_convert(&rot_ax, planet->rot_ax);
    
//    glMultMatrixf((GLfloat*)rot_orig.a);
//    glTranslatef(planet->pos.x, planet->pos.y, planet->pos.z );
//    glMultMatrixf((GLfloat*)rot_ax.a);
    
    gluSphere(sphere->quadratic, sphere->radius, 64, 64);
    
    glPopMatrix();
    
}

OOnode*
ooSgNewSky(void)
{
    OOstars *stars = ooSkyInitStars(8000);
    
    if (stars == NULL) {return NULL;}
    
    OOnode *node = ooSgNewNode(stars, (OOdrawfunc)ooSkyDrawStars, NULL);

    return node;
}


void
ooSgScale(OOscale *scale)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    glScalef(scale->s, scale->s, scale->s);
}

void
ooSgPostScale(OOscale *scale)
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


OOnode*
ooSgNewScale(float scale)
{
    OOscale *scaleNode = (OOscale*) malloc(sizeof(OOscale));
    scaleNode->s = scale;
    
    OOnode *node = ooSgNewNode(scaleNode, (OOdrawfunc)ooSgScale, (OOdrawfunc)ooSgPostScale);
    return node;
}
#endif