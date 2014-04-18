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

enum Gemeni_Stages {
  MERC_REDSTONE = 0,
  MERC_CAPSULE,
};

enum Redstone_Actuators {
  THR_ROCKETDYNE = 0,
};

enum Capsule_Actuators {
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
    case MERC_REDSTONE: {
      sim_stage_t *redstone = sc->stages.elems[MERC_REDSTONE];
      sim_engine_t *eng = ARRAY_ELEM(redstone->engines, THR_ROCKETDYNE);
      sim_engine_set_throttle(eng, SIM_VAL(axises.orbital));
      break;
    }
    case MERC_CAPSULE: {
      if (!sc->detatchComplete) break;
      sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_ROLL_0),
                           SIM_VAL(axises.roll));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_ROLL_0));

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_ROLL_1),
                           -SIM_VAL(axises.roll));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_ROLL_1));

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_PITCH_0),
                           SIM_VAL(axises.pitch));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_PITCH_0));

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_PITCH_1),
                           -SIM_VAL(axises.pitch));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_PITCH_1));

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_YAW_0),
                           SIM_VAL(axises.yaw));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_YAW_0));

      sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_YAW_1),
                           -SIM_VAL(axises.yaw));
      sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_YAW_1));

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

  sim_stage_t *stage = sc->stages.elems[MERC_CAPSULE];

  sim_engine_disable(ARRAY_ELEM(stage->engines, THR_POSI));
  sim_stage_arm_engines(stage);
  sim_engine_disarm(ARRAY_ELEM(stage->engines, THR_POSI));
}

static void
DetatchStage(sim_spacecraft_t *sc)
{
  log_info("detatch commanded");
  sc->detatchPossible = false;

  if (sc->detatchSequence == MERC_REDSTONE) {
    log_info("detatching redstone");
    sim_stage_t *redstone = sc->stages.elems[MERC_REDSTONE];
    sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

    sim_stage_disable_engines(redstone);
    sim_stage_lock_engines(redstone);

    sim_spaccraft_detatch_stage(sc, redstone);

    sim_engine_arm(ARRAY_ELEM(capsule->engines, THR_POSI));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_POSI));

    sc->detatchComplete = false;
    sim_event_enqueue_relative_s(1.0, DetatchComplete, sc);
  }
}


static void
MainEngineToggle(sim_spacecraft_t *sc)
{
  switch (sc->detatchSequence) {
    case MERC_REDSTONE: {
      sim_stage_t *redstone = sc->stages.elems[MERC_REDSTONE];
      sim_engine_t *eng = ARRAY_ELEM(redstone->engines, THR_ROCKETDYNE);
      if (eng->state == SIM_ARMED) sim_engine_fire(eng);
      else if (eng->state == SIM_BURNING) sim_engine_disable(eng);
      break;
    }
    case MERC_CAPSULE: {
      sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];
      sim_engine_t *eng = ARRAY_ELEM(capsule->engines, THR_RETRO_0);
      (void)eng;
      break;
    }
    default:
      assert(0 && "invalid case");
  }
}

static void
GemeniInit(sim_spacecraft_t *sc)
{
  sc->detatchStage = DetatchStage;
  sc->toggleMainEngine = MainEngineToggle;
  sc->axisUpdate = AxisUpdate;
  // inertia tensors are entered in the base form, assuming that the total
  // mass = 1.0
  // for the redstone mercury rocket we assume a solid cylinder for the form
  // 1/2 mrr = 0.5 * 1.0 * 0.89 * 0.89 = 0.39605
  // 1/12 m(3rr + hh) = 27.14764852

  sim_stage_t *launcher = sim_new_stage(sc, "Command-Module",
                                  "spacecrafts/mercury/gemeni.ac");
  (void)launcher;

#if 0
  pl_mass_set(&redstone->obj->m, 1.0f, // Default to 1.0 kg
              0.0f, 0.0f, 0.0f,
              27.14764852, 0.39605, 27.14764852,
              0.0, 0.0, 0.0);
  pl_mass_mod(&redstone->obj->m, 24000.0 + 2200.0);
  pl_mass_translate(&redstone->obj->m, 0.0, 8.9916, 0.0);
  pl_mass_setmin(&redstone->obj->m, 4400.0);
  plSetDragCoef(redstone->obj, 0.5);
  plSetArea(redstone->obj, 2.0*M_PI);

  sim_stage_set_offset3f(redstone, 0.0, 0.0, 0.0);

  //"LOX/ethyl alcohol"
  sim_new_engine("Rocketdyne A7", redstone, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  //orbital
#endif

  sim_stage_t *capsule = sim_new_stage(sc, "Command-Module",
                                 "spacecrafts/mercury/gemeni.ac");
#if 0
  pl_massSet(&capsule->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            1.2188583333, 0.39605, 1.2188583333,
            0.0, 0.0, 0.0);
  pl_mass_mod(&capsule->obj->m, 1354.0);
  pl_mass_translate(&capsule->obj->m, 0.0, 0.55, 0.0);
  pl_mass_set_min(&capsule->obj->m, 1354.0);
  plSetDragCoef(capsule->obj, 0.5);
  plSetArea(capsule->obj, 2.0*M_PI);

  sim_stage_set_offset3f(capsule, 0.0, 17.9832, 0.0);
#endif
  sim_new_engine("Posigrade", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,1.8e3,0.0});

  // Ripple fire 10 s burntime each
  sim_new_engine("Retro 0", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  sim_new_engine("Retro 1", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  sim_new_engine("Retro 2", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  sim_new_engine("Roll 0", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  sim_new_engine("Roll 1", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){-0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  sim_new_engine("Pitch 0", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.00, 2.20, 0.41}, (float3){0.0, 0.0,-108.0});
  sim_new_engine("Pitch 1", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.00, 2.20,-0.41}, (float3){0.0, 0.0,108.0});
  sim_new_engine("Yaw 0", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){0.41, 2.20, 0.00}, (float3){-108.0, 0.0,0.0});
  sim_new_engine("Yaw 1", capsule, SIM_THRUSTER, SIM_DISARMED, 1.0f,
               (float3){-0.41, 2.20, 0.00}, (float3){108.0, 0.0,0.0});


}

static void
GemeniInit2(sim_class_t *cls, void *sc, void *arg)
{
  SIM_SUPER_INIT(cls, sc, arg);
  GemeniInit(sc);
}

MODULE_INIT(gemeni, "spacecraft", NULL)
{
  log_trace("initialising 'gemeni' module");
  sim_class_t *cls = sim_register_class("Spacecraft", "Gemeni",
                                        GemeniInit2, sizeof(sim_spacecraft_t));
  (void)cls;
}
