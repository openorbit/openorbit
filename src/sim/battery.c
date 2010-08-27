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

#include "battery.h"
#include "log.h"

static void
PowerOverloadLog(SIMpowerbus *pb)
{
  ooLogWarn("power overload");
}

void
simResetPowerbus(SIMpowerbus *pb)
{
  pb->currentLoad = 0.0;
  pb->currentPower = 0.0;
}

float
simRequestPower(SIMpowerbus *pb, float power)
{
  if (pb->currentLoad + power > pb->currentPower) {
    // Overload
    return 0.0;
  } else {
    pb->currentLoad += power;
    return power;
  }
}

void
simProducePower(SIMpowerbus *pb, float power)
{
  pb->currentPower += power;
}
