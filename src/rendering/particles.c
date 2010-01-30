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

void
sgDrawParticles(SGparticles *sp)
{
  
}

SGdrawable*
sgNewParticleSystem(const char *name, size_t particleCount)
{
  SGparticles *ps = malloc(sizeof(SGparticles));
  ps->particles = calloc(particleCount, sizeof(SGparticle));
  
  //float *data = calloc(particleCount*(3+3+3+1+1), sizeof(float));
  //ps->pos = data;
  //ps->colour = ps->pos + particleCount * 3;
  //ps->dir = ps->colour + particleCount * 3;
  //ps->lifeTime = ps->dir + particleCount * 3;
  //ps->fadeFactors = ps->lifeTime + particleCount * 1;
  
  ps->slowdown = 2.0f;
  for (size_t i = 0 ; i < particleCount ; ++ i) {
    ps->particles[i].active = true;					// Make All The Particles Active
		ps->particles[i].lifeTime = 1.0f;
    ps->particles[i].fadeFactor = (float)(rand()%100)/1000.0f+0.003f;
    
    ps->particles[i].x = 0.0f;
    ps->particles[i].y = 0.0f;
    ps->particles[i].z = 0.0f;    
    
    ps->particles[i].xd = (float)((rand()%50)-26.0f)*10.0f;
    ps->particles[i].yd = (float)((rand()%50)-25.0f)*10.0f;
    ps->particles[i].zd = (float)((rand()%50)-25.0f)*10.0f;
    
    ps->particles[i].xg = 0.0f;
    ps->particles[i].yg = 0.0f;
    ps->particles[i].zg = 0.0f;
  }

  return ooSgNewDrawable((SGdrawable*)ps, name, (OOdrawfunc)sgDrawParticles);
}

void
sgStepParticleSystem(SGparticles *ps, float dt)
{
  for (size_t i = 0 ; i < ps->particleCount ; ++i) {
    ps->particles[i].x += ps->particles[i].xd/(ps->slowdown*1000);
    ps->particles[i].y += ps->particles[i].yd/(ps->slowdown*1000);
    ps->particles[i].z += ps->particles[i].zd/(ps->slowdown*1000);
    ps->particles[i].lifeTime -= ps->particles[i].fadeFactor * dt;
  }
}
