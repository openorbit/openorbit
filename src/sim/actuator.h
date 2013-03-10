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
#include "rendering/types.h"
#include "rendering/particles.h"
#include "rendering/scenegraph.h"
#include "sim/propellant-tank.h"

typedef enum {
  SIM_ACT_GROUP_FIRST = 0,
  SIM_ACT_ORBITAL = 0,
  SIM_ACT_VERTICAL = 1,
  SIM_ACT_HORISONTAL = 2,
  SIM_ACT_FORWARD = 3,
  SIM_ACT_PITCH = 4,
  SIM_ACT_ROLL = 5,
  SIM_ACT_YAW = 6,
  SIM_ACT_GROUP_LAST = 6,
  SIM_ACT_GROUP_COUNT
} sim_default_actuator_groups_t;


typedef enum {
  SIM_THRUSTER,
  SIM_PROP,
  SIM_TURBO_PROP,
  SIM_JET,
  SIM_LIQUID_ROCKET,
  SIM_SOLID_ROCKET,
} sim_enginekind_t;

typedef enum {
  SIM_DISARMED = 0x00,
  SIM_ARMED = 0x01,
  SIM_BURNING = 0x82,
  SIM_LOCKED_BURNING = 0x83,
  SIM_LOCKED_CLOSED = 0x04,
  SIM_FAULT_CLOSED = 0x05,
  SIM_FAULT_BURNING = 0x86,
} sim_enginestate_t;

#define SIM_ENGINE_BURNING_BIT 0x80

typedef enum {
  SIM_CIRCULAR,
  SIM_END_BURNER,
  SIM_C_SLOT,
  SIM_MOON_BURNER,
  SIM_FINOCYL,
} sim_grainkind_t;


struct sim_engine_t {
  const char *name;
  sim_stage_t *stage;
  sim_enginekind_t kind;
  sim_enginestate_t state;
  float throttle;
  float3 pos;
  float3 dir;
  obj_array_t fuelTanks;
  //SGparticles *psys;
  void (*step)(struct sim_engine_t*, float);
};

typedef struct sim_engine_t sim_engine_t;

struct sim_thruster_t {
  sim_engine_t super;
  float3 fMax;
};
struct sim_propengine_t {
  sim_engine_t super;
};
struct sim_turbopropengine_t {
  sim_engine_t super;
};
struct sim_jetengine_t {
  sim_engine_t super;
};
struct sim_liquidrocketengine_t {
  sim_engine_t super;
  obj_array_t oxidiserTanks;
};
struct sim_solid_rocketengine_t {
  sim_engine_t super;
  sim_grainkind_t grain;
  float_array_t area_function;
};

typedef struct sim_thruster_t sim_thruster_t;
typedef struct sim_propengine_t sim_propengine_t;
typedef struct sim_turbopropengine_t sim_turbopropengine_t;
typedef struct sim_jetengine_t sim_jetengine_t;
typedef struct sim_liquidrocketengine_t sim_liquidrocketengine_t;
typedef struct sim_solid_rocketengine_t sim_solid_rocketengine_t;

sim_engine_t * sim_new_engine(char *name, sim_stage_t *stage, sim_enginekind_t kind,
                         sim_enginestate_t state, float throttle,
                         float3 pos, float3 dir);
void sim_engine_add_tank(sim_engine_t *engine, sim_tank_t *tank);
void sim_engine_add_oxidiser_tank(sim_engine_t *engine, sim_tank_t *tank);
void sim_engine_set_grain_type(sim_engine_t *engine, sim_grainkind_t grain);
void sim_engine_step(sim_engine_t *engine, double dt);
void sim_engine_set_throttle(sim_engine_t *engine, float throttle);
float sim_engine_get_throttle(sim_engine_t *engine);
void sim_engine_fire(sim_engine_t *eng);
void sim_engine_arm(sim_engine_t *eng);
void sim_engine_disarm(sim_engine_t *eng);
void sim_engine_disable(sim_engine_t *eng);
void sim_engine_lock(sim_engine_t *eng);
void sim_engine_unlock(sim_engine_t *eng);
void sim_engine_fail(sim_engine_t *eng);

// Used for grouping actuators that are activated by the same command, for
// example roll thrusters would typically fire on two sides of the spacecraft to
// prevent translational movement.
typedef enum sim_actsequencetype_t {
  OO_Act_Parallel,
  OO_Act_Serial,
  OO_Act_Ripple // Special case of serial
} sim_actsequencetype_t;

struct sim_actuatorgroup_t {
  const char *groupName;
  sim_actsequencetype_t seqType;
  float seqParam0; // Sequence parameters
  obj_array_t actuators;
};


sim_actuatorgroup_t* sim_new_actuator_group(const char *name);

// Standard engine groups are:
// Main: orbital
// Stationkeeping: vertical, horisontal, forward/reverse
// Rotation: yaw, roll, pitch

int sim_get_actuator_group_id(const char *groupName);
const char * sim_get_actuator_group_name(unsigned groupId);


#endif /* ! OO_SIM_ENGINE_H__ */
