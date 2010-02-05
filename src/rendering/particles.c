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
#include "SDL_opengl.h"
#include "texture.h"
#include <vmath/vmath.h>

void
sgDrawParticles(SGparticles *sp)
{
  // Draw particle system, each particle is a textured quad.
  glBindTexture(GL_TEXTURE_2D, sp->texture);
  glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
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

  return ooSgNewDrawable((SGdrawable*)drawable, name, (OOdrawfunc)sgDrawParticles);
}
