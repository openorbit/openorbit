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
#include "rendering/types.h"
#include "physics/physics.h"
#include "physics/orbit.h"
#include "rendering/scenegraph.h"
#include "sim/spacecraft.h"
#include "sim/simtime.h"
#include "sim/simevent.h"

typedef struct {
  float stepSize;     //!< Step size for simulation in seconds
  sim_spacecraft_t *currentSc; //!< Current active spacecraft
  pl_system_t *orbSys;   //!< Root orbit system, this will be the sun initially
  pl_world_t *world;
  sg_window_t *win;
} sim_state_t;

void sim_init(void);

  sg_scene_t* sim_get_scene(void);
void sim_set_orb_sys(pl_system_t *osys);
void sim_set_orb_world(pl_world_t *world);

void sim_step(float dt);

void sim_set_spacecraft(sim_spacecraft_t *sc);
sim_spacecraft_t* sim_get_spacecraft(void);
sim_event_queue_t* sim_get_event_queue(void);
pl_world_t* sim_get_world(void);

#ifdef __cplusplus
}
#endif

#endif /* ! SIM_H__ */
