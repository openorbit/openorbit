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

#include "sim/actuator.h"
#include "sim/spacecraft.h"


static void
TorquerEnable(SIMtorquer *tq)
{
}
static void
TorquerDisable(SIMtorquer *tq)
{
}
static void
TorquerStep(SIMtorquer *tq, float dt)
{
}
static void
TorquerAxisUpdate(SIMtorquer *tq, float axis)
{
}

void
simInitTorquer(SIMtorquer *tq, const char *name, float3 pos, float3 tMax, float3 tMin)
{
  tq->super.stage = NULL;
  tq->super.name = strdup(name);
  tq->super.state = OO_Act_Disarmed;
  tq->super.toggleOn = (OOactuatortoggle)TorquerEnable;
  tq->super.toggleOff = (OOactuatortoggle)TorquerDisable;
  tq->super.step = (OOactuatorstep)TorquerStep;
  tq->super.axisUpdate = (OOactuatorstep)TorquerAxisUpdate;

  tq->pos = pos;
  tq->tMax = tMax;
  tq->tMin = tMin;
}

SIMtorquer*
simNewTorquer(const char *name, float3 pos, float3 tMax, float3 tMin)
{
  SIMtorquer *tq = malloc(sizeof(SIMtorquer));
  simInitTorquer(tq, name, pos, tMax, tMin);
  return tq;
}

void
simSetTorquerPower(SIMtorquer *tq, float v)
{
  assert(tq);
  assert(-1.0f <= v && v <= 1.0f);

  tq->setting = v;
}



static void
ThrusterEnable(SIMthruster *th)
{
}
static void
ThrusterDisable(SIMthruster *th)
{
}
static void
ThrusterStep(SIMthruster *th, float dt)
{
  plForceRelativePos3fv(th->super.stage->sc->obj,
                        th->fMax * th->throttle, th->pos);
}
static void
ThrusterAxisUpdate(SIMtorquer *tq, float axis)
{
}

void
simInitThruster(SIMthruster *th, const char *name, float3 pos, float3 fMax)
{
  th->super.stage = NULL;
  th->super.name = strdup(name);
  th->super.state = OO_Act_Disarmed;
  th->super.toggleOn = (OOactuatortoggle)ThrusterEnable;
  th->super.toggleOff = (OOactuatortoggle)ThrusterDisable;
  th->super.step = (OOactuatorstep)ThrusterStep;
  th->super.axisUpdate = (OOactuatorstep)ThrusterAxisUpdate;

  th->pos = pos;
  th->fMax = fMax;
  th->throttle = 1.0;
}

void
simSetThrottle(SIMthruster *th, float throttle)
{
  if (0.0f <= throttle && throttle <= 1.0f) {

  } else {
    ooLogError("throttle set to %f", throttle);
  }

  assert(th);
  assert(0.0f <= throttle && throttle <= 1.0f);

  th->throttle = throttle;
}


SIMthruster*
simNewThruster(const char *name, float3 pos, float3 fMax)
{
  SIMthruster *th = malloc(sizeof(SIMthruster));
  simInitThruster(th, name, pos, fMax);
  return th;
}

void
simAddActuator(OOstage *stage, OOactuator *act)
{
  act->stage = stage;
  obj_array_push(&stage->actuators, act);
  obj_array_push(&stage->sc->actuators, act);
}

void
simArmActuator(OOactuator *act)
{
  assert(act->state == OO_Act_Disarmed && "invalid state");
  act->state = OO_Act_Armed;
}

void
simDisarmActuator(OOactuator *act)
{
  assert(act->state == OO_Act_Armed && "invalid state");
  act->state = OO_Act_Disarmed;
}

void
simFireActuator(OOactuator *act)
{
  assert(act->state == OO_Act_Armed && "invalid state");
  act->state = OO_Act_Enabled;
}

void
simFireThrusterIfThrottle(SIMthruster *act)
{
  assert((act->super.state == OO_Act_Armed || act->super.state == OO_Act_Enabled)
         && "invalid state");

  if (act->throttle > 0.0) act->super.state = OO_Act_Enabled;
  else act->super.state = OO_Act_Armed;
}


void
simDisableActuator(OOactuator *act)
{
  assert(act->state == OO_Act_Enabled && "invalid state");
  act->state = OO_Act_Armed;
}

void
simLockActuator(OOactuator *act)
{
  if (act->state & SIM_ACTUATOR_ON_MASK) {
    act->state = OO_Act_Locked_Open;
  } else {
    act->state = OO_Act_Locked_Closed;
  }
}

void
simFailActuator(OOactuator *act)
{
  if (act->state & SIM_ACTUATOR_ON_MASK) {
    act->state = OO_Act_Fault_Open;
  } else {
    act->state = OO_Act_Fault_Closed;
  }
}


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
ooGetActuatorGroupName(unsigned groupId)
{
  assert(groupId < OO_Act_Group_Count);
  return actuatorNames[groupId];
}

float
ooGetThrottleForActuatorGroup(unsigned groupId)
{
  float throttle = ooIoGetAxis(NULL, ooGetActuatorGroupName(groupId));
}

void
ooScFireOrbital(OOspacecraft *sc)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Orbital];

//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
//    act->toggleOn(act);
//  }
}

void
ooScFireVertical(OOspacecraft *sc, float dv)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Vertical];
//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
//    act->toggleOn(act);
//  }
}
void
ooScFireHorizontal(OOspacecraft *sc, float dh)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Horisontal];
 // for (int i = 0 ; i < grp->actuators.length ; ++ i) {
 //   OOactuator *act = grp->actuators.elems[i];
 //   act->toggleOn(act);
 // }
}

void
ooScFireForward(OOspacecraft *sc)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Forward];
//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
//    act->toggleOn(act);
//  }
}

void
ooScEngageYaw(OOspacecraft *sc, float dy)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Yaw];
//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
//    act->toggleOn(act);
//  }
}

void
ooScEngagePitch(OOspacecraft *sc, float dp)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Pitch];
//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
//    act->toggleOn(act);
//  }
}

void
ooScEngageRoll(OOspacecraft *sc, float dr)
{
//  OOstage *stage = sc->stages.elems[sc->activeStageIdx];
//  OOactuatorgroup *grp = stage->actuatorGroups.elems[OO_Act_Roll];
//  for (int i = 0 ; i < grp->actuators.length ; ++ i) {
//    OOactuator *act = grp->actuators.elems[i];
 //   act->toggleOn(act);
//  }
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

