/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include "io-manager.h"
#include "sim.h"
#include "sim/spacecraft.h"
#include "sim/spacecraft-control.h"
#include <openorbit/log.h>

static void
sim_spacecraft_control_main_engine_toggle(int state, void *data)
{
  if (state) {
    sim_spacecraft_t *sc = sim_get_spacecraft();
    log_info("main engine toggle");
    sim_spacecraft_toggle_main_engine(sc);
  }
}

static void
sim_spacecraft_control_detatch_stage(int state, void *data)
{
  log_info("detatch commanded 0");

  if (state) {
    sim_spacecraft_t *sc = sim_get_spacecraft();
    log_info("detatch stage");

    if (sc->detatchPossible) {
      sc->detatchStage(sc);
      sc->detatchSequence ++;
    }
  }
}

void
sim_spacecraft_control_init(void)
{
  io_reg_action_handler("main-engine-toggle",
                        sim_spacecraft_control_main_engine_toggle,
                        IO_BUTTON_PUSH, NULL);
  io_reg_action_handler("detatch-stage", sim_spacecraft_control_detatch_stage,
                        IO_BUTTON_PUSH, NULL);
}
