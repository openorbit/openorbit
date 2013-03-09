/*
 Copyright 2011,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef orbit_location_h
#define orbit_location_h
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>
#endif
#include "rendering/types.h"
/*!
  OpenGL 3.3 location ids, these will be used whenever
  GL_ARB_explicit_attrib_location is well supported on the Mac.
 */
#define SG_MAX_TEXTURES 4
#define SG_MAX_LIGHTS 4
#define SG_OUT_FRAGMENT "oo_FragColor"

typedef enum {
  SG_VERTEX = 0,
  SG_NORMAL,
  SG_COLOR,

  SG_TEX_COORD_0,
  SG_TEX_COORD_1,
  SG_TEX_COORD_2,
  SG_TEX_COORD_3,

  SG_ATTRIBUTE_END,

  SG_LIGHT,
  SG_LIGHT_AMB,
  SG_LIGHT_POS,
  SG_LIGHT_SPEC,
  SG_LIGHT_DIFF,
  SG_LIGHT_DIR,
  SG_LIGHT_CONST_ATTEN,
  SG_LIGHT_LINEAR_ATTEN,
  SG_LIGHT_QUAD_ATTEN,

  SG_LIGHT_MOD_GLOB_AMB,

  SG_MATERIAL_EMIT,
  SG_MATERIAL_AMB,
  SG_MATERIAL_DIFF,
  SG_MATERIAL_SPEC,
  SG_MATERIAL_SHINE,


  SG_TEX_0,
  SG_TEX_1,
  SG_TEX_2,
  SG_TEX_3,

  SG_TEX_VALIDITY_0,
  SG_TEX_VALIDITY_1,
  SG_TEX_VALIDITY_2,
  SG_TEX_VALIDITY_3,

  SG_MODELVIEW,
  SG_PROJECTION,
  SG_NORMAL_MATRIX,

  SG_PARAM_COUNT,
} sg_param_id_t;


void sg_location_bind(GLuint prog, sg_param_id_t param);
void sg_location_bind_at_index(GLuint prog, sg_param_id_t param, unsigned index);

void sgSetShaderTex(GLuint program, sg_param_id_t param, GLuint tex);

GLint
sgGetLocationForParamAndIndex(GLuint program, sg_param_id_t param,
                              unsigned index);


#endif /* !orbit_location_h */
