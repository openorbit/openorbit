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


#include "particles.h"

#include <stdlib.h>
#include <assert.h>

static float3
random_direction(float ra, float dec, float maxOffsetAngle)
{
  // TODO: Generate direction with gaussian distribution?
  
  // Random direction
  long dir = (random() % 1000) + 1;
  float dirf = (float)(2.0f * M_PI / (float)dir);
  
  // Random offset angle in direction
  float offsetAngle = maxOffsetAngle / (float)((random() % 99) + 1);
  
  return equ2cart_f(ra + offsetAngle * sinf(dirf),
                    dec + offsetAngle * cosf(dirf));
}


PLparticles*
plNewParticleSystem(const char *name, size_t particleCount)
{
  PLparticles *ps = malloc(sizeof(PLparticles));

  ps->particleCount = particleCount;

  // Note: calloc initialise all fields to 0
  ps->particles = calloc(particleCount, sizeof(PLparticle));
}

void
plDeleteParticleSystem(PLparticles *ps)
{
  free(ps->particles);
  free(ps);
}

void
plParticleSystemSetEmissionRate(PLparticles *ps, float er)
{
  assert(er >= 0.0f);
  ps->emissionRate = er;
}


void
plStepParticleSystem(PLparticles *ps, float dt)
{
  for (size_t i = 0 ; i < ps->particleCount ; ++i) {
    ps->particles[i].p += vf3_s_mul(ps->particles[i].v, dt);
    ps->particles[i].age += dt;

    if (ps->particles[i].age < ps->particles[i].lifeTime) {

    }
  }

  float newPartCount = ps->emissionRate * dt;
  float intPart;
  float frac = modff(newPartCount, &intPart);
  unsigned newParticles = (unsigned) intPart;

  for (unsigned i = 0 ; i < newParticles ; ++i) {

  }
  // Handle remainder
  int rval = random() % 128;
  if ((float)rval/128.0f < frac) {

  }
}