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
#include <stddef.h>
#include <vmath/vmath.h>
typedef void* OOpubsubref;
typedef void* OOrecordtyperef;

typedef void (*OOpubsubupdatefloat)(float);
typedef void (*OOpubsubupdateint)(int);
typedef void (*OOpubsubupdatefloat3)(float3);
typedef void (*OOpubsubupdaterecord)(void *);


OOrecordtyperef simRegisterRecordType(const char *key);


void simRegisterRecordInt(OOrecordtyperef typ, const char *key, size_t offset);
void simRegisterRecordFloat(OOrecordtyperef typ, const char *key, size_t offset);
void simRegisterRecordFloat3(OOrecordtyperef typ, const char *key, size_t offset);

OOrecordtyperef simQueryRecordTyp(const char *key);

OOpubsubref simPublishRecord(OOrecordtyperef typ, const char *key, void *record);
void* simRetrieveRecord(OOpubsubref ref);

OOpubsubref simPublishInt(const char *key, int *theInt);
OOpubsubref simPublishFloat(const char *key, float *theFloat);
OOpubsubref simPublishFloat3(const char *key, float3 *vec);

int simRetrieveInt(OOpubsubref ref);
float simRetrieveFloat(OOpubsubref ref);
float3 simRetrieveFloat3(OOpubsubref ref);

OOpubsubref simQueryValueRef(const char *key);

void simNotifyChange(OOpubsubref val);

void simSubscribeRecord(OOpubsubref val, OOpubsubupdaterecord f);
void simSubscribeInt(OOpubsubref val, OOpubsubupdateint f);
void simSubscribeFloat(OOpubsubref val, OOpubsubupdatefloat f);
void simSubscribeFloat3(OOpubsubref val, OOpubsubupdatefloat3 f);

#endif /* !SIM_PUBSUB_H */
