/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include <ode/ode.h>
#include "log.h"
#include "sim.h"
#include "sim/spacecraft.h"
#include "res-manager.h"
#include "parsers/hrml.h"
#include <vmath/vmath.h>

extern SIMstate gSIM_state;

/*
  Compute the wing lift for a simple wing

  The simple wing relies of having precomputed lift coefficients. We allow one
  per 5 deg angle of attack. We actually intepolate between two angles to
  produce a little bit better results.
 */
void
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
}


OOspacecraft* ooScGetCurrent(void)
{
  return gSIM_state.currentSc;
}

OOspacecraft*
ooScNew(void)
{
  OOspacecraft *sc = malloc(sizeof(OOspacecraft));
  //ooObjVecInit(&sc->stages);
  //sc->mainEngine = NULL;
  //sc->body = dBodyCreate(world);
  sc->activeStageIdx = 0;
  obj_array_init(&sc->stages);

  return sc;
}


void
dMassSetConeTotal(dMass *m, dReal total_mass,
                  dReal radius, dReal height)
{
  float i11 = 3.0/5.0 * total_mass * height * height +
    3.0/20.0 * total_mass * radius * radius;
  float i22 = i11;
  float i33 = 3.0/10.0 * total_mass * radius * radius;
  float cogx, cogy, cogz;

  cogx = 0.0;
  cogy = 0.25 * height; // 1/4 from base, see wikipedia entry on Cone_(geometry)
  cogz = 0.0;


  dMassSetParameters(m, total_mass,
                     cogx, cogy, cogz, // TODO: fix, COG
                     i11, i22, i33,
                     0.0, 0.0, 0.0);
}

void
ooScAddStage(OOspacecraft *sc, OOstage *stage)
{
  obj_array_push(&sc->stages, stage);
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

void
ooScStep(OOspacecraft *sc)
{
  assert(sc != NULL);
  for (size_t i = sc->activeStageIdx ; i < sc->stages.length ; ++ i) {
    OOstage *stage = sc->stages.elems[i];
    if (stage->state == OO_Stage_Enabled) {
      ooScStageStep(sc, stage);
    }
  }
}

void // for scripts and events
ooScForce(OOspacecraft *sc, float rx, float ry, float rz)
{
//    dBodyAddRelForceAtRelPos(sc->body, rx, ry, rz, sc->);
}

void
ooScFireOrbital(OOspacecraft *sc)
{

}

void
ooScFireVertical(OOspacecraft *sc, float dv)
{

}
void
ooScFireHorizontal(OOspacecraft *sc, float dh)
{

}

void
ooScFireForward(OOspacecraft *sc)
{

}

void
ooScEngageYaw(OOspacecraft *sc, float dy)
{

}

void
ooScEngagePitch(OOspacecraft *sc, float dp)
{

}

void
ooScEngageRoll(OOspacecraft *sc, float dr)
{

}


void
ooScStageStep(OOspacecraft *sc, OOstage *stage) {
  assert(sc != NULL);
  assert(stage != NULL);

  for (size_t i = 0 ; i < stage->engines.length; ++ i) {
    OOengine *engine = stage->engines.elems[i];
    if (engine->state == OO_Engine_Burning ||
        engine->state == OO_Engine_Fault_Open)
    {
      dBodyAddRelForceAtRelPos(sc->body,
                               vf3_x(engine->dir) * engine->forceMag,
                               vf3_y(engine->dir) * engine->forceMag,
                               vf3_z(engine->dir) * engine->forceMag,
                               vf3_x(engine->p), vf3_y(engine->p), vf3_z(engine->p));
    }
  }
}

typedef int (*qsort_compar_t)(const void *, const void *);
static int compar_stages(const OOstage **s0, const OOstage **s1) {
  return (*s1)->detachOrder - (*s0)->detachOrder;
}

OOengine*
ooScNewEngine(OOspacecraft *sc,
              float f,
              float x, float y, float z,
              float dx, float dy, float dz)
{
  OOengine *engine = malloc(sizeof(OOengine));
  engine->sc = sc;
  engine->state = OO_Engine_Disabled;
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);

  return engine;
}

OOstage*
ooScNewStage(void)
{
  OOstage *stage = malloc(sizeof(OOstage));
  stage->state = OO_Stage_Idle;
  obj_array_init(&stage->engines);
  obj_array_init(&stage->torquers);
  //stage->id = dBodyNew(...) // In which dWorld?;
  stage->detachOrder = 0;

//  stage->id = dBodyCreate(world);

//  dMass mass;
//  dMassSetZero(&mass);
//  dBodySetMass(stage->id, &mass);
//  dBodyDisable(stage->id);

  return stage;
}

void
ooScStageAddEngine(OOstage *stage, OOengine *engine)
{
  obj_array_push(&stage->engines, engine);
}


OOspacecraft*
ooScLoad(const char *fileName)
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
  OOspacecraft *sc = ooScNew();

  for (HRMLobject *node = root; node != NULL; node = node->next) {
    if (!strcmp(node->name, "spacecraft")) {
      for (HRMLobject *child = node->children; child != NULL ; child = child->next) {
        if (!strcmp(child->name, "stages")) {
          for (HRMLobject *stage = child->children; stage != NULL ; stage = stage->next) {
            HRMLvalue stageName = hrmlGetAttrForName(stage, "name");
            OOstage *newStage = ooScNewStage();
            for (HRMLobject *stageEntry = stage->children ; stageEntry != NULL; stageEntry = stageEntry->next) {
              if (!strcmp(stageEntry->name, "detach-order")) {
                newStage->detachOrder = hrmlGetInt(stageEntry);
              } else if (!strcmp(stageEntry->name, "mass")) {
                newStage->mass = hrmlGetReal(stageEntry);
              } else if (!strcmp(stageEntry->name, "inertial-tensor")) {
                const double *arr = hrmlGetRealArray(stageEntry);
                size_t len = hrmlGetRealArrayLen(stageEntry);
                assert(len == 3 && "inertia tensor must be a 3 component real vector");
                newStage->inertia[0] = arr[0];
                newStage->inertia[1] = arr[1];
                newStage->inertia[2] = arr[2];
              } else if (!strcmp(stageEntry->name, "propulsion")) {
                for (HRMLobject *prop = stageEntry->children; prop != NULL ; prop = prop->next) {
                  if (!strcmp(prop->name, "engine")) {
                    HRMLvalue engineName = hrmlGetAttrForName(prop, "name");
                    const double *pos = NULL;
                    const double *dir = NULL;
                    double thrust;
                    for (HRMLobject *engine = prop->children; engine != NULL ; engine = engine->next) {
                      if (!strcmp(engine->name, "thrust")) {
                        thrust = hrmlGetReal(engine);
                      } else if (!strcmp(engine->name, "fire-once")) {
                        // TODO: Ignored for now
                      } else if (!strcmp(engine->name, "pos")) {
                        pos = hrmlGetRealArray(engine);
                        size_t len = hrmlGetRealArrayLen(engine);
                        assert(len == 3 && "pos must be a 3 component real vector");
                      } else if (!strcmp(engine->name, "dir")) {
                        dir = hrmlGetRealArray(engine);
                        size_t len = hrmlGetRealArrayLen(engine);
                        assert(len == 3 && "dir must be a 3 component real vector");
                      }
                    }

                    if (pos && dir) {
                      OOengine *engine = ooScNewEngine(sc, thrust,
                                                      pos[0], pos[1], pos[2],
                                                      dir[0], dir[1], dir[2]);
                      ooScStageAddEngine(newStage, engine);
                    } else {
                      fprintf(stderr, "no pos or direction of engine found\n");
                    }
                  }
                }
              } else if (!strcmp(stageEntry->name, "attitude")) {
                for (HRMLobject *att = stageEntry->children; att != NULL ; att = att->next) {
                  if (!strcmp(att->name, "engine")) {
                    assert(0 && "not implemented");
                  } else if (!strcmp(att->name, "torquer")) {
                    assert(0 && "not implemented");
                  }
                }
              }
            }
            ooScAddStage(sc, newStage);
          }
        } else if (!strcmp(child->name, "model")) {
          const char *modelName = hrmlGetStr(child);
          char *pathCopy = strdup(path);
          char *lastSlash = strrchr(pathCopy, '/');
          lastSlash[1] = '\0'; // terminate string here and append model
                               // filename
          char *modelPath;
          asprintf(&modelPath, "%s/%s", pathCopy, modelName);



          free(modelPath);
          free(pathCopy);
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
  return sc;
}
