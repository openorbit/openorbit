/* 
 The contents of this file are subject to the Mozilla Public License
 Version 1.1 (the "License"); you may not use this file except in compliance
 with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS" basis,
 WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 for the specific language governing rights and limitations under the
 License.
 
 The Original Code is the Open Orbit space flight simulator.
 
 The Initial Developer of the Original Code is Mattias Holm. Portions
 created by the Initial Developer are Copyright (C) 2009 the
 Initial Developer. All Rights Reserved.
 
 Contributor(s):
 Mattias Holm <mattias.holm(at)openorbit.org>.
 
 Alternatively, the contents of this file may be used under the terms of
 either the GNU General Public License Version 2 or later (the "GPL"), or
 the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
 which case the provisions of GPL or the LGPL License are applicable instead
 of those above. If you wish to allow use of your version of this file only
 under the terms of the GPL or the LGPL and not to allow others to use your
 version of this file under the MPL, indicate your decision by deleting the
 provisions above and replace them with the notice and other provisions
 required by the GPL or the LGPL. If you do not delete the provisions
 above, a recipient may use your version of this file under either the MPL,
 the GPL or the LGPL."
 */

#include <assert.h>

#include "log.h" 
#include "sim.h"
#include "sim/spacecraft.h"

extern SIMstate gSIM_state;


OOspacecraft* ooScGetCurrent()
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
  ooObjVecInit(&sc->stages);
  sc->mainEngine = NULL;
  sc->id = dBodyCreate(world);
  
  for (size_t i = 0 ; i < stageCount ; i ++) {
    ooObjVecPush(&sc->stages, ooScStageNew(world, 100.0));
  }
  
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
  OOstage *stage = ooObjVecPop(&sc->stages);
  // TODO: Insert in free object vector
  dBodyEnable(stage->id);
}
 
void
ooScStep(OOspacecraft *sc)
{
  assert(sc != NULL);
  
  if (sc->mainEngine) {
    switch (sc->mainEngine->state) {
    case OO_Engine_Disabled:
      ooLogInfo("step on turned of engine");
      break;
    case OO_Engine_Enabled:
      ooLogInfo("step on burning engine");
      break;
    case OO_Engine_Fault:
      ooLogInfo("engine step on faulty engine");
      break;
    default:
      assert(0 && "invalid case");
    }
  }
}

void
ooScForce(OOspacecraft *sc, v4f_t f)
{
//  dBodyForceRelative();
}
