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

#include "log.h" 
#include "sim.h"
#include "sim/spacecraft.h"
#include "res-manager.h"
#include "parsers/hrml.h"

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
  sc->id = dBodyCreate(world);
  
  //for (size_t i = 0 ; i < stageCount ; i ++) {
  //  ooObjVecPush(&sc->stages, ooScStageNew(world, 100.0));
  //}
  
  return sc;
}


void
dMassSetConeTotal(dMass *m, dReal total_mass,
                  dReal radius, dReal height)
{
  
  float i11 = 1.0/10.0 * total_mass * height * height + 
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
//  dMassAdjust(dMass *, dReal newmass);
//  dMassRotate(dMass *, const dMatrix3 R);
//  dMassTranslate(dMass *, dReal x, dReal y, dReal z);
//  dMassAdd(dMass *a, const dMass *b);
//  dMassSetBoxTotal(dMass *, dReal total_mass, dReal lx, dReal ly, dReal lz);
//  dMassSetCylinderTotal(dMass *, dReal total_mass, int direction, dReal radius, dReal length);
//  dMassSetCapsuleTotal(dMass *, dReal total_mass, int direction, dReal radius, dReal length);
//  dMassSetSphereTotal(dMass *, dReal total_mass, dReal radius);
//  dMassSetParameters(dMass *, dReal themass,
//                     dReal cgx, dReal cgy, dReal cgz,
//                     dReal I11, dReal I22, dReal I33,
//                     dReal I12, dReal I13, dReal I23);
//  dMassSetZero(dMass *);
//  
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
  
  //if (sc->mainEngine) {
  //  switch (sc->mainEngine->state) {
  //  case OO_Engine_Disabled:
  //    ooLogInfo("step on turned of engine");
  //    break;
  //  case OO_Engine_Enabled:
  //    ooLogInfo("step on burning engine");
  //    break;
  //  case OO_Engine_Fault:
  //    ooLogInfo("engine step on faulty engine");
  //    break;
  //  default:
  //    assert(0 && "invalid case");
  //  }
  //}
}

void
ooScForce(OOspacecraft *sc, v4f_t f)
{
//  dBodyForceRelative();
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
      
    }
  }

  hrmlFreeDocument(spaceCraftDoc);

  ooLogInfo("loaded spacecraft %s", scName.u.str);
  return sc;
}
