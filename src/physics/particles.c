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


#include "particles.h"

#include <stdlib.h>
#include <assert.h>
#include "orbit.h"
#include "palloc.h"

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


pl_particles_t*
pl_new_particle_system(const char *name, size_t particleCount)
{
  pl_particles_t *ps = smalloc(sizeof(pl_particles_t));
  ps->enabled = false;
  ps->autoDisable = false;
  ps->particleCount = particleCount;
  ps->obj = NULL;
  // Note: calloc initialise all fields to 0
  ps->particles = calloc(particleCount, sizeof(pl_particle_t));

  int_array_init(&ps->freeParticles);
  for (int i = 0 ; i < particleCount ; ++ i) {
    int_array_push(&ps->freeParticles, i);
  }

  return ps;
}

void
pl_particles_set_emitter_direction3f(pl_particles_t *ps, float x, float y, float z)
{
  ps->v = vf3_set(x, y, z);
}

void
pl_particles_attach3f(pl_particles_t *ps, pl_object_t *obj, float x, float y, float z)
{
  assert(ps != NULL);
  assert(obj != NULL);
  assert(ps->obj == NULL);
  obj_array_push(&obj->psystem, ps);
  ps->obj = obj;
  if (obj->parent) {
    obj_array_push(&obj->parent->world->particle_systems, ps);
  } else {
    obj_array_push(&obj->world->particle_systems, ps);
  }
  ps->p = vf3_set(x, y, z);
}


void
pl_particles_delete(pl_particles_t *ps)
{
  assert(ps != NULL);
  free(ps->particles);
  free(ps);
}

void
pl_particles_set_emission_rate(pl_particles_t *ps, float er)
{
  assert(ps != NULL);
  assert(er >= 0.0f);
  ps->emissionRate = er;
}

void
pl_particles_enable(pl_particles_t *ps)
{
  assert(ps != NULL);
  ps->enabled = true;
}

void
pl_particles_turn_off(pl_particles_t *ps)
{
  assert(ps != NULL);
  ps->autoDisable = true;
}

void
pl_particles_disable(pl_particles_t *ps)
{
  assert(ps != NULL);
  ps->enabled = false;
}

float rand_percent(int a)
{
  return ((float)(random() % a*2 - a)) * 0.01;
}

void
pl_particles_step(pl_particles_t *ps, float dt)
{
  assert(ps != NULL);
  if (ps->enabled == false) return;

  size_t activeParticles = 0;
  for (size_t i = 0 ; i < ps->particleCount ; ++i) {
    if (ps->particles[i].active) {
      ps->particles[i].p += vf3_s_mul(ps->particles[i].v, dt) + vf3_s_mul(ps->obj->v, dt);
      ps->particles[i].age += dt;
      activeParticles++;

      if (ps->particles[i].age > ps->particles[i].lifeTime) {
        ps->particles[i].active = false;
        int_array_push(&ps->freeParticles, i);
        activeParticles --;
      }
    }
  }

  // Auto disable
  if (ps->autoDisable) {
    if (activeParticles == 0) {
      ps->enabled = false;
    }

    return;
  }

  // Not off or disabled, emitt new particles
  float newPartCount = ps->emissionRate * dt * (1.0 + rand_percent(10));
  float intPart;
  float frac = modff(newPartCount, &intPart);
  unsigned newParticles = (unsigned) intPart;

  // The fraction is handled with randomisation
  int rval = random() % 128;
  if ((float)rval/128.0f < frac) newParticles ++;

  for (unsigned i = 0 ; i < newParticles ; ++i) {
    if (ps->freeParticles.length > 0) {
      int i = int_array_remove(&ps->freeParticles, 0);
      ps->particles[i].active = true;
      ps->particles[i].age = 0.0;
      // Adjust lifetime by +-20 %
      ps->particles[i].lifeTime = ps->lifeTime + ps->lifeTime * rand_percent(20);
      ps->particles[i].p = v_q_rot(ps->p, ps->obj->q);
      ps->particles[i].v = v_q_rot(ps->v * vf3_set(rand_percent(10), rand_percent(10), rand_percent(10)), ps->obj->q);
      ps->particles[i].rgb = ps->rgb;
    } else {
      break;
    }
  }
}
