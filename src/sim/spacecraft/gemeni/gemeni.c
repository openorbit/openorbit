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
#include "log.h"

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
AxisUpdate(OOspacecraft *sc)
{
  // TODO: Replace IO-system queries with sim variable lookups.
  //       This is important in order to allow for multiplexed axis commands,
  //       in turn enabling autopilots and network control.

  OOaxises axises;
  ooGetAxises(&axises);

  switch (sc->detatchSequence) {
    case MERC_REDSTONE: {
      OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
      SIMengine *eng = ARRAY_ELEM(redstone->engines, THR_ROCKETDYNE);
      simEngineSetThrottle(eng, SIM_VAL(axises.orbital));
      break;
    }
    case MERC_CAPSULE: {
      if (!sc->detatchComplete) break;
      OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_ROLL_0),
                           SIM_VAL(axises.roll));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_ROLL_0));

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_ROLL_1),
                           -SIM_VAL(axises.roll));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_ROLL_1));

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_PITCH_0),
                           SIM_VAL(axises.pitch));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_PITCH_0));

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_PITCH_1),
                           -SIM_VAL(axises.pitch));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_PITCH_1));

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_YAW_0),
                           SIM_VAL(axises.yaw));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_YAW_0));

      simEngineSetThrottle(ARRAY_ELEM(capsule->engines, THR_YAW_1),
                           -SIM_VAL(axises.yaw));
      simEngineFire(ARRAY_ELEM(capsule->engines, THR_YAW_1));

      break;
    }
    default:
      assert(0 && "invalid case");
  }
}

static void
DetatchComplete(void *data)
{
  ooLogInfo("detatch complete");

  OOspacecraft *sc = (OOspacecraft*)data;
  sc->detatchPossible = false; // Do not reenable, must be false after last stage detatched
  sc->detatchComplete = true;

  OOstage *stage = sc->stages.elems[MERC_CAPSULE];

  simEngineDisable(ARRAY_ELEM(stage->engines, THR_POSI));
  simStageArmEngines(stage);
  simEngineDisarm(ARRAY_ELEM(stage->engines, THR_POSI));
}

static void
DetatchStage(OOspacecraft *sc)
{
  ooLogInfo("detatch commanded");
  sc->detatchPossible = false;

  if (sc->detatchSequence == MERC_REDSTONE) {
    ooLogInfo("detatching redstone");
    OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
    OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

    simStageDisableEngines(redstone);
    simStageLockEngines(redstone);

    simDetatchStage(sc, redstone);

    simEngineArm(ARRAY_ELEM(capsule->engines, THR_POSI));
    simEngineFire(ARRAY_ELEM(capsule->engines, THR_POSI));

    sc->detatchComplete = false;
    simEnqueueDelta_s(1.0, DetatchComplete, sc);
  }
}


static void
MainEngineToggle(OOspacecraft *sc)
{
  switch (sc->detatchSequence) {
    case MERC_REDSTONE: {
      OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
      SIMengine *eng = ARRAY_ELEM(redstone->engines, THR_ROCKETDYNE);
      if (eng->state == SIM_Armed) simEngineFire(eng);
      else if (eng->state == SIM_Burning) simEngineDisable(eng);
      break;
    }
    case MERC_CAPSULE: {
      OOstage *capsule = sc->stages.elems[MERC_CAPSULE];
      SIMengine *eng = ARRAY_ELEM(capsule->engines, THR_RETRO_0);
      //if (eng->state == SIM_Armed) RetroFire_0(sc);
      break;
    }
    default:
      assert(0 && "invalid case");
  }
}

static OOspacecraft*
GemeniNew(void)
{
  OOspacecraft *sc = smalloc(sizeof(OOspacecraft));
  simScInit(sc, "Gemeni SC");
  sc->detatchStage = DetatchStage;
  sc->toggleMainEngine = MainEngineToggle;
  sc->axisUpdate = AxisUpdate;
  // inertia tensors are entered in the base form, assuming that the total
  // mass = 1.0
  // for the redstone mercury rocket we assume a solid cylinder for the form
  // 1/2 mrr = 0.5 * 1.0 * 0.89 * 0.89 = 0.39605
  // 1/12 m(3rr + hh) = 27.14764852

  OOstage *launcher = simNewStage(sc, "Command-Module",
                                  "spacecrafts/mercury/gemeni.ac");

#if 0
  plMassSet(&redstone->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            27.14764852, 0.39605, 27.14764852,
            0.0, 0.0, 0.0);
  plMassMod(&redstone->obj->m, 24000.0 + 2200.0);
  plMassTranslate(&redstone->obj->m, 0.0, 8.9916, 0.0);
  plMassSetMin(&redstone->obj->m, 4400.0);
  plSetDragCoef(redstone->obj, 0.5);
  plSetArea(redstone->obj, 2.0*M_PI);

  scStageSetOffset3f(redstone, 0.0, 0.0, 0.0);

  //"LOX/ethyl alcohol"
  simNewEngine("Rocketdyne A7", redstone, SIM_Thruster, SIM_Armed, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,370.0e3,0.0});
  //orbital
#endif

  OOstage *capsule = simNewStage(sc, "Command-Module",
                                "spacecrafts/mercury/gemeni.ac");
#if 0
  plMassSet(&capsule->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            1.2188583333, 0.39605, 1.2188583333,
            0.0, 0.0, 0.0);
  plMassMod(&capsule->obj->m, 1354.0);
  plMassTranslate(&capsule->obj->m, 0.0, 0.55, 0.0);
  plMassSetMin(&capsule->obj->m, 1354.0);
  plSetDragCoef(capsule->obj, 0.5);
  plSetArea(capsule->obj, 2.0*M_PI);

  scStageSetOffset3f(capsule, 0.0, 17.9832, 0.0);
#endif
  simNewEngine("Posigrade", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,1.8e3,0.0});

  // Ripple fire 10 s burntime each
  simNewEngine("Retro 0", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  simNewEngine("Retro 1", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  simNewEngine("Retro 2", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  simNewEngine("Roll 0", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  simNewEngine("Roll 1", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){-0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  simNewEngine("Pitch 0", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.00, 2.20, 0.41}, (float3){0.0, 0.0,-108.0});
  simNewEngine("Pitch 1", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.00, 2.20,-0.41}, (float3){0.0, 0.0,108.0});
  simNewEngine("Yaw 0", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){0.41, 2.20, 0.00}, (float3){-108.0, 0.0,0.0});
  simNewEngine("Yaw 1", capsule, SIM_Thruster, SIM_Disarmed, 1.0f,
               (float3){-0.41, 2.20, 0.00}, (float3){108.0, 0.0,0.0});

  return sc;
}

INIT_STATIC_SC_PLUGIN
{
  simNewSpacecraftClass("gemeni", GemeniNew);
}
