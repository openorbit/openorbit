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

#include "propellant-tank.h"
#include "spacecraft.h"
#include "palloc.h"

SIMtank*
simNewTank(sim_stage_t *stage, const char *tankName, float p, float v, float t)
{
  SIMtank *tank = smalloc(sizeof(SIMtank));

  tank->rec = simPubsubMakeRecord(stage->rec, tankName);

  simPublishValue(tank->rec, SIM_TYPE_FLOAT, "pressure", &tank->pressure);
  simPublishValue(tank->rec, SIM_TYPE_FLOAT, "volume", &tank->volume);
  simPublishValue(tank->rec, SIM_TYPE_FLOAT, "temperature", &tank->temperature);

  SIM_VAL(tank->pressure) = p;
  SIM_VAL(tank->volume) = v;
  SIM_VAL(tank->pressure) = p;
}

void
simDeleteTank(SIMtank *tank)
{
  free(tank);
}


void
simOpenValve(SIMtank *tank)
{
}

void
simEnablePump(SIMtank *tank)
{

}
