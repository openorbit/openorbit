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

static const char *actuatorNames[OO_Act_Group_Count] = {
  [OO_Act_Orbital] = "orbital",
  [OO_Act_Vertical] = "vertical",
  [OO_Act_Horisontal] = "horisontal",
  [OO_Act_Forward] = "forward",
  [OO_Act_Pitch] = "pitch",
  [OO_Act_Roll] = "roll",
  [OO_Act_Yaw] = "yaw"
};

int
ooGetActuatorGroupId(const char *groupName)
{
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    if (!strcmp(actuatorNames[i], groupName)) {
      return i;
    }
  }
  return -1;
}

const char*
ooGetActuatorGroupName(unsigned groupId)
{
  assert(groupId < OO_Act_Group_Count);
  return actuatorNames[groupId];
}

float
ooGetThrottleForActuatorGroup(unsigned groupId)
{
  float throttle = ioGetSlider(IO_SLIDER_THROT_0);//ooIoGetAxis(NULL, ooGetActuatorGroupName(groupId));
  return throttle;
}


OOactuatorgroup*
ooScNewActuatorGroup(const char *name)
{
  OOactuatorgroup *eg = smalloc(sizeof(OOactuatorgroup));
  memset(eg, 0, sizeof(OOactuatorgroup));
  obj_array_init(&eg->actuators);
  eg->groupName = strdup(name);
  return eg;
}

void
thruster_step(SIMengine *engine, float dt)
{
  SIMthrust *thruster = (SIMthrust*)engine;

  plForceRelativePos3fv(thruster->super.stage->sc->obj,
                        thruster->fMax * thruster->super.throttle,
                        thruster->super.pos);
}


void
liquid_rocket_step(SIMengine *engine, float dt)
{
  SIMliquidrocketengine *lrocket = (SIMliquidrocketengine*)engine;

  //  plForceRelativePos3fv(lrocket->super.stage->sc->obj,
  //                      lrocket->fMax * lrocket->super.throttle,
  //                      lrocket->super.pos);
}

void
solid_rocket_step(SIMengine *engine, float dt)
{
  SIMsolidrocketengine *srocket = (SIMsolidrocketengine*)engine;
  switch (srocket->grain) {
  case SIM_Circular:
  case SIM_End_Burner:
  case SIM_C_Slot:
  case SIM_Moon_Burner:
  case SIM_Finocyl:
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
jet_step(SIMengine *engine, float dt)
{
  SIMjetengine *jet = (SIMjetengine*)engine;

}

void
prop_step(SIMengine *engine, float dt)
{
  SIMpropengine *prop = (SIMpropengine*)engine;
  plForceRelativePos3fv(prop->super.stage->obj, prop->super.dir,
                        prop->super.pos);
  plTorqueRelative3fv(prop->super.stage->obj,
                     vf3_set(0.0, 0.0, 0.0), prop->super.pos);
}
void
turboprop_step(SIMengine *engine, float dt)
{
  SIMturbopropengine *tprop = (SIMturbopropengine*)engine;
}


SIMengine*
simNewEngine(char *name, sim_stage_t *stage, SIMenginekind kind,
             SIMenginestate state, float throttle, float3 pos, float3 dir)
{
  SIMengine *engine = NULL;

  SIMthrust *thrust = NULL;
  SIMpropengine *prop = NULL;
  SIMturbopropengine *tprop = NULL;
  SIMjetengine *jet = NULL;;
  SIMliquidrocketengine *lrocket = NULL;
  SIMsolidrocketengine *srocket = NULL;

  switch (kind) {
  case SIM_Thruster:
    thrust = smalloc(sizeof(SIMthrust));
    thrust->fMax = dir;
    engine = &thrust->super;
    engine->step = thruster_step;
    break;
  case SIM_Prop:
    prop = smalloc(sizeof(SIMpropengine));
    engine = &prop->super;
    engine->step = prop_step;
    break;
  case SIM_TurboProp:
    tprop = smalloc(sizeof(SIMturbopropengine));
    engine = &tprop->super;
    engine->step = turboprop_step;
    break;
  case SIM_Jet:
    jet = smalloc(sizeof(SIMjetengine));
    engine = &jet->super;
    engine->step = jet_step;
    break;
  case SIM_LiquidRocket:
    lrocket = smalloc(sizeof(SIMliquidrocketengine));
    obj_array_init(&lrocket->oxidiserTanks);
    engine = &lrocket->super;
    engine->step = liquid_rocket_step;
    break;
  case SIM_SolidRocket:
    srocket = smalloc(sizeof(SIMsolidrocketengine));
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
  PLparticles *psys = plNewParticleSystem(name, 1000);
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
simEngineAddTank(SIMengine *engine, SIMtank *tank)
{
  obj_array_push(&engine->fuelTanks, tank);
}

void
simEngineAddOxidiserTank(SIMengine *engine, SIMtank *tank)
{
  if (engine->kind == SIM_LiquidRocket) {
    SIMliquidrocketengine *lrocket = (SIMliquidrocketengine*)engine;
    obj_array_push(&lrocket->oxidiserTanks, tank);
  } else {
    ooLogFatal("cannot add oxidiser tank to that engine");
  }
}

void
simEngineSetGrainType(SIMengine *engine, SIMgrainkind grain)
{
  if (engine->kind == SIM_SolidRocket) {
    SIMsolidrocketengine *srocket = (SIMsolidrocketengine*)engine;
    srocket->grain = grain;
  } else {
    ooLogFatal("cannot set grain type if the engine is not a solid rocket");
  }

}

void
simEngineStep(SIMengine *engine, double dt)
{
  engine->step(engine, dt);
}

void
simEngineSetThrottle(SIMengine *engine, float throttle)
{
  engine->throttle = fminf(1.0f, fmaxf(0.0f, throttle));
}

float
simEngineGetThrottle(SIMengine *engine)
{
  return engine->throttle;
}

void
simEngineFire(SIMengine *eng)
{
  assert((eng->state == SIM_Armed || eng->state == SIM_Burning)
         && "invalid state");

  if (eng->throttle > 0.0) {eng->state = SIM_Burning;}
  else eng->state = SIM_Armed;
}

void
simEngineArm(SIMengine *eng)
{
  assert((eng->state == SIM_Disarmed) && "invalid state");
  eng->state = SIM_Armed;
}

void
simEngineDisarm(SIMengine *eng)
{
  assert((eng->state == SIM_Armed) && "invalid state");
  eng->state = SIM_Disarmed;
}

void
simEngineDisable(SIMengine *eng)
{
  assert((eng->state == SIM_Burning) && "invalid state");
  eng->state = SIM_Armed;
}

void
simEngineLock(SIMengine *eng)
{
  if (eng->state == SIM_Armed) eng->state = SIM_Locked_Closed;
  else if (eng->state == SIM_Burning) eng->state = SIM_Locked_Burning;
  else assert(0 && "invalid state");
}

void
simEngineUnlock(SIMengine *eng)
{
  if (eng->state == SIM_Locked_Burning) eng->state = SIM_Burning;
  else if (eng->state == SIM_Locked_Closed) eng->state = SIM_Armed;
  else assert(0 && "invalid state");
}

void
simEngineFail(SIMengine *eng)
{
  if (eng->state == SIM_Burning) eng->state = SIM_Fault_Burning;
  else eng->state = SIM_Fault_Closed;
}
