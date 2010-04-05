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
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif
#include "texture.h"
#include <vmath/vmath.h>

void
sgDrawParticles(SGparticles *sp)
{
  if (!sp->ps->enabled) return;
  glPushMatrix();

  // Draw particle system, each particle is a textured quad.
  glBindTexture(GL_TEXTURE_2D, sp->texture);
  glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
  glPointSize(1.0);
  glDisable(GL_LIGHTING);
  glBegin(GL_POINTS);
  glColor3f(sp->ps->rgb.x, sp->ps->rgb.y, sp->ps->rgb.z);
  for (int i = 0 ; i < sp->ps->particleCount ; ++ i) {
    if (sp->ps->particles[i].active) {
      glVertex3f(sp->ps->particles[i].p.x, sp->ps->particles[i].p.y, sp->ps->particles[i].p.z);
    }
  }
  glEnd();
  glPopMatrix();
}

SGdrawable* sgNewParticleSystem(const char *name, const char *tex,
                                PLparticles *ps)
{
  SGparticles *drawable = malloc(sizeof(SGparticles));
  drawable->ps = ps;
  
  if (ooTexGet(tex) == NULL) {
    ooTexLoad(tex, tex);    
  }
  drawable->texture = ooTexNum(tex);

  return sgNewDrawable((SGdrawable*)drawable, name, (SGdrawfunc)sgDrawParticles);
}
