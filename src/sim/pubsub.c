/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include <stddef.h>
#include "pubsub.h"

OOpubsubref
ooPubSubPublishInt(const char *key, int *theInt)
{
  return NULL;
}
OOpubsubref
ooPubSubPublishFloat(const char *key, float *theFloat)
{
  return NULL;
}
OOpubsubref
ooPubSubPublishFloat3(const char *key, float3 *vec)
{
  return NULL;
}


OOpubsubref ooPubSubQueryPS(const char *key)
{
  return NULL;
}


void
ooPubSubNotifyChange(OOpubsubref val)
{
}

void
ooPubSubSubscribeFloat(OOpubsubref val, OOpubsubupdatefloat f)
{

}
