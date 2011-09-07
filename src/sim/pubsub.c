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
#include "common/moduleinit.h"
#include <gencds/array.h>
#include <gencds/hashtable.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <jansson.h>

enum type_id {
  SIM_PS_Int,
  SIM_PS_Float,
  SIM_PS_Float3,
  SIM_PS_Float4,
  SIM_PS_Float9,
  SIM_PS_Float16,
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

struct rec_container_t {
  const char *name;
  obj_array_t entries;
};

static hashtable_t *rectypes;
static hashtable_t *objects;
static struct rec_container_t *root;

INIT_PRIMARY_MODULE
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


void*
simRetrieveObject(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  assert(eref->typ == SIM_PS_Record && "not a record object");
  return (void*)((OOpubsubref)eref->val);
}


int
simRetrieveInt(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  assert(eref->typ == SIM_PS_Int && "not an integer value");
  return *(int*)((OOpubsubref)eref->val);
}


float
simRetrieveFloat(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  assert(eref->typ == SIM_PS_Float && "not a float value");
  return *(float*)((OOpubsubref)eref->val);
}


float3
simRetrieveFloat3(OOpubsubref ref)
{
  struct elem_ref *eref = ref;
  assert(eref->typ == SIM_PS_Float3 && "not a float3 vector");
  return *(float3*)((OOpubsubref)eref->val);
}



OOpubsubref
simQueryValueRef(const char *key)
{
  return hashtable_lookup(objects, key);
}

OOpubsubref
simCloneRef(const char *key, OOpubsubref ref)
{
  OOpubsubref new = hashtable_lookup(objects, key);
  if (new) {
    hashtable_remove(objects, key);
  }
  hashtable_insert(objects, key, ref);

  return ref;
}




void
simNotifyChange(OOpubsubref ref)
{
  struct elem_ref *eref = ref;

  for (int i = 0 ; i < eref->updateFuncs.length ; ++i) {
    ((OOpubsubupdate)eref->updateFuncs.elems[i])(eref);
  }
}


void
simSubscribe(OOpubsubref val, OOpubsubupdate f)
{
  struct elem_ref *eref = val;
  obj_array_push(&eref->updateFuncs, f);
}


void
simDumpPubsubDB(void)
{
  list_entry_t *le = hashtable_first(objects);

  while (le) {
    const char *key =  hashtable_entry_key(le);
    struct elem_ref *ref = hashtable_entry_data(le);

    switch (ref->typ) {
    case SIM_PS_Int:
      printf("%s: %d\n", key, *(int*)ref->val);
      break;
    case SIM_PS_Float:
      printf("%s: %f\n", key, *(float*)ref->val);
      break;
    case SIM_PS_Float3: {
      float3 data = *(float3*)ref->val;
      printf("%s: [%f,%f,%f]\n", key, data.x, data.y, data.z);
      break; }
    case SIM_PS_Record:
    default:
      assert(0 && "not implemented");
    }

    le = list_entry_next(le);
  }
}

// JSON validation codes
static const char *mat4x4_str = "[FFFF, FFFF, FFFF, FFFF!]";
static const char *mat3x3_str = "[FFF, FFF, FFF!]";
static const char *float4_str = "[FFFF!]";
static const char *float3_str = "[FFF!]";
static const char *float_str = "F";
static const char *int_str = "I";

void
simPackPubSubDB(void)
{
  struct rec_container_t *rec_cont = root;

  ARRAY_FOR_EACH(i, rec_cont->entries) {
#if 0
    switch (((struct rec_container_t)ARRAY_ELEM(rec_cont->entries, i))) {
      case SIM_PS_Int:
        break;
      case SIM_PS_Float:
        break;
      case SIM_PS_Float3: {
        float3 data = *(float3*)ref->val;
        break; }
      case SIM_PS_Record:
      default:
        assert(0 && "not implemented");
    }
#endif
  }
}


void
simUnpackPubSubDB(void)
{

}
