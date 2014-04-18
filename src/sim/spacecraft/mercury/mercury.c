/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "sim/class.h"
#include "sim/simevent.h"
#include "sim/spacecraft.h"
#include "sim/actuator.h"
#include "common/palloc.h"
#include <openorbit/log.h>

enum Mercury_Stages {
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

void
MercuryAxisUpdate(sim_spacecraft_t *sc)
{
  switch (sc->detatchSequence) {
  case MERC_REDSTONE: {
    sim_stage_t *redstone = sc->stages.elems[MERC_REDSTONE];
    sim_engine_t *eng = ARRAY_ELEM(redstone->engines, THR_ROCKETDYNE);
    sim_engine_set_throttle(eng, SIM_VAL(sc->axises.orbital));
    break;
  }
  case MERC_CAPSULE: {
    if (!sc->detatchComplete) break;
    sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_ROLL_0),
                         SIM_VAL(sc->axises.roll));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_ROLL_0));

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_ROLL_1),
                         -SIM_VAL(sc->axises.roll));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_ROLL_1));

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_PITCH_0),
                         SIM_VAL(sc->axises.pitch));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_PITCH_0));

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_PITCH_1),
                         -SIM_VAL(sc->axises.pitch));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_PITCH_1));

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_YAW_0),
                         SIM_VAL(sc->axises.yaw));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_YAW_0));

    sim_engine_set_throttle(ARRAY_ELEM(capsule->engines, THR_YAW_1),
                         -SIM_VAL(sc->axises.yaw));
    sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_YAW_1));

    break;
  }
  default:
    assert(0 && "invalid case");
  }
}

static void
MercuryDetatchComplete(void *data)
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
MercuryDetatch(sim_spacecraft_t *sc)
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
    sim_event_enqueue_relative_s(1.0, MercuryDetatchComplete, sc);
  }
}

static void
RetroDisable_2(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_disable(ARRAY_ELEM(capsule->engines, THR_RETRO_2));
  sim_engine_disarm(ARRAY_ELEM(capsule->engines, THR_RETRO_2));
}

static void
RetroFire_2(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_RETRO_2));
  sim_event_enqueue_relative_s(10.0, RetroDisable_2, sc);
}

static void
RetroDisable_1(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_disable(ARRAY_ELEM(capsule->engines, THR_RETRO_1));
  sim_engine_disarm(ARRAY_ELEM(capsule->engines, THR_RETRO_1));
}

static void
RetroFire_1(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_RETRO_1));

  sim_event_enqueue_relative_s(10.0, RetroDisable_1, sc);
  sim_event_enqueue_relative_s(5.0, RetroFire_2, sc);
}

static void
RetroDisable_0(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_disable(ARRAY_ELEM(capsule->engines, THR_RETRO_0));
  sim_engine_disarm(ARRAY_ELEM(capsule->engines, THR_RETRO_0));
}


static void
RetroFire_0(void *data)
{
  sim_spacecraft_t *sc = (sim_spacecraft_t*)data;
  sim_stage_t *capsule = sc->stages.elems[MERC_CAPSULE];

  sim_engine_fire(ARRAY_ELEM(capsule->engines, THR_RETRO_0));

  sim_event_enqueue_relative_s(10.0, RetroDisable_0, sc);
  sim_event_enqueue_relative_s(5.0, RetroFire_1, sc);
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
    if (eng->state == SIM_ARMED) RetroFire_0(sc);
    break;
  }
  default:
    assert(0 && "invalid case");
  }
}


static void
MercuryInit(sim_spacecraft_t *sc)
{
  sc->detatchStage = MercuryDetatch;
  sc->toggleMainEngine = MainEngineToggle;
  sc->axisUpdate = MercuryAxisUpdate;
  // inertia tensors are entered in the base form, assuming that the total
  // mass = 1.0
  // for the redstone mercury rocket we assume a solid cylinder for the form
  // 1/2 mrr = 0.5 * 1.0 * 0.89 * 0.89 = 0.39605
  // 1/12 m(3rr + hh) = 27.14764852

  sim_stage_t *redstone = sim_new_stage(sc, "Mercury-Redstone",
                                  "spacecrafts/mercury/redstone.ac");

  pl_mass_set(&redstone->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            27.14764852, 0.39605, 27.14764852,
            0.0, 0.0, 0.0);
  pl_mass_mod(&redstone->obj->m, 24000.0 + 2200.0);
  pl_mass_translate(&redstone->obj->m, 0.0, 8.9916, 0.0);
  pl_mass_set_min(&redstone->obj->m, 4400.0);
  pl_object_set_drag_coef(redstone->obj, 0.5);
  pl_object_set_area(redstone->obj, 2.0*M_PI);

  sim_stage_set_offset3f(redstone, 0.0, 0.0, 0.0);

  //"LOX/ethyl alcohol"
  sim_new_engine("Rocketdyne A7", redstone, SIM_THRUSTER, SIM_ARMED, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  //orbital


  sim_stage_t *capsule = sim_new_stage(sc, "Command-Module",
                                 "spacecrafts/mercury/mercury.ac");

  pl_mass_set(&capsule->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            1.2188583333, 0.39605, 1.2188583333,
            0.0, 0.0, 0.0);
  pl_mass_mod(&capsule->obj->m, 1354.0);
  pl_mass_translate(&capsule->obj->m, 0.0, 0.55, 0.0);
  pl_mass_set_min(&capsule->obj->m, 1354.0);
  pl_object_set_drag_coef(capsule->obj, 0.5);
  pl_object_set_area(capsule->obj, 2.0*M_PI);

  sim_stage_set_offset3f(capsule, 0.0, 17.9832, 0.0);

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
MercuryInit2(sim_class_t *cls, void *sc, void *arg)
{
  SIM_SUPER_INIT(cls, sc, arg);
  MercuryInit(sc);
}


MODULE_INIT(mercury, "spacecraft", NULL)
{
  log_trace("initialising 'mercury' module");

  sim_class_t *cls = sim_register_class("Spacecraft", "Mercury",
                                        MercuryInit2, sizeof(sim_spacecraft_t));

  (void)cls;
}
