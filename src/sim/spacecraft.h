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

/*!
    Spacecraft system simulation header

    The spacecrafts consist of multiple stages, where several stages may be active at the
    same time (e.g. space shuttle where 2 SRBs are burning at the same time).
*/

typedef enum OOenginestate {
  OO_Engine_Disabled,
  OO_Engine_Idle,
  OO_Engine_Burning,
  OO_Engine_Fault_Closed,
  OO_Engine_Fault_Open
} OOenginestate;

typedef enum OOtorquerstate {
  OO_Torquer_Disabled,
  OO_Torquer_Idle,
  OO_Torquer_Spinning,
  OO_Torquer_Fault_Stuck,
  OO_Torquer_Fault_Spinning
} OOtorquerstate;

typedef enum OOstagestate {
  OO_Stage_Idle,
  OO_Stage_Enabled
} OOstagestate;

typedef struct OOstage {
  OOstagestate state;
  int detachOrder; // How and when to detach the stage
  float mass;
  float inertia[3];
  OOobjvector engines; // Main orbital engines
  OOobjvector torquers;
  dBodyID id;
} OOstage;

typedef struct OOspacecraft OOspacecraft;
typedef void (*OOscstepfunc)(OOspacecraft*, double dt);

struct OOspacecraft {
  OOobjvector stages;
  int activeStageIdx; // index in stage vector of active stage, this point out where
                      // detachment happens
  dBodyID body;
  
  OOscstepfunc prestep;
  OOscstepfunc poststep;
};

// Note engine structure is for both engines and RCS thrusters
typedef enum OOenginetype {
  OO_Engine_Solid,
  OO_Engine_Liquid,
  OO_Engine_Jet
} OOenginetype;

typedef struct OOengine {
  OOenginetype kind;
  OOspacecraft *sc;
  OOenginestate state;
  vector_t p;
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  vector_t dir; //!< Unit vector with direction of thruster
} OOengine;

typedef OOengine OOthruster;


// The torquer structure is for more general torquers (that are not mass expelling
// thrusters). These include magnetotorquers and anything else that include rotating
// bodies.
typedef struct OOtorquer {
  OOspacecraft *sc;
  OOtorquerstate state;
  float torque; //!< Nm
  vector_t torqueAxis; //!< Unit vector around which the torque is to be applied
} OOtorquer;

void ooScDetatchStage(OOspacecraft *sc);
void ooScStep(OOspacecraft *sc);
void ooScStageStep(OOspacecraft *sc, OOstage *stage);
void ooScForce(OOspacecraft *sc, float rx, float ry, float rz);
OOspacecraft* ooScGetCurrent(void);
OOengine* ooScNewEngine(OOspacecraft *sc,
                        float f,
                        float x, float y, float z,
                        float dx, float dy, float dz);

OOstage* ooScNewStage(void);
void ooScStageAddEngine(OOstage *stage, OOengine *engine);

void ooScFireOrbital(OOspacecraft *sc);
void ooScFireVertical(OOspacecraft *sc, float dv);
void ooScFireHorizontal(OOspacecraft *sc, float dh);
void ooScFireForward(OOspacecraft *sc);
void ooScFireReverse(OOspacecraft *sc);

void ooScEngageYaw(OOspacecraft *sc, float dy);
void ooScEngagePitch(OOspacecraft *sc, float dp);
void ooScEngageRoll(OOspacecraft *sc, float dr);


OOspacecraft* ooScLoad(const char *file);

#endif /* end of include guard: SPACECRAFT_H_7SCB1CH8 */
