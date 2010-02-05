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
#include "SDL_opengl.h"
#include "physics/particles.h"

typedef struct SGparticles {
  SGdrawable super;
  GLuint texture;
  PLparticles *ps;
} SGparticles;

SGdrawable* sgNewParticleSystem(const char *name, const char *tex,
                                PLparticles *ps);


#endif /* !SG_PARTICLES_H */