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


// Tsiolkovsky Rocket Equation
// Note that this is used by engine models to model their behaviour, the values dm and ve
#include "physics.h"
#include <vmath/vmath.h>
PLdouble3
plTsiolkovsky(PLdouble3 ve, double m0, double dm)
{
  return vd3_s_mul(ve, log(m0 / (m0 - dm)));
}

// Thrust for exhaust velocity ve, delta m and delta t
PLdouble3
plThrust(PLdouble3 ve, double dm, double dt)
{
  return vd3_s_mul(ve, dm / dt);
}
