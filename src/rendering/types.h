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

#ifndef orbit_types_h
#define orbit_types_h

#include <vmath/vmath.h>
#include <vmath/lwcoord.h>
#include <gencds/array.h>
#include "physics/reftypes.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>
#endif

#define SG_OBJ_MAX_TEXTURES 4
#define SG_OBJ_MAX_LIGHTS 4

typedef struct sg_object_t sg_object_t;
typedef struct sg_overlay_t sg_overlay_t;
typedef void (*sg_draw_overlay_t)(sg_overlay_t *);

struct sg_overlay_t {
  bool enabled;
  float x, y;
  float w, h;
  GLuint fbo;
  GLuint tex;
  sg_draw_overlay_t draw;
};

typedef struct {
  unsigned w;
  unsigned h;
  float aspect;
  float fovy;
  unsigned gl_major_vers;
  unsigned gl_minor_vers;
  unsigned glsl_major_vers;
  unsigned glsl_minor_vers;
} sg_render_info_t;


typedef enum {
  SG_CAMERA_FREE, // Free moving camera
  SG_CAMERA_FIXED, // Fixed at location relative to obj, local rotation allowed
  SG_CAMERA_ORBITING // Rotating around target object
} sg_camera_type_t;

typedef struct {
  float4x4 proj_matrix;
  float4x4 view_matrix;

  sg_camera_type_t type;

  union {
    struct {
      lwcoord_t lwc;
      float3 dp;
      quaternion_t q;
      quaternion_t dq;
    } free;
    struct {
      sg_object_t *obj;
      float3 r; // With this offset
      quaternion_t q; // and this rotation (rotate before translation)
      quaternion_t dq; // Delta rotation
    } fixed;
    struct {
      sg_object_t *obj;
      float ra, dec;
      float dra, ddec, dr;
      float r, zoom;
    } orbiting;
  };
} sg_camera_t;


typedef struct {
  GLint pos;
  GLint ambient;
  GLint specular;
  GLint diffuse;
  GLint dir;

  GLint constantAttenuation;
  GLint linearAttenuation;
  GLint quadraticAttenuation;
  GLint globAmbient;
} sg_light_ids_t;

typedef struct {
  GLint emission;
  GLint ambient;
  GLint diffuse;
  GLint specular;
  GLint shininess;
} sg_material_ids_t;


typedef struct  {
  GLuint shaderId;

  // Uniform IDs
  struct {
    GLuint modelViewId;
    GLuint projectionId;
    GLuint normalMatrixId;
    GLuint texIds[SG_OBJ_MAX_TEXTURES];
    sg_light_ids_t lightIds[SG_OBJ_MAX_LIGHTS];
    sg_material_ids_t materialId;
  } uniforms;
  // Attribute IDs
  struct {
    GLuint vertexId;
    GLuint normalId;
    GLuint colourId;
    GLuint texCoordId[SG_OBJ_MAX_TEXTURES];
  } attribs;
} sg_shader_t;

typedef struct {
  obj_array_t scenes;
} sg_scenegraph_t;

typedef struct {
  unsigned char r, g, b, a;
  float x, y, z;
} sg_star_t;

typedef struct {
  sg_camera_t *cam;
  sg_shader_t *shader;

  GLuint vbo;
  GLuint vba;

  size_t a_len;
  size_t n_stars;
  sg_star_t *data;
} sg_background_t;

typedef struct {
  sg_scenegraph_t *sg;
  sg_camera_t *cam;
  sg_background_t *bg;
  obj_array_t objects;
  obj_array_t lights;
  obj_array_t shaders;
} sg_scene_t;

typedef struct {
  sg_scene_t *scene;
  obj_array_t overlays;
  unsigned x, y;
  unsigned w, h;
} sg_viewport_t;


typedef struct {
  obj_array_t viewports;
  unsigned w, h;
} sg_window_t;



typedef GLuint sg_texture_ref_t;

typedef struct {
  float4 emission;
  float4 ambient;
  float4 diffuse;
  float4 specular;
  float shininess;
} sg_material_t;

typedef struct {
  sg_scene_t *scene;
  int lightId;

  float4 pos;
  float4 ambient;
  float4 specular;
  float4 diffuse;
  float3 dir; // Only used for spotlights

  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  float4 globAmbient;
} sg_light_t;


typedef struct {
  sg_object_t *obj;
  int vertexCount;

  GLuint vba;
  GLuint vbo;

  bool hasNormals;
  bool hasTexCoords;
  GLsizei normalOffset;
  GLsizei texCoordOffset;
} sg_geometry_t;


struct sg_object_t {
  struct sg_object_t *parent;
  sg_scene_t *scene;

  PLobject *rigidBody;
  float3 pos;
  float3 dp; // delta pos per time step
  float3 dr; // delta rot per time step
  quaternion_t q; // Quaternion

  float4x4 R;
  float4x4 modelViewMatrix;

  size_t lightCount;
  sg_light_t *lights[SG_OBJ_MAX_LIGHTS];

  size_t texCount;
  GLuint textures[SG_OBJ_MAX_TEXTURES];

  sg_material_t material;

  sg_shader_t *shader;
  sg_geometry_t *geometry;
  obj_array_t subObjects;
};


#endif
