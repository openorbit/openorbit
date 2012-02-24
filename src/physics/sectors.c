/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "physics.h"
#include "common/lwcoord.h"


// Computes whether two lines with radius w both having their origin in some LW coord
// intersects at some point. This can be used for implementing a sweeping collission
// detection system in a large world
void
plLwcIntersectionPoint(const OOlwcoord * restrict a, PLfloat3 da, float wa,
                       const OOlwcoord * restrict b, PLfloat3 db, float wb)
{
  // Transform b to a's segment and compute the intersection point
  //  PLfloat3 bPosInASeg = ooLwcRelVec(b, a->seg);
  //PLfloat3 bNext = vf3_add(bPosInASeg, db);
  //PLfloat3 aNext = vf3_add(a->offs, da);
  
  
}
