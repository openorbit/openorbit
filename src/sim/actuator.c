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


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "common/palloc.h"
#include "sim/actuator.h"
#include "sim/spacecraft.h"
#include <openorbit/log.h>
#include "io-manager.h"

static const char *actuatorNames[SIM_ACT_GROUP_COUNT] = {
  [SIM_ACT_ORBITAL] = "orbital",
  [SIM_ACT_VERTICAL] = "vertical",
  [SIM_ACT_HORISONTAL] = "horisontal",
  [SIM_ACT_FORWARD] = "forward",
  [SIM_ACT_PITCH] = "pitch",
  [SIM_ACT_ROLL] = "roll",
  [SIM_ACT_YAW] = "yaw"
};

int
sim_get_actuator_group_id(const char *groupName)
{
  for (int i = 0 ; i < SIM_ACT_GROUP_COUNT ; ++i) {
    if (!strcmp(actuatorNames[i], groupName)) {
      return i;
    }
  }
  return -1;
}

const char*
sim_get_actuator_group_name(unsigned groupId)
{
  assert(groupId < SIM_ACT_GROUP_COUNT);
  return actuatorNames[groupId];
}

float
sim_get_throttle_for_actuator_group(unsigned groupId)
{
  float throttle = io_get_slider(IO_SLIDER_THROT_0);//ooIoGetAxis(NULL, ooGetActuatorGroupName(groupId));
  return throttle;
}


sim_actuatorgroup_t*
sim_new_actuator_group(const char *name)
{
  sim_actuatorgroup_t *eg = smalloc(sizeof(sim_actuatorgroup_t));
  memset(eg, 0, sizeof(sim_actuatorgroup_t));
  obj_array_init(&eg->actuators);
  eg->groupName = strdup(name);
  return eg;
}

void
thruster_step(sim_engine_t *engine, float dt)
{
  sim_thruster_t *thruster = (sim_thruster_t*)engine;

  pl_object_force_relative_pos3fv(thruster->super.stage->sc->obj,
                        thruster->fMax * thruster->super.throttle,
                        thruster->super.pos);
}


void
liquid_rocket_step(sim_engine_t *engine, float dt)
{
  sim_liquidrocketengine_t *lrocket = (sim_liquidrocketengine_t*)engine;
  (void)lrocket; // TODO
  //  plForceRelativePos3fv(lrocket->super.stage->sc->obj,
  //                      lrocket->fMax * lrocket->super.throttle,
  //                      lrocket->super.pos);
}

void
solid_rocket_step(sim_engine_t *engine, float dt)
{
  sim_solid_rocketengine_t *srocket = (sim_solid_rocketengine_t*)engine;
  switch (srocket->grain) {
  case SIM_CIRCULAR:
  case SIM_END_BURNER:
  case SIM_C_SLOT:
  case SIM_MOON_BURNER:
  case SIM_FINOCYL:
  default:
    assert(0 && "invalid solid rocket type");
  }
  // Thrust F = m Ve + (Pe-Pa)Ae
  //  momentum thrust m Ve: m = rohp tau Ab
  //  pressure thrust (Pe-Pa)Ae
  //  float density = 0.0f;
  // float burn_rate = a0 * pow(Pe, n) + b;
  //float burn_area = 0.0f;
  //float Ve = 0.0f;

  //float momentum_thrust = density * burn_speed * burn_area * Ve;
}

void
jet_step(sim_engine_t *engine, float dt)
{
  sim_jetengine_t *jet = (sim_jetengine_t*)engine;
  (void)jet; // TODO
}

void
prop_step(sim_engine_t *engine, float dt)
{
  sim_propengine_t *prop = (sim_propengine_t*)engine;
  pl_object_force_relative_pos3fv(prop->super.stage->obj, prop->super.dir,
                        prop->super.pos);
  pl_object_torque_relative3fv(prop->super.stage->obj,
                     vf3_set(0.0, 0.0, 0.0), prop->super.pos);
}
void
turboprop_step(sim_engine_t *engine, float dt)
{
  sim_turbopropengine_t *tprop = (sim_turbopropengine_t*)engine;
  (void)tprop; // TODO
}


sim_engine_t*
sim_new_engine(char *name, sim_stage_t *stage, sim_enginekind_t kind,
             sim_enginestate_t state, float throttle, float3 pos, float3 dir)
{
  sim_engine_t *engine = NULL;

  sim_thruster_t *thrust = NULL;
  sim_propengine_t *prop = NULL;
  sim_turbopropengine_t *tprop = NULL;
  sim_jetengine_t *jet = NULL;;
  sim_liquidrocketengine_t *lrocket = NULL;
  sim_solid_rocketengine_t *srocket = NULL;

  switch (kind) {
  case SIM_THRUSTER:
    thrust = smalloc(sizeof(sim_thruster_t));
    thrust->fMax = dir;
    engine = &thrust->super;
    engine->step = thruster_step;
    break;
  case SIM_PROP:
    prop = smalloc(sizeof(sim_propengine_t));
    engine = &prop->super;
    engine->step = prop_step;
    break;
  case SIM_TURBO_PROP:
    tprop = smalloc(sizeof(sim_turbopropengine_t));
    engine = &tprop->super;
    engine->step = turboprop_step;
    break;
  case SIM_JET:
    jet = smalloc(sizeof(sim_jetengine_t));
    engine = &jet->super;
    engine->step = jet_step;
    break;
  case SIM_LIQUID_ROCKET:
    lrocket = smalloc(sizeof(sim_liquidrocketengine_t));
    obj_array_init(&lrocket->oxidiserTanks);
    engine = &lrocket->super;
    engine->step = liquid_rocket_step;
    break;
  case SIM_SOLID_ROCKET:
    srocket = smalloc(sizeof(sim_solid_rocketengine_t));
    engine = &srocket->super;
    engine->step = solid_rocket_step;
    break;
  default:
    assert(0 && "unhandled engine type");
  }
  obj_array_init(&engine->fuelTanks);

  engine->name = strdup(name);
  engine->stage = stage;
  engine->kind = kind;
  engine->state = state;
  engine->throttle = throttle;
  engine->pos = pos;
  engine->dir = vf3_normalise(dir);

  // Create particle system for engine
  //pl_particles_t *psys = plNewParticleSystem(name, 1000);
  //engine->psys = (SGparticles*) sgNewParticleSystem(name,
  //                                                "textures/particle-alpha.png",
  //                                                psys);
  // TODO: Make particle shader
  //sgDrawableLoadShader(&engine->psys->super, "fixed");

  //sgDrawableAddChild(stage->obj->drawable, (SGdrawable*)engine->psys,
  //                   pos, q_rot(1.0f, 0.0f, 0.0f, 0.0f));
  obj_array_push(&stage->engines, engine);
  obj_array_push(&stage->sc->engines, engine);

  return engine;
}

void
sim_engine_add_tank(sim_engine_t *engine, sim_tank_t *tank)
{
  obj_array_push(&engine->fuelTanks, tank);
}

void
sim_engine_add_oxidiser_tank(sim_engine_t *engine, sim_tank_t *tank)
{
  if (engine->kind == SIM_LIQUID_ROCKET) {
    sim_liquidrocketengine_t *lrocket = (sim_liquidrocketengine_t*)engine;
    obj_array_push(&lrocket->oxidiserTanks, tank);
  } else {
    log_fatal("cannot add oxidiser tank to that engine");
  }
}

void
sim_engine_set_grain_type(sim_engine_t *engine, sim_grainkind_t grain)
{
  if (engine->kind == SIM_SOLID_ROCKET) {
    sim_solid_rocketengine_t *srocket = (sim_solid_rocketengine_t*)engine;
    srocket->grain = grain;
  } else {
    log_fatal("cannot set grain type if the engine is not a solid rocket");
  }

}

void
sim_engine_step(sim_engine_t *engine, double dt)
{
  engine->step(engine, dt);
}

void
sim_engine_set_throttle(sim_engine_t *engine, float throttle)
{
  engine->throttle = fminf(1.0f, fmaxf(0.0f, throttle));
}

float
sim_engine_get_throttle(sim_engine_t *engine)
{
  return engine->throttle;
}

void
sim_engine_fire(sim_engine_t *eng)
{
  assert((eng->state == SIM_ARMED || eng->state == SIM_BURNING)
         && "invalid state");

  if (eng->throttle > 0.0) {eng->state = SIM_BURNING;}
  else eng->state = SIM_ARMED;
}

void
sim_engine_arm(sim_engine_t *eng)
{
  assert((eng->state == SIM_DISARMED) && "invalid state");
  eng->state = SIM_ARMED;
}

void
sim_engine_disarm(sim_engine_t *eng)
{
  assert((eng->state == SIM_ARMED) && "invalid state");
  eng->state = SIM_DISARMED;
}

void
sim_engine_disable(sim_engine_t *eng)
{
  assert((eng->state == SIM_BURNING) && "invalid state");
  eng->state = SIM_ARMED;
}

void
sim_engine_lock(sim_engine_t *eng)
{
  if (eng->state == SIM_ARMED) eng->state = SIM_LOCKED_CLOSED;
  else if (eng->state == SIM_BURNING) eng->state = SIM_LOCKED_BURNING;
  else assert(0 && "invalid state");
}

void
sim_engine_unlock(sim_engine_t *eng)
{
  if (eng->state == SIM_LOCKED_BURNING) eng->state = SIM_BURNING;
  else if (eng->state == SIM_LOCKED_CLOSED) eng->state = SIM_ARMED;
  else assert(0 && "invalid state");
}

void
sim_engine_fail(sim_engine_t *eng)
{
  if (eng->state == SIM_BURNING) eng->state = SIM_FAULT_BURNING;
  else eng->state = SIM_FAULT_CLOSED;
}
