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
#include "physics/reftypes.h"
typedef enum OOdefault_actuator_groups {
  OO_Act_Group_First = 0,
  OO_Act_Orbital = 0,
  OO_Act_Vertical = 1,
  OO_Act_Horisontal = 2,
  OO_Act_Forward = 3,
  OO_Act_Pitch = 4,
  OO_Act_Roll = 5,
  OO_Act_Yaw = 6,
  OO_Act_Group_Last = 6,
  OO_Act_Group_Count
} OOdefault_actuator_groups;


typedef enum OOactuatorstate {
  OO_Act_Disarmed = 0x00,
  OO_Act_Armed = 0x01,
  OO_Act_Enabled = 0x82,
  OO_Act_Locked_Open = 0x83,
  OO_Act_Locked_Closed = 0x04,
  OO_Act_Fault_Closed = 0x05,
  OO_Act_Fault_Open = 0x86,
} OOactuatorstate;

#define SIM_ACTUATOR_ON_MASK 0x80

typedef void (*OOactuatortoggle)(OOactuator *);
typedef void (*OOactuatoraxisupdate)(OOactuator *, float axis);
typedef void (*OOactuatorstep)(OOactuator *, float dt);


// Base actuator class, do not instantiate directly
struct OOactuator {
  OOstage *stage;
  const char *name; //!< Engine name, e.g. "Orbital Maneuvering Engine"
  OOactuatorstate state;
  OOactuatortoggle toggleOn;
  OOactuatortoggle toggleOff;
  OOactuatorstep step;
  OOactuatoraxisupdate axisUpdate; // Actuator should use this to update the
                                   // io input, usually an axis
};


// Will arm the actuator if it is disarmed
void simArmActuator(OOactuator *act);
void simDisarmActuator(OOactuator *act);
void simFireActuator(OOactuator *act);
void simDisableActuator(OOactuator *act);
void simLockActuator(OOactuator *act);
void simFailActuator(OOactuator *act);

struct SIMthruster {
  OOactuator super;
  PLparticles *ps;
  float3 fMax;
  float3 pos;
  float throttle;
};

struct SIMtorquer {
  OOactuator super;

  float3 tMax;
  float3 tMin;
  float3 pos;
  float setting;
};

typedef struct SIMtorquer SIMtorquer;
typedef struct SIMthruster SIMthruster;

void simInitTorquer(SIMtorquer *tq, const char *name, float3 pos, float3 tMax, float3 tMin);
SIMtorquer* simNewTorquer(const char *name, float3 pos, float3 tMax, float3 tMin);
void simInitThruster(SIMthruster *th, const char *name, float3 pos, float3 fMax);
SIMthruster* simNewThruster(const char *name, float3 pos, float3 fMax);
void simAddActuator(OOstage *stage, OOactuator *act);

void simSetTorquerPower(SIMtorquer *tq, float v);
void simSetThrottle(SIMthruster *th, float throttle);
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


OOactuatorgroup* ooScNewActuatorGroup(const char *name);
void ooScRegisterInGroup(OOactuatorgroup *eg, OOactuator *actuator);

// Standard engine groups are:
// Main: orbital
// Stationkeeping: vertical, horisontal, forward/reverse
// Rotation: yaw, roll, pitch

int ooGetActuatorGroupId(const char *groupName);
const char * ooGetActuatorGroupName(unsigned groupId);


#endif /* ! OO_SIM_ENGINE_H__ */
