/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIM_PROPELLANT_TANK
#define SIM_PROPELLANT_TANK

#include <sim/pubsub.h>
#include <sim/simtypes.h>

typedef enum {
  SIM_Liquid,
  SIM_Solid,
} SIMpropellanttype;

struct SIMtank {
  sim_record_t *rec;
  SIMpropellanttype kind;
  const char *propellantType;
  sim_float_t pressure;
  sim_float_t volume;
  sim_float_t temperature;
};

typedef struct SIMtank SIMtank;

// Two ways of getting propellant mass, one is to rely on a pumping system and
// request mass through the pumps, the other way is to request mass by opening
// valves attached to the pressurised tank.

SIMtank* simNewTank(sim_stage_t *stage, const char *tankName, float p, float v, float t);
void simDeleteTank(SIMtank *tank);
void simOpenValve(SIMtank *tank);
void simEnablePump(SIMtank *tank);

#endif /* !SIM_PROPELLANT_TANK */
