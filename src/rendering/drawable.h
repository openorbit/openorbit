/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SG_DRAWABLE_H
#define SG_DRAWABLE_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
//#include <OpenGL/glu.h>
#else
#include <GL3/gl3.h>
//#include <GL3/glu.h>
#endif


#include <vmath/vmath.h>
#include "rendering/reftypes.h"
#include "rendering/light.h"
#include "parsers/model.h"
#include "rendering/material.h"
#include "rendering/shader-manager.h"
#include "common/lwcoord.h"
#include <gencds/list.h>

struct SGdrawable {
  LIST_ENTRY(SGdrawable) siblings;
  LIST_HEAD(SGdrawable) children;
  struct SGdrawable *parent;
  const char *name;
  SGscene *scene;
  bool enabled;
  float3 p;  // Position
  float3 dp; // delta pos per time step
  float3 dr; // delta rot per time step
  quaternion_t q; // Quaternion
                  //float s; // Scale
  GLfloat R[16];
  SGdrawfunc draw; // Draw function for this object

  SGlight *lights[8];
  SGshader *shader;
  GLint modelview_id;
  GLint projection_id;
  GLint tex_id[4];
  GLint tex_uni_id[4];
};

typedef struct SGellipsis {
  SGdrawable super;
  double semiMajor;
  double semiMinor;
  double ecc;
  float colour[3];
  size_t vertCount;
  float verts[];
} SGellipsis;

// The ellipsoid is drawn half
typedef struct SGellipsoid {
  SGdrawable super;
  float a;
  float b;
  float c;
  float colour[4];
  size_t vertCountPerStack;
  size_t triCountPerStack;
  size_t stacks;
  size_t slices;
  float verts[];

} SGellipsoid;

typedef struct SGmodeldata {
  GLint tex_id;
  int vertex_count;
  GLuint vbo;
  SGlight *lights[8];

  bool has_normals;
  bool has_tex_coords;
  GLsizei normal_offset;
  GLsizei tex_coord_offset;

  SGmaterial material;
  GLfloat trans[4];
  GLfloat rot[4][4];

  obj_array_t children;
} SGmodeldata;

typedef struct SGmodel2 {
  SGdrawable super;
  obj_array_t roots;
} SGmodel2;

struct SGsphere {
  SGdrawable super;
  SGmaterial mat;

  GLint use_night_tex_id;
  GLboolean use_night_tex_val;
  GLint use_spec_map_id;
  GLboolean use_spec_map_val;

  //  GLUquadricObj *quadratic;

  GLuint northPoleIdx;
  GLuint northPoleCount;
  GLuint southPoleIdx;
  GLuint southPoleCount;

  GLuint sliceCount;
  GLuint stackCount;
  GLuint stripIdx;

  GLfloat *vertices;
  GLfloat *normals;
  GLfloat *texCoords;

  GLfloat radius;
};


struct SGcylinder {
  SGdrawable super;
  //  GLuint texId;
  GLbyte col[3];
  //GLUquadricObj *quadratic;
  GLfloat bottonRadius;
  GLfloat topRadius;
  GLfloat height;
};

typedef struct SGvector {
  SGdrawable super;
  float3 v;
  float col[3];
} SGvector;

#define SG_LABEL_LEN 128
typedef struct SGlabel {
  SGdrawable super;
  float col[3];
  char buff[SG_LABEL_LEN];
} SGlabel;

typedef struct SGdisc {
  SGdrawable super;
  float inner_rad, outer_rad;
  GLfloat *vertices;
} SGdisc;


SGdrawable* sgNewDrawable(SGdrawable *drawable, const char *name, SGdrawfunc df);
void sgDrawableLoadShader(SGdrawable *obj, const char *shader);

void sgSetObjectQuat4f(SGdrawable *obj, float x, float y, float z, float w);
void sgSetObjectQuatv(SGdrawable *obj, quaternion_t q);
void sgSetObjectPosLWAndOffset(SGdrawable *obj, const OOlwcoord *lw, float3 offset);
void sgSetObjectPosLW(SGdrawable *obj, const OOlwcoord *lw);
void sgSetObjectPos3f(SGdrawable *obj, float x, float y, float z);
//void ooSgSetObjectScale(SGdrawable *obj, float s);
void sgSetObjectSpeed(SGdrawable *obj, float dx, float dy, float dz);
void sgSetObjectAngularSpeed(SGdrawable *obj, float drx, float dry, float drz);

SGmaterial* sgSphereGetMaterial(SGsphere *sphere);

SGdrawable* sgNewSphere(const char *name, const char* shader, float radius,
                        const char *tex, const char *nightTex, const char *specMap,
                        SGmaterial *mat);

SGdrawable* sgNewEllipsis(const char *name,
                          double semiMajor, double semiMinor,
                          double longAsc, double inc, double argPeri,
                          float r, float g, float b,
                          size_t vertCount);

SGdrawable* sgLoadModel(const char *file);

void sgDrawableAddChild(SGdrawable * restrict parent, SGdrawable * restrict child,
                        float3 t, quaternion_t q);
void sgPaintDrawable(SGdrawable *drawable);

SGdrawable* sgNewVector(const char *name, float3 vec, float r, float g, float b);

SGdrawable* sgNewEllipsoid(const char *name, float a, float b, float c,
                           float red, float green, float blue, float alpha,
                           unsigned slices, unsigned stacks);


#endif /* !SG_DRAWABLE_H */
