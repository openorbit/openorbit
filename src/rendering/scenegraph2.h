/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#ifndef orbit_scenegraph2_h
#define orbit_scenegraph2_h

#include <OpenGL/gl3.h>
#include <gencds/array.h>
#include <vmath/vmath.h>

#define SG_OBJ_MAX_LIGHTS 4
#define SG_OBJ_MAX_TEXTURES 4

typedef struct SGwindow SGwindow;
typedef struct SGviewport SGviewport;
typedef struct SGscenegraph SGscenegraph;
typedef struct SGscene SGscene;

typedef struct SGcamera SGcamera;
typedef struct SGlight SGlight;
typedef struct SGspotlight SGspotlight;
typedef struct SGmaterial SGmaterial;
typedef struct SGshader SGshader;
typedef struct SGtexture SGtexture;
typedef struct SGgeometry SGgeometry;
typedef struct SGobject SGobject;
typedef struct SGbackground SGbackground;

typedef struct SGlightids SGlightids;
typedef struct SGmaterialids SGmaterialids;

struct SGcamera {
  float4x4 projMatrix;
  float4x4 viewMatrix;
};

struct SGlight{
  float4 pos;
  float4 ambient;
  float4 specular;
  float4 diffuse;
  float3 dir; // Only used for spotlights
  
  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;
  
  float4 globAmbient;
};


struct SGmaterial {
  float4 emission;
  float4 ambient;
  float4 diffuse;
  float4 specular;
  float shininess;
};

struct SGlightids {
  GLint pos;
  GLint ambient;
  GLint specular;
  GLint diffuse;
  GLint dir;

  GLint constantAttenuation;
  GLint linearAttenuation;
  GLint quadraticAttenuation;
  GLint globAmbient;
};

struct SGmaterialids {
  GLint emission;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint shininess;
};


struct SGshader {
  GLuint shaderId;
  GLuint modelViewId;
  GLuint projectionId;
  GLuint texIds[SG_OBJ_MAX_TEXTURES];
  SGlightids lightIds[SG_OBJ_MAX_LIGHTS];
  SGmaterialids materialId;
};

struct SGtexture {
  GLuint texId;
};

struct SGgeometry {
  int vertexCount;
  GLuint vbo;

  bool hasNormals;
  bool hasTexCoords;
  GLsizei normalOffset;
  GLsizei texCoordOffset;
};

struct SGobject {
  SGobject *parent;
  SGscene *scene;
  float3 pos;
  float3 dp; // delta pos per time step
  float3 dr; // delta rot per time step
  quaternion_t q; // Quaternion

  float4x4 R;
  float4x4 modelViewMatrix;

  size_t lightCount;
  SGlight *lights[SG_OBJ_MAX_LIGHTS];

  size_t texCount;
  GLuint textures[SG_OBJ_MAX_TEXTURES];
  
  SGmaterial material;
  
  SGshader *shader;
  SGgeometry *geometry;
  obj_array_t subObjects;
};

struct SGbackground {

};

struct SGscenegraph {
  obj_array_t scenes;
};

struct SGscene {
  SGscenegraph *sg;
  SGcamera *cam;
  SGbackground *bg;
  obj_array_t objects;
  obj_array_t lights;
  obj_array_t shaders;
};

struct SGviewport {
  SGscene *scene;
  obj_array_t overlays;
  unsigned x, y;
  unsigned w, h;
};

struct SGwindow {
  obj_array_t viewports;
  unsigned w, h;
};


/*!
  \param window The window to render
  \param dt The time elapsed since last render call, this is used by
    the animation method.
 */
void sgRenderWindow(SGwindow *window, float dt);
SGwindow* sgCreateWindow(void);
SGviewport* sgCreateViewport(SGwindow *window, unsigned x, unsigned y, unsigned w, unsigned h);
SGscene* sgCreateScene(void);

SGobject* sgCreateObject(SGscene *scene);
SGobject* sgCreateSubObject(SGobject *parent);

void sgObjectSetPos(SGobject *obj, float4 pos);
#endif
