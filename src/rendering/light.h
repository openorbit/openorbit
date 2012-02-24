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
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <vmath/vmath.h>
#include "rendering/reftypes.h"
#include "common/lwcoord.h"

typedef void (*SGenable_light_func)(SGlight *light, GLenum lightId);
typedef void (*SGdisable_light_func)(SGlight *light);

struct SGlight {
  struct SGscene *scene;
  int lightId;
  
  float pos[4];
  
  float ambient[4];
  float specular[4];
  float diffuse[4];
  
  SGenable_light_func enable;
  SGdisable_light_func disable;
};

struct SGspotlight {
  SGlight super;
  float dir[3];
};

struct SGpointlight {
  SGlight super;
};

SGlight* sgNewPointlight(SGscene *sc, float3 p);
SGlight* sgNewPointlight3f(SGscene *sc, float x, float y, float z);
void sgSetLightPos3f(SGlight *light, float x, float y, float z);
void sgSetLightPosv(SGlight *light, float3 v);
void sgSetLightPosLW(SGlight *light, OOlwcoord *lwc);


#endif /* !SG_LIGHT_H__ */
