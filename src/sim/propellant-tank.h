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

#ifndef SIM_PROPELLANT_TANK
#define SIM_PROPELLANT_TANK

struct SIMpropellanttank {
  const char *propellantType;
  float pressure;
  float volume;
};

typedef struct SIMpropellanttank SIMpropellanttank;

// Two ways of getting propellant mass, one is to rely on a pumping system and
// request mass through the pumps, the other way is to request mass by opening
// valves attached to the pressurised tank.

void simOpenValve(SIMpropellanttank *tank);
void simEnablePump(SIMpropellanttank *tank);

#endif /* !SIM_PROPELLANT_TANK */
