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
#include "orbit.h"
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
  ps->enabled = false;
  ps->particleCount = particleCount;
  ps->obj = NULL;
  // Note: calloc initialise all fields to 0
  ps->particles = calloc(particleCount, sizeof(PLparticle));
  return ps;
}

void
plAttachParticleSystem(PLparticles *ps, PLobject *obj)
{
  assert(ps != NULL);
  assert(obj != NULL);
  assert(ps->obj == NULL);

  ps->obj = obj;
  if (obj->parent) {
    obj_array_push(&obj->parent->super.sys->world->partSys, ps);
  } else {
    obj_array_push(&obj->sys->world->partSys, ps);
  }
}


void
plDeleteParticleSystem(PLparticles *ps)
{
  assert(ps != NULL);
  free(ps->particles);
  free(ps);
}

void
plParticleSystemSetEmissionRate(PLparticles *ps, float er)
{
  assert(ps != NULL);
  assert(er >= 0.0f);
  ps->emissionRate = er;
}

void
plEnableParticleSystem(PLparticles *ps)
{
  assert(ps != NULL);
  ps->enabled = true;
}

void
plDisableParticleSystem(PLparticles *ps)
{
  assert(ps != NULL);
  ps->enabled = false;
}


void
plStepParticleSystem(PLparticles *ps, float dt)
{
  assert(ps != NULL);
  if (ps->enabled == false) return;

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
