/*
 *  engine.c
 *  orbit
 *
 *  Created by Mattias Holm on 2009-11-16.
 *  Copyright 2009 LIACS. All rights reserved.
 *
 */

#include "engine.h"
#include <stdlib.h>
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
