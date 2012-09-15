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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>
#endif

#include "rendering/object.h"
#include "rendering/shader-manager.h"
#include "physics/physics.h"
#include <openorbit/log.h>

#include "3ds.h"
#include "ac3d.h"
#include "cmod.h"
#include "collada.h"

struct sg_geometry_t {
  sg_object_t *obj;
  int gl_primitive_type;
  int vertexCount;

  GLuint vba;
  GLuint vbo;

  bool hasNormals;
  bool hasTexCoords;
  GLsizei normalOffset;
  GLsizei texCoordOffset;
};

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
  sg_texture_t *textures[SG_OBJ_MAX_TEXTURES];

  sg_material_t *material;

  sg_shader_t *shader;
  sg_geometry_t *geometry;
  obj_array_t subObjects;
};

void
sg_object_add_child(sg_object_t *obj, sg_object_t *child)
{
  obj_array_push(&obj->subObjects, child);
}
int
sg_objects_compare_dist(sg_object_t const **o0, sg_object_t const **o1)
{
  bool gt = vf3_gt((*o0)->pos, (*o1)->pos);

  if (gt) return -1;
  else return 1;
}

float3
sg_object_get_pos(sg_object_t *obj)
{
  return obj->pos;
}

void
sg_object_set_pos(sg_object_t *obj, float3 pos)
{
  obj->pos = pos;
}

quaternion_t
sg_object_get_quat(sg_object_t *obj)
{
  return obj->q;
}
void
sg_object_set_quat(sg_object_t *obj, quaternion_t q)
{
  obj->q = q;
}

void
sg_object_set_rot(sg_object_t *obj, const float4x4 *r)
{
  memcpy(obj->R, *r, sizeof(float4x4));
}

void
sg_object_set_material(sg_object_t *obj, sg_material_t *mat)
{
  obj->material = mat;
}

void
sg_geometry_draw(sg_geometry_t *geo)
{
  //  SG_CHECK_ERROR;

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);

  glBindBuffer(GL_ARRAY_BUFFER, geo->vbo);
  glBindVertexArray(geo->vba);
  glEnableVertexAttribArray(geo->vba);

  glDrawArrays(GL_TRIANGLES, 0, geo->vertexCount);

  glDisableVertexAttribArray(geo->vba);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //SG_CHECK_ERROR;
}

void
sg_object_draw(sg_object_t *obj)
{
  // Set model matrix for current object, we transpose this
  sg_shader_bind(obj->shader);

  sg_shader_set_projection(obj->shader,
                           *sg_camera_get_projection(sg_scene_get_cam(obj->scene)));
  //glUniformMatrix4fv(obj->shader->uniforms.projectionId, 1, GL_TRUE,
  //                   (GLfloat*)obj->scene->cam->proj_matrix);
  sg_shader_set_model_view(obj->shader,
                           *sg_camera_get_view(sg_scene_get_cam(obj->scene)));

  //glUniformMatrix4fv(obj->shader->uniforms.modelViewId, 1, GL_TRUE,
  //                   (GLfloat*)obj->modelViewMatrix);

  // Set light params for object
  // TODO: Global ambient light as well...
  for (int i = 0 ; i < obj->lightCount ; i ++) {
    sg_light_bind(obj->lights[i], obj->shader);
  }

  // Set texture params
  for (int i = 0 ; i < obj->texCount ; i ++) {
    sg_texture_bind(obj->textures[i], obj->shader);
  }
  //glUniform1iv(obj->shader->texArrayId, SG_OBJ_MAX_TEXTURES, obj->textures);
  // Set material params
  sg_geometry_draw(obj->geometry);
  sg_shader_bind(NULL);

  ARRAY_FOR_EACH(i, obj->subObjects) {
    sg_object_draw(ARRAY_ELEM(obj->subObjects, i));
  }
}

void
sgRecomputeModelViewMatrix(sg_object_t *obj)
{
  if (obj->parent) {
    mf4_cpy(obj->modelViewMatrix, obj->parent->modelViewMatrix);
  } else {
    sg_camera_t *cam = sg_scene_get_cam(obj->scene);
    mf4_cpy(obj->modelViewMatrix, *sg_camera_get_view(cam));
  }

  mf4_mul2(obj->modelViewMatrix, obj->R);
  float4x4 t;
  mf4_make_translate(t, obj->pos);
  mf4_mul2(obj->modelViewMatrix, t);

  ARRAY_FOR_EACH(i, obj->subObjects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(obj->subObjects, i));
  }
}

void
sg_object_animate(sg_object_t *obj, float dt)
{
  obj->q = q_normalise(q_vf3_rot(obj->q, obj->dr, dt));
  q_mf3_convert(obj->R, obj->q);
  obj->pos += obj->dp;

  ARRAY_FOR_EACH(i, obj->subObjects) {
    sg_object_animate(ARRAY_ELEM(obj->subObjects, i), dt);
  }
}

void
sg_object_update(sg_object_t *obj)
{
  if (obj->rigidBody) {
    obj->dp = plGetVel(obj->rigidBody);
    obj->dr = plGetAngularVel(obj->rigidBody);
    obj->q = plGetQuat(obj->rigidBody);
  }

  //ARRAY_FOR_EACH(i, obj->subObjects) {
  //  sg_object_update(ARRAY_ELEM(obj->subObjects, i));
  //}
}



sg_object_t*
sgCreateObject(sg_scene_t *scene)
{
  sg_object_t *obj = malloc(sizeof(sg_object_t));
  memset(obj, 0, sizeof(sg_object_t));

  obj->parent = NULL;
  obj->scene = scene;

  obj->pos = vf3_set(0.0, 0.0, 0.0);
  obj->dp = vf3_set(0.0, 0.0, 0.0); // delta pos per time step
  obj->dr = vf3_set(0.0, 0.0, 0.0); // delta rot per time step
  obj->q = q_rot(1.0, 0.0, 0.0, 0.0); // Quaternion

  mf4_ident(obj->R);
  mf4_ident(obj->modelViewMatrix);

  for (int i = 0 ; i < SG_OBJ_MAX_LIGHTS ; i++) {
    obj->lights[i] = NULL;
  }

  obj->shader = NULL;
  obj->geometry = NULL;

  obj_array_init(&obj->subObjects);

  return obj;
}

sg_object_t*
sgCreateSubObject(sg_object_t *parent)
{
  sg_object_t *obj = malloc(sizeof(sg_object_t));
  memset(obj, 0, sizeof(sg_object_t));

  obj->parent = parent;
  obj->scene = parent->scene;

  obj->pos = vf3_set(0.0, 0.0, 0.0);
  obj->dp = vf3_set(0.0, 0.0, 0.0); // delta pos per time step
  obj->dr = vf3_set(0.0, 0.0, 0.0); // delta rot per time step
  obj->q = q_rot(1.0, 0.0, 0.0, 0.0); // Quaternion

  mf4_ident(obj->R);
  mf4_ident(obj->modelViewMatrix);

  for (int i = 0 ; i < SG_OBJ_MAX_LIGHTS ; i++) {
    obj->lights[i] = NULL;
  }

  obj->shader = NULL;
  obj->geometry = NULL;

  obj_array_init(&obj->subObjects);

  return obj;
}

void
sgObjectSetPos(sg_object_t *obj, float4 pos)
{
  obj->pos = pos;
}

sg_geometry_t*
sg_new_geometry(sg_object_t *obj, int gl_primitive, size_t vertexCount,
                float *vertices, float *normals, float *texCoords)
{
  sg_geometry_t *geo = malloc(sizeof(sg_geometry_t));
  memset(geo, 0, sizeof(sg_geometry_t));
  obj->geometry = geo;
  geo->gl_primitive_type = gl_primitive;
  if (normals) geo->hasNormals = true;
  if (texCoords) geo->hasTexCoords = true;

  size_t vertexDataSize = sizeof(float) * vertexCount * 3;
  size_t normalDataSize = normals ? sizeof(float) * vertexCount * 3 : 0;
  size_t texCoordDataSize = texCoords ? sizeof(float) * vertexCount * 2 : 0;
  size_t buffSize = vertexDataSize + normalDataSize + texCoordDataSize;

  glGenVertexArrays(1, &geo->vba);
  glBindVertexArray(geo->vba);
  glGenBuffers(1, &geo->vbo);

  glBindBuffer(GL_ARRAY_BUFFER, geo->vbo);
  glBufferData(GL_ARRAY_BUFFER,
               buffSize,
               NULL, // Just allocate, will copy with subdata
               GL_STATIC_DRAW);



  glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, vertices);

  sg_shader_t *shader = obj->shader;

  glVertexAttribPointer(sg_shader_get_vertex_attrib(shader),
                        4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(sg_shader_get_vertex_attrib(shader));
  if (normals) {
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normals);
    glVertexAttribPointer(sg_shader_get_normal_attrib(shader),
                          3, GL_FLOAT, GL_FALSE, 0, (void*)vertexDataSize);
    glEnableVertexAttribArray(sg_shader_get_normal_attrib(shader));
  }

  if (texCoords) {
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize,
                    texCoordDataSize, texCoords);
    glVertexAttribPointer(sg_shader_get_texcoord_attrib(shader, 0),
                          2, GL_FLOAT, GL_FALSE, 0,
                          (void*)vertexDataSize + normalDataSize);
    glEnableVertexAttribArray(sg_shader_get_texcoord_attrib(shader, 0));
  }

  glBindVertexArray(0); // Done

  return geo;
}

/*
  Uses the shader manager to load the shader and then binds the variables
  we have standardized on. This funciton will be simplified if OS X ever
  supports the explicit attrib loc extensions.
 */
void
sg_object_load_shader(sg_object_t *obj, const char *name)
{
  obj->shader = sg_load_shader(name, name, name, NULL);
}

void
sg_object_set_rigid_body(sg_object_t *obj, PLobject *rigidBody)
{
  if (obj->parent) {
    ooLogWarn("setting rigid body for sg object that is not root");
    return;
  }
  obj->rigidBody = rigidBody;
}
PLobject*
sg_object_get_rigid_body(const sg_object_t *obj)
{
  return obj->rigidBody;
}

sg_object_t*
sg_new_object(void)
{
  sg_object_t *obj = malloc(sizeof(sg_object_t));
  memset(obj, 0, sizeof(sg_object_t));
  return obj;
}

void
sg_object_set_geo(sg_object_t *obj, int gl_primitive, size_t vertexCount,
                  float *vertices, float *normals, float *texCoords)
{
  obj->geometry = sg_new_geometry(obj, gl_primitive,
                                  vertexCount, vertices, normals, texCoords);
}


sg_object_t*
sg_new_object_with_geo(int gl_primitive, size_t vertexCount,
                       float *vertices, float *normals, float *texCoords)
{
  sg_object_t *obj = malloc(sizeof(sg_object_t));
  memset(obj, 0, sizeof(sg_object_t));
  obj->geometry = sg_new_geometry(obj, gl_primitive,
                                  vertexCount, vertices, normals, texCoords);
}

sg_object_t*
sg_load_object(const char *file)
{
  assert(file && "not null");

  sg_object_t *model = NULL;

  char *dot = strrchr(file, '.');
  if (dot) {
    char *endptr = strchr(dot, '\0');
    assert(endptr != NULL);

    // Dispatch based on file type
    if (!strcmp(dot, ".hrml")) {

    } else if (!strcmp(dot, ".3ds")) {

    } else if (!strcmp(dot, ".dae")) {
      model = collada_load(file);
    } else if (!strcmp(dot, ".ac")) {
      model = ac3d_load(file);
    }
  }
  return model;
}

sg_object_t*
sg_new_sphere(const char *name, sg_shader_t *shader, float radius,
              sg_texture_t *tex, sg_texture_t *nightTex, sg_texture_t *spec,
              sg_material_t *mat)
{
  return NULL;
}

sg_object_t*
sg_new_ellipse(const char *name, float semiMajor,
               float semiMinor, float asc,
               float inc, float argOfPeriapsis,
               float dec, float ra, int segments)
{
  return NULL;
}

sg_object_t*
sg_new_axises(const char *name, float length)
{
  float axis[] = {0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
                  0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f};
  sg_object_t *obj = sg_new_object_with_geo(GL_LINE, 6, axis, NULL, NULL);
  return obj;
}

void
sg_object_set_shader(sg_object_t *obj, sg_shader_t *shader)
{
  obj->shader = shader;
}

void
sg_object_set_shader_by_name(sg_object_t *obj, const char *shname)
{
  obj->shader = sg_get_shader(shname);
}
