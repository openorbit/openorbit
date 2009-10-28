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
#ifndef SIMTIME_H_STPF70AM
#define SIMTIME_H_STPF70AM

typedef struct OOsimtime {
  time_t epoch; // UNIX time
  double currentTime; //!< Current time in earth days relative to epoch
  uint64_t timeStamp; //!< Time stamp (ticking up one every step)
  double timeStampLength;
} OOsimtime;

OOsimtime* ooSimTimeInit(time_t epoch);

static inline void
ooSimTimeTick(OOsimtime *self, double dt)
{
  self->timeStamp ++;
  self->currentTime = (((double)(self->timeStamp)) * dt) / (24.0 * 3600.0);
}

double ooTimeGetJD(void);
time_t ooTimeGetTime(void);

time_t ooTimeGetEpoch(void);
void ooTimeSetEpoch(time_t epoch);

#endif /* end of include guard: SIMTIME_H_STPF70AM */
 
