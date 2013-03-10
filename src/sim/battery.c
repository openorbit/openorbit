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

#include "battery.h"
#include <openorbit/log.h>
#include <gencds/array.h>
static void
PowerOverloadLog(sim_powerbus_t *pb)
{
  log_warn("power overload");
}

void
sim_powerbus_reset(sim_powerbus_t *pb)
{
  pb->currentLoad = 0.0;
  pb->currentPower = 0.0;
}

float
sim_powerbus_request_power(sim_powerbus_t *pb, float power)
{
  if (pb->currentLoad + power > pb->currentPower) {
    // Overload
    pb->overloadAction(pb);
    return 0.0;
  } else {
    pb->currentLoad += power;
    return power;
  }
}

void
sim_powerbus_provide_power(sim_powerbus_t *pb, float power)
{
  pb->currentPower += power;
}

void
sim_powerbus_step(sim_powerbus_t *pb, float dt)
{
  ARRAY_FOR_EACH(i, pb->batteries) {

  }

  ARRAY_FOR_EACH(i, pb->energySources) {

  }
}
