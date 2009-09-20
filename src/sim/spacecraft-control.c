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
ooScReadAxis(OOspacecraft *sc)
{
  // Nice thing is that these return 0.0 if they are not assigned
  float yaw = ooIoGetAxis("yaw");
  float pitch = ooIoGetAxis("pitch");
  float roll = ooIoGetAxis("roll");
  float horizontal = ooIoGetAxis("horizontal");
  float vertical = ooIoGetAxis("vertical");

  ooScFireHorizontal(sc, horizontal);
  ooScFireVertical(sc, vertical);

  ooScEngageYaw(sc, yaw);
  ooScEngagePitch(sc, pitch);
  ooScEngageRoll(sc, roll);
}

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
    
//    if (sc->mainEngine) {
//      switch (sc->mainEngine->state) {
//      case OO_Engine_Disabled:
//        ooLogInfo("firing main engine");
//        sc->mainEngine->state = OO_Engine_Enabled;
//        break;
//      case OO_Engine_Enabled:
//        ooLogInfo("cutting main engine");
//        sc->mainEngine->state = OO_Engine_Disabled;
//        break;
//      case OO_Engine_Fault:
//        ooLogInfo("engine toggle on faulty engine");
//        break;
//      default:
//        assert(0 && "invalid case");
//      }
//    }
  }
}

void
ooSimScCtrlRcsYawLeft(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out yaw
  } else {
    // apply yaw
  }
}

void
ooSimScCtrlRcsYawRight(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out yaw
  } else {
    // apply yaw
  }
}

void
ooSimScCtrlRcsPitchUp(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out pitch
  } else {
    // apply pitch
  }
}

void
ooSimScCtrlRcsPitchDown(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out pitch
  } else {
    // apply pitch
  }
}

void
ooSimScCtrlRcsRollLeft(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out roll
  } else {
    // apply roll
  }
}

void
ooSimScCtrlRcsRollRight(bool buttonUp, void *data)
{
  OOspacecraft *sc = ooScGetCurrent();

  if (buttonUp) {
    // zero out roll
  } else {
    // apply roll
  }
}
