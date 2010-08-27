/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "common/moduleinit.h"
#include "actuator.h"


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
  obj_array_init(&sc->actuators);

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


void
simArmStageActuators(OOstage *stage)
{
  for (int i = 0 ; i < stage->actuators.length ; ++i) {
    OOactuator *act = stage->actuators.elems[i];
    if (act->state == OO_Act_Disarmed) {
      simArmActuator(stage->actuators.elems[i]);
    }
  }
}

void
simDisarmStageActuators(OOstage *stage)
{
  for (int i = 0 ; i < stage->actuators.length ; ++i) {
    OOactuator *act = stage->actuators.elems[i];
    if (act->state == OO_Act_Armed) {
      simDisarmActuator(act);
    }
  }
}

void
simDisableStageActuators(OOstage *stage)
{
  for (int i = 0 ; i < stage->actuators.length ; ++i) {
    OOactuator *act = stage->actuators.elems[i];
    if (act->state == OO_Act_Enabled) {
      simDisableActuator(act);
    }
  }
}
void
simLockStageActuators(OOstage *stage)
{
  for (int i = 0 ; i < stage->actuators.length ; ++i) {
    OOactuator *act = stage->actuators.elems[i];
    simLockActuator(act);
  }
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
      OOactuator *act = actGroup->actuators.elems[j];
      // TODO: Will not really work, we need more parameters if an actuator is
      //       used for multiple functions.
      act->axisUpdate(act, axisVal);
    }
  }


  for (size_t i = 0 ; i < stage->actuators.length; ++ i) {
    OOactuator *act = stage->actuators.elems[i];

    if (act->state & SIM_ACTUATOR_ON_MASK) {
      ooLogInfo("burning actuator");
      act->step(act, dt);
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
  // Actuator arrays
  obj_array_init(&stage->actuators);
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


static void
loadActuatorGroups(HRMLobject *actGroups, OOstage *newStage)
{
  for (HRMLobject *actGroup = actGroups->children;
       actGroup != NULL ; actGroup = actGroup->next)
  {
    if (!strcmp(actGroup->name, "group")) {
      HRMLvalue nameAttr = hrmlGetAttrForName(actGroup, "name");

      for (HRMLobject *groupEntry = actGroup->children;
           groupEntry != NULL ; groupEntry = groupEntry->next)
      {
        if (!strcmp(groupEntry->name, "group-entry")) {
          const char *actuatorName = hrmlGetStr(groupEntry);
          // TODO: Register actuator in named group
          if (nameAttr.typ == HRMLStr) {
            OOactuator * act = NULL;
            for (int i = 0 ; i < newStage->actuators.length ; ++ i) {
              OOactuator *tmp = newStage->actuators.elems[i];
              if (!strcmp(tmp->name, actuatorName)) {
                act = tmp;
              }
            }
            assert(act && "unknown actuator");
            int actuatorGroupId = ooGetActuatorGroupId(nameAttr.u.str);

            if (actuatorGroupId != -1) {
              ooScRegisterInGroup(newStage->actuatorGroups.elems[actuatorGroupId], act);
            }
          } else if (nameAttr.typ == HRMLInt) {
            assert(0 && "custom actuator groups not yet supported");
          } else {
            assert(0 && "invalid type for group name");
          }
        }
      }
    } else {
      assert(0 && "only 'group' children allowed under 'actuator-groups'");
    }
  } // Attitude for loop
}

static void
loadThruster(HRMLobject *thruster, OOstage *newStage)
{
  HRMLvalue thrusterName = hrmlGetAttrForName(thruster, "name");
  assert(thrusterName.typ == HRMLStr);
  const double *pos = NULL;
  const double *dir = NULL;
  double thrust = 0.0;
  for (HRMLobject *thrAttr = thruster->children; thrAttr != NULL ; thrAttr = thrAttr->next) {
    if (!strcmp(thrAttr->name, "thrust")) {
      thrust = hrmlGetReal(thrAttr);
    } else if (!strcmp(thrAttr->name, "pos")) {
      pos = hrmlGetRealArray(thrAttr);
      size_t len = hrmlGetRealArrayLen(thrAttr);
      assert(len == 3 && "pos must be a 3 component real vector");
    } else if (!strcmp(thrAttr->name, "dir")) {
      dir = hrmlGetRealArray(thrAttr);
      size_t len = hrmlGetRealArrayLen(thrAttr);
      assert(len == 3 && "dir must be a 3 component real vector");
    }
  }
  if (pos && dir) {
  //  OOrocket *engine = ooScNewThruster(newStage /*sc*/,
  //                                     thrusterName.u.str,
  //                                     thrust,
  //                                     pos[0], pos[1], pos[2],
   //                                    dir[0], dir[1], dir[2]);
  //  ooScStageAddActuator(newStage, (OOactuator*)engine);
  } else {
    fprintf(stderr, "no pos or direction of engine found\n");
  }
}

static void
loadSolidRocket(HRMLobject *solidRocket, OOstage *newStage)
{
    HRMLvalue engineName = hrmlGetAttrForName(solidRocket, "name");
    assert(engineName.typ == HRMLStr);
    const double *pos = NULL;
    const double *dir = NULL;
    double thrust = 0.0;
    for (HRMLobject *srAttribute = solidRocket->children; srAttribute != NULL ; srAttribute = srAttribute->next) {
      if (!strcmp(srAttribute->name, "thrust")) {
        thrust = hrmlGetReal(srAttribute);
      } else if (!strcmp(srAttribute->name, "pos")) {
        pos = hrmlGetRealArray(srAttribute);
        size_t len = hrmlGetRealArrayLen(srAttribute);
        assert(len == 3 && "pos must be a 3 component real vector");
      } else if (!strcmp(srAttribute->name, "dir")) {
        dir = hrmlGetRealArray(srAttribute);
        size_t len = hrmlGetRealArrayLen(srAttribute);
        assert(len == 3 && "dir must be a 3 component real vector");
      }
    }
    if (pos && dir) {
    //  OOsrb *engine = ooScNewSrb(newStage /*sc*/,
    //                             engineName.u.str,
    //                             thrust,
     //                            pos[0], pos[1], pos[2],
     //                            dir[0], dir[1], dir[2]);
     // ooScStageAddActuator(newStage, (OOactuator*)engine);

    } else {
      fprintf(stderr, "no pos or direction of engine found\n");
    }
}


static void
loadRocket(HRMLobject *rocket, OOstage *newStage)
{
  HRMLvalue engineName = hrmlGetAttrForName(rocket, "name");
  assert(engineName.typ == HRMLStr);
  const double *pos = NULL;
  const double *dir = NULL;
  double thrust = 0.0;
  for (HRMLobject *attr = rocket->children; attr != NULL ; attr = attr->next) {
    if (!strcmp(attr->name, "thrust")) {
      thrust = hrmlGetReal(attr);
    } else if (!strcmp(attr->name, "pos")) {
      pos = hrmlGetRealArray(attr);
      size_t len = hrmlGetRealArrayLen(attr);
      assert(len == 3 && "pos must be a 3 component real vector");
    } else if (!strcmp(attr->name, "dir")) {
      dir = hrmlGetRealArray(attr);
      size_t len = hrmlGetRealArrayLen(attr);
      assert(len == 3 && "dir must be a 3 component real vector");
    }
  }
  if (pos && dir) {
  //  OOrocket *engine = ooScNewLoxEngine(newStage /*sc*/,
  //                                      engineName.u.str,
  //                                      thrust,
  //                                      pos[0], pos[1], pos[2],
  //                                      dir[0], dir[1], dir[2],
  //                                      0.0f);
  //  ooScStageAddActuator(newStage, (OOactuator*)engine);
  } else {
    fprintf(stderr, "no pos or direction of engine found\n");
  }
}

static void
loadActuators(HRMLobject *stageEntry, OOstage *newStage)
{
  HRMLobject *actGroups = NULL;

  for (HRMLobject *prop = stageEntry->children; prop != NULL ; prop = prop->next) {
    if (!strcmp(prop->name, "solid-rocket")) {
      loadSolidRocket(prop, newStage);
    } else if (!strcmp(prop->name, "thruster")) {
      loadThruster(prop, newStage);
    } else if (!strcmp(prop->name, "rocket")) {
      loadRocket(prop, newStage);
    } else if (!strcmp(prop->name, "actuator-groups")) {
      actGroups = prop;
    } // Propulsion for loop
  }
  if (actGroups != NULL) loadActuatorGroups(actGroups, newStage);
}
static void
loadStage(HRMLobject *stage, OOspacecraft *sc, const char *filePath)
{
  const double *inertia = NULL;
  double mass = NAN, fuelMass= NAN;
  const double *stagePos = NULL;
  const double *stageCog = NULL;

  HRMLvalue stageName = hrmlGetAttrForName(stage, "name");
  OOstage *newStage = ooScNewStage(sc, stageName.u.str);
  for (HRMLobject *stageEntry = stage->children ; stageEntry != NULL; stageEntry = stageEntry->next) {
    if (!strcmp(stageEntry->name, "detach-order")) {
//      newStage->detachOrder = hrmlGetInt(stageEntry);
    } else if (!strcmp(stageEntry->name, "mass")) {
      mass = hrmlGetReal(stageEntry);
    } else if (!strcmp(stageEntry->name, "fuel-mass")) {
      fuelMass = hrmlGetReal(stageEntry);
    } else if (!strcmp(stageEntry->name, "inertial-tensor")) {
      inertia = hrmlGetRealArray(stageEntry);
      size_t len = hrmlGetRealArrayLen(stageEntry);
      assert(len == 9 && "inertia tensor must be a 9 component real vector");
    } else if (!strcmp(stageEntry->name, "pos")) {
      stagePos = hrmlGetRealArray(stageEntry);
      size_t len = hrmlGetRealArrayLen(stageEntry);
      assert(len == 3 && "pos vector must be a 3 component real vector");
    } else if (!strcmp(stageEntry->name, "cog")) {
      stageCog = hrmlGetRealArray(stageEntry);
      size_t len = hrmlGetRealArrayLen(stageEntry);
      assert(len == 3 && "cog vector must be a 3 component real vector");
    } else if (!strcmp(stageEntry->name, "actuators")) {
      loadActuators(stageEntry, newStage);
        // End actuators
    } else if (!strcmp(stageEntry->name, "model")) {
      const char *modelName = hrmlGetStr(stageEntry);
      char *pathCopy = strdup(filePath);
      char *lastSlash = strrchr(pathCopy, '/');
      lastSlash[1] = '\0'; // terminate string here and append model
                           // filename
      char *modelPath;
      asprintf(&modelPath, "%s/%s", pathCopy, modelName);

      SGdrawable *drawable = sgLoadModel(modelPath);
      ooScSetStageMesh(newStage, drawable);
      sgSceneAddObj(sc->scene, drawable);

      free(modelPath);
      free(pathCopy);
    }
  } // For all properties in stage

  assert(stageCog && inertia && stagePos);
  plMassSet(&newStage->obj->m, 1.0f, // Default to 1.0 kg
            0.0f, 0.0f, 0.0f,
            inertia[0], inertia[4], inertia[8],
            inertia[1], inertia[2], inertia[5]);

  plMassSetMin(&newStage->obj->m, mass);
  if (isnormal(fuelMass)) mass += fuelMass;
  plMassMod(&newStage->obj->m, mass);
  plMassTranslate(&newStage->obj->m, stageCog[0], stageCog[1], stageCog[2]);
  scStageSetOffset3f(newStage, stagePos[0], stagePos[1], stagePos[2]);
}

OOspacecraft*
ooScLoad(PLworld *world, SGscene *scene, const char *fileName)
{
  char *path = ooResGetPath(fileName);
  HRMLdocument *spaceCraftDoc = hrmlParse(path);


  if (spaceCraftDoc == NULL) {
    // Parser is responsible for pestering the users with errors for now.
    free(path);
    return NULL;
  }
  HRMLobject *root = hrmlGetRoot(spaceCraftDoc);
  HRMLvalue scName = hrmlGetAttrForName(root, "name");
  OOspacecraft *sc = ooScNew(world, scene, scName.u.str);

  for (HRMLobject *node = root; node != NULL; node = node->next) {
    if (!strcmp(node->name, "spacecraft")) {
      for (HRMLobject *child = node->children; child != NULL ; child = child->next) {
        if (!strcmp(child->name, "stages")) {
          for (HRMLobject *stage = child->children; stage != NULL ; stage = stage->next) {
            loadStage(stage, sc, path);
          } // For all stages
        }
      }
    }
  }

  hrmlFreeDocument(spaceCraftDoc);

  ooLogInfo("loaded spacecraft %s", scName.u.str);

  // Ensure that stage vector is sorted by detachOrder
  //qsort(&sc->stages.elems[0], sc->stages.length, sizeof(void*),
  //      (qsort_compar_t)compar_stages);
  free(path);

  plUpdateMass(sc->obj);
  return sc;
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
