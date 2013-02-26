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

#include <vmath/vmath.h>
#include "rendering/types.h"

sg_material_t* sg_new_material(void);

void sg_material_init(sg_material_t *mat);
void sg_material_bind(sg_material_t *mat, sg_shader_t *shader);

void sg_material_set_amb4f(sg_material_t *mat, float r, float g, float b, float a);
void sg_material_set_diff4f(sg_material_t *mat, float r, float g, float b, float a);
void sg_material_set_spec4f(sg_material_t *mat, float r, float g, float b, float a);
void sg_material_set_emiss4f(sg_material_t *mat, float r, float g, float b, float a);
void sg_material_set_shininess(sg_material_t *mat, float s);


float4 sg_material_get_amb(sg_material_t *mat);
float4 sg_material_get_diff(sg_material_t *mat);
float4 sg_material_get_spec(sg_material_t *mat);
float4 sg_material_get_emiss(sg_material_t *mat);
float sg_material_get_shininess(sg_material_t *mat);


#endif /* !SG_MATERIAL_H__ */
