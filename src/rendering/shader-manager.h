/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef RENDERING_SHADER_MANAGER_H
#define RENDERING_SHADER_MANAGER_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/gl3drawable.h"
#include "rendering/location.h"


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

  // Uniform IDs
  struct {
    GLuint modelViewId;
    GLuint projectionId;
    GLuint normalMatrixId;
    GLuint texIds[SG_OBJ_MAX_TEXTURES];
    SGlightids lightIds[SG_OBJ_MAX_LIGHTS];
    SGmaterialids materialId;
  } uniforms;
  // Attribute IDs
  struct {
    GLuint vertexId;
    GLuint normalId;
    GLuint colourId;
    GLuint texCoordId[SG_MAX_TEXTURES];
  } attribs;
};


void sgLoadAllShaders(void);

SGshader* sgLoadProgram(const char *key, const char *vspath, const char *fspath,
                        const char *gspath);
SGshader* sgGetProgram(const char *key);
void sgEnableProgram(GLuint programId);
void sgDisableProgram(void);
SGshader* sgShaderFromKey(const char *key);



#endif /* !RENDERING_SHADER_MANAGER_H */
