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
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

void sgLoadAllShaders(void);

GLuint sgLoadProgram(const char *key, const char *vspath, const char *fspath,
                     const char *gspath);
GLuint sgGetProgram(const char *key);
void sgEnableProgram(GLuint programId);
void sgDisableProgram(void);
GLuint sgShaderFromKey(const char *key);

typedef enum {
  SG_VERTEX0 = 0,
  SG_TEX_COORD0,
  SG_COLOR0,
  SG_TEX0,
  SG_TEX1,
  SG_TEX2,
  SG_TEX3,
  SG_LIGHT0,
  SG_LIGHT1,
  SG_LIGHT2,
  SG_LIGHT3,
  SG_MODELVIEW,
  SG_PROJECTION,
  SG_PARAM_COUNT,
} sg_param_id_t;

GLint sgGetLocationForParam(GLuint program, sg_param_id_t param);
void sgSetShaderTex(GLuint program, sg_param_id_t param, GLuint tex);

#endif /* !RENDERING_SHADER_MANAGER_H */
