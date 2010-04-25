/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

typedef struct OOsimtime {
  double currentTime; //!< Current time in earth days relative to epoch
  int64_t timeStamp; //!< Discrete time stamp in ms since standard UNIX epoch, this allow for +/- 290 M year sim around this point
  double jdBase;
} OOsimtime;

static OOsimtime gTimeState;

void __attribute__ ((constructor)) simTimeInit(void)
{
  time_t currentTime = time(NULL);
  gTimeState.jdBase = (double)(currentTime/86400) + 2440587.5;
  gTimeState.currentTime = gTimeState.jdBase;
  gTimeState.timeStamp = currentTime*1000;
}

void
simTimeTick(double dt)
{
  gTimeState.timeStamp += dt * 1000.0;
  gTimeState.currentTime = gTimeState.jdBase + ((double)(gTimeState.timeStamp)) / (24.0 * 3600.0 * 1000.0);
}

void
simTimeTick_ms(int64_t dms)
{
  gTimeState.timeStamp += dms;
  gTimeState.currentTime = gTimeState.jdBase + ((double)(gTimeState.timeStamp)) / (24.0 * 3600.0 * 1000.0);
}


int64_t
simTimeJDToTimeStamp(double jd)
{
  return (int64_t) (jd - 2440587.5) * 86400.0 * 1000.0;
}


int64_t
ooTimeJDToTimeStamp(double jd)
{
  return (uint64_t) (jd - 2440587.5) * 86400.0 * 1000.0;
}

double
simTimeGetJD(void)
{
  return gTimeState.currentTime;
}


time_t
simTimeGetTime(void)
{
  return gTimeState.timeStamp/1000;
}

int64_t
simTimeGetTimeStamp(void)
{
  return gTimeState.timeStamp;
}
