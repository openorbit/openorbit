/*
  Copyright 2006,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "sim.h"

#include "physics/orbit.h"
#include "rendering/planet.h"
#include "settings.h"

#include "log.h"

SIMstate gSIM_state = {0.0, NULL, NULL, NULL, NULL};

void
ooSimInit(void) {
  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Read in Hz
  gSIM_state.stepSize = 1.0 / freq; // Period in s
  gSIM_state.evQueue = simNewEventQueue();
}


void
ooSimSetSg(SGscenegraph *sg)
{
  gSIM_state.sg = sg;
}


void
ooSimSetOrbSys(PLsystem *osys)
{
  gSIM_state.orbSys = osys;
}

void
ooSimSetOrbWorld(PLworld *world)
{
  gSIM_state.orbWorld = world;
}


void
ooSimStep(float dt)
{
  simTimeTick(dt);
  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);

  sgCamStep(sgGetCam(gSIM_state.sg), dt);

  plWorldClear(gSIM_state.orbWorld);
  plWorldStep(gSIM_state.orbWorld, dt);

  gettimeofday(&end, NULL);

  ooLogTrace("simstep time: %lu us", ((end.tv_sec*1000000 + end.tv_usec) -
                                     (start.tv_sec*1000000 + start.tv_usec)));

  // Step spacecraft systems
  simScStep(gSIM_state.currentSc, dt);
  simDispatchPendingEvents(gSIM_state.evQueue);
}

void
simSetSpacecraft(OOspacecraft *sc)
{
  gSIM_state.currentSc = sc;
}
