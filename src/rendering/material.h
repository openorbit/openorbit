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

#ifndef SG_MATERIAL_H__
#define SG_MATERIAL_H__

#include "rendering/reftypes.h"

struct SGmaterial {
  float emission[4];
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float shininess;
};

void sgInitMaterial(SGmaterial *mat);
void sgBindMaterial(SGmaterial *mat);

void sgSetMaterialAmb4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialDiff4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialSpec4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialEmiss4f(SGmaterial *mat, float r, float g, float b, float a);
void sgSetMaterialShininess(SGmaterial *mat, float s);



#endif /* !SG_MATERIAL_H__ */
