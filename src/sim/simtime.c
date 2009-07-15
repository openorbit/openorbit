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
  OOsimtime *timeState = malloc(sizeof(OOsimtime));
  timeState->epoch = epoch;
  timeState->currentTime = 0.0;
  timeState->timeStamp = 0;
  timeState->timeStampLength = 0.05; // 20 Hz default
  
  return timeState;
}


float
ooTimeGetJD(OOsimtime *self)
{
  return self->currentTime;
}

time_t
ooTimeGetTime(OOsimtime *self)
{
  return self->epoch + (time_t)self->timeStamp * (time_t)(1.0/self->timeStampLength);
}


time_t
ooTimeGetEpoch(OOsimtime *self)
{
  return self->epoch;
}

void
ooTimeSetEpoch(OOsimtime *self, time_t epoch)
{
  self->epoch = epoch;
}
