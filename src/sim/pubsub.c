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

#include "pubsub.h"
#include <gencds/array.h>
#include <gencds/hashtable.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
enum type_id {
  SIM_PS_Int,
  SIM_PS_Float,
  SIM_PS_Float3,
  SIM_PS_Record,
};


struct elem_ref {
  enum type_id typ;
  obj_array_t updateFuncs;
  void *val;
};

struct str_offset_pair {
  const char *key;
  enum type_id typ;
  size_t offset;
};

DECL_ARRAY(struct str_offset_pair, str_offs);
DEF_ARRAY(struct str_offset_pair, str_offs);


struct rec_info {
  const char *typ_name;
  str_offs_array_t fields;
};

static hashtable_t *rectypes;
static hashtable_t *objects;

static void __attribute__((constructor))
simPubSubInit(void)
{
  rectypes = hashtable_new_with_str_keys(1024);
  objects = hashtable_new_with_str_keys(4096);
}


OOrecordtyperef
simRegisterRecordType(const char *key)
{
  assert(hashtable_lookup(rectypes, key) == NULL);

  struct rec_info *ri = malloc(sizeof(struct rec_info));
  ri->typ_name = strdup(key);
  str_offs_array_init(&ri->fields);

  hashtable_insert(rectypes, key, ri);
  return ri;
}


void
simRegisterRecordInt(OOrecordtyperef typ, const char *key, size_t offset)
{
  struct str_offset_pair sop = {.key = strdup(key), .typ = SIM_PS_Int, .offset = offset};
  struct rec_info *ri = (struct rec_info *)typ;
  str_offs_array_push(&ri->fields, sop);
}


void
simRegisterRecordFloat(OOrecordtyperef typ, const char *key, size_t offset)
{
  struct str_offset_pair sop = {.key = strdup(key), .typ = SIM_PS_Float, .offset = offset};
  struct rec_info *ri = (struct rec_info *)typ;
  str_offs_array_push(&ri->fields, sop);
}


void
simRegisterRecordFloat3(OOrecordtyperef typ, const char *key, size_t offset)
{
  struct str_offset_pair sop = {.key = strdup(key), .typ = SIM_PS_Float3, .offset = offset};
  struct rec_info *ri = (struct rec_info *)typ;
  str_offs_array_push(&ri->fields, sop);
}



OOrecordtyperef
simQueryRecordTyp(const char *key)
{
  return hashtable_lookup(rectypes, key);
}



OOpubsubref
simPublishRecord(OOrecordtyperef typ, const char *key, void *record)
{
  struct elem_ref *ref = malloc(sizeof(struct elem_ref));
  ref->typ = SIM_PS_Record;
  ref->val = record;
  obj_array_init(&ref->updateFuncs);
  hashtable_insert(objects, key, ref);

  return (OOpubsubref)ref;
}


void*
simRetrieveRecord(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  return eref->val;
}



OOpubsubref
simPublishInt(const char *key, int *theInt)
{
  struct elem_ref *ref = malloc(sizeof(struct elem_ref));
  ref->typ = SIM_PS_Int;
  ref->val = theInt;
  obj_array_init(&ref->updateFuncs);
  hashtable_insert(objects, key, ref);

  return (OOpubsubref)ref;
}


OOpubsubref
simPublishFloat(const char *key, float *theFloat)
{
  struct elem_ref *ref = malloc(sizeof(struct elem_ref));
  ref->typ = SIM_PS_Float;
  ref->val = theFloat;
  obj_array_init(&ref->updateFuncs);
  hashtable_insert(objects, key, ref);

  return (OOpubsubref)ref;
}


OOpubsubref
simPublishFloat3(const char *key, float3 *vec)
{
  struct elem_ref *ref = malloc(sizeof(struct elem_ref));
  ref->typ = SIM_PS_Float3;
  ref->val = vec;
  obj_array_init(&ref->updateFuncs);
  hashtable_insert(objects, key, ref);

  return (OOpubsubref)ref;
}



int
simRetrieveInt(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  return *(int*)((OOpubsubref)eref->val);
}


float
simRetrieveFloat(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  return *(float*)((OOpubsubref)eref->val);
}


float3
simRetrieveFloat3(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  return *(float3*)((OOpubsubref)eref->val);
}



OOpubsubref
simQueryValueRef(const char *key)
{
  return hashtable_lookup(objects, key);
}



void
simNotifyChange(OOpubsubref ref)
{
  struct elem_ref *eref = ref;

  switch (eref->typ) {
  case SIM_PS_Float:
    for (int i = 0 ; i < eref->updateFuncs.length ; ++i) {
      ((OOpubsubupdatefloat)eref->updateFuncs.elems[i])(*(float*)eref->val);
    }
    break;
  case SIM_PS_Float3:
    for (int i = 0 ; i < eref->updateFuncs.length ; ++i) {
      ((OOpubsubupdatefloat3)eref->updateFuncs.elems[i])(*(float3*)eref->val);
    }
    break;
  case SIM_PS_Int:
    for (int i = 0 ; i < eref->updateFuncs.length ; ++i) {
      ((OOpubsubupdateint)eref->updateFuncs.elems[i])(*(int*)eref->val);
    }
    break;
  case SIM_PS_Record:
    for (int i = 0 ; i < eref->updateFuncs.length ; ++i) {
      ((OOpubsubupdaterecord)eref->updateFuncs.elems[i])(eref->val);
    }
    break;
  default:
    assert(0 && "invalid case");
  }
}



void
simSubscribeRecord(OOpubsubref val, OOpubsubupdaterecord f)
{
  struct elem_ref *eref = val;
  assert(eref->typ == SIM_PS_Record && "value reference not a record");
  obj_array_push(&eref->updateFuncs, f);
}


void
simSubscribeInt(OOpubsubref val, OOpubsubupdateint f)
{
  struct elem_ref *eref = val;
  assert(eref->typ == SIM_PS_Int && "value reference not an int");
  obj_array_push(&eref->updateFuncs, f);
}


void
simSubscribeFloat(OOpubsubref val, OOpubsubupdatefloat f)
{
  struct elem_ref *eref = val;
  assert(eref->typ == SIM_PS_Float && "value reference not a float");
  obj_array_push(&eref->updateFuncs, f);
}


void
simSubscribeFloat3(OOpubsubref val, OOpubsubupdatefloat3 f)
{
  struct elem_ref *eref = val;
  assert(eref->typ == SIM_PS_Float3 && "value reference not a float3");
  obj_array_push(&eref->updateFuncs, f);
}

