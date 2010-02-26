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
#include "parsers/model.h"

struct OOsphere {
  SGdrawable super;
  SGmaterial mat;
  GLuint texId;
  GLUquadricObj *quadratic;
  GLfloat radius;
};


struct SGcylinder {
  SGdrawable super;
  //  GLuint texId;
  GLbyte col[3];
  GLUquadricObj *quadratic;
  GLfloat bottonRadius;
  GLfloat topRadius;
  GLfloat height;
};


void
sgBindMaterial(SGmaterial *mat)
{
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
  glMaterialfv(GL_FRONT, GL_EMISSION, mat->emission);
  glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess);
}

void
sgInitMaterial(SGmaterial *mat)
{
  mat->ambient[0] = 0.2;
  mat->ambient[1] = 0.2;
  mat->ambient[2] = 0.2;
  mat->ambient[3] = 1.0;

  mat->diffuse[0] = 0.8;
  mat->diffuse[1] = 0.8;
  mat->diffuse[2] = 0.8;
  mat->diffuse[3] = 1.0;

  mat->specular[0] = 0.0;
  mat->specular[1] = 0.0;
  mat->specular[2] = 0.0;
  mat->specular[3] = 1.0;

  mat->emission[0] = 0.0;
  mat->emission[1] = 0.0;
  mat->emission[2] = 0.0;
  mat->emission[3] = 1.0;

  mat->shininess = 0.0;
}

void
sgSetMaterialAmb4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->ambient[0] = r;
  mat->ambient[1] = g;
  mat->ambient[2] = b;
  mat->ambient[3] = a;
}

void
sgSetMaterialDiff4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->diffuse[0] = r;
  mat->diffuse[1] = g;
  mat->diffuse[2] = b;
  mat->diffuse[3] = a;
}

void
sgSetMaterialSpec4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->specular[0] = r;
  mat->specular[1] = g;
  mat->specular[2] = b;
  mat->specular[3] = a;
}

void
sgSetMaterialEmiss4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->emission[0] = r;
  mat->emission[1] = g;
  mat->emission[2] = b;
  mat->emission[3] = a;
}

void
sgSetMaterialShininess(SGmaterial *mat, float s)
{
  mat->shininess = s;
}

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


void
ooSgSetObjectPosLW(SGdrawable *obj, const OOlwcoord *lw)
{
  // Get camera position and translate the lw coord with respect to the camera
  OOscene *sc = obj->scene;
  OOscenegraph *sg = sc->sg;
  OOcam *cam = sg->currentCam;

  if (cam->kind == OOCam_Free) {
    float3 relPos = ooLwcRelVec(lw, ((OOfreecam*)cam)->lwc.seg);
    obj->p = relPos;
  } else if (cam->kind == OOCam_Fixed) {
    OOfixedcam *fix = (OOfixedcam*)cam;
    float3 relPos = ooLwcRelVec(lw, fix->body->p.seg) - (mf3_v_mul(fix->body->R, fix->r) + fix->body->p.offs);
    obj->p = relPos;
  } else if (cam->kind == OOCam_Orbit) {
    OOorbitcam *orb = (OOorbitcam*)cam;
    float3 relPos = ooLwcRelVec(lw, orb->body->p.seg);
    obj->p = relPos;
  }
}
void
ooSgSetObjectPos(SGdrawable *obj, float x, float y, float z)
{
  assert(obj != NULL);
  obj->p = vf3_set(x, y, z);
}

void
ooSgSetObjectQuat(SGdrawable *obj, float x, float y, float z, float w)
{
  assert(obj != NULL);
  obj->q = vf4_set(x, y, z, w);
}

void
sgSetObjectQuatv(SGdrawable *obj, quaternion_t q)
{
  assert(obj != NULL);
  obj->q = q;
}

void
ooSgSetObjectScale(SGdrawable *obj, float s)
{
  assert(0);
  assert(obj != NULL);
  //  obj->s = s;
}

void
ooSgSetObjectSpeed(SGdrawable *obj, float dx, float dy, float dz)
{
  assert(obj != NULL);
  obj->dp = vf3_set(dx, dy, dz);
}

void
ooSgSetObjectAngularSpeed(SGdrawable *obj, float drx, float dry, float drz)
{
  assert(obj != NULL);
  obj->dr = vf3_set(drx, dry, drz);
}

SGdrawable*
ooSgNewDrawable(SGdrawable *drawable, const char *name, SGdrawfunc df)
{
  assert(df != NULL);
  assert(drawable != NULL);

  drawable->name = strdup(name);
  drawable->p = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->q = q_rot(1.0f, 0.0f, 0.0f, 0.0f);
  drawable->dr = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->dp = vf3_set(0.0f, 0.0f, 0.0f);

  drawable->draw = df;

  drawable->scene = NULL;
  drawable->enabled = true;

  return drawable;
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

void
ooSgDrawSphere(OOsphere *sp)
{
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_LIGHTING);

  sgBindMaterial(&sp->mat);
  glBindTexture(GL_TEXTURE_2D, sp->texId);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glColor3f(1.0f, 1.0f, 1.0f);
  gluSphere(sp->quadratic, sp->radius, 128, 128);

  // Draw point on the sphere in solid colour and size
  glDisable (GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  //glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDepthFunc(GL_LEQUAL);

  glBegin(GL_POINTS);
  glPointSize(5.0);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glEnd();

  glDisable(GL_TEXTURE_2D); // Lines are not textured...
  glBegin(GL_LINES);

  glColor3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, -2.0*sp->radius);
  glVertex3f(0.0, 0.0, +2.0*sp->radius);

  glEnd();
}

SGdrawable*
ooSgNewSphere(const char *name, float radius, const char *tex)
{
  OOsphere *sp = malloc(sizeof(OOsphere));
  sgInitMaterial(&sp->mat);

  sp->radius = radius;
  ooTexLoad(tex, tex);
  sp->texId = ooTexNum(tex);
  sp->quadratic = gluNewQuadric();
  gluQuadricOrientation(sp->quadratic, GLU_OUTSIDE);
  gluQuadricNormals(sp->quadratic, GLU_SMOOTH);
  gluQuadricTexture(sp->quadratic, GL_TRUE);
  gluQuadricDrawStyle(sp->quadratic, GLU_FILL);
  return ooSgNewDrawable((SGdrawable*)sp, name, (SGdrawfunc) ooSgDrawSphere);
}


SGmaterial*
sgSphereGetMaterial(OOsphere *sphere)
{
  return &sphere->mat;
}


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
  glDisable(GL_LIGHTING); // Lines are not lit, just colored
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glPushMatrix();
  glLineWidth(1.0);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, el->verts);
  glColor3f(el->colour[0], el->colour[1], el->colour[2]);
  glDrawArrays(GL_LINE_LOOP, 0, el->vertCount);
  glDisableClientState(GL_VERTEX_ARRAY);
  glPopMatrix();

  // Place dot at periapsis
  glPointSize(10.0);
  glBegin(GL_POINTS);
  glColor3f(1.0-el->colour[0], 1.0-el->colour[1], 1.0-el->colour[2]);
  glVertex3f(el->verts[0], el->verts[1], el->verts[2]);
  glEnd();
}

SGdrawable*
sgNewEllipsis(const char *name,
              double semiMajor, double semiMinor,
              double longAsc, double inc, double argPeri,
              float r, float g, float b,
              size_t vertCount)
{
  SGellipsis *el = malloc(sizeof(SGellipsis) + vertCount * 3 * sizeof(float));
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
  double w = 2.0 * M_PI / (double)vertCount;
  for (int i = 0 ; i < vertCount ; ++ i) {
    double x = -semiMinor * sin(w * (double)i); // Note, when y is semi major axis, then x is pointing downwards
    double y = semiMajor * cos(w * (double)i) - el->semiMajor*el->ecc;
    double z = 0.0;

    float3 v = vf3_set(x, y, z);
    quaternion_t qasc = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(longAsc));
    quaternion_t qinc = q_rot(0.0, 1.0, 0.0, DEG_TO_RAD(inc));
    quaternion_t qaps = q_rot(0.0, 0.0, 1.0, DEG_TO_RAD(argPeri));

    quaternion_t q = q_mul(qasc, qinc);
    q = q_mul(q, qaps);

    matrix_t m;
    q_m_convert(&m, q);

    // rotate our point
    v = m_v3_mulf(&m, v);

    el->verts[3*i] = vf3_x(v);
    el->verts[3*i+1] = vf3_y(v);
    el->verts[3*i+2] = vf3_z(v);
  }

  return ooSgNewDrawable((SGdrawable*)el, name, (SGdrawfunc)sgDrawEllipsis);
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
sgDrawCylinder(SGcylinder *cyl)
{
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glColor3bv(cyl->col);
  gluCylinder(cyl->quadratic,
              cyl->bottonRadius, cyl->topRadius, cyl->height,
              4, 2);
}

SGdrawable*
sgNewCylinder(const char *name,
              float top, float bottom, float height)
{
  SGcylinder *cyl = malloc(sizeof(SGcylinder));
  cyl->height = height;
  cyl->bottonRadius = bottom;
  cyl->topRadius = top;

  cyl->col[0] = 255;
  cyl->col[1] = 255;
  cyl->col[2] = 255;

  return ooSgNewDrawable((SGdrawable*)cyl, name, (SGdrawfunc)sgDrawCylinder);
}


typedef struct SGmodel {
  SGdrawable super;
  model_t *modelData;
} SGmodel;


static void
setMaterial(material_t *mat)
{
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat->ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat->diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat->specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat->emission);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat->shininess);
}

void
drawModel(model_object_t *model)
{
  glColor3f(1.0, 1.0, 1.0);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glShadeModel(GL_SMOOTH);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
  setMaterial(model->model->materials[model->materialId]);

  glPushMatrix();

  glTranslatef(model->trans[0], model->trans[1], model->trans[2]);
  glMultMatrixf((GLfloat*)model->rot);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, model->vertices.elems);
  glTexCoordPointer(2, GL_FLOAT, 0, model->texCoords.elems);
  glNormalPointer(GL_FLOAT, 0, model->normals.elems);

  glDrawArrays(GL_TRIANGLES, 0, model->vertexCount);

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

#if 0
  // DEBUG: Draw normals, do not delete
  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  int vertices = model->vertexCount;
  for (int i = 0 ; i < vertices ; ++ i) {
    glVertex3fv(&model->vertices.elems[i*3]);
    glVertex3f(model->vertices.elems[i*3 + 0] + model->normals.elems[i*3 + 0],
               model->vertices.elems[i*3 + 1] + model->normals.elems[i*3 + 1],
               model->vertices.elems[i*3 + 2] + model->normals.elems[i*3 + 2]);
  }
  glEnd();

  glPointSize(2.0);
  glBegin(GL_POINTS);
  glColor3f(1.0, 1.0, 0.0);
  for (int i = 0 ; i < vertices ; ++ i) {
    glVertex3fv(&model->vertices.elems[i*3]);
  }
  glEnd();

  glEnable(GL_BLEND);
  glEnable(GL_LIGHTING);
#endif

  for (int i = 0 ; i < model->children.length ; ++ i) {
    drawModel(model->children.elems[i]);
  }

  glPopMatrix();
}

void
sgDrawModel(SGmodel *model)
{
  glEnable(GL_LIGHTING);
  drawModel(model->modelData->objs.elems[0]);
}

SGdrawable*
sgLoadModel(const char *file)
{
  SGmodel *model = malloc(sizeof(SGmodel));
  model->modelData = model_load(file);

  if (model->modelData == NULL) {
    ooLogError("loading model '%s', returned NULL model data", file);
    free(model);
    return NULL;
  }

  return ooSgNewDrawable((SGdrawable*)model, "unnamed",
                         (SGdrawfunc)sgDrawModel);
}

void
sgSetLightPos3f(SGlight *light, float x, float y, float z)
{
  light->pos[0] = vf3_x(x);
  light->pos[1] = vf3_y(y);
  light->pos[2] = vf3_z(z);
  light->pos[3] = 1.0;
}

void
sgSetLightPosv(SGlight *light, float3 v)
{
  light->pos[0] = vf3_x(v);
  light->pos[1] = vf3_y(v);
  light->pos[2] = vf3_z(v);
  light->pos[3] = 1.0;
}

void
sgSetLightPosLW(SGlight *light, OOlwcoord *lwc)
{
  OOscene *sc = light->scene;
  OOscenegraph *sg = sc->sg;
  OOcam *cam = sg->currentCam;

  if (cam->kind == OOCam_Free) {
    float3 relPos = ooLwcRelVec(lwc, ((OOfreecam*)cam)->lwc.seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  } else if (cam->kind == OOCam_Orbit) {
    float3 relPos = ooLwcRelVec(lwc, ((OOorbitcam*)cam)->body->p.seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  }
}


SGlight*
sgNewSpotlight(OOscenegraph *sg, float3 p, float3 dir)
{
  SGspotlight *light = malloc(sizeof(SGspotlight));

  return (SGlight*)light;
}

void
sgEnablePointLight(SGpointlight *light, GLenum lightId)
{
  glEnable(GL_LIGHTING);
  glEnable(lightId);
  light->super.lightId = lightId;

  glLightfv(lightId, GL_POSITION, light->super.pos);
  glLightfv(lightId, GL_AMBIENT, light->super.ambient);
  glLightfv(lightId, GL_DIFFUSE, light->super.diffuse);
  glLightfv(lightId, GL_SPECULAR, light->super.specular);
  //  glLightf(light->super.lightId, GL_CONSTANT_ATTENUATION, 1.0f);
  //  glLightf(light->super.lightId, GL_LINEAR_ATTENUATION, 0.2f);
  //  glLightf(light->super.lightId, GL_QUADRATIC_ATTENUATION, 0.08f);

}

void
sgDisablePointLight(SGpointlight *light)
{
  glDisable(light->super.lightId);
}

void
sgLightSetAmbient4f(SGlight *light, float r, float g, float b, float a)
{
  light->ambient[0] = r;
  light->ambient[1] = g;
  light->ambient[2] = b;
  light->ambient[3] = a;

}
void
sgLightSetSpecular4f(SGlight *light, float r, float g, float b, float a)
{
  light->specular[0] = r;
  light->specular[1] = g;
  light->specular[2] = b;
  light->specular[3] = a;

}

void
sgLightSetDiffuse4f(SGlight *light, float r, float g, float b, float a)
{
  light->diffuse[0] = r;
  light->diffuse[1] = g;
  light->diffuse[2] = b;
  light->diffuse[3] = a;
}



SGlight*
sgNewPointlight3f(OOscene *sc, float x, float y, float z)
{
  SGpointlight *light = malloc(sizeof(SGpointlight));
  light->super.enable = (SGenable_light_func)sgEnablePointLight;
  light->super.disable = (SGdisable_light_func)sgDisablePointLight;
  light->super.scene = sc;

  light->super.pos[0] = x;
  light->super.pos[1] = y;
  light->super.pos[2] = z;
  light->super.pos[3] = 1.0;

  light->super.ambient[0] = 0.0;
  light->super.ambient[1] = 0.0;
  light->super.ambient[2] = 0.0;
  light->super.ambient[3] = 1.0;

  light->super.specular[0] = 1.0;
  light->super.specular[1] = 1.0;
  light->super.specular[2] = 1.0;
  light->super.specular[3] = 1.0;

  light->super.diffuse[0] = 1.0;
  light->super.diffuse[1] = 1.0;
  light->super.diffuse[2] = 1.0;
  light->super.diffuse[3] = 1.0;

  sgSceneAddLight(sc, &light->super);

  return (SGlight*)light;
}

SGlight*
sgNewPointlight(OOscene *sc, float3 p)
{
  return sgNewPointlight3f(sc, vf3_x(p), vf3_y(p), vf3_z(p));
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
