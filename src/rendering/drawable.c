/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include <string.h>
#include <assert.h>

#include "rendering/scenegraph-private.h"
#include "drawable.h"
#include "common/lwcoord.h"
#include "parsers/model.h"

void
sgSetObjectPosLWAndOffset(SGdrawable *obj, const OOlwcoord *lw, float3 offset)
{
  assert(obj != NULL);
  assert(lw != NULL);
  // Get camera position and translate the lw coord with respect to the camera
  SGscene *sc = obj->scene;
  SGscenegraph *sg = sc->sg;
  SGcam *cam = sg->currentCam;

  if (cam->kind == SGCam_Free) {
    float3 relPos = ooLwcDist(lw, &((SGfreecam*)cam)->lwc);
    obj->p = relPos + offset;
  } else if (cam->kind == SGCam_Fixed) {
    SGfixedcam *fix = (SGfixedcam*)cam;
    float3 relPos = ooLwcDist(lw, &fix->body->p) - (mf3_v_mul(fix->body->R, fix->r));
    obj->p = relPos + offset;
  } else if (cam->kind == SGCam_Orbit) {
    SGorbitcam *orb = (SGorbitcam*)cam;
    float3 relPos = ooLwcDist(lw, &orb->body->p);
    obj->p = relPos + offset;
  }
}

void
sgSetObjectPosLW(SGdrawable *obj, const OOlwcoord *lw)
{
  // Get camera position and translate the lw coord with respect to the camera
  SGscene *sc = obj->scene;
  SGscenegraph *sg = sc->sg;
  SGcam *cam = sg->currentCam;

  if (cam->kind == SGCam_Free) {
    float3 relPos = ooLwcDist(lw, &((SGfreecam*)cam)->lwc);
    obj->p = relPos;
  } else if (cam->kind == SGCam_Fixed) {
    SGfixedcam *fix = (SGfixedcam*)cam;
    float3 relPos = ooLwcDist(lw, &fix->body->p) - (mf3_v_mul(fix->body->R, fix->r));
    obj->p = relPos;
  } else if (cam->kind == SGCam_Orbit) {
    SGorbitcam *orb = (SGorbitcam*)cam;
    float3 relPos = ooLwcDist(lw, &orb->body->p);
    obj->p = relPos;
  }
}
void
sgSetObjectPos3f(SGdrawable *obj, float x, float y, float z)
{
  assert(obj != NULL);
  obj->p = vf3_set(x, y, z);
}

void
sgSetObjectQuat4f(SGdrawable *obj, float x, float y, float z, float w)
{
  assert(obj != NULL);
  obj->q = vf4_set(x, y, z, w);

  float3x3 m, mt;
  q_mf3_convert(m, obj->q);
  mf3_transpose2(mt, m); // Transpose matrix for OpenGL

  bzero(obj->R, sizeof(GLfloat[16]));

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      obj->R[i*4+j] = mt[i][j];
    }
  }
  obj->R[15] = 1.0f;
}

void
sgSetObjectQuatv(SGdrawable *obj, quaternion_t q)
{
  assert(obj != NULL);
  obj->q = q;

  float3x3 m, mt;
  q_mf3_convert(m, q);
  mf3_transpose2(mt, m); // Transpose matrix for OpenGL

  bzero(obj->R, sizeof(GLfloat[16]));

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      obj->R[i*4+j] = mt[i][j];
    }
  }
  obj->R[15] = 1.0f;
}

void
sgSetObjectScale(SGdrawable *obj, float s)
{
  assert(0);
  assert(obj != NULL);
  //  obj->s = s;
}

void
sgSetObjectSpeed(SGdrawable *obj, float dx, float dy, float dz)
{
  assert(obj != NULL);
  obj->dp = vf3_set(dx, dy, dz);
}

void
sgSetObjectAngularSpeed(SGdrawable *obj, float drx, float dry, float drz)
{
  assert(obj != NULL);
  obj->dr = vf3_set(drx, dry, drz);
}

SGdrawable*
sgNewDrawable(SGdrawable *drawable, const char *name, SGdrawfunc df)
{
  assert(df != NULL);
  assert(drawable != NULL);

  drawable->name = strdup(name);
  drawable->p = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->q = q_rot(1.0f, 0.0f, 0.0f, 0.0f);
  drawable->dr = vf3_set(0.0f, 0.0f, 0.0f);
  drawable->dp = vf3_set(0.0f, 0.0f, 0.0f);

  bzero(drawable->R, sizeof(GLfloat[16]));
  drawable->R[0] = 1.0f;
  drawable->R[5] = 1.0f;
  drawable->R[10] = 1.0f;
  drawable->R[15] = 1.0f;
  drawable->draw = df;

  drawable->scene = NULL;
  drawable->enabled = true;

  LIST_FIRST(drawable->children) = NULL;
  LIST_LAST(drawable->children) = NULL;
  drawable->siblings.next = NULL;
  drawable->siblings.prev = NULL;
  drawable->parent = NULL;
  return drawable;
}

void
sgDrawSphere(SGsphere *sp)
{
  glEnable(GL_TEXTURE_2D);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  sgBindMaterial(&sp->mat);
  glBindTexture(GL_TEXTURE_2D, sp->texId);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glColor3f(1.0f, 1.0f, 1.0f);

  // Primitive LOD, note that distances may be messed up for smaller objects, as
  // this does not take the camera position into account.
  double angularDiameter = 2.0 * atan(0.5*sp->radius/vf3_abs(sp->super.p));
  //double angularDiameter = sp->radius/vf3_abs(sp->super.p);// Quicker estimate
  if (angularDiameter > DEG_TO_RAD(5.0)) {
    gluSphere(sp->quadratic, sp->radius, 128, 128);
  } else if (angularDiameter > DEG_TO_RAD(1.0)) {
    gluSphere(sp->quadratic, sp->radius, 32, 32);
  } else {
    gluSphere(sp->quadratic, sp->radius, 16, 16);
  }
  // Draw point on the sphere in solid colour and size
  glDisable (GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D); // Lines are not textured...

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, -2.0*sp->radius);
  glVertex3f(0.0, 0.0, -sp->radius);
  glEnd();

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, +2.0*sp->radius);
  glVertex3f(0.0, 0.0, sp->radius);
  glEnd();

  glBegin(GL_POINTS);
  glPointSize(5.0);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glEnd();
}

SGdrawable*
sgNewSphere(const char *name, float radius, const char *tex)
{
  SGsphere *sp = malloc(sizeof(SGsphere));
  sgInitMaterial(&sp->mat);

  sp->radius = radius;
  ooTexLoad(tex, tex);
  sp->texId = ooTexNum(tex);
  sp->quadratic = gluNewQuadric();
  gluQuadricOrientation(sp->quadratic, GLU_OUTSIDE);
  gluQuadricNormals(sp->quadratic, GLU_SMOOTH);
  gluQuadricTexture(sp->quadratic, GL_TRUE);
  gluQuadricDrawStyle(sp->quadratic, GLU_FILL);

  return sgNewDrawable((SGdrawable*)sp, name, (SGdrawfunc) sgDrawSphere);
}


SGdrawable*
sgNewSphere2(const char *name, const char *tex, float radius, float flattening, int stacks, int slices)
{
  SGsphere *sp = malloc(sizeof(SGsphere));
  sgInitMaterial(&sp->mat);

  sp->radius = radius;
  ooTexLoad(tex, tex);
  sp->texId = ooTexNum(tex);

  size_t vertexCount = (stacks * slices - 2) * 2 + 2 * (slices + 1); // For the triangle strips and the poles 

  sp->northPoleIdx = 0;
  sp->northPoleCount = slices + 1;
  sp->southPoleIdx = sp->northPoleCount;
  sp->southPoleCount = slices + 1;
  sp->sliceCount = slices;
  sp->stackCount = stacks;

  sp->vertices = calloc(vertexCount*3, sizeof(GLfloat));
  sp->normals = calloc(vertexCount*3, sizeof(GLfloat));

  return sgNewDrawable((SGdrawable*)sp, name, (SGdrawfunc) sgDrawSphere);
}

void
sgDrawSphere2(SGsphere *sp)
{
  glEnable(GL_TEXTURE_2D); // Lines are not textured...
  glBindTexture(GL_TEXTURE_2D, sp->texId);
  glEnable(GL_LIGHTING); // Lines are not lit, just colored
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  glVertexPointer(3, GL_FLOAT, 0, &sp->vertices[sp->northPoleIdx]);
  glNormalPointer(GL_FLOAT, 0, &sp->normals[sp->northPoleIdx]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, sp->sliceCount+1);

  glVertexPointer(3, GL_FLOAT, 0, &sp->vertices[sp->southPoleIdx]);
  glNormalPointer(GL_FLOAT, 0, &sp->normals[sp->southPoleIdx]);
  glDrawArrays(GL_TRIANGLE_FAN, 0, sp->sliceCount+1);

  for (int i = 0 ; i < sp->stackCount - 2 ; ++ i) {
    glVertexPointer(3, GL_FLOAT, 0, &sp->vertices[sp->stripIdx + i * sp->sliceCount * 2]);
    glNormalPointer(GL_FLOAT, 0, &sp->normals[sp->southPoleIdx]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sp->sliceCount * 2);    
  }

  glDisableClientState(GL_VERTEX_ARRAY);  
  glDisableClientState(GL_NORMAL_ARRAY);
  
}

SGmaterial*
sgSphereGetMaterial(SGsphere *sphere)
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

  return sgNewDrawable((SGdrawable*)el, name, (SGdrawfunc)sgDrawEllipsis);
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

  return sgNewDrawable((SGdrawable*)cyl, name, (SGdrawfunc)sgDrawCylinder);
}


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

  return sgNewDrawable((SGdrawable*)model, "unnamed",
                       (SGdrawfunc)sgDrawModel);
}

void
sgDrawableAddChild(SGdrawable * restrict parent, SGdrawable * restrict child,
                   float3 t, quaternion_t q)
{
  LIST_APPEND(parent->children, child, siblings);

  child->p = t;
  child->q = q;
  child->parent = parent;

  float3x3 m, mt;
  q_mf3_convert(m, q);
  mf3_transpose2(mt, m); // Transpose matrix for OpenGL

  bzero(child->R, sizeof(GLfloat[16]));

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      child->R[i*4+j] = mt[i][j];
    }
  }
  child->R[15] = 1.0f;
}
void
sgDrawableLiberate(SGdrawable *drawable)
{
  assert(drawable->parent != NULL); // Makes no sense to liberate roots
}

void
sgPaintDrawable(SGdrawable *drawable)
{
  glPushMatrix();
  glTranslatef(drawable->p.x, drawable->p.y, drawable->p.z);
  glMultMatrixf(drawable->R);
  drawable->draw(drawable);
  LIST_FOREACH(SGdrawable, child, drawable->children, siblings) {
    sgPaintDrawable(child);
  }

  glPopMatrix();
}