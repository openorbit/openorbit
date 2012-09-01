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


sg_light_t* sgNewPointlight(sg_scene_t *sc, float3 p);
sg_light_t* sgNewPointlight3f(sg_scene_t *sc, float x, float y, float z);
void sgSetLightPos3f(sg_light_t *light, float x, float y, float z);
void sgSetLightPosv(sg_light_t *light, float3 v);
void sgSetLightPosLW(sg_light_t *light, lwcoord_t *lwc);


#endif /* !SG_LIGHT_H__ */
