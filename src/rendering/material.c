/*
 Copyright 2010,2013 Mattias Holm <lorrden(at)openorbit.org>

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

#include "material.h"
#include "scenegraph.h"
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif
#include "common/palloc.h"

struct sg_material_t {
  float4 emission;
  float4 ambient;
  float4 diffuse;
  float4 specular;
  float shininess;
};

sg_material_t*
sg_new_material(void)
{
  sg_material_t *mat = smalloc(sizeof(sg_material_t));

  sg_material_init(mat);
  return mat;
}

void
sg_material_bind(sg_material_t *mat, sg_shader_t *shader)
{
  SG_CHECK_ERROR;
  //glMaterialfv(GL_FRONT, GL_AMBIENT, (GLfloat*)&mat->ambient);
  //glMaterialfv(GL_FRONT, GL_DIFFUSE, (GLfloat*)&mat->diffuse);
  //glMaterialfv(GL_FRONT, GL_SPECULAR, (GLfloat*)&mat->specular);
  //glMaterialfv(GL_FRONT, GL_EMISSION, (GLfloat*)&mat->emission);
  //glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess);
  SG_CHECK_ERROR;
}

void
sg_material_init(sg_material_t *mat)
{
  mat->ambient[0] = 0.2;
  mat->ambient[1] = 0.2;
  mat->ambient[2] = 0.2;
  mat->ambient[3] = 1.0;

  mat->diffuse[0] = 0.8;
  mat->diffuse[1] = 0.8;
  mat->diffuse[2] = 0.8;
  mat->diffuse[3] = 1.0;

  mat->specular[0] = 0.0;
  mat->specular[1] = 0.0;
  mat->specular[2] = 0.0;
  mat->specular[3] = 1.0;

  mat->emission[0] = 0.0;
  mat->emission[1] = 0.0;
  mat->emission[2] = 0.0;
  mat->emission[3] = 1.0;

  mat->shininess = 0.0;
}

void
sg_material_set_amb4f(sg_material_t *mat, float r, float g, float b, float a)
{
  mat->ambient[0] = r;
  mat->ambient[1] = g;
  mat->ambient[2] = b;
  mat->ambient[3] = a;
}

void
sg_material_set_diff4f(sg_material_t *mat, float r, float g, float b, float a)
{
  mat->diffuse[0] = r;
  mat->diffuse[1] = g;
  mat->diffuse[2] = b;
  mat->diffuse[3] = a;
}

void
sg_material_set_spec4f(sg_material_t *mat, float r, float g, float b, float a)
{
  mat->specular[0] = r;
  mat->specular[1] = g;
  mat->specular[2] = b;
  mat->specular[3] = a;
}

void
sg_material_set_emiss4f(sg_material_t *mat, float r, float g, float b, float a)
{
  mat->emission[0] = r;
  mat->emission[1] = g;
  mat->emission[2] = b;
  mat->emission[3] = a;
}

void
sg_material_set_shininess(sg_material_t *mat, float s)
{
  mat->shininess = s;
}


float4
sg_material_get_amb(sg_material_t *mat)
{
  return mat->ambient;
}

float4
sg_material_get_diff(sg_material_t *mat)
{
  return mat->diffuse;
}

float4
sg_material_get_spec(sg_material_t *mat)
{
  return mat->specular;
}

float4
sg_material_get_emiss(sg_material_t *mat)
{
  return mat->emission;
}

float
sg_material_get_shininess(sg_material_t *mat)
{
  return mat->shininess;
}
