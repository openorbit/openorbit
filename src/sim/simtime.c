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
 
 
#include "sim.h"
#include "sim/simtime.h"

OOsimtime*
ooSimTimeInit(time_t epoch)
{
  //  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  //float wc_period = 1.0 / freq; // Period in s
  //Uint32 interv = (Uint32) (wc_period * 1000.0); // SDL wants time in ms
  //float sim_period;
  //ooConfGetFloatDef("openorbit/sim/period", &sim_period, wc_period);


  OOsimtime *timeState = malloc(sizeof(OOsimtime));
  timeState->epoch = epoch;
  timeState->jdBase = (double)(time(NULL)/86400) + 2440587.5;
  timeState->currentTime = timeState->jdBase;
  timeState->timeStamp = 0;
  timeState->timeStampLength = 0.05; // 20 Hz default

  return timeState;
}


double
ooTimeGetJD(void)
{
  OOsimtime *ts = ooSimTimeState();
  return ts->currentTime;
}

time_t
ooTimeGetTime(void)
{
  OOsimtime *ts = ooSimTimeState();
  return ts->epoch + (time_t)ts->timeStamp * (time_t)(1.0/ts->timeStampLength);
}

time_t
ooTimeGetEpoch(void)
{
  OOsimtime *ts = ooSimTimeState();
  return ts->epoch;
}

void
ooTimeSetEpoch(time_t epoch)
{
  OOsimtime *ts = ooSimTimeState();
  ts->epoch = epoch;
}
