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

typedef struct {
  const char *name;
} InitScArgs;


//static hashtable_t *gSpacecraftClasses;

#if 0
void
simNewSpacecraftClass(const char *name, sim_spacecraft_t *(*alloc)(void),
                      void (*init)(sim_spacecraft_t *sc))
{
  SCclass *cls = malloc(sizeof(SCclass));

  cls->name = strdup(name);
  cls->alloc = alloc;
  cls->init = init;
  cls->dealloc = NULL;

  hashtable_insert(gSpacecraftClasses, name, cls);
}
#endif

sim_spacecraft_t*
simNewSpacecraft(const char *className, const char *scName)
{
  sim_class_t *cls = sim_class_get(className);
  //SCclass *cls = hashtable_lookup(gSpacecraftClasses, className);
  if (!cls) {
    ooLogError("no such spacecraft class '%s'", className);
    return NULL;
  }

  sim_spacecraft_t *sc = cls->alloc(cls);
  InitScArgs args = {""};
  cls->init(cls, sc, &(InitScArgs){scName});
  simScInit(sc, scName);

  //cls->init(sc);

  plUpdateMass(sc->obj);
  ooScSetScene(sc, sgGetScene(simGetSg(), "main"));

  //  char *axises;
  // asprintf(&axises, "/sc/%s/axis", scName);

  sim_record_t *rec = simPubsubMakeRecord(sc->rec, "axis");

  if (rec) {
    simPublishValue(rec, SIM_TYPE_FLOAT, "yaw", &sc->axises.yaw);
    simPublishValue(rec, SIM_TYPE_FLOAT, "roll", &sc->axises.roll);
    simPublishValue(rec, SIM_TYPE_FLOAT, "pitch", &sc->axises.pitch);
    simPublishValue(rec, SIM_TYPE_FLOAT, "lateral", &sc->axises.lateral);
    simPublishValue(rec, SIM_TYPE_FLOAT, "vertical", &sc->axises.vertical);
    simPublishValue(rec, SIM_TYPE_FLOAT, "forward", &sc->axises.fwd);
    simPublishValue(rec, SIM_TYPE_FLOAT, "orbital", &sc->axises.orbital);
    simPublishValue(rec, SIM_TYPE_FLOAT, "throttle", &sc->axises.throttle);
  }

  return sc;
}


void
ooGetAxises(OOaxises *axises)
{
  SIM_VAL(axises->pitch) = ioGetAxis(IO_AXIS_Y);
  SIM_VAL(axises->roll) = ioGetAxis(IO_AXIS_X);
  SIM_VAL(axises->yaw) = ioGetAxis(IO_AXIS_RZ);

  SIM_VAL(axises->vertical) = ioGetAxis(IO_AXIS_Z);
  SIM_VAL(axises->lateral) = ioGetAxis(IO_AXIS_RX);
  SIM_VAL(axises->fwd) = ioGetAxis(IO_AXIS_RY);

  SIM_VAL(axises->orbital) = ioGetSlider(IO_SLIDER_THROT_0);
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
simDefaultDetatch(sim_spacecraft_t *sc)
{

}

void
simDefaultPrestep(sim_spacecraft_t *sc, double dt)
{

}

void
simDefaultPoststep(sim_spacecraft_t *sc, double dt)
{

}

void
simDefaultEngineToggle(sim_spacecraft_t *sc)
{
}

void
simDefaultAxisUpdate(sim_spacecraft_t *sc)
{
}




void
simScInit(sim_spacecraft_t *sc, const char *name)
{
  sc->name = strdup(name);

  char *sckey;
  asprintf(&sckey, "/sc/%s", name);
  sc->rec = simPubsubCreateRecord(sckey);
  free(sckey);

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
sim_spacecraft_t*
ooScNew(PLworld *world, SGscene *scene, const char *name)
{
  sim_spacecraft_t *sc = malloc(sizeof(sim_spacecraft_t));

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
simDetatchStage(sim_spacecraft_t *sc, sim_stage_t *stage)
{
  stage->state = OO_Stage_Detatched;
  plDetatchObject(stage->obj);
}

void
simScDetatchStage(sim_spacecraft_t *sc)
{
  if (sc->detatchPossible) {
    sc->detatchStage(sc);
    sc->detatchSequence ++;
  }
}

void
simScToggleMainEngine(sim_spacecraft_t *sc)
{
  sc->toggleMainEngine(sc);
}

PLobject*
ooScGetPLObjForSc(sim_spacecraft_t *sc)
{
  return (PLobject*)sc->obj;
}


void
simScStep(sim_spacecraft_t *sc, float dt)
{
  assert(sc != NULL);


  sc->expendedMass = 0.0;

  OOaxises axises;
  ooGetAxises(&axises);

  sc->prestep(sc, dt);
  sc->axisUpdate(sc);
  for (size_t i = 0 ; i < sc->stages.length ; ++ i) {
    sim_stage_t *stage = sc->stages.elems[i];
    ooScStageStep(stage, &axises, dt);
  }

  plMassMod(&sc->obj->m, sc->obj->m.m - sc->expendedMass);

  sc->poststep(sc, dt);
}

void // for scripts and events
ooScForce(sim_spacecraft_t *sc, float rx, float ry, float rz)
{
  plForceRelative3f(sc->obj, rx, ry, rz);
}


void
ooScSetStageMesh(sim_stage_t *stage, SGdrawable *mesh)
{
  assert(stage != NULL);
  assert(mesh != NULL);
  stage->obj->drawable = mesh;
}

void
ooScSyncStage(sim_stage_t *stage)
{
  //  stage->mesh->p;
  //  stage->mesh->q;
}

void
ooScSyncSpacecraft(sim_spacecraft_t *sc)
{
  for (size_t i = 0 ; i < sc->stages.length; ++ i) {
    ooScSyncStage(sc->stages.elems[i]);
  }
}

void
ooScStageStep(sim_stage_t *stage, OOaxises *axises, float dt) {
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
    //double axisVal = ooIoGetAxis(NULL, axisKeys[i]);
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

sim_stage_t*
simNewStage(sim_spacecraft_t *sc, const char *name, const char *mesh)
{
  sim_stage_t *stage = malloc(sizeof(sim_stage_t));
  stage->state = OO_Stage_Idle;
  stage->sc = sc;
  stage->expendedMass = 0.0;
  stage->rec = simPubsubMakeRecord(sc->rec, name);

  obj_array_init(&stage->engines);
  obj_array_init(&stage->actuatorGroups);
  obj_array_init(&stage->payload);
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    OOactuatorgroup *actGroup = ooScNewActuatorGroup(ooGetActuatorGroupName(i));
    obj_array_push(&stage->actuatorGroups, actGroup);
  }

  stage->obj = plSubObject3f(sc->world, sc->obj, name, 0.0, 0.0, 0.0);

  obj_array_push(&sc->stages, stage);

  // Load stage model
  SGdrawable *model = sgLoadModel(mesh);
  sgDrawableLoadShader(model, "spacecraft");
  ooScSetStageMesh(stage, model);
  sgSceneAddObj(sgGetScene(simGetSg(), "main"), model);

  return stage;
}

void
scStageSetOffset3f(sim_stage_t *stage, float x, float y, float z)
{
  stage->pos = vf3_set(x, y, z);
  stage->obj->p_offset = vf3_set(x, y, z);
}

void
scStageSetOffset3fv(sim_stage_t *stage, float3 p)
{
  stage->pos = p;
  stage->obj->p_offset = p;
}

void
ooScSetScene(sim_spacecraft_t *spacecraft, SGscene *scene)
{
  for (int i = 0 ; i < spacecraft->stages.length ; ++i) {
    sim_stage_t *stage = spacecraft->stages.elems[i];

    if (stage->state != OO_Stage_Detatched) {
      sgSceneAddObj(scene, stage->obj->drawable);
    }
  }
}

void
ooScSetSystem(sim_spacecraft_t *spacecraft, PLsystem *sys)
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
ooScSetPos(sim_spacecraft_t *sc, double x, double y, double z)
{
  plSetObjectPos3d(sc->obj, x, y, z);
}

void
ooScSetSystemAndPos(sim_spacecraft_t *sc, const char *sysName,
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
ooScSetSysAndCoords(sim_spacecraft_t *sc, const char *sysName,
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
simInitStage(sim_stage_t *stage)
{
}

void
simAddStage(sim_spacecraft_t *sc, sim_stage_t *stage)
{
  obj_array_push(&sc->stages, stage);
}

// TODO: This is buggy, gravity is only set on the parent object unless the
//       stage has been detatched.
float3
simGetStageGravityVector(sim_stage_t *stage)
{
  return stage->obj->g_ack;
}

float3
simGetGravityVector(sim_spacecraft_t *sc)
{
  // TODO: Return the real gravity instead of force ackumulator
  return sc->obj->g_ack;
}

float3
simGetVelocityVector(sim_spacecraft_t *sc)
{
  return sc->obj->v;
}

float3
simGetAirspeedVector(sim_spacecraft_t *sc)
{
  return plComputeAirvelocity(sc->obj);
}

float3
simGetForceVector(sim_spacecraft_t *sc)
{
  return sc->obj->f_ack;
}

quaternion_t
simGetQuaternion(sim_spacecraft_t *sc)
{
  return sc->obj->q;
}

const float3x3*
simGetRotMat(sim_spacecraft_t *sc)
{
  return (const float3x3*)&sc->obj->R;
}

float
simGetAltitude(sim_spacecraft_t *sc)
{
  return plComputeAltitude(sc->obj);
}


float3
simGetRelPos(sim_spacecraft_t *sc)
{
  return ooLwcDist(&sc->obj->p, &sc->obj->sys->orbitalBody->obj.p);
}

float3
simGetRelVel(sim_spacecraft_t *sc)
{
  return sc->obj->v - sc->obj->sys->orbitalBody->obj.v;
}

PLsystem*
simGetSys(sim_spacecraft_t *sc)
{
  return sc->obj->sys;
}


void
simStageArmEngines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Disarmed) {
      simEngineArm(eng);
    }
  }
}

void
simStageDisarmEngines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Armed) {
      simEngineDisarm(eng);
    }
  }
}

void
simStageDisableEngines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Burning) {
      simEngineDisable(eng);
    }
  }
}

void
simStageLockEngines(sim_stage_t *stage)
{
  ARRAY_FOR_EACH(i, stage->engines) {
    SIMengine *eng = ARRAY_ELEM(stage->engines, i);
    if (eng->state == SIM_Burning || eng->state == SIM_Armed) {
      simEngineLock(eng);
    }
  }
}



static void
InitSpacecraft(sim_class_t *cls, void *obj, void *arg)
{
  SIM_SUPER_INIT(cls, obj, arg);

  sim_spacecraft_t *sc = obj;
  InitScArgs *args = arg;

  sc->super.name = strdup(args->name);

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
  sc->obj = plObject(world, args->name);
  sc->scene = sgGetScene(sg, "main"); // Just use any of the existing ones
  sc->expendedMass = 0.0;
  sc->mainEngineOn = false;
  sc->toggleMainEngine = simDefaultEngineToggle;
  sc->axisUpdate = simDefaultAxisUpdate;
  plMassSet(&sc->obj->m, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  plSetSystem(world->rootSys, sc->obj);
}

typedef struct {
  int dummy;
} InitStageArgs;

static void
InitStage(sim_class_t *cls, void *obj, void *arg)
{
  SIM_SUPER_INIT(cls, obj, arg);
  sim_stage_t *stage = obj;
}


MODULE_INIT(spacecraft, "object", NULL)
{
  //  gSpacecraftClasses = hashtable_new_with_str_keys(128);
  ooLogError("init sim module");
  sim_class_t *sc_class = sim_register_class("Object", "Spacecraft",
                                             InitSpacecraft,
                                             sizeof(sim_spacecraft_t));
  sim_class_t *stage_class = sim_register_class("Object", "Stage",
                                                InitStage,
                                                sizeof(sim_stage_t));

  // Fields in the spacecraft class
  sim_class_add_field(sc_class, SIM_TYPE_OBJ_ARR,
                      "stages", offsetof(sim_spacecraft_t, stages));
  sim_class_add_field(sc_class, SIM_TYPE_OBJ_ARR,
                      "engines", offsetof(sim_spacecraft_t, engines));


  // Fields in the stage class
  sim_class_add_field(stage_class, SIM_TYPE_OBJ,
                      "sc", offsetof(sim_stage_t, sc));
  sim_class_add_field(stage_class, SIM_TYPE_FLOAT_VEC3,
                      "pos", offsetof(sim_stage_t, pos));
  sim_class_add_field(stage_class, SIM_TYPE_OBJ_ARR,
                      "engines", offsetof(sim_stage_t, engines));
  sim_class_add_field(stage_class, SIM_TYPE_OBJ_ARR,
                      "payload", offsetof(sim_stage_t, payload));
}


