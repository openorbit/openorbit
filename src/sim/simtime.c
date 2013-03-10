/*
  Copyright 2009,2010,2013 Mattias Holm <lorrden(at)openorbit.org>

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


#include "sim.h"
#include "sim/simtime.h"

typedef struct sim_time_context_t {
  double currentTime; //!< Current time in earth days relative to epoch
  int64_t timeStamp; //!< Discrete time stamp in ms since standard UNIX epoch, this allow for +/- 290 M year sim around this point
} sim_time_context_t;

static sim_time_context_t gTimeState;

void __attribute__ ((constructor)) simTimeInit(void)
{
  time_t currentTime = time(NULL);
  gTimeState.timeStamp = currentTime*1000;
  gTimeState.currentTime = (double)(currentTime/86400.0) + 2440587.5;
}

void
sim_time_tick(double dt)
{
  gTimeState.timeStamp += dt * 1000.0;
  gTimeState.currentTime = (double)(gTimeState.timeStamp/86400.0)/1000.0 + 2440587.5;
}

void
sim_time_tick_ms(int64_t dms)
{
  gTimeState.timeStamp += dms;
  gTimeState.currentTime = (double)(gTimeState.timeStamp/86400.0)/1000.0 + 2440587.5;
}


int64_t
sim_time_jd_to_time_stamp(double jd)
{
  return (int64_t) (jd - 2440587.5) * 86400.0 * 1000.0;
}


double
sim_time_get_jd(void)
{
  return gTimeState.currentTime;
}


time_t
sim_time_get_time(void)
{
  return gTimeState.timeStamp/1000;
}

int64_t
sim_time_get_time_stamp(void)
{
  return gTimeState.timeStamp;
}
