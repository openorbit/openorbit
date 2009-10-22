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
#include "scenegraph-private.h"
#include "sky.h"
#include "log.h"
#include <vmath/vmath.h>
#include "geo/geo.h"
#include "physics/orbit.h"
#include "texture.h"
#include "common/lwcoord.h"

typedef struct {
    float uv[2];
    float rgba[4];
    float norm[3];
    float vert[3];
} OOvertex;

typedef struct {
  OOdrawable super;
  size_t vSize;
  size_t vCount;
  OOvertex *vertices;
  uint64_t texId;
} OOmesh;

typedef struct {
  OOdrawable super;
  GLuint texId;
  GLUquadricObj *quadratic;
  GLfloat radius;
} OOsphere;

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

/*!
    Updates a drawable object with ode-data.
*/
void
ooSgSetScenePos(OOscene *sc, float x, float y, float z)
{
  assert(sc != NULL);

  ooLogTrace("setting scene %s pos to [%f, %f, %f]", sc->name, x, y, z);
  sc->t = vf3_set(x, y, z);
}

//void
//ooSgSetSceneScale(OOscene *sc, float scale)
//{
//  assert(sc != NULL);
//  sc->s = scale;
//  sc->si = 1.0f/scale;
//}

void
ooSgSetObjectPosLW(OOdrawable *obj, const OOlwcoord *lw)
{
  // Get camera position and translate the lw coord with respect to the camera
  OOscene *sc = obj->scene;
  OOscenegraph *sg = sc->sg;
  OOcam *cam = sg->currentCam;
  
  if (cam->kind == OOCam_Free) {
    float3 relPos = ooLwcRelVec(lw, ((OOfreecam*)cam->camData)->lwc.seg);
    obj->p = relPos;
  }
}
void
ooSgSetObjectPos(OOdrawable *obj, float x, float y, float z)
{
  assert(obj != NULL);
  obj->p = vf3_set(x, y, z);
}

void
ooSgSetObjectQuat(OOdrawable *obj, float x, float y, float z, float w)
{
  assert(obj != NULL);
  obj->q = vf4_set(x, y, z, w);
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
  obj->dp = vf3_set(dx, dy, dz);
}

void
ooSgSetObjectAngularSpeed(OOdrawable *obj, float drx, float dry, float drz)
{
  assert(obj != NULL);
  obj->dr = vf3_set(drx, dry, drz);
}

OOdrawable*
ooSgNewDrawable(OOdrawable *drawable, const char *name, OOdrawfunc df)
{
  assert(df != NULL);
  assert(drawable != NULL);

  //  OOdrawable *drawable = malloc(sizeof(OOdrawable));
  drawable->name = strdup(name);
  drawable->p = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->q = q_rot(1.0f, 0.0f, 0.0f, 0.0f);
  drawable->s = 1.0;
  drawable->dr = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->dp = vf3_set(0.0f, 0.0f, 0.0f);

  drawable->draw = df;

  drawable->scene = NULL;

  return drawable;
}

OOscenegraph*
ooSgNewSceneGraph()
{
  OOscenegraph *sg = malloc(sizeof(OOscenegraph));
  sg->root = ooSgNewScene(NULL, "root");
  sg->root->sg = sg;

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

  sc->t = vf3_set(0.0, 0.0, 0.0);
  //sc->s = 1.0;
  //sc->si = 1.0;
  
  sc->sg = NULL;
  
  ooObjVecInit(&sc->scenes);
  ooObjVecInit(&sc->objs);

  if (parent) {
    sc->sg = parent->sg;
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
  child->sg = parent->sg;
}

void
ooSgSceneAddObj(OOscene *sc, OOdrawable *object)
{
  assert(sc != NULL);
  assert(object != NULL);

  object->scene = sc;
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
compareDistances(OOdrawable const **o0, OOdrawable const **o1)
{
  bool gt = vf3_gt((*o0)->p, (*o1)->p);

  if (gt) return -1;
  else return 1;
}

void
ooSgSceneDraw(OOscene *sc, bool recurse)
{
  assert(sc != NULL);
  ooLogTrace("drawing scene %s at %vf", sc->name, sc->t);

  // Sort objects based on distance from camera, since we are moving
  // objects and not the camera, this is trivial.
  //  qsort(sc->objs.elems, sc->objs.length, sizeof(OOdrawable*),
  //        (int (*)(void const *, void const *))compareDistances);

  glDepthFunc(GL_LEQUAL);

  // Apply scene transforms
  glPushMatrix();

  // Render objects
  for (size_t i = 0 ; i < sc->objs.length ; i ++) {
    OOdrawable *obj = sc->objs.elems[i];
    ooLogTrace("drawing object %s", obj->name);
    glPushMatrix();
    glTranslatef(vf3_x(obj->p), vf3_y(obj->p), vf3_z(obj->p));
    matrix_t m;
    q_m_convert(&m, obj->q);
    glMultMatrixf((GLfloat*)&m);
    //glScalef(obj->s, obj->s, obj->s);

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

  ooSgCamMove(sg->currentCam);
  ooSgSceneDraw(sg->currentCam->scene, true);

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
  glColor3f(1.0f, 1.0f, 1.0f);
  gluSphere(sp->quadratic, sp->radius, 128, 128);

  //  glMatrixMode(GL_MODELVIEW);

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
  return ooSgNewDrawable((OOdrawable*)sp, name, (OOdrawfunc) ooSgDrawSphere);
}

typedef struct SGellipsis {
  OOdrawable super;
  float semiMajor;
  float semiMinor;
  float ecc;
  float colour[3];
  size_t vertCount;
  float verts[];
} SGellipsis;

void
sgDrawEllipsis(SGellipsis *el)
{
  // Rest may be needed later...
  //glDisable (GL_BLEND);
  //glDisable (GL_DITHER);
  //glDisable (GL_FOG);
  //glDisable (GL_LIGHTING);
  //glDisable (GL_TEXTURE_1D);
  //glDisable (GL_TEXTURE_3D);
  //glShadeModel (GL_FLAT);

  glDisable(GL_TEXTURE_2D); // Lines are not textured...
  // glDisable(GL_DEPTH_TEST);

  glPushMatrix();
  glLineWidth(1.0);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, el->verts);
  glColor3f(el->colour[0], el->colour[1], el->colour[2]);
  glDrawArrays(GL_LINE_LOOP, 0, el->vertCount);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();
}

OOdrawable*
sgNewEllipsis(const char *name,
              float semiMajor, float semiMinor,
              float r, float g, float b,
              size_t vertCount)
{
  SGellipsis *el = malloc(sizeof(SGellipsis) + vertCount * 2 * sizeof(float));
  el->semiMajor = semiMajor;
  el->semiMinor = semiMinor;
  el->ecc = sqrt((semiMajor * semiMajor - semiMinor * semiMinor)
                 / (semiMajor * semiMajor));
  el->colour[0] = r;
  el->colour[1] = g;
  el->colour[2] = b;
  el->vertCount = vertCount;
  ooLogInfo("new ellipses '%s' %f %f %f", name, semiMajor, semiMinor, el->ecc);

  // Build an ellipse with the requested number of vertices
  float w = 2.0 * M_PI / (float)vertCount;
  for (int i = 0 ; i < vertCount ; ++ i) {
    el->verts[2*i] = semiMajor * cos(w * (float)i) - el->semiMajor*el->ecc;
    el->verts[2*i+1] = semiMinor * sin(w * (float)i);
  }

  return ooSgNewDrawable((OOdrawable*)el, name, (OOdrawfunc)sgDrawEllipsis);
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