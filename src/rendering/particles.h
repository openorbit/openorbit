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

#ifndef SG_PARTICLES_H
#define SG_PARTICLES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif
#include "physics/particles.h"

#include "physics/reftypes.h"
#include "rendering/types.h"

#if 0
struct SGparticles {
  SGdrawable super;
  GLuint texture;
  pl_particles_t *ps;
};

SGdrawable* sgNewParticleSystem(const char *name, const char *tex,
                                pl_particles_t *ps);
#endif

#endif /* !SG_PARTICLES_H */
