/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "physics.h"

// For Earth
#define PL_G0 9.80665

PLdouble3
plGetAirspeed(PLobject *obj)
{
  
}

PLdouble3
plGetAirpressure(PLobject *obj)
{
  
}

/*!
 Computes the air preasure at a given altitude, note that for simplicitys sake,
 you can call the methods with 0 for the hb value and use standard ground level
 parameters for Pb and Tb.
 \param Pb Static preasure (reference)
 \param Tb Standard temperature
 \param g0 Gravitational accelleration
 \param M Molar mass of atmosphere
 \param h Height above sea level
 \param hb height of reference point (for Pb / Tb)
 \param Lb Lapse rate in kelvin per metre
 */
double
plPreasureAtAltitudeWithLapse(double Pb, double Tb, double g0, double M,
                              double h, double hb, double Lb)
{
  return Pb * pow(Tb/(Tb+Lb*(h-hb)), g0*M / (PL_UGC*Lb));
}


double
plPreasureAtAltitude(double Pb, double Tb, double g0, double M, double h,
                     double hb)
{
  return Pb * exp((-g0 * M * (h-hb)) / (PL_UGC * Tb));
}
