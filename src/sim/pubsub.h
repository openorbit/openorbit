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

#ifndef SIM_PUBSUB_H
#define SIM_PUBSUB_H

/* We need to define a proper pub sub api for getting data references.
   In general, we need to be able to register two kinds of data, firstly
   singleton data, the second is object data, where an abstract object need to
   be created and queried.
 */
#include <vmath/vmath.h>
typedef void* OOpubsubref;
typedef void (*OOpubsubupdatefloat)(float f);

OOpubsubref ooPubSubPublishInt(const char *key, int *theInt);
OOpubsubref ooPubSubPublishFloat(const char *key, float *theFloat);
OOpubsubref ooPubSubPublishFloat3(const char *key, float3 *vec);

OOpubsubref ooPubSubQueryPS(const char *key);

void ooPubSubNotifyChange(OOpubsubref val);

void ooPubSubSubscribeFloat(OOpubsubref val, OOpubsubupdatefloat f);


#endif /* !SIM_PUBSUB_H */
