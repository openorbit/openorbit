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


#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "actuator.h"
#include "spacecraft.h"


static const char *actuatorNames[OO_Act_Group_Count] = {
  [OO_Act_Orbital] = "orbital",
  [OO_Act_Vertical] = "vertical",
  [OO_Act_Horisontal] = "horisontal",
  [OO_Act_Forward] = "forward",
  [OO_Act_Pitch] = "pitch",
  [OO_Act_Roll] = "roll",
  [OO_Act_Yaw] = "yaw"
};

int
ooGetActuatorGroupId(const char *groupName)
{
  for (int i = 0 ; i < OO_Act_Group_Count ; ++i) {
    if (!strcmp(actuatorNames[i], groupName)) {
      return i;
    }
  }
  return -1;
}

const char*
ooGetActuatorGroupName(int groupId)
{
  assert(groupId < OO_Act_Group_Count);
  return actuatorNames[groupId];
}


float
ooGetThrottleForActuatorGroup(int groupId)
{

}

void
ooScFireOrbital(OOspacecraft *sc)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Orbital];

  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}

void
ooScFireVertical(OOspacecraft *sc, float dv)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Vertical];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}
void
ooScFireHorizontal(OOspacecraft *sc, float dh)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Horisontal];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}

void
ooScFireForward(OOspacecraft *sc)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Forward];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}

void
ooScEngageYaw(OOspacecraft *sc, float dy)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Yaw];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}

void
ooScEngagePitch(OOspacecraft *sc, float dp)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Pitch];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}

void
ooScEngageRoll(OOspacecraft *sc, float dr)
{
  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Roll];
  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
    OOactuator *act = grp->actuators.elems[i];
    act->toggleOn(act);
  }
}


OOrocket*
ooScNewEngine(OOstage *stage,
              const char *name,
              float f,
              float x, float y, float z,
              float dx, float dy, float dz)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.stage = stage;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  return engine;
}

OOactuatorgroup*
ooScNewActuatorGroup(const char *name)
{
  OOactuatorgroup *eg = malloc(sizeof(OOactuatorgroup));
  obj_array_init(&eg->actuators);
  eg->groupName = strdup(name);
  return eg;
}

void
ooScRegisterInGroup(OOactuatorgroup *eg, OOactuator *actuator)
{
  obj_array_push(&eg->actuators, actuator);
}


void
ooSrbStep(OOsrb *srb, float dt)
{
  plForceRelativePos3fv(&srb->super.stage->sc->obj->super,
                        srb->dir * srb->forceMag, srb->p);
}

void
ooRocketStep(OOrocket *rocket, float dt)
{
  plForceRelativePos3fv(&rocket->super.stage->sc->obj->super,
                        rocket->dir * rocket->forceMag * rocket->throttle,
                        rocket->p);
}

void
ooThrusterStep(OOrocket *thruster, float dt)
{
  plForceRelativePos3fv(&thruster->super.stage->sc->obj->super,
                        thruster->dir * thruster->forceMag * thruster->throttle,
                        thruster->p);
}



OOsrb* ooScNewSrb(OOstage *stage,
                  const char *name,
                  float f,
                  float x, float y, float z,
                  float dx, float dy, float dz)
{
  OOsrb *engine = malloc(sizeof(OOsrb));
  engine->super.stage = stage;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  engine->super.step = (OOactuatorstep) ooSrbStep;

  engine->ps = plNewParticleSystem(name, 100);
  plAttachParticleSystem(engine->ps, stage->obj);
  SGdrawable *dps = sgNewParticleSystem(name, "textures/particle-alpha.png", engine->ps);
  sgSceneAddObj(stage->sc->scene, dps);
  return engine;
}

OOrocket* ooScNewLoxEngine(OOstage *stage,
                           const char *name,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz,
                           float fuelPerNmPerS)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.stage = stage;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);

  engine->super.step = (OOactuatorstep) ooRocketStep;

  engine->ps = plNewParticleSystem(name, 100);
  plAttachParticleSystem(engine->ps, stage->obj);
  SGdrawable *dps = sgNewParticleSystem(name, "textures/particle-alpha.png", engine->ps);
  sgSceneAddObj(stage->sc->scene, dps);
  return engine;
}

OOrocket* ooScNewThruster(OOstage *stage,
                          const char *name,
                          float f,
                          float x, float y, float z,
                          float dx, float dy, float dz)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.stage = stage;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  engine->super.step = (OOactuatorstep) ooThrusterStep;

  engine->ps = plNewParticleSystem(name, 100);
  plAttachParticleSystem(engine->ps, stage->obj);
  SGdrawable *dps = sgNewParticleSystem(name, "textures/particle-alpha.png", engine->ps);
  sgSceneAddObj(stage->sc->scene, dps);

  return engine;
}

