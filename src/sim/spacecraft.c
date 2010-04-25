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
#include "actuator.h"
extern SIMstate gSIM_state;

DEF_ARRAY(OOdetatchinstr,detatchprog)

void
ooGetAxises(OOaxises *axises)
{
  axises->yaw = ooIoGetAxis("yaw");
  axises->pitch = ooIoGetAxis("pitch");
  axises->roll = ooIoGetAxis("roll");
  axises->horizontal = ooIoGetAxis("horizontal");
  axises->vertical = ooIoGetAxis("vertical");
  axises->thrust = ooIoGetAxis("thrust");
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


OOspacecraft* ooScGetCurrent(void)
{
  return gSIM_state.currentSc;
}

// TODO: Pass on PLsystem instead of PLworld to ensure that object has valid
//       systems at all times.
OOspacecraft*
ooScNew(PLworld *world, SGscene *scene)
{
  OOspacecraft *sc = malloc(sizeof(OOspacecraft));
  //ooObjVecInit(&sc->stages);
  //sc->mainEngine = NULL;
  //sc->body = dBodyCreate(world);
  sc->activeStageIdx = 0;
  obj_array_init(&sc->stages);
  sc->world = world;
  sc->prestep = NULL;
  sc->poststep = NULL;
  sc->detatchStage = NULL;

  sc->obj = plObject(world);
  sc->scene = scene;
  sc->detatchProg.pc = 0;
  detatchprog_array_init(&sc->detatchProg.instrs);

  plMassSet(&sc->obj->m, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  plSetSystem(world->rootSys, sc->obj);

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
ooScReevaluateMass(OOspacecraft *sc)
{
  memset(&sc->obj->m, 0, sizeof(PLmass));

  plMassSet(&sc->obj->m, 0.0,
            0.0, 0.0, 0.0,
            1.0, 1.0, 1.0,
            0.0, 0.0, 0.0);

  for (int i = 0 ; i < sc->stages.length ; ++ i) {
    OOstage *stage = sc->stages.elems[i];
    if (stage->state != OO_Stage_Detatched) {
      PLmass tmp = stage->obj->m;
      plMassTranslate(&tmp,
                      stage->pos[0],
                      stage->pos[1],
                      stage->pos[2]);

      plMassAdd(&sc->obj->m, &tmp);
    }
  }
}

void
ooScDetatch2(OOspacecraft *sc)
{
  if (sc->detatchProg.pc < sc->detatchProg.instrs.length) {
    OOdetatchinstr instr = sc->detatchProg.instrs.elems[sc->detatchProg.pc];

    for (int i = 0 ; i < instr.numStages ; ++ i) {
      instr.detatch(sc->stages.elems[instr.stageIdx+i]);
      ((OOstage*)sc->stages.elems[instr.stageIdx+i])->state = OO_Stage_Detatched;
    }
    sc->detatchProg.pc ++;

    ooScReevaluateMass(sc);
  }
}

void
ooScDetachStage(OOspacecraft *sc)
{
  int order = ((OOstage*)sc->stages.elems[sc->activeStageIdx])->detachOrder;

  // Move active stage index to next stage with higher detach order
  while (sc->activeStageIdx < sc->stages.length - 1 &&
         ((OOstage*)sc->stages.elems[sc->activeStageIdx])->detachOrder == order)
  {
    sc->activeStageIdx ++;
  }
//  OOstage *stage = ooObjVecPop(&sc->stages);
  // TODO: Insert in free object vector
//  dBodyEnable(stage->id);
}

PLobject*
ooScGetPLObjForSc(OOspacecraft *sc)
{
  return (PLobject*)sc->obj;
}


void
ooScStep(OOspacecraft *sc, float dt)
{
  assert(sc != NULL);

  OOaxises axises;
  ooGetAxises(&axises);

  for (size_t i = sc->activeStageIdx ; i < sc->stages.length ; ++ i) {
    OOstage *stage = sc->stages.elems[i];
    if (stage->state == OO_Stage_Enabled) {
      ooScStageStep(sc, stage, &axises, dt);
    }
  }

  float expendedFuel = 0.0f;
  plMassMod(&sc->obj->m, sc->obj->m.m - expendedFuel);
}

void // for scripts and events
ooScForce(OOspacecraft *sc, float rx, float ry, float rz)
{
//    dBodyAddRelForceAtRelPos(sc->body, rx, ry, rz, sc->);
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
ooScStageStep(OOspacecraft *sc, OOstage *stage, OOaxises *axises, float dt) {
  assert(sc != NULL);
  assert(stage != NULL);
  assert(axises != NULL);

  // Handle for all actuators call actuator handlers
  const static char * axisKeys[] = {
    "orbital", "vertical", "horizontal", "thrust", "pitch", "roll", "yaw"
  };
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    OOactuatorgroup *actGroup = (OOactuatorgroup*)stage->actuatorGroups.elems[i];
    double axisVal = ooIoGetAxis(axisKeys[i]);
    for (int j = 0 ; j < actGroup->actuators.length ; ++j) {
      OOactuator *act = actGroup->actuators.elems[j];
      // TODO: Will not really work, we need more parameters if an actuator is
      //       used for multiple functions.
      act->axisUpdate(act, axisVal);
    }
  }


  for (size_t i = 0 ; i < stage->actuators.length; ++ i) {
    OOactuator *act = stage->actuators.elems[i];

    if (act->state == OO_Act_Burning ||
        act->state == OO_Act_Fault_Open)
    {
      act->step(act, dt);
    }
  }

  float expendedFuel = 0.0f;
  plMassMod(&stage->obj->m, stage->obj->m.m - expendedFuel);
}

typedef int (*qsort_compar_t)(const void *, const void *);
static int compar_stages(const OOstage **s0, const OOstage **s1) {
  return (*s1)->detachOrder - (*s0)->detachOrder;
}


OOstage*
ooScNewStage(OOspacecraft *sc)
{
  OOstage *stage = malloc(sizeof(OOstage));
  stage->state = OO_Stage_Idle;
  stage->sc = sc;
  // Actuator arrays
  obj_array_init(&stage->actuators);
  obj_array_init(&stage->actuatorGroups);
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    OOactuatorgroup *actGroup = ooScNewActuatorGroup(ooGetActuatorGroupName(i));
    obj_array_push(&stage->actuatorGroups, actGroup);
  }
  stage->detachOrder = 0;
  stage->obj = plSubObject3f(sc->world, sc->obj, 0.0, 0.0, 0.0);

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
ooScStageAddActuator(OOstage *stage, OOactuator *actuator)
{
  obj_array_push(&stage->actuators, actuator);
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
    OOrocket *engine = ooScNewThruster(newStage /*sc*/,
                                       thrusterName.u.str,
                                       thrust,
                                       pos[0], pos[1], pos[2],
                                       dir[0], dir[1], dir[2]);
    ooScStageAddActuator(newStage, (OOactuator*)engine);
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
      OOsrb *engine = ooScNewSrb(newStage /*sc*/,
                                 engineName.u.str,
                                 thrust,
                                 pos[0], pos[1], pos[2],
                                 dir[0], dir[1], dir[2]);
      ooScStageAddActuator(newStage, (OOactuator*)engine);
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
    OOrocket *engine = ooScNewLoxEngine(newStage /*sc*/,
                                        engineName.u.str,
                                        thrust,
                                        pos[0], pos[1], pos[2],
                                        dir[0], dir[1], dir[2],
                                        0.0f);
    ooScStageAddActuator(newStage, (OOactuator*)engine);
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
  OOstage *newStage = ooScNewStage(sc);
  for (HRMLobject *stageEntry = stage->children ; stageEntry != NULL; stageEntry = stageEntry->next) {
    if (!strcmp(stageEntry->name, "detach-order")) {
      newStage->detachOrder = hrmlGetInt(stageEntry);
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
  OOspacecraft *sc = ooScNew(world, scene);

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
  qsort(&sc->stages.elems[0], sc->stages.length, sizeof(void*),
        (qsort_compar_t)compar_stages);
  free(path);

  ooScReevaluateMass(sc);
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
