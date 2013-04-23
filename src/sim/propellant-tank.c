/*
 Copyright 2010,2011,2013 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "sim/propellant-tank.h"
#include "sim/spacecraft.h"
#include "common/palloc.h"

sim_tank_t*
sim_new_tank(sim_stage_t *stage, const char *tankName, float p, float v, float t)
{
  sim_tank_t *tank = smalloc(sizeof(sim_tank_t));

  tank->rec = sim_pubsub_make_record(stage->rec, tankName);

  sim_pubsub_publish_val(tank->rec, SIM_TYPE_FLOAT, "pressure", &tank->pressure);
  sim_pubsub_publish_val(tank->rec, SIM_TYPE_FLOAT, "volume", &tank->volume);
  sim_pubsub_publish_val(tank->rec, SIM_TYPE_FLOAT, "temperature", &tank->temperature);

  SIM_VAL(tank->pressure) = p;
  SIM_VAL(tank->volume) = v;
  SIM_VAL(tank->pressure) = p;

  return tank;
}

void
sim_tank_delete(sim_tank_t *tank)
{
  free(tank);
}


void
sim_tank_open_valve(sim_tank_t *tank)
{
}

void
sim_tank_enable_pump(sim_tank_t *tank)
{

}
