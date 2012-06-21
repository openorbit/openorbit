/*
  Copyright 2007 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef PLANET_H__
#define PLANET_H__

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#include <GL3/gl3.h>
#include <GL/glu.h>
#endif

#include <vmath/vmath.h>

typedef struct {
  GLuint texId;
  GLUquadricObj *quadratic;
  float radius;
  float3 pos; // original position (this is should be in a plane)
  float mass;
  quaternion_t q;
} OOplanet;
/* Most of the stuff here should be moved to the dynamics system later on */

typedef struct {
    GLuint texid;
    GLUquadricObj *quadratic;
    float radius;
    float3 pos; // original position (this is should be in a plane)
    float mass;
    quaternion_t rot_orig;
    quaternion_t rot_ax;
} planet_t;


#define MAX_PLANETS 10
extern planet_t gPlanets[MAX_PLANETS];

int planet_add(float x, float y, float z, float radius, float mass, const char *tex_key);

void planet_rot_orig(planet_t *planet, float rot_rad);
void planet_rot_ax(planet_t *planet, float rot_rad);

void planet_draw_all(void);
void planet_draw(planet_t *planet);



#endif /* ! PLANET_H__ */
