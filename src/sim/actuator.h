/*
 Copyright 2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef OO_SIM_ENGINE_H__
#define OO_SIM_ENGINE_H__
#include <vmath/vmath.h>
#include <gencds/array.h>

#include "simtypes.h"
#include "physics/reftypes.h"
#include "rendering/reftypes.h"
#include "rendering/particles.h"
#include "rendering/scenegraph.h"
#include "sim/propellant-tank.h"

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


typedef enum {
  SIM_Thruster,
  SIM_Prop,
  SIM_TurboProp,
  SIM_Jet,
  SIM_LiquidRocket,
  SIM_SolidRocket,
} SIMenginekind;

typedef enum {
  SIM_Disarmed = 0x00,
  SIM_Armed = 0x01,
  SIM_Burning = 0x82,
  SIM_Locked_Burning = 0x83,
  SIM_Locked_Closed = 0x04,
  SIM_Fault_Closed = 0x05,
  SIM_Fault_Burning = 0x86,
} SIMenginestate;

#define SIM_ENGINE_BURNING_BIT 0x80

typedef enum {
  SIM_Circular,
  SIM_End_Burner,
  SIM_C_Slot,
  SIM_Moon_Burner,
  SIM_Finocyl,
} SIMgrainkind;


struct SIMengine {
  const char *name;
  sim_stage_t *stage;
  SIMenginekind kind;
  SIMenginestate state;
  float throttle;
  float3 pos;
  float3 dir;
  obj_array_t fuelTanks;
  SGparticles *psys;
  void (*step)(struct SIMengine*, float);
};

typedef struct SIMengine SIMengine;

struct SIMthrust {
  SIMengine super;
  float3 fMax;
};
struct SIMpropengine {
  SIMengine super;
};
struct SIMturbopropengine {
  SIMengine super;
};
struct SIMjetengine {
  SIMengine super;
};
struct SIMliquidrocketengine {
  SIMengine super;
  obj_array_t oxidiserTanks;
};
struct SIMsolidrocketengine {
  SIMengine super;
  SIMgrainkind grain;
  float_array_t area_function;
};
typedef struct SIMthrust SIMthrust;
typedef struct SIMpropengine SIMpropengine;
typedef struct SIMturbopropengine SIMturbopropengine;
typedef struct SIMjetengine SIMjetengine;
typedef struct SIMliquidrocketengine SIMliquidrocketengine;
typedef struct SIMsolidrocketengine SIMsolidrocketengine;

SIMengine * simNewEngine(char *name, sim_stage_t *stage, SIMenginekind kind,
                         SIMenginestate state, float throttle,
                         float3 pos, float3 dir);
void simEngineAddTank(SIMengine *engine, SIMtank *tank);
void simEngineAddOxidiserTank(SIMengine *engine, SIMtank *tank);
void simEngineSetGrainType(SIMengine *engine, SIMgrainkind grain);
void simEngineStep(SIMengine *engine, double dt);
void simEngineSetThrottle(SIMengine *engine, float throttle);
float simEngineGetThrottle(SIMengine *engine);
void simEngineFire(SIMengine *eng);
void simEngineArm(SIMengine *eng);
void simEngineDisarm(SIMengine *eng);
void simEngineDisable(SIMengine *eng);
void simEngineLock(SIMengine *eng);
void simEngineUnlock(SIMengine *eng);
void simEngineFail(SIMengine *eng);

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

// Standard engine groups are:
// Main: orbital
// Stationkeeping: vertical, horisontal, forward/reverse
// Rotation: yaw, roll, pitch

int ooGetActuatorGroupId(const char *groupName);
const char * ooGetActuatorGroupName(unsigned groupId);


#endif /* ! OO_SIM_ENGINE_H__ */
