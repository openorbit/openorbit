/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

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



#include <stdio.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/viewport.h"

void
sg_set_viewport(sg_viewport_t *viewport)
{
  glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
}

sg_viewport_t*
sg_new_viewport(sg_window_t *window, unsigned x, unsigned y,
                unsigned w, unsigned h)
{
  sg_viewport_t *viewport = malloc(sizeof(sg_viewport_t));
  memset(viewport, 0, sizeof(sg_viewport_t));
  return viewport;
}
