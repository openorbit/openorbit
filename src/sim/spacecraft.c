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


OOspacecraft* ooScGetCurrent(void)
{
  return gSIM_state.currentSc;
}

OOstage*
ooScStageNew(dWorldID world, float m)
{
  OOstage *stage = malloc(sizeof(OOstage));

  stage->id = dBodyCreate(world);

  dMass mass;
  dMassSetZero(&mass);
  dBodySetMass(stage->id, &mass);
  dBodyDisable(stage->id);

  return stage;
}

OOspacecraft*
ooScNew(dWorldID world, size_t stageCount)
{
  OOspacecraft *sc = malloc(sizeof(OOspacecraft));
  //ooObjVecInit(&sc->stages);
  //sc->mainEngine = NULL;
  sc->body = dBodyCreate(world);

  //for (size_t i = 0 ; i < stageCount ; i ++) {
  //  ooObjVecPush(&sc->stages, ooScStageNew(world, 100.0));
  //}

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
  ooObjVecPush(&sc->stages, stage);
}

void
ooScDetatchStage(OOspacecraft *sc)
{
//  OOstage *stage = ooObjVecPop(&sc->stages);
  // TODO: Insert in free object vector
//  dBodyEnable(stage->id);
}

void
ooScStep(OOspacecraft *sc)
{
  assert(sc != NULL);
  for (size_t i = 0 ; i < sc->stages.length ; ++ i) {
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
ooScStageStep(OOspacecraft *sc, OOstage *stage) {
  assert(sc != NULL);
  assert(stage != NULL);

  for (size_t i = 0 ; i < stage->engines.length; ++ i) {
    OOengine *engine = stage->engines.elems[i];
    if (engine->state == OO_Engine_Burning ||
        engine->state == OO_Engine_Fault_Open)
    {
      dBodyAddRelForceAtRelPos(sc->body,
                               engine->dir.x * engine->forceMag,
                               engine->dir.y * engine->forceMag,
                               engine->dir.z * engine->forceMag,
                               engine->p.x, engine->p.y, engine->p.z);
    }
  }
}

typedef int (*qsort_compar_t)(const void *, const void *);
static int compar_stages(const OOstage **s0, const OOstage **s1) {
  return (*s0)->detatchOrder - (*s1)->detatchOrder;
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
  engine->p.v = v3f_make(x, y, z);
  engine->dir.v = v3f_make(dx, dy, dz);

  return engine;
}

OOstage*
ooScNewStage(void)
{
  OOstage *stage = malloc(sizeof(OOstage));
  stage->state = OO_Stage_Idle;
  ooObjVecInit(&stage->engines);
  ooObjVecInit(&stage->torquers);
  //stage->id = dBodyNew(...) // In which dWorld?;
  stage->detatchOrder = 0;

  return stage;
}

void
ooScStageAddEngine(OOstage *stage, OOengine *engine)
{
  ooObjVecPush(&stage->engines, engine);
}


OOspacecraft*
ooScLoad(const char *fileName)
{
  char *path = ooResGetPath(fileName);
  HRMLdocument *spaceCraftDoc = hrmlParse(path);
  free(path);

  if (spaceCraftDoc == NULL) {
    // Parser is responsible for pestering the users with errors for now.
    return NULL;
  }

  OOspacecraft *sc = NULL;
  HRMLobject *root = hrmlGetRoot(spaceCraftDoc);
  HRMLvalue scName = hrmlGetAttrForName(root, "name");

  for (HRMLobject *node = root; node != NULL; node = node->next) {
    if (!strcmp(node->name, "spacecraft")) {
      for (HRMLobject *child = node->children; child != NULL ; child = child->next) {
        if (!strcmp(child->name, "stages")) {
          for (HRMLobject *stage = child->children; stage != NULL ; stage = stage->next) {
            HRMLvalue stageName = hrmlGetAttrForName(stage, "name");
            OOstage *newStage = ooScNewStage();
            for (HRMLobject *stageEntry = stage->children ; stageEntry != NULL; stageEntry = stageEntry->next) {
              if (!strcmp(stageEntry->name, "detach-order")) {
                newStage->detatchOrder = hrmlGetInt(stageEntry);
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
                    const double *pos;
                    const double *dir;
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
                        const double *dir = hrmlGetRealArray(engine);
                        size_t len = hrmlGetRealArrayLen(engine);
                        assert(len == 3 && "dir must be a 3 component real vector");
                      }
                    }

                    OOengine *engine = ooScNewEngine(sc, thrust,
                                                     pos[0], pos[1], pos[2],
                                                     dir[0], dir[1], dir[2]);
                    ooScStageAddEngine(newStage, engine);
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
        }
      }
    }
  }

  hrmlFreeDocument(spaceCraftDoc);

  ooLogInfo("loaded spacecraft %s", scName.u.str);

  // Ensure that stage vector is sorted by detachOrder
  qsort(&sc->stages.elems[0], sc->stages.length, sizeof(void*),
        (qsort_compar_t)compar_stages);

  return sc;
}
