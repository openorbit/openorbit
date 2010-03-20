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

#ifndef PL_PARTICLES_H
#define PL_PARTICLES_H
#include <stdbool.h>
#include <stdlib.h>

#include "rendering/scenegraph.h"
#include "physics/reftypes.h"
#include <vmath/vmath.h>
#include "physics/object.h"

struct PLparticle {
  bool active;
  float	age; // Age of particles
  float	lifeTime; // lifetime
	float	fadeFactor;
  float3 rgb; // Colour
  float3 p; // Position
  float3 v; // Velocity
  float3 s; // Size
};

struct PLparticles {
  PLobject *obj; // Object the particle generator is attached to
  //GLuint texture;
  float directionLimit;
  float3 rgb; // Default colour
  float3 p; // Default position of emitter
  float3 v; // Default velocity vector for new particle
  float emissionRate; // Emmission rate of particles
  size_t particleCount; // Maximum particles in system, old particles are simply
                        // deleted if they are needed.
  size_t particleMask; // Particle count mask, particle count must be power of
                       // two
  size_t oldestParticleIdx;
  size_t newestParticleIdx;
  bool enabled;
  //float *pos;
  //float *colour;
  //float *dir;
  //float *lifeTime;
  //float *fadeFactors;
  
  PLparticle *particles;
};


PLparticles* plNewParticleSystem(const char *name, size_t particleCount);
void plDeleteParticleSystem(PLparticles *ps);
void plStepParticleSystem(PLparticles *ps, float dt);
void plAttachParticleSystem(PLparticles *ps, PLobject *obj);

void plEnableParticleSystem(PLparticles *ps);
void plDisableParticleSystem(PLparticles *ps);

void plParticleSystemSetEmissionRate(PLparticles *ps, float er);

#endif /* !PL_PARTICLES_H */