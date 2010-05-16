/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <assert.h>

#include "sim.h"
#include "sim/simevent.h"
#include "sim/spacecraft.h"
#include "sim/actuator.h"

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
MercuryAxisUpdate(OOspacecraft *sc)
{
  OOaxises axises;
  ooGetAxises(&axises);

  switch (sc->detatchSequence) {
  case MERC_REDSTONE: {
    OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
    SIMthruster *rocketdyne = redstone->actuators.elems[THR_ROCKETDYNE];
    simSetThrottle(rocketdyne, axises.orbital);
    break;
  }
  case MERC_CAPSULE: {
    if (!sc->detatchComplete) break;
    OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

    SIMthruster *roll0 = capsule->actuators.elems[THR_ROLL_0];
    SIMthruster *roll1 = capsule->actuators.elems[THR_ROLL_1];
    simSetThrottle(roll0, fmax(axises.roll, 0.0));
    simSetThrottle(roll1, fmax(-axises.roll, 0.0));

    SIMthruster *pitch0 = capsule->actuators.elems[THR_PITCH_0];
    SIMthruster *pitch1 = capsule->actuators.elems[THR_PITCH_1];
    simSetThrottle(pitch0, fmax(axises.pitch, 0.0));
    simSetThrottle(pitch1, fmax(-axises.pitch, 0.0));

    SIMthruster *yaw0 = capsule->actuators.elems[THR_YAW_0];
    SIMthruster *yaw1 = capsule->actuators.elems[THR_YAW_1];
    simSetThrottle(yaw0, fmax(axises.yaw, 0.0));
    simSetThrottle(yaw1, fmax(-axises.yaw, 0.0));

    simFireThrusterIfThrottle(roll0);
    simFireThrusterIfThrottle(roll1);

    simFireThrusterIfThrottle(pitch0);
    simFireThrusterIfThrottle(pitch1);

    simFireThrusterIfThrottle(yaw0);
    simFireThrusterIfThrottle(yaw1);

    break;
  }
  default:
    assert(0 && "invalid case");
  }
}

static void
MercuryDetatchComplete(void *data)
{
  ooLogInfo("detatch complete");

  OOspacecraft *sc = (OOspacecraft*)data;
  sc->detatchPossible = false; // Do not reenable, must be false after last stage detatched
  sc->detatchComplete = true;

  OOstage *stage = sc->stages.elems[MERC_CAPSULE];
  SIMthruster *posi = stage->actuators.elems[THR_POSI];
  simDisableActuator((OOactuator*)posi);
  simArmStageActuators(stage);
  simDisarmActuator((OOactuator*)posi);
}

static void
MercuryDetatch(OOspacecraft *sc)
{
  sc->detatchPossible = false;

  if (sc->detatchSequence == MERC_REDSTONE) {
    ooLogInfo("detatching redstone");
    OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
    OOstage *capsule = sc->stages.elems[MERC_CAPSULE];
    SIMthruster *posi = capsule->actuators.elems[THR_POSI];

    simDisableStageActuators(redstone);
    simLockStageActuators(redstone);
    simDetatchStage(sc, redstone);

    simArmActuator((OOactuator*)posi);
    simFireActuator((OOactuator*)posi);

    sc->detatchComplete = false;
    simEnqueueDelta_s(simGetEventQueue(), 1.0, MercuryDetatchComplete, sc);
  }
}

static void
RetroDisable_2(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

  simDisableActuator(capsule->actuators.elems[THR_RETRO_2]);
  simDisarmActuator(capsule->actuators.elems[THR_RETRO_2]);
}

static void
RetroFire_2(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

  simFireActuator(capsule->actuators.elems[THR_RETRO_2]);
  simEnqueueDelta_s(simGetEventQueue(), 10.0, RetroDisable_2, sc);
}

static void
RetroDisable_1(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

  simDisableActuator(capsule->actuators.elems[THR_RETRO_1]);
  simDisarmActuator(capsule->actuators.elems[THR_RETRO_1]);
}

static void
RetroFire_1(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

  simFireActuator(capsule->actuators.elems[THR_RETRO_1]);
  simEnqueueDelta_s(simGetEventQueue(), 10.0, RetroDisable_1, sc);
  simEnqueueDelta_s(simGetEventQueue(), 5.0, RetroFire_2, sc);
}

static void
RetroDisable_0(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];

  simDisableActuator(capsule->actuators.elems[THR_RETRO_0]);
  simDisarmActuator(capsule->actuators.elems[THR_RETRO_0]);
}


static void
RetroFire_0(void *data)
{
  OOspacecraft *sc = (OOspacecraft*)data;
  OOstage *capsule = sc->stages.elems[MERC_CAPSULE];
  simFireActuator(capsule->actuators.elems[THR_RETRO_0]);

  simEnqueueDelta_s(simGetEventQueue(), 10.0, RetroDisable_0, sc);
  simEnqueueDelta_s(simGetEventQueue(), 5.0, RetroFire_1, sc);
}

static void
MainEngineToggle(OOspacecraft *sc)
{
  switch (sc->detatchSequence) {
  case MERC_REDSTONE: {
    OOstage *redstone = sc->stages.elems[MERC_REDSTONE];
    OOactuator *th = redstone->actuators.elems[THR_ROCKETDYNE];
    if (th->state == OO_Act_Armed) simFireActuator(th);
    else if (th->state == OO_Act_Enabled) simDisableActuator(th);
    break;
  }
  case MERC_CAPSULE: {
    OOstage *capsule = sc->stages.elems[MERC_CAPSULE];
    OOactuator *th = capsule->actuators.elems[THR_RETRO_0];
    if (th->state == OO_Act_Armed) RetroFire_0(sc);
    break;
  }
  default:
    assert(0 && "invalid case");
  }
}

static OOspacecraft*
MercuryNew(void)
{
  OOspacecraft *sc = malloc(sizeof(OOspacecraft));
  simScInit(sc, "Mercury");
  sc->detatchStage = MercuryDetatch;
  sc->toggleMainEngine = MainEngineToggle;
  sc->axisUpdate = MercuryAxisUpdate;
  // inertia tensors are entered in the base form, assuming that the total mass = 1.0
  // for the redstone mercury rocket we assume a solid cylinder for the form
  // 1/2 mrr = 0.5 * 1.0 * 0.89 * 0.89 = 0.39605
  // 1/12 m(3rr + hh) = 27.14764852

  OOstage *redstone = ooScNewStage(sc, "Mercury-Redstone");
  SGdrawable *redstoneModel = sgLoadModel("spacecrafts/mercury/redstone.ac");
  // TODO: Loading models and attaching them to the stages should really be made
  //       cleaner, this is not a good API
  ooScSetStageMesh(redstone, redstoneModel);
  sgSceneAddObj(sgGetScene(simGetSg(), "main"), redstoneModel);
  plMassSet(&redstone->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            27.14764852, 0.39605, 27.14764852,
            0.0, 0.0, 0.0);
  plMassMod(&redstone->obj->m, 24000.0 + 2200.0);
  plMassTranslate(&redstone->obj->m, 0.0, 8.9916, 0.0);
  plMassSetMin(&redstone->obj->m, 4400.0);
  scStageSetOffset3f(redstone, 0.0, 0.0, 0.0);

  //"LOX/ethyl alcohol"
  SIMthruster *rocketdyne = simNewThruster("Rocketdyne A7",
                                           (float3){0.0,0.0,0.0},
                                           (float3){0.0,370.0e3,0.0});
  simAddActuator(redstone, (OOactuator*)rocketdyne);
  simArmActuator((OOactuator*)rocketdyne);
  //orbital
  OOstage *capsule = ooScNewStage(sc, "Command-Module");
  SGdrawable *capsuleModel = sgLoadModel("spacecrafts/mercury/mercury.ac");
  ooScSetStageMesh(capsule, capsuleModel);
  sgSceneAddObj(sgGetScene(simGetSg(), "main"), capsuleModel);
  plMassSet(&capsule->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            1.2188583333, 0.39605, 1.2188583333,
            0.0, 0.0, 0.0);
  plMassMod(&capsule->obj->m, 1354.0);
  plMassTranslate(&capsule->obj->m, 0.0, 0.55, 0.0);
  plMassSetMin(&capsule->obj->m, 1354.0);
  scStageSetOffset3f(capsule, 0.0, 17.9832, 0.0);

  SIMthruster *posigrade, *retro0, *retro1, *retro2, *roll0, *roll1, *pitch0, *pitch1,
              *yaw0, *yaw1;

  posigrade = simNewThruster("Posigrade", (float3){0.0,0.0,0.0}, (float3){0.0,1.8e3,0.0});

  // Ripple fire 10 s burntime each
  retro0 = simNewThruster("Retro 0", (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  retro1 = simNewThruster("Retro 1", (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});
  retro2 = simNewThruster("Retro 2", (float3){0.0,0.0,0.0}, (float3){0.0,4.5e3,0.0});

  roll0 = simNewThruster("Roll 0", (float3){0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  roll1 = simNewThruster("Roll 1", (float3){-0.82, 0.55, 0.00}, (float3){0.0, 0.0,108.0});
  pitch0 = simNewThruster("Pitch 0", (float3){0.00, 2.20, 0.41}, (float3){0.0, 0.0,-108.0});
  pitch1 = simNewThruster("Pitch 1", (float3){0.00, 2.20,-0.41}, (float3){0.0, 0.0,108.0});
  yaw0 = simNewThruster("Yaw 0", (float3){0.41, 2.20, 0.00}, (float3){-108.0, 0.0,0.0});
  yaw1 = simNewThruster("Yaw 1", (float3){-0.41, 2.20, 0.00}, (float3){108.0, 0.0,0.0});

  simAddActuator(capsule, (OOactuator*)posigrade);
  simAddActuator(capsule, (OOactuator*)retro0);
  simAddActuator(capsule, (OOactuator*)retro1);
  simAddActuator(capsule, (OOactuator*)retro2);
  simAddActuator(capsule, (OOactuator*)roll0);
  simAddActuator(capsule, (OOactuator*)roll1);
  simAddActuator(capsule, (OOactuator*)pitch0);
  simAddActuator(capsule, (OOactuator*)pitch1);
  simAddActuator(capsule, (OOactuator*)yaw0);
  simAddActuator(capsule, (OOactuator*)yaw1);

  return sc;
}

static void SCINIT
Init(void)
{
  simNewSpacecraftClass("mercury", MercuryNew);
}
