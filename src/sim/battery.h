/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIM_BATTERY_H
#define SIM_BATTERY_H

#include <vmath/vmath.h>
#include <gencds/array.h>

// Batteries are the secondary source for power, they are typically charged when
// the sun is visible and discharged in eclipse. This interface is pretty stupid
// and only allow for a constant maximum charge and discharge rate. These
// parameters are obviously dependent on the design and various environmental
// parameters. In the future, this interface should be updated to support more
// complex simulations.
struct SIMbattery {
  float energyContent;    // Joule
  float maxDischargeRate; // Watt
  float maxChargeRate;    // Watt
  float currentLoad;      // Watt
};

typedef struct SIMbattery SIMbattery;

// Energy sources are the primary energy reserves, these may be for example
// solar panels or similar.
struct SIMenergysource {
  float currentPower;     // Watt
};
typedef struct SIMenergysource SIMenergysource;

typedef struct SIMpowerbus SIMpowerbus;
typedef void (*SIMpoweroverload)(SIMpowerbus *pb);

struct SIMpowerbus {
  float currentLoad;
  float currentPower;
  obj_array_t batteries;
  obj_array_t energySources;
  SIMpoweroverload overloadAction;
};

void simResetPowerbus(SIMpowerbus *pb);
float simRequestPower(SIMpowerbus *pb, float power);
void simProducePower(SIMpowerbus *pb, float power);

#endif /* !SIM_BATTERY_H */

