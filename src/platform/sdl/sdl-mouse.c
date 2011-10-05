/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <SDL/SDL.h>

#include <assert.h>

void
platform_get_mouse(float *x, float *y)
{
  assert(x != NULL);
  assert(y != NULL);
  int xp, yp;

  SDL_GetMouseState(&xp, &yp);
  SDL_Surface *videoSurface = SDL_GetVideoSurface();

  *x = ((float)xp / (float)videoSurface->w) * 2.0f - 1.0f;
  *y = ((float)yp / (float)videoSurface->h) * 2.0f - 1.0f;
}
