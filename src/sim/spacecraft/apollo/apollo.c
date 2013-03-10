/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <assert.h>

#include "sim.h"
#include "sim/simevent.h"
#include "sim/spacecraft.h"
#include "sim/actuator.h"
#include "common/palloc.h"
#include <openorbit/log.h>

enum Stages {
  SATURN_1C = 0,
  SATURN_II,
  SATURN_IVB,
  APOLLO_SERVICE,
  APOLLO_COMMAND
};



enum Saturn_1C_II_IVB_Engines {
  ENG_ROCKETDYNE_0 = 0, // Central engine
  ENG_ROCKETDYNE_1,
  ENG_ROCKETDYNE_2,
  ENG_ROCKETDYNE_3,
  ENG_ROCKETDYNE_4
};

enum Apollo_Service_Engines {
  ENG_SERV_PROP = 0,
};
enum Apollo_Command_Engines {
  THR_POSI = 0,
  THR_RETRO_0,
  THR_RETRO_1,
  THR_RETRO_2,
  THR_ROLL_0,
  THR_ROLL_1,
  THR_PITCH_0,
  THR_PITCH_1,
  THR_YAW_0,
  THR_YAW_1,

};

static void
AxisUpdate(sim_spacecraft_t *sc)
{
  // TODO: Replace IO-system queries with sim variable lookups.
  //       This is important in order to allow for multiplexed axis commands,
  //       in turn enabling autopilots and network control.

  sim_axises_t axises;
  sim_get_axises(&axises);

  switch (sc->detatchSequence) {
    case SATURN_1C: {
      sim_stage_t *saturn_1c = sc->stages.elems[SATURN_1C];
      ARRAY_FOR_EACH(i, saturn_1c->engines) {
        sim_engine_set_throttle(ARRAY_ELEM(saturn_1c->engines, i),
                             SIM_VAL(axises.orbital));
      }
      break;
    }
    case SATURN_II: {
      sim_stage_t *saturn_ii = sc->stages.elems[SATURN_II];
      ARRAY_FOR_EACH(i, saturn_ii->engines) {
        sim_engine_set_throttle(ARRAY_ELEM(saturn_ii->engines, i),
                             SIM_VAL(axises.orbital));
      }
      break;
    }
    case SATURN_IVB: {
      sim_stage_t *saturn_ivb = sc->stages.elems[SATURN_IVB];
      ARRAY_FOR_EACH(i, saturn_ivb->engines) {
        sim_engine_set_throttle(ARRAY_ELEM(saturn_ivb->engines, i),
                             SIM_VAL(axises.orbital));
      }
      break;
    }
    case APOLLO_SERVICE: {
      sim_stage_t *apollo_service = sc->stages.elems[APOLLO_SERVICE];
      sim_engine_t *eng = ARRAY_ELEM(apollo_service->engines, ENG_SERV_PROP);
      (void)eng; // TODO
      break;
    }
    case APOLLO_COMMAND: {
      sim_stage_t *apollo_cmd = sc->stages.elems[APOLLO_COMMAND];
      (void)apollo_cmd; // TODO
      break;
    }
    default:
      assert(0 && "invalid case");
  }
}

static void
DetatchComplete(void *data)
{
  log_info("detatch complete");

  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sc->detatchPossible = false; // Do not reenable, must be false after last stage detatched
  sc->detatchComplete = true;

  //sim_stage_t *stage = sc->stages.elems[MERC_CAPSULE];
  //
  //sim_engine_disable(ARRAY_ELEM(stage->engines, THR_POSI));
  //sim_stage_arm_engines(stage);
  //sim_engine_disarm(ARRAY_ELEM(stage->engines, THR_POSI));
}

static void
DetatchStage(sim_spacecraft_t *sc)
{
  log_info("detatch commanded");
  sc->detatchPossible = false;

  switch (sc->detatchSequence) {
  case SATURN_1C: {
    log_info("detatching saturn 1c stage");
    sim_stage_t *sat_1c = sc->stages.elems[SATURN_1C];
    sim_stage_t *sat_ii = sc->stages.elems[SATURN_II];

    sim_stage_disable_engines(sat_1c);
    sim_stage_lock_engines(sat_1c);

    sim_spaccraft_detatch_stage(sc, sat_1c);

    sim_engine_arm(ARRAY_ELEM(sat_ii->engines, THR_POSI));
    sim_engine_fire(ARRAY_ELEM(sat_ii->engines, THR_POSI));

    sc->detatchComplete = false;
    sim_event_enqueue_relative_s(1.0, DetatchComplete, sc);
    break;
  }
  case SATURN_II:
    log_info("detatching saturn ii stage");
    break;
  case SATURN_IVB:
    log_info("detatching saturn ivb stage");
    break;
  case APOLLO_SERVICE:
    log_info("detatching apollo service module");
    break;
  case APOLLO_COMMAND:
    break;
  default:
    assert(0 && "invalid case");
  }
}


static void
MainEngineToggle(sim_spacecraft_t *sc)
{
  switch (sc->detatchSequence) {
    case SATURN_1C: {
      sim_stage_t *sat_1c = sc->stages.elems[SATURN_1C];
      (void)sat_1c; // TODO
      break;
    }
    case SATURN_II: {
      sim_stage_t *sat_ii = sc->stages.elems[SATURN_II];
      (void)sat_ii; // TODO
      break;
    }
    case SATURN_IVB: {
      sim_stage_t *sat_ivb = sc->stages.elems[SATURN_IVB];
      (void)sat_ivb; // TODO
      break;
    }
    case APOLLO_SERVICE: {
      sim_stage_t *apollo_serv = sc->stages.elems[APOLLO_SERVICE];
      (void)apollo_serv; // TODO
      break;
    }
    case APOLLO_COMMAND: {
      sim_stage_t *apollo_cmd = sc->stages.elems[APOLLO_COMMAND];
      (void)apollo_cmd; // TODO
      break;
    }
    default:
      assert(0 && "invalid case");
  }
}

static sim_spacecraft_t*
ApolloNew(void)
{
  sim_spacecraft_t *sc = smalloc(sizeof(sim_spacecraft_t));
  return sc;
}

static void
ApolloInit(sim_spacecraft_t *sc)
{
  sc->detatchStage = DetatchStage;
  sc->toggleMainEngine = MainEngineToggle;
  sc->axisUpdate = AxisUpdate;
  // inertia tensors are entered in the base form, assuming that the total
  // mass = 1.0
  // for the redstone mercury rocket we assume a solid cylinder for the form
  // 1/2 mrr = 0.5 * 1.0 * 0.89 * 0.89 = 0.39605
  // 1/12 m(3rr + hh) = 27.14764852

  sim_stage_t *sat_1c = sim_new_stage(sc, "Saturn 1C",
                                "spacecrafts/saturn/saturn_1c.ac");
  /*sim_stage_t *sat_ii =*/ sim_new_stage(sc, "Saturn II",
                                "spacecrafts/saturn/saturn_ii.ac");
  /*sim_stage_t *sat_ivb =*/ sim_new_stage(sc, "Saturn IVB",
                                 "spacecrafts/saturn/saturn_ivb.ac");
  /*sim_stage_t *apollo_serv =*/ sim_new_stage(sc, "Service Module",
                                     "spacecrafts/saturn/apollo_serv.ac");
  /*sim_stage_t *apollo_cmd =*/ sim_new_stage(sc, "Command Module",
                                    "spacecrafts/saturn/apollo_cmd.ac");


  pl_mass_set(&sat_1c->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            27.14764852, 0.39605, 27.14764852,
            0.0, 0.0, 0.0);
  pl_mass_mod(&sat_1c->obj->m, 24000.0 + 2200.0);
  pl_mass_translate(&sat_1c->obj->m, 0.0, 8.9916, 0.0);
  pl_mass_set_min(&sat_1c->obj->m, 4400.0);
  pl_object_set_drag_coef(sat_1c->obj, 0.5);
  pl_object_set_area(sat_1c->obj, 2.0*M_PI);

  sim_stage_set_offset3f(sat_1c, 0.0, 0.0, 0.0);

  sim_new_engine("Rocketdyne F-1:0", sat_1c, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  sim_new_engine("Rocketdyne F-1:1", sat_1c, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  sim_new_engine("Rocketdyne F-1:2", sat_1c, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  sim_new_engine("Rocketdyne F-1:3", sat_1c, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  sim_new_engine("Rocketdyne F-1:4", sat_1c, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});

}

static void
ApolloInit2(sim_class_t *cls, void *sc, void *arg)
{
  SIM_SUPER_INIT(cls, sc, arg);
  ApolloInit(sc);
}

MODULE_INIT(apollo, "spacecraft", NULL)
{
  log_trace("initialising 'apollo' module");
  sim_class_t *cls = sim_register_class("Spacecraft", "Apollo",
                                        ApolloInit2, sizeof(sim_spacecraft_t));
  (void)cls; // TODO
}
