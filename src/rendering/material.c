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

#include "material.h"
#include "scenegraph.h"
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL3/gl3.h>
#endif

void
sgBindMaterial(SGmaterial *mat)
{
  SG_CHECK_ERROR;
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
  glMaterialfv(GL_FRONT, GL_EMISSION, mat->emission);
  glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess);
  SG_CHECK_ERROR;
}

void
sgInitMaterial(SGmaterial *mat)
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
sgSetMaterialAmb4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->ambient[0] = r;
  mat->ambient[1] = g;
  mat->ambient[2] = b;
  mat->ambient[3] = a;
}

void
sgSetMaterialDiff4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->diffuse[0] = r;
  mat->diffuse[1] = g;
  mat->diffuse[2] = b;
  mat->diffuse[3] = a;
}

void
sgSetMaterialSpec4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->specular[0] = r;
  mat->specular[1] = g;
  mat->specular[2] = b;
  mat->specular[3] = a;
}

void
sgSetMaterialEmiss4f(SGmaterial *mat, float r, float g, float b, float a)
{
  mat->emission[0] = r;
  mat->emission[1] = g;
  mat->emission[2] = b;
  mat->emission[3] = a;
}

void
sgSetMaterialShininess(SGmaterial *mat, float s)
{
  mat->shininess = s;
}
