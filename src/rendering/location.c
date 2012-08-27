/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "location.h"


static const char* param_names[SG_PARAM_COUNT] = {
  [SG_VERTEX] = "oo_Vertex",
  [SG_NORMAL] = "oo_Normal",
  [SG_TEX_COORD] = "oo_TexCoord[%u]",
  [SG_COLOR] = "oo_Color",
  [SG_TEX] = "oo_Texture[%u]",

  [SG_LIGHT] = "oo_Light[%u]",

  [SG_LIGHT_AMB] = "oo_Light[%u].ambient",
  [SG_LIGHT_POS] = "oo_Light[%u].pos",
  [SG_LIGHT_SPEC] = "oo_Light[%u].specular",
  [SG_LIGHT_DIFF] = "oo_Light[%u].diffuse",
  [SG_LIGHT_DIR] = "oo_Light[%u].dir",
  [SG_LIGHT_CONST_ATTEN] = "oo_Light[%u].constantAttenuation",
  [SG_LIGHT_LINEAR_ATTEN] = "oo_Light[%u].linearAttenuation",
  [SG_LIGHT_QUAD_ATTEN] = "oo_Light[%u].quadraticAttenuation",
  [SG_LIGHT_MOD_GLOB_AMB] = "oo_Light[%u].globAmbient",

  [SG_MATERIAL_EMIT] = "oo_Material.emission",
  [SG_MATERIAL_AMB] = "oo_Material.ambient",
  [SG_MATERIAL_DIFF] = "oo_Material.diffuse",
  [SG_MATERIAL_SPEC] = "oo_Material.specular",
  [SG_MATERIAL_SHINE] = "oo_Material.shininess",

  [SG_MODELVIEW] = "oo_ModelViewMatrix",
  [SG_PROJECTION] = "oo_ProjectionMatrix",
  [SG_NORMAL_MATRIX] = "oo_NormalMatrix",
};

void
sgBindLocation(GLuint prog, sg_param_id_t param)
{
  glBindAttribLocation(prog, param, param_names[param]);
}

void
sgBindLocationAtIndex(GLuint prog, sg_param_id_t param, unsigned index)
{
  glBindAttribLocation(prog, param, param_names[param] + index);
}


GLint
sgGetLocationForParam(GLuint program, sg_param_id_t param)
{
  // Handle both attributes and uniforms
  if (param < SG_ATTRIBUTE_END) {
    return glGetAttribLocation (program, param_names[param]);
  }
  return glGetUniformLocation(program, param_names[param]);
}

GLint
sgGetLocationForParamAndIndex(GLuint program, sg_param_id_t param,
                              unsigned index)
{
  assert(index < 100);

  char locName[strlen(param_names[param])];
  snprintf(locName, sizeof(locName), param_names[param], index);

  if (param < SG_ATTRIBUTE_END) {
    return glGetAttribLocation (program, locName);
  }
  return glGetUniformLocation(program, locName);
}

void
sgSetShaderTex(GLuint program, unsigned index, GLuint tex)
{
  glActiveTexture(GL_TEXTURE0 + index);
  glBindTexture(GL_TEXTURE_2D, tex);
  GLint loc = sgGetLocationForParamAndIndex(program, SG_TEX, index);
  glUniform1i(loc, index); // Uniform value is texture unit id
}
