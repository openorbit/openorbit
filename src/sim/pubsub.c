/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pubsub.h"
#include "common/moduleinit.h"
#include <openorbit/log.h>
#include "common/palloc.h"
#include "common/stringextras.h"

#include <gencds/array.h>
#include <gencds/hashtable.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include <jansson.h>

static sim_record_t *_root;

MODULE_INIT(pubsub, NULL)
{
  log_trace("initialising 'pubsub' module");
  _root = simPubsubMakeRecord(NULL, NULL);
}


void
simDumpPubsubDB(void)
{
#if 0
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
#endif
}

sim_value_t*
simGetValueByIndex(sim_record_t *rec, int idx)
{
  if (!rec) return NULL;

  sim_base_t *base = obj_array_get(&rec->entries, idx);
  if (base->type == SIM_TYPE_RECORD) {
    log_error("queried for value, but found record");
    return NULL;
  } else if (base->type == SIM_TYPE_LINK){
    log_error("queried for value, but found link");
    return NULL;
  }

  return (sim_value_t*)base;
}

sim_value_t*
simGetValueByName(sim_record_t *rec, const char *name)
{
  if (!rec) return NULL;

  intptr_t idx = ((intptr_t) hashtable_lookup(rec->key_index_map, name)) - 1;
  if (idx < 0) {
    log_error("unknown name '%s' in record", name);
    return NULL;
  }

  return simGetValueByIndex(rec, idx);
}

sim_record_t*
simGetRecordByIndex(sim_record_t *rec, int idx)
{
  if (!rec) return NULL;
  sim_base_t *base = obj_array_get(&rec->entries, idx);
  if (base->type == SIM_TYPE_RECORD) {
    return (sim_record_t*)base;
  } else if (base->type == SIM_TYPE_LINK) {
    return ((sim_link_t*)base)->target;
  }

  log_error("queried for record, but found value");
  return NULL;
}

sim_record_t*
simGetRecordByName(sim_record_t *rec, const char *name)
{
  if (!rec) return NULL;
  intptr_t idx = ((intptr_t) hashtable_lookup(rec->key_index_map, name)) - 1;
  if (idx < 0) {
    log_error("unknown name '%s' in record", name);
    return NULL;
  }

  return simGetRecordByIndex(rec, idx);
}

sim_record_t*
simGetRecordByNameSilent(sim_record_t *rec, const char *name)
{
  if (!rec) return NULL;
  intptr_t idx = ((intptr_t) hashtable_lookup(rec->key_index_map, name)) - 1;
  if (idx < 0) {
    return NULL;
  }

  return simGetRecordByIndex(rec, idx);
}



sim_record_t*
simPubsubGetRecordWithComps(const char *fst_comp, ...)
{
  sim_record_t *rec = _root;
  va_list ap;
  va_start(ap, fst_comp);

  const char *comp_name = fst_comp;
  while (comp_name) {
    rec = simGetRecordByName(rec, comp_name);
    comp_name = va_arg(ap, const char *);
  }
  va_end(ap);

  return rec;
}


sim_record_t*
simPubsubGetRecord(const char *path)
{
  assert(path[0] == '/');
  const char *comp_start = path;
  sim_record_t *rec = _root;

  char key[SIM_MAX_PUBSUB_COMP_KEY_BUFF_SIZE];

  while (*comp_start) {
    comp_start = strtcpy(key, comp_start+1, '/', sizeof(key));
    rec = simGetRecordByName(rec, key);
    if (!rec) {
      log_trace("could not find '%s'", path);
      return NULL;
    }
  }

  return rec;
}

sim_record_t*
simPubsubCreateRecord(const char *path)
{
  assert(path[0] == '/');
  const char *comp_start = path;
  sim_record_t *rec = _root;
  sim_record_t *parent = rec;
  char key[SIM_MAX_PUBSUB_COMP_KEY_BUFF_SIZE];

  while (*comp_start) {
    comp_start = strtcpy(key, comp_start+1, '/', sizeof(key));
    rec = simGetRecordByNameSilent(parent, key);
    if (!rec) {
      rec = simPubsubMakeRecord(parent, key);
    }
    parent = rec;
  }

  return rec;
}

sim_value_t*
simPubsubGetValue(const char *path)
{
  assert(path[0] == '/');
  const char *comp_start = path;
  sim_record_t *rec = _root;
  sim_value_t *val = NULL;

  char key[SIM_MAX_PUBSUB_COMP_KEY_BUFF_SIZE];

  while (*comp_start) {
    comp_start = strtcpy(key, comp_start+1, '/', sizeof(key));

    if (*comp_start) {
      rec = simGetRecordByName(rec, key);
      if (!rec) {
        log_trace("could not find '%s'", path);
        return NULL;
      }
    } else {
      val = simGetValueByName(rec, key);
      if (!val) {
        log_trace("could not find '%s'", path);
        return NULL;
      }
    }
  }

  return val;
}



sim_link_t*
simPubsubMakeLink(sim_record_t *parent, const char *name)
{
  if (!parent) {
    log_error("cannot make the root object a link");
    return NULL;
  }
  if (!name) {
    log_error("cannot make link with no name");
    return NULL;
  }

  intptr_t link_id = ((intptr_t)hashtable_lookup(parent->key_index_map, name))-1;

  if (link_id < 0) {
    // Does not exist, create new link
    sim_link_t *link = smalloc(sizeof(sim_link_t));
    link->target = NULL;
    link->super.type = SIM_TYPE_LINK;
    link->super.name = strdup(name);

    obj_array_push(&parent->entries, link);
    // Note, index 1 represents object 0 as 0 is returned when
    // there is no entry in the hashtable
    hashtable_insert(parent->key_index_map, name, (void*)ARRAY_LEN(parent->entries));
    return link;
  } else {
    // Already exists
    sim_base_t *base = obj_array_get(&parent->entries, link_id);
    if (base->type == SIM_TYPE_LINK) {
      sim_link_t *link = (sim_link_t*)base;
      link->target = NULL; // Just clear the target and return
      return link;
    }
    log_error("tried to make a link over a record/value '%s'", name);
    return NULL;
  }
}


sim_record_t*
simPubsubMakeRecord(sim_record_t *parent, const char *name)
{
  if (parent && !name) {
    log_error("cannot make record with a non null parent and no name");
    return NULL;
  }

  if (!parent && name) {
    log_error("cannot make root record with a name");
    return NULL;
  }

  sim_record_t *rec = smalloc(sizeof(sim_record_t));
  rec->key_index_map = hashtable_new_with_str_keys(32);
  assert(rec->key_index_map);
  obj_array_init(&rec->entries);
  assert(rec->entries.elems);
  rec->super.type = SIM_TYPE_RECORD;

  rec->super.name = name ? strdup(name) : "";

  if (parent) {
    obj_array_push(&parent->entries, rec);
    // Note, index 1 represents object 0 as 0 is returned when
    // there is no entry in the hashtable
    hashtable_insert(parent->key_index_map, name, (void*)ARRAY_LEN(parent->entries));
  }
  return rec;
}

sim_value_t*
simPubsubMakeValue(sim_record_t *parent, sim_type_id_t typ, const char *name,
                   void *val)
{
  intptr_t idx = ((intptr_t)hashtable_lookup(parent->key_index_map, name)) - 1;

  if (idx < 0) {
    // key did not exist
    sim_value_t *val_desc = smalloc(sizeof(sim_value_t));
    val_desc->super.type = typ;
    val_desc->super.name = strdup(name);
    val_desc->i = val;
    obj_array_init(&val_desc->updateFuncs);

    obj_array_push(&parent->entries, val_desc);
    // Note, index 1 represents object 0 as 0 is returned when
    // there is no entry in the hashtable
    hashtable_insert(parent->key_index_map, name,
                     (void*)ARRAY_LEN(parent->entries));
    return val_desc;
  } else {
    log_error("publishing variable that is already published");
    return NULL;
  }
}


size_t _val_offsets[SIM_TYPE_COUNT] = {
  [SIM_TYPE_BOOL] = offsetof(sim_bool_t, val),
  [SIM_TYPE_CHAR] = offsetof(sim_char_t, val),
  [SIM_TYPE_UCHAR] = offsetof(sim_uchar_t, val),
  [SIM_TYPE_SHORT] = offsetof(sim_short_t, val),
  [SIM_TYPE_USHORT] = offsetof(sim_ushort_t, val),
  [SIM_TYPE_INT] = offsetof(sim_int_t, val),
  [SIM_TYPE_UINT] = offsetof(sim_uint_t, val),
  [SIM_TYPE_LONG] = offsetof(sim_long_t, val),
  [SIM_TYPE_ULONG] = offsetof(sim_ulong_t, val),

  [SIM_TYPE_UINT8] = offsetof(sim_uint8_t, val),
  [SIM_TYPE_UINT16] = offsetof(sim_uint16_t, val),
  [SIM_TYPE_UINT32] = offsetof(sim_uint32_t, val),
  [SIM_TYPE_UINT64] = offsetof(sim_uint64_t, val),
  [SIM_TYPE_INT8] = offsetof(sim_int8_t, val),
  [SIM_TYPE_INT16] = offsetof(sim_int16_t, val),
  [SIM_TYPE_INT32] = offsetof(sim_int32_t, val),
  [SIM_TYPE_INT64] = offsetof(sim_int64_t, val),

  [SIM_TYPE_STR] = offsetof(sim_str_t, val),

  [SIM_TYPE_FLOAT] = offsetof(sim_float_t, val),
  [SIM_TYPE_DOUBLE] = offsetof(sim_double_t, val),
  [SIM_TYPE_COMPLEX_FLOAT] = offsetof(sim_complex_float_t, val),
  [SIM_TYPE_COMPLEX_DOUBLE] = offsetof(sim_complex_double_t, val),

  [SIM_TYPE_FLOAT_VEC3] = offsetof(sim_float3_t, val),
  [SIM_TYPE_FLOAT_VEC4] = offsetof(sim_float4_t, val),
  [SIM_TYPE_FLOAT_VEC3x3] = offsetof(sim_float3x3_t, val),
  [SIM_TYPE_FLOAT_VEC4x4] = offsetof(sim_float4x4_t, val),

  [SIM_TYPE_BOOL_ARR] = offsetof(sim_bool_array_t, val),
  [SIM_TYPE_CHAR_ARR] = offsetof(sim_char_array_t, val),
  [SIM_TYPE_UCHAR_ARR] = offsetof(sim_uchar_array_t, val),
  [SIM_TYPE_SHORT_ARR] = offsetof(sim_short_array_t, val),
  [SIM_TYPE_USHORT_ARR] = offsetof(sim_ushort_array_t, val),
  [SIM_TYPE_INT_ARR] = offsetof(sim_int_array_t, val),
  [SIM_TYPE_UINT_ARR] = offsetof(sim_uint_array_t, val),
  [SIM_TYPE_LONG_ARR] = offsetof(sim_long_array_t, val),
  [SIM_TYPE_ULONG_ARR] = offsetof(sim_ulong_array_t, val),

  [SIM_TYPE_UINT8_ARR] = offsetof(sim_uint8_array_t, val),
  [SIM_TYPE_UINT16_ARR] = offsetof(sim_uint16_array_t, val),
  [SIM_TYPE_UINT32_ARR] = offsetof(sim_uint32_array_t, val),
  [SIM_TYPE_UINT64_ARR] = offsetof(sim_uint64_array_t, val),

  [SIM_TYPE_INT8_ARR] = offsetof(sim_int8_array_t, val),
  [SIM_TYPE_INT16_ARR] = offsetof(sim_int16_array_t, val),
  [SIM_TYPE_INT32_ARR] = offsetof(sim_int32_array_t, val),
  [SIM_TYPE_INT64_ARR] = offsetof(sim_int64_array_t, val),

  [SIM_TYPE_FLOAT_ARR] = offsetof(sim_float_array_t, val),
  [SIM_TYPE_DOUBLE_ARR] = offsetof(sim_double_array_t, val),

  [SIM_TYPE_COMPLEX_FLOAT_ARR] = offsetof(sim_complex_float_array_t, val),
  [SIM_TYPE_COMPLEX_DOUBLE_ARR] = offsetof(sim_complex_double_array_t, val),

  [SIM_TYPE_OBJ_ARR] = offsetof(sim_obj_array_t, val),
};

static inline sim_value_t*
simGetValueDesc(void *sim_val)
{
  sim_char_t *val = sim_val;
  return val->ref;
}

static inline void*
simGetValuePtr(sim_type_id_t type, void *sim_val)
{
  void *res = sim_val + _val_offsets[type];
  return res;
}


void
simPublishValue(sim_record_t *parent, sim_type_id_t type, const char *key,
                void *sim_val)
{
  // Note that ref must be first field in the struct
  sim_char_t *val = sim_val;
  val->ref = simPubsubMakeValue(parent, type, key,
                                simGetValuePtr(type, sim_val));
}

void
simNotifyChangedVal(void *sim_val)
{
  sim_value_t *val = simGetValueDesc(sim_val);
  ARRAY_FOR_EACH(i, val->updateFuncs) {
    SIMvalueobserver observer = ARRAY_ELEM(val->updateFuncs, i);
    observer(val);
  }
}

void
simObserveValue(sim_value_t *val, SIMvalueobserver f)
{
  obj_array_push(&val->updateFuncs, f);
}

void
simObserveNamedValue(const char *key, SIMvalueobserver f)
{
  sim_value_t *val = simPubsubGetValue(key);
  if (val) {
    simObserveValue(val, f);
  }
}

void
simObserveNamedValueOfRecord(sim_record_t *parent, const char *key,
                             SIMvalueobserver f)
{
  sim_value_t *val = simGetValueByName(parent, key);
  if (val) {
    simObserveValue(val, f);
  }
}

void
simLinkRecord(sim_record_t *parent, const char *key, sim_record_t *rec)
{
  sim_link_t *link = simPubsubMakeLink(parent, key);
  if (link) {
    link->target = rec;
  }
}

void
simPubsubSetVal(sim_value_t *ref, sim_type_id_t type_id, void *val)
{
  if (ref->super.type != type_id) {
    log_error("SetValue called, but type_id is not matching the descriptor");
    return;
  }

#define CASE(A, a) case SIM_TYPE_##A:\
SIM_REF_##A(ref) = *(a*)val;\
break

#define UCASE(A, a) CASE(A, a);\
CASE(U##A, unsigned a)

#define UCASE_FIX(A, a) CASE(A, a);\
CASE(U##A, u##a)


  switch (type_id) {
    case SIM_TYPE_STR:
      SIM_REF_STR(ref) = strdup(*(char **)val);
      break;
      CASE(BOOL,_Bool);
      UCASE(CHAR,char);
      UCASE(SHORT,short);
      UCASE(INT,int);
      UCASE(LONG,long);
      UCASE_FIX(INT8, int8_t);
      UCASE_FIX(INT16, int16_t);
      UCASE_FIX(INT32, int32_t);
      UCASE_FIX(INT64, int64_t);
      CASE(FLOAT,float);
      CASE(DOUBLE,double);
      CASE(COMPLEX_FLOAT,_Complex float);
      CASE(COMPLEX_DOUBLE,_Complex double);
      CASE(FLOAT_VEC3,float3);
      CASE(FLOAT_VEC4,float4);
      //CASE(FLOAT_VEC3x3,float3x3);
      //CASE(FLOAT_VEC4x4,float4x4);
    default:
      log_error("asignment of non supported value type");
  }

#undef UCASE_FIX
#undef UCASE
#undef CASE
}


void
simPubsubGetVal(sim_value_t *ref, sim_type_id_t type_id, void *val)
{
  if (ref->super.type != type_id) {
    log_error("SetValue called, but type_id is not matching the descriptor");
    return;
  }

#define CASE(A, a) case SIM_TYPE_##A:\
  *(a*)val = SIM_REF_##A(ref);\
  break

#define UCASE(A, a) CASE(A, a);\
  CASE(U##A, unsigned a)

#define UCASE_FIX(A, a) CASE(A, a);\
  CASE(U##A, u##a)


  switch (type_id) {
    case SIM_TYPE_STR:
      SIM_REF_STR(ref) = strdup(*(char **)val);
      break;
    CASE(BOOL,_Bool);
    UCASE(CHAR,char);
    UCASE(SHORT,short);
    UCASE(INT,int);
    UCASE(LONG,long);
    UCASE_FIX(INT8, int8_t);
    UCASE_FIX(INT16, int16_t);
    UCASE_FIX(INT32, int32_t);
    UCASE_FIX(INT64, int64_t);
    CASE(FLOAT,float);
    CASE(DOUBLE,double);
    CASE(COMPLEX_FLOAT,_Complex float);
    CASE(COMPLEX_DOUBLE,_Complex double);
    CASE(FLOAT_VEC3,float3);
    CASE(FLOAT_VEC4,float4);
    //CASE(FLOAT_VEC3x3,float3x3);
    //CASE(FLOAT_VEC4x4,float4x4);
    default:
      log_error("asignment of non supported value type");
  }

#undef UCASE_FIX
#undef UCASE
#undef CASE
}



#include <jansson.h>
// JSON validation codes
// TODO
//static const char *mat4x4_str = "[FFFF, FFFF, FFFF, FFFF!]";
//static const char *mat3x3_str = "[FFF, FFF, FFF!]";
//static const char *float4_str = "[FFFF!]";
//static const char *float3_str = "[FFF!]";
//static const char *float_str = "F";
//static const char *int_str = "I";

void
simPackPubSubDB(void)
{
#if 0
  struct rec_container_t *rec_cont = root;

  json_t *rootObj = json_object();
  ARRAY_FOR_EACH(i, rec_cont->entries) {
    struct rec_container_t *rec = ARRAY_ELEM(rec_cont->entries, i);
    json_object_set(rootObj, rec->name, json_object());

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
  }
#endif

}


void
simUnpackPubSubDB(void)
{

}
