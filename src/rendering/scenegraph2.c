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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "scenegraph2.h"
#include "shader-manager.h"

#include <OpenGL/gl3.h>


void
sgDrawGeometry(SGgeometry *geo)
{
  //  SG_CHECK_ERROR;
  
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  
  //sgBindMaterial(&mod->material);
  
  //glPushMatrix();
  glPushAttrib(GL_ENABLE_BIT);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  
  //glTranslatef(mod->trans[0], mod->trans[1], mod->trans[2]);
  //glMultMatrixf((GLfloat*)mod->rot);
  
  glBindBuffer(GL_ARRAY_BUFFER, geo->vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  if (geo->hasTexCoords) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  if (geo->hasNormals) glEnableClientState(GL_NORMAL_ARRAY);
  
  //GLfloat modelview[16], projection[16];
  //glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  //glGetFloatv(GL_PROJECTION_MATRIX, projection);
  
  //glUniformMatrix4fv(sgmod->super.modelview_id, 1, GL_FALSE, modelview);
  //glUniformMatrix4fv(sgmod->super.projection_id, 1, GL_FALSE, projection);
  
  glVertexPointer(3, GL_FLOAT, 0, 0);
  if (geo->hasTexCoords) glTexCoordPointer(2, GL_FLOAT, 0,
                                           (GLvoid*)geo->texCoordOffset);
  if (geo->hasNormals) glNormalPointer(GL_FLOAT, 0, (GLvoid*)geo->normalOffset);
  
  glDrawArrays(GL_TRIANGLES, 0, geo->vertexCount);

  glPopClientAttrib();
  glPopAttrib();
  
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //SG_CHECK_ERROR;

}

void
sgDrawObject(SGobject *obj)
{
  // Set model matrix for current object, we transpose this
  glUseProgram(obj->shader->shaderId);

  glUniformMatrix4fv(obj->shader->projectionId, 1, GL_TRUE,
                     (GLfloat*)obj->scene->cam->projMatrix);

  glUniformMatrix4fv(obj->shader->modelViewId, 1, GL_TRUE,
                     (GLfloat*)obj->modelViewMatrix);

  // Set light params for object
  for (int i = 0 ; i < obj->lightCount ; i ++) {
    glUniform3fv(obj->shader->lightIds[i].pos, 1,
                 (GLfloat*)&obj->lights[i]->pos);
    glUniform4fv(obj->shader->lightIds[i].ambient, 1,
                 (GLfloat*)&obj->lights[i]->ambient);
    glUniform4fv(obj->shader->lightIds[i].specular, 1,
                 (GLfloat*)&obj->lights[i]->specular);
    glUniform4fv(obj->shader->lightIds[i].diffuse, 1,
                 (GLfloat*)&obj->lights[i]->diffuse);
    glUniform3fv(obj->shader->lightIds[i].dir, 1,
                 (GLfloat*)&obj->lights[i]->dir);

    glUniform1f(obj->shader->lightIds[i].constantAttenuation,
                obj->lights[i]->constantAttenuation);
    glUniform1f(obj->shader->lightIds[i].linearAttenuation,
                obj->lights[i]->linearAttenuation);
    glUniform1f(obj->shader->lightIds[i].quadraticAttenuation,
                obj->lights[i]->quadraticAttenuation);

    glUniform4fv(obj->shader->lightIds[i].globAmbient, 1,
                 (GLfloat*)&obj->lights[i]->globAmbient);
  }

  // Set texture params
  for (int i = 0 ; i < obj->texCount ; i ++) {
    glUniform1i(obj->shader->texIds[i], obj->textures[i]);
  }
  //glUniform1iv(obj->shader->texArrayId, SG_OBJ_MAX_TEXTURES, obj->textures);
  // Set material params
  sgDrawGeometry(obj->geometry);
  glUseProgram(0);

  ARRAY_FOR_EACH(i, obj->subObjects) {
    sgDrawObject(ARRAY_ELEM(obj->subObjects, i));
  }
}

void
sgRecomputeModelViewMatrix(SGobject *obj)
{
  if (obj->parent) {
    mf4_cpy(obj->modelViewMatrix, obj->parent->modelViewMatrix);
  } else {
    mf4_cpy(obj->modelViewMatrix, obj->scene->cam->viewMatrix);
  }

  mf4_mul2(obj->modelViewMatrix, obj->R);
  float4x4 t;
  mf4_translate(t, obj->pos);
  mf4_mul2(obj->modelViewMatrix, t);

  ARRAY_FOR_EACH(i, obj->subObjects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(obj->subObjects, i));
  }
}

void
sgSetViewport(SGviewport *viewport)
{
  glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
}

void
sgAnimateObject(SGobject *obj, float dt)
{
  ARRAY_FOR_EACH(i, obj->subObjects) {
    sgAnimateObject(ARRAY_ELEM(obj->subObjects, i), dt);
  }
}

void
sgMoveCam(SGcamera *cam)
{

}

void
sgDrawBackground(SGbackground *bg)
{

}

void
sgRenderScene(SGscene *scene, float dt)
{
  sgDrawBackground(scene->bg);
  sgMoveCam(scene->cam);
  ARRAY_FOR_EACH(i, scene->objects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(scene->objects, i));
    sgAnimateObject(ARRAY_ELEM(scene->objects, i), dt);
    sgDrawObject(ARRAY_ELEM(scene->objects, i));
  }
}

void
sgRenderWindow(SGwindow *window, float dt)
{
  ARRAY_FOR_EACH(i, window->viewports) {
    sgSetViewport(ARRAY_ELEM(window->viewports, i));
    sgRenderScene(((SGviewport*)ARRAY_ELEM(window->viewports, i))->scene, dt);
  }
}



SGwindow*
sgCreateWindow(void)
{
  SGwindow *window = malloc(sizeof(SGwindow));
  memset(window, 0, sizeof(SGwindow));
  return window;
}

SGviewport*
sgCreateViewport(SGwindow *window, unsigned x, unsigned y,
                 unsigned w, unsigned h)
{
  SGviewport *viewport = malloc(sizeof(SGviewport));
  memset(viewport, 0, sizeof(SGviewport));
  return viewport;
}

SGscene*
sgCreateScene(void)
{
  SGscene *scene = malloc(sizeof(SGscene));
  memset(scene, 0, sizeof(SGscene));
  return scene;
}

SGobject*
sgCreateObject(SGscene *scene)
{
  SGobject *obj = malloc(sizeof(SGobject));
  memset(obj, 0, sizeof(SGobject));

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

SGobject*
sgCreateSubObject(SGobject *parent)
{
  SGobject *obj = malloc(sizeof(SGobject));
  memset(obj, 0, sizeof(SGobject));

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
sgObjectSetPos(SGobject *obj, float4 pos)
{
  obj->pos = pos;
}

SGgeometry*
sgCreateGeometry(SGobject *obj, size_t vertexCount,
                 float *vertices, float *normals, float *texCoords)
{
  SGgeometry *geo = malloc(sizeof(SGgeometry));
  memset(geo, 0, sizeof(SGgeometry));
  obj->geometry = geo;
  
  if (normals) geo->hasNormals = true;
  if (texCoords) geo->hasTexCoords = true;

  size_t vertexDataSize = sizeof(float) * vertexCount * 3;
  size_t normalDataSize = normals ? sizeof(float) * vertexCount * 3 : 0;
  size_t texCoordDataSize = texCoords ? sizeof(float) * vertexCount * 2 : 0;
  size_t buffSize = vertexDataSize + normalDataSize + texCoordDataSize;

  glGenBuffers(1, &geo->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, geo->vbo);
  glBufferData(GL_ARRAY_BUFFER,
               buffSize,
               NULL, // Just allocate, will copy with subdata
               GL_STATIC_DRAW);
  
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataSize, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize, normalDataSize, normals);
  glBufferSubData(GL_ARRAY_BUFFER, vertexDataSize + normalDataSize,
                  texCoordDataSize, texCoords);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return geo;
}

/*
  Uses the shader manager to load the shader and then binds the variables
  we have standardized on. This funciton will be simplified if OS X ever
  supports the explicit attrib loc extensions.
 */
void
sgObjectLoadShader(SGobject *obj, const char *name)
{
  SGshader *shader = malloc(sizeof(SGshader));
  memset(shader, 0, sizeof(SGshader));
  obj->shader = shader;

  shader->shaderId = sgLoadProgram(name, name, name, NULL);
  
  shader->projectionId = sgGetLocationForParam(shader->shaderId, SG_PROJECTION);
  shader->modelViewId = sgGetLocationForParam(shader->shaderId, SG_MODELVIEW);
  
  for (int i = 0 ; i < SG_OBJ_MAX_LIGHTS ; i++) {
    shader->lightIds[i].pos =
    sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_POS, i);
    shader->lightIds[i].ambient =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_AMB, i);
    shader->lightIds[i].specular =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_SPEC, i);
    shader->lightIds[i].diffuse =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_DIFF, i);
    shader->lightIds[i].dir =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_DIR, i);
    
    shader->lightIds[i].constantAttenuation =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_CONST_ATTEN, i);
    shader->lightIds[i].linearAttenuation =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_LINEAR_ATTEN, i);
    shader->lightIds[i].quadraticAttenuation =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_QUAD_ATTEN, i);
    shader->lightIds[i].globAmbient =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_LIGHT_MOD_GLOB_AMB, i);
  }

  for (int i = 0 ; i < SG_OBJ_MAX_TEXTURES ; i++) {
    shader->texIds[i] =
      sgGetLocationForParamAndIndex(shader->shaderId, SG_TEX, i);
  }

  shader->materialId.emission =
    sgGetLocationForParam(shader->shaderId, SG_MATERIAL_EMIT);
  shader->materialId.ambient =
    sgGetLocationForParam(shader->shaderId, SG_MATERIAL_AMB);
  shader->materialId.diffuse =
    sgGetLocationForParam(shader->shaderId, SG_MATERIAL_DIFF);
  shader->materialId.specular =
    sgGetLocationForParam(shader->shaderId, SG_MATERIAL_SPEC);
  shader->materialId.shininess =
    sgGetLocationForParam(shader->shaderId, SG_MATERIAL_SHINE);
  
  //void sgSetShaderTex(GLuint program, sg_param_id_t param, GLuint tex);
  
  obj->shader = NULL;
}
