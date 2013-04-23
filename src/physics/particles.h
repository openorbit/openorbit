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

#ifndef PL_PARTICLES_H
#define PL_PARTICLES_H
#include <stdbool.h>
#include <stdlib.h>

#include "rendering/scenegraph.h"
#include "physics/reftypes.h"
#include <vmath/vmath.h>
#include "physics/object.h"

struct pl_particle_t {
  bool active;
  float	age; // Age of particles
  float	lifeTime; // lifetime
  float fadeFactor;
  float3 rgb; // Colour
  double3 p; // Position
  double3 v; // Velocity
  float3 s; // Size
};

struct pl_particles_t {
  pl_object_t *obj; // Object the particle generator is attached to
  //GLuint texture;
  float directionLimit;
  float3 rgb; // Default colour
  double3 p; // Default position of emitter (relative to object position)
  double3 v; // Default velocity vector for new particle
  float emissionRate; // Emmission rate of particles
  float lifeTime; // Average life of paticles
  size_t particleCount; // Maximum particles in system, old particles are simply
                        // deleted if they are needed.
  size_t particleMask; // Particle count mask, particle count must be power of
                       // two
  size_t oldestParticleIdx;
  size_t newestParticleIdx;
  bool enabled; // Should we simulate the particle system at all?
  bool autoDisable; // Do not emit new particles, and disable when no particles are alive

  pl_particle_t *particles;
  int_array_t freeParticles;
};


pl_particles_t* pl_new_particle_system(const char *name, size_t particleCount);
void pl_particles_delete(pl_particles_t *ps);
void pl_particles_step(pl_particles_t *ps, float dt);
void pl_particles_attach3f(pl_particles_t *ps, pl_object_t *obj, float x, float y, float z);
void pl_particles_set_emitter_direction3f(pl_particles_t *ps, float x, float y, float z);

void pl_particles_enable(pl_particles_t *ps);
void pl_particles_disable(pl_particles_t *ps);

void pl_particles_set_emission_rate(pl_particles_t *ps, float er);

#endif /* !PL_PARTICLES_H */
