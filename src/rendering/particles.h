/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#ifndef SG_PARTICLES_H
#define SG_PARTICLES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "scenegraph.h"

typedef struct SGparticle {
  bool active;
  float	lifeTime;
	float	fadeFactor;
  uint8_t r, g, b;
  float x, y, z;
  float xd, yd, zd;
  float xg, yg, zg;
} SGparticle;

typedef struct SGparticles {
  OOdrawable super;
  float slowdown;
  size_t particleCount;
  
  //float *pos;
  //float *colour;
  //float *dir;
  //float *lifeTime;
  //float *fadeFactors;
  
  SGparticle *particles;
} SGparticles;

OOdrawable* sgNewParticleSystem(const char *name, size_t particleCount);

#endif /* !SG_PARTICLES_H */