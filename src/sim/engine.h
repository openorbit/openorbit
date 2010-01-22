/*
 Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef OO_SIM_ENGINE_H__
#define OO_SIM_ENGINE_H__
#include <vmath/vmath.h>
#include <gencds/array.h>

#include "simtypes.h"

typedef enum OOdefault_actuator_groups {
  OO_Act_Orbital = 0,
  OO_Act_Vertical = 1,
  OO_Act_Horisontal = 2,
  OO_Act_Forward = 3,
  OO_Act_Pitch = 4,
  OO_Act_Roll = 5,
  OO_Act_Yaw = 6,
  OO_Act_Group_Count
} OOdefault_actuator_groups;


typedef enum OOactuatorstate {
  OO_Act_Disabled,
  OO_Act_Idle,
  OO_Act_Burning,
  OO_Act_Fault_Closed,
  OO_Act_Fault_Open,
  OO_Act_Spinning
} OOactuatorstate;

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

// Note engine structure is for both engines and RCS thrusters

typedef void (*OOenginetoggle)(OOengine *);
typedef void (*OOenginethrottle)(OOengine *, float throttle);
typedef void (*OOenginestep)(OOengine *, float dt);

typedef void (*OOactuatortoggle)(OOengine *);
typedef void (*OOactuatorthrottle)(OOengine *, float throttle);
typedef void (*OOactuatorstep)(OOengine *, float dt);


// Base actuator class, do not instantiate directly
struct OOactuator {
  OOspacecraft *sc;
  const char *name; //!< Engine name, e.g. "Orbital Maneuvering Engine"
  OOactuatorstate state;
  OOactuatortoggle toggleOn;
  OOactuatortoggle toggleOff;
  OOactuatorstep step;
};
// Base engine class, do not instantiate directly
struct OOengine {
  OOactuator super;
  float3 p; //!< Local position relative to stage center
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  float3 dir; //!< Unit vector with direction of thruster
  OOenginethrottle adjustThrottle;
};
// Liquid oxygen engine
struct OOlox_engine {
  OOengine super;
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  OOenginethrottle adjustThrottle;
};

// Solid rocket booster engine
struct OOsrb_engine {
  OOengine super;
  float forceMag; //!< Newton
};

// In atmosphere jet engine
struct OOjet_engine {
  OOengine super;
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
};

// Maneuvering thruster
struct OOthruster {
  OOengine super;
};

// Used for grouping actuators that are activated by the same command, for
// example roll thrusters would typically fire on two sides of the spacecraft to
// prevent translational movement.
struct OOactuatorgroup {
  const char *groupName;
  obj_array_t actuators;
};

// The torquer structure is for more general torquers (that are not mass expelling
// thrusters). These include magnetotorquers and anything else that include rotating
// bodies.
typedef struct OOtorquer {
  OOactuator super;
  OOspacecraft *sc;
  OOtorquerstate state;
  float torque; //!< Nm
  float3 torqueAxis; //!< Unit vector around which the torque is to be applied
} OOtorquer;

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

OOactuatorgroup* ooScNewActuatorGroup(OOspacecraft *sc, const char *name);
void ooScRegisterInGroup(OOactuatorgroup *eg, OOactuator *actuator);

// Standard engine groups are:
// Main: orbital
// Stationkeeping: vertical, horisontal, forward/reverse
// Rotation: yaw, roll, pitch
void ooScFireOrbital(OOspacecraft *sc);
void ooScFireVertical(OOspacecraft *sc, float dv);
void ooScFireHorizontal(OOspacecraft *sc, float dh);
void ooScFireForward(OOspacecraft *sc);
void ooScFireReverse(OOspacecraft *sc);

void ooScEngageYaw(OOspacecraft *sc, float dy);
void ooScEngagePitch(OOspacecraft *sc, float dp);
void ooScEngageRoll(OOspacecraft *sc, float dr);

#endif /* ! OO_SIM_ENGINE_H__ */
