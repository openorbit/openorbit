/*
  Copyright 2006,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIM_H__
#define SIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "physics/physics.h"
#include "physics/orbit.h"
#include "rendering/scenegraph.h"
#include "sim/spacecraft.h"
#include "sim/simtime.h"
#include "sim/simevent.h"

typedef struct {
  float stepSize;     //!< Step size for simulation in seconds
  sim_spacecraft_t *currentSc; //!< Current active spacecraft
  PLsystem *orbSys;   //!< Root orbit system, this will be the sun initially
  PLworld *world;
  SGscenegraph *sg;   //!< Scenegraph of the world
} SIMstate;

void ooSimInit(void);

void ooSimSetSg(SGscenegraph *sg);

void ooSimSetOrbSys(PLsystem *osys);
void ooSimSetOrbWorld(PLworld *world);

void ooSimStep(float dt);

void simSetSpacecraft(sim_spacecraft_t *sc);
sim_spacecraft_t* simGetSpacecraft(void);
OOeventqueue* simGetEventQueue(void);
SGscenegraph* simGetSg(void);
PLworld* simGetWorld(void);

#ifdef __cplusplus
}
#endif

#endif /* ! SIM_H__ */
