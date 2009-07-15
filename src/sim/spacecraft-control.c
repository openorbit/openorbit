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
#include "sim/spacecraft.h"
#include "log.h"

void ooSimScCtrlMainEngineToggle(bool buttonUp, void *data);

void
ooSimScCtrlInit()
{
  ooIoRegCKeyHandler("sc-main-engine-toggle", ooSimScCtrlMainEngineToggle);
}

void
ooSimScCtrlMainEngineToggle(bool buttonUp, void *data)
{
  if (buttonUp) {
    OOspacecraft *sc = ooScGetCurrent();
    
    if (sc->mainEngine) {
      switch (sc->mainEngine->state) {
      case OO_Engine_Disabled:
        ooLogInfo("firing main engine");
        sc->mainEngine->state = OO_Engine_Enabled;
        break;
      case OO_Engine_Enabled:
        ooLogInfo("cutting main engine");
        sc->mainEngine->state = OO_Engine_Disabled;
        break;
      case OO_Engine_Fault:
        ooLogInfo("engine toggle on faulty engine");
        break;
      default:
        assert(0 && "invalid case");
      }
    }
  }
}
