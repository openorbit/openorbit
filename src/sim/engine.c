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


#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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


OOrocket*
ooScNewEngine(OOspacecraft *sc,
              const char *name,
              float f,
              float x, float y, float z,
              float dx, float dy, float dz)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.sc = sc;
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

OOsrb* ooScNewSrb(OOspacecraft *sc,
                  const char *name,
                  float f,
                  float x, float y, float z,
                  float dx, float dy, float dz)
{
  OOsrb *engine = malloc(sizeof(OOsrb));
  engine->super.sc = sc;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  return engine;
}

OOrocket* ooScNewLoxEngine(OOspacecraft *sc,
                           const char *name,
                           float f,
                           float x, float y, float z,
                           float dx, float dy, float dz,
                           float fuelPerNmPerS)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.sc = sc;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  return engine;
}

OOrocket* ooScNewThruster(OOspacecraft *sc,
                          const char *name,
                          float f,
                          float x, float y, float z,
                          float dx, float dy, float dz)
{
  OOrocket *engine = malloc(sizeof(OOrocket));
  engine->super.sc = sc;
  engine->super.state = OO_Act_Disabled;
  engine->super.name = strdup(name);
  engine->forceMag = f;
  engine->p = vf3_set(x, y, z);
  engine->dir = vf3_set(dx, dy, dz);
  return engine;
}
