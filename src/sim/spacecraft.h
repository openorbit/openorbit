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
#include <gencds/array.h>

#include <vmath/vmath.h>
#include "simenvironment.h"
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
  float mass; // Mass of stage in kg
  float inertia[3]; // Inertial tensor
  obj_array_t engines; // Main orbital engines
  obj_array_t torquers;
  dBodyID id; // ID if detatched
} OOstage;

typedef struct OOspacecraft OOspacecraft;
typedef void (*OOscstepfunc)(OOspacecraft*, double dt);
typedef void (*OOscdetatchfunc)(OOspacecraft*); // Detatch stages

// Wing for lifting design
typedef struct OOwing OOwing;
typedef void (*OOwingstep)(OOwing *, const OOsimenv *env);
struct OOwing {
  OOstage *stage;
  OOwingstep step;
};

typedef struct OOsimplewing OOsimplewing;
struct OOsimplewing {
  OOwing super;
  double area;
  double liftCoeffs[72]; // one per 5 degree aoa
};


/*
  Some notes that should be elsewhere:
    When detatching a stage, the spaccrafts cog and inertia tensors need
    to be recompuded from the remaining stages.

    Even though cog may vary due to fuel tanks in the wrong place, we do not
    simulate cog movement or mutation of the inertial tensor due to shape
    difference. The inertia tensor of the sc may however be updated due to
    mass difference caused by fuel consumption and so on.
 */

struct OOspacecraft {
  obj_array_t stages;
  int activeStageIdx; // index in stage vector of active stage, this point out where
                      // detachment happens
  dBodyID body;
  float mass; // Mass of sc in kg
  float inertia[3]; // Inertial tensor
  
  OOscstepfunc prestep;
  OOscstepfunc poststep;
  OOscdetatchfunc detatchStage;
};

// Note engine structure is for both engines and RCS thrusters


typedef struct OOengine OOengine;

typedef void (*OOenginetoggle)(OOengine *);
typedef void (*OOenginestep)(OOengine *, float dt);

// Base engine class, do not instanciate directly
struct OOengine {
  OOspacecraft *sc;
  const char *name; //!< Engine name, e.g. "Orbital Maneuvering Engine"
  OOenginestate state;
  float3 p;
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  float3 dir; //!< Unit vector with direction of thruster

  OOenginetoggle toggleOn;
  OOenginetoggle toggleOff;

  OOenginetoggle throttleUp;
  OOenginetoggle throttleDown;

  OOenginestep step;
};

// Liquid oxygen engine
struct OOlox_engine {
  OOengine super;
};

// Solid rocket booster engine
struct OOsrb_engine {
  OOengine super;
};

// In atmosphere jet engine
struct OOjet_engine {
  OOengine super;
};

// Maneuvering thruster
struct OOthruster {
  OOengine super;
};


// The torquer structure is for more general torquers (that are not mass expelling
// thrusters). These include magnetotorquers and anything else that include rotating
// bodies.
typedef struct OOtorquer {
  OOspacecraft *sc;
  OOtorquerstate state;
  float torque; //!< Nm
  float3 torqueAxis; //!< Unit vector around which the torque is to be applied
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

OOengine* ooScNewSrb(OOspacecraft *sc,
                     float f,
                     float x, float y, float z,
                     float dx, float dy, float dz);

OOengine* ooScNewLoxEngine(OOspacecraft *sc,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz,
                           float fuelPerNmPerS);

OOengine* ooScNewJetEngine(OOspacecraft *sc,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz);

OOengine* ooScNewThruster(OOspacecraft *sc,
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
