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

#ifndef SG_LIGHT_H__
#define SG_LIGHT_H__

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include <vmath/vmath.h>
#include "rendering/types.h"
#include <vmath/lwcoord.h>

//typedef void (*SGenable_light_func)(SGlight *light, GLenum lightId);
//typedef void (*SGdisable_light_func)(SGlight *light);


sg_light_t* sg_new_light(sg_scene_t *sc, float3 p);
sg_light_t* sg_new_light3f(sg_scene_t *sc, float x, float y, float z);

void sg_light_set_pos3f(sg_light_t *light, float x, float y, float z);
void sg_light_set_posv(sg_light_t *light, float3 v);
void sg_light_set_poslw(sg_light_t *light, lwcoord_t *lwc);
float3 sg_light_get_pos(const sg_light_t *light);

void sg_light_set_ambient4f(sg_light_t *light,
                            float r, float g, float b, float a);
float4 sg_light_get_ambient(const sg_light_t *light);


void sg_light_set_specular4f(sg_light_t *light,
                             float r, float g, float b, float a);
float4 sg_light_get_specular(const sg_light_t *light);

void sg_light_set_diffuse4f(sg_light_t *light,
                            float r, float g, float b, float a);
float4 sg_light_get_diffuse(const sg_light_t *light);

void sg_light_set_attenuation(sg_light_t *light,
                              float const_att, float lin_att, float quad_att);
float sg_light_get_const_attenuation(const sg_light_t *light);
float sg_light_get_linear_attenuation(const sg_light_t *light);
float sg_light_get_quadratic_attenuation(const sg_light_t *light);
#endif /* !SG_LIGHT_H__ */
