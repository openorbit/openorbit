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


typedef void (*OOactuatortoggle)(OOactuator *);
typedef void (*OOactuatoraxisupdate)(OOactuator *, float axis);
typedef void (*OOactuatorstep)(OOactuator *, float dt);


// Base actuator class, do not instantiate directly
struct OOactuator {
  OOspacecraft *sc;
  const char *name; //!< Engine name, e.g. "Orbital Maneuvering Engine"
  OOactuatorstate state;
  OOactuatortoggle toggleOn;
  OOactuatortoggle toggleOff;
  OOactuatorstep step;
  OOactuatoraxisupdate axisUpdate; // Actuator should use this to update the
                                   // io input, usually an axis
};


struct OOrocket {
  OOactuator super;
  float3 p; //!< Local position relative to stage center
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
  float3 dir; //!< Unit vector with direction of thruster
};
struct OOsrb {
  OOactuator super;
  float forceMag; //!< Newton
  float3 p; //!< Local position relative to stage center
  float3 dir; //!< Unit vector with direction of srb
};

// In atmosphere jet engine
struct OOjetengine {
  OOactuator super;
  float forceMag; //!< Newton
  float throttle; //!< Percentage of force magnitude to apply
};

// Used for grouping actuators that are activated by the same command, for
// example roll thrusters would typically fire on two sides of the spacecraft to
// prevent translational movement.
typedef enum OOactsequencetype {
  OO_Act_Parallel,
  OO_Act_Serial,
  OO_Act_Ripple // Special case of serial
} OOactsequencetype;
struct OOactuatorgroup {
  const char *groupName;
  OOactsequencetype seqType;
  float seqParam0; // Sequence parameters
  obj_array_t actuators;
};

// The torquer structure is for more general torquers (that are not mass expelling
// thrusters). These include magnetotorquers and anything else that include rotating
// bodies.
struct OOmagtorquer {
  OOactuator super;
  OOspacecraft *sc;
  float torque; //!< Nm
  float3 torqueAxis; //!< Unit vector around which the torque is to be applied
};

OOrocket* ooScNewEngine(OOspacecraft *sc,
                        const char *name,
                        float f,
                        float x, float y, float z,
                        float dx, float dy, float dz);

OOsrb* ooScNewSrb(OOspacecraft *sc,
                  const char *name,
                  float f,
                  float x, float y, float z,
                  float dx, float dy, float dz);

OOrocket* ooScNewLoxEngine(OOspacecraft *sc,
                           const char *name,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz,
                           float fuelPerNmPerS);

OOjetengine* ooScNewJetEngine(OOspacecraft *sc,
                              const char *name,
                              float f,
                              float x, float y, float z,
                              float dx, float dy, float dz);

OOrocket* ooScNewThruster(OOspacecraft *sc,
                          const char *name,
                          float f,
                          float x, float y, float z,
                          float dx, float dy, float dz);

OOactuatorgroup* ooScNewActuatorGroup(const char *name);
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

int ooGetActuatorGroupId(const char *groupName);
const char * ooGetActuatorGroupName(int groupId);


#endif /* ! OO_SIM_ENGINE_H__ */
