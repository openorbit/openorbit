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
 
#ifndef SPACECRAFT_H_7SCB1CH8
#define SPACECRAFT_H_7SCB1CH8

#include <ode/ode.h>

#include <openorbit/openorbit.h>
#include <vmath/vmath.h>

typedef enum OOenginestate {
  OO_Engine_Disabled,
  OO_Engine_Enabled,
  OO_Engine_Fault
} OOenginestate;

typedef struct OOengine {
  OOenginestate state;
  v4f_t p;
} OOengine;

typedef struct OOstage {
  OOobjvector engines;
  dBodyID id;
} OOstage;


typedef struct OOspacecraft {
  OOengine *mainEngine;
  OOobjvector stages;
  dBodyID id;
} OOspacecraft;

void ooScDetatchStage(OOspacecraft *sc);
void ooScStep(OOspacecraft *sc);
void ooScForce(OOspacecraft *sc, v4f_t f);
OOspacecraft* ooScGetCurrent();

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
