/*
  Copyright 2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include <stdlib.h>
#include "log.h"
#include "sim.h"
#include "sim/spacecraft.h"
#include "res-manager.h"
#include "parsers/hrml.h"
#include <vmath/vmath.h>
#include <gencds/hashtable.h>
#include <gencds/array.h>
#include "common/moduleinit.h"
#include "actuator.h"
#include "io-manager.h"
#include "sim/pubsub.h"

extern SIMstate gSIM_state;

static hashtable_t *gSpacecraftClasses;

INIT_PRIMARY_MODULE
{
  gSpacecraftClasses = hashtable_new_with_str_keys(128);
}

void
simNewSpacecraftClass(const char *name, OOspacecraft *(*alloc)(void))
{
  SCclass *cls = malloc(sizeof(SCclass));

  cls->name = strdup(name);
  cls->alloc = alloc;
  cls->dealloc = NULL;

  hashtable_insert(gSpacecraftClasses, name, cls);
}

OOspacecraft*
simNewSpacecraft(const char *className, const char *scName)
{
  SCclass *cls = hashtable_lookup(gSpacecraftClasses, className);
  if (!cls) {
    ooLogError("no such spacecraft class '%s'", className);
    return NULL;
  }

  OOspacecraft *sc = cls->alloc();
  plUpdateMass(sc->obj);
  ooScSetScene(sc, sgGetScene(simGetSg(), "main"));

  char *yawkey, *pitchkey, *rollkey, *vertkey, *horizontalkey, *distkey,
       *orbkey;
  asprintf(&yawkey, "/sc/%s/axis/yaw", scName);
  asprintf(&pitchkey, "/sc/%s/axis/pitch", scName);
  asprintf(&rollkey, "/sc/%s/axis/roll", scName);
  asprintf(&vertkey, "/sc/%s/axis/vertical", scName);
  asprintf(&horizontalkey, "/sc/%s/axis/horizontal", scName);
  asprintf(&distkey, "/sc/%s/axis/distance", scName);
  asprintf(&orbkey, "/sc/%s/axis/orbital", scName);

  simPublishFloat(yawkey, &sc->axises.yaw);
  simPublishFloat(pitchkey, &sc->axises.pitch);
  simPublishFloat(rollkey, &sc->axises.roll);
  simPublishFloat(vertkey, &sc->axises.upDown);
  simPublishFloat(horizontalkey, &sc->axises.leftRight);
  simPublishFloat(distkey, &sc->axises.fwdBack);
  simPublishFloat(orbkey, &sc->axises.orbital);

  free(yawkey);
  free(pitchkey);
  free(rollkey);
  free(vertkey);
  free(horizontalkey);
  free(distkey);
  free(orbkey);

  return sc;
}


void
ooGetAxises(OOaxises *axises)
{
  ooIoGetAxis(&axises->yaw, "yaw");
  ooIoGetAxis(&axises->pitch, "pitch");
  ooIoGetAxis(&axises->roll, "roll");
  ooIoGetAxis(&axises->upDown, "vertical-throttle");
  ooIoGetAxis(&axises->leftRight, "horizontal-throttle");
  ooIoGetAxis(&axises->fwdBack, "distance-throttle");
  ooIoGetAxis(&axises->orbital, "main-throttle");
}


/*
  Compute the wing lift for a simple wing

  The simple wing relies of having precomputed lift coefficients. We allow one
  per 5 deg angle of attack. We actually intepolate between two angles to
  produce a little bit better results.
 */
double
ooSimpleWingLift(OOsimplewing *wing, const OOsimenv *env)
{
  double p = env->airDensity;  // air density
  double v = 0.0; // Velocity
  double a = wing->area;  // Wing area

  unsigned idx = (((unsigned)RAD_TO_DEG(env->aoa)) % 360) / 5;

  double cl0 = wing->liftCoeffs[idx]; // lift coefficient
  double cl1 = wing->liftCoeffs[(idx+1)%72]; // lift coefficient

  double cl = cl0 + (cl1 - cl0) * fmod(RAD_TO_DEG(env->aoa), 5.0);
  double L = 0.5 * p * v * v * a * cl;

  return L;
}

void
simDefaultDetatch(OOspacecraft *sc)
{

}

void
simDefaultPrestep(OOspacecraft *sc, double dt)
{

}

void
simDefaultPoststep(OOspacecraft *sc, double dt)
{

}

void
simDefaultEngineToggle(OOspacecraft *sc)
{
}

void
simDefaultAxisUpdate(OOspacecraft *sc)
{
}




void
simScInit(OOspacecraft *sc, const char *name)
{
  SGscenegraph *sg = simGetSg();
  PLworld *world = simGetWorld();

  obj_array_init(&sc->stages);
  obj_array_init(&sc->engines);

  sc->world = world;
  sc->prestep = simDefaultPrestep;
  sc->poststep = simDefaultPoststep;
  sc->detatchPossible = true;
  sc->detatchComplete = true;
  sc->detatchStage = simDefaultDetatch;
  sc->detatchSequence = 0;
  sc->obj = plObject(world, name);
  sc->scene = sgGetScene(sg, "main"); // Just use any of the existing ones
  sc->expendedMass = 0.0;
  sc->mainEngineOn = false;
  sc->toggleMainEngine = simDefaultEngineToggle;
  sc->axisUpdate = simDefaultAxisUpdate;
  plMassSet(&sc->obj->m, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  plSetSystem(world->rootSys, sc->obj);
}

// TODO: Pass on PLsystem instead of PLworld to ensure that object has valid
//       systems at all times.
OOspacecraft*
ooScNew(PLworld *world, SGscene *scene, const char *name)
{
  OOspacecraft *sc = malloc(sizeof(OOspacecraft));

  simScInit(sc, name);

  return sc;
}


//void
//dMassSetConeTotal(dMass *m, dReal total_mass,
//                  dReal radius, dReal height)
//{
//  float i11 = 3.0/5.0 * total_mass * height * height +
//    3.0/20.0 * total_mass * radius * radius;
//  float i22 = i11;
//  float i33 = 3.0/10.0 * total_mass * radius * radius;
//  float cogx, cogy, cogz;

//  cogx = 0.0;
//  cogy = 0.25 * height; // 1/4 from base, see wikipedia entry on Cone_(geometry)
//  cogz = 0.0;


//  dMassSetParameters(m, total_mass,
//                     cogx, cogy, cogz, // TODO: fix, COG
//                     i11, i22, i33,
//                    0.0, 0.0, 0.0);
//}


void
simDetatchStage(OOspacecraft *sc, OOstage *stage)
{
  stage->state = OO_Stage_Detatched;
  plDetatchObject(stage->obj);
}

void
simScDetatchStage(OOspacecraft *sc)
{
  if (sc->detatchPossible) {
    sc->detatchStage(sc);
    sc->detatchSequence ++;
  }
}

void
simScToggleMainEngine(OOspacecraft *sc)
{
  sc->toggleMainEngine(sc);
}

PLobject*
ooScGetPLObjForSc(OOspacecraft *sc)
{
  return (PLobject*)sc->obj;
}


void
simScStep(OOspacecraft *sc, float dt)
{
  assert(sc != NULL);


  sc->expendedMass = 0.0;

  OOaxises axises;
  ooGetAxises(&axises);

  sc->prestep(sc, dt);
  sc->axisUpdate(sc);
  for (size_t i = 0 ; i < sc->stages.length ; ++ i) {
    OOstage *stage = sc->stages.elems[i];
    ooScStageStep(stage, &axises, dt);
  }

  plMassMod(&sc->obj->m, sc->obj->m.m - sc->expendedMass);

  sc->poststep(sc, dt);
}

void // for scripts and events
ooScForce(OOspacecraft *sc, float rx, float ry, float rz)
{
  plForceRelative3f(sc->obj, rx, ry, rz);
}


void
ooScSetStageMesh(OOstage *stage, SGdrawable *mesh)
{
  assert(stage != NULL);
  assert(mesh != NULL);
  stage->obj->drawable = mesh;
}

void
ooScSyncStage(OOstage *stage)
{
  //  stage->mesh->p;
  //  stage->mesh->q;
}

void
ooScSyncSpacecraft(OOspacecraft *sc)
{
  for (size_t i = 0 ; i < sc->stages.length; ++ i) {
    ooScSyncStage(sc->stages.elems[i]);
  }
}

void
ooScStageStep(OOstage *stage, OOaxises *axises, float dt) {
  assert(stage != NULL);
  assert(axises != NULL);

  // Handle for all actuators call actuator handlers
  const static char * axisKeys[] = {
    "main-throttle",
    "vertical-throttle", "horizontal-throttle", "distance-throttle",
    "pitch", "roll", "yaw"
  };
  stage->expendedMass = 0.0f;

  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    OOactuatorgroup *actGroup = (OOactuatorgroup*)stage->actuatorGroups.elems[i];
    double axisVal = ooIoGetAxis(NULL, axisKeys[i]);
    for (int j = 0 ; j < actGroup->actuators.length ; ++j) {
      //OOactuator *act = actGroup->actuators.elems[j];
      // TODO: Will not really work, we need more parameters if an actuator is
      //       used for multiple functions.
      //act->axisUpdate(act, axisVal);
    }
  }

  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state & SIM_ENGINE_BURNING_BIT) {
      ooLogInfo("engine %s burning", eng->name);
      eng->step(eng, dt);
    }
  }

  plMassMod(&stage->obj->m, stage->obj->m.m - stage->expendedMass);
  stage->sc->expendedMass += stage->expendedMass;
}

OOstage*
ooScNewStage(OOspacecraft *sc, const char *name)
{
  OOstage *stage = malloc(sizeof(OOstage));
  stage->state = OO_Stage_Idle;
  stage->sc = sc;
  stage->expendedMass = 0.0;

  obj_array_init(&stage->engines);
  obj_array_init(&stage->actuatorGroups);
  obj_array_init(&stage->payload);
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    OOactuatorgroup *actGroup = ooScNewActuatorGroup(ooGetActuatorGroupName(i));
    obj_array_push(&stage->actuatorGroups, actGroup);
  }

  stage->obj = plSubObject3f(sc->world, sc->obj, name, 0.0, 0.0, 0.0);

  obj_array_push(&sc->stages, stage);

  return stage;
}

void
scStageSetOffset3f(OOstage *stage, float x, float y, float z)
{
  stage->pos = vf3_set(x, y, z);
  stage->obj->p_offset = vf3_set(x, y, z);
}

void
scStageSetOffset3fv(OOstage *stage, float3 p)
{
  stage->pos = p;
  stage->obj->p_offset = p;
}

void
ooScSetScene(OOspacecraft *spacecraft, SGscene *scene)
{
  for (int i = 0 ; i < spacecraft->stages.length ; ++i) {
    OOstage *stage = spacecraft->stages.elems[i];

    if (stage->state != OO_Stage_Detatched) {
      sgSceneAddObj(scene, stage->obj->drawable);
    }
  }
}

void
ooScSetSystem(OOspacecraft *spacecraft, PLsystem *sys)
{
  PLsystem *oldSys = spacecraft->obj->sys;

  if (oldSys != NULL) {
    for (int i = 0 ; i < oldSys->rigidObjs.length ; ++i) {
      if (oldSys->rigidObjs.elems[i] == spacecraft->obj) {
        obj_array_remove(&oldSys->rigidObjs, i);
      }
    }
  }
  obj_array_push(&sys->rigidObjs, spacecraft->obj);
  spacecraft->obj->sys = sys;
}
void
ooScSetPos(OOspacecraft *sc, double x, double y, double z)
{
  plSetObjectPos3d(sc->obj, x, y, z);
}

void
ooScSetSystemAndPos(OOspacecraft *sc, const char *sysName,
                    double x, double y, double z)
{
  PLastrobody *astrobody = plGetObject(sc->world, sysName);
  if (astrobody != NULL) {
    plSetObjectPosRel3d(sc->obj, &astrobody->obj, x, y, z);
    ooScSetSystem(sc, astrobody->sys);
    float3 v = plComputeCurrentVelocity(astrobody);
    plSetVel3fv(sc->obj, v);
  } else {
    ooLogWarn("astrobody '%s' not found", sysName);
  }
}

void
ooScSetSysAndCoords(OOspacecraft *sc, const char *sysName,
                    double longitude, double latitude, double altitude)
{
  // Find planetoid object
  PLastrobody *astrobody = plGetObject(sc->world, sysName);
  if (astrobody != NULL) {
    // Compute position relative to planet centre, this requires the equatorial
    // radius and the eccentricity of the spheroid, we shoudl also adjust for
    // sideral rotation.
    float3 p = geodetic2cart_f(astrobody->eqRad, astrobody->angEcc,
                               latitude, longitude, altitude);
    plSetObjectPosRel3fv(sc->obj, &astrobody->obj, p);

    ooScSetSystem(sc, astrobody->sys);
    float3 v = plComputeCurrentVelocity(astrobody);

    // Compute standard orbital velocity
    float3 velvec = vf3_normalise(vf3_cross(p, vf3_set(0.0f, 0.0f, 1.0f)));
    velvec = vf3_s_mul(velvec, sqrtf(sc->obj->sys->orbitalBody->GM/vf3_abs(p)));
    plSetVel3fv(sc->obj, vf3_add(v, velvec));
  }
}

void
simInitStage(OOstage *stage)
{
}

void
simAddStage(OOspacecraft *sc, OOstage *stage)
{
  obj_array_push(&sc->stages, stage);
}

// TODO: This is buggy, gravity is only set on the parent object unless the
//       stage has been detatched.
float3
simGetStageGravityVector(OOstage *stage)
{
  return stage->obj->g_ack;
}

float3
simGetGravityVector(OOspacecraft *sc)
{
  // TODO: Return the real gravity instead of force ackumulator
  return sc->obj->g_ack;
}

float3
simGetVelocityVector(OOspacecraft *sc)
{
  return sc->obj->v;
}

float3
simGetForceVector(OOspacecraft *sc)
{
  return sc->obj->f_ack;
}

quaternion_t
simGetQuaternion(OOspacecraft *sc)
{
  return sc->obj->q;
}



void
simStageArmEngines(OOstage *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Disarmed) {
      simEngineArm(eng);
    }
  }
}

void
simStageDisarmEngines(OOstage *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Armed) {
      simEngineDisarm(eng);
    }
  }
}

void
simStageDisableEngines(OOstage *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Burning) {
      simEngineDisable(eng);
    }
  }
}

void
simStageLockEngines(OOstage *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Burning || eng->state == SIM_Armed) {
      simEngineLock(eng);
    }
  }
}


