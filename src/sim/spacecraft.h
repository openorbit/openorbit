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
  OO_Engine_Fault_Closed,
  OO_Engine_Fault_Open
} OOenginestate;


typedef struct OOstage {
  OOobjvector engines;
  dBodyID id;
} OOstage;


typedef struct OOspacecraft {
  OOobjvector engines;
//  OOengine *mainEngine;
//  OOobjvector stages;
  dBodyID body;
} OOspacecraft;

typedef struct OOengine {
  OOspacecraft *sc;
  OOenginestate state;
  vector_t p;
  float forceMag; //!< Newton
  vector_t dir; //!< Unit vector with direction of thruster
} OOengine;


void ooScDetatchStage(OOspacecraft *sc);
void ooScStep(OOspacecraft *sc);
void ooScForce(OOspacecraft *sc, float rx, float ry, float rz);
OOspacecraft* ooScGetCurrent(void);

OOspacecraft* ooScLoad(const char *file);

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
