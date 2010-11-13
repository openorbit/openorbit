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

#include <stdbool.h>
#include "io-manager.h"
#include "sim.h"
#include "sim/spacecraft.h"
#include "sim/spacecraft-control.h"
#include "log.h"

static void
simScCtrlMainEngineToggle(bool down, void *data)
{
  if (down) {
    OOspacecraft *sc = simGetSpacecraft();
    ooLogInfo("main engine toggle");
    simScToggleMainEngine(sc);
  }
}

static void
simScCtrlDetatchStage(bool down, void *data)
{
  ooLogInfo("detatch commanded 0");

  if (down) {
    OOspacecraft *sc = simGetSpacecraft();
    ooLogInfo("detatch stage");
    simScDetatchStage(sc);
  }
}

void
simScCtrlInit(void)
{
  ooIoRegCKeyHandler("main-engine-toggle", simScCtrlMainEngineToggle);
  ooIoRegCKeyHandler("detatch-stage", simScCtrlDetatchStage);
}
