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


#include "mfd.h"

#include <assert.h>

void
simMfdDraw(SIMmfd *mfd)
{
  assert(mfd != NULL);
  // TODO: Draw frame
  mfd->draw(mfd);
}


void
simHudDraw(SIMhud *hud)
{
  assert(hud != NULL);
  // TODO: Draw frame
  hud->draw(hud);
}
